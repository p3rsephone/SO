/**@file controller.c
 *
 * \brief Módulo responsável pelo controlador
 *
 * Este programa irá ler informação passada pelo
 * stdin ou por um ficheiro e irá criar uma rede
 * de nodos (stream processing).
 *
 * Cada nodo irá executar um comando sobre o
 * input recebido e irá enviar a sua saída
 * (ou não) para um processo 'connect', que por
 * sua vez irá passar esse resultado para um ou
 * mais nodos.
 */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "stringProcessing.h"

/* Tamanho do Buffer
 */
#define SIZE_BUF 4096

/**
 * @struct Estrutura que contem toda a informação de um nodo
 */
typedef struct controllerNode{
	int id; ///< id do nodo
	char* cmd; ///< string com o comando a ser executado
	char* args; ///< string com os argumentos do comando
	int* in_ids; ///< ids de todas a ligações ao nó
	int size_in; ///< tamanho de in_ids
	int used_in; ///< quantidade de in_ids
	int* out_ids; ///< ids a quem o nó liga
	int size_out; ///< tamanho de out_ids
	int used_out; ///< quantidade de out_ids
	char* pipeIn_name; ///< nome do pipe de entrada
	char* pipeOut_name; ///< nome do pipe de saída
	int f_in; ///< descritor do pipe de entrada
	int f_out; ///< descritor do pipe de saída
	int pid; ///< id do processo que cria o nó
	int connect_pid; ///< id do processo que faz a ligação da saída
}*ControllerNode;

/**
 * @struct Estrutura que contem todos os nodos
 */
typedef struct controllerInfo{
	int size; ///< tamanho da estrutura
	int used; ///< número de posições usadas
	ControllerNode node; ///< array de nodos
}*ControllerInfo;

/**
 * Inicializa um ControllerInfo
 * @param  size Tamanho inicial
 * @return Nova estrutra
 */
ControllerInfo initControllerInfo(int size){
	ControllerInfo info = malloc (sizeof(struct controllerInfo));
	info->size = size;
	info->used = 0;
	info->node = malloc(sizeof(struct controllerNode) * info->size);

	int i;
	for (i=0; i<size; i++){
		info->node[i].size_in = 5;
		info->node[i].size_out = 5;
		info->node[i].used_in = 0;
		info->node[i].used_out = 0;
		info->node[i].in_ids = malloc (sizeof(int) * info->node[i].size_in);
		info->node[i].out_ids = malloc (sizeof(int) * info->node[i].size_out);
	}

	return info;
}

/**
 * Realoca memória para a estrutra
 * @param info Estrutura
 */
void reallocControllerInfo(ControllerInfo info){
	info->size *= 2;
	info->node = realloc(info->node, sizeof(struct controllerNode) * info->size);
}

/**
 * Realoca memória para o array de ids de um nodo
 * @param info    Estrutura
 * @param id      ID do nodo
 * @param id_name Se é para alocar a entrada ("in") ou a saída ("out")
 */
void reallocNodeIds(ControllerInfo info, int id, char *id_name){
	int* c;
	int *size;

	if (!strcmp(id_name, "in")){
		c = info->node[id].in_ids;
		info->node[id].size_in *= 2;
		size = &(info->node[id].size_in);
	}
	else {
		c = info->node->out_ids;
		info->node[id].size_out *= 2;
		size = &(info->node[id].size_out);
	}

	c = realloc(c, sizeof(int) * *size);
}


/**
 * Procura se um determinado ID já liga a um nodo
 * @param  info  Estrutura
 * @param  id    ID do nodo
 * @param  value ID do nodo que liga a id
 * @param  c     Se o ID liga à entrada ("in") ou à saída ("out")
 * @return       Já existe um value a ligar ao id (true) ou não (false)
 */
int findValue(ControllerInfo info, int id, int value, char* c){
	int i;

	if (!strcmp(c, "in")){
		for (i=0; i<info->node[id].used_in; i++)
			if (info->node[id].in_ids[i] == value)
				return 1;
	}
	else{
		for (i=0; i<info->node[id].used_out; i++)
			if (info->node[id].out_ids[i] == value)
				return 1;
	}
	return 0;
}

/**
 * Adiciona um ID à lista de ids de um nodo
 * @param info  Estrutura
 * @param id    ID do nodo
 * @param value ID do nodo a ligar a id
 * @param c     Se o ID liga à entrada ("in") ou à saída ("out")
 */
void addID(ControllerInfo info, int id, int value, char* c){
	if (!strcmp(c, "in")){
		if (!findValue(info, id, value, "in")){
			info->node[id-1].out_ids[info->node[id-1].used_in] = value;
			info->node[id-1].used_in++;
			if (info->node[id-1].used_in == info->node[id-1].size_in)
				reallocNodeIds(info, id-1, "in");
		}
	}
	else{
		if (!findValue(info, id, value, "out")){
			info->node[id-1].out_ids[info->node[id-1].used_out] = value;
			info->node[id-1].used_out++;
			if (info->node[id-1].used_out == info->node[id-1].size_out)
				reallocNodeIds(info, id-1, "out");
		}
	}
}

