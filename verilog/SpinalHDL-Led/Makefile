all: sbt yosys nextpnr ecppack

sbt:
	sbt run

yosys:
	yosys -p "read_verilog hw/gen/Led.v; synth_ecp5; write_json led.json"

nextpnr:
	nextpnr-ecp5 --textcfg led.config --25k --package CABGA256 --speed 6 --json led.json --lpf led.lpf --freq 24

ecppack:
	ecppack led.config led.bit

burn:
	openFPGALoader led.bit
