#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#define LINE_SIZE 128

char line[LINE_SIZE];

typedef struct {
	int cabinet;
	char * score;
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

int closeFiles(){
	if(fclose(info.in) == EOF)
		return -5;

	if(fclose(info.out) == EOF)
		return -6;

	return 0;
}

int init(){
	int i = 0;
	info.set = (document *) calloc(info.document, sizeof(document)*info.document);

	for(; i < info.document; i++){
		info.set[i].cabinet = i % info.cabinet;
	}

	/* test for initialization
	i = 0;
	for(; i < info.document; i++){
		printf("doc %d, cab %d\n", i, info.set[i].cabinet);
	}
	*/
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

	for(; fgets(line, LINE_SIZE, info.in) != NULL;) {
		fprintf(info.out, "%s", line);
	}

	printf("%d\n", info.cabinet);

	if((retValue = closeFiles()) != 0)
		return retValue;

	return 0;
}
