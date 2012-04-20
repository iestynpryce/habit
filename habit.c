/* Command line utility to practice habit forming using randomised rewards *
 * Author: Iestyn Pryce							   *
 * 09/04/2012
 */

#include "habit.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]) {

	FILE *file;
	char line[80];

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

	//habit *habits;
	const char sep[] = ",";
	size_t nbytes = 80;
	int n = 0;
	int i;

	habit habits[10]; 

	while(fgets(line,nbytes,file) != NULL) {
		/* Parse each line */
		strcpy(habits[n].habit,strtok(line,sep));
		strcpy(habits[n].reward,strtok(NULL,sep));
		habits[n].points = atof(strtok(NULL,sep));
		habits[n].gates  = atoi(strtok(NULL,sep));
		n++;		
	}
	fclose(file);
	
	char buf[nbytes];
	char *str;
	str = (char*) malloc ((nbytes+1));
	int ibuf;

	while(1) {
		printf("-1) quit\n");
		for (i=0;i<n;i++) {
			printf("%d) %s\n",i,habits[i].habit);
		}
		printf("Select a habit (0-%d): ",n-1);
		getline(&str,&nbytes,stdin);
		sscanf(str,"%s",buf);
		ibuf = atoi(buf);

		if (ibuf >= 0 && ibuf < n) {
			perform_habit(&habits[ibuf]);
		}
		if (ibuf == -1) {
			break;
		}
	}

	file = fopen(argv[1],"w");

	/* now that things may have changed, write the file out again *
 	 * with the new information */
	for (i=0;i<n;i++) {
		fprintf(file,"%s,%s,%f,%d\n",habits[i].habit,
				     	     habits[i].reward,
				             habits[i].points,
				     	     habits[i].gates);
	}

	/* Clean up */
	fclose(file);
	//free(habits);
	free(str);

	return 0;
}

int parse_cli_input(char *str) {
	if (strcmp(str,"yes") == 0) {
		return 1;
	}

	return 0;
}

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
		printf("Enter a new habit: ");
		getline(&str,&nbytes,stdin);
		sscanf(str,"%s",h->reward);
		free(str);
	}
}

void new_habit() {
	
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
	}
}
