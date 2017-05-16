/**@file inputProcessing.c
 *
 * \brief Funções que tratam de strings
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Divide uma string em várias strings
 * @param  x       String a dividir
 * @param  divider Separador
 * @return         Array de strings
 */
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

/**
 * Calcula o tamanho de um array de strings (adicionando mais 1 por cada string)
 * @param  c Array de Strings
 * @return Tamanho
 */
int strlenMulti(char **c){
	int i, size = 0;
	for (i=0; c[i]; i++)
		size += strlen(c[i]) + 1;
	return size;
}

/**
 * Concatena um array de strings numa string única separada por espaços (por cópia)
 * @param  c Array de Strings
 * @return String concatenada
 */
char* strcatWithSpaces(char **c){
	char *s = malloc (sizeof(char) * strlenMulti(c));
	int i, j, n;
	for (i=0; c[i]; i++){
		for (j=0; c[i][j]; j++, n++)
			s[n] = c[i][j];
		s[n++] = ' ';
	}
	s[n-2] = '\0';
	return s;
}

/**
 * \brief Número de dígitos de um número
 * @param  n Número
 * @return   Número de dígitos
 */
int numberOfDigits(int n){
	int i = 1;
	while (n/10){
		i++;
		n /= 10;
	}
	return i;
}

char* addCommandPrefix(char* cmd){
	if (!strcmp(cmd, "const" ) ||
		!strcmp(cmd, "filter") ||
		!strcmp(cmd, "window") ||
		!strcmp(cmd, "spawn" ) ){

		char *out = malloc (sizeof(char) * (strlen(cmd) + 3));
		int i;
		out[0] = '.'; out[1] = '/';
		for (i=2; cmd[i-2]; i++)
			out[i] = cmd[i-2];
		return out;
	}
	else{
		char *out = malloc (sizeof(char) * (strlen(cmd) + 1));
		strcpy(out, cmd);
		return out;
	}
}

char* fifoName(int id, char* io){
	int nd = numberOfDigits(id);
	char* c = malloc(sizeof(char) * (5 + nd + strlen(io)));
	sprintf(c, "fifo_%d_%s", id, io);
	return c;
}


int readline(int fildes, char *buf, int nbyte){
	int n = 0, i;
	char c;

	do{
		i = read(fildes, &c, 1);
		if (i){
			buf[n] = c;
			n++;
		}
	}
	while (i && n != nbyte && c != '\n');

	buf[n] = '\0';
	return n;
}