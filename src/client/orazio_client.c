#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "orazio_client.h"
#include "serial_linux.h"


typedef struct OrazioClient {
  PacketHandler packet_handler;
  uint16_t global_seq;
  
  //these are the system variables, updated by the serial communiction
  ResponsePacket response;
  EndEpochPacket end_epoch;
  SystemParamPacket system_param;
  SystemStatusPacket system_status;
  JointParamPacket joint_param;
  JointStatusPacket joint_status;
  DifferentialDriveParamPacket drive_param;
  DifferentialDriveStatusPacket drive_status;
  
  // file descriptor of the serial port
  int fd;
  uint8_t packet_buffer[BUFFER_SIZE_MAX];

  //sanity check on transmission
  PacketSize packet_sizes[PACKET_TYPE_MAX];
  pthread_mutex_t write_mutex;
  pthread_mutex_t read_mutex;

  DifferentialDriveControlPacket drive_control_packet; // deferred drive control packet only one per comm cycle will be sent
  JointControlPacket joint_control_packet;                  // deferred joint control packet only one per comm cycle will be sent
} 
OrazioClient;

static PacketHeader* _initializeBuffer(PacketType type, PacketSize size, void* arg){
  OrazioClient* client=(OrazioClient*)arg;
  return (PacketHeader*) client->packet_buffer;
}
  
// this handler is called whenever a packet is complete
//! no deferred action will take place
static PacketStatus _copyToBuffer(PacketHeader* p, void* args) {
  memcpy(args, p, p->size);
  return Success;
}

static PacketStatus _installPacketOp(OrazioClient* cl,
				     PacketHeader* dest,
				     PacketType type,
				     PacketSize size){
  
  PacketOperations* ops=(PacketOperations*)malloc(sizeof(PacketOperations));
  ops->type=type;
  ops->size=size;
  ops->initialize_buffer_fn=_initializeBuffer;
  ops->initialize_buffer_args=cl;
  ops->on_receive_fn=_copyToBuffer;
  ops->on_receive_args=dest;
  PacketStatus install_result=PacketHandler_installPacket(&cl->packet_handler, ops);
  if (install_result!=Success) {
    printf("error in installing ops");
    exit(0);
    assert(0);
    free(ops);
  }
  return install_result;
}

static void _flushBuffer(OrazioClient* cl){
  while(cl->packet_handler.tx_size){
    uint8_t c=PacketHandler_txByte(&cl->packet_handler);
    write(cl->fd,&c,1);
  }
}

