SETLOCAL ENABLEEXTENSIONS
@echo on

SET SRC_ROOT=%cd%
SET CMAKE_ROOT=%cd%\cmake_build

IF ["%VSINSTALLDIR%"]==[""] goto :NO_VS_VARS

set CMAKE="%VSINSTALLDIR%\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"
set NINJA="%VSINSTALLDIR%\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\Ninja.exe"

set CMAKE_OPTIONS=-G "Ninja" -DUSE_INSTALLED_FRAMEWORK=ON -DCMAKE_MAKE_PROGRAM=%NINJA% -DCMAKE_INSTALL_PREFIX:PATH="%SRC_ROOT%\artifacts\install"

IF [%1]==[] set INVALID_ARG=1
IF [%2]==[] set INVALID_ARG=1
IF [%3]==[] set INVALID_ARG=1

if NOT INVALID_ARG == 1 goto :VALID_CMD_ARGS
echo "Invalid arguements!"
echo "build_win.cmd BUILD_TYPE ADAPTER_TYPE TARGET_NAME [BUILD_NUM]
echo "BUILD_TYPE: debug | release"
echo "ADAPTER_TYPE: real | simulator"
echo "TARGET_NAME: clean | build | rebuild"
echo "BUILD_NUM: (optional) XX.XX.XX.XXXX"
EXIT /B 1

:VALID_CMD_ARGS
:: the first parameter is an optional BUILD_TARGET
SET BUILD_TARGET=%3

IF [%3]==[] SET BUILD_TARGET=REBUILD

IF /I "%BUILD_TARGET%" == "clean" (
	set BUILD_TARGET=clean
	goto :VALID_BUILD_TARGET)

IF /I "%BUILD_TARGET%" == "build" (
	set BUILD_TARGET=all
	goto :VALID_BUILD_TARGET)
	
IF /I "%BUILD_TARGET%" == "rebuild" (
	set BUILD_TARGET=rebuild
	goto :VALID_BUILD_TARGET)

:: Invalid build type specified
echo "Must specify BUILD_TARGET: CLEAN, BUILD, REBUILD (default)."
EXIT /B 1

:VALID_BUILD_TARGET
echo "BUILD_TARGET=%BUILD_TARGET%"

set BUILD_TYPE=%1
set ADAPTER_TYPE=%2
IF NOT [%4]==[] set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILDNUM=%4

if "%BUILD_TYPE%"=="release" (
	set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DRELEASE=ON -DCMAKE_BUILD_TYPE="RelWithDebInfo"
) else ( set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE="Debug" )

if "%ADAPTER_TYPE%"=="simulator" (
	set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DBUILD_SIM=ON
	)

echo "cmake %CMAKE_OPTIONS% %SRC_ROOT%"

if exist %CMAKE_ROOT% rmdir /s /q %CMAKE_ROOT%
mkdir %CMAKE_ROOT%
pushd %CMAKE_ROOT%

%CMAKE% %CMAKE_OPTIONS% %SRC_ROOT%
if %ERRORLEVEL% NEQ 0 echo "***BUILD FAILED *** " && popd && EXIT /B 1

if /I "%BUILD_TARGET%"=="rebuild" goto :NINJA_REBUILD

:NINJA_BUILD
%NINJA% -v %BUILD_TARGET%
if %ERRORLEVEL% NEQ 0 echo "***BUILD FAILED *** " && popd && EXIT /B 1

popd

echo "*** Windows built successfully! ***"
EXIT /B 0

:NINJA_REBUILD
%NINJA% clean
set BUILD_TARGET=all
goto :NINJA_BUILD

:NO_VS_VARS
echo "***BUILD FAILED *** "
echo "build_win.cmd must be run with environment set from Visual Studio. e.g. vcvars64.bat"
EXIT /B 1