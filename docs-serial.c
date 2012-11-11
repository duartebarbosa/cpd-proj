#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include "core.c"

int process(){
	register int sub, doc, cab, tmp, flag = 1;
	int *docPerCab = calloc(info.cabinet, sizeof(int)); 			/* docPerCab[cabinet] */
	float *centroid = malloc(info.cabinet*info.subject*sizeof(float));	/* centroid[cabinet][subject] - centroid of the cabinet */
	float distance, aux;

	while(flag){
		memset(docPerCab, 0, info.cabinet * sizeof(int));
		memset(centroid, 0, info.cabinet * info.subject * sizeof(float));
		
		/* centroid - average for each cabinet and subject */
		for(doc = 0; doc < info.document; doc++){
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(info.set[doc].cabinet, sub) += info.set[doc].score[sub];
			docPerCab[info.set[doc].cabinet]++;
		}
		for(cab = 0; cab < info.cabinet; cab++)
			for(sub = 0; sub < info.subject; sub++)
				CENTROID(cab, sub) /= docPerCab[cab]; 		/* actually compute the average */

		/* calculate distance between cab and doc; set the new cabinet */
		for(flag = 0, doc = 0; doc < info.document; doc++){
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
