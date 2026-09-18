@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo.
echo   VALLEY GOD
echo   Earth. One valley. They do not know you.
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

if not exist "ValleyGod.sln" (
	echo Generating Visual Studio project files...
	if exist "%UEUBT%" (
		"%UEUBT%" -projectfiles -project="%~dp0ValleyGod.uproject" -game -engine -progress
	) 	else (
		"%UEEDITOR%" "%~dp0ValleyGod.uproject" -projectfiles
	)
)

echo Compiling VALLEY GOD ^(first time can take a while^)...
call "%UEBUILD%" ValleyGodEditor Win64 Development -Project="%~dp0ValleyGod.uproject" -WaitMutex
if errorlevel 1 (
	echo.
	echo Compile failed. Open ValleyGod.uproject in Unreal and let it build from there.
	pause
	exit /b 1
)

echo Preparing wet dirt materials and the empty valley map...
"%UEEDITOR%" "%~dp0ValleyGod.uproject" -run=ValleyPrep -unattended -nopause -nosplash -log
if errorlevel 1 (
	echo Prep commandlet reported an error. Launching anyway — runtime materials cover missing Content.
)

echo.
echo Launching. Fly. Watch. Do not expect them to look up.
echo WASD + Q/E fly    mouse look    1-4 weather    0 clear    P pause
echo.

if exist "%~dp0Content\Maps\ValleySlice.umap" (
	"%UEEDITOR%" "%~dp0ValleyGod.uproject" /Game/Maps/ValleySlice -game -log
) else (
	echo Slice map not on disk yet. Launching -game on the default map; the valley still spawns in code.
	"%UEEDITOR%" "%~dp0ValleyGod.uproject" -game -log
)
endlocal
