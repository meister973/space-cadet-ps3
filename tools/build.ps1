# Builds the game in Docker on Windows. Output: build/spacecadet.elf (boot this one).
#   -Clean   rebuild from scratch
#   -Pkg     also make spacecadet.pkg
# Without Docker, use WSL or Linux instead: sh tools/install_toolchain.sh, then sh tools/make.sh
param([switch]$Clean, [switch]$Pkg)

$root = Split-Path -Parent $PSScriptRoot
$image = 'spacecadet-ps3dev:1'

docker image inspect $image *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Output "building the toolchain image $image (one time, a few minutes)"
    Copy-Item (Join-Path $root 'tools\install_toolchain.sh') (Join-Path $root 'docker\install_toolchain.sh')
    docker build -t $image (Join-Path $root 'docker')
    $status = $LASTEXITCODE
    Remove-Item (Join-Path $root 'docker\install_toolchain.sh')
    if ($status -ne 0) { Write-Output 'toolchain image build failed'; exit 1 }
}

$command = 'make'
if ($Clean) { $command = 'make clean >/dev/null; make' }
if ($Pkg) { $command += ' && make pkg' }

docker run --rm -v "${root}:/work" -w /work $image sh -c "$command 2>&1"
if ($LASTEXITCODE -ne 0 -or -not (Test-Path (Join-Path $root 'build\spacecadet.elf'))) {
    Write-Output 'build failed'
    exit 1
}
Write-Output 'build ok'
exit 0
