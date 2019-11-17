/*

  Name: Mason McDaniel
  ID: 1001456342

*/

// Base project source from: https://github.com/CSE3320/Shell-Assignment
// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 2019 Mason McDaniel
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports ten arguments

#define MAX_NUM_HISTORY 14 		// Right now we only want to store 15 items in history (0-14)

// pidHistory and count are global variables. pidHistory keeps track of our pid numbers
// returned by the exec process. count is a simple counting variable that keeps track
// of how many commands have been entered by the user.
int pidHistory[15] = {0}, count = 0;
/* Execution takes in the file locations that we want to check and also the command
the user has inputted then tries to eecute it in each of the given files. If
the command does not exist in the given files then the execution will exit. 
Otherwise the command entered is successfully executed and execution ends. */
void execution(char cwd[], char usrLocal[], char usrBin[], char bin[], char **token);
/* processCMD processes the command sent in by the user. We take in the input
(token) and we check each of our given options to see if the command entered by 
the user is one that we wish to accept. We pass in pidHistory so we can print that 
out when listpids is issued. Likewise with history when history needs to be printed.
Essentially our key function. */
void processCMD(char ** token, int pidHistory[], char ** history);
/* handle_signal is passed in a signal to handle. If that signal is CTRL-Z we
want to pause the process currently running (pidHistory[count]), where count
is the current process number which corresponds to an array value for a PID */
static void handle_signal(int sig);


static void handle_signal(int sig)
{
	// if the signal (sig) we receie is CTRL-Z then execute our kill command
	if(sig == SIGTSTP)
	{
    // Kill sends the signal to the process we wish to sleep or pause
		kill(sig, pidHistory[count]);
	}
}

void execution(char cwd[], char usrLocal[], char usrBin[], char bin[], char ** token)
{
  // cwd is the current working directory we want to try to execute the command (token) in.
  // We use execvp so the additional args in token are run easily. It also means we can easily
  // get an error code back.
  execvp(cwd, token);
  if(errno != 2)
  {
    // We don't want to know if errno is 2 because that means 
    // it wasn't found in the folder. This is more important 
    // for the last exec. If we get that far then we do need to 
    // take error 2 (FNF) into account.
    perror("An error has occured ");
  }
  // In this exec we check if the command exists in folder usr/Local
  execvp(usrLocal, token);
  if(errno != 2)
  {
    perror("An error has occured "); 
  }
  // In this exec we check if the command exists in folder usr/Bin
  execvp(usrBin, token);
  if(errno != 2)
  {
    perror("An error has occured ");    
  }
  // In this exec we check if the command exists in folder bin
  execvp(bin, token);
  if(errno != 2)
  {
    perror("An error has occured "); 
  }
  // If we get to here it means the command issued by the user does 
  // not correspond to a program or file in any of the files we 
  // are checking. This means by definition errno should be 2, but
  // just in case we check for any value non-zero. 
  if(errno != 0)
  {
    printf("%s: Command not found.\n\n", token[0]);
  }
  // If we get to this point of the function nothing has run and the 
  // command was not found so we exit the function successfully.
  exit(EXIT_SUCCESS); 
}

