CC=gcc
CFLAGS=-g -Wall

all: habit.c
	${CC} ${CFLAGS} habit.c -o habit
