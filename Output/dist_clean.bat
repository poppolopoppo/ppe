@ECHO OFF
@REM Cleanup all intermediates and binaries

echo [%DATE%] > "%~d0\%~p0\dist_clean.log"

FOR %%D IN (Binary Intermediate Resources Unity ..\Doc\Doxygen) DO (
    echo Deleting directory '%~d0%~p0%%D' >> "%~d0\%~p0\dist_clean.log"
    rmdir /S /Q "%~d0\%~p0\%%D" >> "%~d0\%~p0\dist_clean.log"
)

cd %~d0%~p0.. && FOR /F %%F IN ('dir /b/s *.suo *.sdf *.fdb') DO (
    echo Deleting file '%%F' >> "%~d0\%~p0\dist_clean.log"
    del "%%F" >> "%~d0\%~p0\dist_clean.log"
) && cd %~d0%~p0

REM end
