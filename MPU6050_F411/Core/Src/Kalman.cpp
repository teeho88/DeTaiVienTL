/*
 * Kalman.cpp
 *
 *  Created on: Dec 15, 2022
 *      Author: Teeho
 */

#include <Kalman_cpp.h>
#include "math.h"

float PI = 3.1416;

Kalman::Kalman(float z0)
{
	x = z0;
}

float Kalman::update(float z)
{
	float D_ = D + Q;
	float K = D_ / (D_ + R);
	x = x + K * (z - x);
	D = (1 - K) * D_;
	return x;
}

GradFilter::GradFilter(float beta_1, float eps_1, int count_to_change_params)
{
	beta1 = beta_1;
	eps1 = eps_1;
	cnt_thres = count_to_change_params;
	cnt = 0;
}

void GradFilter::init(float gx, float gz)
{
	roll = atan2(gz, gx);
}

void GradFilter::init(float roll0)
{
	roll = roll0;
}

float GradFilter::update(float wz, float gx, float gy, float delT)
{
	if (cnt == cnt_thres)
	{
        beta = beta1;
        eps = eps1;
	}
	float norm = sqrt(gx*gx + gy*gy);
	if(norm == 0)
	{
		w_bias += eps*fDiv*delT;
		roll += (wz - w_bias - beta*fDiv)*delT;
		return roll;
	}
	float gxNorm = gx/norm;
	float gyNorm = gy/norm;
	fDiv = sin(roll)*gyNorm - cos(roll)*gxNorm;
	w_bias += eps*fDiv*delT;
	float wCalib = wz - w_bias;
	roll += (wCalib - beta*fDiv)*delT;
	if (roll >= 2*PI)
	{
		roll -= 2*PI;
	}
	if (roll <= -2*PI)
	{
		roll += 2*PI;
	}
	cnt++;
	return roll;
}
