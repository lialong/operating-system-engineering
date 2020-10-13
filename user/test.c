#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int main(){
	int fd = open("./ls", O_RDONLY);
	if(fd < 0){
		fprintf(2, "cannot open %s\n", "./a");
		exit(0);
	}
	struct stat st;
	fstat(fd, &st);
	printf("ino:%d, type:%d", st.ino, st.type);
	exit(0);
}
