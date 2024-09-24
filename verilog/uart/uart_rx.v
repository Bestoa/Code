module uart_rx (
    input uart_clk,
    input uart_rst_n,
    input [15:0] uart_divider,
    input uart_ser_rx,
    input uart_rx_ready_rst,
    output reg [7:0] uart_rx_data,
    output reg uart_rx_ready
);

    reg r0, r1;
    assign flip = r0 == 1 && r1 == 0;

    always @(posedge uart_clk) begin
        if (~uart_rst_n) begin
            r0 <= 1;
            r1 <= 1;
        end else begin
            r0 <= r1;
            r1 <= uart_ser_rx;
        end
    end
    
    localparam IDLE = 0;
    localparam START = 1;
    localparam DATA = 2;
    localparam STOP = 3;

    reg [3:0] receive_state;

    reg [7:0] rx_data;

    reg [15:0] receive_divider;
    reg [3:0] receive_bitcnt;

    always @(posedge uart_clk or negedge uart_rst_n) begin
        if (~uart_rst_n) begin
            receive_state <= IDLE;
            rx_data <= 0;
            uart_rx_ready <= 0;
            receive_divider <= 0;
            receive_bitcnt <= 0;
        end else begin
            if (uart_rx_ready_rst && uart_rx_ready)
                uart_rx_ready <= 0;
            case (receive_state)
                IDLE: begin
                    if (flip) begin
                        receive_state <= START;
                        rx_data <= 0;
                        receive_divider <= 0;
                        uart_rx_ready <= 0;
                    end
                end
                START: begin
                    if (receive_divider * 2 > uart_divider) begin
                        receive_divider <= 0;
                        receive_state <= DATA;
                        receive_bitcnt <= 0;
                    end else begin
                        receive_divider <= receive_divider + 1;
                    end
                end
                DATA: begin
                    if (receive_divider == uart_divider) begin
                        receive_divider <= 0;
                        rx_data <= {uart_ser_rx, rx_data[7:1]};
                        receive_bitcnt <= receive_bitcnt + 1;
                        if (receive_bitcnt == 7) begin
                            receive_state <= STOP;
                            // rx_data will be ready next cycle
                        end
                    end else begin
                        receive_divider <= receive_divider + 1;
                    end
                end
                STOP: begin
                    if (receive_divider == uart_divider) begin
                        receive_divider <= 0;
                        receive_state <= IDLE;
                        // rx_data is ready
                        uart_rx_ready <= 1;
                        uart_rx_data <= rx_data;
                    end else begin
                        receive_divider <= receive_divider + 1;
                    end
                end
            endcase
        end
    end
endmodule
