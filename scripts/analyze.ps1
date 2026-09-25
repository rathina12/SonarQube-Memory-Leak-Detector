param(
  [Parameter(Mandatory=$true)][string]$ProjectPath,
  [string]$Output = "mlpca-report.json",
  [string]$Config = ""
)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Exe = "$Root/build/analyzer/Release/mlpca-analyzer.exe"
if (-not (Test-Path $Exe)) { $Exe = "$Root/build/analyzer/mlpca-analyzer.exe" }
if (-not (Test-Path $Exe)) { throw "Analyzer not built. Run scripts/build.ps1 first." }
$args = @($ProjectPath, "--output", $Output)
if ($Config) { $args += @("--config", $Config) }
& $Exe @args
