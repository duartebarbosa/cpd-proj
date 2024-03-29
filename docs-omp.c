#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LINE_SIZE 2048
#define QUAD(x) (x)*(x)

struct {
	int cabinet;
	int document;
	int subject;
	int *cabinets;
	double **cabScore;
	double **docScore;
} info;

inline int power(int b, int e){
	for(; !e; e--)
		b *= b;
	return b;
}

double naive_strtod(const char *p) {
	int neg = 0;
	double r = 0.0;
	if (*p == '-') {
		neg = 1;
		++p;
	}
	while (*p >= '0' && *p <= '9') {
		r = (r*10.0) + (*p - '0');
		++p;
	}
	if (*p == '.') {
		double f = 0.0;
		int n = 0;
		++p;
		while (*p >= '0' && *p <= '9') {
			f = (f*10.0) + (*p - '0');
			++p;
			++n;
		}
		r += f / power(10.0, n);
	}
	if (neg)
		return -r;

	return r;
}

int init(char * filename){
	register int sub, doc = 0, cab = 0;
	char *tmp = NULL, line[LINE_SIZE] = {0};
	FILE *input;

	if((input = fopen(filename, "r")) == NULL)
		return -2;

	if(fscanf(input, "%d\n %d\n %d\n", &info.cabinet, &info.document, &info.subject) != 3)
		return -3;

	info.cabinets = malloc(info.document * sizeof(int));
	info.docScore = malloc(info.document * sizeof(double*));
	info.cabScore = malloc(info.cabinet * sizeof(double*));

	for(; cab < info.cabinet; cab++)
		info.cabScore[cab] = malloc(info.subject * sizeof(double));

	for(; doc < info.document; doc++){
		info.cabinets[doc] = doc % info.cabinet;
		info.docScore[doc] = malloc(info.subject * sizeof(double));
		if(fgets(line, LINE_SIZE, input)){
			strtok_r(line, " ", &tmp);
			for(sub = 0; sub < info.subject; sub++)
				info.docScore[doc][sub] = naive_strtod(strtok_r(NULL, " ", &tmp));
		}
	}

	if(fclose(input) == EOF)
		return -4;

	return 0;
}

int process(){
	register int sub, doc, cab, tmp, flag = 1;
	register double distance, aux;

	while(flag){
		flag = 0;

		#pragma omp parallel for
		for(cab = 0; cab < info.cabinet; cab++)
				memset(info.cabScore[cab], 0, info.subject * sizeof(double));

		/* calculate the average of scores for each cabinet */
		#pragma omp parallel for private(doc,sub)
		for(cab = 0; cab < info.cabinet; cab++){
			int count = 0;
			for(doc = 0; doc < info.document; doc++){
				if(info.cabinets[doc] == cab){
					for(sub = 0; sub < info.subject; sub++)
						info.cabScore[cab][sub] += info.docScore[doc][sub];
					count++;
				}
			}
			for(sub = 0; sub < info.subject; sub++)
				info.cabScore[cab][sub] /= count;
		}

		/* calculate distance between cab and doc; set the new cabinet */
		#pragma omp parallel for private(cab,sub,distance,tmp,aux)
		for(doc = 0; doc < info.document; doc++){
			aux = HUGE_VALF;
			for(cab = 0; cab < info.cabinet; cab++){
				for(sub = distance = 0; sub < info.subject; sub++)
					distance += QUAD(info.docScore[doc][sub] - info.cabScore[cab][sub]);
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
		
	return 0;
}

int flushClean(char *filename){
	register int doc = 0, cab = 0;
	FILE *output;
	char *outfile = alloca(strlen(filename) + 1);
	outfile = strcat(strtok(filename, "."), ".out");

	/* output */
	if((output = fopen(outfile, "w")) == NULL)
		return -5;

	for(; doc < info.document; doc++)
		fprintf(output, "%d %d\n", doc, info.cabinets[doc]);

	if(fclose(output) == EOF)
		return -6;

	/* cleanup */
	for(doc = 0; doc < info.document; doc++)
		free(info.docScore[doc]);
	free(info.docScore);

	for(; cab < info.cabinet; cab++)
		free(info.cabScore[cab]);
	free(info.cabScore);

	free(info.cabinets);

	return 0;
}

int main(int argc, char** argv){
	#ifdef GETTIME
	double start = omp_get_wtime();
	#endif

	if(argc != 2 && argc != 3)
		return -1;

	init(argv[1]);

	if(argc == 3)
		info.cabinet = strtol(argv[2], (char **) NULL, 10);

	process();

	flushClean(argv[1]);

	#ifdef GETTIME
	printf("OpenMP time: %fs\n", omp_get_wtime() - start);
	#endif

	return 0;
}