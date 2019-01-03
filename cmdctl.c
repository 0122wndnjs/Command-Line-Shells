#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "commando.h"

void cmdctl_add(cmdctl_t *ctl, cmd_t *cmd){
  if(ctl->size < MAX_CHILD){ //if size did not exceed maximum num
    ctl->cmd[ctl->size] = cmd; //put cmd to cmd array of ctl
    ctl->size += 1; //increment size by 1
  }
}

void cmdctl_print(cmdctl_t *ctl){
  int i = 0;
  //printing space justification
  printf("%-4s #%-8s %4s %10s %4s %s\n","JOB","PID","STAT","STR_STAT","OUTB","COMMAND");
  while(ctl->cmd[i] != NULL){//if there is cmd in cmd array
    printf("%-4d #%-8d %4d %10s %4d ",i,ctl->cmd[i]->pid, ctl->cmd[i]->status,
             ctl->cmd[i]->str_status, ctl->cmd[i]->output_size);
    for(int j = 0; j < ARG_MAX + 1; j++){ //print cmd argv
      if(ctl->cmd[i]->argv[j] == NULL){
        break;
      }
      printf("%s ", ctl->cmd[i]->argv[j]);
    }
    printf("\n");
    i++;
  }
}
void cmdctl_update_state(cmdctl_t *ctl, int block){
  int i = 0;
  while(ctl->cmd[i] != NULL){ //update state of each cmd in array
    cmd_update_state(ctl->cmd[i], block);
    i++;
  }
}

void cmdctl_freeall(cmdctl_t *ctl){
  for(int i = 0; i < ctl->size; i++){ //free all cmd in cmd array
    if(ctl->cmd[i] != NULL){
      cmd_free(ctl->cmd[i]);
    }
  }
}
