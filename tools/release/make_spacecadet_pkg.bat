@echo off
rem Makes SpaceCadet.pkg from your own 3D Pinball files.
rem Put this file and the spacecadet_pkg_files folder in your Pinball folder, next to PINBALL.DAT,
rem then double-click this file. SpaceCadet.pkg appears in the same folder.
setlocal
set "HERE=%~dp0"
set "FILES=%HERE%spacecadet_pkg_files"
set "CONTENT_ID=UP0001-PNBL00001_00-0000000000000000"

if not exist "%FILES%\pkg.exe" (
    echo The spacecadet_pkg_files folder is missing.
    echo Put it in this folder together with this file.
    pause
    exit /b 1
)
if not exist "%HERE%pinball.dat" if not exist "%HERE%cadet.dat" (
    echo PINBALL.DAT is not in this folder.
    echo Put this file in your Pinball folder, next to PINBALL.DAT and the SOUND files.
    pause
    exit /b 2
)
rem 3D Pinball keeps its sounds next to PINBALL.DAT, Full Tilt! in a SOUND sub-folder.
if not exist "%HERE%sound*.wav" if not exist "%HERE%SOUND\*.wav" (
    echo The sound files are not in this folder.
    echo Copy the SOUND*.WAV files from your Pinball folder here, next to PINBALL.DAT.
    pause
    exit /b 5
)

set "WORK=%TEMP%\spacecadet_pkg_%RANDOM%%RANDOM%"
mkdir "%WORK%\stage\USRDIR\Data"
copy /y "%FILES%\EBOOT.BIN" "%WORK%\stage\USRDIR\" >nul
copy /y "%FILES%\PARAM.SFO" "%WORK%\stage\" >nul
copy /y "%FILES%\ICON0.PNG" "%WORK%\stage\" >nul

rem Only what the game reads. SOUND is the Full Tilt! sub-folder.
echo Copying your game files...
robocopy "%HERE%." "%WORK%\stage\USRDIR\Data" *.dat *.wav *.mid *.ogg pinball.exe cadet.exe ^
    /s /njh /njs /ndl /nfl /nc /ns /np /xd "%FILES%" >nul
if errorlevel 8 (
    echo Copying the game files failed.
    rmdir /s /q "%WORK%"
    pause
    exit /b 3
)
rem A read-only file in the package cannot be replaced when the package is installed again.
attrib -r "%WORK%\stage\*" /s >nul

echo Making SpaceCadet.pkg...
pushd "%WORK%"
"%FILES%\pkg.exe" --contentid %CONTENT_ID% stage/ SpaceCadet.pkg >nul
set "STATUS=%errorlevel%"
popd
if not "%STATUS%"=="0" (
    echo Making the package failed.
    rmdir /s /q "%WORK%"
    pause
    exit /b 4
)

move /y "%WORK%\SpaceCadet.pkg" "%HERE%SpaceCadet.pkg" >nul
rmdir /s /q "%WORK%"
echo.
echo Done: %HERE%SpaceCadet.pkg
echo Install it on your PS3 with Package Manager. It contains your game files: do not share it.
pause
