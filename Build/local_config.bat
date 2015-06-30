@ECHO off

SET WindowsSDKBasePath81=C:\Program Files (x86)\Windows Kits\8.1\
REM IF NOT EXIST "%WindowsSDKBasePath81%\NUL" (
REM     ECHO Invalid WindowsSDKBasePath81 = '%WindowsSDKBasePath81%'
REM     EXIT /b
REM )

SET WindowsSDKBasePath10=C:\Program Files (x86)\Windows Kits\10\
REM IF NOT EXIST "%WindowsSDKBasePath10%\NUL" (
REM     ECHO Invalid WindowsSDKBasePath10 = '%WindowsSDKBasePath10%'
REM     EXIT /b
REM )

SET ClangBasePath=D:\Program Files (x86)\LLVM\
REM IF NOT EXIST "%ClangBasePath%\NUL" (
REM     ECHO "Invalid ClangBasePath = '%ClangBasePath%'"
REM     EXIT /b
REM )

ECHO #once
ECHO ;-------------------------------------------------------------------------------
ECHO ; Local Config
ECHO ;-------------------------------------------------------------------------------
ECHO .MSVC12BasePath        = '%VS120COMNTOOLS%..\..\'
ECHO .MSVC14BasePath        = '%VS140COMNTOOLS%..\..\'
ECHO .WindowsSDKBasePath81  = '%WindowsSDKBasePath81%'
ECHO .WindowsSDKBasePath10  = '%WindowsSDKBasePath10%'
ECHO .ClangBasePath         = '%ClangBasePath%'
ECHO .DirectXSDKPath        = '%DXSDK_DIR%'
