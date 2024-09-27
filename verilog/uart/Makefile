#DEVICE=GW1NR-LV9QN88PC6/I5
#FAMILY=GW1N-9C
#CST=./top.cst

DEIVCE=GW2A-LV18PG256C8/I7
FAMILY=GW2A-18
CST=./top-20k.cst

all: uart_test.fs

uart_test.json: uart_tx.v uart_rx.v uart_test.v
	yosys -p "read_verilog uart_tx.v uart_rx.v uart_test.v; opt; synth_gowin -json uart_test.json"

uart_test-pnr.json: uart_test.json
	nextpnr-himbaechel  --device ${DEIVCE} --vopt family=${FAMILY}  --json $^ --write $@ --vopt cst=${CST} --vopt freq=27

uart_test.fs: uart_test-pnr.json
	gowin_pack -d ${FAMILY} -o $@ $^

clean:
	rm -rf *.json
	rm -rf *.fs
