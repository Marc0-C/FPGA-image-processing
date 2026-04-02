module top_level (
	// ---- Generic IO ---- //
	input 			CLOCK_50,
	input		[9:0] SW,
	input		[2:2] GPIO,
	input		[1:0] KEY,
	output	[9:0] LEDR,
	output	[7:0] HEX0,
	output	[7:0] HEX1,
	output	[7:0] HEX2,
	output	[7:0] HEX3,
	output	[7:0] HEX4,
	output	[7:0] HEX5,
	
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
	wire [3:0] pixel_buff_out;
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

	
    nios_system u0 (
        .cam_ready_export_export (GPIO[2]), // cam_ready_export.export
        .clk_clk                 (CLOCK_50),                 //              clk.clk
        .hex1_export_export      ({HEX2,HEX1,HEX1}),      //      hex1_export.export
        .hex2_export_export      ({HEX5,HEX4,HEX3}),      //      hex2_export.export
        .key_export_export       (KEY[1]),       //       key_export.export
        .ledr_export_export      (LEDR),      //      ledr_export.export
        .pix_data_export_export  (pixel_data),  //  pix_data_export.export
        .reset_reset_n           (KEY[0]),           //            reset.reset_n
        .sdram_addr       			(DRAM_ADDR),
		  .sdram_ba        			(DRAM_BA),      			
		  .sdram_cas_n      			(DRAM_CAS_N),
		  .sdram_cke        			(DRAM_CKE),
		  .sdram_cs_n       			(DRAM_CS_N),
		  .sdram_dq         			(DRAM_DQ),
		  .sdram_dqm        			({DRAM_UDQM, DRAM_LDQM}),
		  .sdram_ras_n      			(DRAM_RAS_N),
		  .sdram_we_n       			(DRAM_WE_N), 
		  .sw_export_export        (SW)         //        sw_export.export
    );

	 vga_controller controller (
		.VGA_CLK					(VGA_CLK),
		.VGA_DATA				(pixel_buff_out),
		.VGA_ADDR				(vga_adress),
		.VGA_R					(VGA_R),
		.VGA_G					(VGA_G),
		.VGA_B					(VGA_B),
		.VGA_HS					(VGA_HS),
		.VGA_VS					(VGA_VS)
	 );
	 
	 pixel_buffa (
		.clock					(VGA_CLK),
		.data						(pixel_data),
		.rdaddress				(vga_adress),
		.wraddress				(buffer_wadrr),
		.wren						(1'b1),
		.q							(pixel_buff_out)
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
	*/
	 
	 
	 

endmodule
