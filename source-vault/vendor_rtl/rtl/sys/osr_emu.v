//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_emu (

    input  wire         i_hclk                  ,
    input  wire         i_hresetn               ,

    input  wire         i_hsel                  ,
    input  wire [9:0]   i_haddr                 ,
    input  wire [1:0]   i_htrans                ,
    input  wire         i_hwrite                ,
    input  wire [2:0]   i_hsize                 ,
    input  wire [31:0]  i_hwdata                ,
    input  wire         i_hready                ,
    input  wire [2:0]   i_hburst                , 
    input  wire [3:0]   i_hprot                 ,
    input  wire         i_hmastlock             ,

    output wire [31:0]  o_hrdata                ,
    output wire [1:0]   o_hresp                 ,
    output wire         o_hreadyout             ,

    input  wire [31:0]  i_sensor                ,
    input  wire [31:0]  i_soc_err               ,

    input  wire         i_mem_ecc_1b_irom       ,   
    input  wire         i_mem_ecc_1b_iram       ,   
    input  wire         i_mem_ecc_1b_dram       ,   
    input  wire         i_mem_ecc_1b_kmu        ,   
    input  wire         i_mem_ecc_1b_pke0       ,   
    input  wire         i_mem_ecc_1b_pke1       ,   
    input  wire         i_mem_ecc_1b_pke2       ,   
    input  wire         i_mem_ecc_1b_pke3       ,   

    input  wire         i_mem_ecc_mb_irom       ,   
    input  wire         i_mem_ecc_mb_iram       ,   
    input  wire         i_mem_ecc_mb_dram       ,   
    input  wire         i_mem_ecc_mb_kmu        ,   
    input  wire         i_mem_ecc_mb_pke0       ,   
    input  wire         i_mem_ecc_mb_pke1       ,   
    input  wire         i_mem_ecc_mb_pke2       ,   
    input  wire         i_mem_ecc_mb_pke3       ,   

    input  wire [14-1:0]  i_mem_ecc_addr_irom     ,   
    input  wire [16-1:0]  i_mem_ecc_addr_iram     ,   
    input  wire [14-1:0]  i_mem_ecc_addr_dram     ,   
    input  wire [16:0]                       i_mem_ecc_addr_kmu      ,   
    input  wire [16:0]                       i_mem_ecc_addr_pke0     ,   
    input  wire [16:0]                       i_mem_ecc_addr_pke1     ,   
    input  wire [16:0]                       i_mem_ecc_addr_pke2     ,   
    input  wire [16:0]                       i_mem_ecc_addr_pke3     ,   

    input  wire         i_wdt_timeout           ,

    input  wire         i_reset_trng_warning    ,
    input  wire         i_reset_trng_error      ,
    input  wire         i_hw_trng_ht_fail       ,

    input  wire         i_soc_err_axi_dma_wr    ,
    input  wire         i_soc_err_axi_dma_rd    ,
    input  wire         i_soc_err_ahb_mem       ,
    input  wire         i_soc_err_ahb_otp       ,
    input  wire         i_soc_err_ahb_nvm       ,
    input  wire         i_soc_err_ahb_cfg       ,

    input  wire         i_otp_key_crc_err       ,

    input  wire         i_ipre_pchk_err         ,
    input  wire         i_dpre_pchk_err         ,
    input  wire         i_spre_pchk_err         ,
    input  wire         i_ahbdpre_pchk_err      ,
    input  wire         i_iromprc_pchk_err      ,
    input  wire         i_iramprc_pchk_err      ,
    input  wire         i_dramprc_pchk_err      ,
    input  wire         i_ahbprc_pchk_err       ,
    input  wire         i_nvmprc_pchk_err       ,
    input  wire         i_socprc_pchk_err       ,

    input  wire [ 3:0]  i_dmac_axi_bus_rd_wr_to ,  
    input  wire [ 1:0]  i_dmac_ahb_bus_rd_wr_to ,  

    input  wire         i_hsm_soft_rbt_n        ,
    input  wire         i_hsm_soft_rst_n        ,
    input  wire         i_cpu_soft_rst_n        ,

    output wire [31:0]  o_err_sensor            ,   
    output wire [63:0]  o_err_hw                ,   
    output wire [63:0]  o_err_fw                ,   

    output wire         o_irq                   ,
    output wire         o_resetn_all            ,   
    output wire         o_resetn_noboot         ,   
    output wire         o_resetn_cpu                
);

localparam  ADDR_ERR_SENSOR     = 8'h00;    
localparam  ADDR_ERR_SOCERR     = 8'h01;    
localparam  ADDR_ERR_HW_0       = 8'h02;    
localparam  ADDR_ERR_HW_1       = 8'h03;    
localparam  ADDR_ERR_FW_0       = 8'h04;    
localparam  ADDR_ERR_FW_1       = 8'h05;    

