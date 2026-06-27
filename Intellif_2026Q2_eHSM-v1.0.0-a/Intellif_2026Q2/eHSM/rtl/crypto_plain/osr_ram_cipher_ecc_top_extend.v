//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ram_cipher_ecc_top_extend #(
    parameter  P_RAM_AW         = 23    ,
    parameter  P_RAM_DW         = 32    ,
    parameter  P_RAM_CIPHER_EN  = 1     ,
    parameter  P_RAM_CIPHER_RN  = 2     ,
    parameter  P_RAM_BLOCK_AW   = 5     ,
    parameter  P_RAM_ECC_EN     = 1     ,
    parameter  P_RAM_ECC_TM_CHB = (P_RAM_DW == 32) ? 7 : (P_RAM_DW == 64) ? 8 : (P_RAM_DW == 128) ? 9 : 10,
    parameter  P_RAM_ECC_DW     = (P_RAM_ECC_EN == 0)? P_RAM_DW : (P_RAM_DW == 32) ? 7+32 : (P_RAM_DW == 64) ? 8+64 : (P_RAM_DW == 128) ? 9+128 : 10+256 
    )(

    input  wire                           i_hclk              ,
    input  wire                           i_hresetn           ,

    input  wire                           i_ram_cipher_en     , 
    input  wire [P_RAM_CIPHER_RN*4-1:0]   i_ram_cipher_key    , 
    input  wire [P_RAM_DW-1:0]            i_ram_cipher_dnonce , 
    input  wire [P_RAM_AW-1:0]            i_ram_cipher_anonce , 

    input  wire [7:0]                     i_ecc_ck_en         , 
    input  wire [7:0]                     i_ecc_err_rsp_en    , 
    input  wire [7:0]                     i_ecc_tm_en         , 
    input  wire [P_RAM_ECC_TM_CHB-1:0]    i_ecc_tm_ckbits     , 

    output wire                           o_ecc_dec_sec       ,
    output wire                           o_ecc_dec_ded       ,
    output wire [P_RAM_AW-1:0]            o_ecc_err_addr      ,

    output wire                           o_ecc_dec_sec_nd    ,
    output wire                           o_ecc_dec_ded_nd    ,

    input  wire                           i_p_ram_cs          , 
    input  wire [P_RAM_DW/8-1:0]          i_p_ram_wen         ,  
    input  wire [P_RAM_AW-1:0]            i_p_ram_addr        , 
    input  wire [P_RAM_DW-1:0]            i_p_ram_wdata       , 
    output wire [P_RAM_DW-1:0]            o_p_ram_rdata       , 

    output wire                           o_c_ram_cs          , 
    output wire [P_RAM_DW/8-1:0]          o_c_ram_wen         ,  
    output wire [P_RAM_AW-1:0]            o_c_ram_addr        , 
    output wire [P_RAM_ECC_DW-1:0]        o_c_ram_wdata       , 
    input  wire [P_RAM_ECC_DW-1:0]        i_c_ram_rdata          

);

localparam P_RAM_ADDR_WIDTH_MAX = 17 ;

localparam     CIPHER_RAM_p_ADDR_WIDTH       = P_RAM_AW;
localparam     CIPHER_RAM_p_ADDR_BLOCK_WIDTH = P_RAM_BLOCK_AW;
localparam     CIPHER_RAM_p_DATA_WIDTH       = P_RAM_DW;
localparam     CIPHER_RAM_p_ROUND_NUM        = P_RAM_CIPHER_RN;

localparam     ECC_ENC_WIDTH_DIN    = (!P_RAM_ECC_EN) ? P_RAM_DW : (P_RAM_DW == 32) ? 49 : (P_RAM_DW == 64) ? 81 : P_RAM_DW;
localparam     ECC_ENC_WIDTH_CHKBIT = (!P_RAM_ECC_EN) ? P_RAM_AW :(ECC_ENC_WIDTH_DIN == 32) | (ECC_ENC_WIDTH_DIN == 49) ? 7 : (ECC_ENC_WIDTH_DIN == 64) | (ECC_ENC_WIDTH_DIN == 81) ? 8 : (ECC_ENC_WIDTH_DIN == 128) ? 9 : 10;
localparam     ECC_ENC_WIDTH_DOUT   = (!P_RAM_ECC_EN) ? ECC_ENC_WIDTH_DIN + P_RAM_ADDR_WIDTH_MAX : ECC_ENC_WIDTH_DIN + ECC_ENC_WIDTH_CHKBIT;

