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
	double *centroid = malloc(info.cabinet*info.subject*sizeof(double));	/* centroid[cabinet][subject] - centroid of the cabinet */
	double aux, distance;

	while(flag){
		flag = 0;
		memset(docPerCab, 0, info.cabinet * sizeof(int));
		memset(centroid, 0, info.cabinet * info.subject * sizeof(double));

		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			docPerCab[info.set[doc].cabinet]++;
			#pragma omp parallel for
			for(sub = 0; sub < info.subject; sub++){
				CENTROID(info.set[doc].cabinet, sub) += info.set[doc].score[sub];
			}
		}

		#pragma omp parallel for collapse(2) private(sub)
		for(cab = 0; cab < info.cabinet; cab++){
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(cab, sub) /= docPerCab[cab]; 		/* actually compute the average */
		}

		/* calculate distance between cab and doc; define the new cabinet */
		#pragma omp parallel for private(aux, tmp, cab, sub) reduction(+:distance)
		for(doc = 0; doc < info.document; doc++){
			aux = HUGE_VALF;
			for(cab = 0; cab < info.cabinet; cab++){
				distance = 0;
				for(sub = 0; sub < info.subject; sub++){
					distance += QUAD(info.set[doc].score[sub] - CENTROID(cab, sub));
				}
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

