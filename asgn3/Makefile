#------------------------------------------------------------------------------
#Preston Nguyen
#prhunguy
#asgn3
#------------------------------------------------------------------------------

EXECBINS  = queue.o rwlock.o
SOURCES  = $(wildcard *.c)
OBJECTS  = $(SOURCES:%.c=%.o)

CC       = clang
FORMAT   = .clang-format
CFLAGS   = -Wall -Wextra -Werror -pedantic

.PHONY: all clean format

all: $(EXECBINS)

$(EXECBIN): $(OBJECTS)
	$(CC) -o $@ $^

%: %.o
	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXECBINS) $(OBJECTS)

format:
	clang-format -i -style=file *.[ch]



