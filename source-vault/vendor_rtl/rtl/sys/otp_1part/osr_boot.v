//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_boot (
    input  wire                 i_boot_clk              ,  
    input  wire                 i_sys_rst_n             , 
    input  wire                 i_scan_mode             ,

    input  wire                 i_hclk                  ,
    input  wire                 i_hresetn               ,
    input  wire                 i_hsel                  ,
    input  wire [31:0]          i_haddr                 ,
    input  wire [1:0 ]          i_htrans                ,
    input  wire                 i_hwrite                ,
    input  wire [2:0 ]          i_hsize                 ,
    input  wire [2:0 ]          i_hburst                ,
    input  wire [3:0 ]          i_hprot                 ,
    input  wire [31:0]          i_hwdata                ,
    output wire [31:0]          o_hrdata                ,
    output wire                 o_hready                ,
    output wire [1:0 ]          o_hresp                 ,

    input  wire                 i_bus_cipher_en         , 
    input  wire                 i_ram_cipher_en         ,
    input  wire                 i_nvm_cipher_bypass     ,

    output wire                 o_hw_boot_done          ,
    output wire                 o_hw_boot_ok            ,
    output wire                 o_hw_boot_err           ,

    output wire                 o_hw_boot_done_pulse    ,

    output wire                 o_lc_undef              , 
    output wire                 o_lc_destroy            , 
    output wire                 o_lc_debug              , 
    output wire                 o_lc_user               , 
    output wire                 o_lc_manu               ,
    output wire                 o_lc_dev2               ,
    output wire                 o_lc_dev                , 
    output wire                 o_lc_test               , 

    output wire [31:0]          o_life_cycle            ,

    output wire                 o_rdbi_st_done          ,

    output wire                 o_otp_cs                , 
    output wire [31:0]          o_otp_addr              , 
    input  wire                 i_otp_rdy               ,
    input  wire                 i_otp_rd_data_vld       ,
    input  wire [31:0]          i_otp_rd_data           , 

    output wire                 o_crc_err               ,  

    input  wire                 i_trng_alarm            , 
    input  wire                 i_trng_ffvld            ,
    output wire                 o_trng_rd_en            , 
    input  wire [31:0]          i_trng_rd_data          , 
    output wire                 o_trng_skip_stp         , 
    output wire                 o_trng_sclk_sel         , 
    output wire                 o_trng_to_en            , 

    output wire                 o_bus_cipher_en         ,  
    output wire [31:0]          o_bus_cipher_key        ,  
    output wire [31:0]          o_bus_cipher_nce        ,  
    output wire                 o_ram_cipher_en         ,  
    output wire [31:0]          o_ram_cipher_key        ,  
    output wire [31:0]          o_ram_cipher_nce        , 
    output wire                 o_nvm_cipher_bypass     ,  

    input  wire [11:0]          i_offset_att            ,
    output wire [7:0]           o_last_otp              ,

    output wire                 o_kbuf_CSB              ,        
    output wire [7:0]           o_kbuf_A                ,
    output wire [3:0]           o_kbuf_WEB              ,        
    output wire [31:0]          o_kbuf_DI               ,        
    input  wire [31:0]          i_kbuf_DO               ,

    output wire                 o_random_clock_en_set   ,

    output wire [31:0]          o_con_field0            , 
    output wire [31:0]          o_con_field1            , 
    output wire [31:0]          o_con_field2            , 
    output wire [31:0]          o_con_field3            , 
    output wire [31:0]          o_con_field4            , 
    output wire [31:0]          o_con_field5            , 

    output wire                 o_patch_en              ,
    output wire                 o_patch_info_vld        ,
    output wire [11:0]          o_patch_info_addr       ,
    output wire [31:0]          o_patch_otp_rdata       ,

    output wire                 o_reset_trng_warning    ,
    output wire                 o_reset_trng_error      ,
    output wire                 o_reset_trng_n          ,

    output wire [63:0]          o_clc_key               ,

    input  wire                 i_mem_init_done         
);

localparam  [31:0]  IDLE          = 32'ha050_6030 ;  
localparam  [31:0]  RDBI          = 32'h8310_a551 ; 
localparam  [31:0]  RDKD          = 32'h5027_7a7b ; 
localparam  [31:0]  KSKDEC        = 32'h613a_8b2f ; 
localparam  [31:0]  RELES         = 32'h361b_d728 ; 
localparam  [31:0]  RESET         = 32'hc130_5160 ; 

localparam          P_HAVE_KMU    = 1'h1          ;

localparam P_GET_TRNG = 1'h0 ;

localparam P_NOT_LAST_KEY0        = 4'b1111 ;
localparam P_NOT_LAST_KEY1        = 4'b1000 ;

localparam P_KEY_TYPE_SYMMETRIC   = 4'b1110 ;
localparam P_KEY_TYPE_PRIVATE     = 4'b1101 ; 
localparam P_KEY_TYPE_PUBLIC_HASH = 4'b1011 ;
localparam P_KEY_TYPE_RESERVED    = 4'b0111 ;

localparam P_KEY_LC_UNBURNED      = 4'b1111 ;   
localparam P_KEY_LC_UNUSED        = 4'b1110 ;  
localparam P_KEY_LC_AVAILABLE     = 4'b1100 ; 
localparam P_KEY_LC_DISABLED      = 4'b1000 ;
localparam P_KEY_LC_DISTORIED     = 4'b0000 ; 

