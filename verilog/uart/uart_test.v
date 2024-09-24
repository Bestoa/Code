module uart_test (
    input clk,
    input rst_n,
    input ser_rx,
    output ser_tx
);

    reg [7:0] uart_tx_data;
    reg [31:0] counter;
    reg uart_tx_start;

    wire tx_busy;

    uart_tx uart_tx_inst (
        .uart_clk(clk),
        .uart_rst_n(rst_n),
        .uart_tx_start(uart_tx_start),
        .uart_tx_data(uart_tx_data),
        .uart_divider(233), // 27M / 115200 - 1
        .uart_ser_tx(ser_tx),
        .uart_tx_busy(tx_busy)
    );

    wire [7:0] uart_rx_data;
    reg uart_rx_ready_rst;
    wire rx_ready;

    uart_rx uart_rx_inst (
        .uart_clk(clk),
        .uart_rst_n(rst_n),
        .uart_divider(233), // 27M / 115200 - 1
        .uart_ser_rx(ser_rx),
        .uart_rx_ready_rst(uart_rx_ready_rst),
        .uart_rx_data(uart_rx_data),
        .uart_rx_ready(rx_ready)
    );

    reg [15:0] index;

    localparam STRING_SIZE = 15;
    wire [STRING_SIZE*8-1:0] string = {"Hello, world!", 16'h0d0a };

    localparam NUMBER_SIZE = 16;
    wire [NUMBER_SIZE*8-1:0] number = "FEDCBA9876543210";

    reg odd;

    reg [31:0] print_counter;

    always @(posedge clk) begin
        if (!rst_n) begin
            counter <= 0;
            odd <= 0;
            print_counter <= 1;
        end else begin
            counter <= counter + 1;
            if (counter == 27_000_000) begin
                counter <= 0;
                odd <= ~odd;
                if (odd)
                    print_counter = print_counter + 1;
            end
        end
    end

    always @(posedge clk) begin
        if (!rst_n) begin
            counter <= 0;
            odd <= 0;
            print_counter <= 1;
        end else begin
            counter <= counter + 1;
            if (counter == 27_000_000) begin
                counter <= 0;
                odd <= ~odd;
                if (odd) begin
                    print_counter = print_counter + 1;
                end
            end
        end
    end

    reg mode = 0;
    reg [4:0] index_end;
    reg data_latched;

    always @(posedge clk) begin
        if (!rst_n) begin
            uart_tx_start <= 0;
            data_latched <= 0;
        end else begin
            if (counter == 0) begin
                uart_tx_start <= 1;
                index <= 0;
                mode <= 0;
                if (odd) begin
                    uart_tx_data <= string[STRING_SIZE*8-1-:8];
                    index_end <= STRING_SIZE - 1;
                end else begin
                    uart_tx_data <= print_counter_hex[8*8-1-:8];
                    index_end <= 7;
                end
            end else if (mode == 0) begin
                if (uart_tx_start && tx_busy) begin
                    uart_tx_start <= 0;
                    data_latched <= 1;
                end
                if (data_latched && ~tx_busy) begin
                    data_latched <= 0;
                    if (index < index_end) begin
                        index <= index + 1;
                        uart_tx_start <= 1;
                        if (odd)
                            uart_tx_data <= string[(STRING_SIZE-index-1)*8-1-:8];
                        else
                            uart_tx_data <= print_counter_hex[(8-index-1)*8-1-:8];
                    end else begin
                        mode <= 1;
                        uart_rx_ready_rst <= 0;
                    end
                end
            end else begin
                if (rx_ready) begin
                    uart_tx_start <= 1;
                    uart_tx_data <= uart_rx_data;
                    uart_rx_ready_rst <= 1;
                end
                if (uart_tx_start && tx_busy) begin
                    uart_tx_start <= 0;
                end
            end
        end
    end

    reg [8*8-1:0] print_counter_hex;

   integer i, j;
   always @(*) begin
       for (i = 0; i < 8; i= i + 1) begin
           j = print_counter[4*(8-i)-1-:4];
           print_counter_hex[8*(8-i)-1-:8]  = number[j*8+7-:8]; 
       end
   end
endmodule
