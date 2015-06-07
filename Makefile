CC = gcc
CFLAGS = -Wall -g -lpthread -lrt

all : project pociag

project : project.o 
	$(CC) $(CFLAGS) -o $@ $^
		
pociag : pociag.o 
	$(CC) $(CFLAGS) -o $@ $^
	
%.o : %.c
	$(CC) $(CFLAGS) -c $<


.PHONY : clean
clean :
	rm -f *.o project pociag $(OBJS)
