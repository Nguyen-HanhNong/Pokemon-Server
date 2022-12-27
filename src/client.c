/*****************************************************************************/
/* */
/* client.c */
/* Purpose: This is the file where the Pokemon Query Client (PQC) operations begin. The program allows a user to get the pokemon that match a certain type and then write those pokemon to a file of their choice. */
/* How to use: Make sure to compile the file and then link this file when compiling the executable for the program. This is already done for you in the MakeFile. Once you run the executable, you will be prompted different information in the terminal and instructions will be shown at each step and also in the Readme.txt provided */
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

//importing the header file included with the program to get access to its functions, constants and structs
#include "client.h"

/* This function is the function that is ran when the client.c program is first started */
/* Parameters: None */
/* Return values: int which determines whether the program ran sucessfully  */
/* Side effets: creates variables which allocates memory, create and run threads, create sockets to communicate with other programs */
int main() {

  int clientSocket;                  //Integer representing the socket the client is communicating with
  int status;                        //Integer containing the integer returned from the connect function
  struct sockaddr_in clientAddress;  //Struct containing the IP of the client, the PORT the client is going to connect to and the family 

  /* All char pointers, each representing a different result of user input */
  char *gamer_choice = NULL;            //Choice from the menu received from the user
  char *type_choice = NULL;             //Pokemon type received from user
  char *user_file_name_choice = NULL;   //File name to save to recieved from the user 

  /* Two threads, one to do reading operations and one to do saving operations */
  pthread_t read_thread;
  pthread_t save_thread;

  /* The variable that contains the dynamic array but also all other properties to read files and write to files*/
  DynamicArrayType *dynamic_array = NULL;
  dynamic_array = (DynamicArrayType*)malloc(sizeof(DynamicArrayType)); /* Allocating memories to the dynamic_array*/

  /* Check if memory is allocated properly, print error message and exit if not */
  if(dynamic_array == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    return C_NOK;
  }

  /* Initializing the three elements of the DynamicArrayType struct to default values */
  dynamic_array->darray_size = 0;
  dynamic_array->darray_elements = NULL;
  dynamic_array->extra_pokemon_data = NULL;
  
  /* Allocating memory for the struct that will hold the additional data of the dynamic array including information needed to manipulate threads, read from files and save to files. */
  dynamic_array->extra_pokemon_data = (ExpandedThreadType*)malloc(sizeof(ExpandedThreadType));
  
  /* Check if memory is allocated properly, print error message and exit if not */
  if(dynamic_array->extra_pokemon_data == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    return C_NOK;
  }

  /* Initializing most of the properties of the dynamic_array and extra_pokemon_data variables to default values*/
  dynamic_array->extra_pokemon_data->number_of_pokemon_sucesfully_saved = 0;
  dynamic_array->extra_pokemon_data->number_of_saved_files = 0;
  dynamic_array->extra_pokemon_data->number_of_successful_queries = 0;
  dynamic_array->extra_pokemon_data->curr_type_being_read = 0;
  dynamic_array->extra_pokemon_data->all_types_being_read_size = 0;
  dynamic_array->extra_pokemon_data->all_types_being_read = NULL;
  dynamic_array->extra_pokemon_data->all_file_names = NULL;
  dynamic_array->extra_pokemon_data->name_of_saved_file = NULL;
  dynamic_array->extra_pokemon_data->thread_is_paused = C_NOK;
  dynamic_array->extra_pokemon_data->thread_is_running = C_NOK;

  /* Initializing the mutex and cond variables */
  pthread_mutex_init(&dynamic_array->extra_pokemon_data->mutex, NULL);
  pthread_cond_init(&dynamic_array->extra_pokemon_data->cond, NULL);

  /* Allocating memory to the pointer that will contain all the types that will be read */
  dynamic_array->extra_pokemon_data->all_types_being_read = (char **)malloc(sizeof(char *) *(dynamic_array->extra_pokemon_data->all_types_being_read_size + 1));

  /* Check if memory is allocated properly, print error message and exit if not */
  if(dynamic_array->extra_pokemon_data->all_types_being_read == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    return C_NOK;
  }
  
  /* Allocating memory to the pointer that will contain all the file names that will be saved to */
  dynamic_array->extra_pokemon_data->all_file_names = (char **)malloc(sizeof(char *) *(dynamic_array->extra_pokemon_data->all_types_being_read_size + 1));

  /* Check if memory is allocated properly, print error message and exit if not */
  if(dynamic_array->extra_pokemon_data->all_file_names == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    return C_NOK;
  }

  // Create socket with the TCP protocol
  clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (clientSocket < 0) { //Check if socket is created successfully
    printf("*** CLIENT ERROR: Could open socket.\n");
    exit(-1);
  }

  // Setup address of the client
  memset(&clientAddress, 0, sizeof(clientAddress));
  clientAddress.sin_family = AF_INET;
  clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
  clientAddress.sin_port = htons((unsigned short) SERVER_PORT);

  // Connect to server and check whether the client connected to the server successfully
  status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
  if (status < 0) {
    close(clientSocket);
    printf("Unable to establish connection to the PPS! \n");
    exit(-1);
  }

  // Set the client_socket property of the dynamic_array to contain the socket the client connected to
  dynamic_array->extra_pokemon_data->client_socket = clientSocket;

  /* Loop forever until the user tells the program they want to quit */
  while (1) {

     /* Print the menu of options to the user and get the input of which options the user wants to do */
    printf("What do you want to do? Here are the following options: \n a. Type search \n b. Save results \n c. Exit the program \n");
    scanf("%ms", &gamer_choice);

    /* If the user selects option a */
    if(strcmp(gamer_choice, "a") == 0) {
      /* Free the memory allocated to the gamer_choice variable and type_choice variable if they have memory allocated to them*/
      free_char_pointer(&gamer_choice);
      free_char_pointer(&type_choice);

      /* Get the pokemon type the user wants to search for and store it in type_choice variable*/
      printf("What pokemon type do you want to check for?: ");
      scanf("%ms", &type_choice);

      /* Check if the pokemon type the user inputted was a valid pokemon type, if not, send the user back to the menu */
      if(check_valid_pokemon_type(type_choice) == C_NOK) {
          printf("Invalid pokemon type. Please enter a valid pokemon type. \n");
          continue;
      }

      /* Check whether this is not the first pokemon to be stored inside all_types_being_read */
      /* if not, reallocate more space for all_types_being_read to handle a new type being added to the array */
      if(dynamic_array->extra_pokemon_data->curr_type_being_read > 0) {
          dynamic_array->extra_pokemon_data->all_types_being_read = realloc(dynamic_array->extra_pokemon_data->all_types_being_read, sizeof(char *) * (dynamic_array->extra_pokemon_data->all_types_being_read_size + 1));

          /* Check if memory is allocated properly, print error message and exit if not */
          if(dynamic_array->extra_pokemon_data->all_types_being_read == NULL) {
            printf("An error occured while allocating memory. The program will now exit \n");
            return C_NOK;
          }
      }

      /* Allocate space inside all_tyeps_being_read to store the pokemon_type */
      dynamic_array->extra_pokemon_data->all_types_being_read[dynamic_array->extra_pokemon_data->all_types_being_read_size] = malloc(sizeof(char) * (strlen(type_choice) + 1));

      /* Check if memory is allocated properly, print error message and exit if not */
      if(dynamic_array->extra_pokemon_data->all_types_being_read[dynamic_array->extra_pokemon_data->all_types_being_read_size] == NULL) {
        printf("An error occured while allocating memory. The program will now exit \n");
        return C_NOK;
      }

      /* Copy the pokemon type the user inputted to the all_types_being_read double char pointer */
      strcpy(dynamic_array->extra_pokemon_data->all_types_being_read[dynamic_array->extra_pokemon_data->all_types_being_read_size], type_choice);
      dynamic_array->extra_pokemon_data->all_types_being_read_size++; // Increment the size counter for the all_types_being_read variable by 1 

      /* Check if a thread has already been ran, if it is, join the thread so it frees up the thread resources and stops memory leaks from occuring */
      if(dynamic_array->extra_pokemon_data->all_types_being_read_size > 0 && dynamic_array->extra_pokemon_data->number_of_successful_queries > 0 && dynamic_array->extra_pokemon_data->thread_is_running == C_NOK) {
        pthread_join(read_thread, NULL);
      }
      
      /* Create a thread to handle the reading operations with the server */
      pthread_create(&read_thread, NULL, read_pokemon, (void*)dynamic_array);
    }
    /* If the user selected the saving operation */
    else if(strcmp(gamer_choice, "b") == 0) {

      /* Free the memory allocated to the gamer_choice variable and user_file_name_choice variable if they have memory allocated to them*/
      free_char_pointer(&gamer_choice);
      free_char_pointer(&user_file_name_choice);

      /* Stop the user from saving data if they haven't read any data into the program*/
      if(dynamic_array->extra_pokemon_data->number_of_successful_queries == 0) {
          printf("No queries have been completed yet so there is nothing to save. Wait for a query to finish. \n");
          continue;
      }

      /* Get the name of the file the user wants to save to*/
      printf("Enter the name of file which will contain the query results accumulated to this point: \n");
      scanf("%ms", &user_file_name_choice);

      /* Loop until the user enters a valid name for a save file */
      while(1) {
        /* Check if the name of the file is invalid, if it is, get a new file name from the user and continue looping*/
        if(strcmp(user_file_name_choice, "") == 0 || strcmp(user_file_name_choice, ".") == 0 || user_file_name_choice[0] == '\0' || user_file_name_choice[0] == ' ') {
          printf("Unable to create the new file. Please enter the name of the file again \n");
          free_char_pointer(&user_file_name_choice);
          scanf("%ms", &user_file_name_choice);
          continue;
        }

        /* Try to open a file with the name the user entered, if it doesn't open properly, get a new file name from the user and continue looping */
        FILE *temporary_file = fopen(user_file_name_choice, "a");
        if(!temporary_file) {
          printf("Unable to create the new file. Please enter the name of the file again \n");
          free_char_pointer(&user_file_name_choice);
          scanf("%ms", &user_file_name_choice);
          continue;
        }
        fclose(temporary_file); //Close the temporary file
        break;                  //Break out of the loop
      }

      /* Store the file name the user inputted inside the extra_pokemon_data struct */
      dynamic_array->extra_pokemon_data->name_of_saved_file = user_file_name_choice;

      /* Check if a thread has already been ran, if it is, join the thread so it frees up the thread resources and stops memory leaks from occuring */
      if(dynamic_array->extra_pokemon_data->number_of_saved_files > 0 && dynamic_array->extra_pokemon_data->thread_is_paused == C_NOK) {
        pthread_join(save_thread, NULL);
      }

      /* Create a thread to handle the saving operations to the disk */
      pthread_create(&save_thread, NULL, write_pokemon, (void *)dynamic_array);
    }
    /* If the user selects the exit the program option */
    else if(strcmp(gamer_choice, "c") == 0) {
      
      /* Check if the read and save thread have been running and are finished, and join the threads to free up their resources and to avoid memory leaks */
      if(dynamic_array->extra_pokemon_data->all_types_being_read_size > 0 && dynamic_array->extra_pokemon_data->number_of_successful_queries > 0 && dynamic_array->extra_pokemon_data->thread_is_running == C_OK) {
        pthread_cancel(read_thread);
      }
      if(dynamic_array->extra_pokemon_data->number_of_saved_files > 0) {
        pthread_join(save_thread, NULL);
      }

      /* Print the information about queries and files listed in Use Case 4 with the print_final_information function */
      print_final_information(dynamic_array);

      /* Free all the char pointers that contain information inputted by the user, if they contain information*/
      free_char_pointer(&gamer_choice);
      free_char_pointer(&type_choice);
      free_char_pointer(&user_file_name_choice);

      /* Freeing the memory for all the different files saved if the user saved AT LEAST one file */
      if(dynamic_array->extra_pokemon_data->number_of_saved_files > 0) {
        for (int i = 0; i < dynamic_array->extra_pokemon_data->number_of_saved_files; i++) {
          free(dynamic_array->extra_pokemon_data->all_file_names[i]);
        }
      }

      /* Freeing up all the different pokemon types that are searched in the program if it contains AT LEAST one pokemon type*/
      if(dynamic_array->extra_pokemon_data->all_types_being_read_size > 0) {
        for (int i = 0; i < dynamic_array->extra_pokemon_data->all_types_being_read_size; i++) {
          free(dynamic_array->extra_pokemon_data->all_types_being_read[i]);
        }
      }
      /* Free the double char pointer, all_types_being_read */
      free(dynamic_array->extra_pokemon_data->all_types_being_read);

      /* Freeing the memory associated with the dynamic array inside dynamic_array if the user read at least one pokemon */
      if(dynamic_array->darray_size != 0) {
        for (int i=0; i<dynamic_array->darray_size; ++i) {
          free(dynamic_array->darray_elements[i]->name);
          free(dynamic_array->darray_elements[i]->first_type);
          free(dynamic_array->darray_elements[i]->second_type);
          free(dynamic_array->darray_elements[i]);
        }
        free(dynamic_array->darray_elements);
      }

      /* Destroy the mutex and cond variable  */
      if(pthread_mutex_destroy(&dynamic_array->extra_pokemon_data->mutex) != 0) {
        printf("An error occured while destroying the mutex. The program will now exit \n");
        return C_NOK;
      }
      if(pthread_cond_destroy(&dynamic_array->extra_pokemon_data->cond) != 0) {
        printf("An error occured while destroying the condition variable. The program will now exit \n");
        return C_NOK;
      }
      /* Free the all_file_names double pointer, then free the extra_pokemon_data struct and then free the DynamicArrayType struct itself */
      free(dynamic_array->extra_pokemon_data->all_file_names);
      free(dynamic_array->extra_pokemon_data);
      free(dynamic_array);

      send(clientSocket, "stop", sizeof("stop"), 0);  //Send a stop message to the server to indicate that the server should shut down
      close(clientSocket);                            //Close the socket connecting to the server
      printf("CLIENT: Shutting down.\n");             //Print a message recognizing the client program is shutting down
      pthread_exit(NULL);                             //Quit the program
    }
    /* If the user inputted an invalid input, print a message and then return them to the loop where the menu sisi printed again */
    else {
      printf("Invalid choice. Please enter a valid choice. \n");
      free_char_pointer(&gamer_choice);
      continue;
    }
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

/* This function checks if a input string matches the name of a pokemon type */
/* Parameters: *input_type - input (the char pointer that is being checked) */
/* Return values: int, return C_OK (0) if input_type is a valid pokemon type, return C_NOK (-1) if input_type is not a valid pokemon type */
/* Side effects: lots of conditional checking */
int check_valid_pokemon_type(char *input_type) {

  /* Use strcmp to compare to all valid pokemon types according to the Pokemon franchise */
  if (strcmp(input_type, "Normal") == 0 || strcmp(input_type, "Fire") == 0 || strcmp(input_type, "Water") == 0 || strcmp(input_type, "Grass") == 0 || strcmp(input_type, "Electric") == 0 || strcmp(input_type, "Ice") == 0 || strcmp(input_type, "Fighting") == 0 || strcmp(input_type, "Poison") == 0 || strcmp(input_type, "Ground") == 0 || strcmp(input_type, "Flying") == 0 || strcmp(input_type, "Psychic") == 0 || strcmp(input_type, "Bug") == 0 || strcmp(input_type, "Rock") == 0 || strcmp(input_type, "Ghost") == 0 || strcmp(input_type, "Dragon") == 0 || strcmp(input_type, "Dark") == 0 || strcmp(input_type, "Steel") == 0 || strcmp(input_type, "Fairy") == 0) {
    return C_OK;
  }
  return C_NOK;
}

/* This function handles the reading operations for pokemon of a certain type through communication with a server and stores them inside a dynamic array */
/* NOTE: This function is primarily copied from the function read_students from ReadCSV.c from the Professor's Sample Code */
/* Parameters: *arg - input/output (void* casted parameter containing a DynamicArrayType struct) */
/* Return values: nothing since the function is void  */
/* Side effects: uses socket functions to communicate with the server, uses mutex and cond to pause/unpause the thread running this function */
void *read_pokemon(void *arg) {

  DynamicArrayType *dynamic_array = (DynamicArrayType *)arg; //variable containing the DynamicArrayType variable which is passed into the function as a void*
  char buffer[MAX_MESSAGE_BUFFER_SIZE]; //variable containing the buffer that is used to get the number of bytes of the pokemon message being sent from the server
  int bytesReceived = 0;                //variable that contains the return value of the recv function 
 
  /* Check if the mutex has been locked properly, print error message and exit program if not */
  if(pthread_mutex_lock(&dynamic_array->extra_pokemon_data->mutex) != 0) {
      printf("\n The mutex lock operation has failed\n");
      exit(EXIT_FAILURE);
  }
  /* Make the thread idle waiting if the mutex_passing_condition is not met */
  while(dynamic_array->extra_pokemon_data->thread_is_paused == C_OK) {
    pthread_cond_wait(&dynamic_array->extra_pokemon_data->cond, &dynamic_array->extra_pokemon_data->mutex);
  }

  /* Set the thread_is_running variable to C_OK to indicate that the thread running the read_pokemon function is working */
  dynamic_array->extra_pokemon_data->thread_is_running = C_OK;

  sleep(0.5); //Sleep for 0.5 seconds to allow the server time to receive the message to the server

  /* Send the pokemon_type that is currently being read over to the server and check if it sent properly */
  if(send(dynamic_array->extra_pokemon_data->client_socket, dynamic_array->extra_pokemon_data->all_types_being_read[dynamic_array->extra_pokemon_data->curr_type_being_read], strlen(dynamic_array->extra_pokemon_data->all_types_being_read[dynamic_array->extra_pokemon_data->curr_type_being_read]), 0) == -1) {
    printf("SERVER ERROR: Failed to send pokemon type to server \n");
    exit(EXIT_FAILURE);
  }

  /* Store the amount of bytes of the next message inside the buffer variable and store the return output of the recv function inside bytesReceived*/
  bytesReceived = recv(dynamic_array->extra_pokemon_data->client_socket, buffer, MAX_MESSAGE_BUFFER_SIZE, 0);

  /* Check if the recv function returned properly, print error message and exit program if not */
  if(bytesReceived < 0) {
    printf("SERVER ERROR: Failed to receive pokemon type from server \n");
    exit(EXIT_FAILURE);
  }

  /* Null terminate the buffer string */
  buffer[bytesReceived] = '\0';

  /* Convert the buffer to a (long) integer with atol and store it in bytes_from_message variable */
  int bytes_from_message = atol(buffer); 
  char *pokemon_message = (char *)malloc((bytes_from_message+1) * sizeof(char)); //allocate memory to a char pointer to hold the message containing all the pokemon data read from the server

  /* Store the giant string containing the pokemon data read from the server inside pokemon_message and store the return output of the recv function inside bytesReceived*/
  bytesReceived = recv(dynamic_array->extra_pokemon_data->client_socket, pokemon_message, bytes_from_message, 0);

  /* Check if the recv function returned properly, print error message and exit program if not */
  if(bytesReceived < 0) {
    printf("SERVER ERROR: Failed to receive pokemon type from server \n");
    exit(EXIT_FAILURE);
  }

  /* Null-terminate the pokemon_message string */
  pokemon_message[bytesReceived] = '\0';
  char *pointer_to_pokemon_message = pokemon_message; //create a new char pointer to point to the pokemon_message so we can free it later

  /* Create a char pointer and allocate memory to it to store the number of pokemon sent message that is going to be sent from the server */
  char *number_of_pokemon_message = (char *)malloc(MAX_MESSAGE_BUFFER_SIZE * sizeof(char));

  /* Check if memory is allocated properly, print error message and exit if not */
  if(number_of_pokemon_message == NULL) {
    printf("SERVER ERROR: Failed to allocate memory for number_of_pokemon_message \n");
    exit(EXIT_FAILURE);
  }
  
  /* Store the number of pokemon sent message inside the number_of_pokemon_message variable and store the return output of the recv function inside bytesReceived*/
  bytesReceived = recv(dynamic_array->extra_pokemon_data->client_socket, number_of_pokemon_message, MAX_MESSAGE_BUFFER_SIZE, 0);

  /* Check if the recv function returned properly, print error message and exit program if not */
  if(bytesReceived < 0) {
    printf("SERVER ERROR: Failed to receive number of pokemon from server \n");
    exit(EXIT_FAILURE);
  }

  /* Null terminate the number_of_pokemon_message string */
  number_of_pokemon_message[bytesReceived] = '\0';
  int number_of_pokemon = strtol(number_of_pokemon_message, NULL, 10); //conver the number_of_pokemon message into an int and store it in a separate variable

  /* Loop through every pokemon in the pokemon_message */
  for(int i = 0; i < number_of_pokemon; i++) {
    char *pokemon_data_line = strsep(&pokemon_message, "|"); //Get the current pokemon and store it in the pokemon_data_line variable
    PokemonType *curr_pokemon; //create a new variable to store the current pokemon being read
    line_to_pokemon(pokemon_data_line, &curr_pokemon, SEPARATOR); //convert the string into a pokemon and store it in curr_pokemon
    add_pokemon(curr_pokemon, dynamic_array); //add the pokemon to the dynamic array with add_pokemon
  }


  dynamic_array->extra_pokemon_data->number_of_successful_queries += 1;  //Increase the number of successful queries by 1
  dynamic_array->extra_pokemon_data->number_of_pokemon_sucesfully_saved += number_of_pokemon;   /* Incrased the number of pokemon that are sucessfully saved by the amount that were added to the dynamic array during the function processs */
  dynamic_array->extra_pokemon_data->curr_type_being_read += 1; //increase the index that the type is going to be read from in the all_types_being_read array by 1

  pthread_mutex_unlock(&dynamic_array->extra_pokemon_data->mutex); //unlock the mutex
  
  /* Free the memory allocated to the pokemon_message via pointer_to_pokemon_message pointer and free the number_of_pokemon_message variable */
  free(number_of_pokemon_message);
  free(pointer_to_pokemon_message);

  /* Set the thread_is_running variable to C_NOK to indicate that the thread is no longer working */
  dynamic_array->extra_pokemon_data->thread_is_running = C_NOK;

  return C_OK;
}

/* This function writes all the pokemon that are succesfully read into the dynamic array into a file */
/* NOTE: This function is primarily copied from the function write_students from ReadCSV.c from the Professor's Sample Code */
/* Parameters: *arg - input/output (void* casted parameter containing a DynamicArrayType struct) */
/* Return values: nothing since the function is void  */
/* Side effects: opens and closes a file, dynamically allocates memory to multiple different structures, locks and unlocks a mutex and sends signals to other functions to unlock/lock their threads */
void *write_pokemon(void *arg) {
  DynamicArrayType *temporary = (DynamicArrayType *)arg; //variable containing the DynamicArrayType variable which is passed into the function as a void*

  //change the mutex_passing_condition to n, which locks the other thread if its reading
  temporary->extra_pokemon_data->thread_is_paused = C_OK;
  
  /* Officially lock the mutex */
  pthread_mutex_lock(&temporary->extra_pokemon_data->mutex);

  sleep(0.25); //sleep for 0.25 seconds to give time for the server to receive commands

  /* Send a request to the server to stop reading in pokemon while this writing operation is running */
  if(send(temporary->extra_pokemon_data->client_socket, "pause", strlen("pause"), 0) == -1) {
    printf("SERVER ERROR: Failed to send pokemon type to server \n");
    exit(EXIT_FAILURE);
  }

  /* Open a new file at the location specified the user with the name_of_file variable in the appending mode */
  FILE *data_csv_file = data_csv_file = fopen(temporary->extra_pokemon_data->name_of_saved_file, "a");

  /* Loop through every pokemon that is sucessfully saved*/
  for (int i = 0; i < temporary->extra_pokemon_data->number_of_pokemon_sucesfully_saved; i++)
  {
    /* If the data file is not opened properly, print error message and quit the program */
    if (!data_csv_file) {
      printf("File was not able to be opened. Program closing now!");
      exit(EXIT_FAILURE);
    }
    else
    {
      char line[MAX_LENGTH] = "";  //variable that will hold the char conversion of the pokemon
      pokemon_to_line(line, temporary->darray_elements[i], SEPARATOR); //conver the pokemon to a string

      /* Write the pokemon as a string to the file using fprintf */
      fprintf(data_csv_file, "%s\n", line);
    }
  }

  /* If the file is opened properly, close the file*/
  if (data_csv_file != NULL) {
    fclose(data_csv_file);
  } 

  /* Check if the program has a saved a file succesfully before */
  if(temporary->extra_pokemon_data->number_of_saved_files == 0) {
   
    //Allocate memory for the all_file_names variable to store the name of a file
    temporary->extra_pokemon_data->all_file_names[temporary->extra_pokemon_data->number_of_saved_files] = (char *)malloc(strlen(temporary->extra_pokemon_data->name_of_saved_file) + 1);

    /* Check if memory is allocated properly, print error message and exit if not */
    if(temporary->extra_pokemon_data->all_file_names[temporary->extra_pokemon_data->number_of_saved_files] == NULL) {
      printf("An error occured while allocating memory. The program will now exit \n");
      exit(EXIT_FAILURE);
    }

    /* Copy the file name that the program just saved to the all_file_names double pointer */
    strcpy(temporary->extra_pokemon_data->all_file_names[temporary->extra_pokemon_data->number_of_saved_files], temporary->extra_pokemon_data->name_of_saved_file);
    temporary->extra_pokemon_data->number_of_saved_files++; //Increase the counter for number of saved files by 1
  }
  else {
    
    /* Check if the file we just wrote to is a new file and has not already been saved inside name_of_file*/
    if(check_for_previous_file(temporary, temporary->extra_pokemon_data->name_of_saved_file) == C_NOK) {

    /* Allocate more memory to the all_file_names array */
    temporary->extra_pokemon_data->all_file_names = realloc(temporary->extra_pokemon_data->all_file_names, sizeof(char *) * (temporary->extra_pokemon_data->number_of_saved_files + 1));

    /* Check if memory is allocated properly, print error message and exit if not */
    if(!temporary->extra_pokemon_data->all_file_names) {
      printf("An error occured while allocating memory. The program will now exit \n");
      exit(EXIT_FAILURE);
    }
  
    /* Allocate memory inside the all_file_names array for the new file to be added*/
    temporary->extra_pokemon_data->all_file_names[temporary->extra_pokemon_data->number_of_saved_files] = (char *)malloc(strlen(temporary->extra_pokemon_data->name_of_saved_file) + 1);

    /* Check if memory is allocated properly, print error message and exit if not */
    if(temporary->extra_pokemon_data->all_file_names[temporary->extra_pokemon_data->number_of_saved_files] == NULL) {
      printf("An error occured while allocating memory. The program will now exit \n");
      exit(EXIT_FAILURE);
    }

    /* Copy the file name that the program just saved to the all_file_names double pointer */
    strcpy(temporary->extra_pokemon_data->all_file_names[temporary->extra_pokemon_data->number_of_saved_files], temporary->extra_pokemon_data->name_of_saved_file);
    temporary->extra_pokemon_data->number_of_saved_files++; //Increase the counter for number of saved files by 1
    }
  }

  //change the mutex_passing_condition to n, which unlocks the other thread if its reading
  temporary->extra_pokemon_data->thread_is_paused = C_NOK;
  pthread_mutex_unlock(&temporary->extra_pokemon_data->mutex); //Unlock the mutex
  pthread_cond_signal(&temporary->extra_pokemon_data->cond); //Send the signal over to the other thread to officially allow it to start reading again

  /* Send a unpause message to the server to unpause the thread containing reading operations in the server, to allow it to continue its work */
  if(send(temporary->extra_pokemon_data->client_socket, "unpause", strlen("unpause"), 0) == -1) {
    printf("SERVER ERROR: Failed to send pokemon type to server \n");
    exit(EXIT_FAILURE);
  }

  return C_OK;
}

/* This function prints the information about queries and files, outlined in Use Case 4 in the assignment specification */
/* Parameters: *temporary - the massive dynamic array that contains all the properties necessary to read, write and store information */
/* Return values: nothing since it's a void function */
/* Side effects: iterating through the all_file_names pointer array */
void print_final_information(DynamicArrayType *temporary) {
  
  /* Print the total number of queries */
  printf("The total number of queries completed successfully during the session is %d: \n", temporary->extra_pokemon_data->number_of_successful_queries);

  /* Print all the new files created in the instance of the program by looping through all the unique file names stored
  in the all_file_names pointer array */
  printf("The name of the new files created/wrote to during this session are: \n");
  for (int i = 0; i < temporary->extra_pokemon_data->number_of_saved_files; i++) {
    printf("%s \n",temporary->extra_pokemon_data->all_file_names[i]);
  }
}

/* This function convers a string containing pokemon information into a pokemon struct containing information about all properties about a Pokemon */
/* NOTE: This function is primarily copied from the function line_to_student from ReadCSV.c from the Professor's Sample Code */
/* Parameters: *line - input (line that information is being parsed from), **new_pokemon - input/output (pokemon structure that is being initialized with memory and has its properties filled), *separator - input (the character that separates the infomration in the file) */
/* Return values: nothing since the function is void  */
/* Side effects: uses strsep which modifies the input string, allocate memory on the heap to PokemonType struct */
void line_to_pokemon(char *line, PokemonType **new_pokemon, char *separator) {

  /* Create a char pointer for each property of the pokemon being parsed from line using 
  strsep function */
  char *number = strsep(&line, separator);
  char *name = strsep(&line, separator);
  char *first_type = strsep(&line, separator);
  char *second_type = strsep(&line, separator);
  char *total_stats = strsep(&line, separator);
  char *health_points = strsep(&line, separator);
  char *attack = strsep(&line, separator);
  char *defense = strsep(&line, separator);
  char *special_attack = strsep(&line, separator);
  char *special_defense = strsep(&line, separator);
  char *speed = strsep(&line, separator);
  char *generation = strsep(&line, separator);
  char *legendary = strsep(&line, separator);

  /* Allocate memory on the heap for the pokemon pointer */
  *new_pokemon = (PokemonType*) malloc(sizeof(PokemonType));

  /* Check if memory is allocated properly, print error message and exit if not */
  if(*new_pokemon == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    exit(EXIT_FAILURE);
  }

   /* Allocate memory on the heap for each of the char * properties of the pokemon */ 
  (*new_pokemon)->name = (char *)malloc(strlen(name) + 1);
  (*new_pokemon)->first_type = (char *)malloc(strlen(first_type) + 1);
  (*new_pokemon)->second_type = (char *)malloc(strlen(second_type) + 1);

  /* Check if memory is allocated properly, print error message and exit if not */
  if((*new_pokemon)->name == NULL || (*new_pokemon)->first_type == NULL || (*new_pokemon)->second_type == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    exit(EXIT_FAILURE);
  }

  /* Setting the name, first_type and second_type char pointers to contain only the null terminating string so I don't have to manually put it in later after using memset */
  memset((*new_pokemon)->name, '\0', strlen(name)+1);
  memset((*new_pokemon)->first_type, '\0', strlen(first_type)+1);
  memset((*new_pokemon)->second_type, '\0', strlen(second_type)+1);

  /* Copying over the memory containing the characters for the name, first_type and second_type
  properties */
  memcpy((*new_pokemon)->name, name, strlen(name) + 1);
  memcpy((*new_pokemon)->first_type, first_type, strlen(first_type) + 1);
  memcpy((*new_pokemon)->second_type, second_type, strlen(second_type) + 1);

  /* For all of the numeric properties of the pokemon, use strtol to convert the string containing the information about the property to a numeric data type */
  (*new_pokemon)->number = strtol(number, NULL, 10);
  (*new_pokemon)->total_stats = strtol(total_stats, NULL, 10);
  (*new_pokemon)->health_points = strtol(health_points, NULL, 10);
  (*new_pokemon)->attack = strtol(attack, NULL, 0);
  (*new_pokemon)->defense = strtol(defense, NULL, 0);
  (*new_pokemon)->special_attack = strtol(special_attack, NULL, 0);
  (*new_pokemon)->special_defense = strtol(special_defense, NULL, 0);
  (*new_pokemon)->speed = strtol(speed, NULL, 0);
  (*new_pokemon)->generation = strtol(generation, NULL, 0);

  /* Check if the pokemon is legendary or not, fill the legendary property in new_pokemon with a n char if its not a legendary and a y if it is a legendary*/
  if (strcmp(legendary, "False") == 0)
  {
    (*new_pokemon)->legendary = 'n';
    }
    else {
      (*new_pokemon)->legendary = 'y';
    }
}

/* This function adds a pokemon to the end of the dynamic array and also resizes the dynamic array as necessary. */
/* NOTE: This function is primarily copied from the function addStudent() of p7-dynArr.c from Module 9 */
/* Parameters: *pokemon - input (a pointer to a pokemon), *pokemon_dynamic_array - input/output (a pointer to a dynamic array) */
/* Return values: nothing since it's a void function */
/* Side effects: expanding the size of the dynamic array, addition of memory to the heap */
void add_pokemon(PokemonType *pokemon, DynamicArrayType *pokemon_dynamic_array) {

  PokemonType **new_pokemon_array; //new pointer array of Pokemon pointers
  int curr_index; //int representing the current index of the dynamic array

  int insert_position = pokemon_dynamic_array->darray_size; //the position we will be inserting the new pokemon at

  //checking for validity of inputs
  if (insert_position < 0 || insert_position > pokemon_dynamic_array->darray_size) {
    return;
  }

  /*allocating meory for the new pointer array of pokemon pointers */
  new_pokemon_array = calloc(pokemon_dynamic_array->darray_size+1, sizeof(PokemonType*));

  /* Check if memory is allocated properly, print error message and exit if not */
  if(new_pokemon_array == NULL) {
    printf("An error occured while allocating memory. The program will now exit \n");
    exit(EXIT_FAILURE);
  }

  /* copying all the pokemon before the position of the newly inserted transaction */
  for(curr_index=0; curr_index<insert_position; ++curr_index) {
    new_pokemon_array[curr_index] = pokemon_dynamic_array->darray_elements[curr_index];
  }

  //inserting the new transaction at the specified position
  new_pokemon_array[curr_index] = pokemon;

  /* freeing the old dynamic array and then copying the new one over to the old DynamicArrayType structure and increasing the size by one*/
  free(pokemon_dynamic_array->darray_elements);
  pokemon_dynamic_array->darray_elements = new_pokemon_array;
  pokemon_dynamic_array->darray_size++;
}

/* This function converts a pokemon struct to a string that contains all the properties of that Pokemon */
/* NOTE: This function is primarily copied from the function student_to_line from ReadCSV.c from the Professor's Sample Code */
/* Parameters: *line_to_write - input/output (the string that we will the pokemon properties to), *pokemon_to_write - input (the pokemon struct that we will read from), *separator - input (the character that separates each of the properties of the pokemon inside the line_to_write string) */
/* Return values: nothing since the function is void  */
/* Side effects: uses sprintf to copy over numeric data from the pokemon_to_write struct into a string format */
void pokemon_to_line(char *line_to_write, const PokemonType *pokemon_to_write, char *separator) {

  /* Creating a char array for each of the numeric properties from the Pokemon that we have to convert to a string */
  char number[MAX_LENGTH];
  char total[MAX_LENGTH];
  char health_points[MAX_LENGTH];
  char attack[MAX_LENGTH];
  char defense[MAX_LENGTH];
  char special_attack[MAX_LENGTH];
  char special_defense[MAX_LENGTH];
  char speed[MAX_LENGTH];
  char generation[MAX_LENGTH];

  /* Use sprintf to convert the numeric properties of the Pokemon into a char array */
  sprintf(number, "%d", pokemon_to_write->number);
  sprintf(total, "%d", pokemon_to_write->total_stats);
  sprintf(health_points, "%d", pokemon_to_write->health_points);
  sprintf(attack, "%d", pokemon_to_write->attack);
  sprintf(defense, "%d", pokemon_to_write->defense);
  sprintf(special_attack, "%d", pokemon_to_write->special_attack);
  sprintf(special_defense, "%d", pokemon_to_write->special_defense);
  sprintf(speed, "%d", pokemon_to_write->speed);
  sprintf(generation, "%d", pokemon_to_write->generation);

  /* Concatenate each of the properties (except the legendary property) to the line_to_wqrite variable */
  line_to_write = strcat(line_to_write, number);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, pokemon_to_write->name);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, pokemon_to_write->first_type);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, pokemon_to_write->second_type);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, total);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, health_points);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, attack);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, defense);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, special_attack);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, special_defense);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, speed);
  line_to_write = strcat(line_to_write, separator);
  line_to_write = strcat(line_to_write, generation);
  line_to_write = strcat(line_to_write, separator); 

  /* Check the legendary variable, if its the 'y' char, concatenate the word True to the line_to_write, if its the 'n char, concatenate the word Fasle to the line_to_write */
  if(pokemon_to_write->legendary == 'y') {
    line_to_write = strcat(line_to_write, "True");
  }
  else {
    line_to_write = strcat(line_to_write, "False");
  }
}

/* This function checks if a file is already stored in a DynamicArrayType struct */
/* Parameters: *temporary - input (the DynamicArrayType struct that is passed in), *check_file - input (the file that we're checking if it's already been saved) */
/* Return values: int, representing whether the file has been stored or not */
/* Side effects: looping through char double pointer, using strcmp function */
int check_for_previous_file(DynamicArrayType *temporary, char *check_file) {

  /* Loop through every saved file in the temporary DynamicArrayType and compare the names, if they match, then return C_OK (0)*/
  for (int i = 0; i < temporary->extra_pokemon_data->number_of_saved_files; i++) {
    if (strcmp(temporary->extra_pokemon_data->all_file_names[i], check_file) == 0) {
      return C_OK;
    }
  }
  return C_NOK; /* If they're no matches, return C_NOK (-1) */
}
