#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int main(){
	char *path = "a";
	int fd = open(path, O_RDONLY);
	struct dirent de;
	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		printf("de.name:%s, de.inum:%d\n", de.name, de.inum);
	}
	exit(0);
}
