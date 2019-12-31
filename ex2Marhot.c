// issa rantisi
// 208633255

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_LENGTH 512
#define CD_COMMAND "cd"
#define BACKGROUND "&"
#define TILDA "~"
#define HOME "HOME"
#define PROMPT "> "
#define DOUBLE_POINT ".."
#define MINUS "-"
#define EXIT_COMMAND "exit"
#define JOBS "jobs"
#define SPACE ' '

char oldPath[INPUT_LENGTH];
char newPath[INPUT_LENGTH];
/*
 * struct of jobs that contains pid which is int, and the command represent by Array of chars.
 */
typedef struct jobs {
    pid_t pid;
    char jobCommand[INPUT_LENGTH];
} Job_Array;

/**
 * function that check if the line contains "" that decorate that the folder (check also it's not contain space)
 * @param lineFromUser the command that the user want.
 * @return nothing
 */
void checkForCd(char *lineFromUser) {
    int i;
    for (i = 0; i < strlen(lineFromUser); i++) {
        if (lineFromUser[i] == '"') {
            int k = i;
            int j;
            for (j = i; j < strlen(lineFromUser); j++) {
                if (lineFromUser[j] == SPACE) {
                    lineFromUser[j] = '+';
                } else if (lineFromUser[j] == '\"') {
                    lineFromUser[j] = SPACE;
                }
            }
            lineFromUser[k] = SPACE;
        }
    }
}

/**
 * used the split the command to the cells of the arrays that used to send to the execv
 * @param lineFromUser the input from the user.
 * @param execute array of pointer to chars that we will split and put in
 * @param last used for the background sign '&', which later we will delete.
 */
void splitArgs(char *lineFromUser, char **execute, int *last) {
    char *buffer;
    size_t position = 0;
    buffer = strtok(lineFromUser, " , \n");
    execute[position++] = buffer;
    while (buffer != NULL) {
        buffer = strtok(NULL, " ,\n");
        execute[position++] = buffer;
    }
    *last = (int) position;
}

/**
 * cd command menu.
 * @param execute  array of the pointers of the command.
 * @return
 */
void runCdCommand(char **execute) {
    // print the pid of the process.
    printf("%d\n", (int) getpid());
    // case of special cd command we dont want to save it on the old path.
    if (strcmp(oldPath, MINUS) == 0 && strcmp(oldPath, TILDA) == 0 & strcmp(oldPath, DOUBLE_POINT) == 0) {
        strcpy(oldPath, execute[1]);
    }
    // case of TILDA, we want to go HOME.
    if (execute[1] == NULL || strcmp(execute[1], TILDA) == 0) {
        chdir(getenv(HOME));
        strcpy(newPath, getcwd(oldPath, sizeof(oldPath)));
        return;
    } else if (strcmp(execute[1], MINUS) == 0) {
        char temp[INPUT_LENGTH];
        strcpy(temp,getcwd(temp, sizeof(oldPath)));
        chdir(newPath);
        strcpy(newPath, temp);
        return;
    }
    char *salim = malloc(strlen(*++execute) + 1);
    strcpy(salim, *execute);
    int i = 0;
    while (i < strlen(*execute)) {
        if ((*execute)[i] == '+') {
            (*execute)[i] = SPACE;
        }
        i++;
    }
    strcpy(salim, *execute);
    execute++;
    while (*execute != NULL) {
        int i = 0;
        while (i < strlen(*execute)) {
            if ((*execute)[i] == '+') {
                (*execute)[i] = SPACE;
            }
            i++;
        }
        salim = realloc(salim, strlen(salim) + strlen(*execute) + 1);
        strcat(salim, *execute);
        execute++;
    }
    strcpy(newPath, getcwd(oldPath, sizeof(oldPath)));
    chdir(salim);
    free(salim);
}

/**
 * system call for the shell
 * @param lineFromUser  the input from the user
 * @param execute the array of command
 * @param numberOfJobs numbers of the jobs in the job array.
 * @param jobs jobs array.
 * @param background flag that say if it's contain '&' or not.
 * @return
 */
