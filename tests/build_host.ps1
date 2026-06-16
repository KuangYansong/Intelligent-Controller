$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$sources = @(
    "common/crc16.c",
    "common/parameters.c",
    "common/timebase.c",
    "app/system.c",
    "storage/fram.c",
    "storage/param_store.c",
    "storage/fault_log.c",
    "protocol/can_protocol.c",
    "protocol/protocol_service.c",
    "protection/rms.c",
    "protection/i2t.c",
    "protection/protection.c",
    "selftest/selftest.c",
    "driver/spi_driver.c",
    "tests/test_logic.c"
)
$includes = @(".", "common", "storage", "protocol", "protection", "selftest", "driver")
$includeArgs = $includes | ForEach-Object { "-I$(Join-Path $root $_)" }
$sourceArgs = $sources | ForEach-Object { Join-Path $root $_ }
$out = Join-Path $root "tests/test_logic.exe"
$compiler = (Get-Command cc -ErrorAction SilentlyContinue)
if ($null -eq $compiler) {
    $compiler = (Get-Command gcc -ErrorAction Stop)
}
& $compiler.Source @includeArgs @sourceArgs -lm -o $out
& $out
