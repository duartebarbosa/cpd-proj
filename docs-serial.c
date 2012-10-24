#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#define LINE_SIZE 512

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
	int sub = 0, doc = 0;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.set = (document *) calloc(info.document, sizeof(document));

	for(; sub < info.document; sub++){
		info.set[sub].cabinet = sub % info.cabinet;
		info.set[sub].score = (double *) calloc(info.subject, sizeof(double));
	}
	
	for(; fgets(line, LINE_SIZE, info.in)!= NULL;) {
		doc = atoi(strtok(line, " "));
		for(sub = 0; sub < info.subject && (token = strtok(NULL, " ")) != NULL; sub++)
			info.set[doc].score[sub] = atof(token);
	}
	
	return 0;
}

int minimum(double distance[]){
	int sub = 0, cabinet = 0; double min = distance[0];
	for(; sub < info.cabinet; sub++){
		if(min > distance[sub]){
			min = distance[sub];
			cabinet = sub;
		}
	}
	return cabinet;
}

int process(){
	int sub = 0, doc = 0, cabinet = 0;
	double distance[info.cabinet] /* distance from specific doc to cabinet */, centroid[info.cabinet][info.subject]; /* centroid of the cabinet */

	/* centroid - average for each cabinet and subject */
	for(; doc < info.document; doc++){
		for(sub = 0; sub < info.subject; sub++){
			centroid[info.set[doc].cabinet][sub] += info.set[doc].score[sub];
		}
	}
	for(doc = 0; doc < info.document; doc++){
		for(sub = 0; sub < info.subject; sub++){
			centroid[info.set[doc].cabinet][sub] /= info.subject;
		}
	}

	/* calculate distance between cab and doc */
	for(sub = 0, doc = 0; doc < info.document; doc++, cabinet = 0){
		for(; cabinet < info.cabinet; cabinet++, sub = 0){
			for(; sub < info.subject; sub++){
				distance[cabinet] += (info.set[doc].score[sub] - centroid[cabinet][sub])*(info.set[doc].score[sub] - centroid[cabinet][sub]);
			}
		}
		info.set[doc].cabinet = minimum(distance);
		for(cabinet = 0; cabinet < info.cabinet; cabinet++){
			distance[cabinet] = 0;
		}
	}
	return 0;
}

int flushOutput(){
	int sub = 0;
	for(; sub < info.document; sub++){
		fprintf(info.out, "%d %d\n", sub, info.set[sub].cabinet);
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
