// microsecond counter
// increments every 1 microsecond based on the 50MHz clock.

module usec_counter (
    input clk,             
    input reset_n,          
    output reg [31:0] usec 
);

    reg [5:0] prescaler;   // 6 bits to count to 49 cycles
	 
    always @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            prescaler <= 6'd0; // set to 0
            usec <= 32'd0;
        end else begin
            if (prescaler == 6'd49) begin // if 49 add one microsecond and reset
                prescaler <= 6'd0;
                usec <= usec + 1'b1;     
            end else begin
                prescaler <= prescaler + 1'b1; // if not 49 keep counting
            end
        end
    end

endmodule