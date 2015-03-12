@ECHO OFF
@REM Cleanup all intermediates and binaries

echo [%DATE%] > "%~dp0\dist_clean.log"

REM TODO: get in FBuild bff scripts to know why builds are not anymore in Binary/Intermediate/Resources/Unity
FOR %%D IN (x86-debug x86-release x86-profiling x86-final x86-clang-debug x64-debug x64-release x64-profiling x64-final x64-clang-debug) DO (
    echo Deleting directory '%~dp0%%D' >> "%~dp0\dist_clean.log"
    rmdir /S /Q "%~dp0\%%D" >> "%~dp0\dist_clean.log"
)

FOR %%D IN (Binary Intermediate Resources Unity ..\Doc\Doxygen) DO (
    echo Deleting directory '%~dp0%%D' >> "%~dp0\dist_clean.log"
    rmdir /S /Q "%~dp0\%%D" >> "%~dp0\dist_clean.log"
)

echo Deleting local config '%~dp0\..\Build\local_config.bff' >> "%~dp0\dist_clean.log"
del "%~dp0\..\Build\local_config.bff" >> "%~dp0\dist_clean.log"

cd %~d0%~p0.. && FOR /F %%F IN ('dir /b/s *.suo *.sdf *.fdb') DO (
    echo Deleting file '%%F' >> "%~dp0\dist_clean.log"
    del "%%F" >> "%~dp0\dist_clean.log"
) && cd %~dp0

REM end
