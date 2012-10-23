#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#define LINE_SIZE 128

typedef struct {
	int cabinet;
	double * score;
} document;

struct {
	int cabinet;
	int document;
	int subject;
	FILE * in;
	FILE * out;
	document * set;
} info;

int handleIO(char * filename){
	char *outfile = alloca(strlen(filename));

	if((info.in = fopen(filename, "r")) == NULL)
		return -2;

	outfile = strcat(strtok(strcpy(outfile, filename), "."), ".out");
	
	if((info.out = fopen(outfile, "w")) == NULL)
		return -3;

	return 0;
}

int cleanup(){
	if(fclose(info.in) == EOF)
		return -5;

	if(fclose(info.out) == EOF)
		return -6;

	free(info.set);

	return 0;
}

int init(){
	int i = 0, id = 0;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.set = (document *) calloc(info.document, sizeof(document)*info.document);

	for(; i < info.document; i++){
		info.set[i].cabinet = i % info.cabinet;
		info.set[i].score = (double *) calloc(info.subject, sizeof(double)*info.subject);
	}
	
	for(; fgets(line, LINE_SIZE, info.in)!= NULL;) {
		id = atoi(strtok(line, " "));
		for(i = 0; i < info.subject && (token = strtok(NULL, " ")) != NULL; i++)
			info.set[id].score[i] = atof(token);
	}
	
	return 0;
}

int process(){
	int i = 0, id = 0, tmp = 0;

	for(; id < info.document; id++, i = 0, tmp = 0){
		for(; i < info.subject; i++){
			tmp += (info.set[id].score[i] - info.set[id].cabinet)*(info.set[id].score[i] - info.set[id].cabinet);
		}
		info.set[id].cabinet = tmp % info.cabinet;
	}
	return 0;
}

int flushOutput(){
	int i = 0;
	for(; i < info.document; i++){
		fprintf(info.out, "%d %d\n", i, info.set[i].cabinet);
	}

	return 0;
}

int main(int argc, char** argv){
	int retValue;
	if(argc != 2 && argc != 3)
		return -1;
	
	if((retValue = handleIO(argv[1])) != 0)
		return retValue;

	if(fscanf(info.in, "%d\n %d\n %d\n", &info.cabinet, &info.document, &info.subject) != 3)
		return -4;

	if(argc == 3)
		info.cabinet = atoi(argv[2]);

	init();

	process();

	flushOutput();

	if((retValue = cleanup()) != 0)
		return retValue;

	return 0;
}
