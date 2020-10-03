/*
 *
 * Name: Sahaj Amatya 
 * UTA ID: 1001661825 
 *
 */

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017, 2020 Trevor Bakker
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

#define WHITESPACE " \t\n" // We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.   \
                           // In this case  white space                        \
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports TEN arguments
#define MAX_NUM_HISTORY 15
#define MAX_NUM_BUILTINS 7
#define MAX_NUM_PIDS 15

pid_t PIDQueue[MAX_NUM_PIDS];
int front = -1;
int rear = -1;
int capacity = -1;

void enqueuePID(pid_t data)
{
  if (capacity < MAX_NUM_PIDS)
  {
    if (capacity < 0)
    {
      PIDQueue[0] = data;
      front = rear = 0;
      capacity = 1;
    }
    else if (rear == MAX_NUM_PIDS - 1)
    {
      PIDQueue[0] = data;
      rear = 0;
      capacity++;
    }
  }
  else
  {
    return;
  }
}

void dequeuePID(){
  if(capacity<0){
    return;
  }
  else {
    capacity--;
    front++;
  }
}

void showpids(){
  int i;
  for(i=front;i<=rear;i++){
    printf("%d\n", PIDQueue[i]);
  }
}

int checkCMD(char *cmd)
{
  char *dict[MAX_NUM_BUILTINS] = {"quit", "q", "exit", "showpids", "history", "cd", "\n"};
  int i;
  int result = 0;
  for (i = 0; i < MAX_NUM_BUILTINS; i++)
  {
    if (strcmp(cmd, dict[i]) == 0)
    {
      result = 1;
      break;
    }
  }
  return result;
}

int main()
{

  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

  while (1)
  {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
    ;

      /* Parse input */
      char *token[MAX_NUM_ARGUMENTS];
    char history[MAX_NUM_HISTORY][MAX_NUM_ARGUMENTS];
    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    free(working_root);
    char *arguments[10];
    int token_index;
    arguments[token_count - 1] = NULL;
    pid_t pid = fork();
    int historyIndex = 0;
    if (token[0] == NULL)
    {
      token[0] = "\n";
    }
    int isBuiltinCMD = checkCMD(token[0]);
    if (isBuiltinCMD)
    {
      int status;
      wait(&status);
      if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0 || strcmp(token[0], "q") == 0)
      {
        kill(0, SIGINT);
        //exit(0);
      }
      if (strcmp(token[0], "cd") == 0)
      {
        if (chdir(token[1]) == -1)
        {
          printf("%s: No such file or directory.\n", token[1]);
        }
      }
      if (strcmp(token[0], "showpids") == 0)
      {
        printf("This will showpids.\n");
	showpids();
      }
      if (strcmp(token[0], "history") == 0)
      {
        printf("This will show history.\n");
      }
    }
    else
    {
      if (pid == 0)
      {
	enqueuePID(getpid());
        for (token_index = 0; token_index < token_count - 1; token_index++)
        {
          arguments[token_index] = (char *)malloc(strlen(token[token_index]));
          strncpy(arguments[token_index], token[token_index], strlen(token[token_index]));
          strcat(history[historyIndex], arguments[token_index]);
          historyIndex++;
        }

        if (execvp(arguments[0], &arguments[0]) == -1)
        {
          printf("%s: Command not found.\n\n", token[0]);
        }
      }
      else if (pid > 0)
      {
        int status;
        wait(&status);
      }
    }
  }
  return 0;
}
