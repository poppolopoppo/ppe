# script to migrate all includes

require './codingstyle.rb'

CLANGFORMAT_DIFF = File.join(LLVMPATH, 'tools', 'clang-format-diff.py')

Dir.chdir(GITDIR)
$("git diff -U0 HEAD^ | clang-format-diff.py -i -p1 #{CODINGSTYLE_ARG}")
