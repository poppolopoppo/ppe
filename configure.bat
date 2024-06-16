@ECHO OFF

chdir /d "%~dp0"
go build PPE.go
.\PPE configure -and vscode -and vcxproj -Summary

echo 
echo PPE configuration successful: run ./PPE help to see available commands
echo Shell ompletions bindings are available in %~dp0Extras
