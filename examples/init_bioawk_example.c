#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "interface_bioawk.h"


int main()
{
    int max_len = 250;
	printf("running example....\n");
	enum { kMaxArgs = 64 };
    int argc = 0;
    char *argv[kMaxArgs];

    char commandLine[] = "bioawk -tc 'vcf' '{print $2,$2,$2+$2}' ./testFiles/concat.vcf";
    char *p2 = strtok(commandLine, " ");
    const char search = '\'';

    while (p2 && argc < kMaxArgs-1)
    {
        fprintf(stderr,"p = %s\n",p2);
        char * pos1 = strchr(p2,search);
        char * argument_ptr = malloc(max_len);
        strcpy(argument_ptr, p2);
        int has_quotes = 0;
        if(pos1 != NULL){
            char * pos2 = strchr(pos1+1,search);
            if(pos2 == NULL){
                char * pos3;
                do {
                    p2 = strtok(0, " ");
                    strcat(argument_ptr, " ");
                    strcat(argument_ptr, p2);
                    pos3 = strchr(p2,search);
                }while (p2 && pos3 == NULL);
                if(*(pos3+1) != '\0'){
                    fprintf(stderr,"%c is not found at the end of an argument",search);
                    return 1;
                }
            }
            else if(*(pos2+1) != '\0'){
                fprintf(stderr,"%c is found more than twice in an argument.\n",search);
                return 1;
            }
            // strip ['] from argument
            has_quotes = 1;
//            argument_ptr++;
            argument_ptr[strlen(argument_ptr)-1] = 0;

        }
        fprintf(stderr,"argument = %s\n",argument_ptr);
        if(has_quotes){
            argv[argc++] = ++argument_ptr;
        } else{
            argv[argc++] = argument_ptr;
        }
        p2 = strtok(0, " ");
    }

    fprintf(stderr,"argc = %d\n",argc);
    for(auto i; i<argc;i++){
        fprintf(stderr,"arg no %d %s\n",i,argv[i]);
    }

    init_bioawk(argc,argv);

    for(auto i; i<argc;i++){
        free(argv[i]);
    }
    return 0;

}