/**
 * Irá ser responsável por executar um comando de um nodo
 * @param info Estrutura
 * @param id   ID do nodo
 */
void execNode(ControllerInfo info, int id){
	int f_in, f_out;

	f_in = open(info->node[id-1].pipeIn_name, O_RDONLY);
	dup2(f_in, 0);
	
	f_out = open(info->node[id-1].pipeOut_name, O_WRONLY);
	printf("id - %d; f_out - %d\n",id, f_out);
	if (f_out != -1)
		dup2(f_out, 1);

	execlp(info->node[id-1].cmd, info->node[id-1].cmd, info->node[id-1].args, NULL);
}

/**
 * Cria um novo nodo
 * @param info Estrutura
 * @param c    Array com informação do nodo
 */
void createNode(ControllerInfo info, char** c){
	int nodeID = atoi(c[1]);
	info->node[nodeID-1].id = nodeID;
	info->node[nodeID-1].cmd = addCommandPrefix(c[2]);
	info->node[nodeID-1].args = strcatWithSpaces(c+3);
	info->used++;

	char* c1 = fifoName(nodeID, "in");
	char* c2 = fifoName(nodeID, "out");

	info->node[nodeID-1].pipeIn_name = c1;
	info->node[nodeID-1].pipeOut_name = c2;

	mkfifo(c1, 0666);

	int pd[2];
	pipe(pd);
	int x = fork();
		
	if (!x) {
		execNode(info, nodeID);
		_exit(0);
	}

	info->node[nodeID-1].pid = x;
}

/**
 * Irá enviar um input para um nodo
 * @param info Estrutura
 * @param c    Array com o comando a executar
 */
void inject(ControllerInfo info, char** c){
	int nodeID = atoi(c[1]), charsRead = 0;
	char* cmd = addCommandPrefix(c[2]);
	char* args = strcatWithSpaces(c+3);
	char buffer[SIZE_BUF];

	int f_in, pd1[2];
	pipe(pd1);

	if (!fork()){
		close(pd1[1]);
		dup2(pd1[0], 0);
		f_in = open(info->node[nodeID-1].pipeIn_name, O_WRONLY);
		dup2(f_in, 1);
		execlp(cmd, cmd, args, NULL);
		close(pd1[0]);
		_exit(0);
	}
	else{
		close(pd1[0]);
		while ((charsRead = readline(0, buffer, SIZE_BUF)) > 0){
			write(pd1[1], buffer, charsRead);
			memset(buffer, 0, charsRead);
		}
		close(pd1[1]);
	}
}

/**
 * Irá ligar 1 ou mais nodos
 * @param info Estrutura
 * @param c    Informação com as ligações
 */
void connect(ControllerInfo info, char** c){
	int in = atoi(c[1]), i, x, j;
	for (i=2; c[i]; i++){
		int out = atoi(c[i]);
		addID(info, in, out, "out");
		addID(info, out, in, "in");

		char buffer[SIZE_BUF];
		int charsRead = 0;

		x = fork();
		if (!x){
			mkfifo(info->node[in-1].pipeOut_name, 0666);
			int f_out = open(info->node[in-1].pipeOut_name, O_RDONLY);
			
			int fildes[info->node[in-1].used_out];
			if (info->node[in-1].used_out > 1){
				kill(info->node[in-1].connect_pid, SIGINT);
				for (j=0; j<info->node[in-1].used_out; j++){
					int f = info->node[in-1].out_ids[j];
					fildes[j] = open(info->node[f-1].pipeIn_name, O_WRONLY);
				}
			}
			else fildes[0] = open(info->node[out-1].pipeIn_name, O_WRONLY);


			while ((charsRead = readline(f_out, buffer, SIZE_BUF)) > 0){
				for(j=0; j<info->node[in-1].used_out; j++)
					write(fildes[j], buffer, charsRead);
				memset(buffer, 0, charsRead);
			}
			_exit(0);
		}

		info->node[in-1].connect_pid = x;
	}
}

/**
 * Identifica o comando lido e chama a função adqueada
 * @param buf  Buffer com o input
 * @param info Estrutura
 */
void readCommand(char *buf, ControllerInfo info){
	char **c = divideString(buf, " ");

	if (!strcmp(c[0], "node"))
		createNode(info, c);

	else if (!strcmp(c[0], "connect"))
		connect(info, c);

	if (!strcmp(c[0], "inject"))
		inject(info, c);

	if (info->used == info->size)
		reallocControllerInfo(info);
}


/**
 * Função main de controller. Irá ler o input e pasar a readCommand
 * @param  argc Número de argumentos
 * @param  argv Argumentos
 * @return  --
 */
int main(int argc, char *argv[]){
	int charsRead;
	char buf[4096];
	ControllerInfo info = initControllerInfo(20);

	//read file
	if (argc > 1){
		int f = open(argv[1], O_RDONLY);
		while ((charsRead = readline(f, buf, SIZE_BUF)) > 0){
			readCommand(buf, info);
			memset(buf, 0, charsRead);
		}
		close(f);
	}

	//read stdin
	while ((charsRead = readline(0, buf, SIZE_BUF)) > 0){
			readCommand(buf, info);
			memset(buf, 0, charsRead);
	}

	return 0;
}