// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef USERSPACE_PROGRAMS_SHELL_SHELL_HPP_
#define USERSPACE_PROGRAMS_SHELL_SHELL_HPP_

#include <data_structures/array_structures.hpp>
#include <span.hpp>
#include <string.hpp>

#include "graphics_console.hpp"
#include "path.hpp"

namespace System
{

// --------------------------------------------------------------------------------
// Shell
// --------------------------------------------------------------------------------

class Shell
{
    public:
    explicit Shell(GraphicsConsole &console);

    // -------------------------------------------------------------------------
    // Public Interface
    // -------------------------------------------------------------------------

    void Init();

    /**
     * @brief Polling function to check for new input and process it.
     * Should be called inside the main kernel loop.
     */
    void Update();

    private:
    // -------------------------------------------------------------------------
    // Command Processing
    // -------------------------------------------------------------------------

    void ProcessCommand();
    void PrintPrompt();
    void PrintWelcome();
    void WriteCStr(const char *str);

    // -------------------------------------------------------------------------
    // Command Handlers
    // -------------------------------------------------------------------------

    void CmdHelp();
    void CmdClear();
    void CmdEcho(std::string_view args);
    void CmdCd(std::string_view args);
    void CmdLs(std::string_view args);
    void CmdCat(std::string_view args);
    void CmdExec(std::string_view args);
    void CmdExecAsync(std::string_view args);
    void CmdKill(std::string_view args);
    void CmdPwd();

    // -------------------------------------------------------------------------
    // Path utilities
    // -------------------------------------------------------------------------

    Path ResolvePath(std::string_view path_str);

    // -------------------------------------------------------------------------
    // Internal Helpers
    // -------------------------------------------------------------------------

    void OnInput(char c);

    // Data
    GraphicsConsole &console_;
    Path current_dir_{Path::kRoot};

    static constexpr size_t kMaxInput = 128;
    data_structures::StaticVector<char, kMaxInput> input_buffer_;
};

}  // namespace System

#endif  // USERSPACE_PROGRAMS_SHELL_SHELL_HPP_
