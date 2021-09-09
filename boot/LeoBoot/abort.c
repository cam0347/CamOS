#include "abort.h"
#include "types.h"
#include "tty.h"
#include "low_level.h"

void abort(const char *trigger, void abort_handler()) {
    error("\n[PROCESS ABORT PROCEDURE]");
    error(trigger);

    if (abort_handler != null) {
        abort_handler();
    }

    error("[PROCESS ABORTED]");
    sys_hlt();
}