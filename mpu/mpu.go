// Copyright (c) 2015 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

// Package mpu wraps libmpu, an AHRS.
//
// Build example
//
// libmpu.so:
//   $ make -f Makefile-native-shared
//
// mpu go wrapper:
//   $ go build -o mpu.a mpu.go
//

package mpu

/*
#cgo linux LDFLAGS: -limu -lm
#cgo darwin LDFLAGS: -limu -lm
#cgo windows CFLAGS: -IC:/WINDOWS/system32
#cgo windows LDFLAGS: -L. -limu -LC:/WINDOWS/system32

#include <stdlib.h>
#include <stdint.h>
#include "../libimu.h"
*/
import "C"
import (
	"errors"
	"unsafe"
)

// Raw data struct
type RawData struct {
	Gx, Gy, Gz, Ax, Ay, Az, Mx, My, Mz int16
}

type IMUData struct {
	Pitch, Roll, Heading float32
	Gx, Gy, Gz, Ax, Ay, Az int16
	Qx, Qy, Qz, Qw int32
	Mx, My, Mz int16
	Ts, Tsm uint32
	X_accel, Y_accel, Z_accel, X_mag, Y_mag, Z_mag int16
}

// Current version.
var PackageVersion = "v0.2c"

// InitMPU
func InitMPU(sample_rate, yaw_mix_factor int) int {
	return int(C.init_mpu(C.int(sample_rate), C.int(yaw_mix_factor)))
}

func EnableFusion() int {
	return int(C.enableFusion())
}

func DisableFusion() int {
	return int(C.disableFusion())
}

// CloseMPU
func CloseMPU() {
	C.close_mpu()
}

// ReadMPU
func ReadMPU() (pitch, roll, heading float32, err error) {
	i := int(C.read_mpu((*C.float)(unsafe.Pointer(&pitch)),
		(*C.float)(unsafe.Pointer(&roll)),
		(*C.float)(unsafe.Pointer(&heading))))
	if i == -1 {
		err = errors.New("error reading MPU")
	}
	return
}

// ReadMPURaw
func ReadMPURaw() (d RawData, err error) {
	i := int(C.read_mpu_raw(
		(*C.short)(unsafe.Pointer(&d.Gx)),
		(*C.short)(unsafe.Pointer(&d.Gy)),
		(*C.short)(unsafe.Pointer(&d.Gz)),
		(*C.short)(unsafe.Pointer(&d.Ax)),
		(*C.short)(unsafe.Pointer(&d.Ay)),
		(*C.short)(unsafe.Pointer(&d.Az)),
		(*C.short)(unsafe.Pointer(&d.Mx)),
		(*C.short)(unsafe.Pointer(&d.My)),
		(*C.short)(unsafe.Pointer(&d.Mz))))
	if i == -1 {
		err = errors.New("error reading MPU")
	}
	return
}

func ReadMPUAll() (d IMUData, err error) {
	i := int(C.read_mpu_all(
		(*C.float)(unsafe.Pointer(&d.Pitch)),
		(*C.float)(unsafe.Pointer(&d.Roll)),
		(*C.float)(unsafe.Pointer(&d.Heading)),
		(*C.short)(unsafe.Pointer(&d.Gx)),
		(*C.short)(unsafe.Pointer(&d.Gy)),
		(*C.short)(unsafe.Pointer(&d.Gz)),
		(*C.short)(unsafe.Pointer(&d.Ax)),
		(*C.short)(unsafe.Pointer(&d.Ay)),
		(*C.short)(unsafe.Pointer(&d.Az)),
		(*C.long)(unsafe.Pointer(&d.Qx)),
		(*C.long)(unsafe.Pointer(&d.Qy)),
		(*C.long)(unsafe.Pointer(&d.Qz)),
		(*C.long)(unsafe.Pointer(&d.Qw)),
		(*C.short)(unsafe.Pointer(&d.Mx)),
		(*C.short)(unsafe.Pointer(&d.My)),
		(*C.short)(unsafe.Pointer(&d.Mz)),
		(*C.ulong)(unsafe.Pointer(&d.Ts)),
		(*C.ulong)(unsafe.Pointer(&d.Tsm)),
		(*C.short)(unsafe.Pointer(&d.X_accel)),
		(*C.short)(unsafe.Pointer(&d.Y_accel)),
		(*C.short)(unsafe.Pointer(&d.Z_accel)),
		(*C.short)(unsafe.Pointer(&d.X_mag)),
		(*C.short)(unsafe.Pointer(&d.Y_mag)),
		(*C.short)(unsafe.Pointer(&d.Z_mag))))
	if i == -1 {
		err = errors.New("error reading all data from MPU")
	}
	return
}
