setup:
	arduino-cli lib install TM1637 AHT20

build:
	arduino-cli compile --fqbn DxCore:megaavr:avreb:chip=avr16eb14 .

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn DxCore:megaavr:avreb:chip=avr16eb14 --programmer serialupdi
