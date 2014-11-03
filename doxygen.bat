REM launch build of documentation through doxygen

cd "%~d0\%~p0\Doc"
"%~d0\%~p0\Doc\doxygen.exe" %*
