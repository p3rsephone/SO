#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


char* const_(char* input, char* x){
	int size_i = strlen(input) + 1;
	int size_x = strlen(x) + 1;
	char *new = malloc (sizeof(char) * (size_i + size_x));
	strcpy(new, input);
	new[size_i-1] = ':';
	new[size_i] = '\0';
	strcat(new, x);
	new[size_i+size_x] = '\n';
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

/*
int filter_check(char* x, int i1, int i2, char *op){

	if (!strcmp(op, "<"))  return c[i1-1] <  c[i2-1];
	if (!strcmp(op, ">"))  return c[i1-1] >  c[i2-1];
	if (!strcmp(op, ">=")) return c[i1-1] >= c[i2-1];
	if (!strcmp(op, "<=")) return c[i1-1] <= c[i2-1];
	if (!strcmp(op, "==")) return c[i1-1] == c[i2-1];
	if (!strcmp(op, "!=")) return c[i1-1] != c[i2-1];
}

char* filter(char* x, int i1, int i2, char *op){
	if (filter_check(x, i1, i2, op))
		return x;
	else return NULL;
}
*/


void printStrings(char **s, int n){
	int i;
	for (i=0; i<n; i++)
		printf("%s\n", s[i]);
}

char* runCommand(char *command, char *input){
	char input_aux[strlen(input)+1];
	strcpy(input_aux, input);


	char **c = divideString(command, " ");
	//printStrings(c, 2);
	char **i = divideString(input_aux, ":");
	//printStrings(i, 4);
	if (!strcmp(c[0], "const")){
		input = const_(input, c[1]);
		printf("%s\n", input);
		return input;
	}
	if (!strcmp(c[0], "filter")){
		//...
	}
}

int main(){
	char command[10] = "const 10";
	char a[15] = "a:10:b:4";
	runCommand(command, a);
}
