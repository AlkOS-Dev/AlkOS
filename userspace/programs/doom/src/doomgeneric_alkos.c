// doomgeneric for AlkOS

#include "doomgeneric.h"
#include "doomkeys.h"
#include "i_scale.h"

#include <alkos/calls.h>
#include <alkos/sys/input.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static GuiBufferInfo BufferInfo;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex  = 0;

// Keys that Doom cares about
static const VirtualKey s_MonitoredKeys[] = {
    // Movement
    VK_ArrowLeft,
    VK_ArrowRight,
    VK_ArrowUp,
    VK_ArrowDown,
    // Actions
    VK_Space,
    VK_LeftCtrl,
    VK_RightCtrl,
    VK_LeftAlt,
    VK_RightAlt,
    // Menu/System
    VK_Escape,
    VK_Enter,
    VK_Grave,
    VK_Backspace,
    // Function keys
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,
    // Modifiers
    VK_LeftShift,
    VK_RightShift,
    VK_CapsLock,
    VK_NumLock,
    VK_ScrollLock,
    // Navigation
    VK_Home,
    VK_End,
    VK_PageUp,
    VK_PageDown,
    VK_Insert,
    VK_Delete,
    // Numpad
    VK_NumPad0,
    VK_NumPad1,
    VK_NumPad2,
    VK_NumPad3,
    VK_NumPad4,
    VK_NumPad5,
    VK_NumPad6,
    VK_NumPad7,
    VK_NumPad8,
    VK_NumPad9,
    VK_NumPadEnter,
    VK_NumPadDivide,
    VK_NumPadMultiply,
    VK_NumPadAdd,
    VK_NumPadSubtract,
    VK_NumPadDecimal,
    // Special chars
    VK_Minus,
    VK_Equal,
    // Letters (for typing)
    VK_A,
    VK_B,
    VK_C,
    VK_D,
    VK_E,
    VK_F,
    VK_G,
    VK_H,
    VK_I,
    VK_J,
    VK_K,
    VK_L,
    VK_M,
    VK_N,
    VK_O,
    VK_P,
    VK_Q,
    VK_R,
    VK_S,
    VK_T,
    VK_U,
    VK_V,
    VK_W,
    VK_X,
    VK_Y,
    VK_Z,
    // Number row (for typing)
    VK_Key0,
    VK_Key1,
    VK_Key2,
    VK_Key3,
    VK_Key4,
    VK_Key5,
    VK_Key6,
    VK_Key7,
    VK_Key8,
    VK_Key9,
};

#define NUM_MONITORED_KEYS (sizeof(s_MonitoredKeys) / sizeof(s_MonitoredKeys[0]))

// Previous state of monitored keys (to detect press/release)
static bool s_PrevKeyStates[NUM_MONITORED_KEYS] = {0};

