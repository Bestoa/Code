//`define UART_1

module uart_test (
    input clk,
    input rst_n,
`ifdef UART_1
    output ser_tx1,
`endif
    input ser_rx,
    output ser_tx
);

    reg [7:0] uart_tx_data;
    reg [31:0] counter;
    reg uart_tx_valid;
    wire uart_tx_ready;


`ifdef UART_1
    assign ser_tx1 = ser_tx;
`endif

    uart_tx uart_tx_inst (
        .uart_clk(clk),
        .uart_rst_n(rst_n),
        .uart_tx_valid(uart_tx_valid),
        .uart_tx_data(uart_tx_data),
        .uart_divider(233), // 27M / 115200 - 1
        .uart_ser_tx(ser_tx),
        .uart_tx_ready(uart_tx_ready)
    );

    wire [7:0] uart_rx_data;
    reg uart_rx_ready;
    wire uart_rx_valid;

    uart_rx uart_rx_inst (
        .uart_clk(clk),
        .uart_rst_n(rst_n),
        .uart_divider(233), // 27M / 115200 - 1
        .uart_ser_rx(ser_rx),
        .uart_rx_ready(uart_rx_ready),
        .uart_rx_data(uart_rx_data),
        .uart_rx_valid(uart_rx_valid)
    );

    reg [15:0] index;

    localparam STRING_SIZE = 15;
    wire [STRING_SIZE*8-1:0] string = {"hello, world!", 16'h0d0a };

    localparam NUMBER_SIZE = 16;
    wire [NUMBER_SIZE*8-1:0] number = "FEDCBA9876543210";

    reg is_string;

    reg [31:0] print_counter;

    always @(posedge clk) begin
        if (!rst_n) begin
            counter <= 0;
            is_string <= 0;
            print_counter <= 1;
        end else begin
            counter <= counter + 1;
            if (counter == 27_000_000) begin
                counter <= 0;
                is_string <= ~is_string;
                if (is_string)
                    print_counter = print_counter + 1;
            end
        end
    end

    always @(posedge clk) begin
        if (!rst_n) begin
            counter <= 0;
            is_string <= 0;
            print_counter <= 1;
        end else begin
            counter <= counter + 1;
            if (counter == 27_000_000) begin
                counter <= 0;
                is_string <= ~is_string;
                if (is_string) begin
                    print_counter = print_counter + 1;
                end
            end
        end
    end

    reg echo_mode = 0;
    reg [4:0] index_end;

    always @(posedge clk) begin
        if (!rst_n) begin
            uart_tx_valid <= 0;
        end else begin
            if (counter == 0) begin
                uart_tx_valid <= 1;
                index <= 0;
                echo_mode <= 0;
            end else if (echo_mode == 0) begin
                if (uart_tx_ready) begin
                    if (index < index_end) begin
                        index <= index + 1;
                    end else begin
                        echo_mode <= 1;
                        uart_rx_ready <= 0;
                        uart_tx_valid <= 0;
                    end
                end
            end else begin
                if (uart_rx_valid) begin
                    uart_tx_valid <= 1;
                    uart_rx_ready <= 1;
                end
                if (uart_tx_ready) begin
                    uart_tx_valid <= 0;
                end
            end
        end
    end

    always @(*) begin
        if (echo_mode == 0) begin
            if (is_string) begin
                index_end = STRING_SIZE-1;
                uart_tx_data = string[(index_end-index)*8+:8];
            end else begin
                index_end = 8; // add 1 bytes for space
                uart_tx_data = print_counter_hex[(index_end-index)*8+:8];
            end
        end else begin
            uart_tx_data = uart_rx_data;
            index_end = 0;
        end
    end


    reg [9*8-1:0] print_counter_hex;

    integer i, j;
    always @(*) begin
        for (i = 1; i < 9; i= i + 1) begin
            j = print_counter[4*(i-1)+:4];
            print_counter_hex[8*i+:8]  = number[8*j+:8];
        end
        print_counter_hex[0+:8] = 8'h20; // space
    end
endmodule
