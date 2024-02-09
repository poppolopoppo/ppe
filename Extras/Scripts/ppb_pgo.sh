#!/bin/sh

BASEDIR=$(dirname "$0")
PPEDIR="$BASEDIR/../.."

function ppb_profile_command() {
    ./PPE $*
    go tool pprof -proto cpu.pprof default.pgo > merged.pprof && mv merged.pprof default.pgo
}

cd "$PPEDIR" && \
    go build -tags=ppb_profiling PPE.go && \
    echo > default.pgo && \
    ppb_profile_command configure -F && \
    ppb_profile_command configure -F -q -Ide && \
    ppb_profile_command configure -and vscode -and vcxproj -F -q -Ide && \
    ppb_profile_command configure -and vscode -and vcxproj -q -Ide && \
    ppb_profile_command configure && \
    ppb_profile_command check-serialize && \
    ppb_profile_command check-serialize -q -Ide && \
    ppb_profile_command build Programs/UnitTest-Win64-Devel -Rebuild -Summary -CacheMode=READWRITE && \
    ppb_profile_command build Programs/UnitTest-Win64-Devel -q -Ide && \
    ppb_profile_command build Programs/UnitTest-Win64-Devel -Rebuild -Summary && \
    ppb_profile_command build Programs/ShaderToy-Win64-Test -Rebuild -Summary && \
    ppb_profile_command build Programs/ShaderToy-Win64-Test -q -Ide && \
    ppb_profile_command build Programs/ShaderToy-Win64-Test -Rebuild -Summary && \
    ppb_profile_command configure -and vscode -and vcxproj -F -q -Ide && \
    ppb_profile_command compiledb && \
    ppb_profile_command check-cache && \
    ppb_profile_command export-config Test && \
    go build PPE.go