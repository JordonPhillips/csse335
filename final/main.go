package main

import (
	"flag"
	"fmt"
	"log"
	"runtime"
)

func main() {
	var cores int
	flag.IntVar(&cores, "cores", 1, "Number of CPU cores to use.")
	flag.Parse()

	nCPU := runtime.NumCPU()
	if cores > nCPU {
		log.Fatalf("Number of cores selected (%d) exceeds the number of cores"+
			" available (%d).\n", cores, nCPU)
	}
	fmt.Printf("You have selected %d of %d virtual cores\n", cores, nCPU)
}
