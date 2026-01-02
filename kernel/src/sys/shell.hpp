#ifndef KERNEL_SRC_SYS_SHELL_HPP_
#define KERNEL_SRC_SYS_SHELL_HPP_

#include <data_structures/array_structures.hpp>
#include <span.hpp>
#include <string.hpp>

#include "alkos/graphics_console.hpp"
#include "fs/vfs/path.hpp"
#include "io/stream.hpp"

namespace System
{

extern GraphicsConsole *g_active_console;

// --------------------------------------------------------------------------------
// Shell
// --------------------------------------------------------------------------------

class Shell
{
    public:
    explicit Shell(GraphicsConsole &console, IO::IReader &input_reader);

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

    // -------------------------------------------------------------------------
    // Command Handlers
    // -------------------------------------------------------------------------

    void CmdHelp();
    void CmdClear();
    void CmdEcho(std::string_view args);
    void CmdMem();
    void CmdCd(std::string_view args);
    void CmdLs(std::string_view args);
    void CmdCat(std::string_view args);
    void CmdExec(std::string_view args);
    void CmdPwd();

    // -------------------------------------------------------------------------
    // Path utilities
    // -------------------------------------------------------------------------

    vfs::Path ResolvePath(std::string_view path_str);

    // -------------------------------------------------------------------------
    // Internal Helpers
    // -------------------------------------------------------------------------

    void OnInput(char c);

    // Data
    GraphicsConsole &console_;
    IO::IReader &input_reader_;
    vfs::Path current_dir_{vfs::Path::kRoot};

    static constexpr size_t kMaxInput = 128;
    data_structures::StaticVector<char, kMaxInput> input_buffer_;
};

}  // namespace System

#endif  // KERNEL_SRC_SYS_SHELL_HPP_
