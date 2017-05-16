/**@file filter.c
 *
 * \brief Executa o comando filter
 *
 * Este módulo contem o comando de filter, que verifica
 * se determinado input satisfaz uma condição.
 * Se satisfazer essa condição, o input é devolvido.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "stringProcessing.h"
/**
 * \brief Verifica se um input verifica uma determinada condição (n1 ?? n2)
 * @param  input Input
 * @param  c1    Coluna 1
 * @param  c2    Coluna 2
 * @param  op    Operador
 * @return       A condição é verificada (1) ou não (0)
 */
int filter(char* input, int c1, int c2, char* op){
	int i1, i2;
	char** c = divideString(input, ":");

	i1 = atoi(c[c1]);
	i2 = atoi(c[c2]);

	if (!strcmp(op, "<"))  return i1 <  i2;
    else if (!strcmp(op, ">"))  return i1 >  i2;
    else if (!strcmp(op, ">=")) return i1 >= i2;
    else if (!strcmp(op, "<=")) return i1 <= i2;
    else if (!strcmp(op, "==")) return i1 == i2;
    else return i1 != i2; //se é !=
}

/**
 * \brief Função main de filter
 * @param  argc Número de argumentos
 * @param  argv Array de argumentos
 * @return ---
 */
int main(int argc, char *argv[]){
	int charsRead;
	char buffer[4096];
	char** commands = divideString(argv[1], " ");
	int c1, c2;
	
	c1 = atoi(commands[0]) - 1;
	c2 = atoi(commands[2]) - 1;
	
	while((charsRead = readline(0, buffer, 4096)) > 0){
		if (charsRead > 1){
			char inputCopy[charsRead];
			strcpy(inputCopy, buffer);
			if (filter(inputCopy, c1, c2, commands[1]))
				write(1, buffer, charsRead);
			memset(buffer, 0, charsRead);
		}
	}
	return 0;
}
