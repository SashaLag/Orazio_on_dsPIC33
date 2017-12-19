#pragma once
#include "packet_handler.h"
#include "orazio_packets.h"

#ifdef __cplusplus
extern "C" {
#endif

  struct OrazioClient;

  // creates a new orazio client, opening a serial connection on device at the selected baudrate
  struct OrazioClient* OrazioClient_init(const char* device, uint32_t baudrate);

  // destroyes a previously created orazio client
  void OrazioClient_destroy(struct OrazioClient* cl);

  // sends a packet
  // if timeout ==0, the packet sending is deferred
  // otherwise it enables a synchronous operation that waits for timeout packets to be received
  PacketStatus OrazioClient_sendPacket(struct OrazioClient* cl, PacketHeader* p, int timeout);


  //gets a packet from Orazio client
  PacketStatus OrazioClient_get(struct OrazioClient* cl, PacketHeader* dest, PacketType type);


  /**High level functionalities, multithreaded*/

  // flushes the deferred tx queues,
  // reads all packets of an epoch (same seq),
  // and returns
  // call it periodically
  PacketStatus OrazioClient_sync(struct OrazioClient* cl, int cycles);

  //to be called at the beginning after a few loops of sync
  PacketStatus OrazioClient_readConfiguration(struct OrazioClient* cl, int timeout);

  //
  PacketStatus OrazioClient_getBaseVelocities(struct OrazioClient* cl, float *tv, float *rv);

  PacketStatus OrazioClient_setBaseVelocities(struct OrazioClient* cl, float tv, float rv);

  PacketStatus OrazioClient_getBasePosition(struct OrazioClient* cl, float*x, float*y, float*theta);

  PacketStatus OrazioClient_setBasePosition(struct OrazioClient* cl, float x, float y, float theta);

  PacketStatus OrazioClient_setJointControl(struct OrazioClient* cl,
					    uint8_t joint_num,
					    JointMode mode,
					    uint16_t speed);

  PacketStatus OrazioClient_getJointStatus(struct OrazioClient* cl,
					   uint8_t joint_num,
					   JointMode* mode,
					   uint16_t speed);
#ifdef __cplusplus
}
#endif
