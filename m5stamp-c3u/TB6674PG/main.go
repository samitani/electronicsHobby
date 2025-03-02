package main

import (
	"machine"
	"time"
)

func main() {
	tb6647pgPinS := machine.GPIO3
	tb6647pgPinS.Configure(machine.PinConfig{Mode: machine.PinOutput})

	tb6647pgPinA := machine.GPIO4
	tb6647pgPinA.Configure(machine.PinConfig{Mode: machine.PinOutput})

	tb6647pgPinB := machine.GPIO5
	tb6647pgPinB.Configure(machine.PinConfig{Mode: machine.PinOutput})

	btn := machine.GPIO9
	btn.Configure(machine.PinConfig{Mode: machine.PinInput})

	speed := 3 * time.Millisecond

	tb6647pgPinA.Low()
	tb6647pgPinB.Low()
	tb6647pgPinS.High()

	for {
		time.Sleep(speed)
		value := btn.Get()

		// Button pushed
		if !value {

			tb6647pgPinA.High()
			tb6647pgPinB.High()
			time.Sleep(speed)

			tb6647pgPinA.Low()
			tb6647pgPinB.High()
			time.Sleep(speed)

			tb6647pgPinA.Low()
			tb6647pgPinB.Low()
			time.Sleep(speed)

			tb6647pgPinA.High()
			tb6647pgPinB.Low()
		}
	}

}
