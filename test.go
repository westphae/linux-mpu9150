package main

import (
	"./mpu"
	"fmt"
	"time"
)

func main() {
	i := mpu.InitMPU()
	fmt.Printf("%d\n", i)
	defer mpu.CloseMPU()
	time.Sleep(98 * time.Millisecond)
	for {
		pitch, roll, heading, err := mpu.ReadMPU()
		if err == nil {
			//			fmt.Printf("%s\n", err.Error())
			//			time.Sleep(1 * time.Second)
			//			continue
			fmt.Printf("%f, %f, %f\n", pitch, roll, heading)
		}
		time.Sleep(98 * time.Millisecond)
	}
}
