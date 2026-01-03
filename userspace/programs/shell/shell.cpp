#include "shell.hpp"

#include <string.h>
#include <string.hpp>

#include "alkos/sys/power.h"

namespace System
{

GraphicsConsole *g_active_console = nullptr;

Shell::Shell(GraphicsConsole &console, IO::IReader &input_reader)
    : console_(console), input_reader_(input_reader)
{
    // !!! TEMPORARY !!!
    g_active_console = &console_;
    // !!! TEMPORARY !!!
}

void Shell::Init()
{
    console_.Write(
        std::span<const byte>(reinterpret_cast<const byte *>("Welcome to AlkOS Shell!\n"), 24)
    );
    PrintPrompt();
}

void Shell::PrintPrompt()
{
    console_.SetColors(Graphics::Color::Green(), Graphics::Color::Black());
    const char *p = "AlkOS> ";
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(p), strlen(p)));
    console_.SetColors(Graphics::Color::White(), Graphics::Color::Black());
}

void Shell::Update()
{
    auto result = input_reader_.GetChar();
    if (result.has_value()) {
        OnInput(result.value());
    }
}

void Shell::OnInput(char c)
{
    // Handle Special Keys
    if (c == '\n' || c == '\r') {
        console_.PutChar('\n');
        ProcessCommand();
        input_buffer_.Resize(0);  // Clear buffer
        PrintPrompt();
        return;
    }

    if (c == '\b' || c == 0x7F) {  // Backspace
        if (input_buffer_.Size() > 0) {
            input_buffer_.Pop();
            console_.PutChar('\b');
        }
        return;
    }

    // Normal character
    if (input_buffer_.Size() < kMaxInput) {
        input_buffer_.Push(c);
        console_.PutChar(c);
    }
}

void Shell::ProcessCommand()
{
    if (input_buffer_.Size() == 0) {
        return;
    }

    std::string_view line(static_cast<const char *>(input_buffer_.Data()), input_buffer_.Size());

    // Split command and args
    size_t space_pos     = line.find(' ');
    std::string_view cmd = (space_pos == std::string_view::npos) ? line : line.substr(0, space_pos);
    std::string_view args = (space_pos == std::string_view::npos) ? "" : line.substr(space_pos + 1);

    if (cmd == "help") {
        CmdHelp();
    } else if (cmd == "clear") {
        CmdClear();
    } else if (cmd == "echo") {
        CmdEcho(args);
    } else if (cmd == "cd") {
        CmdCd(args);
    } else if (cmd == "ls") {
        CmdLs(args);
    } else if (cmd == "cat") {
        CmdCat(args);
    } else if (cmd == "pwd") {
        CmdPwd();
    } else if (cmd.starts_with("./")) {
        CmdExec(cmd.substr(2));
    } else if (cmd == "exec") {
        CmdExec(args);
    } else if (cmd == "shutdown") {
        Shutdown();
    } else if (cmd == "reboot") {
        Reboot();
    } else {
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>("Unknown command: "), 17)
        );
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(cmd.data()), cmd.size())
        );
        console_.PutChar('\n');
    }
}

void Shell::CmdHelp()
{
    const char *msg =
        "Available commands:\n"
        "  help        - Show this message\n"
        "  clear       - Clear the screen\n"
        "  echo <text> - Print arguments\n"
        "  pwd         - Print working directory\n"
        "  cd <path>   - Change directory\n"
        "  ls [path]   - List directory contents\n"
        "  cat <file>  - Display file contents\n"
        "  exec <file> - Execute a user program\n"
        "  ./<file>    - Execute a user program\n"
        "  shutdown    - Shutdown the system\n"
        "  reboot      - Reboot the system\n";
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(msg), strlen(msg)));
}

void Shell::CmdClear() { console_.Clear(); }

void Shell::CmdEcho(std::string_view args)
{
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size()));
    console_.PutChar('\n');
}

Path Shell::ResolvePath(std::string_view path_str)
{
    if (path_str.empty()) {
        return current_dir_;
    }

    Path path(path_str);
    if (path.IsAbsolute()) {
        return path.GetNormalized();
    }

    // Relative path - combine with current directory
    return (current_dir_ / path).GetNormalized();
}

void Shell::CmdPwd()
{
    const char *path = current_dir_.CString();
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(path), strlen(path)));
    console_.PutChar('\n');
}

