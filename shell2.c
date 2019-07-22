// Harsh Devprakash Gupta
// hgupta20, 650125174
// Refrences used from the course textbook - 8.5.5
// http://www.cplusplus.com/reference/cstring/strtok/ using strtok to parse the line
// https://brennan.io/2015/01/16/write-a-shell-in-c/ - Used to understand the working of shell and its commands
// https://www.geeksforgeeks.org/making-linux-shell-c/ - Understanding execution of pipe commands
// Labs 4 and 5 to understand dup2command
// http://man7.org/linux/man-pages/man7/signal.7.html - for signals and its handlers
// https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/dup2.html- gor dup2 tutorial
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>    
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
// defining input constraints
#define MAXARGS 128
#define MAXLINE 1000
int errno;

// these functions are taken from the course textbook
// defining these functions prototype inorder to use them in each other
int eval(char** command, int *status);
char** parseline(char *cmdline);
int builtin_command(char **argv);
int run_eval(char** command); // run the eval command and get pid
void signalHandler (int handler); // Lab 5
// textbook - pg 768
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);
int countCommands(char** command);
// using write() for signal handlers
ssize_t sio_puts(char s[]){
    return write(STDOUT_FILENO, s, strlen(s));
}
void sio_error(char s[]){
    sio_puts(s);
    _exit(1);
}
void signalHandler(int handler){
    if (handler == SIGINT){
        sio_puts("Caught: Interrupt\n");
        _exit(0);
    }
    else if (handler == SIGCHLD){
        while (waitpid(-1, NULL, 0) > 0){
            //pid_t pid = wait(status);
            //printf("pid:%d status:%d\n", pid, status);
            sio_puts("Caught: Interrupt\n");
        }
        if (errno != ECHILD){
            sio_error("waitpid error");
        }
        sleep(1);
    }
    else if (handler == SIGTSTP){
        sio_puts("Caught: SIGTSTP.\n");
        sleep(1);
        
    }
    else if (handler == SIGTSTP){
        //pid_t pid = getpid();
        sio_puts("Caught: SIGTSTP.\n");
        sleep(1);
        //pause();
        //kill(pid, SIGTSTP);
        //kill(getpid(), SIGTSTP);
        
    }
    else if (handler == SIGCONT){
        pid_t pid = getpid();
        sio_puts("Caught: SIGCONT.\n");
        kill(pid, SIGCONT);
        
    }
    else if(handler == SIGTERM){
        id_t pid = getpgid(getpid());
        sio_puts("Caught: Terminated.\n");
        kill(pid, SIGTERM);
        _exit(0);
    }
    fflush(stdout);
}
// function to handle pipe commands
void pipeCommand(char** command1, char** command2){
    // file descriptors
    int filedes[2];
    pipe(filedes);
    if(fork() == 0){ // child process
        // using dup2 command
        dup2(filedes[0], STDIN_FILENO);
        close(filedes[1]);
        close(filedes[0]);
    
        if (fork() == 0){
            dup2(filedes[1], STDOUT_FILENO);
            close(filedes[0]);
            close(filedes[1]);
            execvp(command1[0], command1);
        }
    
    wait(NULL);
    execvp(command2[0], command2);
    }
    // close all the files
    close(filedes[1]);
    close(filedes[0]);
    wait(NULL);
}
int eval(char** command, int *status)
{ // evaluates the command and parses using parse line
  // there were pid errors and hence I ran it in different functions
    pid_t pid;
    int background = 0;
    if (command == NULL) // for empty input
    {
        return 0;
    }
    if (builtin_command(command))
    {   
        int i = 0;
        int redir1 =0;
        int redir2 = 0;
        int redir3 = 0;
        int pipe = 0;
        char input[128];
        char output[128];
        char output1[128];
        pid = fork();
        if(pid == 0){ // for the child process, code taken from lab 2
            for(int i = 0; command[i] != NULL; i++){ // append
                if(strcmp(command[i], ">>") == 0){
                    command[i] = NULL;
                    strcpy(output1, command[i+1]);
                    redir3=1;
                }
                else if(strcmp(command[i], "<") == 0){ // read
                    command[i] = NULL;
                    strcpy(input, command[i+1]);
                    redir1=1;
                }
                else if(strcmp(command[i], ">") == 0){ // write
                    command[i] = NULL;
                    strcpy(output, command[i+1]);
                    redir2=1;
                }
                else if(strcmp(command[i], "|") == 0){
                    command[i] = NULL;
                    pipe = 1;
                    
                }
                /*else if(strcmp(command[i], "&") == 0){
                    background = 1;
                }*/
                
            }
            if(redir1 == 1){ // read
                int filedes;
                if ((filedes = open(input, O_CREAT | O_APPEND | O_WRONLY, 0)) < 0){
                    perror("Error in opening the file");
                    exit(0);
                }
                dup2(filedes, 0);
                close(filedes);
                
            }
            if(redir2 == 1){ // write
                int filedes;
                if ((filedes = creat(output, 0644)) < 0){
                    perror("Error in opening the file");
                    exit(0);
                }
                dup2(filedes, STDOUT_FILENO);
                close(filedes);
                
            }
            if(redir3 == 1){
                int filedes;
                if ((filedes = open(output1, O_CREAT | O_RDWR | O_APPEND)) < 0){
                    perror("Error in opening the file");
                    exit(0);
                }
                dup2(filedes, 1);
                close(filedes);
                
            }
            if (pipe == 1){
                pipeCommand(command, command);
                return pid;
            }
            if(execvp(command[0],command) == -1)
            {
                printf("Command not Found %s\n", command[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if(pid < 0)
        {
            // make sure there is no forking error
            printf("Forking error\n");
        }
        else
        { // make the parent wait for the child
            /*if(background == 0){
                while(wait(status) > 0){
                    return pid;
                }
            }
            else{
                return pid;
            }*/
            while(wait(status) > 0){
                    return pid;
            }
            /*if (WIFEXITED(status)){
                printf("Hello");
            }
            else if(WIFSIGNALED(status)){
                printf("GET LOST");
            }*/
        }
        return -1;
    }
    
    
}
// function to run the eval command insind the shell
int run_eval(char** argv)
{
    int status;
    pid_t pid;
    
    pid = eval(argv,&(status));
    if(status == 256) { // accounting for a special error which occurs when parent process is incompletely returned
        status = 1;
    }
    printf("pid:%d status:%d\n", pid, status);
    free(argv);
    return 1;
}
// function to parse the command line and build the argv array
char**  parseline(char *cmdline)
{ 
    // to iterate through the list
    int i=0;
    // initialize the array to hold the tokens and allocate memory
    char** argv = malloc(64 * sizeof(char*));
    // for tokens
    char * argc = NULL;
	//Get first argument
    argc = strtok(cmdline," \t\r\a\n"); // list of delimiters
    while(argc != NULL)
    {
        argv[i++] = argc; // add the command in the array argv
        argc = strtok(NULL," \t\r\a\n"); // get the next argument
    
    }
    argv[i] = NULL; // make the last command null so that shell known wehn to exit
    return argv; // return the argv array
}
int builtin_command(char **argv)
{ // for built in commands
    if (strcmp(argv[0], "exit") == 0) // exit command
    {
        exit(0);
    }
    if (strcmp(argv[0], "cd") == 0) // cd command
    {   
        if (argv[1] == NULL)
        {
            printf("Expected argument to change into\n");
            return 0;
        }
        else
        {
            if(chdir(argv[1]) == -1)
	        {
		        printf("No such directory found\n");
                return 0;
	        }
        }
        
    }
    if (strcmp(argv[0], "pwd") == 0) // pwd
    {
        char path[MAXARGS];
        if(getcwd(path,MAXARGS) == NULL)
        {
            printf("Error getting current working directory\n");
            return 0;
        }
        printf("%s\n",path);
        return 0;
    }
         
    return 1;
}
/*void unix_error(char *msg)\
{ // function to give error details for forking
    printf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}
pid_t Fork(void)
{// adding the error check in the function to call fork
    
    pid_t pid;
    pid = fork();
    if (pid < 0){
        unix_error("Fork Error");
        return pid;
    }
}*/
int main(){
    char* command= malloc(sizeof(char)* MAXARGS);
    char** argv;
    int redirection = 0;
    if(signal(SIGINT, signalHandler) == SIG_ERR){
        printf("signal error");
    }
    if(signal(SIGTSTP, signalHandler) == SIG_ERR){
        printf("signal error");
    }
    if(signal(SIGCHLD, signalHandler) == SIG_ERR){
        printf("signal error");
    }
    if(signal(SIGCONT, signalHandler) == SIG_ERR){
        printf("signal error");
    }
    if(signal(SIGTERM, signalHandler) == SIG_ERR){
        printf("signal error");
    }
       
    while(1)
    {
        
        printf("CS361 >");
        fgets(command, MAXARGS, stdin);
        if (feof(stdin))
        {   
            exit(0);
        }
        argv = parseline(command);
        run_eval(argv);
    }
    
}