void systemCall(char *lineFromUser, char **execute, int *numberOfJobs, Job_Array *jobs, int background) {
    int place;
    lineFromUser = strtok(lineFromUser, BACKGROUND);
    pid_t pid = fork();
    if (pid == 0) {
        execvp(*execute, execute);
        fprintf(stderr, "Error in system call\n");
        exit(0);
    } else if (pid > 0) {
        printf("%d\n", pid);
        if (background) {
            strcpy(jobs[*numberOfJobs].jobCommand, lineFromUser);
            jobs[*numberOfJobs].pid = pid;
            (*numberOfJobs)++;
        } else {
            waitpid(pid, &place, 0);
        }
    }
}

/**
 * used to delete the dead process on the job array.
 * @param jobs array of the jobs
 * @param numbersOfJobs number of the jobs.
 * @param i index of the dead process in the array that came from "removeProcess" function.
 */
void deleteFromArray(Job_Array *jobs, int *numbersOfJobs, int i) {
    int j;
    for (j = i; j < *numbersOfJobs; j++) {
        jobs[j].pid = jobs[j + 1].pid;
        strcpy(jobs[j].jobCommand, jobs[j + 1].jobCommand);
    }
    *numbersOfJobs -= 1;
}

/**
 * used to iterate over all the jobs of the array and check if the jobs dead or not using waitpid
 * @param jobs
 * @param numbersOfJobs
 */
void removeProcess(Job_Array *jobs, int *numbersOfJobs) {
    int size = 0, i;
    for (i = 0; i < *numbersOfJobs; i++) {
        if (waitpid(jobs[i].pid, &size, WNOHANG)) {
            deleteFromArray(jobs, numbersOfJobs, i);
        }
    }
}

/**
 * print all the jobs that is in the job arrays which run on the background.
 * @param jobs the array of the jobs.
 * @param numbersOfJobs numbers of the jobs on the array.
 */
void printJobs(Job_Array *jobs, int *numbersOfJobs) {
    // remove the dead jobs
    removeProcess(jobs, numbersOfJobs);
    int i;
    // print the pid and the command of the cell of the jobs array.
    for (i = 0; i < *numbersOfJobs; i++) {
        printf("%d %s\n", jobs[i].pid, jobs[i].jobCommand);
    }
}

void commandMenu(char *lineFromUser, char **execute, int *numbersOfJobs, Job_Array *jobs, int background) {
    if (execute[0] == NULL) {
        return;
    } else if (strcmp(execute[0], CD_COMMAND) == 0) {
        runCdCommand(execute);
    } else if (strcmp(execute[0], EXIT_COMMAND) == 0) {
        printf("%d\n", (int) getpid());
        exit(0);
    } else if (strcmp(execute[0], JOBS) == 0) {
        printJobs(jobs, numbersOfJobs);
    } else {
        systemCall(lineFromUser, execute, numbersOfJobs, jobs, background);
    }
}

/**
 * check if the sign '&' contain on the line the user entered
 * @param arrayOfCommand  Arrays of pointer.
 * @return Yes if it's contain, no if it's not contain
 */
int ifBackground(char **arrayOfCommand) {
    // loop over the arrays.
    while (*arrayOfCommand != NULL) {
        // check if the contain the sign.
        if (strcmp(*arrayOfCommand, BACKGROUND) == 0) {
            return 1;
        }
        // move on to the next cell.
        arrayOfCommand++;
    }
    //case it's does not contain.
    return 0;
}

void run() {
    Job_Array jobs[INPUT_LENGTH];
    char *arrayOfCommand[INPUT_LENGTH];
    char input[INPUT_LENGTH];
    int numbersOfJobs = 0, background = 0, last = 0;
    char sender[INPUT_LENGTH];
    while (1) {
        removeProcess(jobs, &numbersOfJobs);
        printf(PROMPT);
        fgets(input, INPUT_LENGTH, stdin);
        strcpy(sender, input);
        checkForCd(sender);
        splitArgs(sender, arrayOfCommand, &last);
        background = ifBackground(arrayOfCommand);
        if (background) {
            arrayOfCommand[last - 2] = NULL;
        }
        commandMenu(input, arrayOfCommand, &numbersOfJobs, jobs, background);
    }
}

int main() {
    run();
}