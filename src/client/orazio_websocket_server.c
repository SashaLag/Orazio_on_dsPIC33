#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libwebsockets.h>
#include "orazio_client.h"
#include "orazio_print_packet.h"
#include "orazio_shell_globals.h"
#include "orazio_shell_commands.h"

const char *banner[]={
  "orazio_websocket_server",
  "configures orazio from a web browser",
  "usage:"
  "  $> orazio_robot_websocket_server <parameters>",
  "starts a web server on localhost:9000",
  "parameters: ",
  "-serial-device <string>: the serial port (default /dev/orazio)",
  "-resource-path <string>:  the path containing the html ",
  "        files that will be served by embedded http server (default PWD)",
  "        resource path should be set to the html folder of this repo",
  "-gen    : generates the updated html stubs for the current packets",
  "        the output will be written in resource-path. After generation run the script",
  "        gen_html.sh from the folder",
  "-rate <int>: web refresh rate [Hz] (default 20 Hz)",
  "        use low rate (e.g., 1 Hz) for wireless connection",
  0
};

void printBanner(){
  const char*const* line=banner;
  while (*line) {
    printf("%s\n",*line);
    line++;
  }
}

typedef enum {
  System=1,
  Joints=2,
  Drive=3
} WindowMode;

WindowMode mode=System;

#define PATH_SIZE 1024
char resource_path[PATH_SIZE];

#define  BUF_SIZE 10240
#define NUM_VARIABLES 100
VEntry *system_status_vars[NUM_VARIABLES];
VEntry *system_params_vars[NUM_VARIABLES];
VEntry *joint_status_vars[NUM_VARIABLES];
VEntry *joint_params_vars[NUM_VARIABLES];
VEntry *joint_control_vars[NUM_VARIABLES];
VEntry *drive_status_vars[NUM_VARIABLES];
VEntry *drive_params_vars[NUM_VARIABLES];
VEntry *drive_control_vars[NUM_VARIABLES];
VEntry *response_vars[NUM_VARIABLES];

static int callback_http(struct lws *wsi,
		  enum lws_callback_reasons reason, void *user,
		  void *in, size_t len);

static int callback_input_command( /*struct libwebsocket_context * this_context,*/
				  struct lws *wsi,
				  enum lws_callback_reasons reason,
				  void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",   // name
        callback_http, // callback
        0              // per_session_data_size
    },
    {
        "orazio-robot-protocol", // protocol name - very important!
        callback_input_command,   // callback
        0                          // we don't use any per session data
    },
    {
        NULL, NULL, 0   /* End of list */
    }
};


char client_response[10240];
char* client_response_begin=client_response+LWS_SEND_BUFFER_PRE_PADDING;
int client_response_length=0;
typedef struct lws * WebSocketConnectionPtr;

#define MAX_CONNECTIONS 1024

WebSocketConnectionPtr connections[MAX_CONNECTIONS];


int findConnection(WebSocketConnectionPtr conn){
  for (int i=0; i<MAX_CONNECTIONS; ++i)
    if (connections[i]==conn)
      return i;
  return -1;
}

int getFreeConnectionIdx(){
  for (int i=0; i<MAX_CONNECTIONS; ++i)
    if (!connections[i])
      return i;
  return -1;
}
int freeConnection(WebSocketConnectionPtr conn){
  int idx=findConnection(conn);
  if (idx<0)
    return -1;
  connections[idx]=0;
  return 0;
}

void initConnections(){
  memset(connections, 0, sizeof(WebSocketConnectionPtr)*MAX_CONNECTIONS);
}

void printVarInit(void){
  printf("system_status_vars: %d\n",getVarsByPrefix(system_status_vars, "system_status."));
  printf("system_params_vars: %d\n",getVarsByPrefix(system_params_vars, "system_params."));
  printf("joint_status_vars: %d\n",getVarsByPrefix(joint_status_vars, "joint_status."));
  printf("joint_params_vars: %d\n",getVarsByPrefix(joint_params_vars, "joint_params."));
  printf("joint_control_vars: %d\n",getVarsByPrefix(joint_control_vars, "joint_control."));
  printf("drive_status_vars: %d\n",getVarsByPrefix(drive_status_vars, "drive_status."));
  printf("drive_params_vars: %d\n",getVarsByPrefix(drive_params_vars, "drive_params."));
  printf("drive_control_vars: %d\n",getVarsByPrefix(drive_control_vars, "drive_control."));
  printf("response_vars: %d\n",getVarsByPrefix(response_vars, "response_vars."));
}

