// Remote control of instinct evolution display mode.
// Usage: display_control <off | text | graphics>

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include "monkey_and_bananas.hpp"

// Shared memory id.
#define SHMKEY 383

// Shared memory format.
struct SHMEM
{
    MonkeyAndBananas::DISPLAY_TYPE displayControl;
};

main(int argc, char *argv[])
{
    key_t shmkey;
    int shmid;
    struct SHMEM *shmem;
    MonkeyAndBananas::DISPLAY_TYPE control;

    // Get control.
    if (argc != 2)
    {
        fprintf(stderr, "Usage: display_control <off | text | graphics>\n");
        exit(1);
    }
    if (strcmp(argv[1], "off") == 0)
    {
        control = MonkeyAndBananas::NO_DISPLAY;
    } else if (strcmp(argv[1], "text") == 0)
    {
        control = MonkeyAndBananas::TEXT_DISPLAY;
    } else if (strcmp(argv[1], "graphics") == 0)
    {
        control = MonkeyAndBananas::GRAPHICS_DISPLAY;
    }
    else
    {
        fprintf(stderr, "Invalid control\n");
        fprintf(stderr, "Usage: display_control <off | text | graphics>\n");
        exit(1);
    }

    // Create the shared memory segment.
    shmkey = SHMKEY;
    if ((shmid = shmget(shmkey, sizeof(struct SHMEM), 0)) == -1)
    {
        fprintf(stderr, "attach: shmget failed, errno=%d\n",errno);
        fprintf(stderr, "Instincts evolution not running?\n");
        exit(1);
    }

    // Store control.
    shmem = (struct SHMEM *)shmat(shmid, 0, 0);
    shmem->displayControl = control;

    return 0;
}
