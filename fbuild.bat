@ECHO OFF
@REM launch build of project through fast build

cd "%~d0\%~p0"
"%~d0\%~p0\Build\FBuild.exe" %*
