@ECHO OFF
@REM Cleanup all intermediates and binaries

echo [%DATE%] > "%~dp0\dist_clean.log"

FOR %%D IN (Binary Intermediate Resources Unity Projects ..\Doc\Doxygen Saved .vs) DO (
    echo Deleting directory '%~dp0%%D' >> "%~dp0\dist_clean.log"
    rmdir /S /Q "%~dp0\%%D" >> "%~dp0\dist_clean.log"
)

echo Deleting local config '%~dp0\..\Build\_solution_path.bff' >> "%~dp0\dist_clean.log"
del "%~dp0\..\Build\_solution_path.bff" >> "%~dp0\dist_clean.log"

cd %~d0%~p0.. && FOR /F %%F IN ('dir /b/s *.suo *.sdf *.fdb *.sln') DO (
    echo Deleting file '%%F' >> "%~dp0\dist_clean.log"
    del "%%F" >> "%~dp0\dist_clean.log"
) && cd %~dp0

REM end
