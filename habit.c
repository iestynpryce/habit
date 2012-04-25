/* Command line utility to practice habit forming using randomised rewards *
 * Author: Iestyn Pryce							   *
 * 09/04/2012
 */

#include "habit.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

habit *habits;
size_t nrec = 10;
int nhabit = 0;

int main(int argc, char *argv[]) {

	FILE *file;
	char line[80];
	
	/* Seed the random number generator */
	srand(time(0));

	/* Test that we have the right number of command line arguments */
	if (argc != 2) {
		printf("Usage: %s file.habit\n",argv[0]);
		return 1;
	}

	file = fopen(argv[1],"r+");

	if (file == NULL) {
		printf("Unable to open file: %s\n",argv[1]);
		return 1;
	}

	const char sep[] = ",";
	size_t nbytes = 80;
	int i;

	habits = (habit*) malloc(nrec*sizeof(habit));

	if (habits == NULL) {
		fprintf(stderr,"Out of memory\n");
		return OUT_OF_MEMORY;
	}

	/* Read in the habits */
	while(fgets(line,nbytes,file) != NULL) {
		if (nhabit == nrec) {
			nrec += 10;
			habits = xrealloc(habits,nrec*sizeof(habit));
		}
		/* Parse each line */
		strcpy(habits[nhabit].habit,strtok(line,sep));
		strcpy(habits[nhabit].reward,strtok(NULL,sep));
		habits[nhabit].points = atof(strtok(NULL,sep));
		habits[nhabit].gates  = atoi(strtok(NULL,sep));
		nhabit++;		
	}
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

/* Increase the number of points by a random number in the range [1,10] */
void add_rand_point(habit *h) {
	h->points += (rand() % 10) +1;
	printf("You now have %f points\n",h->points);
}

void check_gates(habit *h) {
	int i = (int) h->points / 15;
	if (i > h->gates) {
		h->gates++;
		printf("You've passed a gate, well done\n");
		printf("Treat yourself to: %s\n",h->reward);
		/* We need a new reward and a new habit*/
		new_reward(h);
		new_habit();
	}
}

void new_reward(habit *h) {
	size_t nbytes = 80;
	char* str;
	str = (char*) malloc(nbytes*sizeof(char));

	if (str != NULL) {
		printf("Enter a new reward: ");
		getline(&str,&nbytes,stdin);
		sscanf(str,"%[^\t\n]",h->reward);
		free(str);
	} else {
		fprintf(stderr,"Out of memory\n");
	}
}

/* Add a new habit */
void new_habit() {
	/* Add code for adding a new habit */
	if (habits == NULL) {
		habits = (habit*) malloc(nrec*sizeof(habit));
		if (habits == NULL) {
			/*ERROR*/
			fprintf(stderr,"Out of memory\n");
			return;
		}
	}

	/* Resize array of habits if necessary */
	if (nhabit == nrec) {
		nrec += 10;
		habits = xrealloc(habits,nrec*sizeof(habit));
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
	}
	 
	nhabit++;	
}

/* Wrapper with error handling around realloc */
void *xrealloc (void *ptr, size_t size) {
	void *value = realloc (ptr, size);
	if (value == 0) {
		/* ERROR;*/
		fprintf(stderr,"Out of memory\n");
	}
	return value;
}

/* Once a habit has been selected, ask if it has been completed */
void perform_habit(habit *h) {
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
		}
		free(str);
	} else {
		fprintf(stderr,"Out of memory\n");
	}
}
