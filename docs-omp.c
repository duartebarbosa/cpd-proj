#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <omp.h>

#define LINE_SIZE 512
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
	register int doc;

	for(doc = 0; doc < info.document; doc++)
		free(info.set[doc].score);

	free(info.set);

	if(fclose(info.in) == EOF)
		return -5;

	if(fclose(info.out) == EOF)
		return -6;

	return 0;
}

int init(){
	register int sub = 0, id;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.set = (document *) calloc(info.document, sizeof(document));

	#pragma omp parallel for
	for(id = 0; id < info.document; id++){
		info.set[id].cabinet = id % info.cabinet;
		info.set[id].score = (float *) calloc(info.subject, sizeof(float));
	}
	
	while(fgets(line, LINE_SIZE, info.in)!= NULL){
		id = atoi(strtok(line, " "));
		for(sub = 0; sub < info.subject && (token = strtok(NULL, " ")) != NULL; sub++)
			info.set[id].score[sub] = atof(token);
	}
	
	return 0;
}

int minimum(float distance[]){
	register int cab = 0, result = 0;
	float min = distance[0];

	for(; cab < info.cabinet; cab++){
		if(min > distance[cab]){
			min = distance[cab];
			result = cab;
		}
	}
	return result;
}

int process(){
	register int sub = 0, doc = 0, cab = 0, tmp = 0, flag = 1;
	int *docPerCab = calloc(info.cabinet, sizeof(int)); 			/* docPerCab[info.cabinet] */
	float *distance = calloc(info.cabinet, sizeof(float)); 			/* distance[info.cabinet] - distance from specific doc to cabinet */
	float *centroid = malloc(info.cabinet*info.subject*sizeof(float));	/* centroid[info.cabinet][info.subject] - centroid of the cabinet */

	while(flag){
		flag = 0;
		memset(docPerCab, 0, info.cabinet * sizeof(int));
		memset(centroid, 0, info.cabinet * info.subject * sizeof(float));

		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			for(sub = 0; sub < info.subject; sub++){
				CENTROID(info.set[doc].cabinet, sub) += info.set[doc].score[sub];
			}
		}
		for(doc = 0; doc < info.document; doc++){
			docPerCab[info.set[doc].cabinet]++;
		}

		#pragma omp parallel for collapse(2)
		for(cab = 0; cab < info.cabinet; cab++){
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(cab, sub) /= docPerCab[cab]; /* actually compute the average */
		}

		/* calculate distance between cab and doc; set the new cabinet */
		for(doc = 0, sub = 0; doc < info.document; doc++){
			memset(distance, 0, info.cabinet * sizeof(float));
			for(cab = 0; cab < info.cabinet; cab++){
				for(sub = 0; sub < info.subject; sub++)
					distance[cab] += QUAD(info.set[doc].score[sub] - CENTROID(cab, sub));
			}
			tmp = info.set[doc].cabinet;
			info.set[doc].cabinet = minimum(distance);
			if(tmp != info.set[doc].cabinet)
				flag = 1;
		}
	}

	free(docPerCab);
	free(distance);
	free(centroid);
	
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
