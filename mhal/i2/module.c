#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

void _DrvSclVpeModuleInit(void);
s32 DrvVif_Init(void);

int mload_init (void);
void mload_cleanup(void);

static int __init mhal_driver_init(void)
{
    pr_info("mhal driver init\n");
    DrvVif_Init();
#if defined(SCLOS_TYPE_LINUX_TEST)
    _DrvSclVpeModuleInit();
#endif
    mload_init();
    return 0;
}
static void __exit mhal_driver_exit(void)
{
    mload_cleanup();
    pr_info("mhal exit\n");
}

subsys_initcall(mhal_driver_init);
module_exit(mhal_driver_exit);

MODULE_DESCRIPTION("MHal");
MODULE_AUTHOR("SigmaStar");
MODULE_LICENSE("GPL");
