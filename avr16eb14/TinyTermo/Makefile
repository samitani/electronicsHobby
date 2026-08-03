setup:
	arduino-cli lib install TM1637 AHT20

build:
	arduino-cli compile --fqbn DxCore:megaavr:avreb:chip=avr16eb14,clock=1internal .

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn DxCore:megaavr:avreb:chip=avr16eb14,clock=1internal --programmer serialupdi

fuse:
	avrdude -c serialupdi -p avr16eb14 -U fuses:r:-:h -P /dev/ttyUSB0
