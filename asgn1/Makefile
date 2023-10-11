#------------------------------------------------------------------------------
#Preston Nguyen
#prhunguy
#asgn1
#------------------------------------------------------------------------------

# FORMATS  = $(SOURCES:%.c=%.fmt)


CC       = clang
CFLAGS   = -Wall -Wpedantic -Werror -Wextra

all: memory

memory: memory.o
	$(CC) -o memory memory.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f memory *.o

format:
	clang-format -i -style=file *.[ch]


