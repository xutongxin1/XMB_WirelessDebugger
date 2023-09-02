#ifndef __NVS_API_H__
#define __NVS_API_H__

void NVSFlashWrite(char mode_number);
int NVSFlashRead();
extern  char need_send_RF;
#endif