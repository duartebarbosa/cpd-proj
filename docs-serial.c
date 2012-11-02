#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include "core.c"

inline int minimum(float distance[]){
	register int cab = 0, index = 0;

	for(; cab < info.cabinet; cab++)
		if(distance[index] > distance[cab])
			index = cab;

	return index;
}


int process(){
	register int sub, doc, cab, tmp, flag = 1;
	int *docPerCab = calloc(info.cabinet, sizeof(int)); 			/* docPerCab[cabinet] */
	float *distance = calloc(info.cabinet, sizeof(float)); 			/* distance[cabinet] - distance from specific doc to cabinet */
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
			memset(distance, 0, info.cabinet * sizeof(float));
			for(cab = 0; cab < info.cabinet; cab++){
				for(sub = 0; sub < info.subject; sub++)
					distance[cab] += QUAD(info.set[doc].score[sub] - CENTROID(cab, sub));
			}
			if((tmp = info.set[doc].cabinet) != (info.set[doc].cabinet = minimum(distance)))
				flag = 1;
		}
	}
	
	free(docPerCab);
	free(distance);
	free(centroid);
	
	return 0;
}
