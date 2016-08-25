@echo off
rem Enter script directory.
pushd %~dp0

rem Set common variables.
call ..\common.bat

rem Parse project name.
for /f "tokens=2 delims=( " %%i in ('findstr /c:"project(" CMakeLists.txt') do (
  set project=%%i
)

rem Create and enter build directory.
if not exist build (
  mkdir build
)
pushd build

rem Generate the solution.
cmake -G "%generator%" -T "%toolset%" -DCMAKE_CONFIGURATION_TYPES="%configs%" -DCMAKE_INSTALL_PREFIX:PATH=.. ..
if not %errorlevel% == 0 (
  echo.
  echo Error: Configure failed for project "%project%".
  echo Generator      : "%generator%"
  echo Toolset        : "%toolset%"
  echo Configurations : "%configs%"
  echo.
  pause
  exit 1
)

rem Build and install the library.
for %%a in (%configs%) do (
  cmake --build . --target install --config "%%a"
  if not %errorlevel% == 0 (
    echo.
    echo Error: Build failed for project "%project%".
    echo Generator     : "%generator%"
    echo Toolset       : "%toolset%"
    echo Configuration : "%%a"
    echo.
    pause
    exit 2
  )
)

rem Leave build directory.
popd

rem Kill all processes that could block the build directory.
taskkill /IM VsHub.exe /F
taskkill /IM vctip.exe /F
taskkill /IM PerfWatson2.exe /F

rem Delete the build directory.
rmdir /q /s build

rem Leave script directory.
popd
