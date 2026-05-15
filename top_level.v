module top_level (
	// ---- Generic IO ---- //
	input 			CLOCK_50,
	inout 	[35:0] GPIO,
	input		[9:0] SW,
	input		[1:0] KEY,
	output	[9:0] LEDR,
	output  [7:0]   HEX0, HEX1, HEX2, HEX3, HEX4, HEX5,	
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
   output         VGA_VS,
	
	// ---- SPI IO ---- //
	output         GSENSOR_CS_N,	 // chip select
   output         GSENSOR_SCLK,
   output         GSENSOR_SDI,    // Master Out (MOSI)
   input          GSENSOR_SDO,    // Master In (MISO)
   input  [2:1]   GSENSOR_INT     // Interrupt pins 
	
	
);
	wire [3:0] pixel_data_R;
	wire [3:0] pixel_data_G;
	wire [3:0] pixel_data_B;
	wire [3:0] pixel_buff_R_out;
	wire [3:0] pixel_buff_G_out;
	wire [3:0] pixel_buff_B_out;
	wire [16:0] vga_adress;
	wire [31:0] current_usec;
	//wire nios_mosi; //
   //wire nios_sclk; //
	//reg [16:0] buffer_wadrr;
	wire [16:0] buffer_wadrr;
	reg [3:0] pixels_raw; // test input for pixelbuffer "DATA"
	
	assign GPIO[35:10] = 3'bzzz;
	assign GPIO[6:6] = 3'bzzz;
	assign GPIO[4:3] = 3'bzzz;
	assign GPIO[1:0] = 3'bzzz;
	
	wire multiplexed_miso;
	//wire master_miso;   
	wire master_mosi;
	wire master_sclk;
	//wire master_ss_n;   

	
	// only one bit so just assign outputs and multiplex inputs 
	assign GPIO[8] = nios_mosi;
	assign GSENSOR_SDI = nios_mosi;
	assign GPIO[9] = nios_sclk;
	assign GSENSOR_SCLK = nios_sclk;
	
	assign multiplexed_miso = (GSENSOR_CS_N) ? GPIO[7] : GSENSOR_SDO; //multiplex based on chipselect

	
	wire pio_0_to_1;
	wire pio_1_to_0;
	
   sdram_pll pll0 (
		.inclk0(CLOCK_50),
		.c0(DRAM_CLK)
   );
	 
	vga_pll pll1 (
		.inclk0(CLOCK_50),
		.c0(VGA_CLK)
	);

///////////////////////
  nios_system u0 (
        // --- I/O ---
        .clk_clk                     (CLOCK_50),
        .reset_reset_n               (KEY[0]),
		  
        .sw_export_export            (SW),
        .key_export_export           (KEY[1]),
        .ledr_export_export          (LEDR),
        .hex1_export_export          ({HEX2,HEX1,HEX0}),
        .hex2_export_export          ({HEX5,HEX4,HEX3}),
        
        .sdram_addr                  (DRAM_ADDR),
        .sdram_ba                    (DRAM_BA),
        .sdram_cas_n                 (DRAM_CAS_N),
        .sdram_cke                   (DRAM_CKE),
        .sdram_cs_n                  (DRAM_CS_N),
        .sdram_dq                    (DRAM_DQ),
        .sdram_dqm                   ({DRAM_UDQM, DRAM_LDQM}),
        .sdram_ras_n                 (DRAM_RAS_N),
        .sdram_we_n                  (DRAM_WE_N),
		  //.sdram_clk_clk               (DRAM_CLK),	
		  
        .pix_buff_addr_export_export (buffer_wadrr),
        .pix_data_r_export_export    (pixel_data_R),
		  .pix_data_g_export_export    (pixel_data_G),
        .pix_data_b_export_export    (pixel_data_B),
        .cam_ready_export_export     (GPIO[2]),  

        // --- SPI Connections ---
        .spi_0_external_MISO         (multiplexed_miso),
        .spi_0_external_MOSI         (nios_mosi),
        .spi_0_external_SCLK         (nios_sclk),
        // Bit 0 for Accelerometer, Bit 1 for Camera
        .spi_0_external_SS_n         ({GPIO[5], GSENSOR_CS_N}), 
        //.spi_0_external_SS_n    ({GPIO[10], GSENSOR_CS_N}
		  
		  // --- Counter ---
		  .usec_counter_pio_export_export(current_usec),
		  
		  // --- interporcessor IRQ --
		  .irq_to_proc1_export   (pio_0_to_1),      
		  .irq_from_proc1_export      (pio_1_to_0),     
		  .irq_from_proc0_export      (pio_0_to_1),     
		  .irq_to_proc0_export   (pio_1_to_0)  
		  
    );
///////////////////////
  
 
	 vga_controller controller (
		.VGA_CLK					(VGA_CLK),
		.VGA_DATA_R				(pixel_buff_R_out),
		.VGA_DATA_G				(pixel_buff_G_out),
		.VGA_DATA_B				(pixel_buff_B_out),
		.VGA_ADDR				(vga_adress),
		.VGA_R					(VGA_R),
		.VGA_G					(VGA_G),
		.VGA_B					(VGA_B),
		.VGA_HS					(VGA_HS),
		.VGA_VS					(VGA_VS)
	 );
	 
	 pixel_buffa R(
		.clock					(VGA_CLK),
		.data						(pixel_data_R),
		.rdaddress				(vga_adress),
		.wraddress				(buffer_wadrr),
		.wren						(1'b1),
		.q							(pixel_buff_R_out)
	 );
	
	 pixel_buffa G(
		.clock					(VGA_CLK),
		.data						(pixel_data_G),
		.rdaddress				(vga_adress),
		.wraddress				(buffer_wadrr),
		.wren						(1'b1),
		.q							(pixel_buff_G_out)
	 );
	 
	 pixel_buffa B(
		.clock					(VGA_CLK),
		.data						(pixel_data_B),
		.rdaddress				(vga_adress),
		.wraddress				(buffer_wadrr),
		.wren						(1'b1),
		.q							(pixel_buff_B_out)
	 );
	
	
	usec_counter usec (
		.clk(CLOCK_50),
		.reset_n(KEY[0]),
		.usec(current_usec) 
	);
	
	// compensate for if have issues with firsy column of pixels being offset then can make buffer writing one pixel ahead of vga controller:
	//wire [16:0] rdaddr_latencycomp = (vga_adress == 17'd76799) ? 17'd0 : vga_adress + 17'd1; // probably a better way to do this though
	// also needs ".rdaddress				(vga_adress)," replace vga_address with rdaddr_latencycomp

/*
	always @(posedge VGA_CLK) begin
		if (buffer_wadrr == 17'd76799)
			buffer_wadrr <= 17'd0;
		else
			buffer_wadrr <= buffer_wadrr + 1;

		pixels_raw <= pixels_raw + 1;
	end
*/
	 
	 
	 

endmodule