localparam  ADDR_INT_SENSOR     = 8'h08;    
localparam  ADDR_INT_SOCERR     = 8'h09;    
localparam  ADDR_INT_HW_0       = 8'h0A;    
localparam  ADDR_INT_HW_1       = 8'h0B;    

localparam  ADDR_INTEN_SENSOR   = 8'h10;    
localparam  ADDR_INTEN_SOCERR   = 8'h11;    
localparam  ADDR_INTEN_HW_0     = 8'h12;    
localparam  ADDR_INTEN_HW_1     = 8'h13;    

localparam  ADDR_MEMECC_ADDR_0  = 8'h20;    
localparam  ADDR_MEMECC_ADDR_1  = 8'h21;    
localparam  ADDR_MEMECC_ADDR_2  = 8'h22;    
localparam  ADDR_MEMECC_ADDR_3  = 8'h23;    
localparam  ADDR_MEMECC_ADDR_4  = 8'h24;    
localparam  ADDR_MEMECC_ADDR_5  = 8'h25;    
localparam  ADDR_MEMECC_ADDR_6  = 8'h26;    
localparam  ADDR_MEMECC_ADDR_7  = 8'h27;    

localparam  ADDR_RST_ALL_SENSOR = 8'h40;    
localparam  ADDR_RST_ALL_SOC    = 8'h41;    
localparam  ADDR_RST_ALL_HW_0   = 8'h42;    
localparam  ADDR_RST_ALL_HW_1   = 8'h43;    

localparam  ADDR_RST_NBT_SENSOR = 8'h44;    
localparam  ADDR_RST_NBT_SOC    = 8'h45;    
localparam  ADDR_RST_NBT_HW_0   = 8'h46;    
localparam  ADDR_RST_NBT_HW_1   = 8'h47;    

localparam  ADDR_RST_CPU_SENSOR = 8'h48;    
localparam  ADDR_RST_CPU_SOC    = 8'h49;    
localparam  ADDR_RST_CPU_HW_0   = 8'h4A;    
localparam  ADDR_RST_CPU_HW_1   = 8'h4B;    

localparam  ADDR_RST_ENABLE     = 8'h4C;    
localparam  ADDR_RST_INFO       = 8'h4D;    

localparam  ADDR_ERR_HW_TRIG_0  = 8'h50;    
localparam  ADDR_ERR_HW_TRIG_1  = 8'h51;    

localparam  ADDR_ERR_TRIG_LOCK  = 8'h60;    

localparam     RE2P_p_NUM = 4 ;

wire            reg_req;
wire            reg_rd;
wire    [7:0]   reg_raddr;
wire            reg_wr;
wire    [31:0]  reg_wdata;

wire    [31:0]  sensor_sync;
wire    [31:0]  soc_err_sync;

wire    [31:0]  w_reset_info;
wire    [31:0]  w_reset_enable;

wire            w_resetn_all;
wire            w_resetn_nbt;
wire            w_resetn_cpu;

wire            w_i_hsm_soft_rbt_n    = i_hsm_soft_rbt_n;
wire            w_i_hsm_soft_rst_n    = i_hsm_soft_rst_n;
wire            w_i_cpu_soft_rst_n    = i_cpu_soft_rst_n;

wire            otp_key_crc_err_p       ;
wire            reset_trng_warning_p    ;
wire            reset_trng_error_p      ;
wire            hw_trng_ht_fail_p       ;

reg     [63:0]  err_hw;

