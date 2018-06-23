
#define MIU_BLOCK_NUM 4
#define KERN_CHUNK_NUM 3
#define MIU_PROTECT_ENABLE 1
#define MIU_PROTECT_DISABLE 0

int deleteKRange(unsigned long long start_pa, unsigned long length);
int addKRange(unsigned long long start_pa, unsigned long length);
extern unsigned char *g_kernel_protect_client_id;
void init_glob_miu_kranges(void);
