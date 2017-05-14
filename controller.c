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
}*ControllerNode;

typedef struct controllerInfo{
	int size;
	ControllerNode node;
}*ControllerInfo;

ControllerInfo initControllerInfo(int size){
	ControllerInfo info = malloc (sizeof(struct controllerInfo));
	info->size = size;
	info->node = malloc(sizeof(ControllerNode) * info->size);
	return info;
}

void reallocControllerInfo(ControllerInfo info){
	info->size *= 2;
	info->node = realloc(info->node, sizeof(struct controllerNode) * info->size);
}

//--------------------------//


void readCommand(char *buf, ControllerInfo info){
	char **c = divideString(buf, " ");
	int i;

	if (!strcmp(c[0], "Node")){
		int nodeID = atoi(c[1]);
		info->node[nodeID].id = nodeID;
		info->node[nodeID].cmd = addCommandPrefix(info->node[nodeID].cmd, c[2]);
		info->node[nodeID].args = strcatWithSpaces(c+3);

		execlp(info->node[nodeID].cmd, info->node[nodeID].cmd, info->node[nodeID].args, NULL);
	}
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
}