static void _readPacket(OrazioClient* cl){
  volatile int packet_complete=0;
  while (! packet_complete) {
    uint8_t c;
    int n=read (cl->fd, &c, 1);
    if (n) {
      fflush(stdout);
      PacketStatus status = PacketHandler_rxByte(&cl->packet_handler, c);
      if (0 && status<0){
	printf("error: %d\n", status);
      }
      packet_complete = (status==SyncChecksum);
    }
  }
}

				    
OrazioClient* OrazioClient_init(const char* device, uint32_t baudrate){
  // tries to open and configure a device
  int fd=serial_open(device);
    if(fd<0)
      return 0;
   if (serial_set_interface_attribs(fd, baudrate, 0) <0)
      return 0;
    serial_set_blocking(fd, 1); 
  if  (! fd)
    return 0;
  
  OrazioClient* cl=(OrazioClient*) malloc(sizeof(OrazioClient));
  cl->global_seq=0;
  cl->fd=fd;

  // initializes the packets to send
  DifferentialDriveControlPacket ddcp={
    { .type=DIFFERENTIAL_DRIVE_CONTROL_PACKET_ID,
      .size=sizeof(DifferentialDriveControlPacket),
      .seq=0
    },
    .translational_velocity=0.f,
    .rotational_velocity=0.f
  };
  cl->drive_control_packet = ddcp;
  
  
  
  // initializes the packet system
  PacketHandler_initialize(&cl->packet_handler);

  _installPacketOp(cl, (PacketHeader*)&cl->response, RESPONSE_PACKET_ID, sizeof(cl->response));
  _installPacketOp(cl, (PacketHeader*)&cl->system_param, SYSTEM_PARAM_PACKET_ID, sizeof(cl->system_param));
  _installPacketOp(cl, (PacketHeader*)&cl->system_status, SYSTEM_STATUS_PACKET_ID, sizeof(cl->system_status));
  _installPacketOp(cl, (PacketHeader*)&cl->joint_param, JOINT_PARAM_PACKET_ID, sizeof(cl->joint_param));
  _installPacketOp(cl, (PacketHeader*)&cl->joint_status, JOINT_STATUS_PACKET_ID, sizeof(cl->joint_status));
  _installPacketOp(cl, (PacketHeader*)&cl->drive_param, DIFFERENTIAL_DRIVE_PARAM_PACKET_ID, sizeof(cl->drive_param));
  _installPacketOp(cl, (PacketHeader*)&cl->drive_status, DIFFERENTIAL_DRIVE_STATUS_PACKET_ID, sizeof(cl->drive_status));
  _installPacketOp(cl, (PacketHeader*)&cl->end_epoch, END_EPOCH_PACKET_ID, sizeof(cl->end_epoch));

  // initialize the end epoch packet to make valgrind happy
  cl->end_epoch.type=END_EPOCH_PACKET_ID;
  cl->end_epoch.size=sizeof(cl->end_epoch);
  cl->end_epoch.seq=0;

  // initialize the response
  cl->response.header.type=RESPONSE_PACKET_ID;
  cl->response.header.size=sizeof(cl->response),
  cl->response.header.seq=0;
  cl->response.p_seq=0;
  cl->response.p_type=PACKET_TYPE_MAX;

  // initializes the outbound type/packet size
  memset(&cl->packet_sizes, 0, sizeof(cl->packet_sizes));
  cl->packet_sizes[PARAM_CONTROL_PACKET_ID]=sizeof(ParamControlPacket);
  cl->packet_sizes[SYSTEM_PARAM_PACKET_ID]=sizeof(SystemParamPacket);
  cl->packet_sizes[JOINT_PARAM_PACKET_ID]=sizeof(JointParamPacket);
  cl->packet_sizes[JOINT_CONTROL_PACKET_ID]=sizeof(JointControlPacket);
  cl->packet_sizes[DIFFERENTIAL_DRIVE_PARAM_PACKET_ID]=sizeof(DifferentialDriveParamPacket);
  cl->packet_sizes[DIFFERENTIAL_DRIVE_CONTROL_PACKET_ID]=sizeof(DifferentialDriveControlPacket);
  pthread_mutex_init(&cl->write_mutex,NULL);
  pthread_mutex_init(&cl->read_mutex,NULL);
  return cl;
}


void OrazioClient_destroy(OrazioClient* cl){
  close(cl->fd);
  for (int i=0; i<PACKET_TYPE_MAX; ++i)
    if (cl->packet_handler.operations[i]){
      free(cl->packet_handler.operations[i]);
    }
  pthread_mutex_destroy(&cl->write_mutex);
  pthread_mutex_destroy(&cl->read_mutex);
  free(cl);
}

static PacketStatus _sendPacket(OrazioClient* cl, PacketHeader* p){
  ++cl->global_seq;
  PacketType type=p->type;
  if(type>=PACKET_TYPE_MAX)
    return UnknownType;
  PacketSize expected_size=cl->packet_sizes[type];
  if(! expected_size)
    return UnknownType;
  if(p->size!=expected_size)
    return InvalidSize;
  p->seq=cl->global_seq;
  return PacketHandler_sendPacket(&cl->packet_handler, p);
}

