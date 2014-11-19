/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */


#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"

#include <unistd.h> 	/* for fork() */
#include <sys/types.h> 	/* for wait() */
#include <sys/wait.h> 	/* for wait() */

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
int runCommand(int n,Pgm *p);
/* When non-zero, this global means the user is done using this program. */
int done = 0;

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  int n;

  while (!done) {

    char *line;
    line = readline("> ");

    if (!line) {
      /* Encountered EOF at top level */
      done = 1;
    }
    else {
      /*
       * Remove leading and trailing whitespace from the line
       * Then, if there is anything left, add it to the history list
       * and execute it.
       */
      stripwhite(line);

      if(*line) {
        add_history(line);
        /* execute it */
        n = parse(line, &cmd);
        PrintCommand(n, &cmd);
      }
    }
    
    /* Own stuff testing */
    

    /*
     * Traverse through linked list, and fork process child per program
     * Sets up pipeline between child processes.
     */
    runCommand(0,cmd.pgm);


    /* Own stuff testing - END*/
    if(line) {
      free(line);
    }
  }
  return 0;
}

/*
 * Traverse through linked list, and fork process child per program
 * Sets up pipeline between child processes.
 * Recursive function
 */
int runCommand(int n,Pgm *p){
	pid_t pid;
	if (p == NULL) {
		return -1;
	}
	else {
		/* Create pipe */
		int pipefd[2];

		if (n>0){
			if (pipe(pipefd) == 0) {
				fprintf(stderr,"Parent %i: Created pipe\n",n);
			}
		}

		/* fork a child process */
		pid = fork();
		if (pid < 0){
			fprintf(stderr, "Parent %i: Fork failed",n);
			return 1;
		}

		else if (pid >0) {
		/* Parent process */

			/*** SET UP READING END ***/
			/* Skip pipe for first process */
			if (n>0) {

				/* close writing end */
				if (close(pipefd[1]) != 0){
					fprintf(stderr,"Closing pipefd[1] failed\n");
				}
				/* Connect reading end to STDIN */
				dup2(pipefd[0],0);
				/* close reading end */
				if (close(pipefd[0]) != 0){
					fprintf(stderr,"Closing pipefd[1] failed\n");
				}
			}


			wait(NULL);
			fprintf(stderr,"Parent %i: Finished waiting\n",n);
		}

		else {
		/* child process */
			fprintf(stderr,"Hej from Child%i\n",n);

			/*** SET UP WRITING END ***/
			/* Skip pipe for first process */
			if (n>0) {
				/* close reading end */
				if (close(pipefd[0]) != 0){
					fprintf(stderr,"Closing pipefd[1] failed\n");
				}
				/* Connect writing end to STDOUT */
				dup2(pipefd[1],1);

				/* close writing end */
				if (close(pipefd[1]) != 0){
					fprintf(stderr,"Closing pipefd[1] failed\n");
				}
			}
			/* Recursive call to runCommand, itself */
			runCommand(n+1,p->next);
			/* Execute program binary */
			execvp(p->pgmlist[0],p->pgmlist);
		}
		return 0;
	}
}




/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (whitespace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && whitespace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}
