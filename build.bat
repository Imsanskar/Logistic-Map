@echo off

setlocal

if not exist "bin" mkdir bin
pushd bin


set compile_type="debug"

if "%1"=="debug" set compile_type="debug"
if "%1"=="release" set compile_type="release"
if "%1" == "run" goto Run
if "%2" == "run" goto Run

if %compile_type%=="release" goto ReleaseConfig
if %compile_type%=="debug" goto DebugConfig

:DebugConfig
echo -------------------------------------
echo 	Debug Build configured
echo -------------------------------------
set BUILD_TYPE=Debug
goto CompilerConfig


:ReleaseConfig
echo -------------------------------------
echo 	Release Build configured
echo -------------------------------------
set BUILD_TYPE=Release
goto CompilerConfig


:CompilerConfig
where cl >nul 2>nul
if %ERRORLEVEL% neq 0 goto ClangConfig
set C_COMPILER=cl
set CXX_COMPILER=cl
goto Compile

:ClangConfig
where clang >nul 2>nul
if %ERRORLEVEL% neq 0 goto CompilerConfigError
set C_COMPILER=clang
set CXX_COMPILER=clang++
goto Compile

:Compile
echo Build Type=%BUILD_TYPE%
echo C_COMPILER=%C_COMPILER%
echo CXX_COMPILER=%CXX_COMPILER%
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=%C_COMPILER% -DCMAKE_CXX_COMPILER=%CXX_COMPILER% -B./ -G"Unix Makefiles" ../
make
if %ERRORLEVEL% NEQ 0 goto CompileError
goto Run

:Run
Bifurcation.exe
goto Done

:CompileError
echo Compilation Failed
goto Done

:CompilerConfigError
echo Either install clang or run from developer command prompt

:Done
popd