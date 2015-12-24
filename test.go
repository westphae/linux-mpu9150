package main

import (
	"./mpu"
	"fmt"
)

func main() {
	i := mpu.InitMPU()
	fmt.Printf("%d\n", i)
	defer mpu.CloseMPU()
	for {
		pitch, roll, heading, err := mpu.ReadMPU()
		if err != nil {
			fmt.Printf("%s\n", err.Error())
			continue
		}
		fmt.Printf("%f, %f, %f\n", pitch, roll, heading)
	}
}
