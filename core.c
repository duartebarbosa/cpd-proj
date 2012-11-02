#define LINE_SIZE 512
#define CENTROID(x,y) centroid[(x) + (y) * info.cabinet]
#define QUAD(x) (x)*(x)

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

int cleanup(){
	register int doc = 0;

	for(; doc < info.document; doc++)
		free(info.set[doc].score);

	free(info.set);

	if(fclose(info.in) == EOF)
		return -5;
	if(fclose(info.out) == EOF)
		return -6;

	return 0;
}

int init(){
	register int sub, id;
	char * token = NULL, line[LINE_SIZE] = {0};
	info.set = (document *) calloc(info.document, sizeof(document));

	for(id = 0; id < info.document; id++){
		info.set[id].cabinet = id % info.cabinet;
		info.set[id].score = (float *) calloc(info.subject, sizeof(float));
	}
	
	while(fgets(line, LINE_SIZE, info.in)!= NULL){
		id = atoi(strtok(line, " "));
		for(sub = 0; sub < info.subject && (token = strtok(NULL, " ")) != NULL; sub++)
			info.set[id].score[sub] = atof(token);
	}
	
	return 0;
}

inline int minimum(float distance[]);

int process();

int handleIO(char * filename){
	char *outfile = alloca(strlen(filename));
	outfile = strcat(strtok(strcpy(outfile, filename), "."), ".out");
	
	if((info.in = fopen(filename, "r")) == NULL)
		return -2;
	
	if((info.out = fopen(outfile, "w")) == NULL)
		return -3;

	return 0;
}

int flushOutput(){
	register int doc = 0;
	for(; doc < info.document; doc++){
		fprintf(info.out, "%d %d\n", doc, info.set[doc].cabinet);
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