localparam P_KEY_LEVEL_1          = 4'b1010 ;
localparam P_KEY_LEVEL_2          = 4'b0101 ;  

wire            wr_kmu_en               ;
wire            wr_kmu_done_pulse       ;
wire            cscnt8                  ;

wire            w_rdkd_key0_vld         ; 
wire            w_rdkd_key1_vld         ;
wire            w_rdkd_key2_vld         ;
wire            w_rdkd_key3_vld         ;
wire            w_rdkd_key4_vld         ;
wire            w_rdkd_key5_vld         ;
wire            w_rdkd_key6_vld         ; 
wire            w_rdkd_key7_vld         ;
wire            w_rdkd_kcrc_vld         ;

wire            ipatch_en               ; 
wire            key_alg_sel             ; 
wire            trng_sclk_sel           ; 
wire            trng_self_test_skip     ; 
wire            ram_cipher_en           ; 
wire            bus_cipher_en           ; 
wire            nvm_cipher_bypass       ; 

wire            w_rdkd_done             ;
wire            bc2kdf_dly              ;

wire            kdf_dec_done            ;
wire            crc_err_set             ;
wire            crc_ok_set              ;

wire [255:0]    kdf2bc_key              ;

wire [4:0]      kbuf_A                  ;
wire [255:0]    kbuf_DI                 ;

wire [3:0]      w_ram_we_n              ;

wire            key_invalid_booterr     ;


wire    [31:0]  key_inv = 32'hFFFF_FFFF ;

wire            rd_trng_done            ;

reg                              init_ram_done        ;
reg [11:0]                       icnt                 ;
reg                              r_keep_dec_done      ;
reg                              mem_init_done_f      ;
reg                              mem_init_done_ff     ;
reg                              hw_boot_ok0          ;
reg                              hw_boot_ok1          ;
reg                              hw_boot_ok2          ;
reg                              hw_boot_ok3          ;
reg                              hw_boot_ok           ;
reg                              hw_boot_err          ;
reg                              r_dec_done           ;
reg [31:0]                       current_state        ;
reg [31:0]                       con_field0           ;
reg [31:0]                       con_field1           ;
reg [31:0]                       con_field2           ;
reg [31:0]                       con_field3           ;
reg [31:0]                       con_field4           ;
reg [31:0]                       con_field5           ;
reg [31:0]                       r_otp_key0           ;
reg [31:0]                       r_otp_key1           ;
reg [31:0]                       r_otp_key2           ;
reg [31:0]                       r_otp_key3           ;
reg [31:0]                       r_otp_key4           ;
reg [31:0]                       r_otp_key5           ;
reg [31:0]                       r_otp_key6           ;
reg [31:0]                       r_otp_key7           ;
reg [31:0]                       r_otp_kcrc           ;
reg [31:0]                       kram_wd              ;
reg                              test_mode            ;
reg                              develop_mode         ;
reg                              develop2_mode        ;
reg                              manufacture_mode     ;
reg                              user_mode            ;
reg                              debug_mode           ;
reg                              destroy_mode         ;
reg                              undefine_mode        ;
reg [31:0]                       life_cycle           ;
reg                              random_clock_en_set  ;
reg [31:0]                       key_att              ;
reg                              r_crc_err            ;
reg                              l_crc_err            ;
reg                              r_rdbi_st_done       ;
reg                              r_kskdf_st           ;
reg                              reset_trng_fw_n      ;
reg [10-1:0] r_otp_rd_addr        ;
reg                              r_otp_cs             ;
reg [3:0]                        r_rdkd_cntr          ;
reg [5:0]                        r_ld_key_cntr        ;
reg                              r_bc2kdf_done        ;
reg                              update_n             ;
reg [5:0]                        r_cntr_dly           ;
reg                              kcrc_ok              ;
reg [4:0]                        cscnt                ;
reg [4:0]                        r_kbuf_A             ;
reg [3:0]                        r_ceb                ;
reg [7:0]                        r_cip_addr           ;
reg                              r_wr_katt            ;
reg                              r_hw_boot_done       ;

