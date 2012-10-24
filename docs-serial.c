#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#define LINE_SIZE 512

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
	int doc = 0;
	if(fclose(info.in) == EOF)
		return -5;

	if(fclose(info.out) == EOF)
		return -6;

	for(; doc < info.document; doc++){
		free(info.set[doc].score);
	}

	free(info.set);

	return 0;
}

int init(){
	int sub = 0, doc = 0;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.set = (document *) calloc(info.document, sizeof(document));

	for(; sub < info.document; sub++){
		info.set[sub].cabinet = sub % info.cabinet;
		info.set[sub].score = (float *) calloc(info.subject, sizeof(float));
	}
	
	for(; fgets(line, LINE_SIZE, info.in)!= NULL;) {
		doc = atoi(strtok(line, " "));
		for(sub = 0; sub < info.subject && (token = strtok(NULL, " ")) != NULL; sub++)
			info.set[doc].score[sub] = atof(token);
	}
	
	return 0;
}

int minimum(float distance[]){
	int cab = 0, result = 0;
	float min = distance[0];

	for(; cab < info.cabinet; cab++){
		if(min > distance[cab]){
			min = distance[cab];
			result = cab;
		}
		printf("\tresult: %d, cab: %d, min: %f, distance[cab]: %f\n", result, cab, min, distance[cab]);
	}
	return result;
}

int process(){
	int i = 0,sub = 0, doc = 0, cab = 0;
	float distance[info.cabinet] /* distance from specific doc to cabinet */, centroid[info.cabinet][info.subject]; /* centroid of the cabinet */
	while(i < info.cabinet){
		printf("i: %d\n", i);
		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			for(sub = 0; sub < info.subject; sub++){
				centroid[info.set[doc].cabinet][sub] += info.set[doc].score[sub];
			}
		}
		for(cab = 0; cab < info.cabinet; cab++){
			for(sub = 0; sub < info.subject; sub++){

				/**
				 * FIXME
				 *
				 * Para cada cabinet c e subject s, estamos a obter centroid[c][s]
				 * dividindo o somatório dos score[s] dos documents que estão nesse
				 * cabinet pelo número de subjects total no problema. Ora, isso
				 * assim não é um cálculo de média / centro de massa / whatever. O
				 * que se quer é dividir o somatório pela quantidade de elementos
				 * somados, i.e., dividir pelo número de documents contidos em c.
				 * Certo? Ou estou a delirar?
				 */

				centroid[cab][sub] /= info.subject; /* actually compute the average */
			}
		}

		/* calculate distance between cab and doc */
		for(doc = 0, sub = 0; doc < info.document; doc++){
			for(cab = 0; cab < info.cabinet; cab++){
				for(sub = 0; sub < info.subject; sub++){
					distance[cab] += (info.set[doc].score[sub] - centroid[cab][sub])*(info.set[doc].score[sub] - centroid[cab][sub]);
				}
			}
			printf("doc: %d\n", doc);
			info.set[doc].cabinet = minimum(distance);
			for(cab = 0; cab < info.cabinet; cab++){
				distance[cab] = 0;
			}
		}

		for(cab = 0; cab < info.cabinet; cab++){
			printf("\tcab: %d\n", cab);
			for(sub = 0; sub < info.subject; sub++){
				printf("\t\tsub: %d, centroid: %f\n", sub, centroid[cab][sub]);
			}
		}

		for(cab = 0; cab < info.cabinet; cab++){
			for(sub = 0; sub < info.subject; sub++){
				centroid[cab][sub] = 0; /* re-initialize centroid */
			}
		}

	i++;
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
