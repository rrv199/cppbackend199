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
]

def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', help='Command to start server')
    args = parser.parse_args()
    return shlex.split(args.server)

def run_server(server_args):
    return subprocess.Popen(server_args)

def make_shots():
    for i in range(200):
        endpoint = random.choice(AMMUNITION)
        cmd = ['curl', '-s', f'http://localhost:8080{endpoint}', '-o', '/dev/null']
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(0.05)

def main():
    server_args = start_server()
    
    print("Starting server...")
    server_process = run_server(server_args)
    time.sleep(3)
    
    pid = server_process.pid
    print(f"Server PID: {pid}")
    
    # Запускаем sample.sh в фоне
    sample_cmd = ['./sample.sh', str(pid), '50', '0.1', 'stacks.txt']
    print(f"Running: {' '.join(sample_cmd)}")
    sample_process = subprocess.Popen(sample_cmd)
    
    print("Making requests...")
    make_shots()
    
    print("Waiting for sampling to complete...")
    sample_process.wait(timeout=30)
    
    print("Stopping server...")
    server_process.terminate()
    server_process.wait(timeout=5)
    
    # Генерируем флеймграф из стеков
    print("Generating flamegraph...")
    
    # Клонируем FlameGraph если нужно
    if not os.path.exists('FlameGraph'):
        subprocess.run(['git', 'clone', 'https://github.com/brendangregg/FlameGraph.git'])
    
    # Конвертируем стеки в формат для flamegraph
    with open('stacks.txt', 'r') as f:
        content = f.read()
    
    # Создаем коллапсированный вывод
    with open('stacks.folded', 'w') as f:
        # Парсим GDB output
        lines = content.split('\n')
        current_trace = []
        for line in lines:
            if 'Sample' in line:
                if current_trace:
                    # Записываем трейс
                    trace_str = ';'.join(current_trace)
                    f.write(f"{trace_str} 1\n")
                current_trace = []
            elif '#0' in line or '#1' in line or '#2' in line or '#3' in line or '#4' in line:
                # Извлекаем имя функции
                parts = line.split()
                if len(parts) > 1:
                    func = parts[1].split('(')[0]
                    if func and not func.startswith('0x'):
                        current_trace.append(func)
        if current_trace:
            trace_str = ';'.join(current_trace)
            f.write(f"{trace_str} 1\n")
    
    # Запускаем flamegraph.pl
    with open('stacks.folded', 'r') as input_f:
        with open('graph.svg', 'w') as output_f:
            subprocess.run(
                ['./FlameGraph/flamegraph.pl', '--title', 'Poor Mans Profiler Flame Graph'],
                stdin=input_f,
                stdout=output_f
            )
    
    print("Flamegraph generated: graph.svg")
    
    # Проверяем результат
    if os.path.exists('graph.svg') and os.path.getsize('graph.svg') > 1000:
        print(f"Success: graph.svg size: {os.path.getsize('graph.svg')} bytes")
    else:
        print("Warning: graph.svg is too small")

if __name__ == '__main__':
    main()
