#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#define LINE_SIZE 128

char line[LINE_SIZE];

int main(int argc, char** argv){
	FILE * in =  NULL, * out = NULL;
	int cabinet = 0, document = 0, subject = 0;
	char *outfile = alloca(strlen(argv[1]));
	
	printf("%s\n", outfile);
	if(argc != 2 && argc != 3)
		return -1;
	
	if((in = fopen(argv[1], "r")) == NULL)
		return -2;
	
	strcpy(outfile, argv[1]);
	outfile = strcat(strtok(outfile, "."), ".out");
	
	if((out = fopen(outfile, "w")) == NULL)
		return -3;

	if(fscanf(in, "%d\n %d\n %d\n", &cabinet, &document, &subject) != 3)
		return -4;

	if(argc == 3)
		cabinet = atoi(argv[2]);
	
	for(; fgets(line, LINE_SIZE, in) != NULL;) {
		fprintf(out, "%s", line);
	}

	if(fclose(in) == EOF)
		return -5;

	if(fclose(out) == EOF)
		return -6;

	return 0;
}
