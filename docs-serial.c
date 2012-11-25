#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LINE_SIZE 1024
#define QUAD(x) (x)*(x)

struct {
	int cabinet;
	int document;
	int subject;
	int *cabinets;
	double **score;
	FILE *in;
	FILE *out;
} info;

int handleIO(char * filename){
	char *outfile = alloca(strlen(filename));
	outfile = strcat(strtok(strcpy(outfile, filename), "."), ".out");
	
	if((info.in = fopen(filename, "r")) == NULL)
		return -2;
	
	if((info.out = fopen(outfile, "w")) == NULL)
		return -3;

	return 0;
}

int init(){
	register int sub, doc;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.score = (double **) calloc(info.document, sizeof(double*));
	info.cabinets = (int *) calloc(info.document, sizeof(int));

	for(doc = 0; doc < info.document; doc++){
		info.cabinets[doc] = doc % info.cabinet;
		info.score[doc] = (double *) calloc(info.subject, sizeof(double));
	}

	while(fgets(line, LINE_SIZE, info.in)!= NULL){
		doc = atoi(strtok(line, " "));
		for(sub = 0; sub < info.subject && (token = strtok(NULL, " ")) != NULL; sub++)
			info.score[doc][sub] = atof(token);
	}
	
	return 0;
}


int process(){
	register int sub, doc, cab, tmp, flag = 1;
	register double distance, aux;
	int *docPerCab = calloc(info.cabinet, sizeof(int)); 		/* docPerCab[cabinet] */
	double **centroid = calloc(info.subject, sizeof(double*));	/* centroid[cabinet][subject] - centroid of the cabinet */

	for(sub = 0; sub < info.subject; sub++)
		centroid[sub] = calloc(info.cabinet, sizeof(double));

	while(flag){
		memset(docPerCab, 0, info.cabinet * sizeof(int));

		for(sub = 0; sub < info.subject; sub++)
			memset(centroid[sub], 0, info.subject * sizeof(double));
		
		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			for(sub = 0; sub < info.subject; sub++)
				centroid[sub][info.cabinets[doc]] += info.score[doc][sub];
			docPerCab[info.cabinets[doc]]++;
		}
		for(sub = 0; sub < info.subject; sub++)
			for(cab = 0; cab < info.cabinet; cab++){
				centroid[sub][cab] /= docPerCab[cab]; 		/* actually compute the average */
		}

		/* calculate distance between cab and doc; set the new cabinet */
		for(flag = 0, doc = 0; doc < info.document; doc++){
			aux = HUGE_VALF;
			for(cab = 0; cab < info.cabinet; cab++){
				distance = 0;
				for(sub = 0; sub < info.subject; sub++)
					distance += QUAD(info.score[doc][sub] - centroid[sub][cab]);
				if(distance < aux){
					tmp = cab;
					aux = distance;
				}
			}
			if(info.cabinets[doc] != tmp){
				info.cabinets[doc] = tmp;
				flag = 1;
			}
		}
	}
	
	free(docPerCab);
		
	return 0;
}

int cleanup(){
	register int doc = 0;

	for(; doc < info.document; doc++)
		free(info.score[doc]);

	free(info.cabinets);

	if(fclose(info.in) == EOF)
		return -5;
	if(fclose(info.out) == EOF)
		return -6;

	return 0;
}

int flushOutput(){
	register int doc = 0;
	for(; doc < info.document; doc++){
		fprintf(info.out, "%d %d\n", doc, info.cabinets[doc]);
	}
	return 0;
}

int main(int argc, char** argv){
	int retValue;
	double start = omp_get_wtime();
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

	printf("OpenMP time: %fs\n", omp_get_wtime() - start);

	return 0;
}