// Convert VirtualKey to Doom key
static unsigned char convertToDoomKey(VirtualKey vk)
{
    unsigned char key = 0;

    switch (vk) {
        // Arrow keys
        case VK_ArrowLeft:
            key = KEY_LEFTARROW;
            break;
        case VK_ArrowRight:
            key = KEY_RIGHTARROW;
            break;
        case VK_ArrowUp:
            key = KEY_UPARROW;
            break;
        case VK_ArrowDown:
            key = KEY_DOWNARROW;
            break;

        // Actions
        case VK_Space:
            key = KEY_USE;
            break;
        case VK_LeftCtrl:
        case VK_RightCtrl:
            key = KEY_FIRE;
            break;
        case VK_LeftAlt:
        case VK_RightAlt:
            key = KEY_LALT;
            break;

        // Menu/System
        case VK_Escape:
            key = KEY_ESCAPE;
            break;
        case VK_Enter:
        case VK_NumPadEnter:
            key = KEY_ENTER;
            break;
        case VK_Grave:
            key = KEY_TAB;
            break;
        case VK_Backspace:
            key = KEY_BACKSPACE;
            break;

        // Function keys
        case VK_F1:
            key = KEY_F1;
            break;
        case VK_F2:
            key = KEY_F2;
            break;
        case VK_F3:
            key = KEY_F3;
            break;
        case VK_F4:
            key = KEY_F4;
            break;
        case VK_F5:
            key = KEY_F5;
            break;
        case VK_F6:
            key = KEY_F6;
            break;
        case VK_F7:
            key = KEY_F7;
            break;
        case VK_F8:
            key = KEY_F8;
            break;
        case VK_F9:
            key = KEY_F9;
            break;
        case VK_F10:
            key = KEY_F10;
            break;
        case VK_F11:
            key = KEY_F11;
            break;
        case VK_F12:
            key = KEY_F12;
            break;

        // Modifiers
        case VK_LeftShift:
        case VK_RightShift:
            key = KEY_RSHIFT;
            break;
        case VK_CapsLock:
            key = KEY_CAPSLOCK;
            break;
        case VK_NumLock:
            key = KEY_NUMLOCK;
            break;
        case VK_ScrollLock:
            key = KEY_SCRLCK;
            break;

        // Navigation
        case VK_Home:
            key = KEY_HOME;
            break;
        case VK_End:
            key = KEY_END;
            break;
        case VK_PageUp:
            key = KEY_PGUP;
            break;
        case VK_PageDown:
            key = KEY_PGDN;
            break;
        case VK_Insert:
            key = KEY_INS;
            break;
        case VK_Delete:
            key = KEY_DEL;
            break;

        // Numpad keys
        case VK_NumPad0:
            key = KEYP_0;
            break;
        case VK_NumPad1:
            key = KEYP_1;  // KEY_END
            break;
        case VK_NumPad2:
            key = KEYP_2;  // KEY_DOWNARROW
            break;
        case VK_NumPad3:
            key = KEYP_3;  // KEY_PGDN
            break;
        case VK_NumPad4:
            key = KEYP_4;  // KEY_LEFTARROW
            break;
        case VK_NumPad5:
            key = KEYP_5;  // '5'
            break;
        case VK_NumPad6:
            key = KEYP_6;  // KEY_RIGHTARROW
            break;
        case VK_NumPad7:
            key = KEYP_7;  // KEY_HOME
            break;
        case VK_NumPad8:
            key = KEYP_8;  // KEY_UPARROW
            break;
        case VK_NumPad9:
            key = KEYP_9;  // KEY_PGUP
            break;
        case VK_NumPadDivide:
            key = KEYP_DIVIDE;  // '/'
            break;
        case VK_NumPadMultiply:
            key = KEYP_MULTIPLY;  // '*'
            break;
        case VK_NumPadAdd:
            key = KEYP_PLUS;  // '+'
            break;
        case VK_NumPadSubtract:
            key = KEYP_MINUS;  // '-'
            break;
        case VK_NumPadDecimal:
            key = KEYP_PERIOD;
            break;

        // Special chars
        case VK_Minus:
            key = KEY_MINUS;
            break;
        case VK_Equal:
            key = KEY_EQUALS;
            break;

        // Letters
        case VK_A ... VK_Z:
            key = 'a' + (vk - VK_A);
            break;

        // Number row
        case VK_Key0 ... VK_Key9:
            key = '0' + (vk - VK_Key0);
            break;

        default:
            break;
    }

    return key;
}

static void addKeyToQueue(int pressed, VirtualKey vk)
{
    unsigned char key = convertToDoomKey(vk);
    if (key == 0) {
        return;  // Ignore unmapped keys
    }

    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void DG_Init()
{
    // Create graphics session
    __platform_create_graphic_session(&BufferInfo);

    if (BufferInfo.buffer_ptr == NULL) {
        printf("Failed to create graphics session!\n");
        exit(-1);
    }

    // Set screen dimensions
    DG_ScreenBuffer = (uint32_t *)BufferInfo.buffer_ptr;
    DG_ScreenWidth  = BufferInfo.width;
    DG_ScreenHeight = BufferInfo.height;
    // DG_ScreenMode = &mode_scale_2x; // 640x400
}

void DG_DrawFrame() { __platform_blit(); }

void DG_SleepMs(uint32_t ms)
{
    NanoSleep(ms * 1000000ULL);  // Convert to nanoseconds
}

uint32_t DG_GetTicksMs()
{
    TimeVal tv;
    Timezone tz;
    GetClockValueSysCall(kProcTimePrecise, &tv, &tz);

    return (uint32_t)(tv.remainder / 1000000ULL);
}

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
    // Poll all monitored keys and detect state changes
    // TODO: Inefficient - better to have an event-driven system
    for (unsigned int i = 0; i < NUM_MONITORED_KEYS; ++i) {
        VirtualKey vk     = s_MonitoredKeys[i];
        bool currentState = GetKeyState(vk);

        // Detect state change
        if (currentState != s_PrevKeyStates[i]) {
            // Key state changed
            addKeyToQueue(currentState ? 1 : 0, vk);
            s_PrevKeyStates[i] = currentState;
        }
    }

    // Now check if we have any events in the queue
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
        // key queue is empty
        return 0;
    } else {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

        *pressed = keyData >> 8;
        *doomKey = keyData & 0xFF;

        return 1;
    }
}

void DG_SetWindowTitle(const char *title)
{
    // Not implemented
    (void)title;
}

int main()
{
    doomgeneric_Create(0, NULL);

    while (1) {
        doomgeneric_Tick();
    }

    return 0;
}
