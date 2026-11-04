# Builds the game in the PS3 toolchain container. Output: build/spacecadet.elf (boot this one).
#   -Clean   rebuild from scratch
param([switch]$Clean)

$root = Split-Path -Parent $PSScriptRoot
# The Balatro port's image: ps3dev nightly plus cgcomp's libCg. See D:\Ps3 Port\docker\Dockerfile.
$image = 'balatro-ps3dev:2'
$command = if ($Clean) { 'make clean >/dev/null; make' } else { 'make' }

docker run --rm -v "${root}:/work" -w /work $image sh -c "$command 2>&1"
if ($LASTEXITCODE -ne 0 -or -not (Test-Path (Join-Path $root 'build\spacecadet.elf'))) {
    Write-Output 'build failed'
    exit 1
}
Write-Output 'build ok'
exit 0
