#include <unistd.h>  
#include <stdio.h>

int main(void)
{
	pid_t fPid;

	fPid = fork();
    if (fPid < 0)
    {
        printf("error in fork!");
	}
    else if(fPid == 0)
	{  
        if(execl("mdb", NULL, NULL) < 0)  
        {
            perror("execlp error!");  
            return -1 ;  
        }
    }
	waitpid(fPid, NULL, 0);

	return 0;
}