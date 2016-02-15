all: marcel_shell

marcel_shell: marcel_shell.o
	gcc -g -Wall -o marcel_shell marcel_shell.o

marcel_shell_shell.o: marcel_shell.cpp
	gcc -g -c marcel_shell.cpp

clean: 
	*.o marcel_shell
