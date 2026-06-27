//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_kmu (
    input  wire                 i_hclk                  ,
    input  wire                 i_hresetn               ,
    input  wire                 i_hsel                  ,
    input  wire [31:0]          i_haddr                 ,
    input  wire [ 1:0]          i_htrans                ,
    input  wire                 i_hwrite                ,
    input  wire [ 2:0]          i_hburst                ,
    input  wire [ 2:0]          i_hsize                 ,
    input  wire [ 3:0]          i_hprot                 ,
    input  wire                 i_hmastlock             ,
    input  wire                 i_hready                ,
    input  wire [31:0]          i_hwdata                ,
    output wire [31:0]          o_hrdata                ,
    output wire [ 1:0]          o_hresp                 ,
    output wire                 o_hreadyout             ,

    input  wire                 i_scan_mode             ,    

    input  wire [11:0]          i_offset_att            ,
    input  wire [7:0]           i_last_otp              ,

    output wire                 o_kbuf_CSB              ,        
    output wire [7:0]           o_kbuf_A                ,
    output wire [3:0]           o_kbuf_WEB              ,        
    output wire [31:0]          o_kbuf_DI               ,        
    input  wire [31:0]          i_kbuf_DO               ,   

    output wire                 o_sp_set_hash_key       , 
    output wire                 o_sp_set_hash_key_v1    , 
    output wire                 o_sp_set_ske_key        ,
    output wire                 o_sp_set_ske_key_v1     ,
    output wire                 o_sp_set_chacha_key     ,
    output wire [255:0]         o_sp_key                ,

    input  wire                 i_clear_key             ,

    output wire                 o_key_vld               ,
    output wire [255:0]         o_key_data              ,
    output wire [15:0]          o_key_index             ,

    input  wire [63:0]          i_clc_key               ,

    input  wire                 i_lc_undef              , 
    input  wire                 i_lc_destroy            ,
    input  wire                 i_lc_debug              ,
    input  wire                 i_lc_user               ,
    input  wire                 i_lc_manu               ,
    input  wire                 i_lc_dev                ,
    input  wire                 i_lc_dev2               ,
    input  wire                 i_lc_test                
);

localparam [17:0] P_KMU_CTRL       = 18'h00 ;
localparam [17:0] P_KMU_EXT        = 18'h03 ;
localparam [17:0] P_KMU_KEN        = 18'h30 ;

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

localparam P_KEY_DEF              = 256'h5036E910_951F268C_357B895A_F58D6A13_B8916308_6158C027_59E15280_610235F8 ;

wire [11:0] katt_end = 12'd256/12'd10;

wire        pke_key_nxt;

reg kek_ff          ;
reg kek_fe          ;

reg is_kram_key     ;

reg to_ske          ; 
reg to_hash         ; 
reg to_chacha       ; 
reg to_pke          ; 
reg to_hash_1       ; 
reg to_ske_1        ; 

reg [31:0]      hrdata        ;
reg             r_hsel        ;
reg [19:0]      r_haddr       ;
reg [ 1:0]      r_htrans      ;
reg             r_hwrite      ;
reg [ 2:0]      r_hsize       ;
reg [31:0]      r_hwdata      ;
reg             wr_en         ;
reg             r_wr_en       ;
reg [17:0]      reg_addr      ;
reg             kmu_ctrl_hit  ;
reg             kmu_ext_hit   ;
reg             kmu_ken_hit   ;
reg [15:0]      key_sel       ;
reg [3:0]       in_port_sel   ;
reg [15:0]      key_index     ;
reg [31:0]      key_en        ;
reg             tr_key_ing    ;
reg [7:0]       a_att         ;
reg [1:0]       acnt          ;
reg [31:0]      key_att       ;
reg             r_rd_en       ;
reg [3:0]       cscnt         ;
reg             r_kram_csb    ;
reg             keep_key      ;
reg [255:0]     kbuf_DO_i256  ;
reg             set_key_valid ;
reg             set_key       ;
reg             r_hready      ;

