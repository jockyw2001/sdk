#include "mmapinfo.h"
#include "mapi_syscfg_fetch.h"
#include "pqloader.h"
#include "dictionary.h"
#include "iniparser.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

char buf[40960];

int main(int argc, const char *argv[]){
    struct rlimit limit;
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &limit);
    int i, fd, voff, rval;
    if(strstr(argv[0], "dump_mmap")){
        fd = open("/proc/mi_modules/common/memory_info", O_RDONLY);
        rval = read(fd, buf, 1024);
        if(rval > 0){
            printf(buf);
        }else{
            printf("memory_info read failed!(%s)\n", strerror(errno));
        }
        close(fd);
        voff = 0;
        fd = open("/proc/mi_modules/common/mmap_info", O_RDONLY);
        while((rval = pread(fd, buf, 1024, voff++)) > 0){
            printf(buf);
        }
        if(errno != ENOKEY){
            printf("read mmap_info failed!(%s)\n", strerror(errno));
            return -1;
        }
        close(fd);
    }
    if(strstr(argv[0], "load_mmap")){
        MMAPInfo info;
        ParserMmap(argv[1], &info);
#if 0
        printf("total:%x\n", info.header->total_mem_size);
        printf("miu0:%x\n", info.header->miu0_mem_size);
        printf("miu1:%x\n", info.header->miu1_mem_size);
        printf("miu2:%x\n", info.header->miu2_mem_size);
        printf("miub:%x\n", info.header->miu_boundary);
        printf("miub2:%x\n", info.header->miu_boundary2);
        printf("item:%d\n", info.header->n_mmap_items);
        printf("4k:%x\n", info.header->b_is_4k_align);
        int i;
        for(i = 0; i < info.header->n_mmap_items; ++i){
            printf("name:%s\n", info.name[i]);
            printf("gid:%x\n", info.context[i].u32gID);
            printf("addr:%x\n", info.context[i].u32Addr);
            printf("size:%x\n", info.context[i].u32Size);
            printf("layer:%u\n", info.context[i].u8Layer);
            printf("align:%x\n", info.context[i].u32Align);
            printf("miuNo:%x\n", info.context[i].u32MiuNo);
            printf("cmahid:%x\n", info.context[i].u32CMAHid);
        }
#endif
        fd = open("/proc/mi_modules/common/memory_info", O_WRONLY);
        if(fd < 0){
            printf("failed open memory_info!(%s)\n", strerror(errno));
            return -1;
        }

        write(fd, buf, sprintf(buf, "%u %u %u %u %u %u %u\n",
                    info.header->total_mem_size,
                    info.header->miu0_mem_size,
                    info.header->miu1_mem_size,
                    info.header->miu_boundary,
                    info.header->n_mmap_items,
                    info.header->b_is_4k_align,
                    info.header->miu1_mem_size != 0));
        close(fd);
        fd = open("/proc/mi_modules/common/mmap_info", O_WRONLY);
        if(fd < 0){
            printf("failed open mmap_info!(%s)\n", strerror(errno));
            return -1;
        }
        for(i = 0; i < info.header->n_mmap_items; ++i){
            pwrite(fd, buf, sprintf(buf, "%s %u %u %u %u %u %u %u %u\n",
                        info.name[i],
                        info.context[i].u32gID,
                        info.context[i].u32Addr,
                        info.context[i].u32Size,
                        info.context[i].u8Layer,
                        info.context[i].u32Align,
                        info.context[i].u32MemoryType,
                        info.context[i].u32MiuNo,
                        info.context[i].u32CMAHid), i);
        }
        close(fd);
    }
    if(strstr(argv[0], "dump_config")){
        DumpConfig();
        DumpPQConfig();
    }
    if(strstr(argv[0], "load_config")){
        LoadConfig();
        LoadPQConfig();
    }
	return 0;
}
