#/usr/bin/fish

# Disable file completions for the entire command
# because it does not take files anywhere
complete -c ppe.rb -f

# All subcommands
set -l commands --backup --bash.d --bff --cpphint --dist-clean --export --fbuild --genpch --genpch2 --insights --list-targets --print --ps1 --run --rundbg --upx --vcxproj --vscode

complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--bff --fbuild --run --vcxproj --vscode --genpch"

complete -c ppe.rb -n "__fish_seen_subcommand_from --fbuild"  -a "(ruby (pwd)/ppe.rb -q --list-targets | tr -d '\015' | sort)"
complete -c ppe.rb -n "__fish_seen_subcommand_from --run"  -a "(ruby (pwd)/ppe.rb -q --list-executables | tr -d '\015' | sort)"
complete -c ppe.rb -n "__fish_seen_subcommand_from --rundbg"  -a "(ruby (pwd)/ppe.rb -q --list-executables | tr -d '\015' | sort)"

complete -c ppe.rb -n "__fish_seen_subcommand_from --Compiler"  -a "2019 2022 Insider LLVM"
complete -c ppe.rb -n "__fish_seen_subcommand_from --CppStd"  -a "c++14 c++17 c++20 c++latest"

complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--backup" -d "Generate a snapshot backup archive"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--bash.d" -d "Generate bash completion script"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--bff" -d "BFF generator"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--cpphint" -d "Generate cpp.hint for Intellisense"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--dist-clean" -d "Delete generated files"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--export" -d "Export expanded targets (JSON)"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--fbuild" -d "FASTBuild interop"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--genpch" -d "Generate precompiled headers"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--genpch2" -d "Generate precompiled headers"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--insights" -d "Capture compilation insights"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--list-targets" -d "Export all targets in plain text"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--print" -d "Show config data"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--ps1" -d "Generate powershell completion script"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--run" -d "Run specified target"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--rundbg" -d "Run and debug specified target"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--upx" -d "Compressed specified target executable"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--vcxproj" -d "Generate VisualStudio project files"
complete -c ppe.rb -n "not __fish_seen_subcommand_from $commands" -a "--vscode" -d "Generate VisualStudio Code bindings"

# These are simple options that can be used everywhere.

complete -c ppe.rb -l Rebuild -d 'Clean build all specified targets'
complete -c ppe.rb -l no-Rebuild -d "Don't clean build all specified targets"
complete -c ppe.rb -l Timings -d 'Measure compilation timings'
complete -c ppe.rb -l no-Timings -d "Don't measure compilation timings"
complete -c ppe.rb -l x-fbuild -d "Pass option directly to FASTBuild"

complete -c ppe.rb -l ASAN -d "Use VisualStudio Address Sanitizer (ASAN)"
complete -c ppe.rb -l Cache -d "Use compilation cache"
complete -c ppe.rb -l Compile -d "Select Windows C++ compiler [2019,2022,Insider,LLVM]"
complete -c ppe.rb -l CppStd -d "Select C++ ISO standard [c++14,c++17,c++20,c++latest]"
complete -c ppe.rb -l DeoptimizeWithToken -d "Deoptimize TU when FASTBUILD_DEOPTIMIZE_OBJECT found"
complete -c ppe.rb -l Diagnose -d "Use compilation diagnostics"
complete -c ppe.rb -l Dist -d "Use distributed compilation"
complete -c ppe.rb -l Incremental -d "Use incremental linker"
complete -c ppe.rb -l JMC -d "Use VisualStudio Just-My-Code"
complete -c ppe.rb -l LTO -d "Use link time optimization"
complete -c ppe.rb -l LightCache -d "Experimental cache support using command line analysis"
complete -c ppe.rb -l Minify -d "Use minified format for exported files"
complete -c ppe.rb -l PCH -d "Use precompiled headers"
complete -c ppe.rb -l PDB -d "Generate a Program Debug Database"
complete -c ppe.rb -l PerfSDK -d "Use VisualStudio performance tools"
complete -c ppe.rb -l RuntimeChecks -d "Use runtime checks"
complete -c ppe.rb -l StackSize -d "Define default thread stack size"
complete -c ppe.rb -l StaticCRT -d "Use VisualStudio static CRT (/MT vs /MD)"
complete -c ppe.rb -l StopOnError -d "Stop on first compilation error"
complete -c ppe.rb -l Strict -d "Toggle warning as error and non permissive warnings"
complete -c ppe.rb -l Symbols -d "Toggle generation of debug symbols"
complete -c ppe.rb -l TraditionalPP -d "Use VisualStudio traditional preprocessor (omit /Zc:preprocessor)"
complete -c ppe.rb -l Unity -d "Use unity builds"
complete -c ppe.rb -l UnitySize -d "Size limit for splitting unity files"
complete -c ppe.rb -l WorkerCount -d "Number of parallel workers (0 is automatic)"

complete -c ppe.rb -l no-ASAN -d "Don't use VisualStudio Address Sanitizer (ASAN)"
complete -c ppe.rb -l no-Cache -d "Don't use compilation cache"
complete -c ppe.rb -l no-DeoptimizeWithToken -d "Don't deoptimize TU when FASTBUILD_DEOPTIMIZE_OBJECT found"
complete -c ppe.rb -l no-Diagnose -d "Don't use compilation diagnostics"
complete -c ppe.rb -l no-Dist -d "Don't use distributed compilation"
complete -c ppe.rb -l no-Incremental -d "Don't use incremental linker"
complete -c ppe.rb -l no-JMC -d "Don't use VisualStudio Just-My-Code"
complete -c ppe.rb -l no-LTO -d "Don't use link time optimization"
complete -c ppe.rb -l no-LightCache -d "Don't experimental cache support using command line analysis"
complete -c ppe.rb -l no-Minify -d "Don't use minified format for exported files"
complete -c ppe.rb -l no-PCH -d "Don't use precompiled headers"
complete -c ppe.rb -l no-PDB -d "Don't generate a Program Debug Database"
complete -c ppe.rb -l no-PerfSDK -d "Don't use VisualStudio performance tools"
complete -c ppe.rb -l no-RuntimeChecks -d "Don't use runtime checks"
complete -c ppe.rb -l no-StaticCRT -d "Don't use VisualStudio static CRT (/MT vs /MD)"
complete -c ppe.rb -l no-StopOnError -d "Don't stop on first compilation error"
complete -c ppe.rb -l no-Strict -d "Don't toggle warning as error and non permissive warnings"
complete -c ppe.rb -l no-Symbols -d "Don't toggle generation of debug symbols"
complete -c ppe.rb -l no-TraditionalPP -d "Don't use VisualStudio traditional preprocessor (omit /Zc:preprocessor)"
complete -c ppe.rb -l no-Unity -d "Don't use unity builds"

complete -c ppe.rb -s v -l verbose -d 'Run verbosely'
complete -c ppe.rb -s q -l quiet -d 'Run quietly'
complete -c ppe.rb -s d -l debug -d 'Run with debug'
complete -c ppe.rb -s T -l trace -d 'Trace program execution'
complete -c ppe.rb -l version -d 'Show build version'
complete -c ppe.rb -s h -l help -d 'Show help instructions'
