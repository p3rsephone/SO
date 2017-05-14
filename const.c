#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


char* const_(char* input, char* x){
	
    int size_i = strlen(input);
	int size_x = strlen(x);
	char *new = malloc (sizeof(char) * (size_i + size_x + 1));
	strcpy(new, input);
	new[size_i] = ':';
    new [size_i+1] = '\0';
	strcat(new, x);
	new[size_i+size_x+1] = '\0';
	return new;
}

char** divideString(char x[], char* divider){
	int i = 0, n = 10;
	char **c;
	c = (char **) malloc (sizeof(char *) * n);
	c[0] = strtok(x, divider);
	while (c[i]){
		i++;
		c[i] = strtok(NULL, divider);
		if (i == n){
			c = realloc (c, sizeof(char *) * n*2);
			n = n*2;
		}
	}
	return c;
}
int main(int argc, char*argv[]) { //argv1 = input; argv2 = numero em string;
    if (argc!=3) printf("[CONST]Not enough arguments.\n");
    else {
        printf("%s\n", const_(argv[1], argv[2]));
    }
    return 0;
}   
