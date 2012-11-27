#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	int cabinets = strtol(argv[1], (char **) NULL, 10);
	int documents = strtol(argv[2], (char **) NULL, 10);
	int subjects = strtol(argv[3], (char **) NULL, 10);
	int i = 0, j;

	srand((unsigned) time(NULL));
	FILE *resultfile = fopen (argv[4], "w");
	fprintf(resultfile,"%d\n", cabinets);
	fprintf(resultfile,"%d\n", documents);
	fprintf(resultfile,"%d\n", subjects);

	for (;i < documents; i++){
		fprintf(resultfile,"%d ",i);
		for (j = 0; j < subjects; j++){
			double temp = ((double)rand() * 25 ) / RAND_MAX ;
			fprintf(resultfile,"%3.1lf ",temp);
		}
		fprintf(resultfile,"\n");
	}
	fclose(resultfile);

	return 0;
}
