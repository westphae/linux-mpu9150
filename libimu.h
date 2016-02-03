#ifndef LIBIMU_H
#define LIBIMU_H

// To avoid having to pass the same command line switches when running
// the test apps, you can specify the defaults for your platform here.

// RPi I2C bus
#define DEFAULT_I2C_BUS (1)

// platform independent
//#define DEFAULT_SAMPLE_RATE_HZ	(10)
//#define DEFAULT_YAW_MIX_FACTOR (0)

#ifdef __cplusplus
extern "C" {
#endif

extern void close_mpu(void);
//extern int init_mpu(void);
extern int init_mpu(int sample_rate, int yaw_mix_factor);
extern int read_mpu(float *pitch, float *roll, float *heading);
extern int set_cal(int mag, char *cal_file);

extern int enableFusion();
extern int disableFusion();
#ifdef __cplusplus
}
#endif

#endif /* LIBIMU_H */
