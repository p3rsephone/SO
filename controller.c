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
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <limits.h>
#include "stringProcessing.h"

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
	int nnodes; ///< número de nodes até ao momento
	int nconnects; ///< número de connects até ao momento
	ControllerNode node; ///< array de nodos
}*ControllerInfo;

/**
 * \brief Inicializa um ControllerInfo
 * @param  size Tamanho inicial
 * @return Nova estrutra
 */
ControllerInfo initControllerInfo(int size){
	ControllerInfo info = malloc (sizeof(struct controllerInfo));
	info->size = size;
	info->used = 0;
	info->nconnects = 0;
	info->node = malloc(sizeof(struct controllerNode) * info->size);

	int i;
	for (i=0; i<size; i++){
		info->node[i].id = -1;
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
 * \brief Realoca memória para a estrutra
 * @param info Estrutura
 */
void reallocControllerInfo(ControllerInfo info){
	info->size *= 2;
	info->node = realloc(info->node, sizeof(struct controllerNode) * info->size);
}

/**
 * \brief Realoca memória para o array de ids de um nodo
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
 * \brief Procura se um determinado ID já liga a um nodo
 * @param  info  Estrutura
 * @param  id    ID do nodo
 * @param  value ID do nodo que liga a id
 * @param  c     Se o ID liga à entrada ("in") ou à saída ("out")
 * @return       Já existe um value a ligar ao id (true) ou não (false)
 */
int findValue(ControllerInfo info, int id, int value, char* c){
	int i;
	if (id >= info->size || value >= info->size)
		return 0;

	if (!strcmp(c, "in")){
		for (i=0; i<info->node[id-1].used_in; i++)
			if (info->node[id].in_ids[i] == value)
				return 1;
	}
	else{
		for (i=0; i<info->node[id-1].used_out; i++)
			if (info->node[id-1].out_ids[i] == value)
				return 1;
	}
	return 0;
}

/**
 * \brief Adiciona um ID à lista de ids de um nodo
 * @param info  Estrutura
 * @param id    ID do nodo
 * @param value ID do nodo a ligar a id
 * @param c     Se o ID liga à entrada ("in") ou à saída ("out")
 */
void addID(ControllerInfo info, int id, int value, char* c){
	if (!strcmp(c, "in")){
		if (!findValue(info, id, value, "in")){
			info->node[id-1].in_ids[info->node[id-1].used_in] = value;
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
 * \brief Remove um id de um array e o respetivo descritor de ficheiro
 * @param out_ids Array com os ids
 * @param fildes  Array com os descritores
 * @param size    Tamanho dos arrays
 * @param id      ID a remover
 */
void removeFildes(int* out_ids, int* fildes, int *size, int id){
	int i;
	for (i=0; i<*size-1; i++)
		if (id == out_ids[i]){
			out_ids[i] = out_ids[i+1];
			fildes[i] = fildes[i+1];
			id = out_ids[i+1];
		}
	*size = *size - 1;
}

/** \brief Nome do pipe de saída */
static char* exec_pipe_out_name;
/** \brief ID do processo que executa o exec */
static int exec_child_pid;
/** \brief Diz se o node será removido */
static int exec_remove = 0;

/**
 * \brief Trata dos sinais recebidos pelo execNode
 * @param sig Sinal
 */
void nodeHandler(int sig){
	switch(sig){
		//Abrir o pipe de saída e começar a escrever para lá
		case SIGUSR1 :{
			int f_out = open(exec_pipe_out_name, O_WRONLY);
			if (f_out != -1) 
				dup2(f_out, 1);
			break;
		}
		//Terminar o exec e sair
		case SIGINT :{
			if (exec_remove == 1) 
				_exit(0);
			kill(exec_child_pid, SIGINT);
			break;
		}
		//Remover o nodo atual
		case SIGUSR2 :{
			raise(SIGINT);
			exec_remove = 1;
			break;
		}
	}
}

/**
 * \brief Irá ser responsável por executar um comando de um nodo
 * @param info Estrutura
 * @param id   ID do nodo
 */
void execNode(ControllerInfo info, int id){
	signal(SIGUSR1, nodeHandler);
	signal(SIGINT, nodeHandler);
	signal(SIGUSR2, nodeHandler);
	int pd_out[2], charsRead, x, f_in;
	char buffer[PIPE_BUF];
	pipe(pd_out);
	exec_pipe_out_name = info->node[id-1].pipeOut_name;
	kill(getppid(), SIGUSR1);
	f_in = open(info->node[id-1].pipeIn_name, O_RDONLY);

	//exec command
	if ((x = fork()) == 0){
		dup2(pd_out[1], 1);
		close(pd_out[0]);
		dup2(f_in, 0);
		execlp(info->node[id-1].cmd, info->node[id-1].cmd, info->node[id-1].args, NULL);
	}
	else{
		close(pd_out[1]);
		exec_child_pid = x;
		while((charsRead = readline(pd_out[0], buffer, PIPE_BUF)) > 0){
			write(1, buffer, charsRead);
		}
		//Irá fazer a ligação entre os diferentes connects, caso o nó seja removido
		if (exec_remove){
			while ((charsRead = readline(f_in, buffer, PIPE_BUF)) > 0)
				write(1, buffer, charsRead);
		}
		_exit(0);
	}
}

/**
 * \brief Apenas servirá para quando um node é substituido, 
 * recebendo o sinal do novo node dizendo que pode remover o anterior (createNode)
 * ou para o connect sinalizar que recebeu e processou um bit (disconnect)
 * @param sig Sinal
 */
void default_handler(int sig){}

/**
 * \brief Cria um novo nodo (ou substitui um existente)
 * @param info Estrutura
 * @param c    Array com informação do nodo
 */
void createNode(ControllerInfo info, char** c){
	signal(SIGUSR1, default_handler);
	int nodeID = atoi(c[1]), new_connect = 0, old_pid;

	info->node[nodeID-1].cmd = addCommandPrefix(c[2]);
	info->node[nodeID-1].args = strcatWithSpaces(c+3);

	//Se o nodo já existe, irá substituir-lo com o novo comando
	if (info->node[nodeID-1].id != -1){
		new_connect = 1;
		old_pid = info->node[nodeID-1].pid;
	}
	//Se o nodo não existe, cria um novo
	else{
		info->node[nodeID-1].id = nodeID;
		info->used++;
		info->node[nodeID-1].pipeIn_name = fifoName(info->nnodes, "in");
		info->node[nodeID-1].pipeOut_name = fifoName(info->nconnects, "out");
		info->nnodes++;
		info->nconnects++;
		mkfifo(info->node[nodeID-1].pipeIn_name, 0666);
	}

	int pd[2], x;
	pipe(pd);

	//exec node
	if ((x = fork()) == 0) execNode(info, nodeID);

	else{
		pause();
		info->node[nodeID-1].pid = x;
		if (new_connect){
			kill(x, SIGUSR1);
			kill(old_pid, SIGINT);
		}
	}
}

/**
 * \brief Irá enviar um input para um nodo
 * @param info Estrutura
 * @param c    Array com o comando a executar
 */
void inject(ControllerInfo info, char** c){
	int nodeID = atoi(c[1]), charsRead = 0;
	char* cmd = addCommandPrefix(c[2]);
	char* args = strcatWithSpaces(c+3);
	char buffer[PIPE_BUF];

	int f_in, pd1[2];
	pipe(pd1);

	if (!fork()){
		close(pd1[1]);
		dup2(pd1[0], 0);
		f_in = open(info->node[nodeID-1].pipeIn_name, O_WRONLY);
		dup2(f_in, 1);
		close(f_in);
		execlp(cmd, cmd, args, NULL);
		close(pd1[0]);
		_exit(0);
	}
	else{
		close(pd1[0]);
		while ((charsRead = readline(0, buffer, PIPE_BUF)) > 0){
			write(pd1[1], buffer, charsRead);
			memset(buffer, 0, charsRead);
		}
	}
}

/** \brief Indica se o disconnect foi concluído */
static int disconnect_done = 0;
/** \brief Indica o número de interações já realizadas */
static int disconnect_iterations = 0;
/** \brief Indica o id até ao momento */
static int disconnect_id = 0;

/**
 * \brief Tratará dos sinais do connect (receberá os bits de disconnect)
 * @param sig Sinal
 */
void connect_handler(int sig){
	if (disconnect_done == 1){
		disconnect_done = 0;
		disconnect_iterations = 0;
		disconnect_id = 0;
	}
	switch(sig){
		//Bit 1
		case SIGUSR1 :{
			disconnect_id += pow(2, disconnect_iterations);
			disconnect_iterations ++;
			break;
		}
		//Bit 0
		case SIGUSR2 :{
			disconnect_iterations++;
			break;
		}
		//Done
		case SIGHUP :{
			disconnect_done = 1;
			break;
		}
	}
	//Diz que pode receber um novo sinal
	kill(getppid(), SIGUSR1);
}

/**
 * \brief Irá ligar um nodo a um ou mais nodos
 * @param info Estrutura
 * @param c    Informação com as ligações
 */
void connect(ControllerInfo info, char** c){
	signal(SIGUSR1, connect_handler);
	signal(SIGUSR2, connect_handler);
	signal(SIGHUP, connect_handler);
	int in = atoi(c[1]), i, x, j;
	for (i=2; c[i]; i++){
		int out = atoi(c[i]);
		addID(info, in, out, "out");
		addID(info, out, in, "in");

		char buffer[PIPE_BUF];
		int charsRead = 0;

		x = fork();
		if (!x){
			//Criar um fifo e avisar ao nodo para escrever a sua saída para um pipe
			mkfifo(info->node[in-1].pipeOut_name, 0666);
			kill(info->node[in-1].pid, SIGUSR1);
			int f_out = open(info->node[in-1].pipeOut_name, O_RDONLY);
			int fildes[info->node[in-1].used_out];

			//Se já existe uma ligação, irá terminar-la e criar uma
			//nova com os nodos anteriores + novo
			if (info->node[in-1].used_out > 1){
				kill(info->node[in-1].connect_pid, SIGINT);
				for (j=0; j<info->node[in-1].used_out; j++){
					int f = info->node[in-1].out_ids[j];
					fildes[j] = open(info->node[f-1].pipeIn_name, O_WRONLY);
				}
			}
			//Se não existe ligação apenas abre o novo descitor
			else fildes[0] = open(info->node[out-1].pipeIn_name, O_WRONLY);

			//Ler da saída do nodo
			while ((charsRead = readline(f_out, buffer, PIPE_BUF)) > 0){
				//Remover um nodo da saída (disconnect)
				if (disconnect_done){
					removeFildes(info->node[in-1].out_ids, fildes, &(info->node[in-1].used_out), disconnect_id);
					disconnect_done = 0;
				}
				//Escrever para os nodos que estejam ligados a este
				for(j=0; j<info->node[in-1].used_out; j++){
					write(fildes[j], buffer, charsRead);
				}
				memset(buffer, 0, charsRead);
			}
			_exit(0);
		}
		info->node[in-1].connect_pid = x;
	}
}

/**
 * \brief Irá remover a ligação entre dois nós
 * @param info Estrutura
 * @param c    Array de strings com os ids
 */
void disconnect(ControllerInfo info, char** c){
	signal(SIGUSR1, default_handler);
	int id1, id2;
	id1 = atoi(c[1]);
	id2 = atoi(c[2]);

	//Verifica se a ligação está trocada (id2 -> id1 em vez de id1 -> id2)
	if (!findValue(info, id1, id2, "out")){
		if (findValue(info, id2, id1, "out")){
			int aux = id1;
			id1 = id2;
			id2 = aux;
		}
		//A ligação não existe
		else return;
	}

	//Enviar os bits do id a desligar
	int id1_connect = info->node[id1-1].connect_pid;
	for (; id2 > 0; id2 = id2 >> 1){
		if (id2 % 2) kill(id1_connect, SIGUSR1); //Bit 1
		else kill(id1_connect, SIGUSR2); //Bit 0
		pause(); //Fica à espera para enviar um novo bit
	}
	//Já enviou os bits todos
	kill(id1_connect, SIGHUP);
}

/**
 * \brief Remove um nó da rede
 * @param info Estrutura
 * @param c    Arary de strings com o nodo a remover
 */
void removeNode(ControllerInfo info, char** c){
	int id = atoi(c[1]);
	info->node[id-1].id = -1;
	//Diz ao node que pode parar o exec
	kill(info->node[id-1].pid, SIGUSR2);
}

/**
 * \brief Termina o programa
 */
void quit(){
	system("pkill -f controller");
}

/**
 * \brief Identifica o comando lido e chama a função adqueada
 * @param buf  Buffer com o input
 * @param info Estrutura
 */
void readCommand(char *buf, ControllerInfo info){
	char **c = divideString(buf, " ");

	//node id cmd args
	if (!strcmp(c[0], "node"))
		createNode(info, c);

	//connect id1 id2
	if (!strcmp(c[0], "connect"))
		connect(info, c);

	//inject id1 cmd args
	if (!strcmp(c[0], "inject"))
		inject(info, c);

	//disconnect id1 id2
	if (!strcmp(c[0], "disconnect"))
		disconnect(info, c);

	//remove id
	if (!strcmp(c[0], "remove"))
		removeNode(info, c);

	//quit
	if (!strcmp(c[0], "quit\n"))
		quit(info, c);

	if (info->used == info->size)
		reallocControllerInfo(info);
}

/**
 * Função main de controller. Irá ler o input e passar a readCommand
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
		while ((charsRead = readline(f, buf, PIPE_BUF)) > 0){
			readCommand(buf, info);
			memset(buf, 0, charsRead);
		}
		close(f);
	}

	//read stdin
	while ((charsRead = readline(0, buf, PIPE_BUF)) > 0){
			readCommand(buf, info);
			memset(buf, 0, charsRead);
	}
	quit();
}