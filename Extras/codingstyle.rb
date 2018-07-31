
CODINGSTYLE = <<END_STYLE
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
    ExperimentalAutoDetectBinPacking: false,
    ForEachMacros: ['forrange', 'reverseforrange', 'foreachitem', 'foreachitem', 'reverseforeachitem'],
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
