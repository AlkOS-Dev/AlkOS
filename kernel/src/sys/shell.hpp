#ifndef KERNEL_SRC_SYS_SHELL_HPP_
#define KERNEL_SRC_SYS_SHELL_HPP_

#include <data_structures/array_structures.hpp>
#include <span.hpp>
#include <string.hpp>

#include "sys/graphics_console.hpp"

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

    // Call this when hardware receives a keystroke
    void OnInput(char c);

    private:
    // -------------------------------------------------------------------------
    // Command Processing
    // -------------------------------------------------------------------------

    void ProcessCommand();
    void PrintPrompt();

    // -------------------------------------------------------------------------
    // Command Handlers
    // -------------------------------------------------------------------------

    void CmdHelp();
    void CmdClear();
    void CmdEcho(std::string_view args);
    void CmdMem();

    // Data
    GraphicsConsole &console_;

    static constexpr size_t kMaxInput = 128;
    data_structures::StaticVector<char, kMaxInput> input_buffer_;
};

}  // namespace System

#endif  // KERNEL_SRC_SYS_SHELL_HPP_
