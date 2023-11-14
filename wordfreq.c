#include "ADTs/hashcskmap.h"
#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include "ADTs/stringADT.h"
#include "ADTs/arraylist.h"

#define USAGE "Illegal Flag! Usage: %s ['-afilp']\n"
#define FILE_ERR "Unable to open file: %s\n"

int keycmp(void *x1, void *x2) {
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return strcmp((char *)m1->key, (char *)m2->key);
}

int valcmp(void *x1, void *x2) {
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return ((long)m1->value < (long)m2->value);
}

int inkeycmp(void *x1, void *x2) {
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	//CHANGE ME
	return strcmp((char *)m2->key, (char *)m1->key);
}

int invalcmp(void *x1, void *x2) {
	MEntry *m1 = (MEntry *)x1;
	MEntry *m2 = (MEntry *)x2;
	return ((long)m2->value < (long)m1->value);
}	

void printWordFreq(FILE *f, bool ifA, bool ifF, bool ifI, bool ifL, bool ifP, const CSKMap *m, bool print) {
	const String *st = String_create("");
	char buf[BUFSIZ];
	
	//printf("Function called...");
	while(fgets(buf, BUFSIZ, f) != NULL) {
		st->clear(st);
		st->append(st, buf);
		if (ifL) {
			st->lower(st);
		}
		if (ifP) {
			st->translate(st, "[:punct:]", ' ');
		}

		const ArrayList *al = st->split(st, "");
		//printf("created array...");
		if (al == NULL) {continue;}

		long i;
		//printf("Starting loop...");
		for (i = 0; (i < al->size(al)); i++) {
			char *sp;
			long val;
			(void)al->get(al, i, (void **)&sp);

			if(m->get(m, sp, (void **)&val)) {
				val++;
				//m->put(m, sp, (void *)val);
			} else {
				val = 1;
				//m->put(m, sp, (void *)val);
			}
			m->put(m, sp, (void *)val);	
		}
		al->destroy(al);
	}
	st->destroy(st);

	long n;
	MEntry **mes;
	mes = m->entryArray(m, &n);
	
	//printf("Running...");
	
	if (ifI) {
		//printf("Entering I...");
		if (ifA) {
			sort((void **)mes, n, inkeycmp);
			if (print) {
				long i;
				for (i = 0; i < n; i++) {
					//print = false;
					printf("%8ld %s\n", (long)mes[i]->value, (char *)mes[i]->key);
				}
				print = false;
			}
			//free((void *)mes);

		}
		if (ifF) {
			sort((void **)mes, n, invalcmp);
			if (print) {
				long i;
				for (i = 0; i < n; i++) {
					//print = false;
					printf("%8ld %s\n", (long)mes[i]->value, (char *)mes[i]->key);
				}
				print = false;
			}
			//free((void *)mes);
		}
	} else {
		if (ifA) {
			sort((void **)mes, n, keycmp);
			if (print) {
				long i;
				for (i = 0; i < n; i++) {
					//print = false;
					//printf("Printing for ifA...");
					printf("%8ld %s\n", (long)mes[i]->value, (char *)mes[i]->key);
				}
				print = false;
			}
			//free((void *)mes);
		
		}
		if (ifF) {
			sort((void **)mes, n, valcmp);
			if (print) {
				long i;
				for (i = 0; i < n; i++) {
					//print = false;
					printf("%8ld %s\n", (long)mes[i]->value, (char *)mes[i]->key);
				}
				print = false;
			}
			//free((void *)mes);
		}
	}
	if (print) {
		long i;
		for (i = 0; i < n; i++) {
			//printf("Printing for ifPrint");
			printf("%8ld %s\n", (long)mes[i]->value, (char *)mes[i]->key);
		}
	//free((void *)mes);
	}
	free((void *)mes);
}

int main(int argc, char *argv[]) {
	int exitStatus = EXIT_FAILURE;

	const CSKMap *m = HashCSKMap(0L, 0.0, doNothing);
	if (m == NULL) {
		fprintf(stderr, "Unable to create hash map\n");
		goto cleanup;
	}
	
	bool print;
	bool ifA, ifF, ifI, ifL, ifP;
	ifA = ifF = ifI = ifL = ifP = false;

	int opt;
	opterr = 0;
	while((opt = getopt(argc, argv, "afilp")) != -1) {
		switch(opt) {
			case 'a': ifA = true; break;
			case 'f': ifF = true; break;
			case 'i': ifI = true; break;
			case 'l': ifL = true; break;
			case 'p': ifP = true; break;
			default: 
				  fprintf(stderr, USAGE, argv[0]);
				  goto cleanup;
		}
	}
	
	if (ifI) { 
		if (!ifA && !ifF) {
		fprintf(stderr, USAGE, argv[optind]);
		goto cleanup;
		}
	}

	if (ifA && ifF) {
		fprintf(stderr, USAGE, argv[optind]);
		goto cleanup;	
	}	

	if (optind == argc) {
		//No file specified
		print = true;
		printWordFreq(stdin, ifA, ifF, ifI, ifL, ifP, m, print);
		exitStatus = EXIT_SUCCESS;
		goto cleanup;
	}

	if ((optind + 1) == argc) {
		//One file specified
		print = true;
		FILE *f = fopen(argv[optind], "r");
		if (f == NULL) {
			fprintf(stderr, FILE_ERR, argv[optind]);
			goto cleanup;
		}
		printWordFreq(f, ifA, ifF, ifI, ifL, ifP, m, print);
		fclose(f);
		exitStatus = EXIT_SUCCESS;
		goto cleanup;
	}
	
	else {
		//More than one file specified
		int i;
		FILE *fds[50];
		int nFiles = 0;
		
		for (i = optind; i < argc; i++) {
			//printf("Opening files...");

			FILE *f = fopen(argv[i], "r");
			if (f == NULL) {
				fprintf(stderr, FILE_ERR, argv[i]);
				goto cleanup;
			}
			fds[nFiles++] = f;
			//printf("Got files!");
		}

		for (i = 0; i < nFiles; i++) {
			if (i == (nFiles - 1)) {
				print = true;
			}
			//printf("Printing file...");
			printWordFreq(fds[i], ifA, ifF, ifI, ifL, ifP, m, print);
			fclose(fds[i]);
		}
		exitStatus = EXIT_SUCCESS;
		goto cleanup;	
	}
cleanup:
	if (m != NULL) {
		m->destroy(m);
	}
	return exitStatus;
}