int printVarList(char* buffer, VEntry** list){
  char* s=buffer;
  s+=sprintf(s, "<p> <table border=\"1\">\n");
  while(*list){
    VEntry* var=*list;
    s+=sprintf(s, "<tr><td valign=\"top\"> %s </td>\n", var->name);
    if(var->access_flags&VarRead){
      s+=sprintf(s, "<td style=\"min-width:120px\">");
      s+=VEntry_write(s,var);
      s+=sprintf(s, "</td>");
    }
    // if (var->access_flags&VarWrite){
    //   s+=sprintf(s,"<td> <input type=\"text\" id=\"%s\" onclick=\"stopRefresh()\"/></td>", var->name);
    // }
    s+=sprintf(s, "</tr>\n");
    ++list;
  }
  s+=sprintf(s, "</table> </p>");
  return s-buffer;
}

int printVarMask(const char* filename, VEntry** list){
  char buffer[10240];
  char* s=buffer;
  s+=sprintf(s, "<p> <table border=\"3\">\n");
  while(*list){
    VEntry* var=*list;
    ++list;
    if (! (var->access_flags&VarWrite))
      continue;
    char resource_name[1024];
    strcpy(resource_name,var->name);
    char* r=resource_name;
    while(*r) {
      switch (*r) {
      case '.':
      case '[':
      case ']':
	*r='_';
	break;
      default:;
      }
      ++r;
    }
    s+=sprintf(s, "<tr>\n");
    s+=sprintf(s,"   <td valign=\"top\"> %s </td>\n", var->name);
    s+=sprintf(s,"   <td> <input type=\"text\" id=\"%s\" /> </td>\n", resource_name);
    s+=sprintf(s,"   <td> <button type=\"button\" onclick=\"setVariable('%s')\"/> SET</td>\n", var->name);
    s+=sprintf(s, "</tr>\n");
  }
  s+=sprintf(s, "</table> </p>\n");
  char name_buffer[1024];
  sprintf(name_buffer, "%s/%s", resource_path, filename);
  FILE* f=fopen(name_buffer, "w");
  if (f){
    fprintf(f,"%s", buffer);
  }
  fclose(f);
  return s-buffer;
}


static int callback_http(struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len) {

  char *requested_uri = (char *) in;
  char* mime = (char*) "text/html";
switch (reason) {
  case LWS_CALLBACK_CLIENT_WRITEABLE:
    printf("connection established\n");
    break;
  case LWS_CALLBACK_HTTP: 
    requested_uri = (char *) in;
    printf("requested URI: %s\n", requested_uri);
    if(strstr(requested_uri,"system")){
      mode=System;
    } else if(strstr(requested_uri,"joints")){
      mode=Joints;
    } else if(strstr(requested_uri,"drive")){
      mode=Drive;
    } 
    printf("mode %d\n",mode);
    char resource_file[PATH_SIZE];
    strcpy(resource_file,resource_path);
    if (! strcmp(requested_uri,"/") ){
      strcat(resource_file,"/index.html");
    } else {
      strcat(resource_file,requested_uri);
    }
    printf("serving %s\n",resource_file);
    lws_serve_http_file(wsi, resource_file, mime, 0, 0);
    return 1;
    break;
 default:;
  }
  return 0;
}


static int callback_input_command( /*struct libwebsocket_context * this_context,*/
                                   struct lws *wsi,
                                   enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len) {
  int idx=0;
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
    printf("connection established\n");
    idx=getFreeConnectionIdx();
    if (idx>=0)
      connections[idx]=wsi;
    break;

  case LWS_CALLBACK_RECEIVE:  // the funny part
    {
      printf("received data: %s size %d \n", (char*) in, (int)len);
      char cmd_response[1024];
      executeCommand(cmd_response, (char*)in);
    }
    break;

  case LWS_CALLBACK_CLOSED: { // the funny part
    printf("connection closed \n");
    freeConnection(wsi);
    break;

  }
  default:
    break;
  }
    
  return 0;
}


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
	refreshParams((ParamType)param_control.param_type);
      sem_post(&param_sem);
    }
  }
  return 0;
}


