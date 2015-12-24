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
var PackageVersion = "v0.1"

// InitMPU
func InitMPU() int {
	return int(C.init_mpu())
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
