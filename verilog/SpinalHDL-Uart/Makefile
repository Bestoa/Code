all: sbt yosys nextpnr pack

target=uart

sbt:
	sbt run

yosys:
	yosys -p "read_verilog hw/gen/UartCtrlUsageExample.v; synth_ecp5; write_json uart.json"

nextpnr:
	nextpnr-ecp5 --textcfg $(target).config --25k --package CABGA256 --speed 6 --json $(target).json --lpf $(target).lpf --freq 24

pack:
	ecppack $(target).config $(target).bit

burn:
	openFPGALoader $(target).bit

clean:
	rm -rf hw/gen/* $(target).json $(target).config $(target).bit

