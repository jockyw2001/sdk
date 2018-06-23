#ifndef __CEVA_BOOT_H__
#define __CEVA_BOOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CEVABOOT_API_SUCCESS 0

#define CEVABOOT_DSP_BOOT_ALREADY   (-60)
#define CEVABOOT_FILE_READ_ERR      (-61)
#define CEVABOOT_BOOT_UP_ERR        (-62)
#define CEVABOOT_SHUT_DOWN_ERR      (-63)
#define CEVABOOT_DEVICE_ERR         (-64)

int ceva_boot_up_dsp_ex(int dev_file, const char *boot_ini_path);
int ceva_shut_down_dsp_ex(int dev_file);

int ceva_boot_up_dsp(const char *boot_ini_path);
int ceva_shut_down_dsp();

#ifdef __cplusplus
}
#endif

#endif // __CEVA_BOOT_H__
