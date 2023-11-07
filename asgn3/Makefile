#------------------------------------------------------------------------------
#Preston Nguyen
#prhunguy
#asgn3
#------------------------------------------------------------------------------

# FORMATS  = $(SOURCES:%.c=%.fmt)


CC       = clang
CFLAGS   = -Wall -Wpedantic -Werror -Wextra

all: queue rwlock

queue: queue.o
	$(CC) -o queue queue.o

rwlock: rwlock.o
	$(CC) -o rwlock rwlock.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f queue rwlock *.o

format:
	clang-format -i -style=file *.[ch]

