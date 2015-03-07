/* Command line utility to practice habit forming using randomised rewards *
 * Author: Iestyn Pryce							   *
 * 09/04/2012
 * 02/03/2014 - minor improvements
 */
#include "habit.h"

#include <stdbool.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

const char *program_name;

typedef enum Actions { RECORD, UPDATE, DELETE } Action;

static int add_habit(char *habit, char *reward);
static char *get_default_storage_path();
static int get_id();
static int get_line(char **line, size_t * len, FILE * f);
static char *get_habit(char *line);
static char *get_reward(char *line);
static char *get_string_at_field(char *line, int nfield);
static int get_int_at_field(char *line, int nfield);
static int get_score(char *line);
static int get_ngates(char *line);
static int get_record_score();
static void update_habit(Action a, char *id, ...);
static void update_reward(FILE * f, int id, char *line, char *reward);
static void update_scores(FILE * f, int id, char *line);
static void tee_habit(FILE * f, int id, char *habit, char *reward, int score,
		      int gates);
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

	len = strlen(env) + 1;	/* Add one for '\0' char */
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

	fclose(f);
}

static void update_scores(FILE * f, int id, char *line)
{
	bool achived = false;

	char *habit = get_habit(line);
	char *reward = get_reward(line);
	int score = get_score(line);
	score += get_record_score();
	int ngates = get_ngates(line);

	int achived_gates = score / 15;
	if (achived_gates > ngates) {
		ngates++;
		achived = true;
	}
	tee_habit(f, (int)id, habit, reward, score, ngates);
	if (achived) {
		printf("Congratulations, you passed a gate! ");
		printf("Reward yourself with: %s\n", reward);
	}
}

static void update_habit(Action action, char *strid, ...)
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
	// Find habit with 'id'.
	while (true) {
		int id_tmp;
		nchar = get_line(&line, &bufsize, f);
		if (nchar < 0) {
			break;
		} else {
			sscanf(line, "%d\t", &id_tmp);
			if (id_tmp == id) {
				//update record
				switch (action) {
				case RECORD:
					update_scores(tempfile, id, line);
					break;
				case UPDATE:
					{
						va_list ap;
						va_start(ap, strid);
						update_reward(tempfile, id,
							      line, va_arg(ap,
									   char
									   *));
						va_end(ap);
					}
					break;
				case DELETE:
					// Do nothing i.e. don't print the record
					break;
				}
			} else {
				fprintf(tempfile, "%s\n", line);
			}
		}
		free(line);
	}

	// Close .habit, reset temp file to start.
	fclose(f);
	rewind(tempfile);

	// Reopen .habit file for writing.
	f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "ERROR opening file %s: %s\n", path, __func__);
		fclose(tempfile);
		return;
	}
	// Copy file to ~/.habit
	char c;
	while ((c = getc(tempfile)) != EOF) {
		putc(c, f);
	}
	fclose(f);
	fclose(tempfile);

}

static void update_reward(FILE * f, int id, char *line, char *reward)
{
	//update record
	char *habit = get_habit(line);
	int score = get_score(line);
	int ngates = get_ngates(line);
	printf("Reward for habit %d updated:\n", id);
	tee_habit(f, id, habit, reward, score, ngates);
}

/* Write a record to filehandle f and to stdout */
static void tee_habit(FILE * f, int id, char *habit, char *reward, int score,
		      int gates)
{
	fprintf(f, "%d\t%s\t%s\t%d\t%d\n", id, habit, reward, score, gates);
	printf("%d\t%s\t%s\t%d\t%d\n", id, habit, reward, score, gates);
}

int main(int argc, char *argv[])
{
	char *habit_file_path;

	program_name = argv[0];

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
				case 'd':	/* Delete habit */
					if (argc > 1) {
						update_habit(DELETE, argv[1]);
						++argv;
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
						update_habit(RECORD, argv[1]);
						++argv;
						--argc;
					} else {
						usage();
					}
					break;
				case 'u':	/* Update reward */
					if (argc > 2) {
						update_habit(UPDATE, argv[1],
							     argv[2]);
					} else {
						usage();
					}
					break;
				case 'h':	/* Output usage/help */
					usage();
					break;
				default:	/* Output usage/help if option is unrecognised */
					fprintf(stderr,
						"Unrecognised option: -%c\n",
						(*argv)[1]);
					usage();
				}
			}
			--argc;
			++argv;
		}
	}

	free(habit_file_path);
	return 0;
}

/* return a uniform random value in the range [1,10] */
static int get_record_score()
{
	return (rand() % 10) + 1;
}

/* Print out the programs usage instructions */
static void usage()
{
	printf("\
USAGE: %s [OPTIONS]\n\
\n", program_name);

	printf("\
OPTIONS:\n\
\n\
    -a <habit> <reward>     Add a new habit with a reward\n\
    -d <id>                 Delete habit <id>\n\
    -l                      List all habits (default when no options are given)\n\
    -r <id>                 Record that habit with <id> has been done\n\
    -u <id> <reward>        Update habit <id> with a new reward <reward>\n\
    -h                      This usage message\n\
\n\
");
}
