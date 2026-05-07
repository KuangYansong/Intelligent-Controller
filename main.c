#include "system.h"

int main(void)
{
    static SystemContext system;

    if (System_Init(&system) != SSCB_OK)
    {
        for (;;)
        {
        }
    }

    for (;;)
    {
        System_RunOnce(&system);
    }
}