void Shell::CmdCd(std::string_view args)
{
    // Trim leading/trailing spaces
    while (!args.empty() && args.front() == ' ') {
        args.remove_prefix(1);
    }
    while (!args.empty() && args.back() == ' ') {
        args.remove_suffix(1);
    }

    if (args.empty()) {
        // cd with no args goes to root
        current_dir_ = Path::kRoot;
        return;
    }

    Path new_path = ResolvePath(args);

    // Check if path exists and is a directory
    auto &vfs         = VfsModule::Get();
    auto exists_check = vfs.DirectoryExists(new_path);

    if (!exists_check.has_value()) {
        const char *err = "cd: path not found: ";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size())
        );
        console_.PutChar('\n');
        return;
    }

    if (!exists_check.value()) {
        const char *err = "cd: not a directory: ";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size())
        );
        console_.PutChar('\n');
        return;
    }

    current_dir_ = new_path;
}

void Shell::CmdLs(std::string_view args)
{
    // Trim leading/trailing spaces
    while (!args.empty() && args.front() == ' ') {
        args.remove_prefix(1);
    }
    while (!args.empty() && args.back() == ' ') {
        args.remove_suffix(1);
    }

    Path path = args.empty() ? current_dir_ : ResolvePath(args);

    auto &vfs = VfsModule::Get();

    // Check if the path exists
    auto exists_check = vfs.DirectoryExists(path);
    if (!exists_check.has_value() || !exists_check.value()) {
        const char *err = "ls: cannot access '";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        const char *p = path.CString();
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(p), strlen(p)));
        const char *err2 = "': No such directory\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err2), strlen(err2)));
        return;
    }

    // List directory contents
    vfs.ListDirectory(path, [this](const char *name, bool is_dir) {
        if (is_dir) {
            console_.SetColors(Graphics::Color::Blue(), Graphics::Color::Black());
        }
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(name), strlen(name)));
        if (is_dir) {
            console_.PutChar('/');
            console_.SetColors(Graphics::Color::White(), Graphics::Color::Black());
        }
        console_.PutChar('\n');
    });
}

void Shell::CmdCat(std::string_view args)
{
    // Trim leading/trailing spaces
    while (!args.empty() && args.front() == ' ') {
        args.remove_prefix(1);
    }
    while (!args.empty() && args.back() == ' ') {
        args.remove_suffix(1);
    }

    if (args.empty()) {
        const char *err = "cat: missing file operand\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    Path path = ResolvePath(args);

    auto &vfs = VfsModule::Get();

    // Check if file exists
    auto exists_check = vfs.FileExists(path);
    if (!exists_check.has_value() || !exists_check.value()) {
        const char *err = "cat: ";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size())
        );
        const char *err2 = ": No such file\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err2), strlen(err2)));
        return;
    }

    // Get file size
    auto size_result = vfs.GetFileSize(path);
    if (!size_result.has_value()) {
        const char *err = "cat: cannot read file size\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    size_t file_size = size_result.value();
    if (file_size == 0) {
        return;  // Empty file, nothing to print
    }

    // Read and display file contents in chunks
    static constexpr size_t kBufferSize = 512;
    char buffer[kBufferSize];
    size_t offset = 0;

    while (offset < file_size) {
        size_t to_read   = (file_size - offset) < kBufferSize ? (file_size - offset) : kBufferSize;
        auto read_result = vfs.ReadFile(path, buffer, to_read, offset);

        if (!read_result.has_value()) {
            const char *err = "\ncat: error reading file\n";
            console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
            return;
        }

        size_t bytes_read = read_result.value();
        if (bytes_read == 0) {
            break;
        }

        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(buffer), bytes_read));
        offset += bytes_read;
    }

    // Ensure we end with a newline
    console_.PutChar('\n');
}

void Shell::CmdExec(std::string_view args)
{
    // Trim leading/trailing spaces
    while (!args.empty() && args.front() == ' ') {
        args.remove_prefix(1);
    }
    while (!args.empty() && args.back() == ' ') {
        args.remove_suffix(1);
    }

    if (args.empty()) {
        const char *err = "exec: missing file operand\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    // Path programPath = ResolvePath(args);
    //
    // auto &as       = MemoryModule::Get().GetKernelAddressSpace();
    // auto entry_res = ElfLoader::Load(programPath, as);
    //
    // if (entry_res) {
    //     console_.Write(
    //         std::span<const byte>(reinterpret_cast<const byte *>("Executing user program...\n"), 26)
    //     );
    //
    //     // Execute the program
    //
    //
    //     console_.Write(
    //         std::span<const byte>(reinterpret_cast<const byte *>("User program returned.\n"), 23)
    //     );
    // } else {
    //     console_.Write(
    //         std::span<const byte>(
    //             reinterpret_cast<const byte *>("Failed to load executable.\n"), 27
    //         )
    //     );
    // }
}

}  // namespace System
