#!/bin/sh

LLVMPATH_X86='C:\Program Files (x86)\LLVM'
LLVMPATH_X64='C:\Program Files\LLVM'

if [ -d "$LLVMPATH_X64" ]; then
    LLVMPATH=$LLVMPATH_X64
else
    LLVMPATH=$LLVMPATH_X86
fi

GIT_BRANCH="master"
GIT_REMOTE="bitbucket"

WORKER_COUNT=8

CODINGSTYLE="
{
    Language: Cpp,
    BasedOnStyle: Google,
    AccessModifierOffset: -4,
    AlignConsecutiveAssignments: false,
    AlignOperands: false,
    AlignTrailingComments: true,
    AllowAllParametersOfDeclarationOnNextLine: true,
    AllowShortBlocksOnASingleLine: false,
    AllowShortCaseLabelsOnASingleLine: true,
    AllowShortFunctionsOnASingleLine: Inline,
    AllowShortIfStatementsOnASingleLine: false,
    AlwaysBreakTemplateDeclarations: true,
    BinPackArguments: false,
    BinPackParameters: true,
    BreakBeforeBinaryOperators: None,
    BreakBeforeBraces: Attach,
    BreakBeforeTernaryOperators: true,
    BreakConstructorInitializersBeforeComma: true,
    ColumnLimit: 100,
    ConstructorInitializerAllOnOneLineOrOnePerLine: false,
    ConstructorInitializerIndentWidth: 0,
    Cpp11BracedListStyle: true,
    DerivePointerAlignment: false,
    IndentCaseLabels: false,
    IndentWidth: 4,
    NamespaceIndentation: None,
    PointerAlignment: Middle,
    SpaceBeforeAssignmentOperators: true,
    SpaceBeforeParens: ControlStatements,
    SpaceInEmptyParentheses: false,
    SpacesInParentheses: false,
    UseTab: Never,
}"
CODINGSTYLE=$(echo $CODINGSTYLE)

LINE=0
function print_infos
{
    if [ "$(expr $LINE % 2)" -eq 0 ]; then
        echo -e "\e[0;34;46m  INFOS  \e[5;34;46m $* \033[0m" 1>&2
    else
        echo -e "\e[5;34;46m  INFOS  \e[0;30;46m $* \033[0m" 1>&2
    fi
    LINE=$(expr $LINE + 1)
}
function print_step
{
    echo -e "\e[5;36;44m\n   STEP  \e[0;36;44m $* \033[0m" 1>&2
}
function print_merge
{
    echo -e "\e[5;35;42m\n  MERGE  \e[0;35;42m $* \033[0m" 1>&2
}
function print_error
{
    echo -e "\e[5;33;41m\n  ERROR  \e[0;33;41m $* \033[0m" 1>&2
}
function print_warning
{
    echo -e "\e[5;31;43m\n WARNING \e[0;31;43m $* \033[0m" 1>&2
}

cd "$PWD/$(dirname "$0")/.."
print_infos "Git branch = '$GIT_BRANCH'"
print_infos "LLVM path = '$LLVMPATH'"
print_infos "Repository path = '$PWD'"

print_step "1) Switching to git branch '$GIT_BRANCH'"
if ! git checkout --force "$GIT_BRANCH"; then
    print_error "git: failed to switch branch"
    exit 2
fi
if ! git reset --hard "$GIT_REMOTE/$GIT_BRANCH"; then
    print_error "git: failed to reset hard"
    exit 2
fi

LOG_HISTORY="$PWD/../history.txt"
print_step "2) Fetching git full history for branch '$GIT_BRANCH' in '$LOG_HISTORY'"
if ! git log --format="%H" --reverse > "$LOG_HISTORY"; then
    print_error "git: failed to retrieve history"
    exit 2
fi
echo "Found $(wc -l < "$LOG_HISTORY") commits."

#GIT_TEMP_BRANCH="tmp_$(date +"%y%m%d%H%M%S")"
GIT_TEMP_BRANCH="tmp"
print_step "3) Switching to orphan git branch '$GIT_TEMP_BRANCH'"
git branch -D "$GIT_TEMP_BRANCH"
if ! git checkout --orphan "$GIT_TEMP_BRANCH"; then
    print_error "git: failed to switch to orphan branch"
    exit 2
fi

print_step "4) Removing all files from git cache"
if ! (git rm --cached -r * .gitignore || rm -Rf * .gitignore); then
    print_error "git: failed to remove all cached files"
    exit 2
fi
if ! git clean -fdx .; then
    print_error "git: failed to clean the repository"
    exit 2
fi

print_step "5) Origin empty commit"
if ! echo $(date) > EPOK; then
    print_error "git: failed to create epok"
    exit 2
fi
if ! git add EPOK; then
    print_error "git: failed to add epok"
    exit 2
fi
if ! git commit -m 'EPOK'; then
    print_error "git: failed to commit epok"
    exit 2
fi

CLANG_FORMAT="$LLVMPATH\\bin\\clang-format"
print_step "6) Apply each commit with formatting"
for sha1 in $(cat "$LOG_HISTORY"); do
    print_merge "Commit: $sha1"
    if ! git cherry-pick "$sha1"; then
        if ! git mergetool; then
            print_error "git: failed to cherry-pick"
            exit 2
        fi
    fi
    MODIFIED_SOURCEFILES=$(git diff --name-only HEAD~1 HEAD | grep -E '\.(h|cpp|fx|fxh)$')
    MODIFIED_SOURCEFILES_COUNT=$(echo $MODIFIED_SOURCEFILES | wc -w)
    print_infos "$MODIFIED_SOURCEFILES_COUNT sources files ($WORKER_COUNT workers)"
    if ! echo $MODIFIED_SOURCEFILES | xargs -P $WORKER_COUNT -n 1 "$CLANG_FORMAT" -i "-style=$CODINGSTYLE"; then
        print_error "clang-format: failed to format source files"
        exit 2
    fi
    print_infos "Add formatted source files to git"
    if ! git add $MODIFIED_SOURCEFILES; then
        print_error "git: failed to add source files"
        exit 2
    fi
    print_infos "Amend commit"
    if ! git commit --amend --no-edit; then
        print_error "git: failed to amend commit"
        exit 2
    fi
done
