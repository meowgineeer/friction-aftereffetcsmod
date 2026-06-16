import os

base = r"src\app\icons\hicolor\scalable"
qrc_path = r"src\app\icons\hicolor.qrc"

lines = ['<RCC>', '    <qresource prefix="/icons/friction">']

for root, dirs, files in os.walk(base):
    dirs.sort()
    for f in sorted(files):
        if not f.endswith('.svg'):
            continue
        full = os.path.join(root, f)
        # relative to src\app
        rel = os.path.relpath(full, r"src\app").replace("\\", "/")
        # alias = just filename without extension
        alias = os.path.splitext(f)[0]
        lines.append(f'        <file alias="{alias}">{rel}</file>')

lines += ['    </qresource>', '</RCC>', '']

with open(qrc_path, 'w', newline='\n') as fh:
    fh.write('\n'.join(lines))

print(f"Generated {qrc_path} with {len(lines)-4} icon entries")
