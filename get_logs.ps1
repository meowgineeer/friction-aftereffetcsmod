$run = (Invoke-RestMethod -Uri 'https://api.github.com/repos/meowgineeer/friction-aftereffetcsmod/actions/runs?per_page=1').workflow_runs[0]
$jobs = Invoke-RestMethod -Uri $run.jobs_url
foreach ($job in $jobs.jobs) {
    if ($job.name -match 'Windows') {
        Write-Host 'Job ID:' $job.id
        Invoke-RestMethod -Uri ('https://api.github.com/repos/meowgineeer/friction-aftereffetcsmod/actions/jobs/' + $job.id + '/logs') -OutFile logs.txt
        Select-String -Path logs.txt -Pattern 'error|fatal' -Context 2,10
    }
}
