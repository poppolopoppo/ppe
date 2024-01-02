# ppe.fish

# Function to recursively search for an executable
function __ppe_find_executable --description 'Find an executable recursively starting from current directory'
    set -l executable_name "$argv[1]"

    # Get the absolute path to the current working directory
    set -f current_dir (pwd)

    # Loop through parent directories until root
    while test "$current_dir" != "/"
        # Check if the executable exists in the current directory
        if test -x "$current_dir/$executable_name"
            echo (realpath --relative-to=$PWD "$current_dir/$executable_name")
            return
        end

        # Move up to the parent directory
        set -f current_dir (dirname "$current_dir")
    end

    # If not found, return an empty string or handle accordingly
    echo ""
end

function __ppe_run
    set -l executable_path "$__ppe_executable_name"

    if test -x "./$executable_path"
        "./$executable_path" $argv
    else
        set -l executable_path (__ppe_find_executable "$executable_path")
        set -l root_dir (dirname "$executable_path")
        "./$executable_path" -RootDir="$root_dir" $argv
    end
end

function __ppe_is_cursor_at_end_of_line
  set -l cursor_pos (commandline --cursor)
  set -l line (string sub --start $cursor_pos (commandline))
  string match -q -r '^\\s*$' $line
end

function ppe --description 'Run PoPpOlOpPoPpO build system'
    __ppe_run $argv
end

# Autocomplete function for the 'ppe' command
function __ppe_autocomplete
    # Skip the first parameter using array slicing
    set -f arguments (commandline -o)
    set -f arguments $arguments[2..-1]

    # Append "--" before completed arguments to avoir interpreting command flags specified by the user
    set -f --prepend arguments "--"

    # Detects if the cursor is at the end of a word or ar the start of a new one
    # Trailing spaces are removed from the command-line, so we have to rely on cursor position,
    # and explicitly tell ppe that we want to complete a new argument instead of the last one
    if __ppe_is_cursor_at_end_of_line
        set -f --prepend arguments "-CompleteArg"
    end

    # Memoize autocompletion results when called with same parameters more than once
    # This is a workaround: for some reason, fish (or the prompt) call this function so many times for 1 TAB press :'(
    if test "$__ppe_autocomplete_memoized_arguments" = "$arguments" -a "$__ppe_autocomplete_memoized_status" = "0"
        # Retrieve memoized completions
        set -f completions "$__ppe_autocomplete_memoized_completions"
    else
        # Call __ppe_run with 'autocomplete' command
        set -f completions (__ppe_run -Ide -q autocomplete -MaxResults=20 $arguments)

        set -g __ppe_autocomplete_memoized_status $status # record __ppe_run execution result to invalidate on error
        set -g __ppe_autocomplete_memoized_arguments $arguments
        set -g __ppe_autocomplete_memoized_completions $completions
    end

    # Output each completion on a separate line
    for completion in $completions
        echo -s "$completion"
    end
end

switch (uname)
    case 'MSYS*' 'Msys'
        set -g __ppe_executable_name 'PPE.exe'
    case 'Linux'
        set -g __ppe_executable_name 'PPE'
    case '*'
        set -g __ppe_executable_name 'PPE'
end

set -g __ppe_autocomplete_memoized_arguments
set -g __ppe_autocomplete_memoized_completions
set -g __ppe_autocomplete_memoized_status
complete -c ppe -f -a '(__ppe_autocomplete)' -d 'PoPpOlOpPoPpO build system'
# echo 'Enabled auto-completion for `ppe` PoPpOlOpPoPpO build system' 1>&2
