/*****************************************************************************/
/* */
/* client.h */
/* */
/* Purpose: This is a header file that contains constants, structures and declaration of all functions used in the client.c file. */
/* How to use: use #include "client.h" at the top of any .c files that you want to have client function decalrations, structs and constants */
/* Authors: Nguyen-Hanh Nong */
/* Revision: Revision 2.0 */
/* */
/*****************************************************************************/

//Include guards to protect against double declarations
#ifndef CLIENT_H_
#define CLIENT_H_

//Other libraries that we will need
#include <stdio.h>
#include <pthread.h>

//Variety of constants defined
#define C_NOK -1                      //Constant to represent an error/not correct value
#define C_OK 0                        //Constant to represent a correct return value
#define MAX_LENGTH 100                //Constant to represent the max length of a string
#define SEPARATOR ","                 //Constant to represent a separator between values inside a file
#define SERVER_IP "127.0.0.1"         //Constant to represent the IP address the client will connect to
#define SERVER_PORT 6000              //Constant to represent the port that the client will connect to
#define MAX_MESSAGE_BUFFER_SIZE 100   //Constant to represent the largest message that can be sent to the server without forewarning

/* This structure represents all the information a Pokemon has */
/* Each variable is a characteristic that will be read in from a file */
typedef struct Pokemon {
    short number;           //Number of pokemon
    char *name;             //Name of pokemon
    char *first_type;       //First type of pokemon
    char *second_type;      //Second type of pokemon if appplicable
    short total_stats;      //Sum of all stats of pokemon
    short health_points;    //HP of the pokemon
    short attack;           //attack stat of the pokemon
    short defense;          //defense stat of the pokemon
    short special_attack;   //special attack stat of the pokemon
    short special_defense;  //special defense stat of the pokemon
    short speed;            //speed stat of the pokemon
    short generation;       //generation the pokemon originated from
    char legendary;         //char representing whether the pokemon is a legendary pokemon or not
} PokemonType;

/* This structure contains the information that is needed to read pokemon information and to write pokemon informatino */
/* It also contains the mutex and condition variables to manipulate the running of threads */
typedef struct ExpandedThread {                    
  int number_of_pokemon_sucesfully_saved; //Number of pokemon that were successfully saved to the file
  int number_of_saved_files;              //Number of files that were successfully saved to disk
  int number_of_successful_queries;       //Number of queries that were successfully completed
  int client_socket;                      //Socket that the client uses to communicate with the server
  int curr_type_being_read;               //Current type of pokemon that is going to be read from the server
  int all_types_being_read_size;          //The number of types that will and have been read from the server
  char **all_types_being_read;            //Double pointer containing all the types that will and have been read from the server
  char **all_file_names;                  //Double pointer containing all file names saved to
  char *name_of_saved_file;               //Name of the current file being saved to
  char thread_is_paused;                  //Char representing whether the thread is paused or not
  char thread_is_running;                 //Char representing whether the thread is running or not
  pthread_mutex_t mutex;                  //Mutex that determines who has acces to this struct
  pthread_cond_t cond;                    //Condition that manipualtes the waiting of a thread
} ExpandedThreadType;

/* This structure contains a dynamic implementation from dynArr.c from Module 9 */
/* It also stores all the necessary variables to read from files, write to files, communicate
with the server, and to manipulate threads. */
typedef struct DynamicArray {
  int darray_size;                          //Number of elements inside dynamic array
  PokemonType **darray_elements;            // Array of pokemon where pokemon read from files are stored
  ExpandedThreadType *extra_pokemon_data;   //Structure that contains all the extra data needed to read from files, write to files, communicate with the server, and to manipulate threads
} DynamicArrayType;

/* all function prototypes for functions in client.c */
void free_char_pointer(char **char_pointer);
int check_valid_pokemon_type(char *input_type);
void *read_pokemon(void *arg);
void *write_pokemon(void *arg);
void print_final_information(DynamicArrayType *temporary);
void line_to_pokemon(char *line, PokemonType **new_pokemon, char *separator);
void add_pokemon(PokemonType *pokemon, DynamicArrayType *pokemon_dynamic_array);
int check_for_previous_file(DynamicArrayType *temporary, char *check_file);
void pokemon_to_line(char *line_to_write, const PokemonType *pokemon_to_write, char *separator);

#endif //end of header file