wire                              r_trng_rd_en         = 1'h0                      ; 
wire                              r_trng_rd_done       = 1'h0                      ; 
wire [32*5'h6-1:0]       r_trng_data          = 192'h9b17ef36_8f7a461c_52b03b8d_d7b12c5f_3cec732a_ed6a675e             ; 
wire                              reset_trng_warning   = 1'h0                      ; 
wire                              reset_trng_error     = 1'h0                      ; 


wire init_ram_done_next = icnt == (12'd256 - 12'h1) ; 

wire [11:0] icnt_next = init_ram_done_next ? icnt : (icnt + 12'h1); 

wire dec_wrkmu_done       = r_keep_dec_done & wr_kmu_done_pulse;
wire r_keep_dec_done_next = wr_kmu_done_pulse ? 1'h0 : r_dec_done ? 1'h1 : r_keep_dec_done; 

wire    mem_init_done_f_next  = i_mem_init_done; 
wire    mem_init_done_ff_next = mem_init_done_f; 
wire    hw_boot_ok0_next = (current_state == RELES) & mem_init_done_ff ; 
wire    hw_boot_ok1_next = hw_boot_ok0; 
wire    hw_boot_ok2_next = hw_boot_ok1; 
wire    hw_boot_ok3_next = hw_boot_ok2; 
wire    hw_boot_ok_next  = hw_boot_ok3; 

wire    hw_boot_err_next   = (current_state == RESET) ; 

wire    tdd_mode = test_mode | develop_mode | develop2_mode | debug_mode ;

assign    rd_trng_done = (!P_GET_TRNG) ? 1'h1                                :
                       tdd_mode      ? (r_trng_rd_done | reset_trng_error) : r_trng_rd_done     ;

wire    r_dec_done_next = kdf_dec_done ;

wire    rdbi2reset     = destroy_mode | undefine_mode | (r_rdbi_st_done & (~tdd_mode & reset_trng_error));

        wire    rdbi2reles      = 1'h0          ; 

wire    rdbi2rdkd      = r_rdbi_st_done & rd_trng_done & init_ram_done;

wire    kskdec2reset   = tdd_mode ? 1'h0 : key_invalid_booterr;

wire    rdkd2reles   = P_HAVE_KMU ? 1'h0 : 1'h1 ;

reg [31:0] current_state_next ; 
always @(*)  begin : boot_state_machine
    case(current_state)
        IDLE    :   if(i_otp_rdy)
                        current_state_next = RDBI   ;
                    else                                 
                        current_state_next = IDLE   ;
        RDBI    :   if(rdbi2reset)
                        current_state_next = RESET  ;
                    else if(rdbi2reles)
                        current_state_next = RELES  ;
                    else if(rdbi2rdkd)
                        current_state_next = RDKD   ;
                    else                                 
                        current_state_next = RDBI   ;
        RDKD    :   if(rdkd2reles)
                        current_state_next = RELES  ;
                    else if(w_rdkd_done)
                        current_state_next = KSKDEC ;
                    else                                 
                        current_state_next = RDKD   ;
        KSKDEC  :   if(kskdec2reset)
                        current_state_next = RESET  ;
                    else if(bc2kdf_dly) 
                        current_state_next = RELES  ;
                    else if(dec_wrkmu_done & ~r_bc2kdf_done)  
                        current_state_next = RDKD   ;
                    else  
                        current_state_next = KSKDEC ;
        RESET   :   current_state_next = RESET      ;
        RELES   :   current_state_next = RELES      ;
        default :   current_state_next = IDLE       ;
    endcase
end

wire    w_idle_st_sel  = (current_state == IDLE  );
wire    w_rdbi_st_sel  = (current_state == RDBI  );
wire    w_rdkd_st_sel  = (current_state == RDKD  );
wire    w_kskdf_st_sel = (current_state == KSKDEC);
wire    w_reles_st_sel = (current_state == RELES );
wire    w_reset_st_sel = (current_state == RESET );

wire    [12-1:0] otp_addr = {r_otp_rd_addr[10-1:0],2'h0}; 

wire    w_rdbi_rd_ack       = i_otp_rd_data_vld & w_rdbi_st_sel;

wire    wr_lc         = w_rdbi_rd_ack & (otp_addr == 12'h000);
wire    wr_con_field0 = w_rdbi_rd_ack & (otp_addr == 12'h018);
wire    wr_con_field1 = w_rdbi_rd_ack & (otp_addr == 12'h01C);
wire    wr_con_field2 = w_rdbi_rd_ack & (otp_addr == 12'h020);
wire    wr_con_field3 = w_rdbi_rd_ack & (otp_addr == 12'h024);
wire    wr_con_field4 = w_rdbi_rd_ack & (otp_addr == 12'h028);
wire    wr_con_field5 = w_rdbi_rd_ack & (otp_addr == 12'h02C);

wire    w_rdkd_rd_ack = i_otp_rd_data_vld & w_rdkd_st_sel;

wire    wr_otp_katt = w_rdkd_rd_ack & (r_rdkd_cntr == 4'h0);

wire    [31:0] con_field0_next       = wr_con_field0 ? i_otp_rd_data : con_field0       ; 
wire    [31:0] con_field1_next       = wr_con_field1 ? i_otp_rd_data : con_field1       ; 
wire    [31:0] con_field2_next       = wr_con_field2 ? i_otp_rd_data : con_field2       ; 
wire    [31:0] con_field3_next       = wr_con_field3 ? i_otp_rd_data : con_field3       ; 
wire    [31:0] con_field4_next       = wr_con_field4 ? i_otp_rd_data : con_field4       ; 
wire    [31:0] con_field5_next       = wr_con_field5 ? i_otp_rd_data : con_field5       ; 

wire    [31:0]  r_otp_key0_next  = w_rdkd_key0_vld ? i_otp_rd_data : r_otp_key0; 
wire    [31:0]  r_otp_key1_next  = w_rdkd_key1_vld ? i_otp_rd_data : r_otp_key1; 
wire    [31:0]  r_otp_key2_next  = w_rdkd_key2_vld ? i_otp_rd_data : r_otp_key2; 
wire    [31:0]  r_otp_key3_next  = w_rdkd_key3_vld ? i_otp_rd_data : r_otp_key3; 
wire    [31:0]  r_otp_key4_next  = w_rdkd_key4_vld ? i_otp_rd_data : r_otp_key4; 
wire    [31:0]  r_otp_key5_next  = w_rdkd_key5_vld ? i_otp_rd_data : r_otp_key5; 
wire    [31:0]  r_otp_key6_next  = w_rdkd_key6_vld ? i_otp_rd_data : r_otp_key6; 
wire    [31:0]  r_otp_key7_next  = w_rdkd_key7_vld ? i_otp_rd_data : r_otp_key7; 
wire    [31:0]  r_otp_kcrc_next  = w_rdkd_kcrc_vld ? i_otp_rd_data : r_otp_kcrc; 

reg     [31:0]  kram_wd_next ;   

always @(*) begin
    if (w_rdkd_done) kram_wd_next = key_att ;
    else case(cscnt)
        5'h0    : kram_wd_next = wr_kmu_en ? kbuf_DI[ 63: 32] : 32'h01F8CA69; 
        5'h1    : kram_wd_next = wr_kmu_en ? kbuf_DI[ 95: 64] : 32'h0FC58806;
        5'h2    : kram_wd_next = wr_kmu_en ? kbuf_DI[127: 96] : 32'h320BCDEF;
        5'h3    : kram_wd_next = wr_kmu_en ? kbuf_DI[159:128] : 32'h0F8AE906;
        5'h4    : kram_wd_next = wr_kmu_en ? kbuf_DI[191:160] : 32'h3981F760;
        5'h5    : kram_wd_next = wr_kmu_en ? kbuf_DI[223:192] : 32'h312C840F;
        5'h6    : kram_wd_next = wr_kmu_en ? kbuf_DI[255:224] : 32'h965CDE93;
        default : kram_wd_next = wr_kmu_en ? kbuf_DI[ 31:  0] : 32'h75F29068;
    endcase
end 

wire    lc5 = (i_otp_rd_data == 32'h00000000       ) ;
wire    lc4 = (i_otp_rd_data == 32'h42818414    ) ;
wire    lca = (i_otp_rd_data == 32'h4681A416   ) ; 
wire    lc3 = (i_otp_rd_data == 32'h46C1A416) ;
wire    lc2 = (i_otp_rd_data == 32'h57C1EC96       ) ;
wire    lc1 = (i_otp_rd_data == 32'hD7C5FCDE      ) ;
wire    lc0 = (i_otp_rd_data == 32'hFFFFFFFF    ) ;

wire    test_mode_next         = wr_lc ? lc5 : test_mode         ;                            
wire    develop_mode_next      = wr_lc ? lc4 : develop_mode      ;                            
wire    develop2_mode_next     = wr_lc ? lca : develop2_mode     ;                            
wire    manufacture_mode_next  = wr_lc ? lc3 : manufacture_mode  ;                            
wire    user_mode_next         = wr_lc ? lc2 : user_mode         ;                            
wire    debug_mode_next        = wr_lc ? lc1 : debug_mode        ;                            
wire    destroy_mode_next      = wr_lc ? lc0 : destroy_mode      ;                            
wire    undefine_mode_next     = wr_lc ? ~(lc5 | lc4 | lca | lc3 | lc2 | lc1 | lc0) : undefine_mode;

wire    [31:0] life_cycle_next = wr_lc ? i_otp_rd_data : life_cycle;

wire    random_clock_en_set_next = (~test_mode & wr_con_field0) ? ((i_otp_rd_data[17:16] != 2'b01)) : 1'h0;

assign  ipatch_en           = (con_field0[15:14] == 2'b01) | (con_field0[15:14] == 2'b10) ;
assign  key_alg_sel         = (con_field0[13:12] == 2'b01) ;
assign  nvm_cipher_bypass   = (con_field0[11:10] != 2'b01) | i_nvm_cipher_bypass ;
assign  trng_sclk_sel       = (con_field0[  7:6] == 2'b01) ;
assign  trng_self_test_skip = (con_field0[  5:4] == 2'b01) ;
assign  ram_cipher_en       = (con_field0[  3:2] != 2'b01) | i_ram_cipher_en ;
assign  bus_cipher_en       = (con_field0[  1:0] != 2'b01) | i_bus_cipher_en ;

wire    [31:0]  key_att_next = wr_otp_katt ? (key_inv ^ i_otp_rd_data) : key_att ;

wire    keynl = (key_att[7:4] == P_KEY_LEVEL_1); 

wire    key0  = (r_ld_key_cntr == 6'h1 ); 
wire    key1  = (r_ld_key_cntr == 6'h2 ); 
wire    key2  = (r_ld_key_cntr == 6'h3 ); 
wire    keyn  = (r_ld_key_cntr >= 6'h4 ); 

wire    kend            = ~keyn ? 1'h0 : ~((key_att[31:28] == P_NOT_LAST_KEY0 ) | (key_att[31:28] == P_NOT_LAST_KEY1));

wire    check_crc = crc_err_set;

wire    r_crc_err_next = ( key0 & (key_att[3:0] == P_KEY_LC_AVAILABLE) & check_crc ) | 
                         ( key1 & (key_att[3:0] == P_KEY_LC_AVAILABLE) & check_crc ) |
                         ( key2 & (key_att[3:0] == P_KEY_LC_AVAILABLE) & check_crc ) |
                         ( keyn & (key_att[3:0] == P_KEY_LC_AVAILABLE) & check_crc ) ;

wire    l_crc_err_next  = r_crc_err_next ? 1'h1 : l_crc_err ; 

assign  key_invalid_booterr = (  key0  &  (user_mode | manufacture_mode          )  &  (r_crc_err | (key_att[3:0] != P_KEY_LC_AVAILABLE))  ) | 
                              (  key1  &  (user_mode                             )  &  (r_crc_err | (key_att[3:0] != P_KEY_LC_AVAILABLE))  ) | 
                              (  key2  &  (user_mode | manufacture_mode          )  &  (r_crc_err | (key_att[3:0] != P_KEY_LC_AVAILABLE))  ) | 
                              (  keyn  &  (user_mode | (manufacture_mode & keynl))  &   r_crc_err                                          ) ; 

wire    sel_root_key  = (key1  & 1'h1  ) | 
                        (key2  & 1'h1  ) |
                        (keyn  & keynl ) ;

wire    w_rdbi_rd_vld       = w_rdbi_rd_ack & (otp_addr <= 12'h114);

wire    w_rdbi_rd_lst       = w_rdbi_rd_ack & (otp_addr == 12'h114);

wire    r_rdbi_st_done_next = w_rdbi_rd_lst | r_rdbi_st_done; 

assign  w_rdkd_done         = (r_rdkd_cntr == 4'h9) && i_otp_rd_data_vld;

assign  w_rdkd_key0_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd1);
assign  w_rdkd_key1_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd2);
assign  w_rdkd_key2_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd3);
assign  w_rdkd_key3_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd4);
assign  w_rdkd_key4_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd5);
assign  w_rdkd_key5_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd6);
assign  w_rdkd_key6_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd7);
assign  w_rdkd_key7_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd8);
assign  w_rdkd_kcrc_vld  = w_rdkd_rd_ack & (r_rdkd_cntr[3:0] == 4'd9);

wire    r_kskdf_st_next     = w_kskdf_st_sel;
wire    bc2kdf_go           = (~r_kskdf_st & w_kskdf_st_sel);
wire    [255:0] w_otp_ekey  = {r_otp_key7, r_otp_key6, r_otp_key5, r_otp_key4,
                               r_otp_key3, r_otp_key2, r_otp_key1, r_otp_key0};
wire    [31:0]  w_otp_crc   = r_otp_kcrc;

wire reset_trng_fw_n_next = hw_boot_ok0 | (hw_boot_err & tdd_mode);    

wire    up_hwc  = w_rdbi_rd_ack & (otp_addr == 12'h000); 
wire    up_fwc  = w_rdbi_rd_ack & (otp_addr == 12'h01C); 
wire    up_patch_info  = w_rdbi_rd_ack & (otp_addr == 12'h02C); 
wire    up_key  = w_rdbi_rd_ack & (otp_addr == 12'h114); 

wire    w_otp_rd_done = w_reles_st_sel | w_reset_st_sel;

wire    [12-1:0] addr_hwc2 = 12'h014 + 12'h4;
wire    [12-1:0] addr_fwc2 = 12'h01C + 12'h4;
wire    [12-1:0] addr_key2 = 12'h118;

wire    [10-1:0] addr_hwc = addr_hwc2[12-1:2];
wire    [10-1:0] addr_fwc = addr_fwc2[12-1:2];
wire    [10-1:0] addr_key = addr_key2[12-1:2];

wire    [12-1:0] patch_addr_base = 12'h050;
wire    [12-1:0] key_base_addr = 12'h118;
wire    [10-1:0] r_otp_rd_addr_next = (w_reset_st_sel | w_idle_st_sel) ? 10'b0 : 
                  w_rdbi_rd_lst ? key_base_addr[12-1:2]      :
                  up_hwc        ? addr_hwc      :
                  up_fwc        ? addr_fwc      :
                  up_patch_info ? patch_addr_base[12-1:2] :
                  up_key        ? addr_key      :
                  w_rdbi_rd_vld ? r_otp_rd_addr + 10'b1 : 
                  w_rdkd_rd_ack ? r_otp_rd_addr + 10'b1 : r_otp_rd_addr ;

wire    r_otp_cs_next       = w_reset_st_sel                      | 
                              (w_rdbi_st_sel & w_rdbi_rd_lst    ) | 
                              (w_rdkd_st_sel & w_rdkd_done      ) |
                              (w_otp_rd_done & i_otp_rd_data_vld)   ? 1'b0 : 
                              (w_idle_st_sel & i_otp_rdy) |
                               w_rdkd_st_sel                        ? 1'b1 : r_otp_cs;

wire [3:0] r_rdkd_cntr_next = w_kskdf_st_sel ? 4'h0 : 
                              w_rdkd_rd_ack  ? (r_rdkd_cntr + 4'h1) : r_rdkd_cntr;

wire [5:0] r_ld_key_cntr_next  = (w_rdkd_st_sel & w_rdkd_done) ? (r_ld_key_cntr + 6'h1) : r_ld_key_cntr; 

wire [5:0] kmu_slot = 6'd21;

wire    w_bc2kdf_done       = ((r_ld_key_cntr == kmu_slot[5:0]) | kend) & kdf_dec_done;
wire    bc2kdf_done         = (current_state == RELES) ||
                              (current_state == RESET)  ;

wire r_bc2kdf_done_next = w_bc2kdf_done ? 1'h1 : r_bc2kdf_done ; 

assign bc2kdf_dly = r_bc2kdf_done & (cscnt == 5'h3) ;

wire upd_dfl = 1'h0;
wire upd_nxt = ( (current_state == RDBI) & (current_state_next == RDKD) ) ? 1'h1 : update_n; 
wire    update_n_next       = upd_nxt; 

wire    [7:0] w_ram_addr  = ~hw_boot_ok0 ? {2'b0,r_cntr_dly-6'd1} : 8'b0 ;

wire    [255:0] w_ram_data   =  kdf2bc_key ;

wire    w_ram_ce  = r_dec_done & (|r_cntr_dly) ;

wire    [5:0]   r_cntr_dly_next = r_ld_key_cntr ; 

function ktype_lg;
    input [3:0] ktype;
    ktype_lg = (ktype == P_KEY_TYPE_SYMMETRIC  ) | 
               (ktype == P_KEY_TYPE_PRIVATE    ) | 
               (ktype == P_KEY_TYPE_PUBLIC_HASH) | 
               (ktype == P_KEY_TYPE_RESERVED   ) ; 
endfunction

function klevel_lg;
    input [3:0] klevel;
    klevel_lg = (klevel == P_KEY_LEVEL_1) | 
                (klevel == P_KEY_LEVEL_2) ; 
endfunction

function klc_lg;
    input [3:0] klc;
    klc_lg = (klc == P_KEY_LC_AVAILABLE) ; 
endfunction

wire kcrc_ok_next = crc_err_set ? 1'h0 : crc_ok_set ? 1'h1 : kcrc_ok ; 

wire wr_crk = test_mode | (kcrc_ok & develop_mode | develop2_mode);

wire wr_drk = test_mode | (kcrc_ok & (develop_mode | develop2_mode | manufacture_mode));

assign wr_kmu_en = ( key0 & (wr_crk                                                                                          ) ) | 
                   ( key1 & (wr_drk                                                                                          ) ) | 
                   ( key2 & (test_mode | (                                                                           kcrc_ok)) ) | 
                   ( keyn & (test_mode | (ktype_lg(key_att[11:8]) & klevel_lg(key_att[7:4]) & klc_lg(key_att[3:0]) & kcrc_ok)) ) ;   

assign  wr_kmu_done_pulse = (cscnt == 5'h7);                    
assign  cscnt8 = (cscnt == 5'h8); 

wire    clr_cscnt = w_ram_ce;
wire    [4:0]   cscnt_next = clr_cscnt ? 5'h0 : 
                             cscnt8    ? 5'h8 : (cscnt + 5'h1) ;

wire    [4:0]   r_kbuf_A_next   = w_ram_ce ? kbuf_A : r_kbuf_A ;
wire    [3:0]   r_ceb_next      = w_ram_ce ? w_ram_we_n : r_ceb ; 
wire    [7:0]   r_cip_addr_next = w_ram_ce ? w_ram_addr : r_cip_addr; 

wire            r_wr_katt_next = w_rdkd_done ; 

wire    wr_kram = (cscnt < 5'h8) | r_wr_katt;

wire [4:0] cip_addr = hw_boot_ok0 ? r_cip_addr[4:0] : w_ram_addr[4:0];

assign kbuf_A  = cip_addr        ; 
assign kbuf_DI = w_ram_data      ; 

assign w_ram_we_n      = ~( {4{r_dec_done & (|r_cntr_dly)}} );

wire r_hw_boot_done_next = o_hw_boot_done;

wire w_reset_trng_n = reset_trng_fw_n;

wire [7:0] kbuf_att = i_offset_att[7:0] + ({2'h0,r_ld_key_cntr} - 8'h1);

assign o_hrdata              = 32'h0                        ;
assign o_hready              = 1'h1                         ;
assign o_hresp               = 2'h0                         ;

assign o_hw_boot_done        = hw_boot_ok | hw_boot_err     ;
assign o_hw_boot_ok          = hw_boot_ok                   ;
assign o_hw_boot_err         = hw_boot_err                  ; 
assign o_hw_boot_done_pulse  = o_hw_boot_done & ~r_hw_boot_done ;
assign o_lc_undef            = undefine_mode                ; 
assign o_lc_destroy          = destroy_mode                 ; 
assign o_lc_debug            = debug_mode                   ; 
assign o_lc_user             = user_mode                    ; 
assign o_lc_manu             = manufacture_mode             ; 
assign o_lc_dev2             = develop2_mode                ; 
assign o_lc_dev              = develop_mode                 ; 
assign o_lc_test             = test_mode                    ; 
assign o_life_cycle          = life_cycle                   ;
assign o_rdbi_st_done        = r_rdbi_st_done               ;
assign o_otp_cs              = r_otp_cs                     ;
assign o_otp_addr            = {20'h3300_0 ,otp_addr}   ;
assign o_crc_err             = l_crc_err                    ;
assign o_trng_rd_en          = r_trng_rd_en                 ;
assign o_trng_skip_stp       = trng_self_test_skip          ;
assign o_trng_sclk_sel       = trng_sclk_sel                ;
assign o_trng_to_en          = 1'h0                         ;
assign o_bus_cipher_en       = bus_cipher_en & update_n     ;
assign o_bus_cipher_key      = r_trng_data[32*0 +: 32]      ;
assign o_bus_cipher_nce      = r_trng_data[32*1 +: 32]      ;
assign o_ram_cipher_en       = ram_cipher_en & update_n     ; 
assign o_ram_cipher_key      = r_trng_data[32*2 +: 32]      ;
assign o_ram_cipher_nce      = r_trng_data[32*3 +: 32]      ;
assign o_nvm_cipher_bypass   = nvm_cipher_bypass & update_n     ;
assign o_last_otp            = {2'h0,r_ld_key_cntr[5:0]}    ;
assign o_kbuf_CSB            = ~init_ram_done ? 1'h0      : ~wr_kram  ;
assign o_kbuf_A              = ~init_ram_done ? icnt[7:0] : r_wr_katt ? kbuf_att : {kbuf_A,cscnt[2:0]}     ;
assign o_kbuf_WEB            = ~init_ram_done ? 4'h0      : r_wr_katt ? 4'h0     : ({4{~wr_kram}} | r_ceb) ;
assign o_kbuf_DI             = ~init_ram_done ? 32'h0     : kram_wd   ;
assign o_random_clock_en_set = random_clock_en_set          ; 
assign o_con_field0          = con_field0                   ; 
assign o_con_field1          = con_field1                   ; 
assign o_con_field2          = con_field2                   ; 
assign o_con_field3          = con_field3                   ; 
assign o_con_field4          = con_field4                   ; 
assign o_con_field5          = con_field5                   ; 

assign o_patch_en            = ipatch_en                    ;
assign o_patch_info_vld      = w_rdbi_rd_ack                ;
assign o_patch_info_addr     = otp_addr                     ;
assign o_patch_otp_rdata     = i_otp_rd_data                ;
assign o_reset_trng_warning  = reset_trng_warning           ;
assign o_reset_trng_error    = reset_trng_error             ;
assign o_reset_trng_n        = w_reset_trng_n               ;
assign o_clc_key             = {r_trng_data[32*5 +: 32],r_trng_data[32*4 +: 32]};

osr_seip_kdec #(
   .p_FSRK_ID      (0                        ),
   .p_SSRK_ID      (4                        ) 
)u_seip_kdec(
   .rst_n          (i_sys_rst_n              ),
   .clk            (i_boot_clk               ),
   .i_scan_mode    (i_scan_mode              ),

   .i_bc2kdf_go    (bc2kdf_go                ),
   .i_bc2kdf_done  (bc2kdf_done              ),
   .o_dec_done     (kdf_dec_done             ),
   .i_sel_root_key (sel_root_key             ),
   .o_crc_err_set  (crc_err_set              ),
   .o_crc_ok_set   (crc_ok_set               ),
   .i_bc2kdf_alg   (key_alg_sel              ),
   .i_bc2kdf_ekey  (w_otp_ekey               ),
   .i_bc2kdf_crc   (w_otp_crc                ),
   .i_clc_key      ({r_trng_data[32*5 +: 32],r_trng_data[32*4 +: 32]}),
   .i_bc2kdf_mask  (256'h0                   ),
   .o_masked_key   (kdf2bc_key               ) 
);

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin : init_kmu
    if(!i_sys_rst_n) begin
        init_ram_done        <= 1'h1                      ; 
        icnt                 <= 12'hFFF                   ; 
    end else begin
        init_ram_done        <= init_ram_done_next        ; 
        icnt                 <= icnt_next                 ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin
    if(!i_sys_rst_n) begin
        r_keep_dec_done      <= 1'h0                      ; 
        mem_init_done_f      <= 1'h0                      ; 
        mem_init_done_ff     <= 1'h0                      ; 
        hw_boot_ok0          <= 1'h0                      ; 
        hw_boot_ok1          <= 1'h0                      ; 
        hw_boot_ok2          <= 1'h0                      ; 
        hw_boot_ok3          <= 1'h0                      ; 
        hw_boot_ok           <= 1'h0                      ; 
        hw_boot_err          <= 1'h0                      ; 
        r_dec_done           <= 1'h0                      ; 
        current_state        <= IDLE                      ; 
        con_field0           <= 32'h0                     ; 
        con_field1           <= 32'h0                     ; 
        con_field2           <= 32'h0                     ; 
        con_field3           <= 32'h0                     ; 
        con_field4           <= 32'h0                     ; 
        con_field5           <= 32'h0                     ; 
    end else begin
        r_keep_dec_done      <= r_keep_dec_done_next      ; 
        mem_init_done_f      <= mem_init_done_f_next      ; 
        mem_init_done_ff     <= mem_init_done_ff_next     ; 
        hw_boot_ok0          <= hw_boot_ok0_next          ; 
        hw_boot_ok1          <= hw_boot_ok1_next          ; 
        hw_boot_ok2          <= hw_boot_ok2_next          ; 
        hw_boot_ok3          <= hw_boot_ok3_next          ; 
        hw_boot_ok           <= hw_boot_ok_next           ; 
        hw_boot_err          <= hw_boot_err_next          ; 
        r_dec_done           <= r_dec_done_next           ; 
        current_state        <= current_state_next        ; 
        con_field0           <= con_field0_next           ; 
        con_field1           <= con_field1_next           ; 
        con_field2           <= con_field2_next           ; 
        con_field3           <= con_field3_next           ; 
        con_field4           <= con_field4_next           ; 
        con_field5           <= con_field5_next           ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin : key_attribute
    if(!i_sys_rst_n) begin
        r_otp_key0           <= 32'h7577_c168             ; 
        r_otp_key1           <= 32'h231                   ; 
        r_otp_key2           <= 32'h7fe0                  ; 
        r_otp_key3           <= 32'h3f8                   ; 
        r_otp_key4           <= 32'hff_0000               ; 
        r_otp_key5           <= 32'hb040_637b             ; 
        r_otp_key6           <= 32'h23d                   ; 
        r_otp_key7           <= 32'h249                   ; 
        r_otp_kcrc           <= 32'h0                     ; 
        kram_wd              <= 32'h0                     ; 
    end else begin
        r_otp_key0           <= r_otp_key0_next           ; 
        r_otp_key1           <= r_otp_key1_next           ; 
        r_otp_key2           <= r_otp_key2_next           ; 
        r_otp_key3           <= r_otp_key3_next           ; 
        r_otp_key4           <= r_otp_key4_next           ; 
        r_otp_key5           <= r_otp_key5_next           ; 
        r_otp_key6           <= r_otp_key6_next           ; 
        r_otp_key7           <= r_otp_key7_next           ; 
        r_otp_kcrc           <= r_otp_kcrc_next           ; 
        kram_wd              <= kram_wd_next              ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin
    if(!i_sys_rst_n) begin
        test_mode            <= 1'h0                      ; 
        develop_mode         <= 1'h0                      ; 
        develop2_mode        <= 1'h0                      ; 
        manufacture_mode     <= 1'h0                      ; 
        user_mode            <= 1'h0                      ; 
        debug_mode           <= 1'h0                      ; 
        destroy_mode         <= 1'h0                      ; 
        undefine_mode        <= 1'h0                      ; 
        life_cycle           <= 32'h0                     ; 
        random_clock_en_set  <= 1'h0                      ; 
        key_att              <= 32'hFFFF_FFFF             ; 
        r_crc_err            <= 1'h0                      ; 
        l_crc_err            <= 1'h0                      ; 
        r_rdbi_st_done       <= 1'h0                      ; 
        r_kskdf_st           <= 1'h0                      ; 
    end else begin
        test_mode            <= test_mode_next            ; 
        develop_mode         <= develop_mode_next         ; 
        develop2_mode        <= develop2_mode_next        ; 
        manufacture_mode     <= manufacture_mode_next     ; 
        user_mode            <= user_mode_next            ; 
        debug_mode           <= debug_mode_next           ; 
        destroy_mode         <= destroy_mode_next         ; 
        undefine_mode        <= undefine_mode_next        ; 
        life_cycle           <= life_cycle_next           ; 
        random_clock_en_set  <= random_clock_en_set_next  ; 
        key_att              <= key_att_next              ; 
        r_crc_err            <= r_crc_err_next            ; 
        l_crc_err            <= l_crc_err_next            ; 
        r_rdbi_st_done       <= r_rdbi_st_done_next       ; 
        r_kskdf_st           <= r_kskdf_st_next           ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin : get_trng
    if(!i_sys_rst_n) begin
        reset_trng_fw_n      <= 1'h0                      ; 
    end else begin
        reset_trng_fw_n      <= reset_trng_fw_n_next      ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin
    if(!i_sys_rst_n) begin
        r_otp_rd_addr        <= 10'b0 ; 
        r_otp_cs             <= 1'h0                      ; 
        r_rdkd_cntr          <= 4'h0                      ; 
        r_ld_key_cntr        <= 6'h0                      ; 
        r_bc2kdf_done        <= 1'h0                      ; 
        update_n             <= upd_dfl                   ; 
        r_cntr_dly           <= 6'h0                      ; 
    end else begin
        r_otp_rd_addr        <= r_otp_rd_addr_next        ; 
        r_otp_cs             <= r_otp_cs_next             ; 
        r_rdkd_cntr          <= r_rdkd_cntr_next          ; 
        r_ld_key_cntr        <= r_ld_key_cntr_next        ; 
        r_bc2kdf_done        <= r_bc2kdf_done_next        ; 
        update_n             <= update_n_next             ; 
        r_cntr_dly           <= r_cntr_dly_next           ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin : rw_kram
    if(!i_sys_rst_n) begin
        kcrc_ok              <= 1'h0                      ; 
        cscnt                <= 5'h8                      ; 
        r_kbuf_A             <= 5'h0                      ; 
        r_ceb                <= 4'hf                      ; 
        r_cip_addr           <= 8'h0                      ; 
        r_wr_katt            <= 1'h0                      ; 
    end else begin
        kcrc_ok              <= kcrc_ok_next              ; 
        cscnt                <= cscnt_next                ; 
        r_kbuf_A             <= r_kbuf_A_next             ; 
        r_ceb                <= r_ceb_next                ; 
        r_cip_addr           <= r_cip_addr_next           ; 
        r_wr_katt            <= r_wr_katt_next            ; 
    end
end

always @(posedge i_boot_clk or negedge i_sys_rst_n) begin
    if(!i_sys_rst_n) begin
        r_hw_boot_done       <= 1'h0                      ; 
    end else begin
        r_hw_boot_done       <= r_hw_boot_done_next       ; 
    end
end

endmodule 
