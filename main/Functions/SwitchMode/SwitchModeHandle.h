#ifndef __Handle_H__
#define __Handle_H__

#define MAX_MODE_NUMBER 10
enum WorkMode {
    NONE_MODE=0,
    DAP = 1,
    UART,
    ADC,
    DAC,
    PWM_COLLECT,
    PWM_SIMULATION,
    I2C,
    SPI,
    CAN
};

void DAPHandle(void);
void UartHandle(void);
void ADCHandle(void);
void DACHandle(void);
void PwmCollectHandle(void);
void PwmSimulationHandle(void);
void I2CHandle(void);
void SpiHandle(void);
void CanHandle(void);

extern enum WorkMode working_mode;

void UartTask(int ksock);

#endif