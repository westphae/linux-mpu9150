package main

import (
	"./mpu"
	"fmt"
	"math"
	"net"
	"os"
	"time"
)

type SituationUpdate struct {
	pitch   float64
	roll    float64
	heading float64
}

var updateChan chan SituationUpdate

func updateSender(addr string) {
	updateChan = make(chan SituationUpdate)

	ipAndPort := addr + ":49002"
	udpaddr, err := net.ResolveUDPAddr("udp", ipAndPort)
	if err != nil {
		fmt.Printf("ResolveUDPAddr(%s): %s\n", ipAndPort, err.Error())
		return
	}

	conn, err := net.DialUDP("udp", nil, udpaddr)
	if err != nil {
		fmt.Printf("DialUDP(%s): %s\n", ipAndPort, err.Error())
		return
	}

	// Get updates from the channel, send.
	for {
		u := <-updateChan
		s := fmt.Sprintf("XATTtesting,%f,%f,%f", u.heading, u.pitch, u.roll)
		fmt.Printf("%s\n", s)
		conn.Write([]byte(s))
	}
}

// Converts from radian to degrees and sends to update channel.
func sendUpdate(pitch, roll, heading float32) {
	var u SituationUpdate
	u.pitch = float64(pitch) * (float64(180.0) / math.Pi)
	u.roll = float64(roll) * (float64(180.0) / math.Pi)
	u.heading = float64(heading) * (float64(180.0) / math.Pi)
	updateChan <- u
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("%s <ip>\n", os.Args[0])
		return
	}
	go updateSender(os.Args[1])

	mpu.InitMPU()
	defer mpu.CloseMPU()
	time.Sleep(98 * time.Millisecond)
	for {
		pitch, roll, heading, err := mpu.ReadMPU()
		if err == nil {
			//			fmt.Printf("%s\n", err.Error())
			//			time.Sleep(1 * time.Second)
			//			continue
			fmt.Printf("%f, %f, %f\n", pitch, roll, heading)
			sendUpdate(pitch, roll, heading)
		}
		time.Sleep(98 * time.Millisecond)
	}
}
