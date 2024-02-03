#!/bin/sh

BASEDIR=$(dirname "$0")
cd "$BASEDIR" && \
    go build PPE.go && \
    ./PPE configure -and vscode -and vcxproj -Summary && \
    echo -e "\nPPE configuration successful: run ./PPE help to see available commands" && \
    echo -e "Shell ompletions bindings are available in $BASEDIR/Extras"
