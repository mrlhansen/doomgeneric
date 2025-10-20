#include <novino/termio.h>
#include <_syscall.h>
#include <stdio.h>

#include "m_argv.h"
#include "doomkeys.h"
#include "doomgeneric.h"

pixel_t* DG_ScreenBuffer = NULL;

void M_FindResponseFile(void);
void D_DoomMain (void);

static uint8_t *fb;
static uint64_t ts;
static int kb;

typedef struct {
    uint64_t tv_sec;
    uint64_t tv_nsec;
} timeval_t;

typedef struct {
    uint64_t ts;
    uint16_t type;
    uint16_t code;
    uint32_t value;
} input_event_t;

void DG_Init()
{
    timeval_t tv;
    uint64_t virt;

    virt = syscall(27, 0, 0, 0, 0, 0);
    fb = (void*)virt;
    tiosetflags(TIONOBUF | TIONOSCRL);

    sys_gettime(&tv);
    ts = tv.tv_sec * 1000 + tv.tv_nsec / 1000000;

    kb = sys_open("/devices/keyboard", 1 | 64);
    if(kb < 0)
    {
        perror("failed to open keyboard");
    }
}

void DG_DrawFrame()
{
    int bytes_per_line = 800*4;
//   int bytes_per_line = 1920*4;
    int doom_bytes_per_line = 4 * DOOMGENERIC_RESX;

    for(int i = 0; i < DOOMGENERIC_RESY; i++)
    {
        memcpy(fb + i * bytes_per_line, DG_ScreenBuffer + i * DOOMGENERIC_RESX, doom_bytes_per_line);
    }
}

void DG_SleepMs(uint32_t ms)
{
    sys_sleep(ms * 1000000);
}

uint32_t DG_GetTicksMs()
{
    uint64_t n;
    timeval_t tv;

    sys_gettime(&tv);
    n = tv.tv_sec * 1000 + tv.tv_nsec / 1000000;

    return n - ts;
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
    input_event_t event;

    int sz = sys_read(kb, sizeof(event), &event);
    if(!sz)
    {
        return 0;
    }

    if(event.value == 2)
    {
        return 0;
    }

    if(event.value == 1)
    {
        *pressed = 0;
    }
    else
    {
        *pressed = 1;
    }

    *doomKey = 0;

    switch(event.code)
    {
        case 1:
            *doomKey = KEY_ESCAPE;
            break;
        case 54:
            *doomKey = KEY_ENTER;
            break;
        case 70: // space
            *doomKey = KEY_USE;
            break;
        case 30: // W
            *doomKey = KEY_FIRE;
            break;
        case 34: // Y
            *doomKey = 'y';
            break;
        case 83:
            *doomKey = KEY_UPARROW;
            break;
        case 84:
            *doomKey = KEY_LEFTARROW;
            break;
        case 85:
            *doomKey = KEY_DOWNARROW;
            break;
        case 86:
            *doomKey = KEY_RIGHTARROW;
            break;
    }

    return *doomKey ? 1 : 0;
}

void DG_SetWindowTitle(const char * title)
{

}

void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

	DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);

	DG_Init();

	D_DoomMain();
}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);
    while(1)
    {
        doomgeneric_Tick();
    }
    return 0;
}
