all: smt

smt: my_pipe my_fifo

my_pipe: pipe.c
	gcc -Wall -Wextra -o my_pipe pipe.c

my_fifo: fifo.c
	gcc -Wall -Wextra -o my_fifo fifo.c

clean:
	rm -f my_pipe my_fifo