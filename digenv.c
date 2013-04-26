/*
    ID2206 Operativsystem

    Christoffer Gunning     cgunning@kth.se
    Daniel Forslund         dforsl@kth.se

    This program works as printenv | grep params | sort | pager
    where pager is the pager specified in the PAGER environment
    variable. If no pager is specified less is used and more is
    used if less is unavailable. If no params are specified the
    grep part will be skipped and the program will work like
    printenv | sort | pager.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

/* CONSTANTS */
#define READ        0
#define WRITE       1

/* Two-dimensional array to store the pipes, needs to be changed if more filters is to be implemented*/
int pipes[3][2];

void prepare_read(int pipe);
void prepare_write(int pipe);

/*
    Creates child processes for each filter. The results from each filter is sent
    through a pipe to the next filter. Lastly the result is presented with a pager.
*/
int main(int argc, char **argv, char **envp)
{
    char *filters[] = {"printenv", "grep", "sort"}; /* a list containing the filters to be executed */
    char *pager;
    int i, n = 3;

    for(i = 0; i < n; i++) {    
        pipe(pipes[i]); /* Create a pipe at specified index in the pipe array */

        pid_t pid = fork(); /* Create new process */

        /* handle if fork() failed */
        if (pid < 0){
            perror("error");
            exit(1);
        }

        if (pid == 0) { /* Code for child processes*/
            prepare_write(i); /* set standard output to pipe */

            /* Special case for grep since grep takes arguments */
            if(strcmp(filters[i], "grep") == 0) {
                if(argc > 1) { /* Any arguments? */
                    execvp("grep", argv); /* execute grep with arguments */

                    /* if this is executed, execvp has failed */
                    perror("error");
                    _exit(EXIT_FAILURE);
                }
            } else {
                /* Execute current filter, print error message and exit on error */
                execlp(filters[i],filters[i],NULL);

                /* If this is executed execlp has failes */
                perror("error");
                _exit(EXIT_FAILURE);
            }
        } else { /* Code for parent process */
            prepare_read(i); /* set standard input pipe */
        }
    }

    pager = getenv("PAGER"); /* Get the pager in the environment variable PAGER */
    if (!pager) {
        pager = "less"; /* set pager to less if PAGER is not specified */
    }
    
    execlp(pager,pager,NULL);   /* Execute pager in the environment variable PAGER or less if not specified */

    execlp("more","more",NULL); /* Execute more as pager if previous failed*/

    /* Couldn't find a pager that worked, exit */
    perror("Invalid pager"); 
    _exit(EXIT_FAILURE);
    return EXIT_FAILURE;
}

/*
    Prepare the specified pipe to be written to

    int index - index of the pipe int pipes[]
*/
void prepare_write(int pipe) {
    /* Close read end of pipe - only write enabled */
    if(close(pipes[pipe][READ]) < 0) {
        perror("1error");
       _exit(EXIT_FAILURE);
    }
    /* Use pipe instead of standand output */
    if(dup2(pipes[pipe][WRITE], 1) < 0) {
        perror("error");
       _exit(EXIT_FAILURE);
    }
    /* Close write end of pipe */
    if(close(pipes[pipe][WRITE]) < 0) {
        perror("1error");
       _exit(EXIT_FAILURE);
    }
}

/*
    Prepare the specified pipe to be read from

    int index - index of the pipe int pipes[]
*/
void prepare_read(int pipe) {
    /* Close write end of pipe - only read enabled */
    if(close(pipes[pipe][WRITE]) < 0) {
        perror("error");
       _exit(EXIT_FAILURE);
    }
    /* Use pipe instead of standard input */
    if(dup2(pipes[pipe][READ], 0) < 0) {
        perror("error");
       _exit(EXIT_FAILURE);
    }
    /* Close read end of pipe */
    if(close(pipes[pipe][READ]) < 0) {
        perror("1error");
       _exit(EXIT_FAILURE);
    }
}