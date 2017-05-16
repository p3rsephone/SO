/**@file const.c
 *
 * \brief Executa o comando const
 *
 * Este módulo contem o comando de const, que adiciona 
 * uma determianda string no final de um input, da
 * seguinte forma : input:x , onde x é o que queremos
 * adicionar
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "stringProcessing.h"

/**
 * Retorna uma nova string com o resultado de colocar um ':x' no input recebido
 * @param  input Input
 * @param  x     Valor a colocar
 * @return       Nova string processada
 */
char* const_(char* input, char* x){
	int size_i = strlen(input);
	int size_x = strlen(x);
	char *new = malloc (sizeof(char) * (size_i + size_x +1));
	strcpy(new, input);
	new[size_i-1] = ':';
	new[size_i] = '\0';
	strcat(new, x);
	return new;
}

/**
 * Função main de const_
 * @param  argc Número de argumentos
 * @param  argv Argumentos
 * @return ---
 */
int main(int argc, char *argv[]){
	char buffer[4096];
	char *out;
	int charsRead;
	strcat(argv[1], "\n");
	while((charsRead = readline(0, buffer, 4096)) > 0){
		if (charsRead > 1){
			out = const_(buffer, argv[1]);
			write(1, out, strlen(out));
			memset(buffer, 0, charsRead);
		}
	}
	return 0;
}