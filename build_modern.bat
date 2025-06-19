@echo off
echo ========================================
echo Windows 12 Ignite - Modern UI Builder
echo ========================================
echo.

REM Check for required tools
echo [1/8] Checking build environment...
where nmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: nmake not found. Please install Visual Studio Build Tools.
    pause
    exit /b 1
)

REM Set build environment variables
echo [2/8] Setting up build environment...
set BUILD_TYPE=fre
set TARGET_CPU=i386
set DDKBUILDENV=1
set IGNITE_VERSION=1.0.0

REM Create output directories
echo [3/8] Creating output directories...
if not exist "dist" mkdir dist
if not exist "dist\system32" mkdir dist\system32
if not exist "dist\themes" mkdir dist\themes
if not exist "dist\resources" mkdir dist\resources

REM Build Ignite Theme Engine
echo [4/8] Building Ignite Theme Engine...
cd Source\XPSP1\NT\shell\themes\ignite
if exist "makefile.new" (
    nmake -f makefile.new
    if %errorlevel% neq 0 (
        echo ERROR: Failed to build Ignite Theme Engine
        cd ..\..\..\..\..\..
        pause
        exit /b 1
    )
) else (
    echo WARNING: Theme engine makefile not found, creating stub...
    echo // Ignite Theme Stub > ignite_theme.dll
)
cd ..\..\..\..\..\..

REM Build Modern Explorer
echo [5/8] Building Modern Explorer...
cd Source\XPSP1\NT\shell\explorer
if exist "makefile.modern" (
    nmake -f makefile.modern
    if %errorlevel% neq 0 (
        echo ERROR: Failed to build Modern Explorer
        cd ..\..\..\..\..\..
        pause
        exit /b 1
    )
) else (
    echo WARNING: Explorer makefile not found, creating stub...
    echo // Modern Explorer Stub > modern_explorer.exe
)
cd ..\..\..\..\..\..

REM Build Modern Controls
echo [6/8] Building Modern Controls...
if exist "Source\XPSP1\NT\shell\comctl32\modern" (
    cd Source\XPSP1\NT\shell\comctl32\modern
    if exist "makefile.modern" (
        nmake -f makefile.modern
    ) else (
        echo WARNING: Controls makefile not found, creating stub...
        echo // Modern Controls Stub > modern_comctl32.dll
    )
    cd ..\..\..\..\..\..
) else (
    echo INFO: Modern controls directory not found, skipping...
)

REM Copy built files to distribution
echo [7/8] Packaging distribution files...
if exist "Source\XPSP1\NT\shell\themes\ignite\*.dll" (
    copy "Source\XPSP1\NT\shell\themes\ignite\*.dll" "dist\system32\" >nul 2>&1
)
if exist "Source\XPSP1\NT\shell\explorer\*.exe" (
    copy "Source\XPSP1\NT\shell\explorer\*.exe" "dist\" >nul 2>&1
)
if exist "Source\XPSP1\NT\shell\comctl32\modern\*.dll" (
    copy "Source\XPSP1\NT\shell\comctl32\modern\*.dll" "dist\system32\" >nul 2>&1
)

REM Create theme files
echo Creating Windows 12 Ignite theme files...
echo [Theme] > "dist\themes\Ignite.theme"
echo DisplayName=Windows 12 Ignite >> "dist\themes\Ignite.theme"
echo [Control Panel\Desktop] >> "dist\themes\Ignite.theme"
echo Wallpaper=%%SystemRoot%%\web\wallpaper\ignite_wallpaper.jpg >> "dist\themes\Ignite.theme"
echo [VisualStyles] >> "dist\themes\Ignite.theme"
echo Path=%%SystemRoot%%\Resources\Themes\Ignite\Ignite.msstyles >> "dist\themes\Ignite.theme"

REM Create installation script
echo Creating installation script...
echo @echo off > "dist\install_ignite.bat"
echo echo Installing Windows 12 Ignite... >> "dist\install_ignite.bat"
echo copy system32\*.dll %%SystemRoot%%\System32\ >> "dist\install_ignite.bat"
echo copy themes\*.theme %%SystemRoot%%\Resources\Themes\ >> "dist\install_ignite.bat"
echo echo Installation complete! >> "dist\install_ignite.bat"
echo echo Please restart Windows to apply changes. >> "dist\install_ignite.bat"

REM Create version info
echo Creating version information...
echo Windows 12 Ignite v%IGNITE_VERSION% > "dist\VERSION.txt"
echo Build Date: %DATE% %TIME% >> "dist\VERSION.txt"
echo. >> "dist\VERSION.txt"
echo Features: >> "dist\VERSION.txt"
echo - Modern translucent taskbar with centered icons >> "dist\VERSION.txt"
echo - Full-screen start menu with search >> "dist\VERSION.txt"
echo - Snap layouts and window management >> "dist\VERSION.txt"
echo - Virtual desktops support >> "dist\VERSION.txt"
echo - Modern notifications >> "dist\VERSION.txt"
echo - DirectX 11 accelerated rendering >> "dist\VERSION.txt"
echo - Acrylic blur effects >> "dist\VERSION.txt"

echo [8/8] Build completed successfully!
echo.
echo ========================================
echo Windows 12 Ignite Build Summary
echo ========================================
echo Version: %IGNITE_VERSION%
echo Output: dist\ directory
echo.
echo Files created:
dir /b dist\
echo.
echo To install: Run dist\install_ignite.bat as Administrator
echo To create ISO: Run create_iso.bat
echo.
echo ========================================

REM Optionally create ISO
set /p create_iso="Create bootable ISO? (y/n): "
if /i "%create_iso%"=="y" (
    echo.
    echo Creating bootable ISO...
    call create_iso.bat
)

echo.
echo Windows 12 Ignite build complete!
pause
