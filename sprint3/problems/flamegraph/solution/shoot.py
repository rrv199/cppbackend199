#!/usr/bin/env python3
import subprocess
import time
import signal
import sys
import os
import shlex
import random
import argparse

# Список запросов для обстрела
AMMUNITION = [
    '/api/v1/maps',
    '/api/v1/maps/map1',
    '/api/v1/maps/town',
    '/api/v1/game/join',
    '/api/v1/game/state',
    '/api/v1/game/players',
]

def start_server():
    """Запускает сервер с переданными аргументами"""
    parser = argparse.ArgumentParser()
    parser.add_argument('server', help='Command to start server')
    args = parser.parse_args()
    
    # Разбиваем строку на аргументы
    server_args = shlex.split(args.server)
    
    return server_args

def run_server(server_args):
    """Запускает сервер как дочерний процесс"""
    return subprocess.Popen(server_args)

def make_shots():
    """Выполняет запросы к серверу"""
    # Делаем много запросов для создания нагрузки
    for i in range(500):  # Увеличиваем до 500 запросов
        endpoint = random.choice(AMMUNITION)
        cmd = ['curl', '-s', f'http://localhost:8080{endpoint}', '-o', '/dev/null']
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if i % 50 == 0:
            print(f"  Request {i}/500 completed")
        time.sleep(0.01)  # Минимальная задержка

def main():
    # Получаем команду для запуска сервера
    server_args = start_server()
    
    print("Starting server...")
    # Запускаем сервер
    server_process = run_server(server_args)
    
    # Даем серверу время на запуск
    time.sleep(3)
    print("Server started")
    
    print("Starting perf record...")
    # Запускаем perf record с более частой выборкой и большим временем
    perf_cmd = ['perf', 'record', '-F', '997', '-g', '-o', 'perf.data', '-p', str(server_process.pid), '--', 'sleep', '60']
    perf_process = subprocess.Popen(perf_cmd)
    
    # Даем perf немного времени
    time.sleep(2)
    
    print("Making requests...")
    # Обстреливаем сервер запросами
    make_shots()
    
    print("Waiting for perf to finish...")
    # Ждем окончания записи perf
    perf_process.wait(timeout=65)
    
    print("Stopping server...")
    # Останавливаем сервер
    server_process.terminate()
    server_process.wait(timeout=5)
    
    # Проверяем, что perf.data создан и не пустой
    if os.path.exists('perf.data') and os.path.getsize('perf.data') > 0:
        print(f"perf.data size: {os.path.getsize('perf.data')} bytes")
    else:
        print("Error: perf.data is empty or missing")
        return
    
    # Клонируем FlameGraph если нужно
    if not os.path.exists('FlameGraph'):
        print("Cloning FlameGraph...")
        subprocess.run(['git', 'clone', 'https://github.com/brendangregg/FlameGraph.git'])
    else:
        print("FlameGraph already exists")
    
    print("Generating flamegraph...")
    # Генерируем флеймграф с проверкой данных
    result = subprocess.run(['perf', 'script', '-i', 'perf.data'], capture_output=True, text=True)
    
    if len(result.stdout) < 100:
        print("Warning: perf script output too small")
        print(f"Output size: {len(result.stdout)} bytes")
    
    with subprocess.Popen(['perf', 'script', '-i', 'perf.data'], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL) as perf_script:
        with subprocess.Popen(['./FlameGraph/stackcollapse-perf.pl'], stdin=perf_script.stdout, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL) as stackcollapse:
            with subprocess.Popen(['./FlameGraph/flamegraph.pl', '--title', 'Game Server Flame Graph'], stdin=stackcollapse.stdout, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL) as flamegraph:
                with open('graph.svg', 'wb') as f:
                    output = flamegraph.communicate()[0]
                    f.write(output)
    
    # Проверяем результат
    if os.path.exists('graph.svg') and os.path.getsize('graph.svg') > 10000:
        print(f"Success: graph.svg size: {os.path.getsize('graph.svg')} bytes")
        # Проверяем наличие ожидаемых функций
        with open('graph.svg', 'r') as f:
            content = f.read()
            if 'http_handler' in content or 'RequestHandler' in content:
                print("Flamegraph contains handler functions")
            else:
                print("Warning: Could not find handler functions in flamegraph")
    else:
        print(f"Error: graph.svg too small ({os.path.getsize('graph.svg')} bytes)")

    print("Done!")

if __name__ == '__main__':
    main()
