
module PPB {
    def ppe-find-executable [path: string] {
        let executable = [$path, "PPE.exe"] | path join
        if ($executable | path exists) {
            return [$executable, $"-RootDir=($path)"]
        }
        let parent = $path | path dirname
        if ($parent | is-empty) {
            return []
        }
        ppe-find-executable $parent
    }

    def ppe-complete-args [context: string] {
        let ppe_exe = ppe-find-executable $env.PWD
        if ($ppe_exe | is-empty) {
            return []
        }
        let cmd_opts = if ($context | str ends-with " ") { ["-CompleteArg"] } else { [] } | append ["-MaxResults=100", "-q", "-Ide", "-LogImmediate"]
        let cmd_args = $context | split row -r '\s+' | skip 1
        run-external $ppe_exe.0 ...($ppe_exe | skip 1) autocomplete ...$cmd_opts '--' ...$cmd_args | lines | parse "{value}\t{description}"
    }

    export def --wrapped ppe [...args: string@ppe-complete-args] {
        let ppe_exe = ppe-find-executable $env.PWD
        run-external $ppe_exe.0 ...($ppe_exe | skip 1) ...$args
    }

    export def ppe-external-completer [spans: list<string>] {
        let ppe_exe = ppe-find-executable $env.PWD
        if ($ppe_exe | is-empty) {
            return []
        }
        let cmd_opts = if ($spans | last | is-empty) { ["-CompleteArg"] } else { [] }
            | append ["-MaxResults=20", "-q", "-Ide", "-LogImmediate"]
        let cmd_args = if ($spans | last | is-empty) { $spans | drop 1 } else { $spans } | skip 1
        let results = run-external $ppe_exe.0 ...($ppe_exe | skip 1) autocomplete ...$cmd_opts '--' ...$cmd_args
        $results | lines | parse "{value}\t{description}"
    }
}

use PPB ppe
use PPB ppe-external-completer

let base_completer = $env.config.completions.external.completer
let ppe_completer = {|spans: list<string>|
    ppe-external-completer $spans
}
$env.config.completions.external.completer = {|spans: list<string>|
    match $spans.0 {
    ppe => $ppe_completer,
    _ => $base_completer
    } | do $in $spans
}