localparam     ECC_DEC_WIDTH_DOUT = (!P_RAM_ECC_EN) ? P_RAM_DW + 17 : (P_RAM_DW == 32) ? 49 : (P_RAM_DW == 64) ? 81 : P_RAM_DW;
localparam     ECC_DEC_WIDTH_DIN  = (!P_RAM_ECC_EN) ? P_RAM_DW + 17 :(ECC_DEC_WIDTH_DOUT == 32) ? 39 : (ECC_DEC_WIDTH_DOUT == 49) ? 56 : (ECC_DEC_WIDTH_DOUT == 64) ? 72 : (ECC_DEC_WIDTH_DOUT == 81) ? 89 : (ECC_DEC_WIDTH_DOUT == 128) ? 137 : 266;

localparam     CIPHER_ROM_p_AHB_ADDR_WIDTH   = P_RAM_AW;
localparam     CIPHER_ROM_p_AHB_DATA_WIDTH   = P_RAM_DW;
localparam     CIPHER_ROM_p_ADDR_BLOCK_WIDTH = 5       ;

wire [CIPHER_RAM_p_DATA_WIDTH-1:0]     cipher_o_plain_rdata  ;
wire [CIPHER_RAM_p_ADDR_WIDTH-1:0]     cipher_o_cipher_waddr ;
wire [CIPHER_RAM_p_ADDR_WIDTH-1:0]     cipher_o_cipher_raddr ;
wire [CIPHER_RAM_p_DATA_WIDTH-1:0]     cipher_o_cipher_wdata ;

wire [ECC_ENC_WIDTH_DOUT-1:0]      ecc_enc_encode_dout     ;
wire [ECC_ENC_WIDTH_DIN-1:0]       ecc_enc_encode_din      ;

wire                             ecc_dec_ecc_sec     ;
wire                             ecc_dec_ecc_ded     ;
wire [ECC_DEC_WIDTH_DOUT-1:0]    ecc_dec_decode_dout ;


wire [P_RAM_AW-1:0] ram_addr = cipher_o_cipher_waddr;

reg [P_RAM_AW-1:0]                ecc_dec_addr               ; 
reg [P_RAM_AW-1:0]                p_ram_addr                 ; 
reg [P_RAM_AW-1:0]                p_ram_addr_buf             ; 
reg [CIPHER_RAM_p_DATA_WIDTH-1:0] cipher_plain_rdata         ; 
reg                               rd_flg_vail                ; 
reg                               rd_flg_vail_buf            ; 
reg                               ecc_dec_sec                ; 
reg                               ecc_dec_ded                ; 

