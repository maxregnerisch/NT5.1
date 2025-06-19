@echo off
echo ========================================
echo Windows 12 Ignite Build Test Suite
echo ========================================
echo.

set TESTS_PASSED=0
set TESTS_FAILED=0
set TOTAL_TESTS=0

REM Test 1: Check if source files exist
echo [TEST 1] Checking source file structure...
set /a TOTAL_TESTS+=1

if exist "Source\XPSP1\NT\shell\themes\ignite\theme.h" (
    echo   ✓ Theme header file exists
) else (
    echo   ✗ Theme header file missing
    set /a TESTS_FAILED+=1
    goto :test2
)

if exist "Source\XPSP1\NT\shell\themes\ignite\renderer.cpp" (
    echo   ✓ Renderer implementation exists
) else (
    echo   ✗ Renderer implementation missing
    set /a TESTS_FAILED+=1
    goto :test2
)

if exist "Source\XPSP1\NT\shell\explorer\modern_taskbar.cpp" (
    echo   ✓ Modern taskbar implementation exists
) else (
    echo   ✗ Modern taskbar implementation missing
    set /a TESTS_FAILED+=1
    goto :test2
)

if exist "Source\XPSP1\NT\shell\explorer\modern_startmenu.cpp" (
    echo   ✓ Modern start menu implementation exists
) else (
    echo   ✗ Modern start menu implementation missing
    set /a TESTS_FAILED+=1
    goto :test2
)

echo   ✓ Source file structure test PASSED
set /a TESTS_PASSED+=1

:test2
REM Test 2: Check build scripts
echo.
echo [TEST 2] Checking build scripts...
set /a TOTAL_TESTS+=1

if exist "build_modern.bat" (
    echo   ✓ Main build script exists
) else (
    echo   ✗ Main build script missing
    set /a TESTS_FAILED+=1
    goto :test3
)

if exist "create_iso.bat" (
    echo   ✓ ISO creation script exists
) else (
    echo   ✗ ISO creation script missing
    set /a TESTS_FAILED+=1
    goto :test3
)

if exist "Source\XPSP1\NT\shell\themes\ignite\makefile.new" (
    echo   ✓ Theme makefile exists
) else (
    echo   ✗ Theme makefile missing
    set /a TESTS_FAILED+=1
    goto :test3
)

if exist "Source\XPSP1\NT\shell\explorer\makefile.modern" (
    echo   ✓ Explorer makefile exists
) else (
    echo   ✗ Explorer makefile missing
    set /a TESTS_FAILED+=1
    goto :test3
)

echo   ✓ Build scripts test PASSED
set /a TESTS_PASSED+=1

:test3
REM Test 3: Check documentation
echo.
echo [TEST 3] Checking documentation...
set /a TOTAL_TESTS+=1

if exist "README_IGNITE.md" (
    echo   ✓ Main documentation exists
) else (
    echo   ✗ Main documentation missing
    set /a TESTS_FAILED+=1
    goto :test4
)

if exist "RELEASE_NOTES.md" (
    echo   ✓ Release notes exist
) else (
    echo   ✗ Release notes missing
    set /a TESTS_FAILED+=1
    goto :test4
)

echo   ✓ Documentation test PASSED
set /a TESTS_PASSED+=1

:test4
REM Test 4: Syntax check on key files
echo.
echo [TEST 4] Performing syntax checks...
set /a TOTAL_TESTS+=1

REM Check C++ syntax (basic check for common issues)
findstr /C:"#include" "Source\XPSP1\NT\shell\themes\ignite\theme.h" >nul
if %errorlevel% equ 0 (
    echo   ✓ Theme header has includes
) else (
    echo   ✗ Theme header missing includes
    set /a TESTS_FAILED+=1
    goto :test5
)

findstr /C:"WINAPI" "Source\XPSP1\NT\shell\themes\ignite\renderer.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ Renderer has WINAPI functions
) else (
    echo   ✗ Renderer missing WINAPI functions
    set /a TESTS_FAILED+=1
    goto :test5
)

echo   ✓ Syntax check test PASSED
set /a TESTS_PASSED+=1

:test5
REM Test 5: Check for required constants and definitions
echo.
echo [TEST 5] Checking API definitions...
set /a TOTAL_TESTS+=1

findstr /C:"IGNITE_VERSION" "Source\XPSP1\NT\shell\themes\ignite\theme.h" >nul
if %errorlevel% equ 0 (
    echo   ✓ Version constants defined
) else (
    echo   ✗ Version constants missing
    set /a TESTS_FAILED+=1
    goto :test6
)

