#/usr/bin/env bash
# Generated bash completion script by C:/Code/PPE/ppe.rb
_ppe_completions() {
local cur prev opts targets
COMPREPLY=()
cur="${COMP_WORDS[COMP_CWORD]}"
prev="${COMP_WORDS[COMP_CWORD-1]}"
opts="-w -c --clean --bash.d --bff --cpphint --dist-clean --export --fbuild --genpch --insights --print --vcxproj --vscode --Rebuild --no-Rebuild --Timings --no-Timings --x-fbuild --Cache --no-Cache --Compiler --CppStd --DeoptimizeWithToken --no-DeoptimizeWithToken --Diagnose --no-Diagnose --Dist --no-Dist --Incremental --no-Incremental --LTO --no-LTO --LightCache --no-LightCache --Minify --no-Minify --PCH --no-PCH --PDB --no-PDB --PerfSDK --no-PerfSDK --RuntimeChecks --no-RuntimeChecks --StackSize --StopOnError --no-StopOnError --Strict --no-Strict --Symbols --no-Symbols --Unity --no-Unity --UnitySize -v -q -d -T -t --version -h"
targets="Tools/UnitTest-Win32-Debug Win32-Debug Tools/UnitTest-Win32-FastDebug Win32-FastDebug Tools/UnitTest-Win32-Release Win32-Release Tools/UnitTest-Win32-Profiling Win32-Profiling Tools/UnitTest-Win32-Final Win32-Final Tools/UnitTest-Win64-Debug Win64-Debug Tools/UnitTest-Win64-FastDebug Win64-FastDebug Tools/UnitTest-Win64-Release Win64-Release Tools/UnitTest-Win64-Profiling Win64-Profiling Tools/UnitTest-Win64-Final Win64-Final Tools-Win32-Debug Tools-Win32-FastDebug Tools-Win32-Release Tools-Win32-Profiling Tools-Win32-Final Tools-Win64-Debug Tools-Win64-FastDebug Tools-Win64-Release Tools-Win64-Profiling Tools-Win64-Final Tools Tools/UnitTest-Win32 Win32 Tools-Win32 Tools/UnitTest-Win64 Win64 Tools-Win64 Tools/UnitTest-Debug Debug Tools-Debug Tools/UnitTest-FastDebug FastDebug Tools-FastDebug Tools/UnitTest-Release Release Tools-Release Tools/UnitTest-Profiling Profiling Tools-Profiling Tools/UnitTest-Final Final Tools-Final Tools/UnitTest All"
if [[ ${cur} == -* ]] ; then
   COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
   return 0
else
   COMPREPLY=( $(compgen -W "${targets}" -- ${cur}) )
   return 0
fi
}
complete -F _ppe_completions ppe.rb
