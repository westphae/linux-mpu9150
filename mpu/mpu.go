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

// Current version.
var PackageVersion = "v0.2b"

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
func ReadMPURaw() (gx, gy, gz, ax, ay, az, mx, my, mz float32, err error) {
	i := int(C.read_mpu_raw(
		(*C.float)(unsafe.Pointer(&gx)),
		(*C.float)(unsafe.Pointer(&gy)),
		(*C.float)(unsafe.Pointer(&gz)),
		(*C.float)(unsafe.Pointer(&ax)),
		(*C.float)(unsafe.Pointer(&ay)),
		(*C.float)(unsafe.Pointer(&az)),
		(*C.float)(unsafe.Pointer(&mx)),
		(*C.float)(unsafe.Pointer(&my)),
		(*C.float)(unsafe.Pointer(&mz))))
	if i == -1 {
		err = errors.New("error reading MPU")
	}
	return
}
