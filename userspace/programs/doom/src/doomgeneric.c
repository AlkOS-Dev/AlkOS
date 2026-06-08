#include <stdio.h>

#include "doomgeneric.h"
#include "i_scale.h"
#include "i_video.h"
#include "m_argv.h"

screen_mode_t *DG_ScreenMode = NULL;
pixel_t *DG_ScreenBuffer     = NULL;
int DG_ScreenWidth           = 640;
int DG_ScreenHeight          = 400;

void M_FindResponseFile(void);
void D_DoomMain(void);

static screen_mode_t *doomgeneric_SelectScreenMode()
{
    screen_mode_t *mode;

    // Determine the appropriate scaling mode based on framebuffer resolution
    int scale_x    = DG_ScreenWidth / SCREENWIDTH;
    int scale_y    = DG_ScreenHeight / SCREENHEIGHT;
    int fb_scaling = (scale_x < scale_y) ? scale_x : scale_y;

    if (fb_scaling < 1)
        fb_scaling = 1;
    if (fb_scaling > 5)
        fb_scaling = 5;

    // Determine if we need stretch (aspect ratio correction for 4:3) or squash mode
    boolean use_stretch        = false;
    boolean use_squash         = false;
    boolean use_squash_800x600 = false;

    // Special case: 800x600 uses squash 3x with 50% blending
    if (DG_ScreenWidth == 800 && DG_ScreenHeight == 600) {
        use_squash_800x600 = true;
        fb_scaling         = 3;
        printf("DG_Init: Using special SQUASH 3x mode for 800x600 (50%% blending)\n");
    }
    // Detect stretch mode: height is 1.2x scaled (240, 480, 720, 960, 1200)
    else if (DG_ScreenHeight == SCREENHEIGHT_4_3 * fb_scaling) {
        use_stretch = true;
        printf("DG_Init: Using STRETCH mode for 4:3 aspect ratio\n");
    }
    // Detect squash mode: width is 0.8x scaled (256, 512, 768, 1024, 1280)
    else if (DG_ScreenWidth == SCREENWIDTH_4_3 * fb_scaling) {
        use_squash = true;
        printf("DG_Init: Using SQUASH mode for 4:3 aspect ratio\n");
    } else {
        printf("DG_Init: Using standard SCALE mode\n");
    }

    // Select appropriate screen mode based on scaling factor and mode
    if (use_squash_800x600) {
        // Special case: 800x600 uses squash_3x with 50% blending
        mode = &mode_squash_3x;
    } else if (use_stretch) {
        switch (fb_scaling) {
            case 1:
                mode = &mode_stretch_1x;
                break;
            case 2:
                mode = &mode_stretch_2x;
                break;
            case 3:
                mode = &mode_stretch_3x;
                break;
            case 4:
                mode = &mode_stretch_4x;
                break;
            case 5:
                mode = &mode_stretch_5x;
                break;
        }
    } else if (use_squash) {
        switch (fb_scaling) {
            case 1:
                mode = &mode_squash_1x;
                break;
            case 2:
                mode = &mode_squash_2x;
                break;
            case 4:
                mode = &mode_squash_4x;
                break;
            case 5:
                mode = &mode_squash_5x;
                break;
            default:
                printf("DG_Init: Unsupported squash factor %d, using 2x\n", fb_scaling);
                mode = &mode_squash_2x;
                break;
        }
    } else {
        switch (fb_scaling) {
            case 1:
                mode = &mode_scale_1x;
                break;
            case 2:
                mode = &mode_scale_2x;
                break;
            case 3:
                mode = &mode_scale_3x;
                break;
            case 4:
                mode = &mode_scale_4x;
                break;
            case 5:
                mode = &mode_scale_5x;
                break;
        }
    }

    printf("DG_Init: Selected mode %dx%d\n", mode->width, mode->height);
    return mode;
}

void doomgeneric_Create(int argc, char **argv)
{
    // save arguments
    myargc = argc;
    myargv = argv;

    M_FindResponseFile();

    DG_Init();

    if (DG_ScreenMode == NULL) {
        DG_ScreenMode = doomgeneric_SelectScreenMode();
    }

    D_DoomMain();
}
