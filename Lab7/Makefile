all: smt

smt: reciver sender

reciver: reciver.c
	gcc -Wall -Wextra -o reciver reciver.c

sender: sender.c
	gcc -Wall -Wextra -o sender sender.c

clean:
	rm -f reciver sender