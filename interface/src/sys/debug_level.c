#include "mi_common.h"
#include "mi_print.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

MI_DBG_LEVEL_e _MI_PRINT_GetDebugLevel(const char *name){
    char path[256];
    snprintf(path, sizeof(path), "/proc/mi_modules/mi_%s/debug_level", name);
    FILE *fp = fopen(path, "r");
    if(!fp) return 0;
    int level;
    fscanf(fp, "%d", &level);
    fclose(fp);
    return level;
}

