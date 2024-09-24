all: uart_test.fs

uart_test.json: uart_tx.v uart_rx.v uart_test.v
	yosys -p "read_verilog uart_tx.v uart_rx.v uart_test.v; opt; synth_gowin -json uart_test.json"

uart_test-pnr.json: uart_test.json
	nextpnr-himbaechel  --device GW1NR-LV9QN88PC6/I5 --vopt family=GW1N-9C  --json $^ --write $@ --vopt cst=./top.cst --vopt freq=27

uart_test.fs: uart_test-pnr.json
	gowin_pack -d GW1N-9C -o $@ $^

clean:
	rm -rf *.json
	rm -rf *.fs
