@REM launch build of documentation through doxygen

@REM should install Graphviz2.38
SET PATH=C:\Program Files (x86)\Graphviz2.38\bin\;%PATH%

cd "%~d0\%~p0\Doc"
"%~d0\%~p0\Doc\doxygen.exe" %*
