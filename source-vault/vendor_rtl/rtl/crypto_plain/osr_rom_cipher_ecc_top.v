//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_rom_cipher_ecc_top #(
    parameter  P_ROM_AW         = 17    ,
    parameter  P_ROM_DW         = 32    ,
    parameter  P_ROM_CIPHER_EN  = 1     ,
    parameter  P_ROM_CIPHER_RN  = 2     ,
    parameter  P_ROM_ECC_EN     = 1     ,
    parameter  P_ROM_ECC_TM_CHB = (P_ROM_DW == 32) ? 7 : (P_ROM_DW == 64) ? 8 : (P_ROM_DW == 128) ? 9 : 10,
    parameter  P_ROM_ECC_DW     = (P_ROM_ECC_EN == 0)? P_ROM_DW : (P_ROM_DW == 32) ? 7+32 : (P_ROM_DW == 64) ? 8+64 : (P_ROM_DW == 128) ? 9+128 : 10+256 
    )(

    input  wire                           i_hclk              ,
    input  wire                           i_hresetn           ,

    input  wire [7:0]                     i_ecc_ck_en         , 
    input  wire [7:0]                     i_ecc_err_rsp_en    , 
    input  wire [7:0]                     i_ecc_tm_en         , 
    input  wire [P_ROM_ECC_TM_CHB-1:0]    i_ecc_tm_ckbits     , 

    output wire                           o_ecc_dec_sec       ,
    output wire                           o_ecc_dec_ded       ,
    output wire [P_ROM_AW-3:0]            o_ecc_err_addr      ,

    output wire                           o_ecc_dec_sec_nd    ,
    output wire                           o_ecc_dec_ded_nd    ,

    input  wire                           i_p_rom_cs          , 
    input  wire [P_ROM_AW-1:0]            i_p_rom_addr        , 
    output wire [P_ROM_DW-1:0]            o_p_rom_rdata       , 

    output wire                           o_c_rom_cs          , 
    output wire [P_ROM_AW-3:0]            o_c_rom_addr        , 
    input  wire [P_ROM_ECC_DW-1:0]        i_c_rom_rdata          

);

localparam     CIPHER_ROM_p_AHB_ADDR_WIDTH   = P_ROM_AW;
localparam     CIPHER_ROM_p_AHB_DATA_WIDTH   = P_ROM_DW;
localparam     CIPHER_ROM_p_ADDR_BLOCK_WIDTH = 5       ;

localparam     ECC_DEC_WIDTH_DOUT = (!P_ROM_ECC_EN) ? P_ROM_DW + 17 : (P_ROM_DW == 32) ? 49 : (P_ROM_DW == 64) ? 81 : P_ROM_DW;
localparam     ECC_DEC_WIDTH_DIN  = (!P_ROM_ECC_EN) ? P_ROM_DW + 17 :(ECC_DEC_WIDTH_DOUT == 32) ? 39 : (ECC_DEC_WIDTH_DOUT == 49) ? 56  : (ECC_DEC_WIDTH_DOUT == 64) ? 72 : (ECC_DEC_WIDTH_DOUT == 81) ? 89 : (ECC_DEC_WIDTH_DOUT == 128) ? 137 : 266;

wire [7:0]                     ecc_err_rsp_en    = i_ecc_err_rsp_en;

wire [ECC_DEC_WIDTH_DOUT-1:0]  decode_dout;

reg [P_ROM_AW-3:0]                    ecc_dec_addr                     ; 
reg [P_ROM_AW-3:0]                    p_rom_addr                       ; 
reg [P_ROM_AW-3:0]                    p_rom_addr_buf                   ; 
reg [CIPHER_ROM_p_AHB_DATA_WIDTH-1:0] cipher_bus_hrdata                ; 
reg                                   rd_flg_vail                      ; 
reg                                   rd_flg_vail_buf                  ; 
reg                                   ecc_dec_sec                      ; 
reg                                   ecc_dec_ded                      ; 

