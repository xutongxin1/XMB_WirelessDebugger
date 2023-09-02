#ifndef _UART_CONFIG_H_
#define _UART_CONFIG_H_
#include "UART/uart_val.h"

UartErrT Delete_All_Uart_Task(void);
UartErrT CreateUartTask(UartInitT *uart_config);
UartErrT UartSetup(UartInitT *config);
UartErrT IsUartNumFree(uart_port_t uart_num);
uart_port_t GetUartFreeNum(void);
UartErrT UartStateRegister(UartInitT *config);
uint8_t GetUartManageId(uart_port_t uart_num);
UartErrT UartSetup(UartInitT *config);
void UartSend(UartInitT* uart_config);
void UartRev(UartInitT* uart_config);
#endif