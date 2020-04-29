#!/bin/bash

## 	pre-commit hook
##  ---------------

if git rev-parse --verify HEAD >/dev/null 2>&1
then
	against=HEAD
else
	# Initial commit: diff against an empty tree object
	against=$(git hash-object -t tree /dev/null)
fi

# Redirect output to stderr.
exec 1>&2

## 	Helpers
##  -------

HOOK_EXITCODE=0

function die()
{
	>&2 echo " (!!) ERROR: $*"
	HOOK_EXITCODE=2
}

##  List modified files
##  -------------------

MODIFIED_FILES=$(git diff-index --cached --name-only $against)

##  Bail out for git-repo commits, they should *NEVER* fail
##  -------------------------------------------------------

if test $(cat $MODIFIED_FILES | grep '.gitrepo' | wc -l) != 0; then
    exit 0 # always succeed for git-subrepo commits
fi

## 	Check for non-ASCII filenames (sample)
##  --------------------------------------

# Cross platform projects tend to avoid non-ASCII filenames; prevent
# them from being added to the repository. We exploit the fact that the
# printable range starts at the space character and ends with tilde.
if [ "$allownonascii" != "true" ] &&
	# Note that the use of brackets around a tr range is ok here, (it's
	# even required, for portability to Solaris 10's /usr/bin/tr), since
	# the square bracket bytes happen to fall in the designated range.
	test $(git diff --cached --name-only --diff-filter=A -z $against |
	  LC_ALL=C tr -d '[ -~]\0' | wc -c) != 0
then
	die <<\EOF
Error: Attempt to add a non-ASCII file name.

This can cause problems if you want to work with people on other platforms.

To be portable it is advisable to rename the file.

If you know what you are doing you can disable this check using:

  git config hooks.allownonascii true
EOF
fi

##  Select files to check
##  ---------------------

FILES_TO_CHECK=""
FILES_TO_EXCLUDE='@(Extras/pre-commit.sh)' # contain %NOCOMMIT% for documentation
CHECKED_EXTS='@(*.adoc|*.bff|*.c|*.cpp|*.h|*.cs|*.ini|*.md|*.py|*.rb|*.sh)'

for f in $MODIFIED_FILES; do
    if [[ $f == $CHECKED_EXTS ]]; then
        if [[ $f != $FILES_TO_EXCLUDE ]]; then
            FILES_TO_CHECK="$FILES_TO_CHECK $f"
        fi
    fi
done

if [ -z "$FILES_TO_CHECK" ]; then
    exit $HOOK_EXITCODE # no files to check, bail
fi

## 	Check for whitespace errors (sample)
##  ------------------------------------

# If there are whitespace errors, print the offending file names and fail.
# -> only check white listed files
if ! git diff-index --check --cached $against -- $FILES_TO_CHECK; then
	die "found whitespace error(s)"
fi

## 	Check for %NOCOMMIT% tag inside the diff
##  ----------------------------------------

function diff-lines()
{
    local path=
    local line=
    while read; do
        esc=$'\033'
        if [[ $REPLY =~ ---\ (a/)?.* ]]; then
            continue
        elif [[ $REPLY =~ \+\+\+\ (b/)?([^[:blank:]$esc]+).* ]]; then
            path=${BASH_REMATCH[2]}
        elif [[ $REPLY =~ @@\ -[0-9]+(,[0-9]+)?\ \+([0-9]+)(,[0-9]+)?\ @@.* ]]; then
            line=${BASH_REMATCH[2]}
        elif [[ $REPLY =~ ^($esc\[[0-9;]+m)*([\ +-]) ]]; then
            echo "$path:$line:$REPLY"
            if [[ ${BASH_REMATCH[2]} != - ]]; then
                ((line++))
            fi
        fi
    done
}

# Print the offending lines and fail if %NOCOMMIT% tags are present
# -> only check white listed files
if git diff --cached $against -- $FILES_TO_CHECK | diff-lines | grep --color=auto '%NOCOMMIT%'; then
	die "found %NOCOMMIT% tag(s)"
fi

## 	Finally return an exit code indicating whether any check failed or not
##  ----------------------------------------------------------------------

if [ $HOOK_EXITCODE -ne 0 ]; then
	>&2 echo "abort!"
	exit $HOOK_EXITCODE
fi