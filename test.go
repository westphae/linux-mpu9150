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
	p	float64
	q	float64
	r	float64
	x_a	float64
	y_a	float64
	z_a	float64
	mx	float64
	my	float64
	mz	float64
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
		s := fmt.Sprintf("XATTtesting,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
			u.heading, u.pitch, u.roll,
			u.p/32767, u.q/32767, u.r/32767,
			u.x_a/32767, u.y_a/32767, u.z_a/32767,
			u.mx/32767, u.my/32767, u.mz/32767)
		fmt.Printf("%s\n", s)
		conn.Write([]byte(s))
	}
}

// Converts from radian to degrees and sends to update channel.
func sendUpdate(pitch, roll, heading, p, q, r, x_a, y_a, z_a, mx, my, mz float32) {
	var u SituationUpdate
	u.pitch = float64(pitch) * (float64(180.0) / math.Pi)
	u.roll = float64(roll) * (float64(180.0) / math.Pi)
	u.heading = float64(heading) * (float64(180.0) / math.Pi)
	u.p = float64(p)
	u.q = float64(q)
	u.r = float64(r)
	u.x_a = float64(x_a)
	u.y_a = float64(y_a)
	u.z_a = float64(z_a)
	u.mx = float64(mx)
	u.my = float64(my)
	u.mz = float64(mz)
	updateChan <- u
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("%s <ip>\n", os.Args[0])
		return
	}
	go updateSender(os.Args[1])

	mpu.InitMPU(10, 0)
	defer mpu.CloseMPU()
	time.Sleep(98 * time.Millisecond)
	for {
		raw, err_r := mpu.ReadMPURaw();
		if err_r == nil {
			//			fmt.Printf("%s\n", err.Error())
			//			time.Sleep(1 * time.Second)
			//			continue
			sendUpdate(
				// pitch, roll, heading,
				0, 0, 0,
				raw.Gx, raw.Gy, raw.Gz, raw.Ax, raw.Ay, raw.Az, raw.Mx, raw.My, raw.Mz)
		}
		time.Sleep(98 * time.Millisecond)
	}
}
