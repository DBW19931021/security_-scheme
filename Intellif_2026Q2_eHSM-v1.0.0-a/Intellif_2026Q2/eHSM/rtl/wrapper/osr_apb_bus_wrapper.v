//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_apb_bus_wrapper(
    input  wire                 i_rdc_clk           ,
    input  wire                 i_wdt_clk           ,
    input  wire                 i_uart_clk          ,
    input  wire                 i_tim_clk           ,
    input  wire                 i_crc_clk           ,

    input  wire                 i_rdc_rst_n         ,
    input  wire                 i_wdt_rst_n         ,
    input  wire                 i_uart_rst_n        ,
    input  wire                 i_tim_rst_n         ,
    input  wire                 i_crc_rst_n         ,

    input  wire                 i_hclk              ,
    input  wire                 i_hresetn           ,
    input  wire                 i_hsel              ,
    input  wire [31:0]          i_haddr             ,
    input  wire [ 1:0]          i_htrans            ,
    input  wire                 i_hwrite            ,
    input  wire [ 2:0]          i_hburst            ,
    input  wire [ 2:0]          i_hsize             ,
    input  wire [ 3:0]          i_hprot             ,
    input  wire                 i_hmastlock         ,
    input  wire                 i_hready            ,
    input  wire [31:0]          i_hwdata            ,
    output wire [31:0]          o_hrdata            ,
    output wire [ 1:0]          o_hresp             ,
    output wire                 o_hreadyout         , 

    output wire                 o_psel              ,   
    output wire [31:0]          o_paddr             ,
    output wire                 o_penable           ,
    output wire                 o_pwrite            ,
    output wire [31:0]          o_pwdata            ,
    input  wire [31:0]          i_prdata            ,
    input  wire                 i_pslverr           ,
    input  wire                 i_pready            ,

    input  wire                 i_otp_random_en     ,
    output wire                 o_clock_en          , 

    output wire                 o_rclk_intr         , 
    output wire                 o_tim0_intr         ,
    output wire                 o_tim1_intr         ,
    output wire                 o_wdt_intr          ,
    output wire                 o_wdt1_intr         ,
    output wire                 o_utc_irq           ,   
    output wire                 o_mcnt_irq          ,   
    output wire                 o_wdt_rstn          ,
    input  wire                 i_uart_rxd          ,
    output wire                 o_uart_txd_oe       ,
    output wire                 o_uart_txd          ,
    output wire                 o_uart_intr         
);

localparam     RDC_p_APB_DATA_WIDTH = 32    ;
localparam     RDC_p_APB_ADDR_WIDTH = 20    ;
localparam     RDC_p_CDC_EN         = 0     ;
localparam     RDC_p_CLKEN_TYPE     = 1'b1  ;
localparam     RDC_p_OTP_DEFAULT    = 2'b01 ;

localparam     WDTE_P_LOW_PULSE = 1 ;

localparam     WDT1E_P_LOW_PULSE =1 ; 

localparam     TIM0_P_LOW_PULSE = 0; 

localparam     TIM1_P_LOW_PULSE = 0;

