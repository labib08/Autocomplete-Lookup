# define C compiler & flags
CC = gcc
CFLAGS = -Wall -g

# define libraries to be linked (for example -lm)
LDLIBS =

# define sets of header source files and object files
SRC_DICT2 = driver.c
SRC_DICT3 = main.c

# OBJ is the same as SRC, just replace .c with .o
OBJ_DICT2 = $(SRC_DICT2:.c=.o)
OBJ_DICT3 = $(SRC_DICT3:.c=.o)

# define the executable names
EXE_DICT2 = dict2
EXE_DICT3 = dict3

# build dict2
$(EXE_DICT2): $(OBJ_DICT2)
	$(CC) $(CFLAGS) -o $(EXE_DICT2) $(OBJ_DICT2) $(LDLIBS)

# build dict3
$(EXE_DICT3): $(OBJ_DICT3)
	$(CC) $(CFLAGS) -o $(EXE_DICT3) $(OBJ_DICT3) $(LDLIBS)

# clean rule
clean:
	rm -f $(OBJ_DICT2) $(OBJ_DICT3) $(EXE_DICT2) $(EXE_DICT3)
