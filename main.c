#include "app/firmware.h"

int main(void)
{
    static sscb_firmware_t firmware;

    if (sscb_firmware_init(&firmware) != SSCB_OK) {
        for (;;) {
        }
    }

    if (firmware.system.state == SSCB_STATE_READY) {
        (void)sscb_system_start(&firmware.system);
    }

    for (;;) {
        sscb_firmware_run_once(&firmware);
    }
}
