@echo off
echo ========================================
echo Windows 12 Ignite ISO Creator
echo ========================================
echo.

REM Check for required tools
echo [1/6] Checking ISO creation tools...
where oscdimg >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: oscdimg not found. Trying alternative methods...
    where mkisofs >nul 2>&1
    if %errorlevel% neq 0 (
        echo ERROR: No ISO creation tools found.
        echo Please install Windows ADK (oscdimg) or cdrtools (mkisofs).
        pause
        exit /b 1
    ) else (
        set ISO_TOOL=mkisofs
        echo Using mkisofs for ISO creation.
    )
) else (
    set ISO_TOOL=oscdimg
    echo Using oscdimg for ISO creation.
)

REM Create ISO workspace
echo [2/6] Creating ISO workspace...
if exist "iso_temp" rmdir /s /q iso_temp
mkdir iso_temp
mkdir iso_temp\I386
mkdir iso_temp\SUPPORT
mkdir iso_temp\VALUEADD
mkdir iso_temp\IGNITE
mkdir iso_temp\DOCS

REM Copy base Windows XP files (if available)
echo [3/6] Preparing base system files...
if exist "C:\XP_SOURCE" (
    echo Copying Windows XP base files...
    xcopy /s /e /q "C:\XP_SOURCE\*" iso_temp\ >nul 2>&1
) else (
    echo Creating minimal boot structure...
    echo [Boot Loader] > iso_temp\boot.ini
    echo timeout=30 >> iso_temp\boot.ini
    echo default=multi(0)disk(0)rdisk(0)partition(1)\WINDOWS >> iso_temp\boot.ini
    echo [operating systems] >> iso_temp\boot.ini
    echo multi(0)disk(0)rdisk(0)partition(1)\WINDOWS="Windows 12 Ignite" /fastdetect >> iso_temp\boot.ini
    
    REM Create minimal I386 structure
    echo Windows 12 Ignite Setup > iso_temp\I386\TXTSETUP.SIF
    echo Minimal setup configuration >> iso_temp\I386\TXTSETUP.SIF
)

REM Copy Ignite distribution files
echo [4/6] Adding Windows 12 Ignite components...
if exist "dist" (
    xcopy /s /e /q "dist\*" iso_temp\IGNITE\ >nul 2>&1
    echo Ignite files copied to ISO.
) else (
    echo WARNING: dist directory not found. Run build_modern.bat first.
)

REM Create autorun and documentation
echo [5/6] Creating autorun and documentation...
echo [autorun] > iso_temp\autorun.inf
echo open=IGNITE\install_ignite.bat >> iso_temp\autorun.inf
echo icon=IGNITE\ignite.ico >> iso_temp\autorun.inf
echo label=Windows 12 Ignite >> iso_temp\autorun.inf

echo Windows 12 Ignite - Modern UI for Windows XP > iso_temp\README.txt
echo. >> iso_temp\README.txt
echo This ISO contains the Windows 12 Ignite modernization package. >> iso_temp\README.txt
echo. >> iso_temp\README.txt
echo Features: >> iso_temp\README.txt
echo - Modern translucent taskbar with centered icons >> iso_temp\README.txt
echo - Full-screen start menu with search functionality >> iso_temp\README.txt
echo - Snap layouts for advanced window management >> iso_temp\README.txt
echo - Virtual desktops support >> iso_temp\README.txt
echo - Modern toast notifications >> iso_temp\README.txt
echo - DirectX 11 accelerated rendering >> iso_temp\README.txt
echo - Acrylic blur effects throughout the interface >> iso_temp\README.txt
echo. >> iso_temp\README.txt
echo Installation: >> iso_temp\README.txt
echo 1. Run IGNITE\install_ignite.bat as Administrator >> iso_temp\README.txt
echo 2. Restart your computer >> iso_temp\README.txt
echo 3. Enjoy the modern Windows 12 experience! >> iso_temp\README.txt

REM Create installation guide
echo Creating Installation Guide... > iso_temp\DOCS\INSTALL.txt
echo. >> iso_temp\DOCS\INSTALL.txt
echo WINDOWS 12 IGNITE INSTALLATION GUIDE >> iso_temp\DOCS\INSTALL.txt
echo ===================================== >> iso_temp\DOCS\INSTALL.txt
echo. >> iso_temp\DOCS\INSTALL.txt
echo Prerequisites: >> iso_temp\DOCS\INSTALL.txt
echo - Windows XP SP3 or later >> iso_temp\DOCS\INSTALL.txt
echo - DirectX 11 compatible graphics card >> iso_temp\DOCS\INSTALL.txt
echo - At least 512MB RAM >> iso_temp\DOCS\INSTALL.txt
echo - 100MB free disk space >> iso_temp\DOCS\INSTALL.txt
echo. >> iso_temp\DOCS\INSTALL.txt
echo Installation Steps: >> iso_temp\DOCS\INSTALL.txt
echo 1. Insert the Windows 12 Ignite ISO >> iso_temp\DOCS\INSTALL.txt
echo 2. Navigate to the IGNITE folder >> iso_temp\DOCS\INSTALL.txt
echo 3. Right-click install_ignite.bat and "Run as Administrator" >> iso_temp\DOCS\INSTALL.txt
echo 4. Follow the on-screen prompts >> iso_temp\DOCS\INSTALL.txt
echo 5. Restart your computer when prompted >> iso_temp\DOCS\INSTALL.txt
echo. >> iso_temp\DOCS\INSTALL.txt
echo Troubleshooting: >> iso_temp\DOCS\INSTALL.txt
echo - If installation fails, check Windows Event Viewer >> iso_temp\DOCS\INSTALL.txt
echo - Ensure all antivirus software is temporarily disabled >> iso_temp\DOCS\INSTALL.txt
echo - Run Windows Update before installation >> iso_temp\DOCS\INSTALL.txt

REM Create the ISO
echo [6/6] Creating bootable ISO image...
set ISO_NAME=Windows12_Ignite_v1.0.0_%DATE:~-4%%DATE:~4,2%%DATE:~7,2%.iso

if "%ISO_TOOL%"=="oscdimg" (
    if exist "iso_temp\boot\etfsboot.com" (
        oscdimg -n -b"iso_temp\boot\etfsboot.com" -l"WIN12_IGNITE" iso_temp "%ISO_NAME%"
    ) else (
        oscdimg -n -l"WIN12_IGNITE" iso_temp "%ISO_NAME%"
    )
) else (
    mkisofs -o "%ISO_NAME%" -V "WIN12_IGNITE" -J -R -l iso_temp
)

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo ISO Creation Successful!
    echo ========================================
    echo File: %ISO_NAME%
    echo Size: 
    dir "%ISO_NAME%" | find "%ISO_NAME%"
    echo.
    echo The ISO is ready for distribution or burning to CD/DVD.
    echo You can also mount it in a virtual machine for testing.
    echo.
) else (
    echo.
    echo ERROR: ISO creation failed!
    echo Check the error messages above for details.
    echo.
)

REM Cleanup
echo Cleaning up temporary files...
if exist "iso_temp" rmdir /s /q iso_temp

echo.
echo ISO creation process complete!
pause
