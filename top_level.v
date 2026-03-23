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
	output			VGA_CLK

);

   sdram_pll pll0 (
		.inclk0(CLOCK_50),
		.c0(DRAM_CLK)
   );
	 
	vga_pll pll1 (
		.inclk0(CLOCK_50),
		.c0(VGA_CLK)
	);

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
endmodule
