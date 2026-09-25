@echo off
setlocal EnableExtensions EnableDelayedExpansion
rem Quotes matter: Windows usernames and project paths may contain spaces.
set "PROJ=%~dp0SiltCounty.uproject"
set "EDITOR="

if not "%~1"=="" (
  set "EDITOR=%~1"
  goto :launch
)

for %%V in (5.4 5.5 5.6 5.7 5.8) do (
  set "TRY=C:\Program Files\Epic Games\UE_%%V\Engine\Binaries\Win64\UnrealEditor.exe"
  if exist "!TRY!" set "EDITOR=!TRY!"
)

if not defined EDITOR (
  echo Unreal Editor 5.4+ was not found under "C:\Program Files\Epic Games".
  echo Install Unreal Engine 5.4 or newer, then either:
  echo   double-click "%PROJ%"
  echo or run this script with the editor path in quotes:
  echo   "%~f0" "C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe"
  exit /b 1
)

:launch
echo Opening "%PROJ%"
start "" "%EDITOR%" "%PROJ%"
exit /b 0
