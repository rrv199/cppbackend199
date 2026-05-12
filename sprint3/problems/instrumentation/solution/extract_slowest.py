import re

def get_slowest_function(profile_file):
    """Extract the slowest function from gprof output"""
    try:
        with open(profile_file, 'r') as f:
            lines = f.readlines()
        
        in_flat = False
        functions = []
        
        for line in lines:
            if 'Flat profile:' in line:
                in_flat = True
                continue
            if in_flat and line.strip() and not line.startswith('%') and not line.startswith('time'):
                parts = line.split()
                if len(parts) >= 6:
                    try:
                        # Check if first part is a number (percentage)
                        percent = float(parts[0])
                        if 0 < percent < 100:
                            func_name = parts[-1]
                            # Clean function name
                            if '(' in func_name:
                                func_name = func_name.split('(')[0]
                            func_name = func_name.strip()
                            # Filter out system functions
                            if func_name and not func_name.startswith('__') and func_name not in ['main', 'frame_dummy']:
                                functions.append((percent, func_name))
                    except:
                        pass
            elif in_flat and line.strip() == '':
                break
        
        if functions:
            functions.sort(reverse=True)
            return functions[0][1]
    except Exception as e:
        print(f"Error: {e}")
    return "unknown"

# Get slowest functions for each version
functions = []
for i in range(3):
    func = get_slowest_function(f'profile_v{i}.txt')
    functions.append(func)
    print(f"Version {i}: {func}")

# Write report
with open('report', 'w') as f:
    for func in functions:
        f.write(f"{func}\n")

print(f"\nReport written to 'report'")
