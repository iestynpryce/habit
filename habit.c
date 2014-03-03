/* Command line utility to practice habit forming using randomised rewards *
 * Author: Iestyn Pryce							   *
 * 09/04/2012
 * 02/03/2014 - minor improvements
 */

#include "habit.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

static habit *habits;
static size_t nrec = 10;
static int nhabit = 1;

int main(int argc, char *argv[]) {

	FILE *file;
	
	/* Seed the random number generator */
	srand(time(0));

	/* Test that we have the right number of command line arguments */
	if (argc != 2) {
		printf("Usage: %s file.habit\n",argv[0]);
		return 1;
	}

	file = fopen(argv[1],"r+");

	if (file == NULL) {
		fprintf(stderr,"Unable to open file: %s\n",argv[1]);
		return 1;
	}

	size_t nbytes = 80;
	int i;

	habits = (habit*) malloc(nrec*sizeof(habit));

	if (habits == NULL) {
		fprintf(stderr,"Out of memory\n");
		fclose(file);
		return OUT_OF_MEMORY;
	}

	/* Read in the habits */
	nhabit = read_habits(file,nbytes,nrec);

	fclose(file);
	
	char buf[nbytes];
	int ibuf;
	char *str;
	str = (char*) malloc ((nbytes+1));

	if (str == NULL) {
		fprintf(stderr,"Out of memory\n");
		free(habits);
		return OUT_OF_MEMORY;
	}

	/* Enter a primitive shell to do actions on the habits */
	while(1) {
		printf("-1) quit\n");
		for (i=0;i<nhabit;i++) {
			printf("%d) %s\n",i,habits[i].habit);
		}
		printf("Select a habit (0-%d): ",nhabit-1);
		getline(&str,&nbytes,stdin);
		sscanf(str,"%s",buf);
		ibuf = atoi(buf);

		if (ibuf >= 0 && ibuf < nhabit) {
			perform_habit(&habits[ibuf]);
		} else if (ibuf == -1) {
			break;
		}
	}

	/* Now that things may have changed, write the file out again *
 	 * with the new information */
	file = fopen(argv[1],"w");

	for (i=0;i<nhabit;i++) {
		fprintf(file,"%s,%s,%f,%d\n",habits[i].habit,
				     	     habits[i].reward,
				             habits[i].points,
				     	     habits[i].gates);
	}

	/* Clean up */
	fclose(file);
	free(habits);
	free(str);

	return 0;
}

/* Read in the habit file */
int read_habits(FILE *file, size_t nbytes, int nrec) {
	assert(file != NULL);
	int n=0; 
	char line[nbytes];
	char *tok;
	const char sep[] = ",";
	
	while(fgets(line,nbytes,file) != NULL) {
		if (n == nrec) {
			nrec += 10;
			habits = xrealloc(habits,nrec*sizeof(habit));
			if (habits == NULL) {
				return OUT_OF_MEMORY;
			}
		}
		/* Parse each line */
		tok = strtok(line,sep);
		if (tok != NULL) {
			strcpy(habits[n].habit,tok);
		} else {
			read_error(n);
		}

		tok = strtok(NULL,sep);
		if (tok != NULL) {
			strcpy(habits[n].reward,tok);
		} else {
			read_error(n);
		}
		
		tok = strtok(NULL,sep);
		if (tok != NULL) {
			habits[n].points = atof(tok);
		} else {
			read_error(n);
		}
		
		tok = strtok(NULL,sep);
		if (tok != NULL) {
			habits[n].gates  = atoi(tok);
		} else {
			read_error(n);
		}

		n++;		
	}
	return n;
}

void read_error(int n) {
	fprintf(stderr,"Warning: habit %d malformed. Skipping...\n",n);
}
	

/* Increase the number of points by a random number in the range [1,10] */
void add_rand_point(habit *h) {
	assert(h != NULL);
	h->points += (rand() % 10) +1;
	printf("You now have %f points\n",h->points);
}

void check_gates(habit *h) {
	assert(h != NULL);
	int i = (int) h->points / 15;
	if (i > h->gates) {
		habit *r;
		h->gates++;
		printf("You've passed a gate, well done\n");
		printf("Treat yourself to: %s\n",h->reward);
		/* We need a new reward and a new habit*/
		r = new_reward(h);
		if (r == NULL) {
			fprintf(stderr,"Failed to set new reward\n");
		}
		r = new_habit();
		if (r == NULL) {
			fprintf(stderr,"Failed to add new habit\n");	
		}
	}
}

habit *new_reward(habit *h) {
	assert(h != NULL);
	size_t nbytes = 80;
	char* str;
	str = (char*) malloc(nbytes*sizeof(char));

	if (str != NULL) {
		printf("Enter a new reward: ");
		getline(&str,&nbytes,stdin);
		sscanf(str,"%[^\t\n]",h->reward);
		free(str);
		return h;
	} else {
		fprintf(stderr,"Out of memory\n");
		exit(OUT_OF_MEMORY);
	}
}

/* Add a new habit */
habit *new_habit() {
	/* Add code for adding a new habit */
	if (habits == NULL) {
		habits = (habit*) malloc(nrec*sizeof(habit));
		if (habits == NULL) {
			/*ERROR*/
			fprintf(stderr,"Out of memory\n");
			exit(OUT_OF_MEMORY);
		}
	}

	/* Resize array of habits if necessary */
	if (nhabit == nrec) {
		nrec += 10;
		habits = xrealloc(habits,nrec*sizeof(habit));
		if (habits == NULL) {
			return NULL;
		}
	} 

	size_t nbytes = 80;
	char* str;
	str = (char*) malloc(nbytes*sizeof(char));

	if (str != NULL) {
		printf("Enter a new habit: ");
		getline(&str,&nbytes,stdin);
		sscanf(str,"%[^\t\n]",habits[nhabit].habit);
		printf("Enter a reward: ");
		getline(&str,&nbytes,stdin);
		sscanf(str,"%[^\t\n]",habits[nhabit].reward);
		free(str);
	} else {
		fprintf(stderr,"Out of memory\n");
		return NULL;
	}
	 
	nhabit++;	

	return &habits[nhabit-1];
}

/* Wrapper with error handling around realloc */
void *xrealloc (void *ptr, size_t size) {
	assert(ptr != NULL);
	assert(size > 0);
	void *value = realloc (ptr, size);
	if (value == 0) {
		/* ERROR*/
		fprintf(stderr,"Out of memory\n");
		exit(OUT_OF_MEMORY);
	}
	return value;
}

/* Once a habit has been selected, ask if it has been completed */
void perform_habit(habit *h) {
	assert(h != NULL);
	size_t nbytes = 5;
	char buf[nbytes];
	char *str;
	printf("Habit: %s (points: %f)\n",h->habit,h->points);
	printf("Did you perform habit? [y/n]: ");
	str = (char*) malloc(nbytes*sizeof(char));
	if (str != NULL) {
		getline(&str,&nbytes,stdin);
		sscanf(str,"%s",buf);
		if (strcmp(buf,"y") == 0) { /* if matched */
			add_rand_point(h);
			check_gates(h);
		} else if (strcmp(buf,"n") != 0) {
			fprintf(stderr,"%s not a valid input\n",buf);
		}
		free(str);
	} else {
		fprintf(stderr,"Out of memory\n");
		exit(OUT_OF_MEMORY);
	}
}
