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
static int get_line(char **line, size_t * len, FILE * f);
static char *get_habit(char *line);
static char *get_reward(char *line);
static char *get_string_at_field(char *line, int nfield);
static int get_int_at_field(char *line, int nfield);
static int get_score(char *line);
static int get_ngates(char *line);
static void record_habit(char *id);
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

static char *get_reward(char *line)
{
	return get_string_at_field(line, 2);
}

static char *get_habit(char *line)
{
	return get_string_at_field(line, 1);
}

static char *get_string_at_field(char *line, int nfield)
{
	assert(line);
	char *str;
	size_t len = 128;
	char c;
	int i = 0, j = 0;
	int field = 0;

	str = malloc(len * sizeof(char));
	if (str == NULL) {
		fprintf(stderr, "ERROR allocating memory: %s\n", __func__);
		return NULL;
	}

	c = line[i];
	while (c != '\0' && field < (nfield + 1)) {
		if (c == '\t') {
			field++;
		} else if (field == nfield) {
			if (j == (len - 1)) {
				char *t;
				t = realloc(str, 2 * len * sizeof(char));
				if (t == NULL) {
					fprintf(stderr,
						"ERROR allocating memory: %s\n",
						__func__);
					free(str);
					return NULL;
				} else {
					str = t;
					len *= 2;
				}
			}
			str[j] = c;
			j++;
		}
		i++;
		c = line[i];
	}
	str[j] = '\0';
	return str;
}

static int get_score(char *line)
{
	return get_int_at_field(line, 3);
}

static int get_ngates(char *line)
{
	return get_int_at_field(line, 4);
}

static int get_int_at_field(char *line, int nfield)
{
	char *strscore = get_string_at_field(line, nfield);
	if (strscore != NULL) {
		int score = (int)strtol(strscore, NULL, 0);
		free(strscore);
		return score;
	} else {
		fprintf(stderr, "ERROR extracting score from line \"%s\": %s\n",
			line, __func__);
		return -1;
	}
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
	char *line = NULL;
	char *path;
	FILE *f;
	int id = 0;
	size_t bufsize = 0;
	int nchar = 0;

	path = get_default_storage_path();
	f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "Unable to open file \"%s\": %s\n",
			path, __func__);
		free(path);
		return -1;
	}
	free(path);

	while (true) {
		int id_tmp;
		nchar = get_line(&line, &bufsize, f);
		if (nchar < 0) {
			break;
		} else {
			sscanf(line, "%d\t", &id_tmp);
			if (id_tmp > id) {
				id = id_tmp;
			}
		}
		free(line);
	}

	fclose(f);
	return id + 1;
}

/* Returns number of characters in lineptr, -1 if EOF or error.
 * Caller must free the returned memory assigned to lineptr. */
static int get_line(char **lineptr, size_t * len, FILE * f)
{
	char c;
	char *line = NULL;
	int counter = 0;
	size_t bufsize = 80;

	assert(f);

	line = malloc(bufsize * sizeof(char));
	if (line == NULL) {
		fprintf(stderr, "Error in malloc: %s\n", __func__);
		return -1;
	}

	c = getc(f);
	if (c == EOF) {
		free(line);
		return -1;
	}
	while ((c != '\n') && (c != EOF)) {
		if (counter == (bufsize - 1)) {
			char *tmp = realloc(line, 2 * bufsize * sizeof(char));
			if (tmp != NULL) {
				line = tmp;
				bufsize *= 2;
			} else {
				fprintf(stderr,
					"Error reallocing memory: %s\n",
					__func__);
				return -1;
			}
		}
		line[counter] = c;
		++counter;
		c = getc(f);
	}
	line[counter] = '\0';
	*lineptr = line;
	*len = bufsize;

	return counter + 1;
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
		fprintf(stderr, "Unable to open file \"%s\": %s\n",
			path, __func__);
		free(path);
		exit(-1);
	}

	while ((c = getc(f)) != EOF) {
		fputc(c, stdout);
	}
}

static void record_habit(char *strid)
{
	long int id = strtol(strid, NULL, 0);
	FILE *tempfile = tmpfile();
	FILE *f = NULL;
	char *path = get_default_storage_path();

	char *line = NULL;
	size_t bufsize = 0;
	int nchar;

	if (path == NULL) {
		fprintf(stderr,
			"ERROR failed to get default storage path: %s\n",
			__func__);
		fclose(tempfile);
		return;
	}

	f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "ERROR opening file %s: %s\n", path, __func__);
		fclose(tempfile);
		return;
	}
	// Find habit with 'id'
	while (true) {
		int id_tmp;
		nchar = get_line(&line, &bufsize, f);
		if (nchar < 0) {
			break;
		} else {
			sscanf(line, "%d\t", &id_tmp);
			if (id_tmp == id) {
				//update record
				char *habit = get_habit(line);
				char *reward = get_reward(line);
				int score = get_score(line);
				int ngates = get_ngates(line);
				fprintf(tempfile,
					"%d\t%s\t%s\t%d\t%d\n", (int)id,
					habit, reward, score, ngates);
				printf("%d\t%s\t%s\t%d\t%d\n", (int)id,
				       habit, reward, score, ngates);
			} else {
				fprintf(tempfile, "%s\n", line);
			}
		}
		free(line);
	}

	fclose(f);
	f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "ERROR opening file %s: %s\n", path, __func__);
		fclose(tempfile);
		return;
	}
	// Copy file to ~/.habit
	char c;
	while ((c = getc(tempfile)) != EOF) {
		printf("%c", c);
		putc(c, f);
	}
	fclose(f);
	fclose(tempfile);
	// Write out habit data to temp file up to the record we're updating

	// Write out an updated record

	// Write out the rest of the records

	// Write change information to stdout
}

int main(int argc, char *argv[])
{
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
					if (argc > 2) {
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
					if (argc > 1) {
						record_habit(argv[1]);
						++argv;
						--argc;
					} else {
						usage();
					}
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
