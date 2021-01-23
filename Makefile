CC = gcc
CFLAGS = -Wall -Wextra -Werror

sshell: sshell.o

sshell.o: sshell.c

clean:
	rm -f sshell sshell.o
