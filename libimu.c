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

int enableFusion(void) {
	if (enableAccelerometerFusion()) {
		return -1;
	}
	return 0;
}

int disableFusion(void) {
	if (disableAccelerometerFusion()) {
		return -1;
	}
	return 0;
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
}

int read_mpu_raw(short *gx, short *gy, short *gz, short *ax, short *ay, short *az, short *mx, short *my, short *mz)
{
	int ret;
	static mpudata_t mpu;
	memset(&mpu, 0, sizeof(mpudata_t));
	ret = mpu9150_read(&mpu);
	*gx = mpu.rawGyro[0];
	*gy = mpu.rawGyro[1];
	*gz = mpu.rawGyro[2];
	*ax = mpu.rawAccel[0];
	*ay = mpu.rawAccel[1];
	*az = mpu.rawAccel[2];
	*mx = mpu.rawMag[0];
	*my = mpu.rawMag[1];
	*mz = mpu.rawMag[2];
	return ret;
}

int read_mpu_all(float *pitch, float *roll, float *heading,
		short *gx, short *gy, short *gz,
		short *ax, short *ay, short *az,
		long *qx, long *qy, long *qz, long *qw,
		short *mx, short *my, short *mz,
		unsigned long *ts, unsigned long *mts,
		short *x_accel, short *y_accel, short *z_accel,
		short *x_mag, short *y_mag, short *z_mag)
{
	int ret;
	static mpudata_t mpu;
	memset(&mpu, 0, sizeof(mpudata_t));
	ret = mpu9150_read(&mpu);
	*pitch = mpu.fusedEuler[0];
	*roll = mpu.fusedEuler[1];
	*heading = mpu.fusedEuler[2];
	*gx = mpu.rawGyro[0];
	*gy = mpu.rawGyro[1];
	*gz = mpu.rawGyro[2];
	*ax = mpu.rawAccel[0];
	*ay = mpu.rawAccel[1];
	*az = mpu.rawAccel[2];
	*qx = mpu.rawQuat[0];
	*qy = mpu.rawQuat[1];
	*qz = mpu.rawQuat[2];
	*qw = mpu.rawQuat[3];
	*mx = mpu.rawMag[0];
	*my = mpu.rawMag[1];
	*mz = mpu.rawMag[2];
	*ts = mpu.dmpTimestamp;
	*mts = mpu.magTimestamp;
	*x_accel = mpu.calibratedAccel[0];
	*y_accel = mpu.calibratedAccel[1];
	*z_accel = mpu.calibratedAccel[2];
	*x_mag = mpu.calibratedMag[0];
	*y_mag = mpu.calibratedMag[1];
	*z_mag = mpu.calibratedMag[2];
	return ret;
}

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
