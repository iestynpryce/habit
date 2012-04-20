#include <stdlib.h>
typedef struct {
	char   habit[80];
	char   reward[80];
	float  points;
	int    gates;
} habit;

void add_rand_point(habit *h);
void check_gates(habit *h);
void new_reward(habit *h);
void new_habit();
void perform_habit(habit *h);

void *xrealloc(void *ptr, size_t size);
