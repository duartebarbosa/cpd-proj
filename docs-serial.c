#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LINE_SIZE 1024
#define CENTROID(x,y) centroid[(x) + (y) * info.cabinet]
#define QUAD(x) (x)*(x)

typedef struct {
	int cabinet;
	float * score;
} document;

struct {
	int cabinet;
	int document;
	int subject;
	FILE * in;
	FILE * out;
	document * set;
} info;

inline int minimum(float distance[]){
	register int cab = 0, index = 0;

	for(; cab < info.cabinet; cab++)
		if(distance[index] > distance[cab])
			index = cab;

	return index;
}

int process(){
	register int sub, doc, cab, tmp, flag = 1;
	register double distance, aux;
	int *docPerCab = calloc(info.cabinet, sizeof(int)); 			/* docPerCab[cabinet] */
	float *centroid = malloc(info.cabinet*info.subject*sizeof(float));	/* centroid[cabinet][subject] - centroid of the cabinet */

	while(flag){
		memset(docPerCab, 0, info.cabinet * sizeof(int));
		memset(centroid, 0, info.cabinet * info.subject * sizeof(float));
		
		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(info.set[doc].cabinet, sub) += info.set[doc].score[sub];
			docPerCab[info.set[doc].cabinet]++;
		}
		for(cab = 0; cab < info.cabinet; cab++){
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(cab, sub) /= docPerCab[cab]; 		/* actually compute the average */
		}

		/* calculate distance between cab and doc; set the new cabinet */
		for(flag = 0, doc = 0; doc < info.document; doc++){
			aux = HUGE_VALF;
			for(cab = 0; cab < info.cabinet; cab++){
				distance = 0;
				for(sub = 0; sub < info.subject; sub++)
					distance += QUAD(info.set[doc].score[sub] - CENTROID(cab, sub));
				if(distance < aux){
					tmp = cab;
					aux = distance;
				}
			}
			if(info.set[doc].cabinet != tmp){
				info.set[doc].cabinet = tmp;
				flag = 1;
			}
		}
	}
	
	free(docPerCab);
	free(centroid);
	
	return 0;
}

int cleanup(){
	register int doc = 0;

	for(; doc < info.document; doc++)
		free(info.set[doc].score);

	free(info.set);

	if(fclose(info.in) == EOF)
		return -5;
	if(fclose(info.out) == EOF)
		return -6;

	return 0;
}

int init(){
	register int sub, doc;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.set = (document *) calloc(info.document, sizeof(document));

	for(doc = 0; doc < info.document; doc++){
		info.set[doc].cabinet = doc % info.cabinet;
		info.set[doc].score = (float *) calloc(info.subject, sizeof(float));
	}
	
	while(fgets(line, LINE_SIZE, info.in)!= NULL){
		doc = atoi(strtok(line, " "));
		for(sub = 0; sub < info.subject && (token = strtok(NULL, " ")) != NULL; sub++)
			info.set[doc].score[sub] = atof(token);
	}
	
	return 0;
}

int handleIO(char * filename){
	char *outfile = alloca(strlen(filename));
	outfile = strcat(strtok(strcpy(outfile, filename), "."), ".out");
	
	if((info.in = fopen(filename, "r")) == NULL)
		return -2;
	
	if((info.out = fopen(outfile, "w")) == NULL)
		return -3;

	return 0;
}

int flushOutput(){
	register int doc = 0;
	for(; doc < info.document; doc++){
		fprintf(info.out, "%d %d\n", doc, info.set[doc].cabinet);
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
