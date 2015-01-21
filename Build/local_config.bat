@ECHO off

SET WindowsSDKBasePath=C:\Program Files (x86)\Windows Kits\8.1\
REM IF NOT EXIST "%WindowsSDKBasePath%\NUL" (
REM     ECHO Invalid WindowsSDKBasePath = '%WindowsSDKBasePath%'
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
ECHO .VSBasePath         = '%VS120COMNTOOLS%..\..\'
ECHO .DirectX11SDKPath   = '%DXSDK_DIR%'
ECHO .WindowsSDKBasePath = '%WindowsSDKBasePath%'
ECHO .ClangBasePath      = '%ClangBasePath%'
