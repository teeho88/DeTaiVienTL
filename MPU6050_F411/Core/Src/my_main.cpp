/*
 * my_main.cpp
 *
 *  Created on: Feb 20, 2023
 *      Author: Teeho
 */
#include "my_main.h"

#include "stdio.h"
#include "string.h"
#include "Kalman_cpp.h"
#include "TJ_MPU6050.h"
#include "math.h"

extern uint8_t buffUART[];
extern uint8_t PcData[];

extern I2C_HandleTypeDef hi2c1;

extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

float T = 0;
float Roll_IMU = 0;
GradFilter Filter(15,1,-1);
char tx2pcData[100];

int myMain(void)
{
	uint32_t t_old = __HAL_TIM_GetCounter(&htim2);
	uint16_t init_loops = 50;
	uint16_t num_loop = 0;
	Roll_IMU = 0;
	float acc[3], gyr[3];
	while (1)
	{
		if (strstr((char*) PcData, "IMUCS"))
		{
			sprintf(tx2pcData, ">Bat dau hieu chinh MPU6050\r");
			HAL_UART_Transmit(&huart2, (uint8_t*) tx2pcData, strlen(tx2pcData), 100);
			CalibrateMPU6050();
			sprintf(tx2pcData, ">Hieu chinh xong\r");
			HAL_UART_Transmit(&huart2, (uint8_t*) tx2pcData, strlen(tx2pcData), 100);
			memset(PcData, 0, BUFFER_SIZE);
		}
		if (Read_MPU_Calc(acc, gyr) != HAL_OK)
			continue;
		float wz = gyr[2];
		float gx = acc[0];
		float gy = acc[1];
		if (num_loop < init_loops)
		{
			Roll_IMU += atan2(gy, gx);
			t_old = __HAL_TIM_GetCounter(&htim2);
			num_loop++;
			continue;
		}
		else if (num_loop == init_loops)
		{
			Roll_IMU /= init_loops;
			Filter.init(Roll_IMU);
			num_loop++;
		}

		uint32_t t_cur = __HAL_TIM_GetCounter(&htim2);
		T = (t_cur - t_old) * 0.000001;
		t_old = t_cur;

		Roll_IMU = Filter.update(wz, gx, gy, T);
		Roll_IMU *= (180 / 3.14);
		if (Roll_IMU >= 360)
		{
			Roll_IMU -= 360;
		}
		sprintf(tx2pcData, ">%.4f|%.4f|%.4f|%.4f|%.4f\r", Roll_IMU, wz, gx, gy, T);
		HAL_UART_Transmit(&huart2, (uint8_t*) tx2pcData, strlen(tx2pcData), 100);
	}
}



