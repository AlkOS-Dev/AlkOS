#!/usr/bin/env bash
# Bash completion helper for AlkOS scripts

_comp_alkos_helper() {
    COMPREPLY=()
    local cur="${COMP_WORDS[COMP_CWORD]}"
    local prev="${COMP_WORDS[COMP_CWORD-1]}"

    # Return no completions if cursor is after help option
    if [[ "$prev" == "-h" || "$prev" == "--help" ]]; then
        return 0
    fi

    # Add help option to every command
    COMP_OPTIONS["h|help"]="h|help"
    COMP_DESCRIPTIONS["h|help"]="Show help message"
    COMP_TYPES["h|help"]="flag"

    # To track when to display descriptions
    local alt_display=1
    if [ "${__GENERATE_COMPLETIONS_PREV_LINE:-}" != "$COMP_LINE" ] ||
            [ "${__GENERATE_COMPLETIONS_PREV_POINT:-}" != "$COMP_POINT" ]; then
        __GENERATE_COMPLETIONS_PREV_LINE=$COMP_LINE
        __GENERATE_COMPLETIONS_PREV_POINT=$COMP_POINT
        alt_display=
    fi

    local -A short_opts long_opts
    for option in "${COMP_OPTIONS[@]}"; do
        if [[ "$option" == *"|"* ]]; then
            short_opts["$option"]="-$(echo "$option" | cut -d'|' -f1)"
            long_opts["$option"]="--$(echo "$option" | cut -d'|' -f2)"
        else
            if [[ ${#option} -eq 1 ]]; then
              short_opts["$option"]="-$option"
              long_opts["$option"]=""
            else
              short_opts["$option"]=""
              long_opts["$option"]="--$option"
            fi
        fi
    done

    # ----------------------------------------- #
    #          Handle options choices           #
    # ----------------------------------------- #
    for option in "${COMP_OPTIONS[@]}"; do
        local short="${short_opts[$option]}"
        local long="${long_opts[$option]}"

        if [[ "$prev" == "$short" || "$prev" == "$long" ]]; then
            if [[ "${COMP_TYPES[$option]}" == "string" && -n "${COMP_CHOICES[$option]}" ]]; then
                local -a choices_list=()
                IFS='|' read -ra choices_list <<< "${COMP_CHOICES[$option]}"

                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -W "${choices_list[*]}" -- "$cur") )
                return 0
            elif [[ "${COMP_TYPES[$option]}" == "list" && -n "${COMP_CHOICES[$option]}" ]]; then
                local separator="${COMP_SEPARATORS[$option]}"
                local -a choices_list=()
                IFS='|' read -ra choices_list <<< "${COMP_CHOICES[$option]}"

                # Extract the part after the last separator for current input
                local last_part="$cur" prefix=""
                [[ $cur =~ $separator ]] && last_part="${cur##*$separator}"; prefix="${cur%$last_part}"

                # Prevent bash from adding a space after completion if last_part is not exactly one of the choices
                if [[ ! " ${choices_list[*]} " =~ (^|[[:space:]])$last_part($|[[:space:]]) ]]; then
                    compopt -o nospace 2>/dev/null
                fi

                # shellcheck disable=SC2207
                COMPREPLY=( $(
                    if [[ -n "$alt_display" ]]; then
                        compgen -W "${choices_list[*]}" -- "$last_part"
                    else
                        compgen -W "${choices_list[*]}" -- "$last_part" | sed "s|^|$prefix|"
                    fi
                ) )
                return 0
            elif [[ "${COMP_TYPES[$option]}" == "path" ]]; then
                compopt -o filenames 2>/dev/null

                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -f -- "$cur") )
                return 0
            fi
        fi
    done

    # ----------------------------------------- #
    #     Track current positional argument     #
    # ----------------------------------------- #
    local pos_count=0
    local past_separator=false
    local prev_was_option_with_value=false
    for ((i=1; i<COMP_CWORD; i++)); do
        if [[ "${COMP_WORDS[i]}" == "--" ]]; then
            past_separator=true
            prev_was_option_with_value=false
        elif [[ "$prev_was_option_with_value" == true ]]; then
            # Previous word was an option that expects a value, so this is the value
            prev_was_option_with_value=false
        elif [[ "${COMP_WORDS[i]}" == -* ]] && [[ "$past_separator" == false ]]; then
            # Check if this option expects a value
            prev_was_option_with_value=false
            for option in "${COMP_OPTIONS[@]}"; do
                local short="${short_opts[$option]}"
                local long="${long_opts[$option]}"

                if [[ "${COMP_WORDS[i]}" == "$short" || "${COMP_WORDS[i]}" == "$long" ]]; then
                    if [[ "${COMP_TYPES[$option]}" != "flag" ]]; then
                        prev_was_option_with_value=true
                    fi
                    break
                fi
            done
        else
            # This is a positional argument
            ((pos_count++))
            prev_was_option_with_value=false
        fi
    done


    # ----------------------------------------- #
    #        Handle positional arguments        #
    # ----------------------------------------- #
    if [[ "$cur" != -* ]] || [[ "$past_separator" == true ]]; then
        if [[ $pos_count -lt ${#COMP_POSITIONAL_NAMES[@]} ]]; then
            local current_pos_name="${COMP_POSITIONAL_NAMES[$pos_count]}"

            if [[ -n "${COMP_CHOICES[$current_pos_name]}" ]]; then
                local -a choices_list=()
                IFS='|' read -ra choices_list <<< "${COMP_CHOICES[$current_pos_name]}"

                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -W "${choices_list[*]}" -- "$cur") )
                return 0
            fi

            # Handle file path completion for path type positional arguments
            if [[ "${COMP_TYPES[$current_pos_name]}" == "path" ]]; then
                compopt -o filenames 2>/dev/null

                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -f -- "$cur") )
                return 0
            fi
        fi
    fi

    # ----------------------------------------- #
    #     Handle options with descriptions      #
    # -----------------------------------------
    if [[ "$cur" == -* ]] && [[ "$past_separator" == false ]]; then
        local IFS=$'\n'
        local spliter='  '
        local -a matches=()

        # Calculate width for first column for options that match
        local max_width=0
        for option in "${COMP_OPTIONS[@]}"; do
            local short="${short_opts[$option]}"
            local long="${long_opts[$option]}"

            # Check if cur matches short or long option
            local short_match long_match
            [[ -n "$short" && "${short:0:${#cur}}" == "$cur" ]] && short_match=true || short_match=false
            [[ -n "$long" && "${long:0:${#cur}}" == "$cur" ]] && long_match=true || long_match=false

            # Skip if no match
            if [[ "$short_match" == false && "$long_match" == false ]]; then
                continue
            fi

            matches+=("$option")

            local len
            [[ $short_match == true ]] && len=${#short} || len=0
            [[ $short_match == true && $long_match == true ]] && (( len += 2 ))
            [[ $long_match == true ]] && (( len += ${#long}))
            (( ${#COMP_DESCRIPTIONS[$option]} > 0 )) && (( len += ${#COMP_DESCRIPTIONS[$option]} + 4 )) # 4 for "  ()"
            (( len > max_width )) && max_width=$len
        done

        # Calculate number of columns and column width
        local target_width=$(( COLUMNS * 6 / 10 ))
        (( max_width > target_width )) && target_width=$max_width
        local columns_number=$(( (target_width + ${#spliter}) / (max_width + ${#spliter}) )) # with 2 spaces padding
        local col_width=$(( (target_width - (${#spliter} * (columns_number - 1))) / columns_number ))

        # shellcheck disable=SC2207
        COMPREPLY=( $(
            local -a entries=()

            # Loop through matches and prepare display
            for match in "${matches[@]}"; do
                local short="${short_opts[$match]}"
                local long="${long_opts[$match]}"

                # Check if cur matches short and/or long option
                local short_match long_match
                [[ -n "$short" && "${short:0:${#cur}}" == "$cur" ]] && short_match=true
                [[ -n "$long" && "${long:0:${#cur}}" == "$cur" ]] && long_match=true

                local display
                if [[ $short_match == true ]] && [[ $long_match == true ]]; then
                    display="$short  $long"
                elif [[ $short_match == true ]]; then
                    display="$short"
                elif [[ $long_match == true ]]; then
                    display="$long"
                fi

                # Display with description when more than one completion is possible
                if [[ -n "$alt_display" ]]; then
                    local padding=$(( col_width - ${#display} - 2 ))
                    if [[ -n "${COMP_DESCRIPTIONS[$match]}" ]]; then
                        printf -v buffer "%s  %*s" "$display" "$padding" "(${COMP_DESCRIPTIONS[$match]})"
                    else
                        printf -v buffer "%-*s" "$col_width" "$display"
                    fi

                    entries+=("$buffer")
                else
                    [[ "$short_match" == true ]] && echo "$short" || echo "$long"
                fi
            done

            if [[ -n "$alt_display" ]]; then
                # Sort entries
                entries=($(sort <<< "${entries[*]}"))

                local line=""
                for (( i = 0; i < ${#entries[@]}; i++ )); do
                    line+="${entries[$i]}"
                    if (( (i + 1) % columns_number == 0 )); then
                        # Fill the rest of the line with spaces
                        local remaining=$(( COLUMNS - (col_width * columns_number + ${#spliter} * (columns_number - 1)) ))
                        if [[ $remaining -gt 0 ]]; then
                            line+=$(printf "%*s\n" "$remaining" "")
                        fi

                        echo -n "$line"
                        line=""
                    elif (( i + 1 == ${#entries[@]} )); then
                        # Last item, fill the rest of the line with spaces
                        local col_idx=$(( (i + 1) % columns_number ))
                        local remaining=$(( COLUMNS - (col_width * col_idx + ${#spliter} * (col_idx - 1)) ))
                        line+=$(printf "%*s\n" "$remaining" "")

                        echo -n "$line"
                        line=""
                    else
                        line+=$(printf "%s" "$spliter")
                    fi
                done
            fi
        ) )
    fi
}
