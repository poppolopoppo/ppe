#!/bin/sh

WORKER_COUNT=8

CODINGSTYLE="
{
    Language: Cpp,
    BasedOnStyle: Google,
    AccessModifierOffset: -4,
    AlignConsecutiveAssignments: false,
    AlignOperands: true,
    AlignTrailingComments: true,
    AllowAllParametersOfDeclarationOnNextLine: true,
    AllowShortBlocksOnASingleLine: false,
    AllowShortCaseLabelsOnASingleLine: false,
    AllowShortFunctionsOnASingleLine: Inline,
    AllowShortIfStatementsOnASingleLine: false,
    AlwaysBreakTemplateDeclarations: true,
    BinPackArguments: true,
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
    PointerAlignment: Left,
    SpaceBeforeAssignmentOperators: true,
    SpaceBeforeParens: ControlStatements,
    SpaceInEmptyParentheses: false,
    SpacesInParentheses: false,
    UseTab: Never,
}"
CODINGSTYLE=$(echo $CODINGSTYLE)

LLVMPATH_X86='C:\Program Files (x86)\LLVM'
LLVMPATH_X64='C:\Program Files\LLVM'

if [ -d "$LLVMPATH_X64" ]; then
    LLVMPATH=$LLVMPATH_X64
else
    LLVMPATH=$LLVMPATH_X86
fi
CLANG_FORMAT="$LLVMPATH\\bin\\clang-format"

LINE=0
function print_infos
{
    if [ "$(expr $LINE % 2)" -eq 0 ]; then
        echo -e "\e[0;34;46m  INFOS  \e[5;30;46m $* \033[0m" 1>&2
    else
        echo -e "\e[5;34;46m  INFOS  \e[0;34;46m $* \033[0m" 1>&2
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
print_infos "LLVM path = '$LLVMPATH'"
print_infos "Repository path = '$PWD'"

print_step "Search source files"
SOURCEFILES=$(find . -name '*.h' -or -name '*.cpp')
SOURCEFILES=$(echo $SOURCEFILES)
SOURCEFILES_COUNT=$(echo $SOURCEFILES | wc -w)

print_step "$SOURCEFILES_COUNT sources files ($WORKER_COUNT workers)"
if ! echo $SOURCEFILES | xargs -P $WORKER_COUNT -n 1 "$CLANG_FORMAT" -i "-style=$CODINGSTYLE"; then
    print_error "clang-format: failed to format source files"
    exit 2
fi

print_step "Add formatted source files to git"
if ! git add $SOURCEFILES; then
    print_error "git: failed to add source files"
    exit 2
fi