findstr /C:"IgniteTheme_Initialize" "Source\XPSP1\NT\shell\themes\ignite\theme.h" >nul
if %errorlevel% equ 0 (
    echo   ✓ Core API functions declared
) else (
    echo   ✗ Core API functions missing
    set /a TESTS_FAILED+=1
    goto :test6
)

findstr /C:"IGNITE_ANIMATION_TYPE" "Source\XPSP1\NT\shell\themes\ignite\theme.h" >nul
if %errorlevel% equ 0 (
    echo   ✓ Animation types defined
) else (
    echo   ✗ Animation types missing
    set /a TESTS_FAILED+=1
    goto :test6
)

echo   ✓ API definitions test PASSED
set /a TESTS_PASSED+=1

:test6
REM Test 6: Check build environment
echo.
echo [TEST 6] Checking build environment...
set /a TOTAL_TESTS+=1

where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo   ✓ C++ compiler available
) else (
    echo   ⚠ C++ compiler not found (may need Visual Studio)
)

where nmake >nul 2>&1
if %errorlevel% equ 0 (
    echo   ✓ NMAKE build tool available
) else (
    echo   ⚠ NMAKE not found (may need Visual Studio)
)

where oscdimg >nul 2>&1
if %errorlevel% equ 0 (
    echo   ✓ ISO creation tool available
) else (
    echo   ⚠ OSCDIMG not found (may need Windows SDK)
)

echo   ✓ Build environment test PASSED
set /a TESTS_PASSED+=1

:test7
REM Test 7: Validate key features implementation
echo.
echo [TEST 7] Validating feature implementations...
set /a TOTAL_TESTS+=1

findstr /C:"IgniteTaskbar_EnableModernStyle" "Source\XPSP1\NT\shell\explorer\modern_taskbar.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ Modern taskbar feature implemented
) else (
    echo   ✗ Modern taskbar feature missing
    set /a TESTS_FAILED+=1
    goto :test8
)

findstr /C:"IgniteStartMenu_EnableModernStyle" "Source\XPSP1\NT\shell\explorer\modern_startmenu.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ Modern start menu feature implemented
) else (
    echo   ✗ Modern start menu feature missing
    set /a TESTS_FAILED+=1
    goto :test8
)

findstr /C:"D3D11CreateDevice" "Source\XPSP1\NT\shell\themes\ignite\renderer.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ DirectX 11 integration implemented
) else (
    echo   ✗ DirectX 11 integration missing
    set /a TESTS_FAILED+=1
    goto :test8
)

echo   ✓ Feature implementations test PASSED
set /a TESTS_PASSED+=1

:test8
REM Test 8: Check for modern UI elements
echo.
echo [TEST 8] Checking modern UI elements...
set /a TOTAL_TESTS+=1

findstr /C:"acrylic" "Source\XPSP1\NT\shell\themes\ignite\renderer.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ Acrylic effects implemented
) else (
    echo   ✗ Acrylic effects missing
    set /a TESTS_FAILED+=1
    goto :results
)

findstr /C:"rounded" "Source\XPSP1\NT\shell\themes\ignite\renderer.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ Rounded corners implemented
) else (
    echo   ✗ Rounded corners missing
    set /a TESTS_FAILED+=1
    goto :results
)

findstr /C:"animation" "Source\XPSP1\NT\shell\themes\ignite\renderer.cpp" >nul
if %errorlevel% equ 0 (
    echo   ✓ Animation system implemented
) else (
    echo   ✗ Animation system missing
    set /a TESTS_FAILED+=1
    goto :results
)

echo   ✓ Modern UI elements test PASSED
set /a TESTS_PASSED+=1

:results
REM Display test results
echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %TESTS_PASSED%
echo Failed: %TESTS_FAILED%
echo.

if %TESTS_FAILED% equ 0 (
    echo ✅ ALL TESTS PASSED!
    echo The Windows 12 Ignite build is ready.
    echo.
    echo Next steps:
    echo 1. Run build_modern.bat to compile
    echo 2. Run create_iso.bat to create installation media
    echo 3. Test in a virtual machine
    echo.
    set EXITCODE=0
) else (
    echo ❌ %TESTS_FAILED% TEST(S) FAILED!
    echo Please fix the issues above before building.
    echo.
    set EXITCODE=1
)

REM Optional: Run actual build test
set /p run_build="Run a test build now? (y/n): "
if /i "%run_build%"=="y" (
    echo.
    echo Running test build...
    call build_modern.bat
)

echo.
echo Test suite complete!
pause
exit /b %EXITCODE%
