#File name: Makefile
#Author: Nguyen-Hanh Nong
#Function: This file contains the code that will run the preprocessing, compilation, assembly and linking stages to 
#get executables for the program.

#Variables and rules for the makefile
CC = gcc
CCOPTIONS = -Wall
OBJ = server.o client.o 
all: server client 

#Compiling the server and client executables
server: $(OBJ)
	$(CC) $(CCOPTIONS) -o server server.o -lpthread

client:	$(OBJ)
	$(CC) $(CCOPTIONS) -o client client.o -lpthread

#Linking the C files and header files for the server and client programs
server.o:	server.c server.h
	$(CC) $(CCOPTIONS) -c server.c

client.o:	client.c client.h
	$(CC) $(CCOPTIONS) -c client.c

#Clean function to delete .o and server and client executables 
clean:
	rm -f $(OBJ) server client
