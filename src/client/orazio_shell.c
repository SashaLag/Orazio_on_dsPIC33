#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "orazio_client.h"
#include "orazio_print_packet.h"
#include "orazio_shell_globals.h"
#include "orazio_shell_commands.h"

#define MAX_COMPLETIONS 1024

void* _runnerFn(void* args_){
  while(run){
    OrazioClient_sync(client,1);
    refreshState();
    // if we wait for the state we warn the listener
    if (state_sem_init)
      sem_post(&state_sem);
    // if we have a param operation, we issue it and warn the listener
    if (param_sem_init){
      op_result=OrazioClient_sendPacket(client, (PacketHeader*)&param_control, 100);
      if (op_result==Success)
	refreshParams(param_control.param_type);
      sem_post(&param_sem);
    }
  }
  return 0;
}

int commonSubstring(char* dest, const char* src){
  int k=0;
  while(*dest && *src && *dest == *src)
    ++k, ++dest, ++src;
  *dest=0;
  return k;
}

char** completeCommand(const char* line, int start, int end){
  rl_attempted_completion_over=1;
  char** completions=(char**)malloc(sizeof(char*)*MAX_COMPLETIONS);
  int k=1;
  int cmd_idx=0;
  while(cmd_idx<num_commands){
    const char* name=commands[cmd_idx].name;
    completions[k]=strdup(name);
    if (!strncmp(line, name, end-start)){
      if (k==1)
	completions[0]=strdup(name);
      else {
	commonSubstring(completions[0],name);
      }
      ++k;
    }
    ++cmd_idx;
  }
  if (k==1) {
    free(completions);
    return 0;
  }
  completions[k]=0;
  return completions;
}

char** completeVar(const char* line, int start, int end) {
  rl_attempted_completion_over=1;
  char** completions=(char**)malloc(sizeof(char*)*MAX_COMPLETIONS);
  int k=1;
  int var_idx=0;
  while(var_idx<num_variables){
    const char* name=variable_entries[var_idx].name;
    completions[k]=strdup(name);
    if (!strncmp(line, name, end-start)){
      if (k==1)
	completions[0]=strdup(name);
      else {
	commonSubstring(completions[0],name);
      }
      ++k;
    }
    ++var_idx;
  }
  if (k==1) {
    free(completions);
    return 0;
  }
  completions[k]=0;
  return completions;
}


char** completeLine(const char* line, int start, int end){
  if (start==0)
    return completeCommand(line, start, end);
 return completeVar(line, start, end);
}


int main(int argc, char** argv){
  rl_attempted_completion_function = completeLine;

  Orazio_printPacketInit();
  if (argc < 2) {
    printf("%s <device> [baudrate]\n", argv[0]);
    exit(0);
  }
  const char* device=argv[1];
  uint32_t baudrate = 115200;
  if (argc>2)
    baudrate = atoi(argv[2]);
  printf("starting client on device [%s], with baudrate %d... ",
	 device,
	 baudrate);
  
  client=OrazioClient_init(argv[1], baudrate);
  
  if (! client) {
    printf("Failed\n");
    exit(-1);
  }
  printf("Success\n");

  printf("Synching");
  int sync_cycles=50;
  for (int i=0; i<sync_cycles; ++i){
    OrazioClient_sync(client,1);
    printf(".");
    fflush(stdout);
  }
  printf(" Done\n");

  OrazioClient_readConfiguration(client, 100);
  printf("Looping\n");
  refreshParams(ParamSystem);
  refreshParams(ParamJoints);
  refreshParams(ParamDrive);
  
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_t runner_thread;
  pthread_create(&runner_thread, &attr, _runnerFn, 0);

  while (run) {
    char *buffer = readline("orazio> ");
    if (buffer) {
      char response[10000];
      executeCommand(response, buffer);
      if (*buffer)
	add_history(buffer);
      free(buffer);
    } else
      run=0;
  }

  void* runner_result;
  pthread_join(runner_thread, &runner_result);
  OrazioClient_destroy(client);
}