void processCMD(char ** token, int pidHistory[], char ** history)
{
  /* Variable Declarations */ 
  int i = 0, status = 0; // int i is used as an iterator to keep track
  // of certain item counts. status keeps track of the status of any return from
  // execution().
  char cwd[MAX_COMMAND_SIZE] = "./"; // A string that can be used to search the current working
  // directory using execvp
  char bin[MAX_COMMAND_SIZE] = "/bin/"; // A string that can be used to search bin
  char usrBin[MAX_COMMAND_SIZE] = "/usr/bin/"; // A string that can be used to search /usr/bin
  char usrLocal[MAX_COMMAND_SIZE] = "/usr/local/bin/"; 
  // A string that can be used to search /usr/local/bin
  
  /* Concatenate all of our search strings
  with the command issued by the user */
  strcat(cwd, token[0]);
  strcat(usrLocal, token[0]);
  strcat(usrBin, token[0]);
  strcat(bin, token[0]);

  /* In this block of code we analyze user inputs and 
  try to match them to the commands we wish to
  try to execute */
  if(strcmp(token[0], "cd") == 0)
  { 
    chdir(token[1]); // if the user issues the cd command
    // we change directory (ie. cd ..) 
  } 
  else if(strcmp(token[0], "bg") == 0)
  {
  	// if the string is bg we background the process
  	// that was run last. 
    kill(SIGCONT, pidHistory[count-1]);
    //SIGCONT tells the process (pidHistory[count-1])
    // to continue as normal in the background
  }
  else if(strchr(token[0], '!') != NULL)
  {
  	// The '!' signifies the user wishes to exec
  	// a command that has already been run history
    int commandExists = 0;
    // commandExists is a pseudo-boolean value which
    // will be set to 1 if condtions are met in our below 
    // sequence of if statements
    if(((int)token[0][1] <= (count + '0')))
    { 
      // If the first char following the '!' is less
      // than or equal to the ascii value count 
      // (hence int + '0') we my proceed to accept the value
      if((int)token[0][2] >= '0')
      {
      	// this means we have a value greater than or equal to 10
      	// because token[0][2] would be !(1-9)(1-9)
        if(((int)token[0][1] + (int)token[0][2]) > (count + '0' + '0'))
        {
          // if we get in here that means that the the two values following
          // the '!' is greater than count double the ascii value of count
  		  // We do this because count is an int and we need to compare
          // its value to potentially two chars. Adding an in to a char
          // gives the ascii value of that int so this may be used to compare
          // total values. We don't ant anything greater than the given count
          // so we do nothing. commandExists = 0
        }
        else
        {
          // this means that the above conditions were met and the amount
          // of commands currently in history are greater than the amount 
          // specified by the user. 
          commandExists = 1;
        }
      }
      else if(((int)token[0][1] + (int)token[0][2]) == (count + '0'))
      {
      	// if the command is equal to the amount (or current number of)
      	// historical elements then the commandExists
        commandExists = 1;
      }
      else
      {
      	// If we get here we avoided failing any of the preivous tests
      	// and thus the command must exist in history and should be run.
        commandExists = 1;
      }
    }
    if(commandExists == 1)
    {
      // The command exists so run the following:
      /* Declare variables */
      char * test = token[0]+1; // move to the token past '!'
      char * foundInHistory[MAX_NUM_ARGUMENTS]; // a string that will convert our 
      // full history string into a tokenized command execvp can use.
      int test2 = atoi(test); // test2 holds the integer value of everything past
      // the '!'

      /* Here we reuse the tokenization function implemented at the beginning of 
      the program. */

      int token_count = 0; // Keeps track of the amount of tokens in the string                               
                                                           
      // Pointer to point to the token
      // parsed by strsep
      char *arg_ptr;                                         
                                                           
      char *working_str  = strdup(history[test2]);                
      // we are going to move the working_str pointer so
      // keep track of its original value so we can deallocate
      // the correct amount at the end
      char *working_root = working_str;
      // Tokenize the input strings with whitespace used as the delimiter
      while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) && 
                (token_count<MAX_NUM_ARGUMENTS))
      {
        foundInHistory[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
        if(strlen(foundInHistory[token_count]) == 0)
        {
          foundInHistory[token_count] = NULL;
          // if the token inputted is null then break out of the loop
          break;
        }
        
        foundInHistory[token_count][strlen(foundInHistory[token_count])] = '\0';
        // add a null pointer to the end to prevent error from execvp 
        token_count++;
      }
      
      free(working_root); // releasse memory we allocated.
      foundInHistory[token_count] = NULL; 
      // the latest token is nullified to stop undefined behavior.
      processCMD(foundInHistory, pidHistory, history);
      // Pass our token (foundinhistory) our list of pids
      // and the current history at this point recursively
      // back into the current function.
    }
    else
    {
      // This means the command does not exist in history
      // so we alert the user.
      printf("Command not in history\n");
    }
  }
  else if(strcmp(token[0], "history") == 0)
  {
  	// this is our history command if issued by the user
  	// we access our history data structure and print out
  	// every currently existing member. (0-14) for 15 total
    for(i=0;i<MAX_NUM_HISTORY;i++)
    {
      if(strcmp(history[i], "") == 0)
      {
        break;
      }
      else
      {
        printf("%d: ", i);
        printf("%s", history[i]);
      }
    }
  }
  else if(strcmp(token[0], "listpids") == 0)
  {
  	// this command means we need to display to the user
  	// all of the process ids we've collected every
  	// time a command has executed  (0-14) for 15 total
    for(i=0;i<MAX_NUM_HISTORY;i++)
    {
      if(pidHistory[i] == 0)
      {
        break;
      }
      else
      {
        printf("%d: ", i);
        printf("%d\n", pidHistory[i]);
      } 
    }
  }
  else if((strcmp(token[0], "exit") == 0) || (strcmp(token[0], "quit") == 0))
  {
  	// If the user issues an exit or quit command the program should exit
  	// accoriding to the requirements we should exit with code 0 so we 
  	// do so below.
    exit(0);
  }
  else
  {
  	// if none of these predefined commands work then that means we
  	// should search trhough our given folders to find the command
  	// or file specified by the user.
    pidHistory[count] = fork();
    // first we fork the current pid to start a process to handle 
    // an exec.
    if(pidHistory[count] == 0)
    {
      // then if the process is currently in the child we send the 
      // predefined strings and our current user token to the 
      // execution function wehre it will be executed using execvp.	
      execution(cwd, usrLocal, usrBin, bin, token);
    }
    else
    {
      // If we get here we are in the parent and should receive a 
      // status from the execution function.
      waitpid(pidHistory[count], &status, 0); 
    }
  }
  // Everytime go through here we should add to the total amount of 
  // commands (count)
  count++;
}

