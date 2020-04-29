#include <stdio.h>
#include <string.h>

#include "interface_bioawk.h"


int main()
{
	printf("running example....\n");

	enum { kMaxArgs = 64 };
	int argc = 0;
	char *argv[kMaxArgs];

	 char commandLine[250] = "bioawk";
	char *p2 = strtok(commandLine, " ");
	
	while (p2 && argc < kMaxArgs-1)
	  {
	    argv[argc++] = p2;
	    p2 = strtok(0, " ");
	  }
	argv[argc] = 0;
	
	init_bioawk(argc,argv);

	return 0;
}

