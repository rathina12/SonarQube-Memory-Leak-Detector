$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
cmake -S "$Root/analyzer" -B "$Root/build/analyzer" -DCMAKE_BUILD_TYPE=Release
cmake --build "$Root/build/analyzer" --config Release
ctest --test-dir "$Root/build/analyzer" -C Release --output-on-failure
if (Get-Command mvn -ErrorAction SilentlyContinue) {
  Push-Location "$Root/sonar-plugin"
  try { mvn clean package } finally { Pop-Location }
} else {
  Write-Warning "Maven not found; analyzer built/tested, Sonar plugin build skipped."
}
