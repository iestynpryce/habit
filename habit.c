/* Command line utility to practice habit forming using randomised rewards *
 * Author: Iestyn Pryce							   *
 * 09/04/2012
 * 02/03/2014 - minor improvements
 */
#include "habit.h"

#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
static habit *habits;
static size_t nrec = 10;
static int nhabit = 1;

static char *get_default_storage_path();
static int get_id();
static void usage();

static int add_habit(char *habit, char *reward)
{
	FILE *f;
	char *path;
	int id;

	path = get_default_storage_path();
	f = fopen(path, "a");
	if (f == NULL) {
		fprintf(stderr, "Unable to open file \"%s\": %s\n", path,
			__func__);
		free(path);
		return -1;
	}

	id = get_id();

	if (fprintf(f, "%d\t%s\t%s\t%d\t%d\n", id, habit, reward, 0, 0) < 0) {
		fprintf(stderr, "Error writing to file \"%s\": %s\n", path,
			__func__);
		free(path);
		fclose(f);
		return -1;
	}

	free(path);
	fclose(f);
	return 0;
}

static char *get_default_storage_path()
{
	char *path = NULL;
	size_t len = 0;

	char *env = getenv("HOME");
	if (env == NULL) {
		fprintf(stderr, "Failed: getenv(\"HOME\"): %s\n", __func__);
		return NULL;
	}

	len = strlen(env + 1);	/* Add one for '\0' char */
	len += 7;		/* Add 7 for '/.habit' */
	path = malloc(len * sizeof(char));
	if (path == NULL) {
		fprintf(stderr, "Failed to allocate memory: %s\n", __func__);
		return NULL;
	}
	strncpy(path, env, len);
	strcat(path, "/.habit");

	return path;
}

/* Get the next id which should be used from the .habit file */
int get_id()
{
	char buf[80];
	char *path;
	FILE *f;
	int id = 0;
	bool check_buf = true;

	path = get_default_storage_path();
	f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "Unable to open file \"%s\": %s\n", path,
			__func__);
		free(path);
		return -1;
	}
	free(path);

	while (fgets(buf, 80, f) != NULL) {
		int id_tmp;
		/* Only check the buffer for the current ids if the check_buf flag is true.
		 * This will be true if the last buffer contained a new line.
		 * FIXME: this behaviour is likely to be buggy if the buffer straddles two
		 * lines i.e. "foo\n3    bar". An issue with long lines.
		 */
		if (check_buf) {
			sscanf(buf, "%d\t", &id_tmp);
			if (id_tmp > id) {
				id = id_tmp;
			}
		}
		check_buf = false;
		if (strstr(buf, "\n")) {
			check_buf = true;
		}
	}

	fclose(f);
	return id + 1;
}

/* Check if a file exists at the filepath. Return 1 if it does, 0 otherwise. */
static int file_exists(const char *filepath)
{
	int ret = 0;
	struct stat buffer;

	if (stat(filepath, &buffer) == 0) {
		ret = 1;
	}

	return ret;
}

void list_habits(char *path)
{
	char c;
	FILE *f = fopen(path, "r+");
	if (f == NULL) {
		fprintf(stderr, "Unable to open file \"%s\": %s\n", path,
			__func__);
		free(path);
		exit(-1);
	}

	while ((c = fgetc(f)) != EOF) {
		fputc(c, stdout);
	}
}

int main(int argc, char *argv[])
{
//      FILE *file;
	char *habit_file_path;

	/* Seed the random number generator */
	srand(time(0));

	habit_file_path = get_default_storage_path();

	/* Check if the file exists, if it doesn't create an empty file */
	if (!file_exists(habit_file_path)) {
		int fd = open(habit_file_path, O_CREAT | O_WRONLY | O_EXCL,
			      S_IRUSR | S_IWUSR);
		if (fd < 0) {
			fprintf(stderr, "failed to open file %s: %s\n",
				habit_file_path, __func__);
			free(habit_file_path);
			return -1;
		}
		close(fd);
	}

	/* Remove count of program name - now gives only a count of the args */
	--argc;
	++argv;

	/* Print out habits if no arguments are given */
	if (argc < 1) {
		list_habits(habit_file_path);
	} else {
		while (argc) {
			if (**argv == '-') {
				switch ((*argv)[1]) {
				case 'a':	/* Add a new habit */
					if (argc > 1) {
						add_habit(argv[1], argv[2]);
						++argv;
						++argv;
						--argc;
						--argc;
					} else {
						usage();
					}
					break;
				case 'l':	/* List current habits */
					list_habits(habit_file_path);
					break;
				case 'r':	/* Record habit as done */
					printf
					    ("TODO: Record habit as having been done\n");
					break;

				}
			}
			--argc;
			++argv;
		}
	}

	free(habit_file_path);
	return 0;
}

