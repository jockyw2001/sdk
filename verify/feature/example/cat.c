#include "cat1.h"
#include "cat2.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "busybox_wrapper.h"

shell_rcode cmd_cat1(const char *argv[], int count){
    printf("[%s]\n", __func__);
    return 0;
}

shell_rcode cmd_cat2(const char *argv[], int count){
    printf("[%s]\n", __func__);
    return 0;
}

shell_rcode cmd_cat3(const char *argv[], int count){
    printf("[%s]\n", __func__);
    return 0;
}

shell_rcode cmd_cat4(const char *argv[], int count){
    printf("[%s]\n", __func__);
    return 0;
}

static busybox_cmd_t cmds[] = {
    {"cat1", cmd_cat1},
    {"cat4", cmd_cat4},
    {"cat2", cmd_cat2},
    {"cat3", cmd_cat3},
};

int main(){
    struct rlimit limit;
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &limit);
    //printf("cat1 test: %d\n", cat1(3, 5));
    //printf("cat2 test: %d\n", cat2());
    busybox_loop(cmds, sizeof(cmds)/sizeof(*cmds));
    return 0;
}