generate
if(P_RAM_CIPHER_EN)begin:cipher_enable
   wire                                   cipher_clk            = i_hclk; 
   wire                                   cipher_rst_n          = i_hresetn;
   wire                                   cipher_i_enable       = i_ram_cipher_en;
   wire [CIPHER_RAM_p_ROUND_NUM*4-1:0]    cipher_i_key          = i_ram_cipher_key;
   wire [CIPHER_RAM_p_DATA_WIDTH-1:0]     cipher_i_data_nonce   = i_ram_cipher_dnonce; 
   wire [CIPHER_RAM_p_ADDR_WIDTH-1:0]     cipher_i_addr_nonce   = i_ram_cipher_anonce;
   wire [CIPHER_RAM_p_ADDR_WIDTH-1:0]     cipher_i_plain_waddr  = i_p_ram_cs ? i_p_ram_addr : {{CIPHER_RAM_p_ADDR_WIDTH}{1'b0}}; 
   wire [CIPHER_RAM_p_ADDR_WIDTH-1:0]     cipher_i_plain_raddr  = i_p_ram_cs ? i_p_ram_addr : {{CIPHER_RAM_p_ADDR_WIDTH}{1'b0}};
   wire [CIPHER_RAM_p_DATA_WIDTH-1:0]     cipher_i_plain_wdata  = |o_c_ram_wen ? i_p_ram_wdata : {{CIPHER_RAM_p_DATA_WIDTH}{1'b0}};
   wire [CIPHER_RAM_p_DATA_WIDTH-1:0]     cipher_i_cipher_rdata = !rd_flg_vail ? {{CIPHER_RAM_p_DATA_WIDTH}{1'b0}} : ecc_dec_decode_dout[P_RAM_DW+P_RAM_ADDR_WIDTH_MAX-1:P_RAM_ADDR_WIDTH_MAX];
   ram_cipher_top #(
       .p_ADDR_WIDTH           ( CIPHER_RAM_p_ADDR_WIDTH       ), 
       .p_ADDR_BLOCK_WIDTH     ( CIPHER_RAM_p_ADDR_BLOCK_WIDTH ), 
       .p_DATA_WIDTH           ( CIPHER_RAM_p_DATA_WIDTH       ), 
       .p_ROUND_NUM            ( CIPHER_RAM_p_ROUND_NUM        )  
       ) u_ram_cipher_top (
       .clk                  ( cipher_clk            ), 
       .rst_n                ( cipher_rst_n          ), 
       .i_enable             ( cipher_i_enable       ), 
       .i_key                ( cipher_i_key          ), 
       .i_data_nonce         ( cipher_i_data_nonce   ), 
       .i_addr_nonce         ( cipher_i_addr_nonce   ), 
       .i_plain_waddr        ( cipher_i_plain_waddr  ), 
       .i_plain_raddr        ( cipher_i_plain_raddr  ), 
       .i_plain_wdata        ( cipher_i_plain_wdata  ), 
       .o_plain_rdata        ( cipher_o_plain_rdata  ), 
       .o_cipher_waddr       ( cipher_o_cipher_waddr ), 
       .o_cipher_raddr       ( cipher_o_cipher_raddr ), 
       .o_cipher_wdata       ( cipher_o_cipher_wdata ), 
       .i_cipher_rdata       ( cipher_i_cipher_rdata )  
       ); 
end
else begin:cipher_bypass
   assign cipher_o_plain_rdata  = !rd_flg_vail ? {{CIPHER_RAM_p_DATA_WIDTH}{1'b0}} : ecc_dec_decode_dout[P_RAM_DW+P_RAM_ADDR_WIDTH_MAX-1:P_RAM_ADDR_WIDTH_MAX];
   assign cipher_o_cipher_waddr = i_p_ram_cs ? i_p_ram_addr : {{CIPHER_RAM_p_ADDR_WIDTH}{1'b0}};
   assign cipher_o_cipher_raddr = i_p_ram_cs ? i_p_ram_addr : {{CIPHER_RAM_p_ADDR_WIDTH}{1'b0}};
   assign cipher_o_cipher_wdata = |o_c_ram_wen ? i_p_ram_wdata : {{CIPHER_RAM_p_DATA_WIDTH}{1'b0}};
end
endgenerate

generate
if(P_RAM_ECC_EN)begin:ecc_enable
    assign ecc_enc_encode_din      = |o_c_ram_wen ?  {cipher_o_cipher_wdata,{{{17-P_RAM_AW}{1'b0}},cipher_o_cipher_waddr}} : {{ECC_ENC_WIDTH_DIN}{1'b0}};
    wire [ECC_ENC_WIDTH_CHKBIT-1:0]    ecc_enc_tm_chkbits      = i_ecc_tm_ckbits;
    wire [7:0]                         ecc_enc_tm_sel_ecc_code = i_ecc_tm_en;
    wire [ECC_DEC_WIDTH_DIN-1:0]     ecc_dec_decode_din  = rd_flg_vail ? {i_c_ram_rdata,{{{17-P_RAM_AW}{1'b0}},ecc_dec_addr}} : {{ECC_DEC_WIDTH_DIN}{1'b0}};
    wire [7:0]                       ecc_dec_ecc_en      = i_ecc_ck_en; 
    wire [ECC_DEC_WIDTH_DIN-1:0]     ecc_dec_xor_dedin   = {ECC_DEC_WIDTH_DIN{1'b0}};

    osr_ecc_encode #(
        .WIDTH_DIN        ( ECC_ENC_WIDTH_DIN    ), 
        .WIDTH_CHKBIT     ( ECC_ENC_WIDTH_CHKBIT ), 
        .WIDTH_DOUT       ( ECC_ENC_WIDTH_DOUT   )  
        ) u_osr_ecc_encode (
        .encode_din             ( ecc_enc_encode_din      ), 
        .tm_chkbits             ( ecc_enc_tm_chkbits      ), 
        .tm_sel_ecc_code        ( ecc_enc_tm_sel_ecc_code ), 
        .encode_dout            ( ecc_enc_encode_dout     )  
        ); 

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
end
else begin:ram_ecc_bypass
    assign ecc_enc_encode_dout = |o_c_ram_wen ?  {cipher_o_cipher_wdata,{{{P_RAM_ADDR_WIDTH_MAX-P_RAM_AW}{1'b0}},cipher_o_cipher_waddr}} : {{P_RAM_DW+P_RAM_ADDR_WIDTH_MAX}{1'b0}}; 
    assign ecc_dec_ecc_sec     = 1'b0;
    assign ecc_dec_ecc_ded     = 1'b0;
    assign ecc_dec_decode_dout = rd_flg_vail ? {i_c_ram_rdata,{P_RAM_ADDR_WIDTH_MAX{1'b0}}} : {{ECC_DEC_WIDTH_DOUT}{1'b0}};
end
endgenerate

wire[P_RAM_AW-1:0] ecc_dec_addr_next = ram_addr;
wire[P_RAM_AW-1:0] p_ram_addr_next = i_p_ram_addr ;
wire[P_RAM_AW-1:0] p_ram_addr_buf_next = p_ram_addr;
wire [CIPHER_RAM_p_DATA_WIDTH-1:0] cipher_plain_rdata_next = cipher_o_plain_rdata ;

wire rd_flg_vail_next = i_p_ram_cs & (|i_p_ram_wen == 1'b0) ;
wire rd_flg_vail_buf_next = rd_flg_vail;
wire ecc_dec_sec_next = ecc_dec_ecc_sec;
wire ecc_dec_ded_next = ecc_dec_ecc_ded;

assign o_ecc_dec_sec       = ecc_dec_sec & rd_flg_vail_buf;
assign o_ecc_dec_ded       = ecc_dec_ded & rd_flg_vail_buf;
assign o_ecc_err_addr      = p_ram_addr_buf;
assign o_ecc_dec_sec_nd    = ecc_dec_ecc_sec & rd_flg_vail;
assign o_ecc_dec_ded_nd    = ecc_dec_ecc_ded & rd_flg_vail;
assign o_p_ram_rdata       = rd_flg_vail ? cipher_o_plain_rdata : (ecc_dec_ded && rd_flg_vail_buf && (i_ecc_err_rsp_en != 8'h5A)) ? cipher_plain_rdata : {{P_RAM_DW}{1'b0}}; 
assign o_c_ram_cs          = i_p_ram_cs;
assign o_c_ram_wen         = i_p_ram_cs ? i_p_ram_wen : {{P_RAM_DW/8}{1'b0}};
assign o_c_ram_addr        = i_p_ram_cs ? cipher_o_cipher_waddr : {{P_RAM_AW}{1'b0}};
assign o_c_ram_wdata       = i_p_ram_cs ? ecc_enc_encode_dout[ECC_ENC_WIDTH_DOUT-1:P_RAM_ADDR_WIDTH_MAX] : {{P_RAM_ECC_DW}{1'b0}} ;

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        ecc_dec_addr                    <= {P_RAM_AW{1'b0}}                ; 
        p_ram_addr                      <= {P_RAM_AW{1'b0}}                ; 
        p_ram_addr_buf                  <= {P_RAM_AW{1'b0}}                ; 
        cipher_plain_rdata              <= {CIPHER_RAM_p_DATA_WIDTH{1'b0}} ; 
        rd_flg_vail                     <= 1'b0                            ; 
        rd_flg_vail_buf                 <= 1'b0                            ; 
        ecc_dec_sec                     <= 1'b0                            ; 
        ecc_dec_ded                     <= 1'b0                            ; 
    end else begin
        ecc_dec_addr                    <= ecc_dec_addr_next               ; 
        p_ram_addr                      <= p_ram_addr_next                 ; 
        p_ram_addr_buf                  <= p_ram_addr_buf_next             ; 
        cipher_plain_rdata              <= cipher_plain_rdata_next         ; 
        rd_flg_vail                     <= rd_flg_vail_next                ; 
        rd_flg_vail_buf                 <= rd_flg_vail_buf_next            ; 
        ecc_dec_sec                     <= ecc_dec_sec_next                ; 
        ecc_dec_ded                     <= ecc_dec_ded_next                ; 
    end
end

endmodule 
