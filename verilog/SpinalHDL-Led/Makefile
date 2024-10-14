all: sbt yosys nextpnr ecppack

sbt:
	sbt run

yosys:
	yosys -p "read_verilog hw/gen/Led.v hw/verilog/pll.v; synth_ecp5; write_json led.json"

nextpnr:
	nextpnr-ecp5 --textcfg led.config --25k --package CABGA256 --speed 6 --json led.json --lpf led.lpf --freq 24

pack:
	ecppack led.config led.bit

burn:
	openFPGALoader led.bit

clean:
	rm -rf hw/gen/* led.json led.config led.bit

