# Builds the host test image once, then runs the unit tests in tests/.
$root = Split-Path -Parent $PSScriptRoot
$image = 'spacecadet-tests:1'

docker image inspect $image *> $null
if ($LASTEXITCODE -ne 0) {
    docker build -q -t $image (Join-Path $root 'tests') | Out-Null
    if ($LASTEXITCODE -ne 0) { Write-Output 'test image build failed'; exit 1 }
}

docker run --rm -v "${root}:/work" -w /work/tests $image sh -c 'make -s clean && make -s'
exit $LASTEXITCODE