/* Read in the habit file */
int read_habits(FILE * file, size_t nbytes, int nrec)
{
	assert(file != NULL);
	int n = 0;
	char line[nbytes];
	char *tok;
	const char sep[] = ",";
	while (fgets(line, nbytes, file) != NULL) {
		if (n == nrec) {
			nrec += 10;
			habits = xrealloc(habits, nrec * sizeof(habit));
			if (habits == NULL) {
				return OUT_OF_MEMORY;
			}
		}
		/* Parse each line */
		tok = strtok(line, sep);
		if (tok != NULL) {
			strcpy(habits[n].habit, tok);
		} else {
			read_error(n);
		}

		tok = strtok(NULL, sep);
		if (tok != NULL) {
			strcpy(habits[n].reward, tok);
		} else {
			read_error(n);
		}

		tok = strtok(NULL, sep);
		if (tok != NULL) {
			habits[n].points = atof(tok);
		} else {
			read_error(n);
		}

		tok = strtok(NULL, sep);
		if (tok != NULL) {
			habits[n].gates = atoi(tok);
		} else {
			read_error(n);
		}

		n++;
	}
	return n;
}

void read_error(int n)
{
	fprintf(stderr, "Warning: habit %d malformed. Skipping...\n", n);
}

/* Increase the number of points by a random number in the range [1,10] */
void add_rand_point(habit * h)
{
	assert(h != NULL);
	h->points += (rand() % 10) + 1;
	printf("You now have %f points\n", h->points);
}

void check_gates(habit * h)
{
	assert(h != NULL);
	int i = (int)h->points / 15;
	if (i > h->gates) {
		habit *r;
		h->gates++;
		printf("You've passed a gate, well done\n");
		printf("Treat yourself to: %s\n", h->reward);
		/* We need a new reward and a new habit */
		r = new_reward(h);
		if (r == NULL) {
			fprintf(stderr, "Failed to set new reward\n");
		}
		r = new_habit();
		if (r == NULL) {
			fprintf(stderr, "Failed to add new habit\n");
		}
	}
}

habit *new_reward(habit * h)
{
	assert(h != NULL);
	size_t nbytes = 80;
	char *str;
	str = (char *)malloc(nbytes * sizeof(char));
	if (str != NULL) {
		printf("Enter a new reward: ");
		getline(&str, &nbytes, stdin);
		sscanf(str, "%[^\t\n]", h->reward);
		free(str);
		return h;
	} else {
		fprintf(stderr, "Out of memory\n");
		exit(OUT_OF_MEMORY);
	}
}

/* Add a new habit */
habit *new_habit()
{
	/* Add code for adding a new habit */
	if (habits == NULL) {
		habits = (habit *) malloc(nrec * sizeof(habit));
		if (habits == NULL) {
			 /*ERROR*/ fprintf(stderr, "Out of memory\n");
			exit(OUT_OF_MEMORY);
		}
	}

	/* Resize array of habits if necessary */
	if (nhabit == nrec) {
		nrec += 10;
		habits = xrealloc(habits, nrec * sizeof(habit));
		if (habits == NULL) {
			return NULL;
		}
	}

	size_t nbytes = 80;
	char *str;
	str = (char *)malloc(nbytes * sizeof(char));
	if (str != NULL) {
		printf("Enter a new habit: ");
		getline(&str, &nbytes, stdin);
		sscanf(str, "%[^\t\n]", habits[nhabit].habit);
		printf("Enter a reward: ");
		getline(&str, &nbytes, stdin);
		sscanf(str, "%[^\t\n]", habits[nhabit].reward);
		free(str);
	} else {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}

	nhabit++;
	return &habits[nhabit - 1];
}

/* Wrapper with error handling around realloc */
void *xrealloc(void *ptr, size_t size)
{
	assert(ptr != NULL);
	assert(size > 0);
	void *value = realloc(ptr, size);
	if (value == 0) {
		/* ERROR */
		fprintf(stderr, "Out of memory\n");
		exit(OUT_OF_MEMORY);
	}
	return value;
}

/* Once a habit has been selected, ask if it has been completed */
void perform_habit(habit * h)
{
	assert(h != NULL);
	size_t nbytes = 5;
	char buf[nbytes];
	char *str;
	printf("Habit: %s (points: %f)\n", h->habit, h->points);
	printf("Did you perform habit? [y/n]: ");
	str = (char *)malloc(nbytes * sizeof(char));
	if (str != NULL) {
		getline(&str, &nbytes, stdin);
		sscanf(str, "%s", buf);
		if (strcmp(buf, "y") == 0) {	/* if matched */
			add_rand_point(h);
			check_gates(h);
		} else if (strcmp(buf, "n") != 0) {
			fprintf(stderr, "%s not a valid input\n", buf);
		}
		free(str);
	} else {
		fprintf(stderr, "Out of memory\n");
		exit(OUT_OF_MEMORY);
	}
}

/* Print out the programs usage instructions */
static void usage()
{
	printf("TODO\n");
}
