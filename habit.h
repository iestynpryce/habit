typedef struct {
	char   habit[80];
	char   reward[80];
	float  points;
	int    gates;
} habit;

int parse_cli_input(char *str);

void add_rand_point(habit *h);
void check_gates(habit *h);
void new_reward(habit *h);
void perform_habit(habit *h);
