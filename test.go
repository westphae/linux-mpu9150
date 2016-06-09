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
	heading float32
	pitch   float32
	roll    float32
	gx	float32
	gy	float32
	gz	float32
	ax	float32
	ay	float32
	az	float32
	qx	float32
	qy	float32
	qz	float32
	qw	float32
	mx	float32
	my	float32
	mz	float32
	ts 	uint32
	tsm 	uint32
	x_accel float32
	y_accel float32
	z_accel float32
	x_mag float32
	y_mag float32
	z_mag float32
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
		s := fmt.Sprintf("XATTtesting,%f,%f,%f",
			u.heading, u.pitch, u.roll)
		fmt.Printf("%s\n", s)
		fmt.Printf("gyro: %+6.4f %+6.4f %+6.4f\n",
			u.gx, u.gy, u.gz)
		fmt.Printf("acc:  %+6.4f %+6.4f %+6.4f\n",
			u.ax, u.ay, u.az)
		fmt.Printf("quat: %+6.4f %+6.4f %+6.4f %+6.4f\n",
			u.qx, u.qy, u.qz, u.qw)
		fmt.Printf("mag:  %+6.4f %+6.4f %+6.4f\n",
			u.mx, u.my, u.mz)
		fmt.Printf("ts:   %d %d\n", u.ts, u.tsm)
		fmt.Printf("cacc: %+6.4f %+6.4f %+6.4f\n",
			u.x_accel, u.y_accel, u.z_accel)
		fmt.Printf("magc: %+6.4f %+6.4f %+6.4f\n",
			u.x_mag, u.y_mag, u.z_mag)
		fmt.Printf("\n")
		conn.Write([]byte(s))
	}
}

// Converts from radian to degrees and sends to update channel.
func sendUpdate(d mpu.IMUData) {
	var u SituationUpdate
	u.pitch = float32(d.Pitch*180.0/math.Pi)
	u.roll = float32(d.Roll*180.0/math.Pi)
	u.heading = float32(d.Heading*180.0/math.Pi)
	u.gx = float32(d.Gx)/32767
	u.gy = float32(d.Gy)/32767
	u.gz = float32(d.Gz)/32767
	u.ax = float32(d.Ax)/32767
	u.ay = float32(d.Ay)/32767
	u.az = float32(d.Az)/32767
	u.qx = float32(d.Qx)/2147483647
	u.qy = float32(d.Qy)/2147483647
	u.qz = float32(d.Qz)/2147483647
	u.qw = float32(d.Qw)/2147483647
	u.mx = float32(d.Mx)/32767
	u.my = float32(d.My)/32767
	u.mz = float32(d.Mz)/32767
	u.ts = d.Ts
	u.tsm = d.Tsm
	u.x_accel = float32(d.X_accel)/32767
	u.y_accel = float32(d.Y_accel)/32767
	u.z_accel = float32(d.Z_accel)/32767
	u.x_mag = float32(d.X_mag)/32767
	u.y_mag = float32(d.Y_mag)/32767
	u.z_mag = float32(d.Z_mag)/32767
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
		d, err_r := mpu.ReadMPUAll();
		if err_r == nil {
			//			fmt.Printf("%s\n", err.Error())
			//			time.Sleep(1 * time.Second)
			//			continue
			sendUpdate(d)
		}
		time.Sleep(98 * time.Millisecond)
	}
}
