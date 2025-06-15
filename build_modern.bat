@echo off
echo Building Windows 12 Ignite Modern UI...

REM Set build environment
set BUILD_TYPE=fre
set TARGET_CPU=i386
set DDKBUILDENV=1

REM Build modern theme components
echo Building Ignite Theme...
cd Source\XPSP1\NT\shell\themes\ignite
nmake -f makefile.new

REM Build modern explorer
echo Building Modern Explorer...
cd ..\..\..\explorer
nmake -f makefile.modern

REM Build modern controls
echo Building Modern Controls...
cd ..\comctl32\modern
nmake -f makefile.modern

REM Build modern features
echo Building Modern Features...
cd ..\..\modern_features
nmake -f makefile.modern

REM Build modern window manager
echo Building Modern Window Manager...
cd ..\..\..\windows\core\modern_wm
nmake -f makefile.modern

REM Create installation files
echo Creating installation package...
cd ..\..\..\..\..\..
mkdir dist
copy Source\XPSP1\NT\shell\themes\ignite\*.dll dist\
copy Source\XPSP1\NT\shell\explorer\*.exe dist\
copy Source\XPSP1\NT\shell\comctl32\modern\*.dll dist\

echo Build completed successfully!
echo Files are in the 'dist' directory.

REM Create ISO
echo Creating bootable ISO...
call create_iso.bat

echo Windows 12 Ignite build complete!