reg [63:0]                      err_hw_dff               ;
reg                             reg_wr_dly               ;
reg [7:0]                       reg_waddr                ;
reg [31:0]                      reg_rdata                ;
reg [31:0]                      csr_int_sensor           ;
reg [31:0]                      csr_int_soc_err          ;
reg [31:0]                      csr_int_hw_0             ;
reg [31:0]                      csr_int_hw_1             ;
reg [31:0]                      csr_err_fw_0             ;
reg [31:0]                      csr_err_fw_1             ;
reg [31:0]                      csr_inten_sensor         ;
reg [31:0]                      csr_inten_soc_err        ;
reg [31:0]                      csr_inten_hw_0           ;
reg [31:0]                      csr_inten_hw_1           ;
reg [14-1:0] csr_mem_ecc_addr_irom    ;
reg [16-1:0] csr_mem_ecc_addr_iram    ;
reg [14-1:0] csr_mem_ecc_addr_dram    ;
reg [16:0]                      csr_mem_ecc_addr_kmu     ;
reg [16:0]                      csr_mem_ecc_addr_pke0    ;
reg [16:0]                      csr_mem_ecc_addr_pke1    ;
reg [16:0]                      csr_mem_ecc_addr_pke2    ;
reg [16:0]                      csr_mem_ecc_addr_pke3    ;
reg [31:0]                      csr_rsten_all_sensor     ;
reg [31:0]                      csr_rsten_all_soc        ;
reg [31:0]                      csr_rsten_all_hw_0       ;
reg [31:0]                      csr_rsten_all_hw_1       ;
reg [31:0]                      csr_rsten_nbt_sensor     ;
reg [31:0]                      csr_rsten_nbt_soc        ;
reg [31:0]                      csr_rsten_nbt_hw_0       ;
reg [31:0]                      csr_rsten_nbt_hw_1       ;
reg [31:0]                      csr_rsten_cpu_sensor     ;
reg [31:0]                      csr_rsten_cpu_soc        ;
reg [31:0]                      csr_rsten_cpu_hw_0       ;
reg [31:0]                      csr_rsten_cpu_hw_1       ;
reg                             resetn_all               ;
reg                             resetn_noboot            ;
reg                             resetn_cpu               ;
reg [31:0]                      err_hw_trig_0            ;
reg [31:0]                      err_hw_trig_1            ;
reg [31:0]                      err_trig_lock            ;
reg                             csr_resetn_all_enable    ;
reg                             csr_resetn_nbt_enable    ;
reg                             csr_resetn_cpu_enable    ;
reg [31:0]                      csr_reset_info           ;


wire                             csr_soc_nbt_rst_n_1      = 1'b1;

wire [RE2P_p_NUM-1:0]    re2p_out_pulse ;

wire    [63:0]  err_hw_trig     = {err_hw_trig_1,err_hw_trig_0};
wire    [63:0]  err_hw_dff_next = err_hw | err_hw_trig; 

wire            reg_wr_dly_next = i_hready ? (reg_req & i_hwrite) : reg_wr_dly;
wire    [7:0]   reg_waddr_next = i_hready ? reg_raddr : reg_waddr;
reg     [31:0]  reg_rdata_next ;

