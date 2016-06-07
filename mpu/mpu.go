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
	Gx, Gy, Gz, Ax, Ay, Az, Mx, My, Mz float32
}

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
func ReadMPURaw() (d RawData, err error) {
	i := int(C.read_mpu_raw(
		(*C.float)(unsafe.Pointer(&d.Gx)),
		(*C.float)(unsafe.Pointer(&d.Gy)),
		(*C.float)(unsafe.Pointer(&d.Gz)),
		(*C.float)(unsafe.Pointer(&d.Ax)),
		(*C.float)(unsafe.Pointer(&d.Ay)),
		(*C.float)(unsafe.Pointer(&d.Az)),
		(*C.float)(unsafe.Pointer(&d.Mx)),
		(*C.float)(unsafe.Pointer(&d.My)),
		(*C.float)(unsafe.Pointer(&d.Mz))))
	if i == -1 {
		err = errors.New("error reading MPU")
	}
	return
}
