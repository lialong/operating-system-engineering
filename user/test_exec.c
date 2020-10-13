#include "kernel/types.h"
#include "user/user.h"

int main(){
	char *argv[4];
	argv[0] = "echo";
	argv[1] = "hello";
	argv[2] = "123123";
	argv[3] = 0;
	exec("echo", argv);
	printf("exec error\n");
	exit(0);
}
