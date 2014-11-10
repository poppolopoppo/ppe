@ECHO OFF
@REM Cleanup all intermediates and binaries

echo [%DATE%] > "%~d0\%~p0\dist_clean.log"

FOR %%D IN (Binary Intermediate Resources Unity) DO (
    echo Deleting '%~d0%~p0%%D' ... >> "%~d0\%~p0\dist_clean.log"
    rmdir /S /Q "%~d0\%~p0\%%D" >> "%~d0\%~p0\dist_clean.log"
)

REM end
