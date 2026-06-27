//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ram2ahb(
input  wire           i_clk             ,
input  wire           i_rst_n           ,

input  wire           i_otp_en          ,
input  wire [31:0]    i_otp_addr        ,
output wire           o_otp_vld         ,
output wire [31:0]    o_otp_data        ,

output wire           o_hsel            ,
output wire [31:0]    o_haddr           ,
output wire [2:0]     o_hsize           ,
output wire [1:0]     o_htrans          ,
output wire           o_hwrite          ,
output wire [31:0]    o_hwdata          ,
output wire           o_hready          ,
output wire           o_hmastlock       ,
output wire [2:0]     o_hburst          , 
output wire [3:0]     o_hprot           ,

input  wire [31:0]    i_hrdata          ,
input  wire [1:0]     i_hresp           ,
input  wire           i_hreadyout         
);

reg             otp_vld  ; 
reg [31:0]      otp_data ; 
reg             hsel     ; 
reg             vlde     ; 

wire        otp_vld_next  = vlde & i_hreadyout;
wire [31:0] otp_data_next = i_hrdata ;

wire hsel_next = (i_otp_en & i_hreadyout & ~otp_vld_next) & ~hsel;  

wire vlde_next = (i_otp_en & hsel) | (~i_hreadyout & vlde); 

assign o_hsel           = hsel & i_otp_en;
assign o_haddr          = i_otp_addr;
assign o_hsize          = 3'h2;
assign o_hburst         = 3'h0;   
assign o_hprot          = 4'h0;   
assign o_hmastlock      = 1'h0;
assign o_htrans         = {o_hsel,1'h0};
assign o_hwrite         = 1'h0;
assign o_hwdata         = 32'h0;
assign o_hready         = i_hreadyout;

assign o_otp_vld  = otp_vld ;
assign o_otp_data = otp_data;

always @(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        otp_vld       <= 1'h0          ; 
        otp_data      <= 32'h0         ; 
        hsel          <= 1'h0          ; 
        vlde          <= 1'h0          ; 
    end else begin
        otp_vld       <= otp_vld_next  ; 
        otp_data      <= otp_data_next ; 
        hsel          <= hsel_next     ; 
        vlde          <= vlde_next     ; 
    end
end

endmodule
