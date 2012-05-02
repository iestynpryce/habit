#include <stdio.h>
#include <stdlib.h>
typedef struct {
	char   habit[80];
	char   reward[80];
	float  points;
	int    gates;
} habit;

enum {  NONE,
	OUT_OF_MEMORY,
};

int read_habits(FILE *file, size_t nbytes, int nrec);
void read_error(int n);
void add_rand_point(habit *h);
void check_gates(habit *h);
habit *new_reward(habit *h);
habit *new_habit();
void perform_habit(habit *h);

void *xrealloc(void *ptr, size_t size);
