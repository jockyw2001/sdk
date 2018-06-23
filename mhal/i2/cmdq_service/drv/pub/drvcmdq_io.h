#ifndef __DRVCMDQ_IO_HH__
#define __DRVCMDQ_IO_HH__

#define IOCTL_CMDQ_TEST1_CONFIG                                (1)
#define IOCTL_VIP_MAGIC                                        ('3')
/**
* Used to set CMDQ cmd, use ST_IOCTL_VIP_CMDQ_CONFIG.
*/
#define IOCTL_CMDQ_TEST_1                            _IO(IOCTL_VIP_MAGIC,  IOCTL_CMDQ_TEST1_CONFIG)


#endif
