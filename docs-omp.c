#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "core.c"

int process(){
	register int sub, doc, cab, tmp, flag = 1;
	int *docPerCab = calloc(info.cabinet, sizeof(int)); 			/* docPerCab[cabinet] */
/*	double *centroid = malloc(info.cabinet*info.cabinet*sizeof(double));*/	/* centroid[cabinet][subject] - centroid of the cabinet */
	double **centroid = (double **) calloc(info.cabinet, sizeof(double *));
	double aux, distance;

	for(cab = 0; cab < info.cabinet; cab++){
		centroid[cab] = malloc (info.subject*sizeof(double));
	}

	while(flag){
		flag = 0;
		memset(docPerCab, 0, info.cabinet * sizeof(int));
		for(cab = 0; cab < info.cabinet; cab++){
			memset(centroid[cab], 0, info.subject*sizeof(double));
		}

		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			docPerCab[info.cabinets[doc]]++;
			#pragma omp parallel for
			for(sub = 0; sub < info.subject; sub++){
				centroid[info.cabinets[doc]][sub] += info.score[doc][sub];
			}
		}

		#pragma omp parallel for collapse(2) private(cab)
		for(sub = 0; sub < info.subject; sub++)
			for(cab = 0; cab < info.cabinet; cab++){
				centroid[cab][sub] /= docPerCab[cab]; 		/* actually compute the average */
		}

		/* calculate distance between cab and doc; define the new cabinet */
		#pragma omp parallel for private(aux, tmp, cab, sub, distance)
		for(doc = 0; doc < info.document; doc++){
			aux = HUGE_VALF;
			for(cab = 0; cab < info.cabinet; cab++){
				distance = 0;
				for(sub = 0; sub < info.subject; sub++){
					distance += QUAD(info.score[doc][sub] - centroid[cab][sub]);
				}
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
	for(cab = 0; cab < info.cabinet; cab++){
		free(centroid[cab]);
	}
	return 0;
}