int main(int argc, char** argv){
  int c=1;
  resource_path[0]=0;
  const char* device=0;
  uint32_t baudrate = 115200;
  int rate = 20;
  int gen  = 0;
  while (c<argc) {
    if (! strcmp(argv[c],"-resource-path")) {
      c++;
      strcpy(resource_path, argv[c]);
    } else if (! strcmp(argv[c], "-serial-device") ) {
      c++;
      device=argv[c];
    } else if (! strcmp(argv[c], "-rate") ) { 
      c++;
      rate=atoi(argv[c]);
    } else if (! strcmp(argv[c], "-gen") ) { 
      gen=1;
    } else if (! strcmp(argv[c], "-baud") ) { 
      c++;
      baudrate=atoi(argv[c]);
      rate=atoi(argv[c]);
    } else if (! strcmp(argv[c], "-h") || ! strcmp(argv[c], "-help")) {
      printBanner();
      return 0;
    }
    c++;
  }
  if (! strlen(resource_path)) {
    char *r=getcwd(resource_path,PATH_SIZE);
    if (!r) {
      strcpy(resource_path,".");
    }
  }
  printf("running with parameters\n");;
  printf(" resource_path: %s\n", resource_path);
  printf(" serial_device: %s\n", device);
  printf(" baud: %d\n", baudrate);
  printf(" rate: %d Hz\n", rate);
  Orazio_printPacketInit();
  printVarInit();
  
  if (gen) {
    printf("generating stubs in folder %s\n", resource_path);
    printVarMask("system_params.html", system_params_vars);
    printVarMask("joint_params.html", joint_params_vars);
    printVarMask("drive_params.html", drive_params_vars);
    printVarMask("joint_control.html", joint_control_vars);
    printVarMask("drive_control.html", drive_control_vars);
    return 0;
  }
  
  client=OrazioClient_init(device, baudrate);
  
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

  // server url will be http://localhost:9000
  int port = 9000;
  const char *interface = NULL;
  // we're not using ssl
  //const char *cert_path = NULL;
  //const char *key_path = NULL;
  // no special options
  int opts = 0;
    
  struct lws_context_creation_info info;

  memset(&info, 0, sizeof info);
  info.port = port;
  info.iface = interface;
  info.protocols = protocols;
  //info.extensions = lws_get_internal_extensions();
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;
  info.gid = -1;
  info.uid = -1;
  info.options = opts;

  struct lws_context *context = lws_create_context(&info);    
  if (context == NULL) {
    fprintf(stderr, "libwebsocket init failed\n");
    OrazioClient_destroy(client);
    return -1;
  }
  int count=0;
  while (run) {
    char response_string[BUF_SIZE];
    char system_status_string[BUF_SIZE];
    char joint_status_string[BUF_SIZE];
    char drive_status_string[BUF_SIZE];
    char system_params_string[BUF_SIZE];
    char joint_params_string[BUF_SIZE];
    char drive_params_string[BUF_SIZE];
    printVarList(response_string,response_vars);
    printVarList(system_status_string,system_status_vars);
    printVarList(system_params_string,system_params_vars);
    printVarList(joint_status_string,joint_status_vars);
    printVarList(joint_params_string,joint_params_vars);
    printVarList(drive_status_string,drive_status_vars);
    printVarList(drive_params_string,drive_params_vars);
    
    char buffer[102400];
    char* bend=buffer;
    const char* mode_name=0;
    const char* status_string=0;
    const char* param_string=0;
    switch(mode) {
    case System:
      mode_name="System";
      status_string=system_status_string;
      param_string=system_params_string;
      break;
    case Joints:
      mode_name="Joint";
      status_string=joint_status_string;
      param_string=joint_params_string;
      break;
    case Drive:
      mode_name="Drive";
      status_string=drive_status_string;
      param_string=drive_params_string;
    default:;
    }
    if (mode_name) {
      bend+=sprintf(bend, "<p> <table border=\"0\">");
      bend+=sprintf(bend, "<tr>\n");
      bend+=sprintf(bend, "<th> %s Status  </th>\n", mode_name);
      bend+=sprintf(bend, "<th> %s Params  </th>\n", mode_name);
      bend+=sprintf(bend, "</tr>\n");
      bend+=sprintf(bend, "<tr>\n");
      bend+=sprintf(bend, "<td valign=\"top\"> %s </td>\n", status_string);
      bend+=sprintf(bend, "<td valign=\"top\"> %s </td>\n", param_string);
      bend+=sprintf(bend, "</tr>\n");
      bend+=sprintf(bend, "</table> </p>\n");
      //bend+=sprintf(bend, "<p> %s </p>\n", response_string);
    
      client_response_length=bend-buffer+1;
      memcpy(client_response_begin, buffer, client_response_length);
      //printf("%s\n", buffer);
      for (int c=0; c<MAX_CONNECTIONS; ++c){
	WebSocketConnectionPtr wsi=connections[c];
	if (wsi)
	  lws_write(wsi, (unsigned char*) client_response_begin, client_response_length, LWS_WRITE_TEXT);
      }
    }
    lws_service(context, 1000.0/rate);
    count++;
  }
    
  lws_context_destroy(context);
  
  void* runner_result;
  pthread_join(runner_thread, &runner_result);
  OrazioClient_destroy(client);
}
