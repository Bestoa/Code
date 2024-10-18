all: sbt yosys nextpnr pack

sbt:
	sbt run

yosys:
	yosys -p "read_verilog hw/gen/MemDemo.v; synth_ecp5; write_json mem.json"

nextpnr:
	nextpnr-ecp5 --textcfg mem.config --25k --package CABGA256 --speed 6 --json mem.json --lpf mem.lpf --freq 24

pack:
	ecppack mem.config mem.bit

burn:
	openFPGALoader mem.bit

clean:
	rm -rf hw/gen/* mem.json mem.config mem.bit

