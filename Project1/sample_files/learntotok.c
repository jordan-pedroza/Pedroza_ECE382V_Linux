#include <stdio.h>
#include <string.h>


int main(void)
{
	char s[] = "ls -a";
	char d[] = " ";
	
	char *token = strtok(s, d);

	while(token != NULL){
		printf("%s\n", token);
		token = strtok(NULL, d);
	}


}