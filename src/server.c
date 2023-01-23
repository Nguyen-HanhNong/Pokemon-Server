/*****************************************************************************/
/* */
/* server.c */
/* Purpose: This is the file where the Pokemon Property Server (PPS) operations begin. The program reads from a file which is specified by the user and sends data back and forth to a client program. */
/* How to use: Make sure to compile the file and then link this file when compiling the executable for the program. This is already done for you in the MakeFile. Once you run the executable, you will be prompted to enter a name of a file which will be a read from. */
/* Authors: Nguyen-Hanh Nong */
/* Revision: Revision 2.0 */
/* */
/*****************************************************************************/

//libraries that will be used in the program
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

//importing the header file included with the program to get access to its functions, constants and structs
#include "server.h"

/* This function is the function that is ran when the server.c program is first started */
/* Parameters: None */
/* Return values: int which determines whether the program ran sucessfully  */
/* Side effets: creates variables which allocates memory, create and run threads, create sockets to communicate with other programs */
int main() {

  char *file_name = NULL;                             // name of the file that will be read from
  int serverSocket;                                   // the socket of the server
  int clientSocket;                                   // the socket of the client that the server will connect to
  int status;                                         // return variable from bind and listen functions
  int bytesRcv;                                       // return variable from recv function
  socklen_t addrSize;                                 // size of address 
  int new_request = 0;                                // amount of new pokemon types that have not been read
  struct sockaddr_in serverAddress;                   // address of the server
  struct sockaddr_in clientAddr;                      // address of the client
  ServerReadType *readStruct;                         // struct that contains all information to read from the file
  ServerReadType readAddress;                         // struct that readStruct pointer points to
  pthread_t server_read_thread;                       // thread to read from a file

  /* Loop forever until the user tells the user they want to quit the program or input a valid file name*/
  while(1) {
    /* Get user input regarding the location of the file that the user wants to open */
    printf("Enter the name of the pokemon.csv file or type q to quit the program: \n");
    scanf("%ms", &file_name);

    /* If the user types q, free any dynamically alloacted data and mutex and quit the program */ 
    if(strcmp(file_name, "q") == 0) {
      free_char_pointer(&file_name);
      exit(C_OK);
    }
    /* If the location of the file we want to open doesn't exist, or it is not possible to open, print message regarding this and then prompts the Gamer to enter the name of the file again, or to exit the program.*/
    if(file_exists(file_name) == C_NOK) {
      free_char_pointer(&file_name);
      printf("Pokemon file is not found. Please enter the name of the file again. \n");
      continue;
    }
    /* Exit the loop if a file exists and can be opened */
    break; 
  }

  /* Initializing the readStruct variable with default values*/
  readStruct = &readAddress;
  readStruct->boolean_counter = 0;
  readStruct->file_name = file_name;
  readStruct->pokemon_types_array = NULL;
  readStruct->curr_number_of_pokemon_types = 0;
  readStruct->pokemon_types_array_size = 0;
  readStruct->thread_is_paused = C_NOK;

  //Allocates memory for the pokemon_types_array double char pointer inside the readStruct struct
  readStruct->pokemon_types_array = (char **)malloc(sizeof(char *) *(readStruct->pokemon_types_array_size + 1));

  /* Check if memory is allocated properly, print error message and exit if not */
  if(readStruct->pokemon_types_array == NULL) {
    printf("Error allocating memory for pokemon_types_array. \n");
    exit(C_NOK);
  }

  // Create the server socket
  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverSocket < 0) {
    printf("*** SERVER ERROR: Could not open socket.\n");
    exit(-1);
  }

  // Setup the server address
  memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

  // Bind the server socket
  status = bind(serverSocket,  (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (status < 0) {
    printf("*** SERVER ERROR: Could not bind socket.\n");
    close(serverSocket);
    exit(-1);
  }

  // Set up the line-up to handle up to 5 clients in line 
  status = listen(serverSocket, 5);
  if (status < 0) {
    printf("*** SERVER ERROR: Could not listen on socket.\n");
    close(serverSocket);
    exit(-1);
  }

  // Waiting for a client to connect, and checking if the connection was successful
  addrSize = sizeof(clientAddr);
  clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
  if (clientSocket < 0) {
      printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
      exit(-1);
  }

  printf("SERVER: Received client connection.\n");
  readStruct->client_socket = clientSocket; // storing the client socket in readStruct

  /* Initializing the mutex and cond variables inside readStruct and checking whether they initialized properly */
  if(pthread_mutex_init(&readStruct->lock, NULL) != 0) {
      printf("SERVER ERROR: mutex failed to initialize \n");
      exit(EXIT_FAILURE);
  }
  if(pthread_cond_init(&readStruct->cond, NULL) != 0) {
      printf("SERVER ERROR: cond failed to initialize \n");
      exit(EXIT_FAILURE);
  }

  printf("SERVER: Starting server \n");
  
  // Go into infinite loop to talk to client
  while (1) {
    char buffer[MAX_MESSAGE_BUFFER_SIZE];               // buffer to store data that is received from a client program
    // Get the message from the client
    bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
    buffer[bytesRcv] = '\0'; // null terminate the message

    /* Check whether the message is a valid message, if not, print an error message and then wait for new messages to come in */
    if(strcmp(buffer, " ") == 0 || strcmp(buffer, "") == 0) {
      printf("DEBUG: Empty message received. \n");
      continue;
    }

    printf("SERVER: Received client request: %s\n", buffer); //print the message the server received from the client

    /* If the message was pause */
    if(strcmp(buffer,"pause") == 0) {
      if(readStruct->boolean_counter == 1) { //Check if the read thread is already running
        pthread_mutex_lock(&readStruct->lock); //lock the mutex
        readStruct->thread_is_paused = C_OK; //set the thread_is_paused variable to C_OK to indicate that the thread is paused
      }
    }
    /* If the message was unapuse */
    else if(strcmp(buffer, "unpause") == 0) {
      if(readStruct->boolean_counter == 1) { //Check is the read thread is already running
        pthread_mutex_lock(&readStruct->lock); //lock the mutex
        readStruct->thread_is_paused = C_NOK; //set the thread_is_paused variable to C_NOK to indicate that the thread is unpaused
        pthread_cond_signal(&readStruct->cond); //signal the cond variable to unpause/wake the thread
      }
    }
    /* If the message was stop, print a message and then break out of the infinite loop to close the server */
    else if(strcmp(buffer, "stop") == 0) {
      printf("SERVER: Received stop request. \n");
      break;
    }
    /* If it was not any of the messages above, assume that the message was a pokemon type*/
    else {

      readStruct->pokemon_types_array_size += 1; //increase the pokemon_types_array_size variable by 1
      new_request += 1;                          //increase the new_request variable by 1

      /* If there is already a pokemon type stored inside pokemon_types_array, reallocate more memory to store another pokemon type */
      if(readStruct->pokemon_types_array_size > 1) {
        readStruct->pokemon_types_array = (char **)realloc(readStruct->pokemon_types_array, sizeof(char *) *(readStruct->pokemon_types_array_size + 1));
      }

      /* Allocate memory to store the pokemon type inside pokemon_types_array */
      readStruct->pokemon_types_array[readStruct->pokemon_types_array_size - 1] = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
      strcpy(readStruct->pokemon_types_array[readStruct->pokemon_types_array_size - 1], buffer); //copy the pokemon type into the pokemon_types_array
    }
    /* Check whether there is a new pokemon_type for the read thread to read */
    if(readStruct->boolean_counter == 0 && new_request > 0 && readStruct->thread_is_paused == C_NOK) {
      
      /* If there is a pokemon_type that has been read, join the read thread once it's finished to collect the resources and to avoid memory leaks */
      if(readStruct->curr_number_of_pokemon_types > 0) {
        pthread_join(server_read_thread, NULL);
      }
      
      /* Create a thread to read pokemon of a certain type */
      pthread_create(&server_read_thread, NULL, server_read_pokemon, (void *)readStruct); 
      new_request -= 1; //reduce the amount of new requests to read by 1
      readStruct->curr_number_of_pokemon_types += 1; //increase the index to read inside pokemon_types_array by 1
    }
  }

  /* Join a thread if a pokemon_type has been stored inside the pokemon_types_arary */
  if(readStruct->pokemon_types_array_size > 0) {
    pthread_join(server_read_thread, NULL);
  }

  /* Free memory from the name of the file the user entered */
  free_char_pointer(&file_name);

  /* Free the memory from all the pokemon types stored inside pokemon_types_array then free the double char pointer itself */
  for(int i = 0; i < readStruct->pokemon_types_array_size; i++) {
    free_char_pointer(&readStruct->pokemon_types_array[i]);
  }
  free(readStruct->pokemon_types_array);

  // Don't forget to close the sockets!
  close(serverSocket);
  printf("SERVER: Shutting down.\n");
  pthread_exit(NULL);
}

/* This function checks if a file exists at a specified location  */
/* NOTE: This function is primarily copied from the function fexists from main_bin.c from Tutorial 7 */
/* Parameters: *location (input) - the location of the file we want to check that exists */
/* Return values: int representing whether a file_exists or not  */
/* Side effects: uses FileIO functions from stdio.h, fclose() function can fail and produce undefined behaviour */
int file_exists(char *location) {

    FILE *fp = NULL; //variable containing the file that will be opened
    fp = fopen(location, "r"); //opening the file and assigning result to fp

    /* If the file was not able to be opened/doesn't exist, return C_NOK (-1) */
    if(fp == NULL) {
        return C_NOK;
    }

    /* If the file does exist, close the file and return C_OK (0) */
    fclose(fp);
    return C_OK;
}

/* This function handles the reading pokemon from a file and then store those pokemon inside a string and then send the information to a client program */
/* NOTE: This function is primarily copied from the function read_students from ReadCSV.c from the Professor's Sample Code */
/* Parameters: *arg - input/output (void* casted parameter containing a ServerReadType struct) */
/* Return values: nothing since the function is void  */
/* Side effects: uses socket functions to communicate with the server, uses mutex and cond to pause/unpause the thread running this function, uses FileIO functions to read from a file */
void *server_read_pokemon(void *arg) {

  /* Cast the void parameter to a ServerReadType struct */
  ServerReadType* passed_in = (ServerReadType*)arg;
  passed_in->boolean_counter = 1; //set the boolean_counter variable to 1 to indicate that the thread is running

  char *pokemon_send_string = NULL;       //String that will contain all the pokemon of a certain type
  char line[MAX_MESSAGE_BUFFER_SIZE];     //String that will contain a line from the file
  int num_lines = 0;                      //Variable that will count the number of lines in the file
  int saved = 0;                          //Variable that will count the number of pokemon of a certain type that exist inside the file
  int curr_pokemon_index = passed_in->curr_number_of_pokemon_types - 1; //Int that represents the index of the pokemon type that we want to read from the file

  /* Open the file  in read mode */
  FILE *fp = fopen(passed_in->file_name, "r");

  /* If the file was not able to be opened, print an error message and exit the function */
  if(fp == NULL) {
      printf("SERVER ERROR: file failed to open \n");
      exit(EXIT_FAILURE);
  }

  /* Lock the mutex */
  pthread_mutex_lock(&passed_in->lock);

  /* Loop through every line of the file from top to bottom and save each line into line variable*/
  while (fscanf(fp, "%[^\n]\n", line) != EOF) {

    /* Make the thread idle waiting if the thread_is_paused condition is met */
    while(passed_in->thread_is_paused == C_OK) {
      pthread_cond_wait(&passed_in->cond, &passed_in->lock);
    }
    if (num_lines > 0) { /* Skip the header */
      /* Create a char pointer to store a copy of the line that is being scanned */
      char *type_check_string = (char *)malloc(strlen(line) + 1);

      /* Check if memory is allocated properly, print error message and exit if not */
      if(type_check_string == NULL) {
            printf("An error occured while allocating memory. The program will now exit \n");
            exit(EXIT_FAILURE);
      }

      /* Copy the line being read to type_check_string variable and null-terminated the copied string*/
      strcpy(type_check_string, line);
      type_check_string[strlen(line)] = '\0';
      
      /* Check if the type of the pokemon being read matches the one we want to search for*/
      if(check_pokemon_type(type_check_string, passed_in->pokemon_types_array[curr_pokemon_index], SEPARATOR) == 0) {
        
        /* Null-terminate the line */
        line[strlen(line)] = '\0';
        /* Check if any pokemon have been saved or not */
        if (saved == 0)
        { /* Allocate memory inside pokemon_send_string for the string, the null-terminating character and the separator, '|' */
          pokemon_send_string = (char *)malloc(sizeof(char) * (strlen(line) + 2)); 
          /* Check if memory is allocated properly, print error message and exit if not */
          if(pokemon_send_string == NULL) { 
            printf("An error occured while allocating memory. The program will now exit \n");
            exit(EXIT_FAILURE);
          }
          strcpy(pokemon_send_string, line); //Copy the line to the pokemon_send_string variable
        }
        else {
          /* Re-allocate memory inside pokemon_send_string for the string, the null-terminating character and the separator, '|' */
          pokemon_send_string = (char *)realloc(pokemon_send_string, sizeof(char) * (strlen(pokemon_send_string) + strlen(line) + 2));
          /* Check if memory is allocated properly, print error message and exit if not */
          if(pokemon_send_string == NULL)
          {
            printf("An error occured while allocating memory. The program will now exit \n");
            exit(EXIT_FAILURE);
          }
          strcat(pokemon_send_string, line); //Concatenate the line to the pokemon_send_string variable
        }
        strcat(pokemon_send_string, "|"); //Concatenate the | character to separate the pokemon in the string
        saved += 1; //Increment the number of pokemon that have been saved by 1
      }
      /* Free the copied string */
      free(type_check_string); 
    }
    num_lines++; //Increase the num_lines counter by 1
  }
  fclose(fp); //Close the file

  /* Reallocate space in pokemon_send_string for a null-terminating character */
  pokemon_send_string = (char *)realloc(pokemon_send_string, sizeof(char) * (strlen(pokemon_send_string) + 1));
  /* Check if memory is allocated properly, print error message and exit if not */
  if (pokemon_send_string == NULL)
  {
    printf("An error occured while allocating memory. The program will now exit \n");
    exit(EXIT_FAILURE);
  }
  /* Add the null-terminating character to the end of the string */
  pokemon_send_string[strlen(pokemon_send_string)] = '\0';

  char pokemon_send_string_size[MAX_MESSAGE_BUFFER_SIZE];     //String that will contain the amount of bytes of the pokemon_send_string variable
  char number_of_pokemon_send_string[MAX_MESSAGE_BUFFER_SIZE]; //String that will contain the number of pokemon that have been saved

  /* Convert the size of the pokemon_send_string into a string and store inside pokemon_send_string_size */
  sprintf(pokemon_send_string_size, "%ld", strlen(pokemon_send_string));
  pokemon_send_string_size[strlen(pokemon_send_string_size)] = '\0'; //Null-terminate the string

  /* Convert the number of pokemon that have been saved into a string and store inside number_of_pokemon_send_string */
  sprintf(number_of_pokemon_send_string, "%d", saved);
  number_of_pokemon_send_string[strlen(number_of_pokemon_send_string)] = '\0'; //Null-terminate the string

  /* Make the thread idle waiting if the thread_is_paused condition is met */
  while(passed_in->thread_is_paused == C_OK) {
    pthread_cond_wait(&passed_in->cond, &passed_in->lock);
  }
  /* Send the size of the pokemon_send_string variable to the client and check whether it was sent successfully */
  if(send(passed_in->client_socket, pokemon_send_string_size, strlen(pokemon_send_string_size), 0) == -1) {
    printf("SERVER ERROR: failed to send message bytes to client \n");
    exit(EXIT_FAILURE);
  }

  sleep(0.25); //Make the thread sleep for 0.25 seconds to stop the messages from being sent too fast and corrupting

  /* Send the string containing the pokemon read from the file to the client and check whether it was sent sucessfully */
  if(send(passed_in->client_socket, pokemon_send_string, strlen(pokemon_send_string), 0) == -1) {
    printf("SERVER ERROR: failed to send message to client \n");
    exit(EXIT_FAILURE);
  }

  /* Send the string containing the number of pokemon saved to the client and check whether it was sent sucessfully */
  if(send(passed_in->client_socket, number_of_pokemon_send_string, strlen(number_of_pokemon_send_string), 0) == -1) {
    printf("SERVER ERROR: failed to send message to client \n");
    exit(EXIT_FAILURE);
  }

  /* Unlock the mutex */
  pthread_mutex_unlock(&passed_in->lock);
  free(pokemon_send_string); //Free the memory allocated to the pokemon_send_string variable

  passed_in->boolean_counter = 0; //Set the boolean_counter to 0 to indicate that the thread is done running

  pthread_exit(NULL); //Exit the thread
}

/* This function checks if a pokemon's type matches with a type inputted by the user */
/* Parameters: *data_line - input (line that information is being parsed from), *ideal_type - input (the pokemon type we want to compare with), *separator - input (the character that separates the infomration in the file) */
/* Return values: int, C_OK (0) if the two types match and C_NOK (-1) if the two types do not match  */
/* Side effects: uses void casting to remove warning about unused variables, use strsep which modify the input string*/
int check_pokemon_type(char* data_line, char* ideal_type, char *separator) {

  /* Uses the strsep function twice to get to the information containing the pokemon's type in the data_line string, the variables are not used at all though */
  char *garbage_number = strsep(&data_line, separator);
  char *garbage_name = strsep(&data_line, separator);

  /* Void cast the functions to get rid of unused variable warning */
  (void)garbage_name;
  (void)garbage_number;

  /* Get the pokemon type in the string from the file */
  char *pokemon_type = strsep(&data_line, separator); 

  /* Use strcmp to see if the two Pokemon types match, return C_OK (0) if they do match, return C_NOK (-1) if they don't match */
  if(strcmp(pokemon_type, ideal_type) == 0) {
    return C_OK;
  }
  else {
    return C_NOK;
  }
}

/* This function frees data in a char pointer if it contains any dynamically allocated data */
/* Parameters: **char_pointer (input/output) - the pointer that is possibly being freed  */
/* Return values: int representing whether a file_exists or not  */
/* Side effects: possibly freeing dynamically allocated data */
void free_char_pointer(char **char_pointer) {
  /* Check whether data has been allocated to the pointer, if so, then free the data the data is pointing to and then set the pointer to point to NULL */
  if(*char_pointer != NULL) {
    free(*char_pointer);
    *char_pointer = NULL;
  }
}
