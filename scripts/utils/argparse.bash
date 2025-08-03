#!/bin/bash
declare -A ARGPARSE_OPTIONS=()
declare -A ARGPARSE_POSITIONAL=()
declare -A ARGPARSE_VALUES=()
declare -A ARGPARSE_DESCRIPTIONS=()
declare -A ARGPARSE_REQUIRED=()
declare -A ARGPARSE_DEFAULTS=()
declare -A ARGPARSE_CHOICES=()
declare -A ARGPARSE_TYPES=()
ARGPARSE_HELP_FUNCTION=""
ARGPARSE_SCRIPT_NAME=""
ARGPARSE_SCRIPT_DESCRIPTION=""
ARGPARSE_POSITIONAL_NAMES=()

# Initialize the argument parser
# Usage: argparse_init "script_name" "description" [help_function]
argparse_init() {
    ARGPARSE_SCRIPT_NAME="${1:-$(basename "$0")}"
    ARGPARSE_SCRIPT_DESCRIPTION="$2"
    ARGPARSE_HELP_FUNCTION="$3"

    # Clear previous state
    ARGPARSE_OPTIONS=()
    ARGPARSE_POSITIONAL=()
    ARGPARSE_VALUES=()
    ARGPARSE_DESCRIPTIONS=()
    ARGPARSE_REQUIRED=()
    ARGPARSE_DEFAULTS=()
    ARGPARSE_CHOICES=()
    ARGPARSE_TYPES=()
    ARGPARSE_POSITIONAL_NAMES=()
}

# Add an option argument
# Usage: argparse_add_option "short|long" "description" [required] [default] [choices] [type]
argparse_add_option() {
    local option="$1"
    local description="$2"
    local required="${3:-false}"
    local default="$4"
    local choices="$5"
    local type="${6:-string}"

    ARGPARSE_OPTIONS["$option"]="$option"
    ARGPARSE_DESCRIPTIONS["$option"]="$description"
    ARGPARSE_REQUIRED["$option"]="$required"
    ARGPARSE_DEFAULTS["$option"]="$default"
    ARGPARSE_CHOICES["$option"]="$choices"
    ARGPARSE_TYPES["$option"]="$type"

    # Set default value if provided
    [[ -n "$default" ]] && ARGPARSE_VALUES["$option"]="$default"
}

# Add a positional argument
# Usage: argparse_add_positional "name" "description" [required] [choices]
argparse_add_positional() {
    local name="$1"
    local description="$2"
    local required="${3:-true}"
    local choices="$4"

    ARGPARSE_POSITIONAL["$name"]="$name"
    ARGPARSE_DESCRIPTIONS["$name"]="$description"
    ARGPARSE_REQUIRED["$name"]="$required"
    ARGPARSE_CHOICES["$name"]="$choices"
    ARGPARSE_POSITIONAL_NAMES+=("$name")
}

# Get the value of an argument
# Usage: argparse_get "option_or_positional_name"
argparse_get() {
    local key="$1"
    echo "${ARGPARSE_VALUES[$key]}"
}

# Check if an argument was provided
# Usage: argparse_is_set "option_or_positional_name"
argparse_is_set() {
    local key="$1"
    [[ -n "${ARGPARSE_VALUES[$key]}" ]]
}

