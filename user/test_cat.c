#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

int main(){
	int pid, childProcessStatus, exitPid;
	char *argv[2];
	pid = fork();
	if(pid == 0){
		argv[0] = "cat";
		argv[1] = 0;
		close(0);
		open("input.txt", O_RDONLY);
		exec("cat", argv);
		exit(0);
	}
	exitPid = wait(&childProcessStatus);
	printf("main: fork a child process %d\n", pid);
	printf("main: child %d process exit with status %d\n", exitPid, childProcessStatus);
	exit(0);
}
