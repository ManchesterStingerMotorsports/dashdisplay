@echo off
setlocal

if not "%~2"=="" goto usage
if not "%~1"=="" if /i not "%~1"=="build" if /i not "%~1"=="preview" goto usage

if not defined FSDASH_MSYS2_ROOT (
    set "FSDASH_MSYS2_ROOT=C:\msys64"
    if exist "C:\msys64-dashdisplay\ucrt64\bin\g++.exe" set "FSDASH_MSYS2_ROOT=C:\msys64-dashdisplay"
)
if not exist "%FSDASH_MSYS2_ROOT%\usr\bin\make.exe" goto missing_tools
if not exist "%FSDASH_MSYS2_ROOT%\ucrt64\bin\g++.exe" goto missing_tools
if not exist "%FSDASH_MSYS2_ROOT%\ucrt64\bin\SDL2.dll" goto missing_tools

if not exist "%~dp0deps\lvgl\lvgl.h" goto missing_deps
if not exist "%~dp0deps\lv_drivers\sdl\sdl.c" goto missing_deps

pushd "%~dp0" || exit /b 1
set "MSYSTEM=UCRT64"
set "CHERE_INVOKING=1"
"%FSDASH_MSYS2_ROOT%\usr\bin\bash.exe" -lc "make -j$(nproc)"
set "result=%errorlevel%"
if not "%result%"=="0" goto done
if /i "%~1"=="build" goto done

set "PATH=%FSDASH_MSYS2_ROOT%\ucrt64\bin;%PATH%"
python -B test_src\preview_dashboard.py --launch
set "result=%errorlevel%"

:done
popd
exit /b %result%

:missing_tools
echo MSYS2 UCRT64 with Make, GCC, SDL2 and pkg-config is required. 1>&2
echo Set FSDASH_MSYS2_ROOT if it is installed in a different folder. 1>&2
exit /b 1

:missing_deps
echo Place LVGL 8.3.11 in deps\lvgl and lv_drivers in deps\lv_drivers. 1>&2
exit /b 1

:usage
echo Usage: dash.cmd [build ^| preview] 1>&2
exit /b 1
