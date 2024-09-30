all: smt

smt: cat_utilite grep_utilite

cat_utilite: src/cat.c
	gcc -Wall -Wextra -o cat_utilite src/cat.c

grep_utilite: src/grep.c
	gcc -Wall -Wextra -o grep_utilite src/grep.c

clean:
	rm -f cat_utilite grep_utilite
