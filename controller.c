#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "stringProcessing.h"

#define SIZE_BUF 4096

//----------Struct----------//

typedef struct controllerNode{
	int id;
	char* cmd;
	char* args;
	int* in_ids;
	int size_in;
	int* out_ids;
	int size_out;
	char* pipeIn_name;
	char* pipeOut_name;
}*ControllerNode;

typedef struct controllerInfo{
	int size;
	int used;
	ControllerNode node;
}*ControllerInfo;

ControllerInfo initControllerInfo(int size){
	ControllerInfo info = malloc (sizeof(struct controllerInfo));
	info->size = size;
	info->used = 0;
	info->node = malloc(sizeof(struct controllerNode) * info->size);

	int i;
	for (i=0; i<size; i++){
		info->node[i].size_in = 5;
		info->node[i].size_out = 5;
		info->node[i].in_ids = malloc (sizeof(int) * info->node[i].size_in);
		info->node[i].out_ids = malloc (sizeof(int) * info->node[i].size_out);
	}

	return info;
}

void reallocControllerInfo(ControllerInfo info){
	info->size *= 2;
	info->node = realloc(info->node, sizeof(struct controllerNode) * info->size);
}

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

//--------------------------//


void readCommand(char *buf, ControllerInfo info){
	char **c = divideString(buf, " ");
	int i;

	if (!strcmp(c[0], "node")){
		int nodeID = atoi(c[1]) - 1;
		info->node[nodeID].id = nodeID;
		info->node[nodeID].cmd = addCommandPrefix(info->node[nodeID].cmd, c[2]);
		info->node[nodeID].args = strcatWithSpaces(c+3);

		info->used++;
		//execlp(info->node[nodeID].cmd, info->node[nodeID].cmd, info->node[nodeID].args, NULL);
	}

	else if (!strcmp(c[0], "connect")){

	}

	if (info->used == info->size)
		reallocControllerInfo(info);
}


int main(int argc, char *argv[]){
	int charsRead;
	char buf[4096];
	ControllerInfo info = initControllerInfo(20);
	//ler de um ficheiro
	/*
	if (argc > 1){
		FILE *f = fopen(argv[1], "r");
		char *line = NULL;
		size_t size = 0;
		while ((charsRead = getline(&line, &size, f)) > 0){
			printf("%s", line);
		}
	}
	*/
	//ler do stdin
	//else
		while ((charsRead = read(0, buf, SIZE_BUF)) > 0){
			readCommand(buf, info);
			memset(buf, 0, charsRead);
		}

		/*
		int i;
		for (i=0; i<info->used; i++){
			printf("Node - %d\n", info->node[i].id);
			printf("Command - %s\n", info->node[i].cmd);
			printf("Args %s\n", info->node[i].args);
		}
		*/

}