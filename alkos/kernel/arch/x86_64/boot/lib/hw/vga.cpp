#include <string.h>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

#include "hw/vga.hpp"

/**
 * @file vga.cpp
 * @brief Implementation of VGA text mode terminal driver
 */

// ------------------------------
// Types and variables
// ------------------------------

/**
 * @name VGA Hardware Text Mode Colors
 * Standard 16-color VGA text mode palette
 * @{
 */
enum VgaColor {
    VGA_COLOR_BLACK         = 0,   ///< Standard black
    VGA_COLOR_BLUE          = 1,   ///< Standard blue
    VGA_COLOR_GREEN         = 2,   ///< Standard green
    VGA_COLOR_CYAN          = 3,   ///< Standard cyan
    VGA_COLOR_RED           = 4,   ///< Standard red
    VGA_COLOR_MAGENTA       = 5,   ///< Standard magenta
    VGA_COLOR_BROWN         = 6,   ///< Standard brown
    VGA_COLOR_LIGHT_GREY    = 7,   ///< Standard light grey
    VGA_COLOR_DARK_GREY     = 8,   ///< Standard dark grey
    VGA_COLOR_LIGHT_BLUE    = 9,   ///< Bright blue
    VGA_COLOR_LIGHT_GREEN   = 10,  ///< Bright green
    VGA_COLOR_LIGHT_CYAN    = 11,  ///< Bright cyan
    VGA_COLOR_LIGHT_RED     = 12,  ///< Bright red
    VGA_COLOR_LIGHT_MAGENTA = 13,  ///< Bright magenta
    VGA_COLOR_LIGHT_BROWN   = 14,  ///< Yellow (bright brown)
    VGA_COLOR_WHITE         = 15,  ///< Bright white
};
/** @} */

/**
 * @name VGA Display Constants
 * Standard VGA text mode dimensions
 * @{
 */
static constexpr size_t kVgaWidth  = 80;  ///< Standard VGA text mode width
static constexpr size_t kVgaHeight = 25;  ///< Standard VGA text mode height
/** @} */

/**
 * @name Terminal State Variables
 * Global state tracking for the terminal
 * @{
 */
static size_t g_TerminalRow;        ///< Current cursor row position
static size_t g_TerminalColumn;     ///< Current cursor column position
static uint8_t g_TerminalColor;     ///< Current text color attributes
static uint16_t *g_TerminalBuffer;  ///< Pointer to VGA memory buffer (0xB8000)
/** @} */

/**
 * @brief Create a VGA color attribute byte
 *
 * @param fg Foreground color
 * @param bg Background color
 * @return Combined color attribute byte
 *
 * Combines foreground and background colors into a single byte
 * where the background color is in the high nibble.
 */
static FORCE_INLINE_F uint8_t VgaEntryColor(const VgaColor fg, const VgaColor bg)
{
    return fg | bg << 4;
}

/**
 * @brief Create a VGA character entry
 *
 * @param uc Character to display
 * @param color Color attribute byte
 * @return Combined VGA character entry
 *
 * Creates a 16-bit VGA character entry with the character in the low byte
 * and the color attribute in the high byte.
 */
static FORCE_INLINE_F uint16_t VgaEntry(const unsigned char uc, const uint8_t color)
{
    return static_cast<uint16_t>(uc) | static_cast<uint16_t>(color) << 8;
}

/**
 * @brief Initialize the VGA text mode terminal
 *
 * Sets up a 80x25 text mode display with light grey text on black background.
 * Clears the screen and positions the cursor at (0,0).
 */
void VgaTerminalInit()
{
    g_TerminalRow    = 0;
    g_TerminalColumn = 0;
    g_TerminalColor  = VgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    g_TerminalBuffer = reinterpret_cast<uint16_t *>(0xB8000);

    for (size_t y = 0; y < kVgaHeight; y++) {
        for (size_t x = 0; x < kVgaWidth; x++) {
            const size_t index      = y * kVgaWidth + x;
            g_TerminalBuffer[index] = VgaEntry(' ', g_TerminalColor);
        }
    }
}

/**
 * @brief Set the current text color
 *
 * @param color Combined foreground and background color value
 *              (foreground in lower 4 bits, background in upper 4 bits)
 */
void VgaTerminalSetColor(const uint8_t color) { g_TerminalColor = color; }

/**
 * @brief Write a character with specified color at given coordinates
 *
 * @param c Character to display
 * @param color Color attribute for the character
 * @param x X-coordinate (0 to 79)
 * @param y Y-coordinate (0 to 24)
 */
void VgaTerminalPutEntryAt(const char c, const uint8_t color, const size_t x, const size_t y)
{
    const size_t index      = y * kVgaWidth + x;
    g_TerminalBuffer[index] = VgaEntry(c, color);
}

/**
 * @brief Write a character at the current cursor position
 *
 * @param c Character to display
 *
 * Automatically advances the cursor and handles wraparound
 * when reaching the end of a line or screen.
 */
void VgaTerminalPutChar(const char c)
{
    VgaTerminalPutEntryAt(c, g_TerminalColor, g_TerminalColumn, g_TerminalRow);
    if (++g_TerminalColumn == kVgaWidth) {
        g_TerminalColumn = 0;
        if (++g_TerminalRow == kVgaHeight)
            g_TerminalRow = 0;
    }
}

/**
 * @brief Write a string of specified length
 *
 * @param data Pointer to the string data
 * @param size Number of characters to write
 */
void VgaTerminalWrite(const char *data, const size_t size)
{
    for (size_t i = 0; i < size; i++) VgaTerminalPutChar(data[i]);
}

/**
 * @brief Write a null-terminated string in default color (light grey on black)
 *
 * @param data Pointer to the null-terminated string
 */
void VgaTerminalWriteString(const char *data)
{
    VgaTerminalSetColor(VgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    VgaTerminalWrite(data, strlen(data));
}

/**
 * @brief Write a null-terminated string in error color (light red on black)
 *
 * @param data Pointer to the null-terminated string
 *
 * Temporarily changes color to light red, writes the string,
 * then restores the previous color.
 */
void VgaTerminalWriteError(const char *data)
{
    VgaTerminalSetColor(VgaEntryColor(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    VgaTerminalWrite(data, strlen(data));
    VgaTerminalSetColor(VgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}
