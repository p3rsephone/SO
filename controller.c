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
	int size_in, used_in;
	int* out_ids;
	int size_out, used_out;
	char* pipeIn_name;
	char* pipeOut_name;
	int f_in, f_out;
}*ControllerNode;

typedef struct controllerInfo{
	int size;
	int used;
	int* pids;
	ControllerNode node;
}*ControllerInfo;

ControllerInfo initControllerInfo(int size){
	ControllerInfo info = malloc (sizeof(struct controllerInfo));
	info->size = size;
	info->used = 0;
	info->node = malloc(sizeof(struct controllerNode) * info->size);
	info->pids = malloc(sizeof(int) * info->size);

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

void reallocControllerInfo(ControllerInfo info){
	info->size *= 2;
	info->node = realloc(info->node, sizeof(struct controllerNode) * info->size);
	info->pids = realloc(info->pids, sizeof(int) * info->size);
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

/*
void forkNode(ControllerInfo info, int id){
	int f_in, f_out, pd1[2], pd2[2], charsRead = -1;
	//f_out = open(info->node[id-1].pipeOut_name, O_WRONLY);
	pipe(pd1);
	pipe(pd2);
	char buf[SIZE_BUF];

	int i, x;
	for (i=0; i<3; i++){
		if (!fork()){
			switch(i){
				case 0: {
					f_in = open(info->node[id-1].pipeIn_name, O_RDONLY);
					while ((charsRead = readline(f_in, buf, SIZE_BUF)) > 0){
						write(pd1[1], buf, charsRead);
					}
					close(pd1[1]);
					_exit(0);
				}
				case 1:	{
					//close(pd1[1]);
					//close(pd2[0]);
					dup2(pd1[0], 0);
					//dup2(pd2[1], 1);
					execlp(info->node[id].cmd, info->node[id].cmd, info->node[id].args, NULL);
					//close(pd1[0]);
					//close(pd2[1]);
					_exit(0);
				}
				case 2: {
					close(pd2[1]);
					while ((charsRead = readline(pd2[0], buf, SIZE_BUF)) > 0){
						write(1, buf, charsRead);
					}
					close(pd2[0]);
					_exit(0);
				}
			}
		}
	}
}
*/

void execNode(ControllerInfo info, int id){
	int f_in;

	f_in = open(info->node[id-1].pipeIn_name, O_RDONLY);

	//f_out = open(info->node[id-1].pipeOut_name, O_WRONLY);

	dup2(f_in, 0);
	//dup2(f_out, 1);

	execlp(info->node[id-1].cmd, info->node[id-1].cmd, info->node[id-1].args, NULL);
}


void readCommand(char *buf, ControllerInfo info){
	char **c = divideString(buf, " ");
	if (!strcmp(c[0], "node")){
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
		mkfifo(c2, 0666);

		int pd[2];
		pipe(pd);
		int x = fork();
		//filho
		if (!x){
			execNode(info, nodeID);
		}

		info->pids[nodeID-1] = x;


		//execlp(info->node[nodeID].cmd, info->node[nodeID].cmd, info->node[nodeID].args, NULL);
	}

/*
	else if (!strcmp(c[0], "connect")){
		int in = atoi(c[1]);
		for (i=2; c[i]; i++){
			int out = atoi(c[i]);
			addID(info, in, out, "out");
			addID(info, out, in, "in");
		}
	}
	*/


	if (!strcmp(c[0], "inject")){
		int nodeID = atoi(c[1]), charsRead = 0;
		char* cmd = addCommandPrefix(c[2]);
		char* args = strcatWithSpaces(c+3);
		char buffer[SIZE_BUF];

		int f_in, pd1[2];
		//f_out = open(info->node[nodeID].pipeOut_name, O_RDONLY);
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
		}
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
	

	while ((charsRead = readline(0, buf, SIZE_BUF)) > 0){
			readCommand(buf, info);
			memset(buf, 0, charsRead);
		}

		
		int i;
		for (i=0; i<info->used; i++){
			printf("Node - %d\n", info->node[i].id);
			printf("Command - %s\n", info->node[i].cmd);
			printf("Args - %s\n", info->node[i].args);
		}
		printf("%d\n", info->node[0].out_ids[0]);
		
}
