@ECHO OFF
@REM launch build of project through fast build

CD "%~d0\%~p0"

IF NOT EXIST "%~d0\%~p0\Build\local_config.bff" (
    ECHO "New machine detected, creating local configuration ..."
    "%~d0\%~p0\Build\local_config.bat" > "%~d0\%~p0\Build\local_config.bff"
)

"%~d0\%~p0\Build\FBuild.exe" %*
