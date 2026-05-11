import re

with open('json_loader.cpp', 'r') as f:
    content = f.read()

# Заменяем чтение offset
content = re.sub(
    r'offset\.dx = .*?office_obj\.at\("offsetX"\).*?;',
    'offset.dx = office_obj.at("offsetX").as_int64();',
    content
)
content = re.sub(
    r'offset\.dy = .*?office_obj\.at\("offsetY"\).*?;',
    'offset.dy = office_obj.at("offsetY").as_int64();',
    content
)

with open('json_loader.cpp', 'w') as f:
    f.write(content)
print("Fixed")
