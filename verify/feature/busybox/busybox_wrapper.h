#ifndef _BUSYBOX_WRAPPER_H_
#define _BUSYBOX_WRAPPER_H_

typedef unsigned char shell_rcode;

typedef struct {
	const char *name;
	shell_rcode (*func)(const char *argv[], int count);
} busybox_cmd_t;

void busybox_loop(busybox_cmd_t *cmdlist, int count);

#endif /* _BUSYBOX_WRAPPER_H_ */
