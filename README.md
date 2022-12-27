# Pokemon Server Application

## What is this application?
This application allows for **(local) client-server functionality** to occur in C. The program enables clients to **request** a list of pokemon of varying types and **save** them to their local desktop in text files. 

## Choice of programming language
This application was written in C for several reasons
- Fine access to threads and low-level memory allows the querying operations to be very fast.
- Low memory footprint allows operations to be deployed on weaker hardware.
- Multithreading support allows for the server to handle several clients at once with minimal performance overhead.

## Video of the application running:
https://user-images.githubusercontent.com/81977350/209725059-d7cf019e-9532-4d14-a0d5-491a4d32f326.mp4

## Instructions to compile and run the application:

## Linux
1. Use the MakeFile under the src/ directory to compile the C files
2. Open up at  two terminals, one for the server and another for the clients (can have more)
3. In one of the terminals, run the server executable by typing ./server
4. Once the server is running, type in the file that you want to read from (by default, it is pokemon.csv)
5. In the other terminal, run the client executable by typing ./client
6. Once there, the terminal will open up the options on what can be done in the program.

## Potential Improvements and Advancements
- Moving the data to a server/off the local computer and allowing the server to query data to a server elsewhere
- Allowing clients to add new pokemon as needed
