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

void print_output(cmdctl_t *ctl, int n);
void print_echo(char *input, int echo);

int main(int argc, char *argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); //turn off output buffering

  int echo = 0; //variable to detect if command arg is --echo
  char cmd_input[MAX_LINE]; //char buffer for user input
  char *cmd_argv[ARG_MAX+1]; //break user input into tokens and save in this argv
  int cmd_argc; //argument count
  cmdctl_t cmdctl = {}; //initializing cmdctl
  cmdctl_t *ctl = &cmdctl;

  if(argc >= 2){ //if there is commandline arguments other than program name
    if(!strcmp(argv[1],"--echo")){ //check if it is --echo for echoing
      echo = 1; //set var echo to be 1 (which is true)
    }
  }
  else if(getenv("COMMANDO_ECHO") != NULL){//if env var COMMANDO_ECHO is set to anything
    echo = 1; //echo is true
  }

  //INPUT LOOP
  while(1){
    printf("@> "); //printing prompt
    //getting user input; if it's NULL, exit program with printing END OF INPUT
    if(fgets(cmd_input, MAX_LINE, stdin) == NULL){
      if(ctl != NULL){ //check if there is any ctl existing
          cmdctl_freeall(ctl); //free them before exit
        }
      printf("\nEnd of input\n");
      exit(1);
    }

    if(cmd_input[0] == '\n'){
      print_echo(cmd_input, echo);
      goto END;
    }  //if user hits enter jump to end of loop

    print_echo(cmd_input, echo);//echo the input before parsing into tokens!

    parse_into_tokens(cmd_input, cmd_argv, &cmd_argc); //break input into argv

    if(!strcmp(cmd_argv[0], "help")){ //help
      printf("COMMANDO COMMANDS\n");
      printf("help               : show this message\n");
      printf("exit               : exit the program\n");
      printf("list               : list all jobs that have been started giving information on each\n");
      printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
      printf("output-for int     : print the output for given job number\n");
      printf("output-all         : print output for all jobs\n");
      printf("wait-for int       : wait until the given job number finishes\n");
      printf("wait-all           : wait for all jobs to finish\n");
      printf("command arg1 ...   : non-built-in is run as a job\n");
    }
    else if(!strcmp(cmd_argv[0], "exit")){ //free mem and exit
      cmdctl_freeall(ctl);
      return 1;
    }
    else if(!strcmp(cmd_argv[0], "list")){ // list jobs
      cmdctl_print(ctl);
    }
    else if(!strcmp(cmd_argv[0],"output-for")){ //print output
      int n = atoi(cmd_argv[1]);
      print_output(ctl, n); //helper function
    }
    else if(!strcmp(cmd_argv[0],"output-all")){ //print all of outputs
      for(int i = 0; i < ctl->size; i++){
        print_output(ctl, i);
      }
    }
    else if(!strcmp(cmd_argv[0], "wait-for")){ //wait for certain job
      int n = atoi(cmd_argv[1]);
      cmd_update_state(ctl->cmd[n], DOBLOCK); //parent needs to stop before child so DOBLOCK
    }
    else if(!strcmp(cmd_argv[0], "wait-all")){ //wait all of jobs
      cmdctl_update_state(ctl, DOBLOCK); //parent needs to stop before child so DOBLOCK
    }
    else if(!strcmp(cmd_argv[0],"pause")){ //pause for some time
      long nano = atol(cmd_argv[1]);
      int sec = atoi(cmd_argv[2]);
      pause_for(nano, sec);//use pause_for from util.c
    }
    else{ //if it is not build-in command
      cmd_t *cmd = cmd_new(cmd_argv); //create new cmd
      cmd_start(cmd); //start new cmd
      cmdctl_add(ctl,cmd); //add it to cmdctl
    }

    END: cmdctl_update_state(ctl, NOBLOCK); //end of loop update states of jobs; no need to stop so NOBLOCK

  }

  return 0;

}

void print_echo(char *input, int echo){
  if(echo){
    printf("%s",input);
  }
}

void print_output(cmdctl_t *ctl, int n){ //definition of helper function
  printf("@<<< Output for %s[#%d] (%d bytes):\n", ctl->cmd[n]->name, ctl->cmd[n]->pid, ctl->cmd[n]->output_size);
  printf("----------------------------------------\n");
  cmd_print_output(ctl->cmd[n]);
  printf("----------------------------------------\n");
}
