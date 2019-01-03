#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "commando.h"


cmd_t *cmd_new(char *argv[]){
  cmd_t *cmd;
  cmd = malloc(sizeof(cmd_t)); //allocating memory chunck for cmd
  strcpy(cmd->name, argv[0]); //put program name(such as ls, gcc) to cmd->name
  for(int i = 0; i < ARG_MAX; i++){ //put rest of the command arguments
    if(argv[i] != NULL){
      cmd->argv[i] = strdup(argv[i]);
    }
    else{
      cmd->argv[i] = NULL; //make sure last index is null
      break;
    }
  }
  //setting fields to default values
  cmd->finished = 0;
  snprintf(cmd->str_status,STATUS_LEN+1,"%s","INIT");
  cmd->pid = -1;
  cmd->out_pipe[0] = -1;
  cmd->out_pipe[1] = -1;
  cmd->status = -1;
  cmd->output = NULL;
  cmd->output_size = -1;

  return cmd;
}

void cmd_free(cmd_t *cmd){
  for(int i = 0; i < ARG_MAX; i++){ //free all argument arrays
    if(cmd->argv[i] != NULL){//if it is not null there is something in array so
      free(cmd->argv[i]); // free it
    }
    else{
      break;
    }
  }
  if(cmd->output != NULL){ //free output buf if not null
    free(cmd->output);
  }
  free(cmd); //finally free cmd itself
}

void cmd_start(cmd_t *cmd){
  snprintf(cmd->str_status,STATUS_LEN+1,"%s","RUN");//set str_status to RUN
  int pipe_result = pipe(cmd->out_pipe); //create pipe before forking
  if(pipe_result != 0) { //error checking for creating pipe
    perror("Failed to create pipe");
    exit(1);
  }

  pid_t child = fork(); // fork - child created
  if(child == 0){ //if the process is child
    dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO); //set stdout to write end of pipe
    close(cmd->out_pipe[PREAD]); //close read end because it is not needed
    execvp(cmd->name, cmd->argv); //execute the command and arguments
    perror("errno indicates"); //error checking
    printf("Couldn't run '%s' because of error\n",cmd->name);
    exit(1);
  }
  else{
    cmd->pid = child; //assign child pid to cmd->pid of parent for further management
    close(cmd->out_pipe[PWRITE]); //close write end of pipe because it is not needed
  }
}

void cmd_update_state(cmd_t *cmd, int block){
  pid_t pid;
  int status;
  if(cmd->pid == -1){ //if it is child process just ignore
    return;
  }
  if(cmd->finished == 1){ //if fininshed == 1 which means state already changed, ignore
    return;
  }
  else{
    pid = waitpid(cmd->pid, &status, block); //parent wait for child to execute the program
    if(pid == cmd->pid){
      if(WIFEXITED(status)){
        cmd->status = WEXITSTATUS(status);
        snprintf(cmd->str_status,STATUS_LEN+1,"EXIT(%d)",cmd->status);
        cmd->finished = 1;
        cmd_fetch_output(cmd);
        printf("@!!! %s[#%d]: %s\n",cmd->name, cmd->pid, cmd->str_status);
      }
    }
  }
}

char *read_all(int fd, int *nread){//have a big problem
  int size = 1024; //initial size 1024 bytes
  int bytesread;
  char *buffer = (char *)malloc(size); //allocate memory for buffer
  bytesread = read(fd, buffer, size);//read file into buffer
  while(bytesread >= size){ //if file has more bytes to be read
    size *= 2; //double the size
    buffer = realloc(buffer, size); //realloc the buffer with doubled size
    bytesread += read(fd, buffer+bytesread, size-bytesread); //continue at the previous offset
  }
  buffer[bytesread] = '\0'; //buffer need to be null-terminated
  *nread = bytesread;

  return buffer;

}

void cmd_fetch_output(cmd_t *cmd){
  int nread = 0;
  if(cmd->finished == 0){
    printf("%s[#%d] not finished yet\n",cmd->name, cmd->pid);
    return;
  }
  else{ //if finished
    cmd->output = read_all(cmd->out_pipe[PREAD], &nread); //take output from pipe and put it in cmd->output
    cmd->output_size = nread; //set output size to # of bytes read
    close(cmd->out_pipe[PREAD]); //close pipe because it is not needed anymore
  }

}

void cmd_print_output(cmd_t *cmd){
  if(cmd->output != NULL){ //if it is not null print output
    void *vbuf = cmd->output;
    write(STDOUT_FILENO, (char *)vbuf, cmd->output_size); //print output using write()
  }
  else { //no output yet
    printf("%s[#%d] has no output yet\n",cmd->name, cmd->pid);
  }
}