wire    cscnt1 = (cscnt == 4'h1); 
wire    cscnt2 = (cscnt == 4'h2); 
wire    cscnt3 = (cscnt == 4'h3); 
wire    cscnt4 = (cscnt == 4'h4); 
wire    cscnt5 = (cscnt == 4'h5); 
wire    cscnt6 = (cscnt == 4'h6); 
wire    cscnt7 = (cscnt == 4'h7); 
wire    cscnt8 = (cscnt == 4'h8); 

reg  [31:0] hrdata_next ;

wire            r_hsel_next       = i_hsel    ; 
wire [19:0]     r_haddr_next      = i_haddr[19:0]  ; 
wire [ 1:0]     r_htrans_next     = i_htrans  ; 
wire            r_hwrite_next     = i_hwrite  ; 
wire [ 2:0]     r_hsize_next      = i_hsize   ; 
wire [31:0]     r_hwdata_next     = i_hwdata  ; 

wire            bus_vld0          = i_hsel & i_htrans[1] & (i_hsize==3'h2);

wire            bus_vld          = r_hsel & r_htrans[1] & (r_hsize==3'h2);
wire            wr_en_next       = bus_vld & r_hwrite;   
wire            r_wr_en_next     = wr_en;                
wire            rd_en            = bus_vld & ~r_hwrite; 
wire  [17:0]    reg_addr_next    = bus_vld ? r_haddr[19:2] : reg_addr; 

wire         kmu_ctrl_hit_next   = (reg_addr_next[17:0] == P_KMU_CTRL  ) ;
wire         kmu_ext_hit_next    = (reg_addr_next[17:0] == P_KMU_EXT   ) ;
wire         kmu_ken_hit_next    = (reg_addr_next[17:0] == P_KMU_KEN   ) ;

wire         wr_kmu_ctrl = wr_en & kmu_ctrl_hit ; 
wire         wr_kmu_ext  = wr_en & kmu_ext_hit  ; 
wire         wr_kmu_ken  = wr_en & kmu_ken_hit  ; 

wire [15:0]  key_sel_next      = wr_kmu_ctrl  ? r_hwdata[31:16] : key_sel    ;
wire [3:0]   in_port_sel_next  = wr_kmu_ctrl  ? r_hwdata[7:4]   : in_port_sel ;

wire [15:0]  key_index_next  = wr_kmu_ext   ? r_hwdata[15:0] : key_index    ;

wire [31:0]  key_en_next     = wr_kmu_ken   ? (r_hwdata[31:0] & key_en) : key_en       ;

reg          tr_en;
always @(*) begin
    case(key_sel[7:0])
        8'd0    : tr_en = key_en[0 ]; 
        8'd1    : tr_en = key_en[1 ];
        8'd2    : tr_en = key_en[2 ];
        8'd3    : tr_en = key_en[3 ];
        8'd4    : tr_en = key_en[4 ];
        8'd5    : tr_en = key_en[5 ];
        8'd6    : tr_en = key_en[6 ];
        8'd7    : tr_en = key_en[7 ];
        8'd8    : tr_en = key_en[8 ];
        8'd9    : tr_en = key_en[9 ];
        8'd10   : tr_en = key_en[10];
        8'd11   : tr_en = key_en[11];
        8'd12   : tr_en = key_en[12];
        8'd13   : tr_en = key_en[13];
        8'd14   : tr_en = key_en[14];
        8'd15   : tr_en = key_en[15];
        8'd16   : tr_en = key_en[16];
        8'd17   : tr_en = key_en[17];
        8'd18   : tr_en = key_en[18];
        8'd19   : tr_en = key_en[19];
        8'd20   : tr_en = key_en[20];
        8'd21   : tr_en = key_en[21];
        8'd22   : tr_en = key_en[22];
        8'd23   : tr_en = key_en[23];
        8'd24   : tr_en = key_en[24];
        8'd25   : tr_en = key_en[25];
        8'd26   : tr_en = key_en[26];
        8'd27   : tr_en = key_en[27];
        8'd28   : tr_en = key_en[28];
        8'd29   : tr_en = key_en[29];
        8'd30   : tr_en = key_en[30];
        8'd31   : tr_en = key_en[31];
        default : tr_en = 1'h0 ;
    endcase
end 

wire start_transport = tr_en & wr_kmu_ctrl & r_hwdata[0];

wire tr_key_ing_next = (start_transport & pke_key_nxt) ? 1'h1 : i_clear_key ? 1'h0 : tr_key_ing ; 

wire        nd_att_k    = bus_vld0 ? (i_haddr[14:13] == 2'h1) : 1'h0 ;  
wire        nd_att_a    = bus_vld0 ? (i_haddr[14:13] == 2'h2) : 1'h0 ;  

wire [7:0]  a_att_next  = nd_att_k ? (i_haddr[12:5] + i_offset_att[7:0]) :            
                          nd_att_a ? (i_haddr[9:2]  + i_offset_att[7:0]) : a_att  ;  

wire        ud_att      = nd_att_k ? ((i_haddr[12:5] + i_offset_att[7:0]) != a_att) :
                          nd_att_a ? ((i_haddr[9:2]  + i_offset_att[7:0]) != a_att) : 1'b0  ;  

wire [1:0]  acnt_next = ud_att ? 2'h0 : (acnt == 2'h3) ? 2'h3 : (acnt + 2'h1) ;

wire [31:0] key_att_next = (acnt == 2'h1) ? i_kbuf_DO : key_att ;

wire    notkey  = (reg_addr_next[11] == 1'h0);
wire    iskey   = (reg_addr_next[17:11] == 7'h1);
wire    isatt   = (reg_addr_next[17:11] == 7'h2);

wire    key0    = ((11'h000 <= reg_addr_next[10:0]) & (reg_addr_next[10:0] < 11'h008) & iskey) | ((11'h000 == reg_addr_next[10:0]) & isatt);
wire    key1    = ((11'h008 <= reg_addr_next[10:0]) & (reg_addr_next[10:0] < 11'h010) & iskey) | ((11'h001 == reg_addr_next[10:0]) & isatt);
wire    key2    = ((11'h010 <= reg_addr_next[10:0]) & (reg_addr_next[10:0] < 11'h018) & iskey) | ((11'h002 == reg_addr_next[10:0]) & isatt);

wire    sec0    = ((11'h010 <= reg_addr_next[10:0]) & (reg_addr_next[10:0] < 11'h018) & iskey) ;

wire    keyuser = ( ({3'h0,i_last_otp} <= reg_addr_next[10:0]) & (reg_addr_next[10:0] < katt_end[10:0]    ) & isatt ) | 
                  ( ({i_last_otp,3'h0} <= reg_addr_next[10:0]) & (reg_addr_next[10:0] < i_offset_att[10:0]) & iskey ) ; 

wire    keyoa   = ( ({3'h0,i_last_otp} >  reg_addr_next[10:0])                                          & isatt ) |
                  ( ({i_last_otp,3'h0} >  reg_addr_next[10:0])                                          & iskey ) ;

wire    keyo    = keyoa & ~(key0 | key1 | key2) ; 

function rdken;
    input [3:0] key_typ           ;
    input [3:0] key_lev           ;
    input [3:0] key_lc            ;
    input       manu_mode         ;
    input       user_debug_mode   ;
    rdken = (key_lc == P_KEY_LC_UNBURNED) ? 1'h1                                                                    : 
            manu_mode                     ? ((key_lev != P_KEY_LEVEL_1) ? 1'h1 : (key_typ != P_KEY_TYPE_SYMMETRIC)) :
            user_debug_mode               ? (key_typ != P_KEY_TYPE_SYMMETRIC)                                       : 1'h1 ;
endfunction

wire ud_mode = i_lc_user | i_lc_debug;

wire rw_k0 = 1'h0;

wire rw_k1 = 1'h0;

wire    canread_k  = ( notkey                                                                  ) |
                     ( key0 & rw_k0                                                            ) | 
                     ( key1 & rw_k1                                                            ) |
                     ( keyo & rdken(key_att[11:8],key_att[7:4],key_att[3:0],(i_lc_dev2 | i_lc_manu),ud_mode) ) |
                     ( keyuser                                                                 ) ;

wire root_key_can_rw = key0 ? rw_k0 : key1 ? rw_k1 : 1'h1;

wire key_addr_ok = key0 | key1 | key2 | keyo | keyuser ;

wire canread = sec0 ? 1'h0 : isatt ? key_addr_ok : ( (canread_k | (root_key_can_rw & (i_lc_test | i_lc_dev) & key_addr_ok)) & ~(i_lc_destroy | i_lc_undef) );

function wrken;
    input [3:0] key_lev     ;
    input [3:0] key_lc      ;
    input       manu_mode   ;                     
    wrken = (key_lc == P_KEY_LC_UNBURNED) ? 1'h1 : (manu_mode & (key_lev != P_KEY_LEVEL_1)) ;
endfunction

wire dev2_manu  = i_lc_dev2 | i_lc_manu;

wire canwrite_k = ( key0 & rw_k0                                      ) | 
                  ( key1 & rw_k1                                      ) |
                  ( keyo & wrken(key_att[7:4],key_att[3:0],dev2_manu) ) |                  
                  ( keyuser                                           ) ;

wire noatt_addr_ok = key_addr_ok & iskey ;

wire canwrite = ~(i_lc_destroy | i_lc_undef)                       &  
                     ( (root_key_can_rw & (i_lc_test | i_lc_dev | i_lc_debug) & noatt_addr_ok) | canwrite_k ) ;

wire kbuf_wra = r_wr_en & isatt & keyuser ;
wire kbuf_wrk = sec0 ? 1'h0 : (r_wr_en & iskey & canwrite);
wire kbuf_wr = kbuf_wra | kbuf_wrk ;

wire r_rd_en_next = rd_en ;

wire kbuf_rd_a = r_rd_en & isatt ;
wire kbuf_rd_k = r_rd_en & iskey ;

wire kbuf_rd = kbuf_rd_a | kbuf_rd_k ;

always @(*) begin
    case(r_hwdata[23:16])
        8'hff   : begin
            is_kram_key = 1'h0  ; 
        end
        8'hfe   : begin
            is_kram_key = 1'h0  ; 
        end
        default : begin
            is_kram_key = 1'h1  ; 
        end
    endcase
end 

wire    kmu_start_transport = start_transport & is_kram_key ;

wire    [3:0]   cscnt_next = kmu_start_transport ? 4'h0 : 
                             cscnt8              ? 4'h8 : (cscnt + 4'h1) ;

wire    r_kram_csb_next = kmu_start_transport ? 1'h0 : 
                          cscnt7              ? 1'h1 : r_kram_csb ;
assign  pke_key_nxt   = 1'h0 ;
wire    keep_key_next = (set_key & pke_key_nxt) ? 1'h1 : i_clear_key ? 1'h0 : keep_key ;
wire    kp_key = keep_key | set_key;
wire [255:0] kbuf_DO_i256_next = cscnt1  ? {kbuf_DO_i256[255: 32], i_kbuf_DO}                       : 
                                 cscnt2  ? {kbuf_DO_i256[255: 64], i_kbuf_DO, kbuf_DO_i256[31:0]}   :
                                 cscnt3  ? {kbuf_DO_i256[255: 96], i_kbuf_DO, kbuf_DO_i256[63:0]}   :
                                 cscnt4  ? {kbuf_DO_i256[255:128], i_kbuf_DO, kbuf_DO_i256[95:0]}   :
                                 cscnt5  ? {kbuf_DO_i256[255:160], i_kbuf_DO, kbuf_DO_i256[127:0]}  :
                                 cscnt6  ? {kbuf_DO_i256[255:192], i_kbuf_DO, kbuf_DO_i256[159:0]}  :
                                 cscnt7  ? {kbuf_DO_i256[255:224], i_kbuf_DO, kbuf_DO_i256[191:0]}  :
                                 ((set_key & ~pke_key_nxt) | i_clear_key) ? {4{i_clc_key}}: 
                                 kp_key  ? kbuf_DO_i256[255:0]                                      :
                                           {                       i_kbuf_DO, kbuf_DO_i256[223:0]}  ;

wire    set_key_valid_next = kmu_start_transport ? 1'h1 : 
                             cscnt8          ? 1'h0 : set_key_valid;
wire    set_key_next = set_key_valid & cscnt8 ; 

wire clr_hready = (bus_vld & wr_en_next)    |   
                  wr_en                     |
                  kmu_start_transport       |  
                  (rd_en                  ) ;  
wire    pke_key = 1'h0 ;
wire set_hready = (~pke_key | ~tr_key_ing) ? cscnt8 : i_clear_key ; 
wire r_hready_next = bus_vld0   ? 1'h0 : 
                     clr_hready ? 1'h0 :
                     kbuf_rd    ? 1'h0 :
                     set_hready ? 1'h1 : r_hready;

always @(*) begin
    case(r_hwdata[23:16])
        8'hff   : begin
            kek_ff      = 1'h1  ; 
            kek_fe      = 1'h0  ;      
        end
        8'hfe   : begin
            kek_ff      = 1'h0  ; 
            kek_fe      = 1'h1  ;      
        end
        default : begin
            kek_ff      = 1'h0  ; 
            kek_fe      = 1'h0  ;      
        end
    endcase
end 

wire transport_chip_root_key = 1'h0 ;

wire transport_device_root_key = 1'h0 ; 

wire    i_boot_key_vld = 1'h0   ;

wire trk0 = (key_sel[7:0] == 8'h0); 
wire trk1 = (key_sel[7:0] == 8'h1); 

wire transport_ex_key     = trk0 ? transport_chip_root_key   :
                            trk1 ? transport_device_root_key : 1'h1 ;

wire [255:0] tr_key = key_sel[8] ? {kbuf_DO_i256[127:0], kbuf_DO_i256[255:128]} : kbuf_DO_i256;

wire         set_key_ex   = transport_ex_key & set_key & key_sel[15] ;
wire [255:0] key_ex       = {256{set_key_ex}} & tr_key ;
wire [15:0]  key_index_ex = {16{set_key_ex}} & key_index ;

wire transmit_ramkey_pulse = set_key  & ~key_sel[15] ;

wire [255:0] no_meaning       = 256'hf13b90ac_b2a91ed0_96ac87b6_45bde0f9_f13b90ac_b2a91ed0_96ac87b6_45bde6f9 ;

wire [255:0] scan_mode_rtl_key = {`OSR_SCAN_RTL_KEY,`OSR_SCAN_RTL_KEY} ;

wire [255:0] rtl_key          = i_scan_mode ? scan_mode_rtl_key : 256'hf13b90ac_b2a91ed0_96ac87b6_45bde0f9_f13b90ac_b2a91ed0_96ac87b6_45bde6f9 ;

wire         w_ex_set_ske_key =  (wr_kmu_ctrl & r_hwdata[0]) & ((kek_ff & (i_lc_test | i_lc_dev | i_lc_dev2)) |
                                                                (kek_fe & (i_lc_test | i_lc_dev | i_lc_dev2 | i_lc_manu)));

wire [255:0] kek_ehsm_mux = i_scan_mode ? scan_mode_rtl_key : {`OSR_RTL_KEY_INSTALL_KEK_EHSM,`OSR_RTL_KEY_INSTALL_KEK_EHSM} ;
wire [255:0] kek_soc_mux = i_scan_mode ? scan_mode_rtl_key : {`OSR_RTL_KEY_INSTALL_KEK_SOC ,`OSR_RTL_KEY_INSTALL_KEK_SOC } ;

wire [255:0] w_ex_key         = kek_ff ? kek_ehsm_mux : 
                                kek_fe ? kek_soc_mux : no_meaning ;       

wire [255:0] key_to_internal_sp = i_boot_key_vld    ? rtl_key       : 
                                  w_ex_set_ske_key  ? w_ex_key      : tr_key          ;

wire transmit_kek_pulse = w_ex_set_ske_key & ~key_sel_next[15];

always @(*) begin
    to_ske      = 1'h0  ; 
    to_hash     = 1'h0  ; 
    to_chacha   = 1'h0  ; 
    to_pke      = 1'h0  ;
    to_hash_1   = 1'h0  ; 
    to_ske_1    = 1'h0  ; 
    case(in_port_sel_next)
        4'h0    : begin 
            to_ske      = transmit_ramkey_pulse | transmit_kek_pulse | i_boot_key_vld ; 
        end
        4'h1    : begin
            to_hash     = transmit_ramkey_pulse                      ; 
        end
        4'h2    : begin
            to_chacha   = transmit_ramkey_pulse                      ; 
        end
        default : begin

        end
    endcase
end

always @(*) begin : ahb_rw
    if(iskey | isatt) begin
        hrdata_next = i_kbuf_DO[31:0];
    end else begin
        case(reg_addr)
            P_KMU_CTRL  : hrdata_next = {key_sel,8'h0,in_port_sel,4'h0} ;
            P_KMU_EXT   : hrdata_next = {16'b0,key_index}               ;
            P_KMU_KEN   : hrdata_next = key_en                          ;
            default     : hrdata_next = 32'h0                           ;
        endcase
    end
end

assign o_hrdata             = canread ? hrdata : 32'h0;
assign o_hresp              = 2'h0;
assign o_hreadyout          = r_hready ;
assign o_kbuf_CSB           = ~(acnt == 2'h0) & r_kram_csb & ~kbuf_wr & ~kbuf_rd;
assign o_kbuf_A             = (acnt == 2'h0)            ? a_att         : 
                              (kbuf_rd_a | kbuf_wra)    ? a_att         : 
                              (kbuf_rd_k | kbuf_wrk)    ? reg_addr[7:0] : {key_sel[4:0],cscnt[2:0]};
assign o_kbuf_WEB           = {4{~kbuf_wr}}; 
assign o_kbuf_DI            = r_hwdata; 

assign o_sp_set_hash_key    = to_hash               ;
assign o_sp_set_hash_key_v1 = to_hash_1             ;
assign o_sp_set_ske_key     = to_ske                ; 
assign o_sp_set_ske_key_v1  = to_ske_1              ;
assign o_sp_set_chacha_key  = to_chacha             ;

assign o_sp_key             = key_to_internal_sp    ; 
assign o_key_vld            = set_key_ex            ;
assign o_key_data           = key_ex                ;
assign o_key_index          = key_index_ex          ;

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        hrdata        <= 32'h0              ; 
        r_hsel        <= 1'h0               ; 
        r_haddr       <= 20'h0              ; 
        r_htrans      <= 2'h0               ; 
        r_hwrite      <= 1'h0               ; 
        r_hsize       <= 3'h0               ; 
        r_hwdata      <= 32'h0              ; 
        wr_en         <= 1'h0               ; 
        r_wr_en       <= 1'h0               ; 
        reg_addr      <= 18'h0              ; 
        kmu_ctrl_hit  <= 1'h0               ; 
        kmu_ext_hit   <= 1'h0               ; 
        kmu_ken_hit   <= 1'h0               ; 
        key_sel       <= 16'h0              ; 
        in_port_sel   <= 4'h0               ; 
        key_index     <= 16'h0              ; 
        key_en        <= 32'hFFFF_FFFF      ; 
        tr_key_ing    <= 1'h0               ; 
    end else begin
        hrdata        <= hrdata_next        ; 
        r_hsel        <= r_hsel_next        ; 
        r_haddr       <= r_haddr_next       ; 
        r_htrans      <= r_htrans_next      ; 
        r_hwrite      <= r_hwrite_next      ; 
        r_hsize       <= r_hsize_next       ; 
        r_hwdata      <= r_hwdata_next      ; 
        wr_en         <= wr_en_next         ; 
        r_wr_en       <= r_wr_en_next       ; 
        reg_addr      <= reg_addr_next      ; 
        kmu_ctrl_hit  <= kmu_ctrl_hit_next  ; 
        kmu_ext_hit   <= kmu_ext_hit_next   ; 
        kmu_ken_hit   <= kmu_ken_hit_next   ; 
        key_sel       <= key_sel_next       ; 
        in_port_sel   <= in_port_sel_next   ; 
        key_index     <= key_index_next     ; 
        key_en        <= key_en_next        ; 
        tr_key_ing    <= tr_key_ing_next    ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        a_att         <= 8'h0               ; 
        acnt          <= 2'h3               ; 
        key_att       <= 32'hFFFF_FFFF      ; 
        r_rd_en       <= 1'h0               ; 
        cscnt         <= 4'h8               ; 
        r_kram_csb    <= 1'h1               ; 
        keep_key      <= 1'h0               ; 
        kbuf_DO_i256  <= P_KEY_DEF          ; 
        set_key_valid <= 1'h0               ; 
        set_key       <= 1'h0               ; 
        r_hready      <= 1'h1               ; 
    end else begin
        a_att         <= a_att_next         ; 
        acnt          <= acnt_next          ; 
        key_att       <= key_att_next       ; 
        r_rd_en       <= r_rd_en_next       ; 
        cscnt         <= cscnt_next         ; 
        r_kram_csb    <= r_kram_csb_next    ; 
        keep_key      <= keep_key_next      ; 
        kbuf_DO_i256  <= kbuf_DO_i256_next  ; 
        set_key_valid <= set_key_valid_next ; 
        set_key       <= set_key_next       ; 
        r_hready      <= r_hready_next      ; 
    end
end

endmodule 
