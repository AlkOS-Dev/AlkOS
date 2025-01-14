#ifndef KERNEL_ABI_TERMINA_HPP_
#define KERNEL_ABI_TERMINA_HPP_

#include <defines.h>
#include <todo.hpp>
#include <types.hpp>

TODO_BY_THE_END_OF_MILESTONE0
/* REWORK THAT */

extern "C" {
/**
 * @brief Initializes the terminal for input and output operations.
 * @note Must set up any required buffers or hardware state.
 */
void TerminalInit();

/**
 * @brief Outputs a single character to the terminal.
 * @param c The character to output.
 * @note Must ensure the character is displayed immediately.
 */
WRAP_CALL void TerminalPutChar(char c);

/**
 * @brief Writes a null-terminated string to the terminal.
 * @param data The string to write.
 * @note Must handle newlines and special characters appropriately. (TODO)
 */
WRAP_CALL void TerminalWriteString(const char *data);

/**
 * @brief Writes an error message to the terminal.
 * @param data The error string to write.
 * @note Should visually distinguish error messages (e.g., with a specific color or prefix).
 */
WRAP_CALL void TerminalWriteError(const char *data);

/**
 * @brief Reads a single character from the terminal input.
 * @return The character read from input.
 * @note Must block until a character is available.
 */
WRAP_CALL char TerminalGetChar();

/**
 * @brief Reads a line of input from the terminal into a buffer.
 * @param buffer The buffer to store the input.
 * @param size The maximum number of characters to read (including null terminator).
 * @return The number of characters read, including the null terminator.
 * @note Must block until a character is available.
 */
WRAP_CALL size_t TerminalReadLine(char *buffer, size_t size);
}

#include <abi/terminal.hpp>

#endif  // KERNEL_ABI_TERMINA_HPP_
