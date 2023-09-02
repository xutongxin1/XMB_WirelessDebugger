#ifndef __TCP_SERVER1_H__
#define __TCP_SERVER1_H__

#define PORT                        1920
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3

// typedef struct uart_configrantion
// {
//   // int CH;
//   // int mode;
//   // char *band;
//   // char *parity;
//   // int data;
//   // int stop;
//   QueueHandle_t* uart_queue;
//   struct uart_pin pin;
//   uart_port_t uart_num;
//   enum UartIOMode mode;
//   uart_config_t uart_config_;

// } ;

//#include "UART/uart_config.h"
//#include "../wireless-esp8266-dap/WirelessDAP_main/usbip_server.h"

//enum StateT {
//  ACCEPTING,
//  ATTACHING,
//  EMULATING,
//  EL_DATA_PHASE
//};
#define ACCEPTING 0
#define ATTACHING 1
#define EMULATING 2
#define EL_DATA_PHASE 3

void TCPInstructionTask();
void HeartBeat(unsigned int len, char *rx_buffer);
void CommandJsonAnalysis(unsigned int len, void *rx_buffer, int ksock);
void ChangeWorkMode(int mode);



#endif