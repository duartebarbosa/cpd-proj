#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_SIZE 128

char line[LINE_SIZE];

int main(int argc, char** argv){
	FILE * in =  NULL, * out = NULL;
	int cabinet = 0, document = 0, subject = 0;
	
	if(argc != 2 && argc != 3)
		return -1;
	
	if((in = fopen(argv[1], "r")) == NULL)
		return -2;
	
	if((out = fopen(strcat(argv[1], ".out"), "w")) == NULL)
		return -3;

	if(fscanf(in, "%d\n %d\n %d\n", &cabinet, &document, &subject) != 3)
		return -4;

	if(argc == 3)
		cabinet = atoi(argv[2]);

	printf("%s %d %d\n", argv[2], document, subject); /* bug! argv[2] doesn't evaluate to the correct value. not sure why! */

	for(; fgets(line, LINE_SIZE, in) != NULL;) {
		fprintf(out, "%s", line);
	}

	if(fclose(in) == EOF)
		return -5;

	if(fclose(out) == EOF)
		return -6;

	return 0;
}
