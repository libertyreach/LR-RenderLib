#!/usr/bin/env pwsh
# Configure + build renderlib (and the demo) into build/. Always Release.

$ErrorActionPreference = "Stop"

# Ninja (not the default VS generator) so build/compile_commands.json gets
# emitted for clangd. clang keeps the editor's flags matching the build.
if (-not $env:CC)  { $env:CC  = "clang" }
if (-not $env:CXX) { $env:CXX = "clang++" }

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

Write-Host "`nDemo binary: build/bin/renderlib_demo" -ForegroundColor Green
