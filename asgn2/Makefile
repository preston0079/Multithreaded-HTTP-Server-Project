#------------------------------------------------------------------------------
#Preston Nguyen
#prhunguy
#asgn2
#------------------------------------------------------------------------------


all: httpserver

httpserver: httpserver.o 
	$(CC) -o httpserver httpserver.o asgn2_helper_funcs.a

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f httpserver *.o

format:
	clang-format -i -style=file *.[ch]