int main(void)
{
  char * cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
  // the string entered in by the user from the bash shell.
  int i = 0, history_id = 0;
  // allocate the memory addresses we need for 
  // each value of history
  char ** history = malloc(15 * sizeof(char*));

  // allocate the memory addresses we need for 
  // each value of history
  for(i=0;i<15;i++)
  {
    history[i] = (char*)malloc(MAX_COMMAND_SIZE);
  }
  
  // our data structure which will hold the information
  // for our signal handler
  struct sigaction act;

  // Zero out the sigaction struct
  memset (&act, '\0', sizeof(act));
 
  
  // Set the handler to use the function handle_signal() 
  act.sa_handler = &handle_signal;
  
  // Install the handler and check the return value.
  if ((sigaction(SIGINT , &act, NULL) < 0) || (sigaction(SIGTSTP , &act, NULL) < 0))
  {
  	// if we receive an error status print it and then exit. 
    perror ("sigaction: ");
    return 1;
  }

  while(1)
  {
    // Print out the msh prompt
    printf ("msh> ");
    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while(!fgets (cmd_str, MAX_COMMAND_SIZE, stdin));

    // Here we store the full command entered by the user into history
    strncpy(history[history_id], cmd_str, strlen(cmd_str));

    //Parse input
    char * token[MAX_NUM_ARGUMENTS];
    int token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup(cmd_str);                
    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
    // Tokenize the input strings with whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
      if(strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
        // if the token inputted is null then break out of the loop
        break;
      }
        token_count++;
    }

    if(token[0] == NULL)
    {
      // if the token inputted is null then skip further operation
      continue;
    }
   
    // Incremement the history item tracker "history_id". We do this every time
    // to keep track of how many lines have been entered by the user.
    history_id++;
    if(history_id > MAX_NUM_HISTORY)
    {
      history_id = 0;
    }

    // We use count to keep track of the processes of our function
    // everytime a user runs a command we increment this until max 
    // allowed (14) when we get there set it back to zero so we stay
    // there forever
    if(count > MAX_NUM_HISTORY)
    {
      count = 0;
    }

    // tracker keeps track of the i variable because when we find a ';' 
    // we want to reset the count so holdingToken[tracker] will contain the
    // new set of commands after the ';'
    int tracker = 0;

    // holdingToken is a copycat variable that holds the values of token
    // we wish to store if there's more than one command. In the function
    // below we implement the algorithm to take advantage of this var.  
    char * holdingToken[MAX_NUM_ARGUMENTS];
    
    // Here we iterate through the token's arguments
    for(i=0;i<MAX_NUM_ARGUMENTS;i++)
    {
      // as we go through we look for a ';' if there is a semicolon then we break holdingToken
      // and send it in as a command. After that we set the tracker back to zero so we 
      // can track the next possible commands after the ';'.
    	if(strchr(token[i], ';') != NULL)
      {
        holdingToken[i] = NULL; 
        processCMD(holdingToken, pidHistory, history); 
        tracker = 0;
      }
      else if(token[i] != NULL)
      {
        // if the token at any given point is not NULL then we want to store that token
        // in our holding char *
        holdingToken[tracker] = token[i];
        tracker++;
      }
      if(token[i+1] == NULL)
      {
        // Here we anticipate the NULL token if the next token we will read will be NULL
        // then we want to go ahead and send the command to the processCMD function
        holdingToken[tracker] = NULL;
        processCMD(holdingToken, pidHistory, history);
        break;
      }
    }
    
    // free the mem we've allocated
    free(working_root);
  }
  // free the mem we've used.
  free(history);
  return 0;
}
