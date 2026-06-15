import urllib.request
import json
import zipfile
import io

try:
    url = "https://api.github.com/repos/meowgineeer/friction-aftereffetcsmod/actions/runs?per_page=1"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req) as response:
        data = json.loads(response.read().decode())
        run_id = data['workflow_runs'][0]['id']
        
    jobs_url = data['workflow_runs'][0]['jobs_url']
    req = urllib.request.Request(jobs_url)
    with urllib.request.urlopen(req) as response:
        jobs_data = json.loads(response.read().decode())
        
    for job in jobs_data['jobs']:
        if 'Windows' in job['name'] or 'windows' in job['name'].lower():
            print(f"Found Windows job: {job['id']}")
            log_url = f"https://api.github.com/repos/meowgineeer/friction-aftereffetcsmod/actions/jobs/{job['id']}/logs"
            req = urllib.request.Request(log_url)
            try:
                with urllib.request.urlopen(req) as log_res:
                    log_content = log_res.read().decode('utf-8', errors='replace')
                    lines = log_content.split('\n')
                    for i, line in enumerate(lines):
                        if 'error C' in line or 'fatal error' in line or 'CMake Error' in line or 'LNK' in line:
                            start = max(0, i - 2)
                            end = min(len(lines), i + 8)
                            print('\n'.join(lines[start:end]))
                            break
            except Exception as e:
                print(f"Failed to fetch logs for job {job['id']}: {e}")
except Exception as e:
    print(f"Error: {e}")
