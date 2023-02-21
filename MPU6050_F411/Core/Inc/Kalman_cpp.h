/*
 * Kalman_cpp.h
 *
 *  Created on: Dec 16, 2022
 *      Author: Teeho
 */

#ifndef INC_KALMAN_CPP
#define INC_KALMAN_CPP

#ifdef __cplusplus
class Kalman
{
public:
	Kalman(float z0);
	float update(float z);
private:
	float Q = 0.13;
	float R = 1;
	float D = 1000;
	float x;
};

class GradFilter
{
public:
	GradFilter(float beta_1, float eps_1, int count_to_change_params);
	void init(float gx, float gz);
	void init(float roll0);
	float update(float wy, float gx, float gz, float delT);
private:
	float beta = 15;
	float beta1;
	float eps = 1;
	float eps1;
	float roll;
	float fDiv;
	float w_bias;
	int cnt;
	int cnt_thres;
};
#endif

#endif /* INC_KALMAN_CPP */
