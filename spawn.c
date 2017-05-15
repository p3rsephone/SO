/*@file spawn.c
 *
 * \brief Executa o comando spawn
 *
 * Este módulo contem o comando spawn, que irá
 * executar um comando (com ou sem argumentos)
 * e irá colocar o valor do exit status no
 * input recebido. Substitui a ocorrência de $n
 * pelo valor da coluna n do input.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "stringProcessing.h"

/**
 * \brief Executa um comando e coloca o valor do exit status no input (input + :x)
 * @param  input    Input
 * @param  commands Array com o comando e os seus 
 * @return String com input + exit_status
 */
char* spawn(char* input, char** commands){
	char inputCopy[strlen(input) + 1];
	strcpy(inputCopy, input);
	char **c = divideString(inputCopy, ":");
	char *new;
	int i, x, status;
	int pd[2];
	
	for (i=0; commands[i]; i++){
		if (commands[i][0] == '$'){
			char aux[strlen(commands[i])];
			strcpy(aux, commands[i]+1);
			int n = atoi(aux) -1;
			strcpy(commands[i], c[n]);
			if (commands[i][strlen(c[n])-1] == '\n')
				commands[i][strlen(c[n])-1] = '\0';
		}
	}

	pipe(pd);
	x = fork();

	if (!x){
		close(pd[0]);
		fclose(stderr);
		dup2(pd[1], 1);
		execvp(commands[0], commands);
		_exit(errno);
	}
	close(pd[0]);
	close(pd[1]);
	wait(&status);

	char status_string[4];
	sprintf(status_string, "%d", WEXITSTATUS(status));
	new = malloc (sizeof(char) * (strlen(input) + strlen(status_string) + 1));
	strcpy(new, input);
	new[strlen(input)-1] = ':';
	new[strlen(input)] = '\0';
	strcat(new, status_string);
	return new;
}

/**
 * \brief Função main de window
 * @param  argc Número de argumentos
 * @param  argv Array de argumentos
 * @return ---
 */
int main(int argc, char *argv[]){
	char buffer[4096];
	int charsRead;
	char* out;
	
	while((charsRead = read(0, buffer, 4096)) > 0){
		if (charsRead > 1){
			char argvCopy[strlen(argv[1])];
			strcpy(argvCopy, argv[1]);
			char** commands = divideString(argvCopy, " ");
			out = spawn(buffer, commands);
			printf("%s\n", out);
			memset(buffer, 0, charsRead);
		}
	}
	return 0;
}