#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LINE_SIZE 1024
#define CENTROID(x,y) centroid[(x) + (y) * info.cabinet]
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

inline int power(int b, int e){
	register int res = 1;
	for(; e != 0; e--)
		res *= b;
	return res;
}

double naive_strtod(const char *p) {
    double r = 0.0;
    int neg = 0;
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
	register int sub, doc;
	char *tmp = NULL, line[LINE_SIZE] = {0};
	
	if((info.in = fopen(filename, "r")) == NULL)
		return -2;
	
	if(fscanf(info.in, "%d\n %d\n %d\n", &info.cabinet, &info.document, &info.subject) != 3)
		return -4;
	
	info.cabinets = malloc(info.document*sizeof(int));
	info.score = malloc(info.document*sizeof(double*));

	for(doc = 0; doc < info.document; doc++){
		info.cabinets[doc] = doc % info.cabinet;
		info.score[doc] = malloc(info.subject*sizeof(double));
		if(fgets(line, LINE_SIZE, info.in)!= NULL){
			strtok_r(line, " ", &tmp);
			for(sub = 0; sub < info.subject; sub++)
				info.score[doc][sub] = naive_strtod(strtok_r(NULL, " ", &tmp));
		}
	}	
	return 0;
}

int process(){
	register int sub, doc, cab, tmp, flag = 1;
	register double distance, aux;
	int *docPerCab = malloc(info.cabinet*sizeof(int)); 			/* docPerCab[cabinet] */
	double *centroid = malloc(info.cabinet*info.subject*sizeof(double));	/* centroid[cabinet][subject] - centroid of the cabinet */

	while(flag){
		memset(docPerCab, 0, info.cabinet * sizeof(int));
		memset(centroid, 0, info.cabinet * info.subject * sizeof(double));
		
		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(info.cabinets[doc], sub) += info.score[doc][sub];
			docPerCab[info.cabinets[doc]]++;
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
					distance += QUAD(info.score[doc][sub] - CENTROID(cab, sub));
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
	free(centroid);
	
	return 0;
}

int flushClean(char *filename){
	register int doc = 0;
	char *outfile = alloca(strlen(filename)+1);
	outfile = strcat(strtok(strcpy(outfile, filename), "."), ".out");

	if(fclose(info.in) == EOF)
		return -5;
	
	/* output */
	if((info.out = fopen(outfile, "w")) == NULL)
		return -3;

	for(; doc < info.document; doc++){
		fprintf(info.out, "%d %d\n", doc, info.cabinets[doc]);
	}

	if(fclose(info.out) == EOF)
		return -6;

	/* cleanup */
	for(doc = 0; doc < info.document; doc++)
		free(info.score[doc]);

	free(info.score);
	free(info.cabinets);

	return 0;
}

int main(int argc, char** argv){
	double start = omp_get_wtime();
	if(argc != 2 && argc != 3)
		return -1;

	if(argc == 3)
		info.cabinet = atoi(argv[2]);

	init(argv[1]);

	process();

	flushClean(argv[1]);

	printf("OpenMP time: %fs\n", omp_get_wtime() - start);

	return 0;
}
