#include <linux/module.h>
#include <linux/printk.h>

MODULE_LICENSE("GPL v2");

#ifdef DEBUG
extern s32 DrvVif_Init(void);
static int __init mhal_init_driver(void)
{
    DrvVif_Init();
    pr_info("mhal driver init\n");
    return 0;
}

static void __exit mhal_exit_driver(void)
{
    pr_info("mhal driver exit\n");
}

module_init(mhal_init_driver);
module_exit(mhal_exit_driver);
#endif
