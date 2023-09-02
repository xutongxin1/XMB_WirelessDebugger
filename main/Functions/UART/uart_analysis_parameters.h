#ifndef __UART_ANALYSIS_PARAMETERS_H__
#define __UART_ANALYSIS_PARAMETERS_H__

#include "UART/uart_val.h"


int Uart1ParameterAnalysis(void *attach_rx_buffer, UartInitT* uart_config);
int Uart2ParameterAnalysis(void *attach_rx_buffer, UartInitT* uart_config);

#endif