module uart_tx (
    input uart_clk,
    input uart_rst_n,
    input uart_tx_start,
    input [7:0] uart_tx_data,
    input [15:0] uart_divider,
    output uart_ser_tx,
    output uart_tx_busy
);
    reg [9:0] send_pattern;
    assign uart_ser_tx = send_pattern[0];

    reg [15:0] send_divider;
    reg [10:0] send_bitcnt;

    reg [3:0] send_state;

    localparam SEND_WAIT = 0;
    localparam SEND = 1;
    localparam IDLE = 2;
    
    assign uart_tx_busy = send_state != IDLE;

    reg [7:0] latched_tx_data;

    always @(posedge uart_clk or negedge uart_rst_n) begin
        if (~uart_rst_n) begin
            send_state <= IDLE;
            send_bitcnt <= 0;
            send_pattern <= 10'b11_1111_1111;
            send_divider <= 0;
            latched_tx_data <= 0;
        end else begin
            send_divider <= send_divider + 1;
            if (send_divider == uart_divider) begin
                send_divider <= 0;
            end
            case (send_state)
                SEND_WAIT: begin
                    if (send_divider == uart_divider) begin
                        send_pattern <= {1'b1, latched_tx_data, 1'b0};
                        send_state <= SEND;
                    end
                end
                SEND: begin
                    if (send_divider == uart_divider) begin
                        send_pattern <= {1'b1, send_pattern[9:1]};
                        send_bitcnt <= send_bitcnt + 1;
                        if (send_bitcnt == 9) begin
                            send_state <= IDLE;
                        end
                    end
                end
                IDLE: begin
                    if (uart_tx_start) begin
                        send_state <= SEND_WAIT;
                        send_bitcnt <= 0;
                        latched_tx_data <= uart_tx_data;
                    end
                end
            endcase
        end
    end
endmodule
