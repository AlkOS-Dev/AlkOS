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
    COMP_OPTIONS+=("h|help")
    COMP_DESCRIPTIONS["h|help"]="Show help message"
    COMP_TYPES["h|help"]="flag"

    # To track when to display descriptions, Bash sets COMP_TYPE to 9 when the user
    # has pressed tab twice on an incomplete option. In that case, we want to display
    # descriptions. Otherwise, we don't display them to avoid cluttering the output.
    # Since Bash 4.x only.
    local alt_display
    [[ $COMP_TYPE -eq 9 ]] && alt_display= || alt_display=1

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
            elif [[ "${COMP_TYPES[$option]}" == "file" ]]; then
                compopt -o filenames 2>/dev/null
                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -f -- "$cur") )
                return 0
            elif [[ "${COMP_TYPES[$option]}" == "directory" ]]; then
                compopt -o filenames 2>/dev/null
                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -d -- "$cur") )
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

            if [[ "${COMP_TYPES[$current_pos_name]}" == "file" ]]; then
                compopt -o filenames 2>/dev/null
                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -f -- "$cur") )
                return 0
            elif [[ "${COMP_TYPES[$current_pos_name]}" == "directory" ]]; then
                compopt -o filenames 2>/dev/null
                # shellcheck disable=SC2207
                COMPREPLY=( $(compgen -d -- "$cur") )
                return 0
            fi
        fi
    fi

    # ----------------------------------------- #
    #     Handle options with descriptions      #
    # -----------------------------------------
    if [[ "$cur" == -* ]] && [[ "$past_separator" == false ]]; then
        local IFS=$'\n'
        local spacer='  '
        local -a matches=()

        # Get matches
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
        done

        # Sort entries alphabetically
        matches=( $(sort <<< "${matches[*]}") )

        local target_width=$(( COLUMNS * 3 / 4 ))
        local total_matches=${#matches[@]}
        local columns_number=1
        local rows_per_column=$(( (total_matches + columns_number - 1) / columns_number ))

        local -a column_widths=()
        local -a sorted=()

        # Find optimal number of columns
        while true; do
            local rows_per_column_temp=$(( (total_matches + columns_number - 1) / columns_number ))
            local column_widths_temp=()
            local sorted_temp=()
            local total_width=0

            # Calculate width for each column
            for (( col = 0; col < columns_number; col++ )); do
                local max_col_width=0
                for (( row = 0; row < rows_per_column_temp; row++ )); do
                    local idx=$(( col * rows_per_column_temp + row ))
                    if [[ $idx -lt $total_matches ]]; then
                        local entry="${matches[idx]}"
                        sorted_temp+=("$entry")

                        local entry_width=0
                        entry_width+=${#short_opts[$entry]}
                        if [[ -n "${long_opts[$entry]}" ]]; then
                            # for the two spaces between short and long
                            (( entry_width > 0 )) && (( entry_width += 2 ))

                            (( entry_width += ${#long_opts[$entry]} ))
                        fi
                        if [[ -n "${COMP_DESCRIPTIONS[$entry]}" ]]; then
                            # Add 4 for the "  ()"
                            (( entry_width += 4 + ${#COMP_DESCRIPTIONS[$entry]} ))
                        fi
                        (( entry_width > max_col_width )) && max_col_width=$entry_width
                    fi
                done
                column_widths_temp+=("$max_col_width")
                (( total_width += max_col_width ))
            done

            # Add splitter width between columns
            (( total_width += ${#spacer} * (columns_number - 1) ))

            # Check if this configuration fits
            if (( total_width <= target_width && max_col_width != 0 )); then
                rows_per_column=$rows_per_column_temp
                column_widths=("${column_widths_temp[@]}")
                sorted=("${sorted_temp[@]}")

                (( columns_number++ ))
            else
                unset columns_numbers_temp
                unset column_widths_temp
                unset sorted_temp

                (( columns_number-- ))
                break
            fi
        done

        # Ensure we have at least 1 column
        (( columns_number < 1 )) && columns_number=1

        # shellcheck disable=SC2207
        COMPREPLY=( $(
            local -a to_display=()

            # Display with description
            for (( row = 0; row < rows_per_column; row++ )); do
                for (( col = 0; col < columns_number; col++ )); do
                    local idx=$(( col * rows_per_column + row ))
                    if [[ $idx -lt $total_matches ]]; then
                        local match="${sorted[idx]}"

                        local short="${short_opts[$match]}"
                        local long="${long_opts[$match]}"

                        # Check if cur matches short and/or long option
                        local short_match long_match
                        [[ -n "$short" && "${short:0:${#cur}}" == "$cur" ]] && short_match=true
                        [[ -n "$long" && "${long:0:${#cur}}" == "$cur" ]] && long_match=true

                        local option
                        if [[ $short_match == true ]] && [[ $long_match == true ]]; then
                            option="$short  $long"
                        elif [[ $short_match == true ]]; then
                            option="$short"
                        elif [[ $long_match == true ]]; then
                            option="$long"
                        fi

                        if [[ -n "$alt_display" ]]; then
                            if [[ -n "${COMP_DESCRIPTIONS[$match]}" ]]; then
                                local padding=$(( column_widths[col] - ${#option} - 2 ))
                                printf -v buffer "%s  %*s" "$option" "$padding" "(${COMP_DESCRIPTIONS[$match]})"
                            else
                                printf -v buffer "%-*s" "${column_widths[col]}" "$option"
                            fi

                            # Edge case: if user want to complete already completed option which
                            # is not the newest one, don't display it
                            if [[ "$cur" == "$short" || "$cur" == "$long" ]]; then
                                continue
                            fi

                            to_display+=("$buffer")
                        else
                            [[ "$short_match" == true ]] && echo "$short" || echo "$long"
                        fi
                    fi
                done
            done

            if [[ -n "$alt_display" ]]; then
                # Fill the rest of the line with spaces and add spacers between columns
                local line=""
                for (( i = 0; i < ${#to_display[@]}; i++ )); do
                    line+="${to_display[i]}"
                    if (( (i + 1) % columns_number == 0 || i == ${#to_display[@]} - 1 )); then
                        # Fill the rest of the line with spaces
                        local remaining=$(( COLUMNS - ${#line} ))
                        if [[ $remaining -gt 0 ]]; then
                            line+=$(printf "%*s\n" "$remaining" "")
                        fi

                        echo -n "$line"
                        line=""
                    else
                        line+=$(printf "%s" "$spacer")
                    fi
                done
            fi
        ) )
    fi
}
