@echo off
setlocal enabledelayedexpansion

:: ============================================================================
:: Setup zlib via vcpkg for Qt6 with CMake
:: ============================================================================

echo ============================================
echo Setting up zlib via vcpkg for Qt6/CMake
echo ============================================

:: Configuration - VCPKG_ROOT and VCPKG_TRIPLET can be set externally
:: (MSVC builds use x64-windows, MinGW builds use x64-mingw-static)
if not defined VCPKG_ROOT set "VCPKG_ROOT=C:\vcpkg"
if not defined VCPKG_TRIPLET set VCPKG_TRIPLET=x64-windows

:: Check if vcpkg is already installed
if exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg already installed at %VCPKG_ROOT%
    goto :install_zlib
)

:: ============================================================================
:: Install vcpkg
:: ============================================================================
echo.
echo Installing vcpkg...

:: Check if git is available
where git >nul 2>&1
if ERRORLEVEL 1 (
    echo ERROR: Git is not installed or not in PATH
    echo Please install Git from https://git-scm.com/
    exit /b 1
)

:: Clone vcpkg repository
if not exist "%VCPKG_ROOT%" (
    echo Cloning vcpkg repository to %VCPKG_ROOT%...
    git clone https://github.com/microsoft/vcpkg.git "%VCPKG_ROOT%"
    if ERRORLEVEL 1 (
        echo ERROR: Failed to clone vcpkg repository
        exit /b 1
    )
)

:: Bootstrap vcpkg
echo Bootstrapping vcpkg...
pushd "%VCPKG_ROOT%"
call bootstrap-vcpkg.bat
if ERRORLEVEL 1 (
    echo ERROR: Failed to bootstrap vcpkg
    popd
    exit /b 1
)
popd

echo vcpkg installed successfully!

:: ============================================================================
:: Install zlib
:: ============================================================================
:install_zlib
echo.
echo Installing zlib via vcpkg...

"%VCPKG_ROOT%\vcpkg.exe" install zlib:%VCPKG_TRIPLET%
if ERRORLEVEL 1 (
    echo ERROR: Failed to install zlib
    exit /b 1
)

echo.
echo zlib installed successfully!

:: ============================================================================
:: Display CMake integration info
:: ============================================================================
echo.
echo ============================================
echo Setup Complete!
echo ============================================
echo.
echo To use zlib with CMake, add one of the following to your CMake command:
echo.
echo Option 1: Use vcpkg toolchain file:
echo   cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ...
echo.
echo Option 2: Set CMAKE_PREFIX_PATH:
echo   set CMAKE_PREFIX_PATH=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%
echo.
echo For Qt6 projects, you can combine with Qt paths:
echo   set CMAKE_PREFIX_PATH=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%;C:\Qt\6.9.3\msvc2022_64
echo.
echo In your CMakeLists.txt, use:
echo   find_package(ZLIB REQUIRED)
echo   target_link_libraries(your_target PRIVATE ZLIB::ZLIB)
echo.

:: Set environment variable for current session
set CMAKE_PREFIX_PATH=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%;%CMAKE_PREFIX_PATH%
echo CMAKE_PREFIX_PATH set for current session.

echo.
echo Done!
exit /b 0
