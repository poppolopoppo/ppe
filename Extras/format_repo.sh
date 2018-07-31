#!/bin/sh

WORKER_COUNT=$(($NUMBER_OF_PROCESSORS - 1))

CODINGSTYLE="
{
    Language: Cpp,
    Standard: Cpp11,
    BasedOnStyle: Google,
    AccessModifierOffset: -4,
    AlignConsecutiveAssignments: false,
    AlignOperands: true,
    AlignTrailingComments: true,
    AllowAllParametersOfDeclarationOnNextLine: true,
    AllowShortBlocksOnASingleLine: false,
    AllowShortCaseLabelsOnASingleLine: true,
    AllowShortFunctionsOnASingleLine: Inline,
    AllowShortIfStatementsOnASingleLine: false,
    AllowShortLoopsOnASingleLine: false,
    AlwaysBreakBeforeMultilineStrings: true,
    AlwaysBreakTemplateDeclarations: true,
    BinPackArguments: false,
    BinPackParameters: false,
    BreakBeforeBinaryOperators: None,
    BreakBeforeBraces: Attach,
    BreakBeforeTernaryOperators: true,
    BreakConstructorInitializersBeforeComma: true,
    ColumnLimit: 120,
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
    ExperimentalAutoDetectBinPacking: true,
}"
CODINGSTYLE="
{
    Language: Cpp,
    Standard: Cpp11,
    AccessModifierOffset: -4,
    AlignConsecutiveAssignments: false,
    AlignConsecutiveDeclarations: false,
    AlignOperands: true,
    AlignTrailingComments: false,
    AllowAllParametersOfDeclarationOnNextLine: true,
    AllowShortBlocksOnASingleLine: false,
    AllowShortCaseLabelsOnASingleLine: true,
    AllowShortFunctionsOnASingleLine: All,
    AllowShortIfStatementsOnASingleLine: false,
    AllowShortLoopsOnASingleLine: false,
    AlwaysBreakTemplateDeclarations: false,
    BinPackParameters: true,
    BreakBeforeBraces: Stroustrup,
    BreakBeforeTernaryOperators: true,
    ColumnLimit: 0,
    ConstructorInitializerAllOnOneLineOrOnePerLine: true,
    ConstructorInitializerIndentWidth: 4,
    DerivePointerBinding: true,
    ExperimentalAutoDetectBinPacking: true,
    IndentCaseLabels: false,
    IndentWidth: 4,
    IndentWrappedFunctionNames: true,
    KeepEmptyLinesAtTheStartOfBlocks: false,
    MaxEmptyLinesToKeep: 1,
    NamespaceIndentation: None,
    PenaltyExcessCharacter: 0,
    PointerAlignment: Left,
    SpacesInAngles: true,
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
    echo -e "\e[5;36;44m   STEP  \e[0;36;44m $* \033[0m" 1>&2
}
function print_merge
{
    echo -e "\e[5;35;42m  MERGE  \e[0;35;42m $* \033[0m" 1>&2
}
function print_error
{
    echo -e "\e[5;33;41m  ERROR  \e[0;33;41m $* \033[0m" 1>&2
}
function print_warning
{
    echo -e "\e[5;31;43m\n WARNING \e[0;31;43m $* \033[0m" 1>&2
}

cd "$PWD/$(dirname "$0")/.."
print_infos "LLVM path = '$LLVMPATH'"
print_infos "Repository path = '$PWD'"
print_infos "Worker count = $WORKER_COUNT"

print_step "Search source files"
SOURCEFILES=$(find Source/ -name '*.h' -or -name '*.cpp')
SOURCEFILES=$(echo $SOURCEFILES)
SOURCEFILES_COUNT=$(echo $SOURCEFILES | wc -w)

print_step "$SOURCEFILES_COUNT sources files ($WORKER_COUNT workers)"
if ! echo $SOURCEFILES | xargs -P $WORKER_COUNT -n 1 "$CLANG_FORMAT" -i "-style=$CODINGSTYLE"; then
    print_error "clang-format: failed to format source files"
    exit 2
fi

if false; then
    print_step "Add formatted source files to git"
    for f in $SOURCEFILES; do
        if ! git add $f; then
            print_error "git: failed to add source files"
            exit 2
        fi
    done
fi
