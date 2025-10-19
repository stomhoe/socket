# Exit on error
$ErrorActionPreference = "Stop"

# Set build directory
$BUILD_DIR = "build"

# Set executable name (match your project name)
$exeName = "reto-1"

# Determine OS
$isWindowsOS = $PSVersionTable.OS -match "Windows"

# Try to find Visual Studio or use default generator
function Test-VSTools {
    # Check for various VS installations
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstall = & $vsWhere -latest -property installationPath
        if ($vsInstall) {
            $vcvars = "$vsInstall\VC\Auxiliary\Build\vcvars64.bat"
            return (Test-Path $vcvars)
        }
    }
    
    # Fallback: check if cl.exe is available
    try {
        & cl.exe /? > $null 2>&1
        return $true
    } catch {
        return $false
    }
}

# Choose CMake generator - use Visual Studio if available, otherwise default
if ($isWindowsOS) {
    if (Test-VSTools) {
        Write-Host "Visual Studio tools detected"
        $generator = "Visual Studio 17 2022"
    } else {
        Write-Host "No Visual Studio tools found, using default generator"
        $generator = $null  # Let CMake choose the default
    }
} else {
    $generator = "Unix Makefiles"
}

if ($generator) {
    Write-Host "Using CMake generator: $generator"
} else {
    Write-Host "Using default CMake generator"
}

# Get absolute paths
$sourceDir = Get-Location
$buildPath = Join-Path $sourceDir $BUILD_DIR

# Create build directory if it doesn't exist
if (-not (Test-Path $buildPath)) {
    New-Item -ItemType Directory -Path $buildPath | Out-Null
}

# Configure the project (stay in current directory)
if ($generator) {
    cmake -G "$generator" -B $buildPath -S $sourceDir
} else {
    cmake -B $buildPath -S $sourceDir
}

# Build the project
cmake --build $buildPath --config Release

# Find the executable - Visual Studio creates different folder structures
if ($isWindowsOS) {
    # Check common locations for the executable (using absolute paths)
    $possiblePaths = @(
        (Join-Path $buildPath "$exeName.exe"),           # Direct build (Makefiles, Ninja)
        (Join-Path $buildPath "Release\$exeName.exe"),   # Visual Studio Release
        (Join-Path $buildPath "Debug\$exeName.exe")      # Visual Studio Debug
    )
    
    $exePath = $null
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $exePath = $path
            break
        }
    }
} else {
    $exePath = (Join-Path $buildPath $exeName)
}

if ($exePath -and (Test-Path $exePath)) {
    Write-Host "Running $exeName from: $exePath"
    & $exePath
} else {
    Write-Warning "Executable '$exeName' not found in any of the expected locations."
    Write-Host "Checked paths:"
    if ($isWindowsOS) {
        $possiblePaths | ForEach-Object { Write-Host "  $_" }
    } else {
        Write-Host "  ./$exeName"
    }
}
