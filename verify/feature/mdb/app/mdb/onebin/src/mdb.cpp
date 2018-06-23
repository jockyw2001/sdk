#include <unistd.h>  
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mdb_service.h"
#include "mdb_console.h"
#include "mdb.h"

int mdb_main(void)
{
	pid_t fPid;
	fPid = fork();
    if (fPid < 0)
    {
        printf("error in fork!");  
		return -1;
	}
    else if (fPid == 0)
	{
		mdb_service();
		return 0;
	}
	fPid = fork();
    if (fPid < 0)   
    {
        printf("error in fork!");  
		return -1;
	}
    else if (fPid == 0)
	{
		return mdb_console(1);
	}
	waitpid(fPid, NULL, 0);

	return 0;
}
