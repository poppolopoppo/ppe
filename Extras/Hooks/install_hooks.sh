#!/bin/sh

SCRIPT="#!/bin/sh

ruby "${0%/*}/pre-commit.rb"
"
rm -f .git/hooks/pre-commit
echo "$SCRIPT" > .git/hooks/pre-commit
