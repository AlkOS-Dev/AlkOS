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
    Path current_dir_{Path(std::string_view(&kPathSeparator, 1))};

    static constexpr size_t kMaxInput = 128;
    data_structures::StaticVector<char, kMaxInput> input_buffer_;
};

}  // namespace System

#endif  // USERSPACE_PROGRAMS_SHELL_SHELL_HPP_
