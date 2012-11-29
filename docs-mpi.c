#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define LINE_SIZE 1024
#define QUAD(x) (x)*(x)

#define DOCUMENTS info.doc_sub_cab[0]
#define SUBJECTS info.doc_sub_cab[1]
#define CABINETS info.doc_sub_cab[2]

#define MASTER 0
#define INIT_TAG 1

#define FILL_TAG 20
#define CABSCORE_TAG 3

#define FOREIGN_CHUNK(x,y) ((y)*((x)/numtasks))
#define FOREIGN_LIMIT_CHUNK(x,y) (((y)+1)*((x)/numtasks))

#define CHUNK(x) (taskid*((x)/numtasks))
#define LIMIT_CHUNK(x) ((taskid+1)*((x)/numtasks))

struct {
	int doc_sub_cab[3];
	int *cabinets;
	double **cabScore;
	double **docScore;
} info;

int numtasks, taskid, len;

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

int memInit(){
	register int doc = 0, cab = 0;

	info.cabinets = malloc(DOCUMENTS * sizeof(int));
	info.docScore = malloc(DOCUMENTS * sizeof(double*));
	info.cabScore = malloc(CABINETS * sizeof(double*));

	for(; cab < CABINETS; cab++)
		info.cabScore[cab] = malloc(SUBJECTS * sizeof(double));

	for(; doc < DOCUMENTS; doc++){
		info.cabinets[doc] = doc % CABINETS;
		info.docScore[doc] = malloc(SUBJECTS * sizeof(double));
	}

	return 0;
}

