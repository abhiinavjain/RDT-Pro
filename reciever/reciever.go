package main

import (
	"log"
	"net"
	"os"
)

func errcheck(err error) {
	if err != nil {
		log.Fatalf("Error : %s", err)
	}
}

func main() {
	if len(os.Args) != 3 {
		log.Fatalf("Usage: %s <listen_port> <output_file>", os.Args[0])
	}

	listenPort := os.Args[1]
	outputFile := os.Args[2]

	listenAddr := ":" + listenPort

	log.Printf("REciever Starting")
	log.Printf("Listening on %s, writing to %s", listenAddr, outputFile)

	lAddr, err := net.ResolveUDPAddr("udp", listenAddr)
	errcheck(err)

	conn, err := net.ListenUDP("udp", lAddr)
	errcheck(err)

	defer conn.Close()
	log.Printf("Successfully listening on %s", conn.LocalAddr())

	file, err := os.Create(outputFile)
	errcheck(err)

	defer file.Close()

	buffer := make([]byte, 1024)

	log.Println("Waiting for data")

	for {
		n, addr, err := conn.ReadFromUDP(buffer)

		errcheck(err)

		log.Printf("Recieved %d bytes from %s", n, addr.String())

		_, err = file.Write(buffer[:n])

		errcheck(err)

	}

}
