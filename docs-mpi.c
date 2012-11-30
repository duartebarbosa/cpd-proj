#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define LINE_SIZE 2048
#define QUAD(x) (x)*(x)

#define DOCUMENTS info.doc_sub_cab[0]
#define SUBJECTS info.doc_sub_cab[1]
#define CABINETS info.doc_sub_cab[2]

#define MASTER 0
#define INIT_TAG 1

#define FILL_TAG 20
#define CABSCORE_TAG 3
#define FLAG_TAG 50
#define CABINETS_TAG 60

#define CHUNK(x) ((x)/info.numtasks)
#define LIMIT_INF_CHUNK(x) ((info.taskid)*((x)/info.numtasks))
#define LIMIT_SUP_CHUNK(x) (((info.taskid+1)==info.numtasks)?(x):((info.taskid+1)*((x)/info.numtasks)))

#define FLIMIT_INF_CHUNK(x,y) ((y)*((x)/info.numtasks))
#define FLIMIT_SUP_CHUNK(x,y) (((y+1)==info.numtasks)?(x):((y+1)*((x)/info.numtasks)))

struct {
	int numtasks;
	int taskid;
	int doc_sub_cab[3];
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
	register int sub, doc, cab, tmp, task, count;
	register double distance, aux;
	int flag = 1;
int i = 0;
	MPI_Request request_cab[CABINETS], request_task[info.numtasks];
	MPI_Status status[CABINETS];

	/*printf("MPI task %d of %d\n", info.taskid, info.numtasks);*/

	while(flag){
		int lim_inf = LIMIT_INF_CHUNK(CABINETS), lim_sup = LIMIT_SUP_CHUNK(CABINETS);
		flag = 0;	
		/*MPI_Barrier(MPI_COMM_WORLD);*/

		if(info.taskid == MASTER){
			for(cab = 0; cab < (CABINETS); cab++)
				memset(info.cabScore[cab], 0, SUBJECTS * sizeof(double));
		}
		else
			for(cab = lim_inf; cab < lim_sup; cab++)
				memset(info.cabScore[cab], 0, SUBJECTS * sizeof(double));	
/*
		for(cab = lim_inf; cab < lim_sup; cab++){
			for(sub = 0; sub < SUBJECTS; sub++)
				printf("task: %d - cab %d, sub %d, result: %f, address: %x\n", info.taskid, cab, sub, info.cabScore[cab][sub], &info.cabScore[cab][sub]);
		}
*/
		/* calculate the average of scores for each cabinet */
		for(cab = lim_inf; cab < lim_sup; cab++){
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
		MPI_Barrier(MPI_COMM_WORLD);
		if(info.taskid == MASTER){
			/* falta calcular o resto */
			for(task = 1; task < info.numtasks; task++){
				lim_inf = FLIMIT_INF_CHUNK(CABINETS,task);
				lim_sup = FLIMIT_SUP_CHUNK(CABINETS,task);
				for(cab = lim_inf; cab < lim_sup; cab++){
					/*printf("task: %d, inf: %d, cab: %d, sup: %d\n", info.taskid, lim_inf, cab, lim_sup);*/
					MPI_Recv(info.cabScore[cab], SUBJECTS, MPI_DOUBLE, task, CABSCORE_TAG, MPI_COMM_WORLD, &status[cab-CHUNK(CABINETS)]);
				}
			}
		}
		else {
			MPI_Request request_cab[CHUNK(CABINETS)];
			for(cab = lim_inf; cab < lim_sup; cab++)
				MPI_Isend(info.cabScore[cab], SUBJECTS, MPI_DOUBLE, MASTER, CABSCORE_TAG, MPI_COMM_WORLD, &request_cab[cab-lim_inf]);
			MPI_Waitall(CHUNK(CABINETS), request_cab, status);
		}

		if(info.taskid == MASTER){
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
					if(!flag){
						flag = 1;
						for(task = 1; task < info.numtasks; task++)
							MPI_Isend(&flag, 1, MPI_INT, task, FLAG_TAG, MPI_COMM_WORLD, &request_task[task]);
					}
				}
			}
			if(!flag){
				for(task = 1; task < info.numtasks; task++)
					MPI_Isend(&flag, 1, MPI_INT, task, FLAG_TAG, MPI_COMM_WORLD, &request_task[task]);
			}
			MPI_Waitall(info.numtasks-1, request_task, MPI_STATUS_IGNORE);
			if(flag){
				for(task = 1; task < info.numtasks; task++)
					MPI_Isend(info.cabinets, DOCUMENTS, MPI_INT, task, CABINETS_TAG, MPI_COMM_WORLD, &request_task[task]);
				MPI_Waitall(info.numtasks-1, request_task, MPI_STATUS_IGNORE);
			}
		}
		else {
			MPI_Recv(&flag, 1, MPI_INT, MASTER, FLAG_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(flag)
				MPI_Recv(info.cabinets, DOCUMENTS, MPI_INT, MASTER, CABINETS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
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

	MPI_Status status;
	MPI_Request reqs[DOCUMENTS];

	int doc, sub, cab, task;
	char *tmp = NULL, line[LINE_SIZE] = {0};

	#ifdef GETTIME
	double start = MPI_Wtime();
	#endif

	if(argc != 2 && argc != 3)
		return -1;

	/* MPI Initialization */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf ("Error starting MPI program. Terminating.\n");
		/*MPI_Abort(MPI_COMM_WORLD, ret);*/
		return -1;
	}
	
	MPI_Comm_size(MPI_COMM_WORLD, &info.numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &info.taskid);

	if(info.taskid == MASTER){
		FILE *input;
		MPI_Request info_reqs[info.numtasks-1];

		if((input = fopen(argv[1], "r")) == NULL)
			return -2;

		if(fscanf(input, "%d\n %d\n %d\n", &CABINETS, &DOCUMENTS, &SUBJECTS) != 3)
			return -3;

		if(argc == 3)
			CABINETS = strtol(argv[2], (char **) NULL, 10);

		for(task = 1; task < info.numtasks; task++)
			MPI_Isend(info.doc_sub_cab, 3, MPI_INT, task, INIT_TAG, MPI_COMM_WORLD, &info_reqs[task-1]);

		MPI_Waitall(info.numtasks - 1, info_reqs, MPI_STATUS_IGNORE);

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

		/*printf("task: %d doc: %d, sub: %d, cab: %d\n", info.taskid, DOCUMENTS, SUBJECTS, CABINETS);*/

		for(task = 1; task < info.numtasks; task++)
			for(doc = 0; doc < DOCUMENTS; doc++)
				MPI_Send(info.docScore[doc], SUBJECTS, MPI_DOUBLE, task, FILL_TAG, MPI_COMM_WORLD);
	}
	else{
		MPI_Recv(info.doc_sub_cab, 3, MPI_INT, MASTER, INIT_TAG, MPI_COMM_WORLD, &status);

	/*	printf("task: %d doc: %d, sub: %d, cab: %d\n", info.taskid, DOCUMENTS, SUBJECTS, CABINETS);*/
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
	/*printf("MPI Communication time: %lf, for task %d of %d\n", MPI_Wtime() - start, info.taskid, info.numtasks);*/

	process();

	if(info.taskid == MASTER)
		flushClean(argv[1]);
	else
		cleanup();

	MPI_Finalize();

	#ifdef GETTIME
	printf("MPI time: %lf\n", MPI_Wtime() - start);
	#endif

	return 0;
}