int process(){
	register int sub, doc, cab, tmp, flag = 1, task, count;
	register double distance, aux;
int i = 0;
	MPI_Request request[CABINETS];
	MPI_Status status[CABINETS];

	/*printf("MPI task %d of %d\n", taskid, numtasks);*/

	while(flag){
		flag = 0;
		printf("coco, task: %d\n", taskid);
		for(cab = 0; cab < (CABINETS); cab++)
			memset(info.cabScore[cab], 0, SUBJECTS * sizeof(double));

		/* calculate the average of scores for each cabinet */
		for(cab = 0; cab < (CABINETS); cab++){
			/*printf("%d task: %d, cab: %d\n", i, taskid, cab);*/
			count = 0;
			for(doc = 0; doc < DOCUMENTS; doc++){
				if(info.cabinets[doc] == cab){
					for(sub = 0; sub < SUBJECTS; sub++)
						info.cabScore[cab][sub] += info.docScore[doc][sub];
					count++;
				}
			}
			for(sub = 0; sub < SUBJECTS; sub++)
				info.cabScore[cab][sub] /= count;
		}

printf("---Barreira, i: %d, task: %d of %d\n", i, taskid, numtasks);
MPI_Barrier(MPI_COMM_WORLD);

		if(taskid == MASTER){
			/* falta calcular o resto */
			printf("cheguei\n");
			for(task = 1; task < numtasks; task++){
				for(cab = LIMIT_CHUNK(CABINETS); cab < CABINETS; cab++){
					printf("\ttask %d: RECEIVE cab %d\n", task, cab);
					MPI_Recv(info.cabScore[cab], SUBJECTS, MPI_DOUBLE, task, CABSCORE_TAG, MPI_COMM_WORLD, &status[cab]);
					
				}
			}
		}
		else {
			for(cab = CHUNK(CABINETS); cab < (CABINETS); cab++){
				printf("\ttask: %d: SEND cab: %d\n", taskid, cab);
				MPI_Isend(info.cabScore[cab], SUBJECTS, MPI_DOUBLE, MASTER, CABSCORE_TAG, MPI_COMM_WORLD, &request[cab]);
				MPI_Wait(&request[cab], &status[cab]);
			}

		}
		
		
		MPI_Barrier(MPI_COMM_WORLD);
		if(taskid == MASTER){
			/* calculate distance between cab and doc; set the new cabinet */
			for(doc = 0; doc < DOCUMENTS; doc++){
				aux = HUGE_VALF;
				for(cab = 0; cab < CABINETS; cab++){
					for(sub = distance = 0; sub < SUBJECTS; sub++)
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
		MPI_Barrier(MPI_COMM_WORLD);
		i++;
	}
		
	return 0;
}

int cleanup(){
	register int doc = 0, cab = 0;
	for(doc = 0; doc < DOCUMENTS; doc++)
		free(info.docScore[doc]);
	free(info.docScore);

	for(; cab < CABINETS; cab++)
		free(info.cabScore[cab]);
	free(info.cabScore);

	free(info.cabinets);
}

int flushClean(char *filename){
	register int doc = 0;
	FILE *output;
	char *outfile = alloca(strlen(filename) + 1);
	outfile = strcat(strtok(filename, "."), ".out");

	/* output */
	if((output = fopen(outfile, "w")) == NULL)
		return -5;

	for(; doc < DOCUMENTS; doc++)
		fprintf(output, "%d %d\n", doc, info.cabinets[doc]);

	if(fclose(output) == EOF)
		return -6;

	cleanup();
	return 0;
}

int main(int argc, char** argv){

	char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;
	MPI_Request reqs[DOCUMENTS], info_reqs[3];

	int doc, sub, cab, task;
	char *tmp = NULL, line[LINE_SIZE] = {0};

	int chunksize;
	int dest;
	int repeatflag = 1;
	int cabinet_num;
	int loop = 0;
	int position=0;
	int mode = 0;
	double start = MPI_Wtime();
	
	if(argc != 2 && argc != 3)
		return -1;

	/* MPI Initialization */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf ("Error starting MPI program. Terminating.\n");
		/*MPI_Abort(MPI_COMM_WORLD, ret);*/
		return -1;
	}
	
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);	/* total number of tasks */
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);		/* my TaskID */
	MPI_Get_processor_name(hostname, &len);		/* my computer's name */

	if(taskid == MASTER){
		FILE *input;
		if((input = fopen(argv[1], "r")) == NULL)
			return -2;

		if(fscanf(input, "%d\n %d\n %d\n", &CABINETS, &DOCUMENTS, &SUBJECTS) != 3)
			return -3;

		if(argc == 3)
			CABINETS = strtol(argv[2], (char **) NULL, 10);

		for(task = 0; task < numtasks; task++)
			MPI_Isend(info.doc_sub_cab, 3, MPI_INT, task, INIT_TAG, MPI_COMM_WORLD, &info_reqs[0]);

		memInit();

		for(doc = 0; doc < DOCUMENTS; doc++){
			if(fgets(line, LINE_SIZE, input)){
				strtok_r(line, " ", &tmp);
				for(sub = 0; sub < SUBJECTS; sub++)
					info.docScore[doc][sub] = naive_strtod(strtok_r(NULL, " ", &tmp));
			}
		}

		if(fclose(input) == EOF)
			return -4;

		/*printf("task: %d doc: %d, sub: %d, cab: %d\n", taskid, DOCUMENTS, SUBJECTS, CABINETS);*/

		for(task = 1; task < numtasks; task++)
			for(doc = 0; doc < DOCUMENTS; doc++)
				MPI_Send(info.docScore[doc], SUBJECTS, MPI_DOUBLE, task, FILL_TAG, MPI_COMM_WORLD);
	}
	else{
		MPI_Recv(info.doc_sub_cab, 3, MPI_INT, MASTER, INIT_TAG, MPI_COMM_WORLD, &status);

	/*	printf("task: %d doc: %d, sub: %d, cab: %d\n", taskid, DOCUMENTS, SUBJECTS, CABINETS);*/
		memInit();

		for(doc = 0; doc < DOCUMENTS; doc++)
			MPI_Recv(info.docScore[doc], SUBJECTS, MPI_DOUBLE, MASTER, FILL_TAG, MPI_COMM_WORLD, &status);
/*
		for(doc = 0; doc < DOCUMENTS; doc++){
			for(sub=0; sub < SUBJECTS; sub++)
				printf("doc: %d, sub: %f\n", doc, info.docScore[doc][sub]);
		}
*/
	}
	/*printf("MPI Communication time: %lf, for task %d of %d\n", MPI_Wtime() - start, taskid, numtasks);*/

	process();

	if(taskid == MASTER)
		flushClean(argv[1]);
	else
		cleanup();

	MPI_Finalize();
	printf("MPI time: %lf\n", MPI_Wtime() - start);

	return 0;
}
