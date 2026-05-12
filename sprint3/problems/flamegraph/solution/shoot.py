#!/usr/bin/env python3
import subprocess
import time
import os
import shlex
import random
import argparse
import signal

AMMUNITION = [
    '/api/v1/maps',
    '/api/v1/maps/map1',
    '/api/v1/maps/town',
    '/api/v1/game/join',
    '/api/v1/game/state',
]

def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', help='Command to start server')
    args = parser.parse_args()
    return shlex.split(args.server)

def run_server(server_args):
    return subprocess.Popen(server_args)

def make_shots():
    for i in range(300):
        endpoint = random.choice(AMMUNITION)
        cmd = ['curl', '-s', f'http://localhost:8080{endpoint}', '-o', '/dev/null']
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if i % 50 == 0:
            print(f"  Request {i}/300")
        time.sleep(0.02)

def main():
    server_args = start_server()
    
    print("Starting server...")
    server_process = run_server(server_args)
    time.sleep(3)
    
    pid = server_process.pid
    print(f"Server PID: {pid}")
    
    # Запускаем perf record в фоне
    print("Starting perf record...")
    perf_cmd = ['perf', 'record', '-F', '99', '-g', '-o', 'perf.data', '-p', str(pid), '--', 'sleep', '30']
    perf_process = subprocess.Popen(perf_cmd, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    
    time.sleep(2)
    
    print("Making requests...")
    make_shots()
    
    print("Waiting for perf to finish...")
    perf_process.wait(timeout=35)
    
    print("Stopping server...")
    server_process.terminate()
    server_process.wait(timeout=5)
    
    # Проверяем perf.data
    if os.path.exists('perf.data') and os.path.getsize('perf.data') > 0:
        print(f"perf.data size: {os.path.getsize('perf.data')} bytes")
    else:
        print("Error: perf.data is empty")
        return
    
    # Клонируем FlameGraph
    if not os.path.exists('FlameGraph'):
        print("Cloning FlameGraph...")
        subprocess.run(['git', 'clone', 'https://github.com/brendangregg/FlameGraph.git'])
    
    # Генерируем флеймграф
    print("Generating flamegraph...")
    
    # Создаем пайп для генерации
    perf_script = subprocess.Popen(['perf', 'script', '-i', 'perf.data'], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    stackcollapse = subprocess.Popen(['./FlameGraph/stackcollapse-perf.pl'], stdin=perf_script.stdout, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    flamegraph = subprocess.Popen(['./FlameGraph/flamegraph.pl', '--title', 'Game Server Flame Graph'], stdin=stackcollapse.stdout, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    
    with open('graph.svg', 'wb') as f:
        f.write(flamegraph.communicate()[0])
    
    # Проверяем результат
    if os.path.exists('graph.svg') and os.path.getsize('graph.svg') > 10000:
        print(f"Success: graph.svg size: {os.path.getsize('graph.svg')} bytes")
    else:
        print(f"Warning: graph.svg size: {os.path.getsize('graph.svg') if os.path.exists('graph.svg') else 0} bytes")

if __name__ == '__main__':
    main()
