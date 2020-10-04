/*
 *
 * Name: Sahaj Amatya
 * UTA ID: 1001661825
 *
 *          ___           ___           ___     
 *         /__/\         /  /\         /__/\
 *        |  |::\       /  /:/_        \  \:\
 *        |  |:|:\     /  /:/ /\        \__\:\
 *      __|__|:|\:\   /  /:/ /::\   ___ /  /::\
 *     /__/::::| \:\ /__/:/ /:/\:\ /__/\  /:/\:\
 *     \  \:\~~\__\/ \  \:\/:/~/:/ \  \:\/:/__\/
 *      \  \:\        \  \::/ /:/   \  \::/     
 *       \  \:\        \__\/ /:/     \  \:\
 *        \  \:\         /__/:/       \  \:\
 *         \__\/         \__\/         \__\/    
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
#define MAX_NUM_HISTORY 15   // MSH history will only store upto 15 commands
#define MAX_NUM_BUILTINS 7   // Built-in cmds, eg., "quit", "q", "cd", etc.
#define MAX_NUM_PIDS 15      // MSH will store the PIDs upto the last 15 processes

/*
 * This is the Queue where the PIDs will be stored.
 */
pid_t PIDQueue[MAX_NUM_PIDS];
int front = -1;
int rear = -1;

/*
 * This is the Queue where the command history will be stored.
 */
char *historyRecords[MAX_NUM_HISTORY];
int frontHist = -1;
int rearHist = -1;

/*
 * Function to dequeue PIDs, removes the head and shifts all entries back by
 * an index.
 */
void dequeuePID()
{
  int j;
  for (j = 0; j < MAX_NUM_PIDS - 1; j++)
  {
    PIDQueue[j] = PIDQueue[j + 1];
  }
  rear--;
}

/*
 * Function to enqueue PIDs, queue will be dequeued when number of entries surpasses
 * MAX_NUM_PIDS.
 */
void enqueuePID(pid_t data)
{
  if (rear == MAX_NUM_PIDS - 1)
  {
    dequeuePID();
  }
  if (front == -1)
  {
    front = 0;
  }
  rear++;
  PIDQueue[rear] = data;
}

/*
 * Function to print all PIDs of processes created by the program.
 */
void showpids()
{
  int i;
  printf("PID\n------\n");
  for (i = front; i <= rear; i++)
  {
    printf("%d\n", PIDQueue[i]);
  }
}

/*
 * Function to dequeue command history, removes the head and shifts all entries back by
 * an index.
 */
void dequeueHistory()
{
  int j;
  for (j = 0; j < MAX_NUM_HISTORY - 1; j++)
  {
    historyRecords[j] = historyRecords[j + 1];
  }
  rearHist--;
}

/*
 * Function to enqueue command to history, queue will be dequeued when number of entries surpasses
 * MAX_NUM_HISTORY.
 */
void enqueueHistory(char *cmd)
{
  if (rearHist == MAX_NUM_HISTORY - 1)
  {
    dequeueHistory();
  }
  if (frontHist == -1)
  {
    frontHist = 0;
  }
  rearHist++;
  historyRecords[rearHist] = (char *)malloc(strlen(cmd));
  strcpy(historyRecords[rearHist], cmd);
}

/*
 * Function to print all commands in historyRecords[]
 */
void showHistory()
{
  int i;
  for (i = frontHist; i <= rearHist; i++)
  {
    printf("[%d]: %s", i, historyRecords[i]);
  }
}

/*
 * Function to check if the command entered is a builtin command.
 * Builtin commands are listed in char* dict[].
 * Returns 1 if command is present in dict, 0 if not 
 */
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
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));

    if (strcmp(cmd_str, "\n") != 0)
    {
      enqueueHistory(cmd_str); // enqueue entered cmd to historyRecords[],
    }                          // it will not store when user hits ENTER.
    /* 
     * The following container pulls commands from historyRecords[].
     * It extracts the "!" off the command and stores the cmd index. 
     * The command in historyRecords[cmdIndex] is then stored in cmd_str to be
     * executed later on by the program.
     */
    if (cmd_str[0] == '!')
    {
      int cmdIndex;
      sscanf(cmd_str, "%*c%d", &cmdIndex);
      if (cmdIndex >= 0 && cmdIndex <= rearHist)
      {
        strcpy(cmd_str, historyRecords[cmdIndex]);
      }
      else
      {
        continue;
      }
    }
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
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
    char *arguments[MAX_NUM_ARGUMENTS];
    int token_index;
    arguments[token_count - 1] = NULL;
    if (token[0] == NULL) //to counter segfault when user hits return
    {
      token[0] = "\n";
    }
    int isBuiltinCMD = checkCMD(token[0]);
    if (isBuiltinCMD)
    {
      if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0 
		      || strcmp(token[0], "q") == 0)
      {
        exit(0);
      }
      else if (strcmp(token[0], "cd") == 0)
      {
        if (chdir(token[1]) == -1)
        {
          printf("%s: No such file or directory.\n", token[1]);
        }
      }
      else if (strcmp(token[0], "showpids") == 0)
      {
        showpids();
      }
      else if (strcmp(token[0], "history") == 0)
      {
        showHistory();
      }
    }
    else
    {
      pid_t pid = fork();
      if (pid == 0)
      { //this for loop prepares arguments[] for the execvp syscall,
        //inspired by 
	//https://github.com/CSE3320/Shell-Assignment/blob/master/Useful-Examples/execvp.c
        for (token_index = 0; token_index < token_count - 1; token_index++)
        {
          arguments[token_index] = (char *)malloc(strlen(token[token_index]));
          strncpy(arguments[token_index], token[token_index], strlen(token[token_index]));
        }
        if (execvp(arguments[0], &arguments[0]) == -1)
        {
          printf("%s: Command not found.\n\n", token[0]);
	  exit(0);
	}
      }
      else if (pid > 0)
      {
        int status;
        wait(&status);
        enqueuePID(pid);
      }
    }
  }
  return 0;
}
