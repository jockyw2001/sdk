#ifndef _ST_ISP_H_
#define _ST_ISP_H_

#define FW_VER_BCD 0x02	//for test.
typedef struct FW_RELEASE_VERSION {
	unsigned char   major;
	unsigned char minor;
	unsigned short build;
} FW_RELEASE_VERSION;

void ISP_usb_iqsvr_open(void);
void ISP_usb_iqsvr_close(void);
PCAM_IQSVR_HANDLE *ISP_usb_get_iqsvr_handle(void);

#endif