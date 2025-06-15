@echo off
echo Creating Windows 12 Ignite ISO...

REM Create ISO structure
mkdir iso_temp
mkdir iso_temp\i386
mkdir iso_temp\support
mkdir iso_temp\valueadd

REM Copy Windows XP base files (would normally copy from existing XP installation)
echo Copying base system files...
REM xcopy /s /e C:\WINDOWS\system32\*.* iso_temp\i386\

REM Copy our modern UI files
echo Copying modern UI components...
copy dist\*.dll iso_temp\i386\
copy dist\*.exe iso_temp\i386\

REM Copy resources
echo Copying modern resources...
xcopy /s /e Source\XPSP1\NT\shell\resources\ignite\*.* iso_temp\support\

REM Create boot files
echo Creating boot configuration...
echo [Version] > iso_temp\txtsetup.sif
echo Signature="$Windows NT$" >> iso_temp\txtsetup.sif
echo [SourceDisksNames] >> iso_temp\txtsetup.sif
echo 1="Windows 12 Ignite Setup",\i386 >> iso_temp\txtsetup.sif

REM Create autorun
echo [autorun] > iso_temp\autorun.inf
echo open=setup.exe >> iso_temp\autorun.inf
echo icon=setup.exe,0 >> iso_temp\autorun.inf
echo label=Windows 12 Ignite >> iso_temp\autorun.inf

REM Create ISO using oscdimg (Windows SDK tool)
echo Building ISO image...
oscdimg -n -m -b iso_temp\boot\etfsboot.com iso_temp Windows12_Ignite_x86.iso

if exist Windows12_Ignite_x86.iso (
    echo ISO created successfully: Windows12_Ignite_x86.iso
    echo Size: 
    dir Windows12_Ignite_x86.iso | find ".iso"
) else (
    echo Error creating ISO
)

REM Cleanup
rmdir /s /q iso_temp

echo ISO creation complete!

