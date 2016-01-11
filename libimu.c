////////////////////////////////////////////////////////////////////////////
//
//  This file is part of linux-mpu9150
//
//  Copyright (c) 2013 Pansenti, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>

#include "mpu9150.h"
#include "linux_glue.h"
#include "libimu.h"


// static void fused_euler_angles(mpudata_t *mpu);
// static void fused_quaternion(mpudata_t *mpu);
// static void calibrated_accel(mpudata_t *mpu);
// static void calibrated_mag(mpudata_t *mpu);


int i2c_bus;
int sample_rate;
int yaw_mix_factor;
int verbose;

void close_mpu(void)
{
	mpu9150_exit();
}



int init_mpu(int sample_rate, int yaw_mix_factor)
{
	int ret;
	verbose = 1;
	i2c_bus = DEFAULT_I2C_BUS;
	// sample_rate = DEFAULT_SAMPLE_RATE_HZ;
	// yaw_mix_factor = DEFAULT_YAW_MIX_FACTOR;

	mpu9150_set_debug(verbose);
	if ((ret = mpu9150_init(i2c_bus, sample_rate, yaw_mix_factor)) != 0) {
		return ret;
	}

	set_cal(0, NULL);
	set_cal(1, NULL);

	return ret;
}

int read_mpu(float *pitch, float *roll, float *heading)
{
	int ret;
	static mpudata_t mpu;
	memset(&mpu, 0, sizeof(mpudata_t));
	ret = mpu9150_read(&mpu);
	*pitch = mpu.fusedEuler[0];
	*roll = mpu.fusedEuler[1];
	*heading = mpu.fusedEuler[2];
	return ret;
	// fused_euler_angles(&mpu);
	// fused_quaternions(&mpu);
	// calibrated_accel(&mpu);
	// calibrated_mag(&mpu);
}

// void fused_euler_angles(mpudata_t *mpu)
// {
// 	mpu->fusedEuler[VEC3_X] * RAD_TO_DEGREE;
// 	mpu->fusedEuler[VEC3_Y] * RAD_TO_DEGREE;
// 	mpu->fusedEuler[VEC3_Z] * RAD_TO_DEGREE;
// }

// void fused_quaternions(mpudata_t *mpu)
// {
// 	mpu->fusedQuat[QUAT_W];
// 	mpu->fusedQuat[QUAT_X];
// 	mpu->fusedQuat[QUAT_Y];
// 	mpu->fusedQuat[QUAT_Z];
// }

// void calibrated_accel(mpudata_t *mpu)
// {
// 	mpu->calibratedAccel[VEC3_X];
// 	mpu->calibratedAccel[VEC3_Y];
// 	mpu->calibratedAccel[VEC3_Z];
// }

// void calibrated_mag(mpudata_t *mpu)
// {
// 	mpu->calibratedMag[VEC3_X];
// 	mpu->calibratedMag[VEC3_Y];
// 	mpu->calibratedMag[VEC3_Z];
// }

int set_cal(int mag, char *cal_file)
{
	int i;
	FILE *f;
	char buff[32];
	long val[6];
	caldata_t cal;

	if (cal_file) {
		f = fopen(cal_file, "r");

		if (!f) {
			perror("open(<cal-file>)");
			return -1;
		}
	}
	else {
		if (mag) {
			f = fopen("./magcal.txt", "r");

			if (!f) {
				printf("Default magcal.txt not found\n");
				return 0;
			}
		}
		else {
			f = fopen("./accelcal.txt", "r");

			if (!f) {
				printf("Default accelcal.txt not found\n");
				return 0;
			}
		}
	}

	memset(buff, 0, sizeof(buff));

	for (i = 0; i < 6; i++) {
		if (!fgets(buff, 20, f)) {
			printf("Not enough lines in calibration file\n");
			break;
		}

		val[i] = atoi(buff);

		if (val[i] == 0) {
			printf("Invalid cal value: %s\n", buff);
			break;
		}
	}

	fclose(f);

	if (i != 6)
		return -1;

	cal.offset[0] = (short)((val[0] + val[1]) / 2);
	cal.offset[1] = (short)((val[2] + val[3]) / 2);
	cal.offset[2] = (short)((val[4] + val[5]) / 2);

	cal.range[0] = (short)(val[1] - cal.offset[0]);
	cal.range[1] = (short)(val[3] - cal.offset[1]);
	cal.range[2] = (short)(val[5] - cal.offset[2]);

	if (mag)
		mpu9150_set_mag_cal(&cal);
	else
		mpu9150_set_accel_cal(&cal);

	return 0;
}
