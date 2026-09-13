@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo.
echo   SILT COUNTY
echo   Chief and Gooch. Two trucks. One muddy slice.
echo.

set "UEEDITOR="
set "UEBUILD="

for %%D in (
	"%ProgramFiles%\Epic Games\UE_5.8"
	"%ProgramFiles%\Epic Games\UE_5.8.2"
	"%ProgramFiles%\Epic Games\UE_5.7"
	"%ProgramFiles(x86)%\Epic Games\UE_5.8"
	"D:\Epic Games\UE_5.8"
	"E:\Epic Games\UE_5.8"
	"C:\UE_5.8"
	"D:\UE_5.8"
) do (
	if exist "%%~D\Engine\Binaries\Win64\UnrealEditor.exe" (
		set "UEEDITOR=%%~D\Engine\Binaries\Win64\UnrealEditor.exe"
		set "UEBUILD=%%~D\Engine\Build\BatchFiles\Build.bat"
		set "UEUBT=%%~D\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"
	)
)

if not defined UEEDITOR (
	echo Unreal Engine 5.8 was not found.
	echo.
	echo 1. Install the Epic Games Launcher
	echo 2. In the Launcher, open Unreal Engine ^> Library ^> Add Versions
	echo 3. Install 5.8 ^(latest hotfix is fine^)
	echo 4. Install Visual Studio 2022 with "Game development with C++"
	echo 5. Run PLAY.bat again
	echo.
	pause
	exit /b 1
)

echo Using: %UEEDITOR%
echo.

if not exist "SiltCounty.sln" (
	echo Generating Visual Studio project files...
	if exist "%UEUBT%" (
		"%UEUBT%" -projectfiles -project="%~dp0SiltCounty.uproject" -game -engine -progress
	) else (
		"%UEEDITOR%" "SiltCounty.uproject" -projectfiles
	)
)

echo Compiling SILT COUNTY ^(first time can take a while^)...
call "%UEBUILD%" SiltCountyEditor Win64 Development -Project="%~dp0SiltCounty.uproject" -WaitMutex
if errorlevel 1 (
	echo.
	echo Compile failed. Open SiltCounty.uproject in Unreal and let it build from there.
	pause
	exit /b 1
)

echo Preparing wet materials and the county slice...
"%UEEDITOR%" "SiltCounty.uproject" -run=SiltPrep -unattended -nopause -nosplash
if errorlevel 1 (
	echo Prep commandlet reported an error. Opening the editor anyway — it will try again on startup.
)

echo.
echo Launching. Wait for the garage. Gooch talks. Then you drive.
echo Player 1 Chief  =  WASD
echo Player 2 Gooch  =  IJKL  or  second gamepad
echo.
if exist "%~dp0Content\Maps\SiltCountySlice.umap" (
	"%UEEDITOR%" "SiltCounty.uproject" /Game/Maps/SiltCountySlice -game -log
) else (
	echo Opening the editor so it can finish writing the slice. Press Play ^(Alt+P^) when it loads.
	"%UEEDITOR%" "SiltCounty.uproject"
)
endlocal
