/*****************************************************************************/
/* */
/* server.h */
/* */
/* Purpose: This is a header file that contains constants, structures and declaration of all functions used in the server.c file */
/* How to use: use #include "server.h" at the top of any .c files that you want to have server's function decalrations, structs and constants */
/* Authors: Nguyen-Hanh Nong */
/* Revision: Revision 2.0 */
/* */
/*****************************************************************************/

//Include guards to protect against double declarations
#ifndef SERVER_H_
#define SERVER_H_

//Other libraries that we will need
#include <stdio.h>
#include <pthread.h>

//Variety of constants defined
#define C_NOK -1                      //Constant to represent an error/not correct value
#define C_OK 0                        //Constant to represent a correct return value
#define SEPARATOR ","                 //Constant to represent a separator between values inside a file
#define SERVER_IP "127.0.0.1"         //Constant to represent the IP address the client will connect to
#define SERVER_PORT 6000              //Constant to represent the port that the client will connect to
#define MAX_MESSAGE_BUFFER_SIZE 100   //Constant to represent the largest message that can be sent to the server without forewarning

/* This is a structure that contains all the information that is needed to read pokemon information from the specified file, manipulate the reading thread and to store pokemon as a string. */
typedef struct ServerRead {
  char *file_name;                  //Name of the file that contains the pokemon information
  char boolean_counter;             //Boolean counter to determine whether the thread is running or not
  char **pokemon_types_array;       //Double pointer containing all the pokemon types that will be and has been read from the file
  char thread_is_paused;            //Char representing whether the thread is paused or not
  int client_socket;                //Socket that the server uses to communicate with the client
  int curr_number_of_pokemon_types; //Index of the current pokemon type that is being read from the file
  int pokemon_types_array_size;     //The amount of pokemon types inside the pokemon_types_array double char pointer
  pthread_mutex_t lock;             //Mutex that determines whether the reading thread is running or not
  pthread_cond_t cond;              //Condition variable that determines whether the reading thread is paused or not
} ServerReadType;

/* all function prototypes for functions in server.c */
int file_exists(char *location);
void *server_read_pokemon(void *arg);
int check_pokemon_type(char *data_line, char *ideal_type, char *separator);
void free_char_pointer(char **char_pointer);

#endif //end of header file