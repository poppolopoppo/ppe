@ECHO OFF
@REM launch build of project through fast build

CD "%~d0\%~p0"

IF NOT EXIST "%~d0\%~p0\Build\local_config.bff" (
    ECHO No local configuration detected, running first run setup ...
    CMD /C "%~d0\%~p0\Build\local_config.bat" >"%~d0\%~p0\Build\local_config.bff"
)

"%~d0\%~p0\Build\FBuild.exe" %*
