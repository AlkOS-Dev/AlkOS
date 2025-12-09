#include "sys/shell.hpp"
#include <string.h>
#include <modules/memory.hpp>
#include <string.hpp>

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
        "  help   - Show this message\n"
        "  clear  - Clear the screen\n"
        "  echo   - Print arguments\n";
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(msg), strlen(msg)));
}

void Shell::CmdClear() { console_.Clear(); }

void Shell::CmdEcho(std::string_view args)
{
    console_.Write(std::span<const byte>(reinterpret_cast<const byte *>(args.data()), args.size()));
    console_.PutChar('\n');
}

}  // namespace System
