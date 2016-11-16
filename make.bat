@echo off
pushd %~dp0

rem Project
for /f "tokens=2 delims=( " %%i in ('findstr /c:"project(" CMakeLists.txt') do (
  set project=%%i
)

rem Settings
set gegnerator=Visual Studio 14 2015 Win64
set toolset=v140
set configs=Release;Debug

if exist ..\..\res\settings.bat (
  call ..\..\res\settings.bat
)

rem Build Directory
if not exist build (
  mkdir build
)
pushd build

rem Solution
cmake -G "%gegnerator%" -T "%toolset%" -DCMAKE_CONFIGURATION_TYPES="%configs%" -DCMAKE_INSTALL_PREFIX:PATH=.. ..
if %errorlevel% == 0 (
  for %%a in (%configs%) do (
    cmake --build . --target install --config "%%a"
  )
) else (
  echo.
  echo Error: Configure failed for project "%project%".
  echo Generator      : "%gegnerator%"
  echo Toolset        : "%toolset%"
  echo Configurations : "%configs%"
  echo.
  pause
)

popd
popd
