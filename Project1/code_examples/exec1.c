#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* exec Example 1 */
int main(){
    int cpid;
    char *inString;

    while(inString = readline("# ")){
	cpid = fork();
	if (cpid == 0){
	    execlp(inString, inString, (char *)NULL);
	}else{
	    wait((int *)NULL);
	}
    }
}