wire    [31:0]  csr_int_sensor_next = sensor_sync | (((reg_wr && (reg_waddr == ADDR_INT_SENSOR)) ? (~reg_wdata) : {32{1'b1}}) & csr_int_sensor); 
wire    [31:0]  csr_int_soc_err_next= soc_err_sync| (((reg_wr && (reg_waddr == ADDR_INT_SOCERR)) ? (~reg_wdata) : {32{1'b1}}) & csr_int_soc_err);
wire    [31:0]  csr_int_hw_0_next = err_hw_dff[31: 0] | (((reg_wr && (reg_waddr == ADDR_INT_HW_0)) ? (~reg_wdata) : {32{1'b1}}) & csr_int_hw_0); 
wire    [31:0]  csr_int_hw_1_next = err_hw_dff[63:32] | (((reg_wr && (reg_waddr == ADDR_INT_HW_1)) ? (~reg_wdata) : {32{1'b1}}) & csr_int_hw_1); 
wire    [31:0]  csr_err_fw_0_next = (reg_wr && (reg_waddr == ADDR_ERR_FW_0)) ? reg_wdata : csr_err_fw_0; 
wire    [31:0]  csr_err_fw_1_next = (reg_wr && (reg_waddr == ADDR_ERR_FW_1)) ? reg_wdata : csr_err_fw_1; 

wire    [31:0]  csr_inten_sensor_next = (reg_wr && (reg_waddr == ADDR_INTEN_SENSOR)) ? reg_wdata : csr_inten_sensor; 
wire    [31:0]  csr_inten_soc_err_next = (reg_wr && (reg_waddr == ADDR_INTEN_SOCERR)) ? reg_wdata : csr_inten_soc_err; 
wire    [31:0]  csr_inten_hw_0_next = (reg_wr && (reg_waddr == ADDR_INTEN_HW_0)) ? reg_wdata : csr_inten_hw_0; 
wire    [31:0]  csr_inten_hw_1_next = (reg_wr && (reg_waddr == ADDR_INTEN_HW_1)) ? reg_wdata : csr_inten_hw_1; 

wire    [14-1:0]  csr_mem_ecc_addr_irom_next = (i_mem_ecc_1b_irom || i_mem_ecc_mb_irom) ? i_mem_ecc_addr_irom : csr_mem_ecc_addr_irom; 
wire    [16-1:0]  csr_mem_ecc_addr_iram_next = (i_mem_ecc_1b_iram || i_mem_ecc_mb_iram) ? i_mem_ecc_addr_iram : csr_mem_ecc_addr_iram; 
wire    [14-1:0]  csr_mem_ecc_addr_dram_next = (i_mem_ecc_1b_dram || i_mem_ecc_mb_dram) ? i_mem_ecc_addr_dram : csr_mem_ecc_addr_dram; 
wire    [16:0]  csr_mem_ecc_addr_kmu_next  = (i_mem_ecc_1b_kmu  || i_mem_ecc_mb_kmu ) ? i_mem_ecc_addr_kmu  : csr_mem_ecc_addr_kmu ; 
wire    [16:0]  csr_mem_ecc_addr_pke0_next = (i_mem_ecc_1b_pke0 || i_mem_ecc_mb_pke0) ? i_mem_ecc_addr_pke0 : csr_mem_ecc_addr_pke0; 
wire    [16:0]  csr_mem_ecc_addr_pke1_next = (i_mem_ecc_1b_pke1 || i_mem_ecc_mb_pke1) ? i_mem_ecc_addr_pke1 : csr_mem_ecc_addr_pke1; 
wire    [16:0]  csr_mem_ecc_addr_pke2_next = (i_mem_ecc_1b_pke2 || i_mem_ecc_mb_pke2) ? i_mem_ecc_addr_pke2 : csr_mem_ecc_addr_pke2; 
wire    [16:0]  csr_mem_ecc_addr_pke3_next = (i_mem_ecc_1b_pke3 || i_mem_ecc_mb_pke3) ? i_mem_ecc_addr_pke3 : csr_mem_ecc_addr_pke3; 

wire    [31:0]  csr_rsten_all_sensor_next  = (reg_wr && (reg_waddr == ADDR_RST_ALL_SENSOR)) ? reg_wdata : csr_rsten_all_sensor; 
wire    [31:0]  csr_rsten_all_soc_next     = (reg_wr && (reg_waddr == ADDR_RST_ALL_SOC   )) ? reg_wdata : csr_rsten_all_soc   ; 
wire    [31:0]  csr_rsten_all_hw_0_next    = (reg_wr && (reg_waddr == ADDR_RST_ALL_HW_0  )) ? reg_wdata : csr_rsten_all_hw_0  ; 
wire    [31:0]  csr_rsten_all_hw_1_next    = (reg_wr && (reg_waddr == ADDR_RST_ALL_HW_1  )) ? reg_wdata : csr_rsten_all_hw_1  ; 
wire    [31:0]  csr_rsten_nbt_sensor_next  = (reg_wr && (reg_waddr == ADDR_RST_NBT_SENSOR)) ? reg_wdata : csr_rsten_nbt_sensor; 
wire    [31:0]  csr_rsten_nbt_soc_next     = (reg_wr && (reg_waddr == ADDR_RST_NBT_SOC   )) ? reg_wdata : csr_rsten_nbt_soc   ; 
wire    [31:0]  csr_rsten_nbt_hw_0_next    = (reg_wr && (reg_waddr == ADDR_RST_NBT_HW_0  )) ? reg_wdata : csr_rsten_nbt_hw_0  ; 
wire    [31:0]  csr_rsten_nbt_hw_1_next    = (reg_wr && (reg_waddr == ADDR_RST_NBT_HW_1  )) ? reg_wdata : csr_rsten_nbt_hw_1  ; 
wire    [31:0]  csr_rsten_cpu_sensor_next  = (reg_wr && (reg_waddr == ADDR_RST_CPU_SENSOR)) ? reg_wdata : csr_rsten_cpu_sensor; 
wire    [31:0]  csr_rsten_cpu_soc_next     = (reg_wr && (reg_waddr == ADDR_RST_CPU_SOC   )) ? reg_wdata : csr_rsten_cpu_soc   ; 
wire    [31:0]  csr_rsten_cpu_hw_0_next    = (reg_wr && (reg_waddr == ADDR_RST_CPU_HW_0  )) ? reg_wdata : csr_rsten_cpu_hw_0  ; 
wire    [31:0]  csr_rsten_cpu_hw_1_next    = (reg_wr && (reg_waddr == ADDR_RST_CPU_HW_1  )) ? reg_wdata : csr_rsten_cpu_hw_1  ; 

wire            resetn_all_next    = w_resetn_all; 
wire            resetn_noboot_next = w_resetn_nbt; 
wire            resetn_cpu_next    = w_resetn_cpu; 

wire            trig_lock = err_trig_lock != 32'h965f_0cba;

wire            wr_err_hw_trig_0      = reg_wr && (reg_waddr == ADDR_ERR_HW_TRIG_0    );
wire            wr_err_hw_trig_1      = reg_wr && (reg_waddr == ADDR_ERR_HW_TRIG_1    );

wire    [31:0]  err_hw_trig_0_next     = trig_lock            ? 32'h0            :
                                         wr_err_hw_trig_0     ? {2'd0,
                                                                reg_wdata[29:24],
                                                                4'd0,
                                                                reg_wdata[19:12],
                                                                4'd0,
                                                                reg_wdata[7:0]} : 32'h0      ;
wire    [31:0]  err_hw_trig_1_next     = trig_lock            ? 32'h0            :
                                         wr_err_hw_trig_1     ? {23'd0,
                                                                 reg_wdata[8],
                                                                 4'd0,
                                                                 reg_wdata[3:0]} : 32'h0     ;

wire    [31:0] err_trig_lock_next = (reg_wr && reg_waddr == ADDR_ERR_TRIG_LOCK) ? reg_wdata : err_trig_lock;

wire            csr_resetn_all_enable_next = ~resetn_all    ? 1'b0 : (reg_wr && (reg_waddr == ADDR_RST_ENABLE)) ? reg_wdata[0]  : csr_resetn_all_enable; 
wire            csr_resetn_nbt_enable_next = ~resetn_noboot ? 1'b0 : (reg_wr && (reg_waddr == ADDR_RST_ENABLE)) ? reg_wdata[8]  : csr_resetn_nbt_enable; 
wire            csr_resetn_cpu_enable_next = ~resetn_cpu    ? 1'b0 : (reg_wr && (reg_waddr == ADDR_RST_ENABLE)) ? reg_wdata[16] : csr_resetn_cpu_enable; 

wire    [31:0]  csr_reset_info_next = w_reset_info | (((reg_wr && (reg_waddr == ADDR_RST_INFO)) ? (~reg_wdata) : {32{1'b1}}) & csr_reset_info);

assign reg_req   = i_hsel & i_htrans[1] & (i_hsize==3'h2) & i_hready;
assign reg_rd    = reg_req & (~i_hwrite);
assign reg_raddr = i_haddr[9:2];
assign reg_wr    = i_hready & reg_wr_dly;
assign reg_wdata = i_hwdata;

assign sensor_sync = i_sensor;
assign soc_err_sync = i_soc_err ;

assign w_reset_enable = {15'b0,csr_resetn_cpu_enable,7'b0,csr_resetn_nbt_enable,7'b0,csr_resetn_all_enable};
assign w_reset_info = { 8'b0,
                        6'b0,                     ~w_i_cpu_soft_rst_n,~resetn_cpu   ,  
                        5'b0,~csr_soc_nbt_rst_n_1,~w_i_hsm_soft_rst_n,~resetn_noboot,  
                        6'b0,                     ~w_i_hsm_soft_rbt_n,~resetn_all   }; 

assign w_resetn_all = ~(|({sensor_sync,soc_err_sync,err_hw_dff} & {csr_rsten_all_sensor,csr_rsten_all_soc,{csr_rsten_all_hw_1,csr_rsten_all_hw_0}}) & csr_resetn_all_enable); 
assign w_resetn_nbt = ~(|({sensor_sync,soc_err_sync,err_hw_dff} & {csr_rsten_nbt_sensor,csr_rsten_nbt_soc,{csr_rsten_nbt_hw_1,csr_rsten_nbt_hw_0}}) & csr_resetn_nbt_enable); 
assign w_resetn_cpu = ~(|({sensor_sync,soc_err_sync,err_hw_dff} & {csr_rsten_cpu_sensor,csr_rsten_cpu_soc,{csr_rsten_cpu_hw_1,csr_rsten_cpu_hw_0}}) & csr_resetn_cpu_enable); 

assign otp_key_crc_err_p    = re2p_out_pulse[3] ;
assign reset_trng_warning_p = re2p_out_pulse[2] ;
assign reset_trng_error_p   = re2p_out_pulse[1] ;
assign hw_trng_ht_fail_p    = re2p_out_pulse[0] ;

wire [63:0] w_err_hw = { csr_int_hw_1       ,
                         csr_int_hw_0[31:8] ,
                         err_hw_dff[7:0]    };

wire                     re2p_clk       = i_hclk    ;
wire                     re2p_rst_n     = i_hresetn ;
wire [RE2P_p_NUM-1:0]    re2p_in_edge   = {i_otp_key_crc_err,i_reset_trng_warning,i_reset_trng_error,i_hw_trng_ht_fail};

assign o_hrdata                = reg_rdata;
assign o_hresp                 = 2'b0;
assign o_hreadyout             = 1'b1;

assign o_err_sensor            = i_sensor;
assign o_err_hw                = w_err_hw;
assign o_err_fw                = {csr_err_fw_1,csr_err_fw_0};
assign o_irq                   = |({csr_int_sensor  ,csr_int_soc_err  ,csr_int_hw_1  ,csr_int_hw_0  }
                                & {csr_inten_sensor ,csr_inten_soc_err,csr_inten_hw_1,csr_inten_hw_0});
assign o_resetn_all            = resetn_all   ;
assign o_resetn_noboot         = resetn_noboot;
assign o_resetn_cpu            = resetn_cpu   ;

osr_redge_to_pulse #(
    .p_NUM     ( RE2P_p_NUM )  
    ) u_redge_to_pulse (
    .clk           ( re2p_clk       ), 
    .rst_n         ( re2p_rst_n     ), 
    .in_edge       ( re2p_in_edge   ), 
    .out_pulse     ( re2p_out_pulse )  
    ); 

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        err_hw_dff               <= 64'h0                         ; 
        reg_wr_dly               <= 1'b0                          ; 
        reg_waddr                <= 8'h0                          ; 
        reg_rdata                <= 32'h0                         ; 
        csr_int_sensor           <= 32'h0                         ; 
        csr_int_soc_err          <= 32'h0                         ; 
        csr_int_hw_0             <= 32'h0                         ; 
        csr_int_hw_1             <= 32'b0                         ; 
        csr_err_fw_0             <= 32'h0                         ; 
        csr_err_fw_1             <= 32'h0                         ; 
        csr_inten_sensor         <= 32'h0                         ; 
        csr_inten_soc_err        <= 32'h0                         ; 
        csr_inten_hw_0           <= 32'h0                         ; 
        csr_inten_hw_1           <= 32'h0                         ; 
        csr_mem_ecc_addr_irom    <= {14{1'b0}} ; 
        csr_mem_ecc_addr_iram    <= {16{1'b0}} ; 
        csr_mem_ecc_addr_dram    <= {14{1'b0}} ; 
        csr_mem_ecc_addr_kmu     <= 17'h0                         ; 
        csr_mem_ecc_addr_pke0    <= 17'h0                         ; 
        csr_mem_ecc_addr_pke1    <= 17'h0                         ; 
        csr_mem_ecc_addr_pke2    <= 17'h0                         ; 
        csr_mem_ecc_addr_pke3    <= 17'h0                         ; 
        csr_rsten_all_sensor     <= 32'h0                         ; 
        csr_rsten_all_soc        <= 32'h0                         ; 
        csr_rsten_all_hw_0       <= 32'h0                         ; 
        csr_rsten_all_hw_1       <= 32'h0                         ; 
        csr_rsten_nbt_sensor     <= 32'h0                         ; 
        csr_rsten_nbt_soc        <= 32'h0                         ; 
        csr_rsten_nbt_hw_0       <= 32'h0                         ; 
        csr_rsten_nbt_hw_1       <= 32'h0                         ; 
        csr_rsten_cpu_sensor     <= 32'h0                         ; 
        csr_rsten_cpu_soc        <= 32'h0                         ; 
        csr_rsten_cpu_hw_0       <= 32'h0                         ; 
        csr_rsten_cpu_hw_1       <= 32'h0                         ; 
        resetn_all               <= 1'b1                          ; 
        resetn_noboot            <= 1'b1                          ; 
        resetn_cpu               <= 1'b1                          ; 
    end else begin
        err_hw_dff               <= err_hw_dff_next               ; 
        reg_wr_dly               <= reg_wr_dly_next               ; 
        reg_waddr                <= reg_waddr_next                ; 
        reg_rdata                <= reg_rdata_next                ; 
        csr_int_sensor           <= csr_int_sensor_next           ; 
        csr_int_soc_err          <= csr_int_soc_err_next          ; 
        csr_int_hw_0             <= csr_int_hw_0_next             ; 
        csr_int_hw_1             <= csr_int_hw_1_next             ; 
        csr_err_fw_0             <= csr_err_fw_0_next             ; 
        csr_err_fw_1             <= csr_err_fw_1_next             ; 
        csr_inten_sensor         <= csr_inten_sensor_next         ; 
        csr_inten_soc_err        <= csr_inten_soc_err_next        ; 
        csr_inten_hw_0           <= csr_inten_hw_0_next           ; 
        csr_inten_hw_1           <= csr_inten_hw_1_next           ; 
        csr_mem_ecc_addr_irom    <= csr_mem_ecc_addr_irom_next    ; 
        csr_mem_ecc_addr_iram    <= csr_mem_ecc_addr_iram_next    ; 
        csr_mem_ecc_addr_dram    <= csr_mem_ecc_addr_dram_next    ; 
        csr_mem_ecc_addr_kmu     <= csr_mem_ecc_addr_kmu_next     ; 
        csr_mem_ecc_addr_pke0    <= csr_mem_ecc_addr_pke0_next    ; 
        csr_mem_ecc_addr_pke1    <= csr_mem_ecc_addr_pke1_next    ; 
        csr_mem_ecc_addr_pke2    <= csr_mem_ecc_addr_pke2_next    ; 
        csr_mem_ecc_addr_pke3    <= csr_mem_ecc_addr_pke3_next    ; 
        csr_rsten_all_sensor     <= csr_rsten_all_sensor_next     ; 
        csr_rsten_all_soc        <= csr_rsten_all_soc_next        ; 
        csr_rsten_all_hw_0       <= csr_rsten_all_hw_0_next       ; 
        csr_rsten_all_hw_1       <= csr_rsten_all_hw_1_next       ; 
        csr_rsten_nbt_sensor     <= csr_rsten_nbt_sensor_next     ; 
        csr_rsten_nbt_soc        <= csr_rsten_nbt_soc_next        ; 
        csr_rsten_nbt_hw_0       <= csr_rsten_nbt_hw_0_next       ; 
        csr_rsten_nbt_hw_1       <= csr_rsten_nbt_hw_1_next       ; 
        csr_rsten_cpu_sensor     <= csr_rsten_cpu_sensor_next     ; 
        csr_rsten_cpu_soc        <= csr_rsten_cpu_soc_next        ; 
        csr_rsten_cpu_hw_0       <= csr_rsten_cpu_hw_0_next       ; 
        csr_rsten_cpu_hw_1       <= csr_rsten_cpu_hw_1_next       ; 
        resetn_all               <= resetn_all_next               ; 
        resetn_noboot            <= resetn_noboot_next            ; 
        resetn_cpu               <= resetn_cpu_next               ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        err_hw_trig_0            <= 32'h0                         ; 
        err_hw_trig_1            <= 32'h0                         ; 
    end else begin
        err_hw_trig_0            <= err_hw_trig_0_next            ; 
        err_hw_trig_1            <= err_hw_trig_1_next            ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        err_trig_lock            <= 32'h0                         ; 
    end else begin
        err_trig_lock            <= err_trig_lock_next            ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        csr_resetn_all_enable    <= 1'b0                          ; 
        csr_resetn_nbt_enable    <= 1'b0                          ; 
        csr_resetn_cpu_enable    <= 1'b0                          ; 
        csr_reset_info           <= 32'b0                         ; 
    end else begin
        csr_resetn_all_enable    <= csr_resetn_all_enable_next    ; 
        csr_resetn_nbt_enable    <= csr_resetn_nbt_enable_next    ; 
        csr_resetn_cpu_enable    <= csr_resetn_cpu_enable_next    ; 
        csr_reset_info           <= csr_reset_info_next           ; 
    end
end

always @ (*)
begin

    err_hw[ 0] = i_mem_ecc_1b_irom;
    err_hw[ 1] = i_mem_ecc_1b_iram;
    err_hw[ 2] = i_mem_ecc_1b_dram;
    err_hw[ 3] = i_mem_ecc_1b_kmu ;
    err_hw[ 4] = i_mem_ecc_1b_pke0;
    err_hw[ 5] = i_mem_ecc_1b_pke1;
    err_hw[ 6] = i_mem_ecc_1b_pke2;
    err_hw[ 7] = i_mem_ecc_1b_pke3;
    err_hw[11:8]  = 4'd0;

    err_hw[12] = i_mem_ecc_mb_irom  ; 
    err_hw[13] = i_mem_ecc_mb_iram  ;
    err_hw[14] = i_mem_ecc_mb_dram  ;
    err_hw[15] = i_mem_ecc_mb_kmu   ;
    err_hw[16] = i_mem_ecc_mb_pke0  ;
    err_hw[17] = i_mem_ecc_mb_pke1  ;
    err_hw[18] = i_mem_ecc_mb_pke2  ;
    err_hw[19] = i_mem_ecc_mb_pke3  ;
    err_hw[23:20] = 4'd0;

    err_hw[24] = i_soc_err_axi_dma_wr;
    err_hw[25] = i_soc_err_axi_dma_rd;
    err_hw[26] = i_soc_err_ahb_mem;
    err_hw[27] = i_soc_err_ahb_otp;
    err_hw[28] = i_soc_err_ahb_nvm;
    err_hw[29] = i_soc_err_ahb_cfg;
    err_hw[31:30] = 2'd0;

    err_hw[32] = hw_trng_ht_fail_p;
    err_hw[33] = reset_trng_error_p;
    err_hw[34] = reset_trng_warning_p;
    err_hw[35] = otp_key_crc_err_p;
    err_hw[39:36] = 4'd0;

    err_hw[40] = i_wdt_timeout;
    err_hw[41] = 1'b0;
    err_hw[43:42] = 2'd0;

    err_hw[44] = 1'h0;
    err_hw[45] = 1'b0;

    err_hw[46] = i_ipre_pchk_err    ;
    err_hw[47] = i_dpre_pchk_err    ;
    err_hw[48] = i_spre_pchk_err    ;
    err_hw[49] = i_ahbdpre_pchk_err ;
    err_hw[50] = i_iromprc_pchk_err ;
    err_hw[51] = i_iramprc_pchk_err ;
    err_hw[52] = i_dramprc_pchk_err ;
    err_hw[53] = i_ahbprc_pchk_err  ;
    err_hw[54] = i_nvmprc_pchk_err  ;
    err_hw[55] = i_socprc_pchk_err  ;
    err_hw[56] = 1'b0;
    err_hw[60:57] = i_dmac_axi_bus_rd_wr_to    ;
    err_hw[62:61] = i_dmac_ahb_bus_rd_wr_to    ;
    err_hw[63] = 1'b0; 
end

always @ (*)
begin
    reg_rdata_next = reg_rdata;
    if (i_hready)
    begin
        reg_rdata_next = 32'h0;

        if (reg_rd && i_hready)
        begin
            case (reg_raddr)
                ADDR_ERR_SENSOR     : reg_rdata_next = sensor_sync;         
                ADDR_ERR_SOCERR     : reg_rdata_next = soc_err_sync;        
                ADDR_ERR_HW_0       : reg_rdata_next = csr_int_hw_0 ;       
                ADDR_ERR_HW_1       : reg_rdata_next = csr_int_hw_1 ;       
                ADDR_ERR_FW_0       : reg_rdata_next = csr_err_fw_0 ;       
                ADDR_ERR_FW_1       : reg_rdata_next = csr_err_fw_1 ;       

                ADDR_INT_SENSOR     : reg_rdata_next = csr_int_sensor;      
                ADDR_INT_SOCERR     : reg_rdata_next = csr_int_soc_err;     
                ADDR_INT_HW_0       : reg_rdata_next = csr_int_hw_0;        
                ADDR_INT_HW_1       : reg_rdata_next = csr_int_hw_1;        

                ADDR_INTEN_SENSOR   : reg_rdata_next = csr_inten_sensor;    
                ADDR_INTEN_SOCERR   : reg_rdata_next = csr_inten_soc_err;   
                ADDR_INTEN_HW_0     : reg_rdata_next = csr_inten_hw_0;      
                ADDR_INTEN_HW_1     : reg_rdata_next = csr_inten_hw_1;      

                ADDR_MEMECC_ADDR_0  : reg_rdata_next[14-1:0] = csr_mem_ecc_addr_irom;    
                ADDR_MEMECC_ADDR_1  : reg_rdata_next[16-1:0] = csr_mem_ecc_addr_iram;    
                ADDR_MEMECC_ADDR_2  : reg_rdata_next[14-1:0] = csr_mem_ecc_addr_dram;    
                ADDR_MEMECC_ADDR_3  : reg_rdata_next[16:0] = csr_mem_ecc_addr_kmu ;    
                ADDR_MEMECC_ADDR_4  : reg_rdata_next[16:0] = csr_mem_ecc_addr_pke0;    
                ADDR_MEMECC_ADDR_5  : reg_rdata_next[16:0] = csr_mem_ecc_addr_pke1;    
                ADDR_MEMECC_ADDR_6  : reg_rdata_next[16:0] = csr_mem_ecc_addr_pke2;    
                ADDR_MEMECC_ADDR_7  : reg_rdata_next[16:0] = csr_mem_ecc_addr_pke3;    

                ADDR_RST_ALL_SENSOR : reg_rdata_next = csr_rsten_all_sensor;
                ADDR_RST_ALL_SOC    : reg_rdata_next = csr_rsten_all_soc   ;
                ADDR_RST_ALL_HW_0   : reg_rdata_next = csr_rsten_all_hw_0  ;
                ADDR_RST_ALL_HW_1   : reg_rdata_next = csr_rsten_all_hw_1  ;
                ADDR_RST_NBT_SENSOR : reg_rdata_next = csr_rsten_nbt_sensor;
                ADDR_RST_NBT_SOC    : reg_rdata_next = csr_rsten_nbt_soc   ;
                ADDR_RST_NBT_HW_0   : reg_rdata_next = csr_rsten_nbt_hw_0  ;
                ADDR_RST_NBT_HW_1   : reg_rdata_next = csr_rsten_nbt_hw_1  ;
                ADDR_RST_CPU_SENSOR : reg_rdata_next = csr_rsten_cpu_sensor;
                ADDR_RST_CPU_SOC    : reg_rdata_next = csr_rsten_cpu_soc   ;
                ADDR_RST_CPU_HW_0   : reg_rdata_next = csr_rsten_cpu_hw_0  ;
                ADDR_RST_CPU_HW_1   : reg_rdata_next = csr_rsten_cpu_hw_1  ;

                ADDR_RST_ENABLE     : reg_rdata_next = w_reset_enable;      
                ADDR_RST_INFO       : reg_rdata_next = csr_reset_info;      

                ADDR_ERR_TRIG_LOCK      : reg_rdata_next = err_trig_lock     ;

                default             : reg_rdata_next = 32'h0;
            endcase
        end
    end
end

endmodule 
