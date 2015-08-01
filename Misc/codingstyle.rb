
CODINGSTYLE = <<END_STYLE
{
    Language: Cpp,
    BasedOnStyle: Google,

    AccessModifierOffset: -4,
    AlignConsecutiveAssignments: true,
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
}
END_STYLE
CODINGSTYLE.gsub!("\n", '')
CODINGSTYLE_ARG="-style=#{CODINGSTYLE}"

GITDIR      = File.dirname($0)
SOURCEDIR   = File.join(GITDIR, '..', 'Source')

LLVMPATHX86 = 'C:\Program Files (x86)\LLVM'
LLVMPATHX64 = 'C:\Program Files\LLVM'
LLVMPATH    = Dir.exist?(LLVMPATHX64) ? LLVMPATHX64 : LLVMPATHX86

CLANGFORMAT = File.join(LLVMPATH, 'bin', 'clang-format')
