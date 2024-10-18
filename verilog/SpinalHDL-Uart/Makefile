PROJECT=uart

all: sbt yosys nextpnr pack

sbt:
	sbt run

yosys:
	yosys ${PROJECT}.ys

nextpnr:
	nextpnr-ecp5 --textcfg ${PROJECT}.config --25k --package CABGA256 --speed 6 --json ${PROJECT}.json --lpf ${PROJECT}.lpf --freq 24

pack:
	ecppack ${PROJECT}.config ${PROJECT}.bit

burn:
	openFPGALoader ${PROJECT}.bit

clean:
	rm -rf hw/gen/* *.json *.config *.bit

