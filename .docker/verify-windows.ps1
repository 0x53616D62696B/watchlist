$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)

$requiredPaths = @(
    'libs/SQLiteCpp/CMakeLists.txt',
    'libs/glfw/CMakeLists.txt',
    'libs/googletest/CMakeLists.txt',
    'libs/imgui/imgui.cpp',
    'libs/tracy/CMakeLists.txt',
    'libs/thread-pool/include/BS_thread_pool.hpp'
)

foreach ($requiredPath in $requiredPaths) {
    if (-not (Test-Path $requiredPath)) {
        throw "Missing submodule content: $requiredPath. Run: git submodule update --init --recursive"
    }
}

function Invoke-Checked([string]$Command, [string[]]$Arguments) {
    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Command $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
    }
}

Invoke-Checked cmake @('--workflow', '--preset', 'windows-mingw')
Invoke-Checked ctest @('--test-dir', 'build/windows-mingw', '-L', 'Unit', '--output-on-failure')
Invoke-Checked ctest @('--test-dir', 'build/windows-mingw', '-L', 'System', '--output-on-failure')

Invoke-Checked cmake @('--workflow', '--preset', 'windows-msvc')
Invoke-Checked ctest @('--test-dir', 'build/windows-msvc', '-L', 'Unit', '--output-on-failure')
Invoke-Checked ctest @('--test-dir', 'build/windows-msvc', '-L', 'System', '--output-on-failure')
