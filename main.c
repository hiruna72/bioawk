/****************************************************************
Copyright (C) Lucent Technologies 1997
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name Lucent Technologies or any of
its entities not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

const char	*version = "version 20110810";

#define DEBUG
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "awk.h"
#include "ytab.h"

extern	char	**environ;
extern	int	nfields;

int	dbg	= 0;
Awkfloat	srand_seed = 1;
char	*cmdname;	/* gets argv[0] for error messages */
extern	FILE	*yyin;	/* lex input file */
char	*lexprog;	/* points to program argument if it exists */
extern	int errorflag;	/* non-zero if any syntax errors; set by yyerror */
int	compile_time = 2;	/* for error printing: */
/* 2 = cmdline, 1 = compile, 0 = running */

#define	MAX_PFILE	20	/* max number of -f's */

char	*pfile[MAX_PFILE];	/* program filenames from -f's */
int	npfile = 0;	/* number of filenames */
int	curpfile = 0;	/* current filename */

int	safe	= 0;	/* 1 => "safe" mode */

int main(int argc, char *argv[])
{
    fprintf(stderr, "\n[%s] CMD:", __func__);
    for (int i = 0; i < argc; ++i) {
        fprintf(stderr, " %s", argv[i]);
    }
    fprintf(stderr,"\n");

    const char *fs = NULL;
    char tmp[16];
    FILE *fp = NULL; // stdout redirect file


    setlocale(LC_CTYPE, "");
    setlocale(LC_NUMERIC, "C"); /* for parsing cmdline & prog */
    cmdname = argv[0];
    if (argc == 1) {
        fprintf(stderr,
                "usage: %s [-o output file] [-F fs] [-v var=value] [-c fmt] [-tH] [-f progfile | 'prog'] [file ...]\n",
                cmdname);
        exit(1);
    }
    signal(SIGFPE, fpecatch);

    srand_seed = 1;
    srand(srand_seed);

    yyin = NULL;
    symtab = makesymtab(NSYMTAB/NSYMTAB);
    while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
        if (strcmp(argv[1],"-version") == 0 || strcmp(argv[1],"--version") == 0) {
            printf("awk %s\n", version);
            exit(0);
            break;
        }
        if (strncmp(argv[1], "--", 2) == 0) {	/* explicit end of args */
            argc--;
            argv++;
            break;
        }
        switch (argv[1][1]) {
            case 's':
                if (strcmp(argv[1], "-safe") == 0)
                    safe = 1;
                break;
            case 'f':	/* next argument is program filename */
                if (argv[1][2] != 0) {  /* arg is -fsomething */
                    if (npfile >= MAX_PFILE - 1)
                        FATAL("too many -f options");
                    pfile[npfile++] = &argv[1][2];
                } else {		/* arg is -f something */
                    argc--; argv++;
                    if (argc <= 1)
                        FATAL("no program filename");
                    if (npfile >= MAX_PFILE - 1)
                        FATAL("too many -f options");
                    pfile[npfile++] = argv[1];
                }
                break;
            case 'F':	/* set field separator */
                if (argv[1][2] != 0) {	/* arg is -Fsomething */
                    if (argv[1][2] == 't' && argv[1][3] == 0)	/* wart: t=>\t */
                        fs = "\t";
                    else if (argv[1][2] != 0)
                        fs = &argv[1][2];
                } else {		/* arg is -F something */
                    argc--; argv++;
                    if (argc > 1 && argv[1][0] == 't' && argv[1][1] == 0)	/* wart: t=>\t */
                        fs = "\t";
                    else if (argc > 1 && argv[1][0] != 0)
                        fs = &argv[1][0];
                }
                if (fs == NULL || *fs == '\0')
                    WARNING("field separator FS is empty");
                break;
            case 't':
                strcpy(tmp, "FS=\\t");  setclvar(tmp);
                strcpy(tmp, "OFS=\\t"); setclvar(tmp);
                break;
            case 'v':	/* -v a=1 to be done NOW.  one -v for each */
                if (argv[1][2] != 0) {  /* arg is -vsomething */
                    if (isclvar(&argv[1][2]))
                        setclvar(&argv[1][2]);
                    else
                        FATAL("invalid -v option argument: %s", &argv[1][2]);
                } else {		/* arg is -v something */
                    argc--; argv++;
                    if (argc <= 1)
                        FATAL("no variable name");
                    if (isclvar(argv[1]))
                        setclvar(argv[1]);
                    else
                        FATAL("invalid -v option argument: %s", argv[1]);
                }
                break;
            case 'd':
                dbg = atoi(&argv[1][2]);
                if (dbg == 0)
                    dbg = 1;
                printf("awk %s\n", version);
                break;
            case 'H':
                bio_flag |= BIO_SHOW_HDR;
                break;
            case 'o':
                printf("o option %d %s \n",argc,argv[2]);
                // redirect stdout to file
                if((fp=freopen(argv[2], "w" ,stdout))==NULL) {
                    printf("Cannot open file %s.\n",argv[2]);
                    exit(1);
                }
                --argc;
                ++argv;
                break;
            case 'c':
                if (argv[1][2] != 0) {	/* arg is -csomething */
                    if ((bio_fmt = bio_get_fmt(&argv[1][2])) == BIO_NULL) return 1;
                } else {		/* arg is -c something */
                    argc--; argv++;
                    if (argc <= 1)
                        FATAL("no variable name");
                    if ((bio_fmt = bio_get_fmt(argv[1])) == BIO_NULL) return 1;
                }
                break;
            default:
                WARNING("unknown option %s ignored", argv[1]);
                break;
        }
        if ((argv[1][1] == 't' || argv[1][1] == 'H') && argv[1][2] != 0) { /* dealing with for example "-tc help" */
            char *p;
            for (p = &argv[1][2]; *p; ++p) *(p-1) = *p;
            *(p-1) = *p;
        } else --argc, ++argv;
    }
    /* argv[1] is now the first argument */
    if (npfile == 0) {	/* no -f; first argument is program */
        if (argc <= 1) {
            if (dbg)
                exit(0);
            FATAL("no program given");
        }
        dprintf( ("program = |%s|\n", argv[1]) );
        lexprog = argv[1];
        argc--;
        argv++;
    }
    recinit(recsize);
    syminit();
    compile_time = 1;
    argv[0] = cmdname;	/* put prog name at front of arglist */
    dprintf( ("argc=%d, argv[0]=%s\n", argc, argv[0]) );
    arginit(argc, argv);
    if (!safe)
        envinit(environ);
    yyparse();
    setlocale(LC_NUMERIC, ""); /* back to whatever it is locally */
    if (fs)
        *FS = qstring(fs, '\0');
    dprintf( ("errorflag=%d\n", errorflag) );
    if (errorflag == 0) {
        compile_time = 0;
        run(winner);
    } else
        bracecheck();

    // close redircting file before exiting..
    if (fp != NULL) fclose(fp);


    return(errorflag);
}

int pgetc(void)		/* get 1 character from awk program */
{
    int c;

    for (;;) {
        if (yyin == NULL) {
            if (curpfile >= npfile)
                return EOF;
            if (strcmp(pfile[curpfile], "-") == 0)
                yyin = stdin;
            else if ((yyin = fopen(pfile[curpfile], "r")) == NULL)
                FATAL("can't open file %s", pfile[curpfile]);
            lineno = 1;
        }
        if ((c = getc(yyin)) != EOF)
            return c;
        if (yyin != stdin)
            fclose(yyin);
        yyin = NULL;
        curpfile++;
    }
}

char *cursource(void)	/* current source file name */
{
    if (npfile > 0)
        return pfile[curpfile];
    else
        return NULL;
}
