#include "shell.hpp"

#include <stdio.h>
#include <string.h>
#include <string.hpp>

#include <alkos/calls.h>

static u64 ParsePid(const std::string_view str)
{
    unsigned long long res = 0;
    for (const char c : str) {
        if (c < '0' || c > '9') {
            return 0;
        }
        res = res * 10 + (c - '0');
    }
    return res;
}

namespace System
{

Shell::Shell(GraphicsConsole &console) : console_(console) {}

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
    char c;
    auto read = fread(&c, 1, 1, stdin);
    if (read != 1) {
        return;  // No input
    }
    OnInput(c);
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

    // Trim leading/trailing spaces
    while (!args.empty() && args.front() == ' ') {
        args.remove_prefix(1);
    }
    while (!args.empty() && args.back() == ' ') {
        args.remove_suffix(1);
    }

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
        if (args == "&") {
            CmdExecAsync(cmd.substr(2));
        } else {
            CmdExec(cmd.substr(2));
        }
    } else if (cmd == "exec") {
        CmdExec(args);
    } else if (cmd == "exec_async") {
        CmdExecAsync(args);
    } else if (cmd == "shutdown") {
        Shutdown();
    } else if (cmd == "reboot") {
        Reboot();
    } else if (cmd == "kill") {
        CmdKill(args);
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
        "  ./<file> &  - Execute a program asynchronously\n"
        "  kill <pid>  - Kill a running process by PID\n"
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
    FileInfo info;
    int result = GetFileInfo(new_path.CString(), &info);

    if (result != 0) {
        const char *err = "cd: path not found: ";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size())
        );
        console_.PutChar('\n');
        return;
    }

    if (info.type != kFileTypeDirectory) {
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

    // Allocate buffer for directory entries
    static constexpr size_t kMaxEntries = 128;
    DirEntry entries[kMaxEntries];
    size_t num_entries = 0;

    // Call ReadDirectory syscall
    int result = ReadDirectory(path.CString(), entries, kMaxEntries, &num_entries);

    if (result != 0) {
        const char *err = "ls: cannot access '";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        const char *p = path.CString();
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(p), strlen(p)));
        const char *err2 = "': No such directory\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err2), strlen(err2)));
        return;
    }

    // Display entries
    for (size_t i = 0; i < num_entries; i++) {
        const auto &entry = entries[i];

        // Color directories blue
        if (entry.type == kFileTypeDirectory) {
            console_.SetColors(Graphics::Color::Blue(), Graphics::Color::Black());
        }

        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(entry.name), strlen(entry.name))
        );

        if (entry.type == kFileTypeDirectory) {
            console_.PutChar('/');
            console_.SetColors(Graphics::Color::White(), Graphics::Color::Black());
        }

        console_.PutChar('\n');
    }
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

    // Open file for reading
    FILE *file = fopen(path.CString(), "r");
    if (!file) {
        const char *err = "cat: ";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        console_.Write(
            std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size())
        );
        const char *err2 = ": No such file\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err2), strlen(err2)));
        return;
    }

    // Read and display file contents in chunks
    static constexpr size_t kBufferSize = 512;
    char buffer[kBufferSize];

    while (true) {
        size_t bytes_read = fread(buffer, 1, kBufferSize, file);
        if (bytes_read == 0) {
            if (ferror(file)) {
                const char *err = "\ncat: error reading file\n";
                console_.Write(
                    std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err))
                );
            }
            break;
        }

        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(buffer), bytes_read));
    }

    fclose(file);

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

    Path programPath = ResolvePath(args);
    const u64 pid    = Exec(programPath.CString());

    if (pid == 0) {
        const char *err = "exec: invalid filename or not enough resources\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    int result = Wait(pid);
    if (result == std::numeric_limits<int>::max()) {
        const char *err = "exec: failed during waiting unknown issue\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    if (result != 0) {
        const char *err = "exec: process failed with status:";
        char buff[128];
        snprintf(buff, 128, "%d", result);

        const char *msg = "Process failed with status ";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(msg), strlen(msg)));
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(buff), strlen(buff)));
        console_.PutChar('\n');
        return;
    }

    char buff[128];
    snprintf(buff, 128, "%llu", pid);

    const char *msg = "Correctly processed process with PID: ";
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(msg), strlen(msg)));
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(buff), strlen(buff)));
    console_.PutChar('\n');
}

void Shell::CmdExecAsync(std::string_view args)
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

    Path programPath = ResolvePath(args);
    const u64 pid    = ExecAsync(programPath.CString());

    if (pid == 0) {
        const char *err = "exec: invalid filename or not enough resources\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    char buff[128];
    snprintf(buff, 128, "%llu", pid);

    const char *msg = "Started process in background. PID: ";
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(msg), strlen(msg)));
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(buff), strlen(buff)));
    console_.PutChar('\n');
}

void Shell::CmdKill(std::string_view args)
{
    // Trim leading/trailing spaces
    while (!args.empty() && args.front() == ' ') {
        args.remove_prefix(1);
    }
    while (!args.empty() && args.back() == ' ') {
        args.remove_suffix(1);
    }

    if (args.empty()) {
        const char *err = "kill: missing PID operand\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    const u64 pid = ParsePid(args);

    if (pid == 0) {
        const char *err = "kill: PID must be unsigned integer\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    const int result = Kill(pid);

    if (result) {
        const char *err = "kill: failed to kill process...\n";
        console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
        return;
    }

    const char *err = "Successfully killed the process...\n";
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(err), strlen(err)));
}
}  // namespace System
