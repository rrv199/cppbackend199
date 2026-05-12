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
    '/api/v1/maps/unknown',
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
    for i in range(50):  # Делаем 50 запросов
        endpoint = random.choice(AMMUNITION)
        cmd = ['curl', '-s', f'http://localhost:8080{endpoint}', '-o', '/dev/null']
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(0.1)

def main():
    # Получаем команду для запуска сервера
    server_args = start_server()
    
    # Запускаем сервер
    server_process = run_server(server_args)
    
    # Даем серверу время на запуск
    time.sleep(2)
    
    # Запускаем perf record для профилирования
    perf_cmd = ['perf', 'record', '-o', 'perf.data', '-p', str(server_process.pid), '--', 'sleep', '10']
    perf_process = subprocess.Popen(perf_cmd)
    
    # Даем perf немного времени
    time.sleep(1)
    
    # Обстреливаем сервер запросами
    make_shots()
    
    # Ждем окончания записи perf
    perf_process.wait(timeout=15)
    
    # Останавливаем сервер
    server_process.terminate()
    server_process.wait(timeout=5)
    
    # Генерируем flamegraph
    # Проверяем наличие FlameGraph директории
    if not os.path.exists('FlameGraph'):
        print("Cloning FlameGraph repository...")
        subprocess.run(['git', 'clone', 'https://github.com/brendangregg/FlameGraph.git'])
    
    # Запускаем пайп для генерации флеймграфа
    with subprocess.Popen(['perf', 'script', '-i', 'perf.data'], stdout=subprocess.PIPE) as perf_script:
        with subprocess.Popen(['./FlameGraph/stackcollapse-perf.pl'], stdin=perf_script.stdout, stdout=subprocess.PIPE) as stackcollapse:
            with subprocess.Popen(['./FlameGraph/flamegraph.pl'], stdin=stackcollapse.stdout, stdout=subprocess.PIPE) as flamegraph:
                with open('graph.svg', 'wb') as f:
                    f.write(flamegraph.communicate()[0])
    
    print("Flamegraph generated: graph.svg")

if __name__ == '__main__':
    main()
