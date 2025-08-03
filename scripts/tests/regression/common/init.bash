#!/bin/bash

REGRESSION_INIT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${REGRESSION_INIT_DIR}/../../../utils/conf_handlers.bash"
source "${REGRESSION_INIT_DIR}/../../../utils/helpers.bash"

init() {
  local conf_name="$1"
  local conf_arch="$2"

  pretty_success "Initializing regression environment for ${conf_name} configuration and ${conf_arch} architecture"

  local conf_arg="${conf_arch} ${conf_name} ${VERBOSE_FLAG}"

  # Reuse the conf file if it exists
  source_conf_file
  if is_conf_file_present ; then
    pretty_info "Reusing old configuration..."

    verify_conf_var_exists "CONF_BUILD_DIR"
    verify_conf_var_exists "CONF_TOOL_DIR"

    conf_arg="${conf_arg} -b ${CONF_BUILD_DIR}"
    conf_arg="${conf_arg} -t ${CONF_TOOL_DIR}"
  fi

  pretty_info "Reconfiguring build environment..."
  base_runner "init" true "${REGRESSION_INIT_DIR}/../../../config/configure.bash" ${conf_arg}
  pretty_success "Build environment reconfigured"

  pretty_info "Rebuilding the project..."
  base_runner "init" true "${REGRESSION_INIT_DIR}/../../../actions/build_alkos.bash" ${VERBOSE_FLAG}
  pretty_success "Project rebuilt"

  pretty_success "Regression environment initialized"
}
