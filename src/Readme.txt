name: Nguyen-Hanh Nong
studentNumber: 101220611

List of source files to be submitted:

client.c
client.h 
Makefile
Nong_Design_Document_Assignment_4.pdf
pokemon.csv
server.c 
server.h 

Instructions to compile and running server.c:

I've attached a Makefile to allow for easy compilation and testing of my program.

Step 1: Type the make command in the terminal where the Makefile and files are located

make 

Step 2: Type in the following command into the terminal
./server

Step 3: Once in the program, type in the location of the pokemon.csv file you want to read from (With the tar file, there's one in this directory if you want to use this one)

Step 4: Afterwards, you should see the program idle. It is waiting for you to run the client executable (look below for instructions for how to compile and run client.c)

Instructions to compile and running client.c:

I've attached a Makefile to allow for easy compilation and testing of my program.

Step 1: Type the make command in the terminal where the Makefile and files are located

make 

Step 2: Type in the following command into the terminal
./client 

Step 3: You should see both the server and client start to activate. You will be in the heart of the program, where you will have three options.

a. Type search
b. Save results
c. Exit the program

Type in a, b or c into the terminal to select one of the options
