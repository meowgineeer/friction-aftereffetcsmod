import urllib.request
import json
import zipfile
import io

run_id = 27652468015

# Get artifacts
url = f"https://api.github.com/repos/meowgineeer/friction-aftereffetcsmod/actions/runs/{run_id}/artifacts"
with urllib.request.urlopen(url) as r:
    artifacts = json.loads(r.read().decode())

for a in artifacts['artifacts']:
    print(f"Artifact: {a['name']} | size: {a['size_in_bytes']} | url: {a['archive_download_url']}")
