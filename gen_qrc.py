import os

# The .qrc file is at src/app/icons/hicolor.qrc
# All icon paths in the qrc must be relative to src/app/icons/
qrc_path = r"src\app\icons\hicolor.qrc"
base = r"src\app\icons\hicolor\scalable"
qrc_dir = r"src\app\icons"

lines = ['<RCC>', '    <qresource prefix="/icons/friction">']

for root, dirs, files in os.walk(base):
    dirs.sort()
    for f in sorted(files):
        if not f.endswith('.svg'):
            continue
        full = os.path.join(root, f)
        # relative to qrc_dir (i.e., src\app\icons)
        rel = os.path.relpath(full, qrc_dir).replace("\\", "/")
        alias = os.path.splitext(f)[0]
        lines.append(f'        <file alias="{alias}">{rel}</file>')

lines += ['    </qresource>', '</RCC>', '']

with open(qrc_path, 'w', newline='\n') as fh:
    fh.write('\n'.join(lines))

print(f"Generated {qrc_path} with {len(lines)-4} entries")
print("Sample path:", lines[3] if len(lines) > 3 else "N/A")
