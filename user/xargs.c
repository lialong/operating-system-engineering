#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
	int i;
	int j = 0;
	int k;
	int l,m = 0;
    char block[32];
	char buf[32];
	char *p = buf;
    char *lineSplit[32];
    for(i = 1; i < argc; i++){
        lineSplit[j++] = argv[i];
    }
	while( (k = read(0, block, sizeof(block))) > 0){
		for(l = 0; l < k; l++){
			if(block[l] == '\n'){				
                buf[m++] = 0;
				m = 0;
				lineSplit[j++] = p;
				p = buf;
				lineSplit[j] = 0;				
				if(fork() == 0){
					exec(argv[1], lineSplit);					
                }
				j = argc - 1;
                wait(0);
            }else if(block[l] == ' ') {                
				buf[m++] = 0;				
				lineSplit[j++] = p;
				p = &buf[m];
            }else {
				buf[m++] = block[l];
			}
        }
    }
	exit(0);
}

