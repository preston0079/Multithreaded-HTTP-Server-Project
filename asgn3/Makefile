#------------------------------------------------------------------------------
#Preston Nguyen
#prhunguy
#asgn3
#------------------------------------------------------------------------------

# FORMATS  = $(SOURCES:%.c=%.fmt)


# CC       = clang
# CFLAGS   = -Wall -Wpedantic -Werror -Wextra

# all: queue rwlock

# queue: queue.o
# 	$(CC) -o queue queue.o

# rwlock: rwlock.o
# 	$(CC) -o rwlock rwlock.o


# %: %.o
# 	$(CC) -o $@ $^

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $<

# clean:
# 	rm -f queue rwlock *.o

# format:
# 	clang-format -i -style=file *.[ch]

EXECBINS  = queue rwlock
SOURCES  = $(wildcard *.c)
OBJECTS  = $(SOURCES:%.c=%.o)
FORMATS  = $(SOURCES:%.c=%.fmt)

CC       = clang
FORMAT   = clang-format
CFLAGS   = -Wall -Wpedantic -Werror -Wextra -g

.PHONY: all clean format

all: $(EXECBINS)

#$(EXECBIN): $(OBJECTS)
#	$(CC) -o $@ $^

%: %.o
	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXECBINS) $(OBJECTS)

format: $(FORMATS)

%.fmt: %.c
	$(FORMAT) -i $<
	touch $@


