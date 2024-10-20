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
    wire uart_rx_valid;

    uart_rx uart_rx_inst (
        .uart_clk(clk),
        .uart_rst_n(rst_n),
        .uart_divider(233), // 27M / 115200 - 1
        .uart_ser_rx(ser_rx),
        .uart_rx_ready(1'b1),
        .uart_rx_data(uart_rx_data),
        .uart_rx_valid(uart_rx_valid)
    );

    wire fifo_empty;
    reg fifo_rd_en;
    wire [7:0] uart_tx_echo_data;

    SimpleFIFO fifo_inst (
        .clk(clk),
        .rst(~rst_n),
        .wr_en(uart_rx_valid),
        .rd_en(fifo_rd_en),
        .data_in(uart_rx_data),
        .data_out(uart_tx_echo_data),
        .full(),
        .empty(fifo_empty)
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

    reg echo_mode;
    reg [4:0] index_end;

    always @(posedge clk) begin
        if (!rst_n) begin
            uart_tx_valid <= 0;
            fifo_rd_en <= 0;
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
                        uart_tx_valid <= 0;
                    end
                end
            end else begin
                if (!fifo_empty && !uart_tx_valid) begin
                    fifo_rd_en <= 1;
                    if (fifo_rd_en) begin
                        uart_tx_valid <= 1;
                        fifo_rd_en <= 0;
                    end
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
            uart_tx_data = uart_tx_echo_data;
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

module SimpleFIFO (
    input wire clk,
    input wire rst,
    input wire wr_en,      // Write enable
    input wire rd_en,      // Read enable
    input wire [7:0] data_in, // Data input
    output reg [7:0] data_out, // Data output
    output reg empty,      // FIFO empty flag
    output reg full        // FIFO full flag
);

    parameter DEPTH = 16;
    reg [7:0] fifo_mem [0:DEPTH-1];
    reg [3:0] rd_ptr, wr_ptr;
    reg [4:0] count;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            rd_ptr <= 0;
            wr_ptr <= 0;
            count <= 0;
            empty <= 1;
            full <= 0;
        end else begin
            if (wr_en) begin
                fifo_mem[wr_ptr] <= data_in;
                wr_ptr <= (wr_ptr + 1) % DEPTH;
                if (!full) begin
                    count <= count + 1;
                end
                if (count == DEPTH - 1) begin
                    full <= 1;
                end
                empty <= 0;
            end

            if (rd_en && !empty) begin
                data_out <= fifo_mem[rd_ptr];
                rd_ptr <= (rd_ptr + 1) % DEPTH;
                count <= count - 1;
                if (count == 1) begin
                    empty <= 1;
                end
                full <= 0;
            end

            // Handle wrap-around and discard
            if (wr_en && full) begin
                rd_ptr <= (rd_ptr + 1) % DEPTH;
            end
        end
    end

endmodule
