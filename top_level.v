module top_level (
	// ---- Generic IO ---- //
	input 			CLOCK_50,
	
	// ---- SDRAM IO ---- //
	output	[12:0]	DRAM_ADDR,
	output	[1:0]		DRAM_BA,
	output			DRAM_CAS_N,
	output			DRAM_CKE,
	output			DRAM_CLK,
	output			DRAM_CS_N,
	inout		[15:0]	DRAM_DQ,
	output			DRAM_LDQM,
	output			DRAM_UDQM,
	output			DRAM_RAS_N,
	output			DRAM_WE_N,
	
	// ---- VGA IO ---- //
	output			VGA_CLK,
	output [3:0]   VGA_R,
   output [3:0]   VGA_G,
   output [3:0]   VGA_B,
   output         VGA_HS,
   output         VGA_VS
	
);
	wire [3:0] pixel_data;
	wire [16:0] vga_adress;
	reg [16:0] buffer_wadrr;
	reg [3:0] pixels_raw; // test input for pixelbuffer "DATA"

   sdram_pll pll0 (
		.inclk0(CLOCK_50),
		.c0(DRAM_CLK)
   );
	 
	vga_pll pll1 (
		.inclk0(CLOCK_50),
		.c0(VGA_CLK)
	);

	/*
    nios_system u0 (
		.clk_clk       		(CLOCK_50),
		.sdram_addr    		(DRAM_ADDR),
		.sdram_ba      		(DRAM_BA),      		
		.sdram_cas_n   		(DRAM_CAS_N),
		.sdram_cke     		(DRAM_CKE),
		.sdram_cs_n    		(DRAM_CS_N),
		.sdram_dq      		(DRAM_DQ),
		.sdram_dqm     		({DRAM_UDQM, DRAM_LDQM}),
		.sdram_ras_n   		(DRAM_RAS_N),
		.sdram_we_n    		(DRAM_WE_N)     		
    );
	 */
	 vga_controller controller (
		.VGA_CLK					(VGA_CLK),
		.VGA_DATA				(pixel_data),
		.VGA_ADDR				(vga_adress),
		.VGA_R					(VGA_R),
		.VGA_G					(VGA_G),
		.VGA_B					(VGA_B),
		.VGA_HS					(VGA_HS),
		.VGA_VS					(VGA_VS)
	 );
	 
	 pixel_buffa (
		.clock					(VGA_CLK),
		.data						(pixels_raw),
		.rdaddress				(vga_adress),
		.wraddress				(buffer_wadrr),
		.wren						(1'b1),
		.q							(pixel_data)
	 );
	
	
	// compensate for if have issues with firsy column of pixels being offset then can make buffer writing one pixel ahead of vga controller:
	//wire [16:0] rdaddr_latencycomp = (vga_adress == 17'd76799) ? 17'd0 : vga_adress + 17'd1; // probably a better way to do this though
	// also needs ".rdaddress				(vga_adress)," replace vga_address with rdaddr_latencycomp


	always @(posedge VGA_CLK) begin
		if (buffer_wadrr == 17'd76799)
			buffer_wadrr <= 17'd0;
		else
			buffer_wadrr <= buffer_wadrr + 1;

		pixels_raw <= pixels_raw + 1;
	end
	 
	 
	 

endmodule