wire [CIPHER_ROM_p_AHB_DATA_WIDTH-1:0]    cipher_rom_o_bus_hrdata     = rd_flg_vail ? decode_dout[P_ROM_DW+17-1:17] : {{CIPHER_ROM_p_AHB_DATA_WIDTH}{1'b0}}; 
wire [CIPHER_ROM_p_AHB_ADDR_WIDTH-1:0]    cipher_rom_o_mem_haddr      = i_p_rom_cs ? i_p_rom_addr : {{CIPHER_ROM_p_AHB_ADDR_WIDTH}{1'b0}};

wire                             ecc_dec_ecc_sec     ;
wire                             ecc_dec_ecc_ded     ;
wire [ECC_DEC_WIDTH_DOUT-1:0]    ecc_dec_decode_dout ;

assign decode_dout = ecc_dec_decode_dout;

wire[P_ROM_AW-3:0] ecc_dec_addr_next = cipher_rom_o_mem_haddr[P_ROM_AW-1:2];
wire[P_ROM_AW-3:0] p_rom_addr_next = i_p_rom_addr[P_ROM_AW-1:2];
wire[P_ROM_AW-3:0] p_rom_addr_buf_next = p_rom_addr;
wire [CIPHER_ROM_p_AHB_DATA_WIDTH-1:0] cipher_bus_hrdata_next = cipher_rom_o_bus_hrdata;

wire rd_flg_vail_next = i_p_rom_cs;
wire rd_flg_vail_buf_next = rd_flg_vail;
wire ecc_dec_sec_next = ecc_dec_ecc_sec;
wire ecc_dec_ded_next = ecc_dec_ecc_ded;

wire [ECC_DEC_WIDTH_DIN-1:0]     ecc_dec_decode_din  = rd_flg_vail ? {i_c_rom_rdata,{{{17-{P_ROM_AW-2}}{1'b0}},ecc_dec_addr}} : {{ECC_DEC_WIDTH_DIN}{1'b0}} ;   
wire [7:0]                       ecc_dec_ecc_en      = i_ecc_ck_en; 
wire [ECC_DEC_WIDTH_DIN-1:0]     ecc_dec_xor_dedin   ={ECC_DEC_WIDTH_DIN{1'b0}};

assign o_ecc_dec_sec       = ecc_dec_sec & rd_flg_vail_buf;
assign o_ecc_dec_ded       = ecc_dec_ded & rd_flg_vail_buf;
assign o_ecc_err_addr      = p_rom_addr_buf;
assign o_ecc_dec_sec_nd    = ecc_dec_ecc_sec & rd_flg_vail;
assign o_ecc_dec_ded_nd    = ecc_dec_ecc_ded & rd_flg_vail;
assign o_p_rom_rdata       = rd_flg_vail ? cipher_rom_o_bus_hrdata : (ecc_dec_ded && rd_flg_vail_buf && (ecc_err_rsp_en != 8'h5A)) ? cipher_bus_hrdata : {{P_ROM_DW}{1'b0}}; 
assign o_c_rom_cs          = i_p_rom_cs;
assign o_c_rom_addr        = i_p_rom_cs ? cipher_rom_o_mem_haddr[P_ROM_AW-1:2] : {{P_ROM_AW-2}{1'b0}} ;

osr_ecc_decode #(
    .WIDTH_DOUT     ( ECC_DEC_WIDTH_DOUT ), 
    .WIDTH_DIN      ( ECC_DEC_WIDTH_DIN  )  
    ) u_osr_ecc_decode (
    .decode_din         ( ecc_dec_decode_din  ), 
    .ecc_en             ( ecc_dec_ecc_en      ), 
    .xor_dedin          ( ecc_dec_xor_dedin   ), 
    .ecc_sec            ( ecc_dec_ecc_sec     ), 
    .ecc_ded            ( ecc_dec_ecc_ded     ), 
    .decode_dout        ( ecc_dec_decode_dout )  
    ); 

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        ecc_dec_addr                          <= {{P_ROM_AW-2}{1'b0}}                  ; 
        p_rom_addr                            <= {{P_ROM_AW-2}{1'b0}}                  ; 
        p_rom_addr_buf                        <= {{P_ROM_AW-2}{1'b0}}                  ; 
        cipher_bus_hrdata                     <= {{CIPHER_ROM_p_AHB_DATA_WIDTH}{1'b0}} ; 
        rd_flg_vail                           <= 1'b0                                  ; 
        rd_flg_vail_buf                       <= 1'b0                                  ; 
        ecc_dec_sec                           <= 1'b0                                  ; 
        ecc_dec_ded                           <= 1'b0                                  ; 
    end else begin
        ecc_dec_addr                          <= ecc_dec_addr_next                     ; 
        p_rom_addr                            <= p_rom_addr_next                       ; 
        p_rom_addr_buf                        <= p_rom_addr_buf_next                   ; 
        cipher_bus_hrdata                     <= cipher_bus_hrdata_next                ; 
        rd_flg_vail                           <= rd_flg_vail_next                      ; 
        rd_flg_vail_buf                       <= rd_flg_vail_buf_next                  ; 
        ecc_dec_sec                           <= ecc_dec_sec_next                      ; 
        ecc_dec_ded                           <= ecc_dec_ded_next                      ; 
    end
end

endmodule 