PacketStatus OrazioClient_sendPacket(OrazioClient* cl, PacketHeader* p, int timeout){
  PacketStatus send_result=GenericError;
  // non blocking operation
  if (! timeout) {
    pthread_mutex_lock(&cl->write_mutex);
    send_result=_sendPacket(cl,p);
    pthread_mutex_unlock(&cl->write_mutex);
    return send_result;
  }

  //blocking operation
  // no one else can write or read, we read packets
  // until timeout or until the response is received
  pthread_mutex_lock(&cl->write_mutex);
  pthread_mutex_lock(&cl->read_mutex);  
  send_result=_sendPacket(cl,p);
  if(send_result!=Success) {
    goto safe_exit;
  }
  _flushBuffer(cl);
  uint16_t awaited_seq=p->seq;
  uint16_t awaited_type=p->type;
  send_result = Timeout;
  while(timeout>0){
    _readPacket(cl);
    if (cl->response.p_type==awaited_type
	&& cl->response.p_seq==awaited_seq){
      send_result = Success;
      break;
    }
    timeout--;
  }
 safe_exit:
  pthread_mutex_unlock(&cl->read_mutex);
  pthread_mutex_unlock(&cl->write_mutex);
  return send_result;
}

PacketStatus OrazioClient_sync(OrazioClient* cl, int cycles) {
  for (int c=0; c<cycles; ++c){
    pthread_mutex_lock(&cl->write_mutex);
     _flushBuffer(cl);
     if(cl->drive_control_packet.header.seq)
       _sendPacket(cl, (PacketHeader*) (&cl->drive_control_packet));
    pthread_mutex_unlock(&cl->write_mutex);
    uint16_t current_seq=cl->end_epoch.seq;
    do {
      pthread_mutex_lock(&cl->read_mutex);
     _readPacket(cl);
      pthread_mutex_unlock(&cl->read_mutex);
    } while (current_seq==cl->end_epoch.seq);
    //printf ("Sync! current_seq:%d\n", cl->end_epoch.seq);
  }
  return Success;
}

PacketStatus OrazioClient_readConfiguration(struct OrazioClient* cl, int timeout){

  printf("Querying params\n");
  ParamControlPacket query={
    {
      .type=PARAM_CONTROL_PACKET_ID,
      .size=sizeof(ParamControlPacket),
      .seq=0
    },
    .action=ParamRequest,
    .param_type=ParamSystem
  };
  PacketStatus status=OrazioClient_sendPacket(cl, (PacketHeader*)&query, timeout);
  printf("  [System] Status: %d\n", status);
  if (status!=Success)
    return status;
  
  query.param_type=ParamJoints;
  status=OrazioClient_sendPacket(cl, (PacketHeader*)&query, timeout);
  printf("  [Joints] Status: %d\n", status);
  if (status!=Success)
    return status;

  query.param_type=ParamDrive;
  status=OrazioClient_sendPacket(cl, (PacketHeader*)&query, timeout);
  printf("  [Drive] Status: %d\n", status);
  if (status!=Success)
    return status;

  printf("Done\n");
  return status;
}

//gets a packet from Orazio client
PacketStatus OrazioClient_get(struct OrazioClient* cl, PacketHeader* dest, PacketType type){
  if (type>=PACKET_TYPE_MAX)
    return PacketTypeOutOfBounds;
  // all packets received have been registered in the handler ops
  // and the var on_receive_args of the buffer points to the correct variable
  const PacketOperations* ops=cl->packet_handler.operations[type];
  if (! ops){
    return UnknownType;
  }
  assert(ops->type==type);
  pthread_mutex_lock(&cl->read_mutex);
  memcpy(dest, ops->on_receive_args, ops->size); 
  pthread_mutex_unlock(&cl->read_mutex);
  return Success;
}

void OrazioClient_setBaseVelocity(struct OrazioClient* cl, float tv, float rv){
  cl->drive_control_packet.translational_velocity=tv;
  cl->drive_control_packet.rotational_velocity=rv;
  cl->drive_control_packet.header.seq=1;
}

void OrazioClient_getOdometryPosition(struct OrazioClient* cl, float* x, float* y, float* theta){
  *x=cl->drive_status.odom_x;
  *y=cl->drive_status.odom_y;
  *theta=cl->drive_status.odom_theta;
}