argparse_show_help() {
    # Build usage line: script positionals options
    local usage_line="$ARGPARSE_SCRIPT_NAME"

    # Add positional arguments first
    for pos_name in "${ARGPARSE_POSITIONAL_NAMES[@]}"; do
        if [[ "${ARGPARSE_REQUIRED[$pos_name]}" == "true" ]]; then
            usage_line+=" <$pos_name>"
        else
            usage_line+=" [$pos_name]"
        fi
    done

    # Add options to usage line
    for option in "${!ARGPARSE_OPTIONS[@]}"; do
        local option_display=""
        local required="${ARGPARSE_REQUIRED[$option]}"
        local type="${ARGPARSE_TYPES[$option]}"

        # Format option for usage line
        if [[ "$option" == *"|"* ]]; then
            # Handle short|long format
            local short=$(echo "$option" | cut -d'|' -f1)
            local long=$(echo "$option" | cut -d'|' -f2)
            if [[ "$type" == "flag" ]]; then
                option_display="[--$long | -$short]"
            else
                option_display="[-$short <${short^^}> | --$long <${long^^}>]"
            fi
        else
            # Single option
            if [[ "$type" == "flag" ]]; then
                option_display="[-$option]"
            else
                option_display="[-$option <${option^^}>]"
            fi
        fi

        # Make required options not bracketed
        if [[ "$required" == "true" ]]; then
            option_display="${option_display//\[/}"
            option_display="${option_display//\]/}"
            option_display="<${option_display}>"
        fi

        usage_line+=" $option_display"
    done

    echo "$usage_line"
    echo ""
    echo "Description:"
    echo -e "\t$ARGPARSE_SCRIPT_DESCRIPTION"
    echo ""
    echo "Positional arguments:"

    # Show positional arguments descriptions
    for pos_name in "${ARGPARSE_POSITIONAL_NAMES[@]}"; do
        local desc="${ARGPARSE_DESCRIPTIONS[$pos_name]}"
        local choices="${ARGPARSE_CHOICES[$pos_name]}"
        local required="${ARGPARSE_REQUIRED[$pos_name]}"

        local mandatory_text=""
        [[ "$required" == "true" ]] && mandatory_text=" - MANDATORY"

        echo -e "\t<$pos_name>$mandatory_text - $desc"
        [[ -n "$choices" ]] && echo -e "\t\tSupported: $choices"
    done

    echo ""
    echo "Options:"

    # Show options descriptions
    for option in "${!ARGPARSE_OPTIONS[@]}"; do
        local desc="${ARGPARSE_DESCRIPTIONS[$option]}"
        local default="${ARGPARSE_DEFAULTS[$option]}"
        local choices="${ARGPARSE_CHOICES[$option]}"
        local required="${ARGPARSE_REQUIRED[$option]}"
        local type="${ARGPARSE_TYPES[$option]}"

        # Format option display for description
        local opt_display=""
        if [[ "$option" == *"|"* ]]; then
            local short=$(echo "$option" | cut -d'|' -f1)
            local long=$(echo "$option" | cut -d'|' -f2)
            if [[ "$type" == "flag" ]]; then
                opt_display="--$long | -$short"
            else
                opt_display="-$short <${short^^}> | --$long <${long^^}>"
            fi
        else
            if [[ "$type" == "flag" ]]; then
                opt_display="-$option"
            else
                opt_display="-$option <${option^^}>"
            fi
        fi

        echo -e "\t\"$opt_display\" - $desc"
        [[ -n "$default" ]] && echo -e "\tDefault: $default"
        [[ -n "$choices" ]] && echo -e "\tSupported: $choices"
    done

    # Call custom help function if provided
    [[ -n "$ARGPARSE_HELP_FUNCTION" && $(type -t "$ARGPARSE_HELP_FUNCTION") == "function" ]] && "$ARGPARSE_HELP_FUNCTION"
}

# Validate choice
argparse_validate_choice() {
    local key="$1"
    local value="$2"
    local choices="${ARGPARSE_CHOICES[$key]}"

    [[ -z "$choices" ]] && return 0

    local IFS='|'
    local choice_array=($choices)
    for choice in "${choice_array[@]}"; do
        [[ "$value" == "$choice" ]] && return 0
    done

    dump_error "Invalid value '$value' for $key. Valid choices: $choices"
}

argparse_parse() {
    local positional_index=0

    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                argparse_show_help
                exit 0
                ;;
            -*)
                local found_option=""
                local option_key=""

                # Find matching option
                for option in "${!ARGPARSE_OPTIONS[@]}"; do
                    local IFS='|'
                    local option_variants=($option)
                    for variant in "${option_variants[@]}"; do
                        if [[ "$1" == "-$variant" || "$1" == "--$variant" ]]; then
                            found_option="$option"
                            option_key="$option"
                            break 2
                        fi
                    done
                done

                if [[ -z "$found_option" ]]; then
                    dump_error "Unknown option: $1"
                fi

                # Check if option expects a value
                local option_type="${ARGPARSE_TYPES[$option_key]}"
                if [[ "$option_type" == "flag" ]]; then
                    ARGPARSE_VALUES["$option_key"]="true"
                    shift
                else
                    if [[ $# -lt 2 ]]; then
                        dump_error "Option $1 requires a value"
                    fi
                    argparse_validate_choice "$option_key" "$2"
                    ARGPARSE_VALUES["$option_key"]="$2"
                    shift 2
                fi
                ;;
            *)
                # Handle positional arguments
                if [[ $positional_index -lt ${#ARGPARSE_POSITIONAL_NAMES[@]} ]]; then
                    local pos_name="${ARGPARSE_POSITIONAL_NAMES[$positional_index]}"
                    argparse_validate_choice "$pos_name" "$1"
                    ARGPARSE_VALUES["$pos_name"]="$1"
                    ((positional_index++))
                    shift
                else
                    dump_error "Unknown argument: $1"
                fi
                ;;
        esac
    done

    # Check required arguments
    for key in "${!ARGPARSE_REQUIRED[@]}"; do
        if [[ "${ARGPARSE_REQUIRED[$key]}" == "true" && -z "${ARGPARSE_VALUES[$key]}" ]]; then
            if [[ -n "${ARGPARSE_POSITIONAL[$key]}" ]]; then
                dump_error "Required positional argument '$key' not provided"
            else
                dump_error "Required option '$key' not provided"
            fi
        fi
    done
}

# Convenience function to set up common patterns
# Usage: argparse_quick_setup "script_name" "description"
argparse_quick_setup() {
    argparse_init "$1" "$2"
    argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
    argparse_add_option "r|run" "Expected to run the script" true "" "" "flag"
}