localparam     UTIM_RTC_CLK_FREQ_N = `OSR_UTC_TIM_SEC_DIV ;
localparam     UTIM_RTC_CLK_CNT_N  = 28            ;           
localparam     UTIM_APB_RSP_ERR_EN = 1'b0          ;         

localparam     MCNT_MONO_CNT_NUM   = 16    ; 
localparam     MCNT_APB_RSP_ERR_EN = 1'b0         ; 

localparam       NVMCIPHER_P_ENC_AW        = 19;
localparam       NVMCIPHER_P_ADDR_BW       = 5;

wire [31:0] w_prdatas10 = 32'h0     ;

wire [31:0]    h2p_o_hrdata     ;
wire           h2p_o_hreadyout  ;
wire [1:0]     h2p_o_hresp      ;
wire [31:0]    h2p_o_pwdata     ;
wire           h2p_o_penable    ;
wire           h2p_o_psels0     ;
wire           h2p_o_psels1     ;
wire           h2p_o_psels2     ;
wire           h2p_o_psels3     ;
wire           h2p_o_psels4     ;
wire           h2p_o_psels5     ;
wire           h2p_o_psels6     ;
wire           h2p_o_psels7     ;
wire           h2p_o_psels8     ;
wire           h2p_o_psels9     ;
wire           h2p_o_psels10    ;
wire           h2p_o_psels11    ;
wire           h2p_o_psels12    ;
wire           h2p_o_psels13    ;
wire           h2p_o_psels14    ;
wire           h2p_o_psels15    ;
wire [26:0]    h2p_o_paddr      ;
wire           h2p_o_pwrite     ;

wire [RDC_p_APB_DATA_WIDTH-1:0]      rdc_o_s_prdata      = 32'b0; 
wire                                 rdc_o_s_pready      = 1'b1 ;
wire                                 rdc_o_irq           = 1'b0 ;
wire                                 rdc_o_clock_en      = 1'b1 ;

wire [31:0]    wdt_o_prdata    ;
wire           wdt_o_wdtintr   ;
wire           wdt_o_nresetout ;

wire           wdt1_o_wdtintr   = 1'h0  ;
wire           wdt1_o_nresetout = 1'h1  ;

wire        wdte_o_2pulse = wdt_o_nresetout; 


wire [31:0]    uart_prdata   ;
wire           uart_pready   ;
wire           uart_pslverr  ;
wire           uart_txd      ;
wire           uart_txen     ;
wire           uart_baudtick ;
wire           uart_txint    ;
wire           uart_rxint    ;
wire           uart_txovrint ;
wire           uart_rxovrint ;
wire           uart_uartint  ;

wire [31:0]    crc_o_prdata  ;

wire [31:0]    tim_o_prdata   ;
wire           tim_o_int_tim0 ;
wire           tim_o_int_tim1 ;

wire        tim0_o_2pulse = tim_o_int_tim0; 

wire        tim1_o_2pulse = tim_o_int_tim1;

wire [31:0]    utim_o_prdata   = 32'h0      ;
wire           utim_o_pready   = 1'h1       ;
wire           utim_o_irq      = 1'h0       ;

wire [31:0]    mcnt_o_prdata   = 32'h0      ;
wire           mcnt_o_pready   = 1'h1       ;
wire           mcnt_o_irq      = 1'h0       ;

wire [31:0]    nvmcipher_o_prdata        = 32'h0; 
wire           nvmcipher_o_pready        = 1'b1;

wire           h2p_hclk         = i_hclk         ;
wire           h2p_hresetn      = i_hresetn      ;
wire [31:0]    h2p_i_haddr      = i_haddr        ;
wire [1:0]     h2p_i_htrans     = i_htrans       ;
wire           h2p_i_hwrite     = i_hwrite       ;
wire [31:0]    h2p_i_hwdata     = i_hwdata       ;
wire           h2p_i_hsel       = i_hsel         ;
wire           h2p_i_hready     = i_hready       ;
wire [31:0]    h2p_i_prdatas0   = rdc_o_s_prdata ;
wire [31:0]    h2p_i_prdatas1   = wdt_o_prdata   ;
wire [31:0]    h2p_i_prdatas2   = 32'h0          ;
wire [31:0]    h2p_i_prdatas3   = 32'h0          ;
wire [31:0]    h2p_i_prdatas4   = uart_prdata    ;
wire [31:0]    h2p_i_prdatas5   = 32'h0          ;
wire [31:0]    h2p_i_prdatas6   = crc_o_prdata   ;
wire [31:0]    h2p_i_prdatas7   = nvmcipher_o_prdata;
wire [31:0]    h2p_i_prdatas8   = tim_o_prdata   ;
wire [31:0]    h2p_i_prdatas9   = 32'h0          ;
wire [31:0]    h2p_i_prdatas10  = w_prdatas10    ;
wire [31:0]    h2p_i_prdatas11  = 32'h0          ;
wire [31:0]    h2p_i_prdatas12  = utim_o_prdata  ;
wire [31:0]    h2p_i_prdatas13  = mcnt_o_prdata  ;
wire [31:0]    h2p_i_prdatas14  = 32'h0          ;
wire [31:0]    h2p_i_prdatas15  = 32'h0          ;
wire           h2p_i_pready0    = rdc_o_s_pready ; 
wire           h2p_i_pready1    = 1'b1           ;
wire           h2p_i_pready2    = 1'b1           ;
wire           h2p_i_pready3    = 1'b1           ;
wire           h2p_i_pready4    = uart_pready    ;
wire           h2p_i_pready5    = 1'b1           ;
wire           h2p_i_pready6    = 1'b1           ;
wire           h2p_i_pready7    = nvmcipher_o_pready;
wire           h2p_i_pready8    = 1'b1           ;
wire           h2p_i_pready9    = 1'b1           ;
wire           h2p_i_pready10   = 1'b1           ;
wire           h2p_i_pready11   = 1'b1           ;
wire           h2p_i_pready12   = utim_o_pready  ;
wire           h2p_i_pready13   = mcnt_o_pready  ;
wire           h2p_i_pready14   = i_pready       ;
wire           h2p_i_pready15   = 1'h1           ;
wire           h2p_i_pclk_phase = 1'h1           ;

wire           wdt_pclk        = i_wdt_clk        ;
wire           wdt_presetn     = i_wdt_rst_n      ;
wire           wdt_i_psel      = h2p_o_psels1     ;
wire           wdt_i_pwrite    = h2p_o_pwrite     ;
wire           wdt_i_penable   = h2p_o_penable    ;
wire [9:0]     wdt_i_paddr     = h2p_o_paddr[9:0] ;
wire [31:0]    wdt_i_pwdata    = h2p_o_pwdata     ;

wire           uart_pclk     = i_uart_clk         ;  
wire           uart_presetn  = i_uart_rst_n       ;
wire           uart_psel     = h2p_o_psels4       ;
wire [11:2]    uart_paddr    = h2p_o_paddr[11:2]  ;
wire           uart_penable  = h2p_o_penable      ;
wire           uart_pwrite   = h2p_o_pwrite       ;
wire [31:0]    uart_pwdata   = h2p_o_pwdata       ;
wire           uart_rxd      = i_uart_rxd         ;

wire           crc_pclk      = i_crc_clk          ;
wire           crc_presetn   = i_crc_rst_n        ;
wire           crc_i_psel    = h2p_o_psels6       ;
wire           crc_i_penable = h2p_o_penable      ;
wire           crc_i_pwrite  = h2p_o_pwrite       ;
wire [17:0]    crc_i_paddr   = h2p_o_paddr[17:0]  ; 
wire [31:0]    crc_i_pwdata  = h2p_o_pwdata       ;

wire           tim_pclk       = i_tim_clk         ; 
wire           tim_presetn    = i_tim_rst_n       ; 
wire           tim_i_psel     = h2p_o_psels8      ; 
wire           tim_i_penable  = h2p_o_penable     ; 
wire           tim_i_pwrite   = h2p_o_pwrite      ; 
wire [7:0]     tim_i_paddr    = h2p_o_paddr[7:0]  ; 
wire [31:0]    tim_i_pwdata   = h2p_o_pwdata      ; 

assign o_hrdata                      = h2p_o_hrdata               ;
assign o_hresp                       = h2p_o_hresp                ;
assign o_hreadyout                   = h2p_o_hreadyout            ;

assign o_psel                        = h2p_o_psels14              ; 
assign o_paddr                       = {5'h0,h2p_o_paddr[26:0]}   ;
assign o_penable                     = h2p_o_penable              ;
assign o_pwrite                      = h2p_o_pwrite               ;
assign o_pwdata                      = h2p_o_pwdata               ;

assign o_clock_en                    = rdc_o_clock_en             ;

assign o_rclk_intr                   = rdc_o_irq                  ;
assign o_tim0_intr                   = tim0_o_2pulse              ;
assign o_tim1_intr                   = tim1_o_2pulse              ;
assign o_wdt_intr                    = wdt_o_wdtintr              ;
assign o_wdt1_intr                   = wdt1_o_wdtintr             ; 
assign o_utc_irq                     = utim_o_irq                 ;
assign o_mcnt_irq                    = mcnt_o_irq                 ; 
assign o_wdt_rstn                    = wdte_o_2pulse              ;

assign o_uart_txd_oe                 = uart_txen                  ;
assign o_uart_txd                    = uart_txd                   ;
assign o_uart_intr                   = uart_uartint               ;

osr_ahb2apb u_ahb2apb (
    .hclk            ( h2p_hclk         ), 
    .hresetn         ( h2p_hresetn      ), 
    .i_haddr         ( h2p_i_haddr      ), 
    .i_htrans        ( h2p_i_htrans     ), 
    .i_hwrite        ( h2p_i_hwrite     ), 
    .i_hwdata        ( h2p_i_hwdata     ), 
    .i_hsel          ( h2p_i_hsel       ), 
    .i_hready        ( h2p_i_hready     ), 
    .i_prdatas0      ( h2p_i_prdatas0   ), 
    .i_prdatas1      ( h2p_i_prdatas1   ), 
    .i_prdatas2      ( h2p_i_prdatas2   ), 
    .i_prdatas3      ( h2p_i_prdatas3   ), 
    .i_prdatas4      ( h2p_i_prdatas4   ), 
    .i_prdatas5      ( h2p_i_prdatas5   ), 
    .i_prdatas6      ( h2p_i_prdatas6   ), 
    .i_prdatas7      ( h2p_i_prdatas7   ), 
    .i_prdatas8      ( h2p_i_prdatas8   ), 
    .i_prdatas9      ( h2p_i_prdatas9   ), 
    .i_prdatas10     ( h2p_i_prdatas10  ), 
    .i_prdatas11     ( h2p_i_prdatas11  ), 
    .i_prdatas12     ( h2p_i_prdatas12  ), 
    .i_prdatas13     ( h2p_i_prdatas13  ), 
    .i_prdatas14     ( h2p_i_prdatas14  ), 
    .i_prdatas15     ( h2p_i_prdatas15  ), 
    .i_pready0       ( h2p_i_pready0    ), 
    .i_pready1       ( h2p_i_pready1    ), 
    .i_pready2       ( h2p_i_pready2    ), 
    .i_pready3       ( h2p_i_pready3    ), 
    .i_pready4       ( h2p_i_pready4    ), 
    .i_pready5       ( h2p_i_pready5    ), 
    .i_pready6       ( h2p_i_pready6    ), 
    .i_pready7       ( h2p_i_pready7    ), 
    .i_pready8       ( h2p_i_pready8    ), 
    .i_pready9       ( h2p_i_pready9    ), 
    .i_pready10      ( h2p_i_pready10   ), 
    .i_pready11      ( h2p_i_pready11   ), 
    .i_pready12      ( h2p_i_pready12   ), 
    .i_pready13      ( h2p_i_pready13   ), 
    .i_pready14      ( h2p_i_pready14   ), 
    .i_pready15      ( h2p_i_pready15   ), 
    .i_pclk_phase    ( h2p_i_pclk_phase ), 
    .o_hrdata        ( h2p_o_hrdata     ), 
    .o_hreadyout     ( h2p_o_hreadyout  ), 
    .o_hresp         ( h2p_o_hresp      ), 
    .o_pwdata        ( h2p_o_pwdata     ), 
    .o_penable       ( h2p_o_penable    ), 
    .o_psels0        ( h2p_o_psels0     ), 
    .o_psels1        ( h2p_o_psels1     ), 
    .o_psels2        ( h2p_o_psels2     ), 
    .o_psels3        ( h2p_o_psels3     ), 
    .o_psels4        ( h2p_o_psels4     ), 
    .o_psels5        ( h2p_o_psels5     ), 
    .o_psels6        ( h2p_o_psels6     ), 
    .o_psels7        ( h2p_o_psels7     ), 
    .o_psels8        ( h2p_o_psels8     ), 
    .o_psels9        ( h2p_o_psels9     ), 
    .o_psels10       ( h2p_o_psels10    ), 
    .o_psels11       ( h2p_o_psels11    ), 
    .o_psels12       ( h2p_o_psels12    ), 
    .o_psels13       ( h2p_o_psels13    ), 
    .o_psels14       ( h2p_o_psels14    ), 
    .o_psels15       ( h2p_o_psels15    ), 
    .o_paddr         ( h2p_o_paddr      ), 
    .o_pwrite        ( h2p_o_pwrite     )  
    ); 

osr_wdt u_wdt (
    .pclk           ( wdt_pclk        ), 
    .presetn        ( wdt_presetn     ), 
    .i_psel         ( wdt_i_psel      ), 
    .i_pwrite       ( wdt_i_pwrite    ), 
    .i_penable      ( wdt_i_penable   ), 
    .i_paddr        ( wdt_i_paddr     ), 
    .i_pwdata       ( wdt_i_pwdata    ), 
    .o_prdata       ( wdt_o_prdata    ), 
    .o_wdtintr      ( wdt_o_wdtintr   ), 
    .o_nresetout    ( wdt_o_nresetout )  
    ); 

osr_uart u_uart (
    .pclk         ( uart_pclk     ), 
    .presetn      ( uart_presetn  ), 
    .psel         ( uart_psel     ), 
    .paddr        ( uart_paddr    ), 
    .penable      ( uart_penable  ), 
    .pwrite       ( uart_pwrite   ), 
    .pwdata       ( uart_pwdata   ), 
    .prdata       ( uart_prdata   ), 
    .pready       ( uart_pready   ), 
    .pslverr      ( uart_pslverr  ), 
    .rxd          ( uart_rxd      ), 
    .txd          ( uart_txd      ), 
    .txen         ( uart_txen     ), 
    .baudtick     ( uart_baudtick ), 
    .txint        ( uart_txint    ), 
    .rxint        ( uart_rxint    ), 
    .txovrint     ( uart_txovrint ), 
    .rxovrint     ( uart_rxovrint ), 
    .uartint      ( uart_uartint  )  
    ); 

osr_apb_crc u_crc (
    .pclk         ( crc_pclk      ), 
    .presetn      ( crc_presetn   ), 
    .i_psel       ( crc_i_psel    ), 
    .i_penable    ( crc_i_penable ), 
    .i_pwrite     ( crc_i_pwrite  ), 
    .i_paddr      ( crc_i_paddr   ), 
    .i_pwdata     ( crc_i_pwdata  ), 
    .o_prdata     ( crc_o_prdata  )  
    ); 

osr_timer u_timer (
    .pclk          ( tim_pclk       ), 
    .presetn       ( tim_presetn    ), 
    .i_psel        ( tim_i_psel     ), 
    .i_penable     ( tim_i_penable  ), 
    .i_pwrite      ( tim_i_pwrite   ), 
    .i_paddr       ( tim_i_paddr    ), 
    .i_pwdata      ( tim_i_pwdata   ), 
    .o_prdata      ( tim_o_prdata   ), 
    .o_int_tim0    ( tim_o_int_tim0 ), 
    .o_int_tim1    ( tim_o_int_tim1 )  
    ); 

endmodule 
