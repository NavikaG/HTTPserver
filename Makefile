#makefile 
CFLAGS= -g -Wall -Wextra -Wpedantic -Wshadow -O2 -pthread
SOURCES=loadbalancer.c
EXEBIN=loadbalancer

all:    $(EXEBIN)

$(EXEBIN) :   $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -o $(EXEBIN)
clean:
	rm -f $(EXEBIN)
	
spotless:
	rm -f $(EXEBIN)