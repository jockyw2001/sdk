#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "mi_bar_dep.h"

int user_test(void){
    pthread_t *tid = malloc(sizeof(*tid));
    pthread_create(tid, NULL, NULL, NULL);
}
