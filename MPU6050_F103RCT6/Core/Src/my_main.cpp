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
#include "flash_storage.h"

extern uint8_t buffUART[];
extern uint8_t PcData[];

extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

float T = 0;
float Roll_IMU = 0;
float Roll_T;
GradFilter Filter(15,1,-1);
Kalman RFCKal(0);
char tx2pcData[100];

float acc[3], gyr[3];
extern float PI;

volatile uint16_t RFCAdcValue = 0;
float RFC_Factor;
uint16_t MinRFC = 13;
uint16_t MaxRFC = 3650;
uint16_t RangeADC;
float RFCAngle = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    // Conversion Complete & DMA Transfer Complete As Well
	RFCAdcValue = RFCKal.update(RFCAdcValue);
	RFCAngle = RFCAdcValue*RFC_Factor;
}

void RFC_Calib()
{
	sprintf(tx2pcData,">.Bat dau hieu chinh RFC\r");
	HAL_UART_Transmit(&huart1, (uint8_t*)tx2pcData, strlen(tx2pcData), 100);
	while(1)
	{
		if(strstr((char*)PcData,"RFCCE"))
		{
			char str[5];
			memcpy(str,&PcData[5],4);
			MinRFC = atoi(str);
			memcpy(str,&PcData[9],4);
			MaxRFC = atoi(str);
			break;
		}
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&RFCAdcValue, 1);
		sprintf(tx2pcData,">%d\r",RFCAdcValue);
		HAL_UART_Transmit(&huart1, (uint8_t*)tx2pcData, strlen(tx2pcData), 100);
		HAL_Delay(3);
	}
	sprintf(tx2pcData,">.Hieu chinh RFC xong\r");
	HAL_UART_Transmit(&huart1, (uint8_t*)tx2pcData, strlen(tx2pcData), 100);
}

int myMain(void)
{
	Flash_Assign_Param(&MinRFC, &MaxRFC, MinRFC, MaxRFC);
	// Init values
	Flash_Soft_GetOffset();
	RangeADC = MaxRFC - MinRFC;
	RFC_Factor = 360.0f/RangeADC;

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&RFCAdcValue, 1);

	HAL_TIM_Base_Start(&htim2);
	uint32_t t_old = __HAL_TIM_GetCounter(&htim2);
	uint16_t init_loops = 50;
	uint16_t num_loop = 0;
	Roll_IMU = 0;
	uint8_t cnt_trans = 0;
	while (1)
	{
		cnt_trans++;
		if (strstr((char*) PcData, "IMUCS"))
		{
			sprintf(tx2pcData, ">Bat dau hieu chinh MPU6050\r");
			HAL_UART_Transmit(&huart1, (uint8_t*) tx2pcData, strlen(tx2pcData), 100);
			CalibrateMPU6050();
			memset(PcData, 0, BUFFER_SIZE);
			sprintf(tx2pcData, ">Hieu chinh xong\r");
			HAL_UART_Transmit(&huart1, (uint8_t*) tx2pcData, strlen(tx2pcData), 100);
		}
		if (strstr((char*) PcData, "RFCCS"))
		{
			RFC_Calib();
			memset(PcData, 0, BUFFER_SIZE);
			Flash_Soft_SetOffset(MinRFC, MaxRFC);
			RangeADC = MaxRFC - MinRFC;
			RFC_Factor = 360.0f / RangeADC;
		}

		if (Read_MPU_Calc(acc, gyr) != HAL_OK)
			continue;

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&RFCAdcValue, 1);

		float wz = gyr[2];
		float gx = acc[1];
		float gy = -acc[0];
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
		Roll_T = Roll_IMU*(180 / PI) - RFCAngle;
		if(cnt_trans >= 1)
		{
			cnt_trans = 0;
			sprintf(tx2pcData, ">%.4f|%.4f|%.4f|%.4f|%.4f\r\n", Roll_T, wz, gx, gy, T);
			HAL_UART_Transmit(&huart1, (uint8_t*) tx2pcData, strlen(tx2pcData), 100);
		}
	}
}



