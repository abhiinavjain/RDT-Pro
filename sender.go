package main

import (
	"log"
	"net"
	"os"
)

func main() {
	if len(os.Args) != 3 {
		log.Fatal("Illegal commad written ", os.Args[0])

	}

	recvAddr := os.Args[1]
	filePath := os.Args[2]

	log.Println("Sender starting")
	log.Printf("Sending %s to %s", filePath, recvAddr)

	rAdd, err := net.ResolveUDPAddr("udp", recvAddr)
	errcheck(err)

	conn, err := net.ListenPacket("udp", ":0")

	errcheck(err)
	defer conn.Close()

	log.Printf("sender listening on %s", conn.LocalAddr())

	file, err := os.Open(filePath)
	errcheck(err)

	defer file.Close()

	buffer := make([]byte, 1024)

	log.Printf("Starting file transfer")

	for {
		n, err := file.Read(buffer)

		if err != nil {
			if err.Error() == "EOF" {
				log.Println("Reached end of file")
				break
			}

			log.Fatalf("Error reading file: %s", err)

		}

		_, err = conn.WriteTo(buffer[:n], rAdd)
		errcheck(err)

	}

	log.Printf("File transfer complete")

}

func errcheck(err error) {
	if err != nil {
		log.Fatal("Error: %s", err)
	}
}
