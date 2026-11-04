# Starts the game in RPCS3 with an Xbox pad as Player 1, for playing it by hand.
# The profile applies to this launch only: RPCS3's own input settings stay as they are.
$root = Split-Path -Parent $PSScriptRoot
$rpcs3Dir = 'C:\Users\claud\Desktop\rpcs3'
$elf = Join-Path $root 'build\spacecadet.elf'
$data = Join-Path $rpcs3Dir 'dev_hdd0\game\PNBL00001\USRDIR\Data'

if (-not (Test-Path $elf)) { Write-Output "missing ELF, run tools/build.ps1 first: $elf"; exit 2 }
if (Get-Process rpcs3 -ErrorAction SilentlyContinue) { Write-Output 'rpcs3 is already running'; exit 3 }

# A test run leaves these behind, and a scripted press would fight the real pad.
Remove-Item (Join-Path $data 'pad_script.txt'), (Join-Path $data 'dump_frames.txt') -ErrorAction SilentlyContinue

$profiles = Join-Path $rpcs3Dir 'config\input_configs\global'
New-Item -ItemType Directory -Force $profiles | Out-Null
Copy-Item (Join-Path $PSScriptRoot 'rpcs3_xinput_input.yml') (Join-Path $profiles 'spacecadet_xinput.yml') -Force

Start-Process -FilePath (Join-Path $rpcs3Dir 'rpcs3.exe') -ArgumentList '--no-gui', '--input-config', 'spacecadet_xinput', "`"$elf`""
Write-Output 'started'
