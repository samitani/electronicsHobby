package main

import (
	"context"
	"machine"
	"sync"
	"time"
)

/**
 * Push the button, then LED become OFF
 */
func main() {
	var wg sync.WaitGroup
	ctx, cancel := context.WithCancel(context.Background())

	wg.Add(1)
	go led(ctx, 1000, machine.GPIO7)
	wg.Add(1)
	go led(ctx, 500, machine.GPIO10)

	pin := machine.GPIO9
	pin.Configure(machine.PinConfig{Mode: machine.PinInput})

	for {
		time.Sleep(time.Millisecond * 100)
		value := pin.Get()
		if !value {
			cancel()
			break
		}
	}

	wg.Wait()

}

func led(ctx context.Context, interval time.Duration, led machine.Pin) {
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})
	for {
		select {
		case <-ctx.Done():
			return
		default:
			led.Low()
			time.Sleep(time.Millisecond * interval)
			led.High()
			time.Sleep(time.Millisecond * interval)
		}
	}
}
