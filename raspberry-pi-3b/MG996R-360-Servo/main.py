from gpiozero import Servo
from time import sleep

servo = Servo(pin=21, initial_value=0.5/1000, min_pulse_width=0.5/1000, max_pulse_width=2.5/1000, frame_width=20.0/1000, pin_factory=None )

print("left turn slow")
servo.value = 0.5
sleep(5)

print("left turn")
servo.value = 1
sleep(5)

print("stop")
servo.value = None
sleep(5)

print("right turn slow")
servo.value = -0.5
sleep(5)

print("right turn")
servo.value = -1
sleep(5)

print("stop")
servo.value = None
sleep(5)
