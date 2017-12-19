#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "orazio_client.h"
#include "orazio_print_packet.h"

typedef struct {
  struct OrazioClient* client;
  volatile int run;
} RunnerParams;


void* _runnerFn(void* args_){
  RunnerParams* args=(RunnerParams*) args_;
  while(args->run){
    OrazioClient_sync(args->client,1);
    char buffer[BUFFER_SIZE_MAX];
    char s[4096];
    char* s_end=s;
    PacketHeader* header=(PacketHeader*) buffer;
    if (OrazioClient_get(args->client, header, SYSTEM_STATUS_PACKET_ID)==Success){
      s_end+=Orazio_printPacket(s_end, header);
    }
    if (OrazioClient_get(args->client, header, SYSTEM_PARAM_PACKET_ID)==Success)
      s_end+=Orazio_printPacket(s_end, header);
    //Orazio_printPacket(stdout,&args->client->joint_status);
    //Orazio_printPacket(stdout,&args->client->joint_param);
    //Orazio_printPacket(stdout, &args->client->drive_status);
    //Orazio_printPacket(stdout, &args->client->drive_param);
    if (s_end!=s)
      printf("%s\n",s);
    DifferentialDriveControlPacket base_control={
      {
	.type=DIFFERENTIAL_DRIVE_CONTROL_PACKET_ID,
	.size=sizeof(DifferentialDriveControlPacket),
	.seq=0
      },
      .translational_velocity=0.1,
      .rotational_velocity=0.1
    };
    OrazioClient_sendPacket(args->client, (PacketHeader*)&base_control, 0);
  }
  return 0;
}

#define LINE_BUF_SIZE 1024
#define ARGS_MAX 10
#define ARG_LENGTH_MAX 20

int main(int argc, char** argv) {
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
  
  struct OrazioClient* client=OrazioClient_init(argv[1], baudrate);
  
  if (! client) {
    printf("Failed\n");
    exit(-1);
  }
  printf("Success\n");

  printf("Synching...");
  OrazioClient_sync(client,10);
  printf(" Done\n");

  OrazioClient_readConfiguration(client, 100);
  
  printf("Looping\n");
  RunnerParams runner_params={
    .client=client,
    .run=1
  };
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_t runner_thread;
  pthread_create(&runner_thread, &attr, _runnerFn, &runner_params);

  while(runner_params.run){
    char buf[1000];
    scanf("%s",buf);
    OrazioClient_readConfiguration(client,100);
  }
  void* runner_result;
  pthread_join(runner_thread, &runner_result);
  
  OrazioClient_destroy(client);
}
