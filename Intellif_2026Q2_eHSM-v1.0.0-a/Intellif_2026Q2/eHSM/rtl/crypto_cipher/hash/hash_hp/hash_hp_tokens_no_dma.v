//========================================================================================
// Copyright (C) 2024 Open Security Research Inc. - All Rights Reserved
//                                         Token      
// Define : DMA_EXT DMA32 
// feature_dma32 a9e5d03f18e5e08a2780db6bef17a7fdb365d8f4
// 813 1522181e89b3f31cc78698aacbac86cac5e5e69b
//========================================================================================
module osr_hash_hp_top #(
    parameter   p_AHB_ADDR_WIDTH            =  12
   ,parameter   p_AHB_DATA_WIDTH            =  32
   ,parameter   p_HMAC_EN                   =  1'b1
   ,parameter   p_MD5_EN                    =  1'b1
   ,parameter   p_SHA1_EN                   =  1'b1
   ,parameter   p_SHA256_EN                 =  1'b1
   ,parameter   p_SHA512_EN                 =  1'b1
   ,parameter   p_SHA3_EN                   =  1'b1
   ,parameter   p_SM3_EN                    =  1'b1
   ,parameter   p_CDC_EN                    =  1'b1
   ,parameter   p_ROUND_NUM                 =  1'd1
   ,parameter   p_DMA_EN                    =  1'b1
   ,parameter   p_DMA_ADDR_WIDTH            =  32
   ,parameter   p_DMA_DATA_WIDTH            =  32
   ,parameter   p_DMA_FIFO_DP_WIDTH         =  5
   ,parameter   p_MAX_OUTSTANDING           =  0
   ,parameter   [15:0] p_DMA_BYTE_GRANULARITY =  16'd128
   ,parameter   p_DMA_DATA_WORD             =  p_DMA_DATA_WIDTH/8
   ,parameter   p_PRJ_NUM                   =  16'h00E4
   ,parameter   p_MAR                       =  4'h1
   ,parameter   p_MIR                       =  4'h1
)(
    input   wire                            clk_core
   ,input   wire                            rst_n_core
   ,input   wire                            clk_ahb
   ,input   wire                            rst_n_ahb
   ,input   wire                            clk_axi         
   ,input   wire                            rst_n_axi       
   ,input   wire                            i_s_hsel
   ,input   wire    [p_AHB_ADDR_WIDTH-1:0]  i_s_haddr
   ,input   wire                            i_s_hwrite
   ,input   wire    [2:0]                   i_s_hsize
   ,input   wire    [2:0]                   i_s_hburst
   ,input   wire    [3:0]                   i_s_hprot
   ,input   wire    [1:0]                   i_s_htrans
   ,input   wire                            i_s_hmastlock
   ,input   wire                            i_s_hready
   ,input   wire    [p_AHB_DATA_WIDTH-1:0]  i_s_hwdata
   ,output  wire                            o_s_hreadyout
   ,output  wire                            o_s_hresp
   ,output  wire    [p_AHB_DATA_WIDTH-1:0]  o_s_hrdata
   ,output  wire    [p_DMA_ADDR_WIDTH-1:0]  o_dma_saddr
   ,output  wire    [p_DMA_ADDR_WIDTH-1:0]  o_dma_daddr
   ,output  wire    [31:0]                  o_dma_rlen
   ,output  wire    [31:0]                  o_dma_wlen
   ,output  wire                            o_dma_rstart
   ,output  wire                            o_dma_suspend
   ,output  wire                            o_dma_wstart
   ,input   wire                            i_dma_suspend_done
   ,input   wire                            i_dma_rdone
   ,input   wire                            i_dma_wdone
   ,output  wire    [p_DMA_FIFO_DP_WIDTH:0] o_dma_r_fifo_num_count
   ,output  wire                            o_dma_r_fifo_full
   ,input   wire                            i_dma_r_fifo_wr
   ,input   wire    [p_DMA_DATA_WIDTH-1:0]  i_dma_r_fifo_wr_data
   ,output  wire    [p_DMA_FIFO_DP_WIDTH:0] o_dma_w_fifo_num_count
   ,output  wire                            o_dma_w_fifo_empty
   ,input   wire                            i_dma_w_fifo_rd
   ,output  wire    [p_DMA_DATA_WIDTH-1:0]  o_dma_w_fifo_rd_data
   ,output  wire                            o_irq
   ,input   wire                            i_ahb_endian
   ,input   wire    [255:0]                 i_sp_data
   ,input   wire                            i_sp_valid
);
    localparam  Y126Zc              = 10;
    localparam  Y127Zc              = 0;
    localparam  Y128Zc              = 1;
    localparam  A220Yc              = p_SHA3_EN ? 1152 : 1024;
    localparam  A222Yc              = p_SHA3_EN ? 1600 : 512;
    localparam  A221Yc              = A220Yc      /32;
    localparam  A223Yc              = A222Yc        /32;
    localparam  A714Yc              = 3;
    localparam  A715Yc              = p_DMA_DATA_WIDTH;
    localparam  A716Yc              = p_DMA_FIFO_DP_WIDTH;
    localparam  Y239Zc              = 01;
    localparam  Y240Zc              = p_DMA_DATA_WIDTH;
    localparam  Y241Zc              = p_DMA_ADDR_WIDTH;
    localparam  Y242Zc              = 8;
    localparam  Y243Zc              = 8;
    localparam  Y244Zc              = A716Yc           ;
    localparam  Y245Zc              = A716Yc           ;
    localparam  Y246Zc              = 3;
    localparam  Y247Zc              = 1;
    localparam  Y248Zc              = 0;
    localparam  Y249Zc              = 32;
    localparam  Y250Zc              = 0;
    localparam  Y251Zc              = 0;
    localparam  Y252Zc              = 1;
    localparam  Y253Zc              = 1;
    localparam  Y254Zc              = p_CDC_EN;
    localparam  Y255Zc              = p_CDC_EN;
    localparam  Y256Zc              = 0;
    localparam  Y257Zc              = 0;
    localparam  Y258Zc              = 0;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y259Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y260Zc                  ;
    wire    [p_AHB_ADDR_WIDTH-1:0]      Y261Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y262Zc                  ;
    wire                                Y263Zc                  ;
    wire                                Y264Zc                  ;
    wire    [3:0]                       Y265Zc                  ;
    wire                                Y266Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y267Zc                  ;
    wire                                Y268Zc                  ;
    wire    [15:0]                      Y269Zc                  ;
    wire                                Y270Zc                  ;
    wire    [15:0]                      Y271Zc                  ;
    wire                                Y272Zc                  ;
    wire    [p_AHB_ADDR_WIDTH-1:0]      Y273Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y274Zc                  ;
    wire                                Y275Zc                  ;
    wire                                Y276Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y277Zc                  ;
    wire    [p_AHB_ADDR_WIDTH-1:0]      Y278Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y279Zc                  ;
    wire                                Y280Zc                  ;
    wire                                Y281Zc                  ;
    wire    [p_AHB_DATA_WIDTH-1:0]      Y282Zc                  ;
    wire                                Y283Zc                  ;
    wire                                Y284Zc                  ;
    wire                                Y285Zc                  ;
    wire                                Y286Zc                  ;
    wire    [3:0]                       A511Yc                  ;
    wire                                Y287Zc                  ;
    wire                                Y288Zc                  ;
    wire    [1:0]                       Y289Zc                  ;
    wire                                Y290Zc                  ;
    wire                                Y291Zc                  ;
    wire                                Y292Zc                  ;
    wire                                Y293Zc                  ;
    wire                                Y294Zc                  ;
    wire                                Y295Zc                  ;
    wire                                Y296Zc                  ;
    wire                                Y297Zc                    ;
    wire                                Y298Zc                  ;
    wire                                Y299Zc                  ;
    wire                                Y300Zc                  ;
    wire    [127:0]                     Y301Zc                  ;
    wire    [127:0]                     Y302Zc                  ;
    wire                                Y303Zc                  ;
    wire                                Y304Zc                  ;
    wire    [31:0]                      Y305Zc                  ;
    wire    [31:0]                      Y306Zc                  ;
    wire                                Y307Zc                  ;
    wire                                Y308Zc                  ;
    wire                                Y309Zc                  ;
    wire                                Y310Zc                  ;
    wire    [31:0]                      Y311Zc                  ;
    wire    [31:0]                      Y312Zc                  ;
    wire                                Y313Zc                  ;
    wire                                Y314Zc                  ;
    wire    [31:0]                      Y315Zc                  ;
    wire                                Y316Zc                  ;
    wire    [5:0]                       Y317Zc                  ;
    wire    [31:0]                      Y318Zc                  ;
    wire                                Y319Zc                  ;
    wire    [5:0]                       Y320Zc                  ;
    wire    [1599:0]                    Y321Zc                  ;
    wire    [49:0]                      Y322Zc                  ;
    wire    [A222Yc        -1:0]        Y323Zc                  ;
    wire    [2:0]                       Y324Zc                  ;
    wire    [1:0]                       Y325Zc                  ;
    wire    [63:0]                      Y326Zc                  ;
    wire    [63:0]                      Y327Zc                  ;
    wire    [31:0]                      Y328Zc                  ;
    wire    [31:0]                      Y329Zc                  ;
    wire    [3:0]                       Y330Zc                  ;
    wire    [3:0]                       Y331Zc                  ;
    wire    [31:0]                      Y332Zc                  ;
    wire    [31:0]                      Y333Zc                  ;
    wire    [127:0]                     A227Yc                  ;
    wire                                A229Yc                  ;
    wire                                Y334Zc                  ;
    wire                                A136Yc                  ;
    wire                                A943Yc                  ;
    wire                                Y335Zc                  ;
    wire    [A715Yc            -1:0]    A950Yc                  ;
    wire    [A715Yc            -1:0]    Y336Zc                  ;
    wire                                Y337Zc                  ;
    wire                                Y338Zc                  ;
    wire    [A716Yc           :0]       Y339Zc                  ;
    wire                                Y340Zc                  ;
    wire                                A935Yc                  ;
    wire                                A936Yc                  ;
    wire                                Y341Zc                  ;
    wire                                A934Yc                  ;
    wire                                Y342Zc                  ;
    wire    [A715Yc            -1:0]    A942Yc                  ;
    wire    [A715Yc            -1:0]    Y343Zc                  ;
    wire                                Y344Zc                  ;
    wire                                Y345Zc                  ;
    wire    [A716Yc           :0]       Y346Zc                  ;
    wire                                Y347Zc                  ;
    wire    [p_DMA_ADDR_WIDTH-1:0]      Y348Zc                  ;
    wire    [2:0]                       Y349Zc                  ;
    wire    [1:0]                       Y350Zc                  ;
    wire    [1:0]                       Y351Zc                  ;
    wire    [p_DMA_DATA_WIDTH-1:0]      Y352Zc                  ;
    wire    [3:0]                       Y353Zc                  ;
    wire                                Y354Zc                  ;
    wire    [p_DMA_DATA_WIDTH-1:0]      Y355Zc                  ;
    wire                                Y356Zc                  ;
    wire    [p_DMA_ADDR_WIDTH-1:0]      Y357Zc                  ;
    wire    [2:0]                       Y358Zc                  ;
    wire    [1:0]                       Y359Zc                  ;
    wire    [1:0]                       Y360Zc                  ;
    wire    [p_DMA_DATA_WIDTH-1:0]      Y361Zc                  ;
    wire    [3:0]                       Y362Zc                  ;
    wire                                Y363Zc                  ;
    wire    [p_DMA_DATA_WIDTH-1:0]      Y364Zc                  ;
    wire                                Y365Zc                  ;
    wire                                Y195Zc                  ;
    wire                                Y366Zc                  ;
    wire                                Y367Zc                  ;
    wire                                Y368Zc                  ;
    wire                                Y369Zc                  ;
    wire                                Y370Zc                  ;
    wire                                Y371Zc                  ;
    wire    [127:0]                     Y372Zc                  ;
    wire    [31:0]                      Y373Zc                  ;
    wire                                Y374Zc                  ;
    wire                                Y375Zc                  ;
    wire    [3:0]                       Y376Zc                  ;
    wire                                Y377Zc                  ;
    wire                                Y378Zc                  ;
    wire    [A223Yc       -1:0]         Y379Zc                  ;
    wire                                Y380Zc                  ;
    wire                                Y381Zc                  ;
    wire                                Y382Zc                  ;
    wire                                Y383Zc                  ;
    wire                                A928Yc                  ;
    wire                                Y384Zc                  ;
    wire    [127:0]                     Y385Zc                  ;
    wire                                Y386Zc                  ;
    wire    [A222Yc        -1:0]        Y387Zc                  ;
    wire    [A220Yc      -1:0]          Y388Zc                  ;
    wire    [A222Yc        -1:0]        Y389Zc                  ;
    wire    [A221Yc     -1:0]           Y390Zc                  ;
    wire                                Y391Zc                  ;
    wire                                Y392Zc                  ;
    wire                                Y393Zc                  ;
    wire                                Y394Zc                  ;
    wire                                Y395Zc                  ;
    wire                                Y396Zc                  ;
    wire                                Y397Zc                  ;
    wire                                Y398Zc                  ;
    wire    [7:0]                       Y399Zc                  ;
    wire    [7:0]                       Y400Zc                  ;
    wire    [A714Yc            -1:0]    Y401Zc                  ;
    wire    [A714Yc            -1:0]    Y402Zc                  ;
    wire    [A714Yc            -1:0]    Y403Zc                  ;
    wire                                A933Yc                   ;
    wire                                Y404Zc                  ;
    wire                                Y405Zc                  ;
    wire                                Y406Zc                  ;
    wire                                Y407Zc                  ;
    wire                                Y408Zc                  ;
    wire    [511:0]                     Y409Zc                  ;
    wire                            Y062Zc                      ;
hash_hp_Y145Zc        
    #(
      .Y146Zc             ("Y147Zc"                 )
   ,  .Y148Zc             (p_AHB_DATA_WIDTH       )
    )
    Y410Zc             
    (
      .A486Yc             (i_ahb_endian           )
   ,  .A152Yc             (Y260Zc                 )
   ,  .Y149Zc             (o_s_hrdata             )
    );
hash_hp_Y145Zc        
    #(
      .Y146Zc             ("Y147Zc"                 )
   ,  .Y148Zc             (p_AHB_DATA_WIDTH       )
    )
    Y411Zc             
    (
      .A486Yc             (i_ahb_endian           )
   ,  .A152Yc             (i_s_hwdata             )
   ,  .Y149Zc             (Y259Zc                 )
    );
hash_hp_A658Yc        
    #(
      .A659Yc             (p_AHB_ADDR_WIDTH       )
   ,  .A660Yc             (p_AHB_DATA_WIDTH       )
    )
    Y412Zc  
    (
      .i_s_hclk           (clk_ahb                )
   ,  .i_s_hresetn        (rst_n_ahb              )
   ,  .A661Yc             (i_s_hsel               )
   ,  .A662Yc             (i_s_haddr              )
   ,  .A663Yc             (i_s_hwrite             )
   ,  .A664Yc             (i_s_hsize              )
   ,  .A665Yc             (i_s_hburst             )
   ,  .A666Yc             (i_s_hprot              )
   ,  .A667Yc             (i_s_htrans             )
   ,  .A668Yc             (i_s_hmastlock          )
   ,  .A669Yc             (i_s_hready             )
   ,  .A670Yc             (Y259Zc                 )
   ,  .A671Yc             (o_s_hreadyout          )
   ,  .A672Yc             (o_s_hresp              )
   ,  .A673Yc             (Y260Zc                 )
   ,  .A674Yc             (Y261Zc                 )
   ,  .A675Yc             (Y262Zc                 )
   ,  .A676Yc             (Y263Zc                 )
   ,  .A677Yc             (Y264Zc                 )
   ,  .A678Yc             (Y265Zc                 )
   ,  .A679Yc             (Y266Zc                 )
   ,  .A680Yc             (Y267Zc                 )
   ,  .A681Yc             (Y272Zc                 )
    );
hash_hp_reg_bank 
# ( 
.p_AHB_ADDR_WIDTH (p_AHB_ADDR_WIDTH ) 
, .p_AHB_DATA_WIDTH (p_AHB_DATA_WIDTH ) 
, .p_PRJ_NUM (p_PRJ_NUM ) 
, .p_CDC_EN (p_CDC_EN ) 
, .p_DIGEST_WIDTH (A222Yc         ) 
, .p_MAR (p_MAR ) 
, .p_MIR (p_MIR ) 
) 
A547Yc     
( 
.clk (clk_ahb ) 
, .rst_n (rst_n_ahb ) 
, .i_addr (Y261Zc        ) 
, .i_wdata (Y262Zc         ) 
, .i_read_en (Y263Zc           ) 
, .i_write_en (Y264Zc            ) 
, .i_wstrobe (Y265Zc           ) 
, .i_priority (Y266Zc            ) 
, .o_rdata (Y267Zc         ) 
, .o_readyout (Y272Zc         ) 
, .o_enable (Y283Zc      ) 
, .o_suspend (Y285Zc       ) 
, .i_clr_enable (Y369Zc               ) 
, .i_clr_suspend (Y370Zc                ) 
, .i_suspend_sr (Y371Zc               ) 
, .o_msel (A511Yc    ) 
, .o_hmacen (Y287Zc      ) 
, .o_spen (Y288Zc    ) 
, .o_data_type (Y289Zc            ) 
, .o_up_cfg (Y290Zc      ) 
, .o_dmaen (Y291Zc     ) 
, .i_core_ris (Y393Zc             ) 
, .o_core_done_clr (Y292Zc             ) 
, .i_dma_ris (Y397Zc            ) 
, .o_dma_done_clr (Y294Zc            ) 
, .i_suspend_ris (Y398Zc                ) 
, .o_suspend_done_clr (Y295Zc                ) 
, .o_core_irqen (Y298Zc          ) 
, .o_dma_irqen (Y299Zc         ) 
, .o_suspend_irqen (Y300Zc             ) 
, .o_msg_len (Y301Zc       ) 
, .o_msg_cnt (Y302Zc       ) 
, .o_msg_cnt_wren (Y303Zc            ) 
, .i_msg_cnt (Y372Zc         ) 
, .o_key_len (Y305Zc       ) 
, .o_key_cnt (Y306Zc       ) 
, .o_key_cnt_wren (Y307Zc            ) 
, .i_key_cnt (Y373Zc         ) 
, .o_hmac_key (Y309Zc        ) 
, .o_last (Y310Zc    ) 
, .o_mdin (Y311Zc    ) 
, .o_mdin_wren (Y313Zc         ) 
, .i_core_ready (Y375Zc               ) 
, .o_hash_in (Y315Zc    ) 
, .o_hash_in_wren (Y316Zc         ) 
, .o_hash_in_addr (Y317Zc         ) 
, .i_hash_out (Y323Zc        ) 
, .i_pro_msg_cnt (A227Yc             ) 
, .i_blk_pad_fail (Y334Zc                 ) 
, .o_mbl () 
, .o_motdl () 
, .i_to (1'b0) 
, .o_to_threshold () 
, .o_saddr (Y326Zc        ) 
, .o_daddr (Y327Zc        ) 
, .o_rlen (Y328Zc       ) 
, .o_wlen (Y329Zc       ) 
, .o_arbus (Y332Zc     ) 
, .o_awbus (Y333Zc     ) 
, .o_arid () 
, .o_awid () 
); 
    generate
    if (p_CDC_EN) begin:Y446Zc   
hash_hp_Y169Zc          #(.Y125Zc      (p_DMA_ADDR_WIDTH))
            Y447Zc           (
             .clk_axi               (clk_axi        )
            ,.rst_n_axi             (rst_n_axi      )
            ,.clk_core              (clk_core       )
            ,.rst_n_core            (rst_n_core     )
            ,.Y170Zc                (Y326Zc        [p_DMA_ADDR_WIDTH-1:0] )
            ,.Y171Zc                (Y327Zc        [p_DMA_ADDR_WIDTH-1:0] )
            ,.Y172Zc                (Y328Zc         )
            ,.Y173Zc                (Y329Zc         )
            ,.Y174Zc                (A935Yc         )
            ,.Y175Zc                (Y341Zc            )
            ,.Y176Zc                (A934Yc         )
            ,.Y177Zc                (A943Yc         )
            ,.Y178Zc                (Y296Zc                )
            ,.Y180Zc                (Y335Zc         )
            ,.Y181Zc                (Y342Zc         )
            ,.Y179Zc                 (A936Yc            )
            ,.Y182Zc                (i_dma_wdone    )
            ,.Y183Zc                (i_dma_rdone    )
            ,.Y184Zc                (i_dma_suspend_done    )
            ,.Y185Zc                (o_dma_saddr    )
            ,.Y186Zc                (o_dma_daddr    )
            ,.Y187Zc                (o_dma_rlen     )
            ,.Y188Zc                (o_dma_wlen     )
            ,.Y189Zc                (o_dma_wstart   )
            ,.Y190Zc                (o_dma_rstart   )
            ,.Y192Zc                (o_dma_suspend  )
            ,.Y193Zc                (Y195Zc            )
            ,.Y191Zc                (Y062Zc         )
        );
    end
    else begin:Y448Zc    
            assign o_dma_suspend  = A935Yc       ;
            assign Y195Zc            = Y341Zc          ;
            assign o_dma_rstart = A934Yc       ;
            assign o_dma_wstart = A943Yc       ;
            assign Y062Zc         = Y296Zc                ;
            assign o_dma_saddr  = Y326Zc        [p_DMA_ADDR_WIDTH-1:0];
            assign o_dma_daddr  = Y327Zc        [p_DMA_ADDR_WIDTH-1:0];
            reg    Y449Zc     ;
            reg    Y450Zc     ;
            assign o_dma_rlen = Y328Zc       ;
            assign o_dma_wlen = Y329Zc       ;
            always @(posedge clk_axi or negedge rst_n_axi)
            if (!rst_n_axi) begin
                Y449Zc      <= 1'b0;
                Y450Zc      <= 1'b0;
            end
            else begin
                Y449Zc      <=  o_dma_wstart ? 1'b1 :
                                i_dma_wdone ? 1'b0 :
                                Y449Zc     ;
                Y450Zc      <=  o_dma_rstart ? 1'b1 :
                                i_dma_rdone ? 1'b0 :
                                Y450Zc     ;
            end
            assign Y335Zc       = Y449Zc     ;
            assign Y342Zc       = Y450Zc     ;
            assign A936Yc             = i_dma_suspend_done;
    end
    endgenerate
    assign o_dma_r_fifo_num_count = Y346Zc                ;
    assign o_dma_r_fifo_full      = Y345Zc           ;
    assign Y344Zc                 = i_dma_r_fifo_wr;
    assign A942Yc                 = i_dma_r_fifo_wr_data;
    assign o_dma_w_fifo_num_count = Y339Zc                ;
    assign o_dma_w_fifo_empty     = Y338Zc            ;
    assign o_dma_w_fifo_rd_data   = A950Yc              ;
    assign Y337Zc                 = i_dma_w_fifo_rd;
    generate
    if(p_CDC_EN==1'b1) begin : Y451Zc
hash_hp_Y058Zc     
    #(
      .A220Yc             (p_AHB_DATA_WIDTH       )
    )
    Y452Zc       
    (
      .clk_core           (clk_core               )
   ,  .rst_n_core         (rst_n_core             )
   ,  .clk_ahb            (clk_ahb                )
   ,  .rst_n_ahb          (rst_n_ahb              )
   ,  .A718Yc             (Y283Zc                 )
   ,  .Y059Zc             (Y284Zc                 )
   ,  .A068Yc             (Y285Zc                 )
   ,  .Y060Zc             (Y286Zc                 )
   ,  .A781Yc             (Y292Zc                 )
   ,  .Y061Zc             (Y293Zc                 )
   ,  .A782Yc             (Y294Zc                 )
   ,  .Y062Zc             (Y296Zc                 )
   ,  .A783Yc             (Y295Zc                     )
   ,  .Y063Zc             (Y297Zc                     )
   ,  .A731Yc             (Y303Zc                 )
   ,  .Y064Zc             (Y304Zc                 )
   ,  .A735Yc             (Y307Zc                 )
   ,  .Y065Zc             (Y308Zc                 )
   ,  .Y066Zc             (Y366Zc                 )
   ,  .A719Yc             (Y369Zc                 )
   ,  .Y067Zc             (Y367Zc                 )
   ,  .A720Yc             (Y370Zc                 )
   ,  .Y068Zc             (Y368Zc                 )
   ,  .A721Yc             (Y371Zc                 )
   ,  .Y069Zc             (A229Yc                 )
   ,  .Y070Zc             (Y334Zc                 )
   ,  .Y071Zc             (Y392Zc                 )
   ,  .Y072Zc             (Y393Zc                 )
   ,  .Y073Zc             (Y395Zc                 )
   ,  .Y074Zc             (Y397Zc                 )
   ,  .Y075Zc             (Y396Zc                 )
   ,  .Y076Zc             (Y398Zc                 )
   ,  .A739Yc             (Y311Zc                 )
   ,  .A740Yc             (Y313Zc                 )
   ,  .A744Yc             (Y375Zc                 )
   ,  .Y077Zc             (Y312Zc                 )
   ,  .Y078Zc             (Y314Zc                 )
   ,  .Y079Zc             (Y374Zc                 )
   ,  .Y080Zc              (Y315Zc                 )
   ,  .Y081Zc             (Y316Zc                 )
   ,  .Y082Zc             (Y317Zc                 )
   ,  .Y083Zc              (Y318Zc                 )
   ,  .Y084Zc             (Y319Zc                 )
   ,  .Y085Zc             (Y320Zc                 )
   ,  .A789Yc              (A933Yc                  )
   ,  .A706Yc              (o_irq                  )
    );
    end
    else begin : Y453Zc
        assign Y284Zc                   = Y283Zc     ;
        assign Y286Zc                   = Y285Zc      ;
        assign Y334Zc                   = A229Yc             ;
        assign Y369Zc                   = Y366Zc           ;
        assign Y370Zc                   = Y367Zc            ;
        assign Y371Zc                   = Y368Zc           ;
        assign Y293Zc                   = Y292Zc            ;
        assign Y296Zc                   = Y294Zc           ;
        assign Y297Zc                       = Y295Zc               ;
        assign Y304Zc                   = Y303Zc           ;
        assign Y308Zc                   = Y307Zc           ;
        assign Y393Zc                   = Y392Zc         ;
        assign Y397Zc                   = Y395Zc        ;
        assign Y398Zc                   = Y396Zc            ;
        assign Y375Zc                   = Y374Zc           ;
        assign Y312Zc                   = Y311Zc   ;
        assign Y314Zc                   = Y313Zc        ;
        assign Y318Zc                   = Y315Zc   ;
        assign Y319Zc                   = Y316Zc        ;
        assign Y320Zc                   = Y317Zc        ;
        assign o_irq                    = A933Yc;
    end
    endgenerate
    function [31:0] Y454Zc           (
        input [31:0] Y455Zc 
    );
        Y454Zc           = {
            Y455Zc [(8*0)+:8],
            Y455Zc [(8*1)+:8],
            Y455Zc [(8*2)+:8],
            Y455Zc [(8*3)+:8] 
        };
    endfunction
    wire [255:0] Y456Zc       ;
    wire [511:0] Y457Zc    ;
    assign Y456Zc        =
        i_ahb_endian==1'b0 ?
            {
                Y454Zc          (i_sp_data[(255-32*0)-:32]),
                Y454Zc          (i_sp_data[(255-32*1)-:32]),
                Y454Zc          (i_sp_data[(255-32*2)-:32]),
                Y454Zc          (i_sp_data[(255-32*3)-:32]),
                Y454Zc          (i_sp_data[(255-32*4)-:32]),
                Y454Zc          (i_sp_data[(255-32*5)-:32]),
                Y454Zc          (i_sp_data[(255-32*6)-:32]),
                Y454Zc          (i_sp_data[(255-32*7)-:32]) 
            }
      : i_sp_data
    ;
hash_hp_Y158Zc              Y458Zc               (
        .clk            (clk_ahb            )
       ,.rst_n          (rst_n_ahb          )
       ,.Y161Zc         (i_sp_valid         )
       ,.A152Yc         (Y456Zc             )
       ,.Y162Zc          (Y457Zc             )
    );
    assign Y409Zc   =
        {
            Y457Zc    [(32*8 )+:32],
            Y457Zc    [(32*9 )+:32],
            Y457Zc    [(32*10)+:32],
            Y457Zc    [(32*11)+:32],
            Y457Zc    [(32*12)+:32],
            Y457Zc    [(32*13)+:32],
            Y457Zc    [(32*14)+:32],
            Y457Zc    [(32*15)+:32],
            Y457Zc    [(32*0)+:32],
            Y457Zc    [(32*1)+:32],
            Y457Zc    [(32*2)+:32],
            Y457Zc    [(32*3)+:32],
            Y457Zc    [(32*4)+:32],
            Y457Zc    [(32*5)+:32],
            Y457Zc    [(32*6)+:32],
            Y457Zc    [(32*7)+:32] 
        }
    ;
hash_hp_A713Yc          
    #(
      .A714Yc             (A714Yc                 )
   ,  .A715Yc             (A715Yc                 )
   ,  .A716Yc             (A716Yc                 )
   ,  .A717Yc             (p_CDC_EN               )
   ,  .A061Yc             (p_MD5_EN               )
   ,  .A062Yc             (p_SHA1_EN              )
   ,  .A063Yc             (p_SHA256_EN            )
   ,  .A064Yc             (p_SHA512_EN            )
   ,  .A219Yc             (p_SHA3_EN              )
   ,  .A065Yc             (p_SM3_EN               )
    )
    Y459Zc    
    (
      .clk_axi            (clk_axi                )
   ,  .rst_n_axi          (rst_n_axi              )
   ,  .clk_core           (clk_core               )
   ,  .rst_n_core         (rst_n_core             )
   ,  .A718Yc             (Y284Zc                 )
   ,  .A719Yc             (Y366Zc                 )
   ,  .A720Yc             (Y367Zc                 )
   ,  .A721Yc             (Y368Zc                 )
   ,  .A111Yc             (A511Yc                 )
   ,  .A722Yc             (Y287Zc                 )
   ,  .A723Yc             (Y288Zc                 )
   ,  .A724Yc             (Y290Zc                 )
   ,  .A725Yc             (Y291Zc                 )
   ,  .A726Yc             (Y298Zc                 )
   ,  .A727Yc             (Y299Zc                 )
   ,  .A728Yc             (Y300Zc                 )
   ,  .A729Yc             (Y301Zc                 )
   ,  .A730Yc             (Y302Zc                 )
   ,  .A731Yc             (Y304Zc                 )
   ,  .A732Yc             (Y372Zc                 )
   ,  .A733Yc             (Y305Zc                 )
   ,  .A734Yc             (Y306Zc                 )
   ,  .A735Yc             (Y308Zc                 )
   ,  .A736Yc             (Y373Zc                 )
   ,  .A737Yc             (Y309Zc                 )
   ,  .A738Yc             (Y409Zc                 )
   ,  .A112Yc             (Y310Zc                 )
   ,  .A739Yc             (Y312Zc                 )
   ,  .A740Yc             (Y314Zc                 )
   ,  .A741Yc             (Y318Zc                 )
   ,  .A155Yc             (Y319Zc                 )
   ,  .A742Yc             (Y320Zc                 )
   ,  .A743Yc             (Y323Zc       [A222Yc        -1:0])
   ,  .A744Yc             (Y374Zc                 )
   ,  .A745Yc             (Y376Zc                 )
   ,  .A746Yc             (Y377Zc                 )
   ,  .A747Yc             (Y405Zc                 )
   ,  .A748Yc             (Y378Zc                 )
   ,  .A749Yc             (Y379Zc                 )
   ,  .A754Yc             (Y380Zc                 )
   ,  .A750Yc             (Y381Zc                 )
   ,  .A751Yc             (Y382Zc                 )
   ,  .A752Yc             (Y383Zc                 )
   ,  .A753Yc             (A928Yc                 )
   ,  .A755Yc             (Y384Zc                 )
   ,  .A756Yc             (Y385Zc                 )
   ,  .A758Yc             (Y386Zc                 )
   ,  .A757Yc             (Y406Zc                 )
   ,  .A759Yc             (Y388Zc                 )
   ,  .A760Yc             (Y387Zc                 )
   ,  .A761Yc             (Y389Zc                 )
   ,  .A762Yc             (Y390Zc                 )
   ,  .A763Yc             (Y391Zc                 )
   ,  .A764Yc             (Y408Zc                 )
   ,  .A068Yc             (Y286Zc                 )
   ,  .A363Yc             (A136Yc                 )
   ,  .A765Yc             (Y195Zc                 )
   ,  .A767Yc             (A935Yc                 )
   ,  .A766Yc             (Y341Zc                 )
   ,  .A768Yc             (A934Yc                 )
   ,  .A769Yc             (Y342Zc                 )
   ,  .A770Yc              
                                                        (A942Yc                 )
   ,  .A771Yc             (Y344Zc                 )
   ,  .A772Yc             (Y345Zc                 )
   ,  .A773Yc                
                                                        (Y346Zc                 )
   ,  .A774Yc             (A943Yc                 )
   ,  .A775Yc             (Y335Zc                 )
   ,  .A776Yc             (A936Yc                 )
   ,  .A777Yc              
                                                        (A950Yc                 )
   ,  .A778Yc             (Y337Zc                 )
   ,  .A779Yc             (Y338Zc                 )
   ,  .A780Yc                
                                                        (Y339Zc                 )
   ,  .A781Yc             (Y293Zc                 )
   ,  .A782Yc             (Y296Zc                 )
   ,  .A783Yc             (Y297Zc                    )
   ,  .A784Yc             (Y392Zc                 )
   ,  .A785Yc             (Y395Zc                 )
   ,  .A786Yc             (Y396Zc                 )
   ,  .A787Yc             (Y401Zc                 )
   ,  .A788Yc             (Y402Zc                 )
   ,  .A789Yc              (Y403Zc                 )
   ,  .A706Yc              (A933Yc                  )
   ,  .A790Yc             (Y404Zc                 )   
    );
hash_hp_A702Yc         
    #(
      .A703Yc             (A714Yc                 )
    )
    Y460Zc   
    (
      .clk                (clk_core               )
   ,  .rst_n              (rst_n_core             )
   ,  .A704Yc             (Y401Zc                 )
   ,  .A705Yc             (Y402Zc                 )
   ,  .A706Yc              (Y403Zc                 )
    );
hash_hp_A504Yc            
    #(
      .A505Yc             (p_HMAC_EN              )
   ,  .A061Yc             (p_MD5_EN               )
   ,  .A062Yc             (p_SHA1_EN              )
   ,  .A063Yc             (p_SHA256_EN            )
   ,  .A064Yc             (p_SHA512_EN            )
   ,  .A065Yc             (p_SM3_EN               )
   ,  .A219Yc             (p_SHA3_EN              )
   ,  .A066Yc             (p_ROUND_NUM            )
   ,  .A220Yc             ()
   ,  .A221Yc             ()
   ,  .A222Yc             ()
   ,  .A223Yc             ()
   ,  .A506Yc             (p_PRJ_NUM              )
   ,  .A507Yc              (p_MAR                  )
   ,  .A508Yc              (p_MIR                  )
    )
    Y461Zc              
    (
      .clk                (clk_core               )
   ,  .rst_n              (rst_n_core             )
   ,  .A111Yc             (Y376Zc                 )
   ,  .A105Yc             (Y377Zc                 )
   ,  .A424Yc             (1'b1                   )
   ,  .A486Yc             (3'd0                   )
   ,  .A441Yc              (Y405Zc                 )
   ,  .A355Yc             (Y378Zc                 )
   ,  .A154Yc             (Y387Zc                 )
   ,  .A365Yc             (Y379Zc                 )
   ,  .A104Yc             (Y380Zc                 )
   ,  .A068Yc             (A928Yc                 )
   ,  .A364Yc             (Y381Zc                 )
   ,  .A109Yc             (Y382Zc                 )
   ,  .A110Yc             (Y383Zc                 )
   ,  .A112Yc             (Y384Zc                 )
   ,  .A113Yc              (Y385Zc                 )
   ,  .A442Yc              (Y406Zc                 )
   ,  .A152Yc             (Y388Zc                 )
   ,  .A362Yc             (Y390Zc                 )
   ,  .A354Yc             (Y391Zc                 )
   ,  .A440Yc             (Y386Zc                 )
   ,  .A433Yc             (Y407Zc                 )
   ,  .A224Yc             (Y389Zc                 )
   ,  .A432Yc             (Y408Zc                 )
   ,  .A509Yc              ()
   ,  .A510Yc              ()
   ,  .A114Yc             (A227Yc                 )
   ,  .A115Yc             (A229Yc                 )
   ,  .A116Yc             (A136Yc                 )
    );
endmodule
module hash_hp_reg_bank
#(
    parameter           p_AHB_ADDR_WIDTH    =  12          
   ,parameter           p_AHB_DATA_WIDTH    =  32          
   ,parameter           p_CDC_EN            =  1'b0
   ,parameter           p_DIGEST_WIDTH      =  1600
   ,parameter   [15:0]  p_PRJ_NUM           =  16'h00E4
   ,parameter   [3:0]   p_MAR               =  4'h1    
   ,parameter   [3:0]   p_MIR               =  4'h1    
)(
    input   wire                            clk          
   ,input   wire                            rst_n         
   ,input   wire    [p_AHB_ADDR_WIDTH-1:0]  i_addr         
   ,input   wire    [p_AHB_DATA_WIDTH-1:0]  i_wdata        
   ,input   wire                            i_read_en      
   ,input   wire                            i_write_en     
   ,input   wire    [3:0]                   i_wstrobe      
   ,input   wire                            i_priority     
   ,output  reg     [p_AHB_DATA_WIDTH-1:0]  o_rdata        
   ,output  wire                            o_readyout     
   ,output  wire                            o_enable        
   ,output  wire                            o_suspend
   ,input   wire                            i_clr_enable    
   ,input   wire                            i_clr_suspend   
   ,input   wire                            i_suspend_sr    
   ,output  wire    [3:0]                   o_msel          
   ,output  wire                            o_hmacen
   ,output  wire                            o_spen          
   ,output  wire    [1:0]                   o_data_type                
   ,output  wire                            o_up_cfg 
   ,output  wire                            o_dmaen        
   ,input   wire                            i_core_ris      
   ,output  wire                            o_core_done_clr     
   ,input   wire                            i_dma_ris          
   ,output  wire                            o_dma_done_clr      
   ,input   wire                            i_suspend_ris      
   ,output  wire                            o_suspend_done_clr    
   ,output  wire                            o_core_irqen    
   ,output  wire                            o_dma_irqen         
   ,output  wire                            o_suspend_irqen         
   ,output  wire    [127:0]                 o_msg_len       
   ,output  wire    [127:0]                 o_msg_cnt       
   ,output  wire                            o_msg_cnt_wren
   ,input   wire    [127:0]                 i_msg_cnt      
   ,output  wire    [31:0]                  o_key_len       
   ,output  wire    [31:0]                  o_key_cnt       
   ,output  wire                            o_key_cnt_wren  
   ,input   wire    [127:0]                 i_pro_msg_cnt   
   ,input   wire                            i_blk_pad_fail  
   ,input   wire    [31:0]                  i_key_cnt      
   ,output  wire                            o_hmac_key      
   ,output  wire                            o_last         
   ,output  wire    [31:0]                  o_mdin          
   ,output  wire                            o_mdin_wren    
   ,input   wire                            i_core_ready   
   ,output  wire    [31:0]                  o_hash_in       
   ,output  wire                            o_hash_in_wren     
   ,output  wire    [5:0]                   o_hash_in_addr     
   ,input   wire    [p_DIGEST_WIDTH-1:0]    i_hash_out      
   ,output  wire    [2:0]                   o_mbl           
   ,output  wire    [1:0]                   o_motdl         
   ,input   wire                            i_to            
   ,output  wire    [15:0]                  o_to_threshold  
   ,output  wire    [63:0]                  o_saddr         
   ,output  wire    [63:0]                  o_daddr         
   ,output  wire    [31:0]                  o_rlen          
   ,output  wire    [31:0]                  o_wlen          
   ,output  wire    [31:0]                  o_arbus 
   ,output  wire    [31:0]                  o_awbus
   ,output  wire    [3:0]                   o_arid
   ,output  wire    [3:0]                   o_awid
);
    localparam                          p_REG_NUM       = 'd64      ;
    localparam                          p_HASH_SEL_BASE =  0        ;
    localparam                          p_DMA_SEL_BASE  =  17       ;
    localparam                          p_SEED_SEL_BASE =  28       ;
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_CTRL        = 12'h00      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_CFG         = 12'h04      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SUS_SR      = 12'h08      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_RISR        = 12'h10      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_IMCR        = 12'h14      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MISR        = 12'h18      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SUS_PAD_SR  = 12'h1C      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGLEN0     = 12'h30      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGLEN1     = 12'h34      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGLEN2     = 12'h38      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGLEN3     = 12'h3C      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGCNT0     = 12'h40      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGCNT1     = 12'h44      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGCNT2     = 12'h48      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MSGCNT3     = 12'h4C      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_KEYLEN      = 12'h60      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_KEYCNT      = 12'h70      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_PRO_MSGCNT0 = 12'h80      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_PRO_MSGCNT1 = 12'h84      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_PRO_MSGCNT2 = 12'h88      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_PRO_MSGCNT3 = 12'h8C      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MDIN_CR     = 12'hB0      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_MDIN        = 12'hC0      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_VERSION     = 12'hFC      ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_HASHINLB    = 12'h100     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_HASHINUB    = 12'h1C4     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_HASHOUTLB   = 12'h200     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_HASHOUTUB   = 12'h2C4     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_0      =  12'h300    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_1      =  12'h304    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_2      =  12'h308    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_3      =  12'h30c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_4      =  12'h310    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_5      =  12'h314    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_6      =  12'h318    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_7      =  12'h31c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_8      =  12'h320    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_9      =  12'h324    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_10     =  12'h328    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_11     =  12'h32c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_12     =  12'h330    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_13     =  12'h334    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_14     =  12'h338    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_15     =  12'h33c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_16     =  12'h340    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_17     =  12'h344    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_18     =  12'h348    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_19     =  12'h34c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_20     =  12'h350    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_21     =  12'h354    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_22     =  12'h358    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_23     =  12'h35c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_24     =  12'h360    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_25     =  12'h364    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_26     =  12'h368    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_27     =  12'h36c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_28     =  12'h370    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_29     =  12'h374    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_30     =  12'h378    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_31     =  12'h37c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_32     =  12'h380    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_33     =  12'h384    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_34     =  12'h388    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_SEED_35     =  12'h38c    ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMACR       = 12'h300     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMASR       = 12'h304     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMATOTH     = 12'h308     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMASADRL    = 12'h490     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMASADRH    = 12'h494     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMADADRL    = 12'h498     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMADADRH    = 12'h49c     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMARLEN     = 12'h4a0     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMAWLEN     = 12'h4a4     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMAAWCC     = 12'h4a8     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMAARCC     = 12'h4ac     ; 
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_A_DMAID       = 12'h4b0     ; 
    localparam  [31:0] p_VERSION = {p_PRJ_NUM,8'd0,p_MAR,p_MIR};
    wire    [31:0]          w_wdata_swp         ;
    wire    [31:0]          w_wdata_swp_t       ;
    reg                     r_enable            ;
    reg                     r_suspend           ;
    wire                    w_enable_in         ;
    wire                    w_suspend_in        ;
    reg     [3:0]           r_msel              ;
    reg                     r_hmacen            ;
    reg                     r_spen              ;
    reg     [1:0]           r_data_type         ;
    reg                     r_up_cfg            ;
    reg                     r_dmaen             ;
    wire    [3:0]           w_msel_in           ;
    wire                    w_hmacen_in         ;
    wire                    w_spen              ;
    wire    [1:0]           w_data_type_in      ;
    wire                    w_up_cfg_in         ;
    wire                    w_dmaen_in          ;
    reg                     r_core_irqen        ;
    reg                     r_dma_irqen         ;
    reg                     r_suspend_irqen     ;
    wire                    w_core_irqen_in     ;
    wire                    w_dma_irqen_in      ;
    wire                    w_suspend_irqen_in  ;
    wire                    w_core_mi           ;
    wire                    w_dma_mi            ;
    wire                    w_suspend_mi        ;
    wire                    w_core_done_clr     ;
    wire                    w_dma_done_clr      ;
    wire                    w_suspend_done_clr  ;
    reg     [31:0]          r_msg_len   [0:3]   ;
    wire    [31:0]          w_msg_len_in[0:3]   ;
    reg     [31:0]          r_msg_cnt   [0:3]   ;
    wire    [31:0]          w_msg_cnt_in[0:3]   ;
    wire    [3:0]           w_msg_len_cnt_equal ;
    reg     [3:0]           r_msg_cnt_wren      ;
    wire    [3:0]           w_msg_cnt_wren_in   ;
    wire                    w_msg_cnt_wren_o    ;
    reg     [31:0]          r_key_len           ;
    wire    [31:0]          w_key_len_in        ;
    reg     [31:0]          r_key_cnt           ;
    wire    [31:0]          w_key_cnt_in        ;
    reg                     r_key_cnt_wren      ;
    wire                    w_key_cnt_wren_in   ;
    reg                     r_hmac_key          ;
    reg                     r_last              ;
    wire                    w_hmac_key_in       ;
    wire                    w_last_in           ;
    wire                    w_mdin_wren         ;
    wire                    w_core_ready        ;
    wire                    w_hash_in_wren      ;
    wire    [49:0]          w_hin_wren          ;
    wire    [1599:0]        w_hin               ;
    wire    [5:0]           w_hash_in_addr      ;
    wire                    w_hout_rden         ;
    wire    [p_DIGEST_WIDTH-1:0]
                            w_hash_out          ;
    wire    [31:0]          w_hout_rdata        ;
    wire    [31:0]          w_hout_rdata_swp    ;
    wire    [31:0]          w_hout_rdata_swp_t  ;
    reg     [2:0]           r_mbl               ;
    reg     [1:0]           r_motdl             ;
    reg                     r_to                ;
    reg     [15:0]          r_to_thrd           ; 
    reg     [31:0]          r_saddr_l           ;
    reg     [31:0]          r_saddr_h           ;
    wire    [31:0]          w_saddr_l_pre       ;
    wire    [31:0]          w_saddr_h_pre       ;
    reg     [31:0]          r_daddr_l           ;
    reg     [31:0]          r_daddr_h           ;
    wire    [31:0]          w_daddr_l_pre       ;
    wire    [31:0]          w_daddr_h_pre       ;
    reg     [31:0]          r_rlen              ;
    wire    [31:0]          w_rlen_pre          ;
    reg     [31:0]          r_wlen              ;
    wire    [31:0]          w_wlen_pre          ;
    reg     [31:0]          r_arbus             ;
    reg     [31:0]          r_awbus             ;
    reg     [3:0]           r_arid              ;
    reg     [3:0]           r_awid              ;
    wire    [31:0]          w_arbus             ;
    wire    [31:0]          w_awbus             ;
    wire    [3:0]           w_arid              ;
    wire    [3:0]           w_awid              ;
    wire                    w_read_en_NC        ;
    assign  w_read_en_NC =  i_read_en           ;
    wire                    w_priority_NC       ;
    assign  w_priority_NC=  i_priority          ;
    wire                    w_to_NC             ;
    assign  w_to_NC      =  i_to                ;
    wire    [127:0]         w_msg_cnt_NC        ;
    assign  w_msg_cnt_NC =  i_msg_cnt           ;
    wire    [31:0]          w_key_cnt_NC        ;
    assign  w_key_cnt_NC =  i_key_cnt           ;
    wire    [p_REG_NUM-1:0]   w_wsel        ;
    genvar i;
    integer j;
    wire   w_adr_eq_ctrl         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_CTRL        ;
    wire   w_adr_eq_cfg          = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_CFG         ;
    wire   w_adr_eq_sus_sr       = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SUS_SR      ;
    wire   w_adr_eq_risr         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_RISR        ;
    wire   w_adr_eq_imcr         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_IMCR        ;
    wire   w_adr_eq_misr         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MISR        ;
    wire   w_adr_eq_sus_pad_sr   = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SUS_PAD_SR      ;
    wire   w_adr_eq_msglen0      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGLEN0     ;
    wire   w_adr_eq_msglen1      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGLEN1     ;
    wire   w_adr_eq_msglen2      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGLEN2     ;
    wire   w_adr_eq_msglen3      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGLEN3     ;
    wire   w_adr_eq_msgcnt0      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGCNT0     ;
    wire   w_adr_eq_msgcnt1      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGCNT1     ;
    wire   w_adr_eq_msgcnt2      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGCNT2     ;
    wire   w_adr_eq_msgcnt3      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MSGCNT3     ;
    wire   w_adr_eq_keylen       = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_KEYLEN      ;
    wire   w_adr_eq_keycnt       = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_KEYCNT      ;
    wire   w_adr_eq_pro_msgcnt0  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_PRO_MSGCNT0     ;
    wire   w_adr_eq_pro_msgcnt1  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_PRO_MSGCNT1     ;
    wire   w_adr_eq_pro_msgcnt2  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_PRO_MSGCNT2     ;
    wire   w_adr_eq_pro_msgcnt3  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_PRO_MSGCNT3     ;
    wire   w_adr_eq_mdin_cr      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MDIN_CR     ;
    wire   w_adr_eq_mdin         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_MDIN        ;
    wire   w_adr_eq_version      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_VERSION     ;
    wire   w_adr_eq_hashinlb     = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_HASHINLB    ;
    wire   w_adr_eq_seed[0:35]                                                          ;
    assign w_adr_eq_seed[0]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_0       ;
    assign w_adr_eq_seed[1]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_1       ;
    assign w_adr_eq_seed[2]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_2       ;
    assign w_adr_eq_seed[3]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_3       ;
    assign w_adr_eq_seed[4]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_4       ;
    assign w_adr_eq_seed[5]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_5       ;
    assign w_adr_eq_seed[6]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_6       ;
    assign w_adr_eq_seed[7]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_7       ;
    assign w_adr_eq_seed[8]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_8       ;
    assign w_adr_eq_seed[9]  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_9       ;
    assign w_adr_eq_seed[10] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_10      ;
    assign w_adr_eq_seed[11] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_11      ;
    assign w_adr_eq_seed[12] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_12      ;
    assign w_adr_eq_seed[13] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_13      ;
    assign w_adr_eq_seed[14] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_14      ;
    assign w_adr_eq_seed[15] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_15      ;
    assign w_adr_eq_seed[16] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_16      ;
    assign w_adr_eq_seed[17] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_17      ;
    assign w_adr_eq_seed[18] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_18      ;
    assign w_adr_eq_seed[19] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_19      ;
    assign w_adr_eq_seed[20] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_20      ;
    assign w_adr_eq_seed[21] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_21      ;
    assign w_adr_eq_seed[22] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_22      ;
    assign w_adr_eq_seed[23] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_23      ;
    assign w_adr_eq_seed[24] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_24      ;
    assign w_adr_eq_seed[25] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_25      ;
    assign w_adr_eq_seed[26] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_26      ;
    assign w_adr_eq_seed[27] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_27      ;
    assign w_adr_eq_seed[28] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_28      ;
    assign w_adr_eq_seed[29] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_29      ;
    assign w_adr_eq_seed[30] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_30      ;
    assign w_adr_eq_seed[31] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_31      ;
    assign w_adr_eq_seed[32] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_32      ;
    assign w_adr_eq_seed[33] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_33      ;
    assign w_adr_eq_seed[34] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_34      ;
    assign w_adr_eq_seed[35] = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_SEED_35      ;
    wire   w_adr_eq_dmacr    = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMACR       ;
    wire   w_adr_eq_dmasr    = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMASR       ;
    wire   w_adr_eq_dmatoth  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMATOTH     ;
    wire   w_adr_eq_dmasadrl = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMASADRL    ;
    wire   w_adr_eq_dmasadrh = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMASADRH    ;
    wire   w_adr_eq_dmadadrl = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMADADRL    ;
    wire   w_adr_eq_dmadadrh = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMADADRH    ;
    wire   w_adr_eq_dma_rlen = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMARLEN     ;
    wire   w_adr_eq_dma_wlen = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMAWLEN     ;
    wire   w_adr_eq_dmaawcc  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMAAWCC     ;
    wire   w_adr_eq_dmaarcc  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMAARCC     ;
    wire   w_adr_eq_dmaid    = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_A_DMAID       ;
    wire   w_adr_eq_hash_in;
    assign w_wsel[p_HASH_SEL_BASE+ 0]   = w_adr_eq_ctrl     & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 1]   = w_adr_eq_cfg      & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 2]   = w_adr_eq_risr     & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 3]   = w_adr_eq_imcr     & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 4]   = w_adr_eq_msglen0  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 5]   = w_adr_eq_msglen1  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 6]   = w_adr_eq_msglen2  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 7]   = w_adr_eq_msglen3  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 8]   = w_adr_eq_msgcnt0  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+ 9]   = w_adr_eq_msgcnt1  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+10]   = w_adr_eq_msgcnt2  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+11]   = w_adr_eq_msgcnt3  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+12]   = w_adr_eq_keylen   & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+13]   = w_adr_eq_keycnt   & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+14]   = w_adr_eq_mdin_cr  & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+15]   = w_adr_eq_mdin     & i_write_en; 
    assign w_wsel[p_HASH_SEL_BASE+16]   = w_adr_eq_hashinlb & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 0]   = w_adr_eq_dmacr    & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 1]   = w_adr_eq_dmasr    & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 2]   = w_adr_eq_dmatoth  & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 3]   = w_adr_eq_dmasadrl & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 4]   = w_adr_eq_dmasadrh & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 5]   = w_adr_eq_dmadadrl & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 6]   = w_adr_eq_dmadadrh & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 7]   = w_adr_eq_dma_rlen & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 8]   = w_adr_eq_dma_wlen & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE + 9]   = w_adr_eq_dmaawcc  & i_write_en; 
    assign w_wsel[p_DMA_SEL_BASE +10]   = w_adr_eq_dmaarcc  & i_write_en; 
    generate                                                              
        for (i=0;i<36;i=i+1) begin:W_SEL_SEED
            assign w_wsel[p_SEED_SEL_BASE+i] = w_adr_eq_seed[i] & i_write_en;
        end
    endgenerate    
    assign w_enable_in = 
            w_wsel[p_HASH_SEL_BASE+0] && i_wstrobe[0] && i_wdata[0] ? 1'b1 :
            i_clr_enable                                            ? 1'b0 :
                                                                      r_enable;
    assign w_suspend_in = 
            w_wsel[p_HASH_SEL_BASE+0] && i_wstrobe[0] && i_wdata[1] ? 1'b1 :
            i_clr_suspend                                           ? 1'b0 :
                                                                      r_suspend;
    assign w_msel_in =    
            w_wsel[p_HASH_SEL_BASE+ 1] && i_wstrobe[0] ? i_wdata[3:0]  : r_msel;
    assign w_hmacen_in =
            w_wsel[p_HASH_SEL_BASE+ 1] && i_wstrobe[0] ? i_wdata[4]    : r_hmacen;
    assign w_spen = 
            w_wsel[p_HASH_SEL_BASE+ 1] && i_wstrobe[0] ? i_wdata[5]    : r_spen;
    assign w_data_type_in = 
            w_wsel[p_HASH_SEL_BASE+ 1] && i_wstrobe[1] ? i_wdata[9:8]  : r_data_type;
    assign w_up_cfg_in =
            w_wsel[p_HASH_SEL_BASE+ 1] && i_wstrobe[1] ? i_wdata[12]   : r_up_cfg;
    assign w_dmaen_in =
            w_wsel[p_HASH_SEL_BASE+ 1] && i_wstrobe[2] ? i_wdata[16]   : r_dmaen;
    assign w_core_done_clr = 
            w_wsel[p_HASH_SEL_BASE+ 2] && i_wstrobe[0] && (~i_wdata[0]);
    assign w_dma_done_clr  = 
            w_wsel[p_HASH_SEL_BASE+ 2] && i_wstrobe[0] && (~i_wdata[1]);
    assign w_suspend_done_clr = 
            w_wsel[p_HASH_SEL_BASE+ 2] && i_wstrobe[0] && (~i_wdata[2]);
    assign w_core_irqen_in = 
            w_wsel[p_HASH_SEL_BASE+ 3] && i_wstrobe[0] ? i_wdata[0]   : 
                                                        r_core_irqen;
    assign w_dma_irqen_in  = 
            w_wsel[p_HASH_SEL_BASE+ 3] && i_wstrobe[0] ? i_wdata[1]   : 
                                                        r_dma_irqen;
    assign w_suspend_irqen_in  = 
            w_wsel[p_HASH_SEL_BASE+ 3] && i_wstrobe[0] ? i_wdata[2]   : 
                                                        r_suspend_irqen;
    assign w_core_mi = r_core_irqen && i_core_ris;
    assign w_dma_mi  = r_dma_irqen && i_dma_ris;
    assign w_suspend_mi  = r_suspend_irqen && i_suspend_ris;
    generate
    for(i=0;i<4;i=i+1) begin : gMSGLEN
        assign w_msg_len_in[i][ 7: 0] = 
                w_wsel[p_HASH_SEL_BASE+4+i] & i_wstrobe[0] ? i_wdata[ 7: 0] : 
                                                             r_msg_len[i][ 7: 0];
        assign w_msg_len_in[i][15: 8] = 
                w_wsel[p_HASH_SEL_BASE+4+i] & i_wstrobe[1] ? i_wdata[15: 8] : 
                                                             r_msg_len[i][15: 8];
        assign w_msg_len_in[i][23:16] = 
                w_wsel[p_HASH_SEL_BASE+4+i] & i_wstrobe[2] ? i_wdata[23:16] : 
                                                             r_msg_len[i][23:16];
        assign w_msg_len_in[i][31:24] = 
                w_wsel[p_HASH_SEL_BASE+4+i] & i_wstrobe[3] ? i_wdata[31:24] : 
                                                             r_msg_len[i][31:24];
    end
    endgenerate
    generate
        for (i=0;i<4;i=i+1) begin : gMSGCNT
        assign w_msg_cnt_in[i][ 7: 0] = 
                w_wsel[p_HASH_SEL_BASE+8+i] & i_wstrobe[0] ? i_wdata[ 7: 0] : 
                                                             r_msg_cnt[i][ 7: 0];
        assign w_msg_cnt_in[i][15: 8] = 
                w_wsel[p_HASH_SEL_BASE+8+i] & i_wstrobe[1] ? i_wdata[15: 8] : 
                                                             r_msg_cnt[i][15: 8];
        assign w_msg_cnt_in[i][23:16] = 
                w_wsel[p_HASH_SEL_BASE+8+i] & i_wstrobe[2] ? i_wdata[23:16] : 
                                                             r_msg_cnt[i][23:16];
        assign w_msg_cnt_in[i][31:24] = 
                w_wsel[p_HASH_SEL_BASE+8+i] & i_wstrobe[3] ? i_wdata[31:24] : 
                                                             r_msg_cnt[i][31:24];
        assign w_msg_cnt_wren_in[i] = p_CDC_EN ? w_wsel[8+i] && !i_core_ready : w_wsel[8+i];
        end
    endgenerate
    assign w_msg_cnt_wren_o = |r_msg_cnt_wren;
    assign w_key_len_in[ 7: 0] = 
            w_wsel[p_HASH_SEL_BASE+12] & i_wstrobe[0] ? i_wdata[ 7: 0] : 
                                                        r_key_len[ 7: 0];
    assign w_key_len_in[15: 8] = 
            w_wsel[p_HASH_SEL_BASE+12] & i_wstrobe[1] ? i_wdata[15: 8] : 
                                                        r_key_len[15: 8];
    assign w_key_len_in[23:16] = 
            w_wsel[p_HASH_SEL_BASE+12] & i_wstrobe[2] ? i_wdata[23:16] : 
                                                        r_key_len[23:16];
    assign w_key_len_in[31:24] = 
            w_wsel[p_HASH_SEL_BASE+12] & i_wstrobe[3] ? i_wdata[31:24] : 
                                                        r_key_len[31:24];
    assign w_key_cnt_in[ 7: 0] = 
            w_wsel[p_HASH_SEL_BASE+13] & i_wstrobe[0] ? i_wdata[ 7: 0] : 
                                                        r_key_cnt[ 7: 0];
    assign w_key_cnt_in[15: 8] = 
            w_wsel[p_HASH_SEL_BASE+13] & i_wstrobe[1] ? i_wdata[15: 8] : 
                                                        r_key_cnt[15: 8];
    assign w_key_cnt_in[23:16] = 
            w_wsel[p_HASH_SEL_BASE+13] & i_wstrobe[2] ? i_wdata[23:16] : 
                                                        r_key_cnt[23:16];
    assign w_key_cnt_in[31:24] = 
            w_wsel[p_HASH_SEL_BASE+13] & i_wstrobe[3] ? i_wdata[31:24] : 
                                                        r_key_cnt[31:24];
    assign w_key_cnt_wren_in = p_CDC_EN ? w_wsel[13] && !i_core_ready : w_wsel[13];
    assign w_hmac_key_in =
            w_wsel[p_HASH_SEL_BASE+14] && i_wstrobe[0] ? i_wdata[0] : 
                                                         r_hmac_key;
    assign w_last_in =
            w_wsel[p_HASH_SEL_BASE+14] && i_wstrobe[2] ? i_wdata[16] : 
                                                         r_last;
    assign w_mdin_wren = p_CDC_EN ? w_wsel[p_HASH_SEL_BASE+15] &&!i_core_ready :
                         w_wsel[p_HASH_SEL_BASE+15];
    assign w_core_ready = !p_CDC_EN ? (w_adr_eq_mdin ? i_core_ready : 1'b1 ):
                         (w_adr_eq_mdin ||
                          w_adr_eq_hash_in ||
                          w_adr_eq_keycnt ||
                          w_adr_eq_msgcnt0 ||
                          w_adr_eq_msgcnt1 || 
                          w_adr_eq_msgcnt2 || 
                          w_adr_eq_msgcnt3) && i_write_en ||
                          w_core_done_clr || 
                          w_suspend_done_clr ||
                          w_dma_done_clr ? i_core_ready :
                          1'b1;
    assign w_adr_eq_hash_in = (({i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} >= p_A_HASHINLB) && 
                ({i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} <= p_A_HASHINUB));
    assign w_hash_in_wren = p_CDC_EN ? i_write_en && w_adr_eq_hash_in &&!i_core_ready :
                            i_write_en && w_adr_eq_hash_in;
    assign w_hash_in_addr = i_addr[7:2];
    assign w_hout_rden = 
                ({i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} >= p_A_HASHOUTLB) && 
                ({i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} <= p_A_HASHOUTUB);
    assign w_hash_out = i_hash_out >> (i_addr[7:2]*32);
    assign w_hout_rdata = w_hash_out[31:0];
hash_hp_dswp
    #(
      .p_CH1_DWIDTH       (32                     )
   ,  .p_CH2_DWIDTH       (32                     )
    )
    u_hash_hp_dswp
    (
      .i_data_type        (r_data_type            )
   ,  .i_ch1              (i_wdata                )
   ,  .o_ch1              (w_wdata_swp_t          )
   ,  .i_ch2              (w_hout_rdata           )
   ,  .o_ch2              (w_hout_rdata_swp_t     )
   ,  .i_ch3              (256'd0                 )
   ,  .o_ch3              (                       )
   ,  .i_ch4              (256'd0                 )
   ,  .o_ch4              (                       )
   ,  .i_ch5              (256'd0                 )
   ,  .o_ch5              (                       )
    );
    assign w_wdata_swp = {
        w_wdata_swp_t[(8*0)+:8],
        w_wdata_swp_t[(8*1)+:8],
        w_wdata_swp_t[(8*2)+:8],
        w_wdata_swp_t[(8*3)+:8] 
    };
    assign w_hout_rdata_swp = {
        w_hout_rdata_swp_t[(8*0)+:8],
        w_hout_rdata_swp_t[(8*1)+:8],
        w_hout_rdata_swp_t[(8*2)+:8],
        w_hout_rdata_swp_t[(8*3)+:8] 
    };
    assign w_saddr_l_pre[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+3] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_saddr_l[ 7: 0];
    assign w_saddr_l_pre[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+3] & i_wstrobe[1] ? i_wdata[15: 8] : r_saddr_l[15: 8];
    assign w_saddr_l_pre[23:16] = 
            w_wsel[p_DMA_SEL_BASE+3] & i_wstrobe[2] ? i_wdata[23:16] : r_saddr_l[23:16];
    assign w_saddr_l_pre[31:24] = 
            w_wsel[p_DMA_SEL_BASE+3] & i_wstrobe[3] ? i_wdata[31:24] : r_saddr_l[31:24];
    assign w_saddr_h_pre[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+4] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_saddr_h[ 7: 0];
    assign w_saddr_h_pre[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+4] & i_wstrobe[1] ? i_wdata[15: 8] : r_saddr_h[15: 8];
    assign w_saddr_h_pre[23:16] = 
            w_wsel[p_DMA_SEL_BASE+4] & i_wstrobe[2] ? i_wdata[23:16] : r_saddr_h[23:16];
    assign w_saddr_h_pre[31:24] = 
            w_wsel[p_DMA_SEL_BASE+4] & i_wstrobe[3] ? i_wdata[31:24] : r_saddr_h[31:24];
    assign w_daddr_l_pre[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+5] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_daddr_l[ 7: 0];
    assign w_daddr_l_pre[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+5] & i_wstrobe[1] ? i_wdata[15: 8] : r_daddr_l[15: 8];
    assign w_daddr_l_pre[23:16] = 
            w_wsel[p_DMA_SEL_BASE+5] & i_wstrobe[2] ? i_wdata[23:16] : r_daddr_l[23:16];
    assign w_daddr_l_pre[31:24] = 
            w_wsel[p_DMA_SEL_BASE+5] & i_wstrobe[3] ? i_wdata[31:24] : r_daddr_l[31:24];
    assign w_daddr_h_pre[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+6] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_daddr_h[ 7: 0];
    assign w_daddr_h_pre[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+6] & i_wstrobe[1] ? i_wdata[15: 8] : r_daddr_h[15: 8];
    assign w_daddr_h_pre[23:16] = 
            w_wsel[p_DMA_SEL_BASE+6] & i_wstrobe[2] ? i_wdata[23:16] : r_daddr_h[23:16];
    assign w_daddr_h_pre[31:24] = 
            w_wsel[p_DMA_SEL_BASE+6] & i_wstrobe[3] ? i_wdata[31:24] : r_daddr_h[31:24];
    assign w_rlen_pre[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+7] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_rlen[ 7: 0];
    assign w_rlen_pre[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+7] & i_wstrobe[1] ? i_wdata[15: 8] : r_rlen[15: 8];
    assign w_rlen_pre[23:16] = 
            w_wsel[p_DMA_SEL_BASE+7] & i_wstrobe[2] ? i_wdata[23:16] : r_rlen[23:16];
    assign w_rlen_pre[31:24] = 
            w_wsel[p_DMA_SEL_BASE+7] & i_wstrobe[3] ? i_wdata[31:24] : r_rlen[31:24];
    assign w_wlen_pre[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+8] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_wlen[ 7: 0];
    assign w_wlen_pre[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+8] & i_wstrobe[1] ? i_wdata[15: 8] : r_wlen[15: 8];
    assign w_wlen_pre[23:16] = 
            w_wsel[p_DMA_SEL_BASE+8] & i_wstrobe[2] ? i_wdata[23:16] : r_wlen[23:16];
    assign w_wlen_pre[31:24] = 
            w_wsel[p_DMA_SEL_BASE+8] & i_wstrobe[3] ? i_wdata[31:24] : r_wlen[31:24];
    assign w_awbus[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+9] & i_wstrobe[0] ? i_wdata[ 7: 0] : r_awbus[ 7: 0];
    assign w_awbus[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+9] & i_wstrobe[1] ? i_wdata[15: 8] : r_awbus[15: 8];
    assign w_awbus[23:16] = 
            w_wsel[p_DMA_SEL_BASE+9] & i_wstrobe[2] ? i_wdata[23:16] : r_awbus[23:16];
    assign w_awbus[31:24] = 
            w_wsel[p_DMA_SEL_BASE+9] & i_wstrobe[3] ? i_wdata[31:24] : r_awbus[31:24];
    assign w_arbus[ 7: 0] = 
            w_wsel[p_DMA_SEL_BASE+10]& i_wstrobe[0] ? i_wdata[ 7: 0] : r_arbus[ 7: 0];
    assign w_arbus[15: 8] = 
            w_wsel[p_DMA_SEL_BASE+10]& i_wstrobe[1] ? i_wdata[15: 8] : r_arbus[15: 8];
    assign w_arbus[23:16] = 
            w_wsel[p_DMA_SEL_BASE+10]& i_wstrobe[2] ? i_wdata[23:16] : r_arbus[23:16];
    assign w_arbus[31:24] = 
            w_wsel[p_DMA_SEL_BASE+10]& i_wstrobe[3] ? i_wdata[31:24] : r_arbus[31:24];
    assign w_awid = 4'b0;
    assign w_arid = 4'b0;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            r_enable        <= 1'd0;
            r_suspend       <= 1'd0;
            r_msel          <= 4'd0;
            r_hmacen        <= 1'b0;
            r_spen          <= 1'b0;
            r_data_type     <= 2'd0;
            r_up_cfg        <= 1'd0;
            r_dmaen         <= 1'b0;
            r_core_irqen    <= 1'd0;
            r_dma_irqen     <= 1'd0;
            r_suspend_irqen <= 1'd0;
            r_key_len       <= 32'd0;
            r_key_cnt       <= 32'd0;
            r_hmac_key      <= 1'b0;
            r_last          <= 1'b0;
            r_msg_cnt_wren  <= 4'h0;
            r_key_cnt_wren  <= 1'b0;
            r_arbus         <= 32'b0;
            r_awbus         <= 32'b0;
            r_arid          <= 4'b0;
            r_awid          <= 4'b0;
        end
        else begin
            r_enable        <=  w_enable_in;
            r_suspend       <=  w_suspend_in;
            r_msel          <=  w_msel_in;
            r_hmacen        <=  w_hmacen_in;
            r_spen          <=  w_spen;
            r_data_type     <=  w_data_type_in;
            r_up_cfg        <=  w_up_cfg_in;
            r_dmaen         <=  w_dmaen_in;
            r_core_irqen    <=  w_core_irqen_in;
            r_dma_irqen     <=  w_dma_irqen_in;
            r_suspend_irqen <=  w_suspend_irqen_in;
            r_key_len       <=  w_key_len_in;
            r_key_cnt       <=  w_key_cnt_in;
            r_hmac_key      <=  w_hmac_key_in;
            r_last          <=  w_last_in;
            r_msg_cnt_wren  <=  w_msg_cnt_wren_in;
            r_key_cnt_wren  <=  w_key_cnt_wren_in;
            r_arbus         <=  w_arbus;
            r_awbus         <=  w_awbus;
            r_arid          <=  w_arid;
            r_awid          <=  w_awid;
        end
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            for(j=0;j<4;j=j+1) begin
                r_msg_len[j]     <= 32'd0;
                r_msg_cnt[j]     <= 32'd0;
            end
        end
        else begin
            for(j=0;j<4;j=j+1) begin
                r_msg_len[j]     <= w_msg_len_in[j];
                r_msg_cnt[j]     <= w_msg_cnt_in[j];
            end
        end
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            r_mbl       <= 3'd7;
            r_motdl     <= 2'd1;
            r_to        <= 1'd0;
            r_to_thrd   <= 16'd0;
            r_saddr_l   <= 32'd0;
            r_saddr_h   <= 32'd0;
            r_daddr_l   <= 32'd0;
            r_daddr_h   <= 32'd0;
            r_rlen      <= 32'd0;
            r_wlen      <= 32'd0;
        end
        else begin
            r_mbl       <=  3'b0; 
            r_motdl     <=  2'b0;
            r_to        <=  1'b0;
            r_to_thrd   <=  16'b0;
            r_saddr_l   <=  w_saddr_l_pre;
            r_saddr_h   <=  w_saddr_h_pre;
            r_daddr_l   <=  w_daddr_l_pre;
            r_daddr_h   <=  w_daddr_h_pre;
            r_rlen      <=  w_rlen_pre;
            r_wlen      <=  w_wlen_pre;
        end
    end
    always@(*)
    begin
        o_rdata = {p_AHB_DATA_WIDTH{1'b0}};
        case(1'b1)
            w_adr_eq_ctrl       :   o_rdata = {30'd0,r_suspend,r_enable};
            w_adr_eq_cfg        :   o_rdata = {15'd0,r_dmaen,
                                               3'd0,r_up_cfg,2'd0,r_data_type,
                                               2'd0,r_spen,r_hmacen,r_msel};
            w_adr_eq_sus_sr     :   o_rdata = {31'd0,i_suspend_sr};
            w_adr_eq_risr       :   o_rdata = {29'd0,i_suspend_ris,i_dma_ris,i_core_ris};
            w_adr_eq_imcr       :   o_rdata = {29'd0,r_suspend_irqen,r_dma_irqen,r_core_irqen};
            w_adr_eq_misr       :   o_rdata = {29'd0,w_suspend_mi,w_dma_mi,w_core_mi};
            w_adr_eq_sus_pad_sr :   o_rdata = {31'd0,i_blk_pad_fail};
            w_adr_eq_msglen0    :   o_rdata = r_msg_len[0];
            w_adr_eq_msglen1    :   o_rdata = r_msg_len[1];
            w_adr_eq_msglen2    :   o_rdata = r_msg_len[2];
            w_adr_eq_msglen3    :   o_rdata = r_msg_len[3];
            w_adr_eq_msgcnt0    :   o_rdata = r_msg_cnt[0];
            w_adr_eq_msgcnt1    :   o_rdata = r_msg_cnt[1];
            w_adr_eq_msgcnt2    :   o_rdata = r_msg_cnt[2];
            w_adr_eq_msgcnt3    :   o_rdata = r_msg_cnt[3];
            w_adr_eq_keylen     :   o_rdata = r_key_len;
            w_adr_eq_keycnt     :   o_rdata = r_key_cnt;
            w_adr_eq_pro_msgcnt0:   o_rdata = i_pro_msg_cnt[31:0];
            w_adr_eq_pro_msgcnt1:   o_rdata = i_pro_msg_cnt[63:32];
            w_adr_eq_pro_msgcnt2:   o_rdata = i_pro_msg_cnt[95:64];
            w_adr_eq_pro_msgcnt3:   o_rdata = i_pro_msg_cnt[127:96];
            w_adr_eq_mdin_cr    :   o_rdata = {15'd0,r_last,
                                               15'd0,r_hmac_key};
            w_adr_eq_version    :   o_rdata = p_VERSION;
            w_hout_rden         :   o_rdata = w_hout_rdata_swp;
            w_adr_eq_dmacr      :   o_rdata = {22'd0,r_motdl,5'h0,r_mbl};
            w_adr_eq_dmasr      :   o_rdata = {31'd0,r_to};
            w_adr_eq_dmatoth    :   o_rdata = {16'd0,r_to_thrd};
            w_adr_eq_dmasadrl   :   o_rdata = r_saddr_l;
            w_adr_eq_dmasadrh   :   o_rdata = r_saddr_h;
            w_adr_eq_dmadadrl   :   o_rdata = r_daddr_l;
            w_adr_eq_dmadadrh   :   o_rdata = r_daddr_h;
            w_adr_eq_dma_rlen   :   o_rdata = r_rlen;
            w_adr_eq_dma_wlen   :   o_rdata = r_wlen;
            w_adr_eq_dmaawcc    :   o_rdata = r_awbus; 
            w_adr_eq_dmaarcc    :   o_rdata = r_arbus;
            w_adr_eq_dmaid      :   o_rdata = {24'b0,r_arid,r_awid};
            default             :   o_rdata = {p_AHB_DATA_WIDTH{1'b0}};
        endcase
    end
    assign o_readyout       = w_core_ready          ;
    assign o_enable         = r_enable              ;
    assign o_suspend        = r_suspend             ;
    assign o_msel           = r_msel                ;
    assign o_hmacen         = r_hmacen              ;
    assign o_spen           = r_spen                ;
    assign o_data_type      = r_data_type           ;
    assign o_up_cfg         = r_up_cfg              ;
    assign o_dmaen          = r_dmaen               ;
    assign o_core_done_clr  = w_core_done_clr       ;
    assign o_dma_done_clr   = w_dma_done_clr        ;
    assign o_suspend_done_clr  = w_suspend_done_clr ;
    assign o_core_irqen     = r_core_irqen          ;
    assign o_dma_irqen      = r_dma_irqen           ;
    assign o_suspend_irqen  = r_suspend_irqen       ;
    generate
    for(i=0;i<4;i=i+1) begin : gOMSGLEN
        assign o_msg_len[i*32+:32] = r_msg_len[i]   ;
    end
    endgenerate
    generate
    for(i=0;i<4;i=i+1) begin : gOMSGCNT
        assign o_msg_cnt[i*32+:32] = r_msg_cnt[i]   ;
    end
    endgenerate
    generate
    for(i=0;i<4;i=i+1) begin : CNTcompare
        assign w_msg_len_cnt_equal[i] = (r_msg_cnt[i] == r_msg_len[i]) ? 1'b1 : 1'b0;
    end
    endgenerate
    assign o_msg_cnt_wren   = w_msg_cnt_wren_o;
    assign o_key_len        = r_key_len             ;
    assign o_key_cnt        = r_key_cnt             ;
    assign o_key_cnt_wren   = r_key_cnt_wren        ;
    assign o_hmac_key       = r_hmac_key            ;
    assign o_last           = r_last                ;
    assign o_mdin_wren      = w_mdin_wren           ;
    assign o_mdin           = w_wdata_swp           ;
    assign o_hash_in_wren   = w_hash_in_wren        ;
    assign o_hash_in        = w_wdata_swp           ;
    assign o_hash_in_addr   = w_hash_in_addr        ;
    assign o_mbl            = r_mbl                 ;
    assign o_motdl          = r_motdl               ;
    assign o_to_threshold   = r_to_thrd             ;
    assign o_saddr          = {r_saddr_h,
                               r_saddr_l}           ;
    assign o_daddr          = {r_daddr_h,
                               r_daddr_l}           ;
    assign o_rlen           = &w_msg_len_cnt_equal ? 32'b0 : r_rlen ;
    assign o_wlen           = r_wlen                ;
    assign o_arbus          = r_arbus;
    assign o_awbus          = r_awbus;
    assign o_arid           = r_arid;
    assign o_awid           = r_awid;
endmodule
module hash_hp_dswp_unit
#(
    parameter   p_D_WIDTH               =  32 
)
(
    input   wire    [1:0]               i_data_type
   ,input   wire    [p_D_WIDTH-1:0]     i_din 
   ,output  reg     [p_D_WIDTH-1:0]     o_dout
);
    localparam  p_NUMOFWORD     = p_D_WIDTH/32;
    localparam  p_NUMOFHALFWORD = p_D_WIDTH/16;
    localparam  p_NUMOFBYTE     = p_D_WIDTH/8 ;
    localparam  p_NUMOFBIT      = p_D_WIDTH/1 ;
    integer i;
    wire    w_no_swp    ;
    wire    w_hw_swp    ;
    wire    w_byte_swp  ;
    wire    w_bit_swp   ;
    assign w_no_swp     = i_data_type== 2'd0;
    assign w_hw_swp     = i_data_type== 2'd1;
    assign w_byte_swp   = i_data_type== 2'd2;
    assign w_bit_swp    = i_data_type== 2'd3;
    always@*
    begin
        if(w_no_swp) begin
            o_dout = i_din;
        end
        else if (w_hw_swp) begin
            for (i=0;i<p_NUMOFHALFWORD;i=i+1) begin 
                o_dout[i*16+:16] = i_din[(p_NUMOFHALFWORD-1-i)*16+:16];
            end
        end
        else if (w_byte_swp) begin
            for (i=0;i<p_NUMOFBYTE;i=i+1) begin 
                o_dout[i*8+:8] = i_din[(p_NUMOFBYTE-1-i)*8+:8];
            end
        end
        else if (w_bit_swp) begin
            for (i=0;i<p_NUMOFBIT;i=i+1) begin 
                o_dout[i*1+:1] = i_din[(p_NUMOFBIT-1-i)*1+:1];
            end
        end
        else begin
            o_dout = i_din;
        end
    end
endmodule
module hash_hp_dswp
#(
    parameter   p_CH1_DWIDTH            =   256
   ,parameter   p_CH2_DWIDTH            =   256
   ,parameter   p_CH3_DWIDTH            =   256
   ,parameter   p_CH4_DWIDTH            =   256
   ,parameter   p_CH5_DWIDTH            =   256
)
(
    input   wire    [1:0]               i_data_type
   ,input   wire    [p_CH1_DWIDTH-1:0]  i_ch1 
   ,output  wire    [p_CH1_DWIDTH-1:0]  o_ch1 
   ,input   wire    [p_CH2_DWIDTH-1:0]  i_ch2 
   ,output  wire    [p_CH2_DWIDTH-1:0]  o_ch2 
   ,input   wire    [p_CH3_DWIDTH-1:0]  i_ch3 
   ,output  wire    [p_CH3_DWIDTH-1:0]  o_ch3 
   ,input   wire    [p_CH4_DWIDTH-1:0]  i_ch4 
   ,output  wire    [p_CH4_DWIDTH-1:0]  o_ch4 
   ,input   wire    [p_CH5_DWIDTH-1:0]  i_ch5 
   ,output  wire    [p_CH5_DWIDTH-1:0]  o_ch5 
);
    localparam  p_CH1NUMOFWORD  = p_CH1_DWIDTH/32;
    localparam  p_CH2NUMOFWORD  = p_CH2_DWIDTH/32;
    localparam  p_CH3NUMOFWORD  = p_CH3_DWIDTH/32;
    localparam  p_CH4NUMOFWORD  = p_CH4_DWIDTH/32;
    localparam  p_CH5NUMOFWORD  = p_CH5_DWIDTH/32;
    genvar i;
    generate
        for(i=0;i<p_CH1NUMOFWORD;i=i+1) begin : gDSWPCH1
hash_hp_dswp_unit
        #(
          .p_D_WIDTH  (32                 )
        )
        u_hash_hp_dswp_unit_ch1
        (
          .i_data_type(i_data_type        )
       ,  .i_din      (i_ch1[i*32+:32]    )
       ,  .o_dout     (o_ch1[i*32+:32]    )
        );
        end
    endgenerate
    generate
        for(i=0;i<p_CH2NUMOFWORD;i=i+1) begin : gDSWPCH2
hash_hp_dswp_unit
        #(
          .p_D_WIDTH  (32                 )
        )
        u_hash_hp_dswp_unit_ch2
        (
          .i_data_type(i_data_type        )
       ,  .i_din      (i_ch2[i*32+:32]    )
       ,  .o_dout     (o_ch2[i*32+:32]    )
        );
        end
    endgenerate
    generate
        for(i=0;i<p_CH3NUMOFWORD;i=i+1) begin : gDSWPCH3
hash_hp_dswp_unit
        #(
          .p_D_WIDTH  (32                 )
        )
        u_hash_hp_dswp_unit_ch3
        (
          .i_data_type(i_data_type        )
       ,  .i_din      (i_ch3[i*32+:32]    )
       ,  .o_dout     (o_ch3[i*32+:32]    )
        );
        end
    endgenerate
    generate
        for(i=0;i<p_CH4NUMOFWORD;i=i+1) begin : gDSWPCH4
hash_hp_dswp_unit
        #(
          .p_D_WIDTH  (32                 )
        )
        u_hash_hp_dswp_unit_ch4
        (
          .i_data_type(i_data_type        )
       ,  .i_din      (i_ch4[i*32+:32]    )
       ,  .o_dout     (o_ch4[i*32+:32]    )
        );
        end
    endgenerate
    generate
        for(i=0;i<p_CH5NUMOFWORD;i=i+1) begin : gDSWPCH5
hash_hp_dswp_unit
        #(
          .p_D_WIDTH  (32                 )
        )
        u_hash_hp_dswp_unit_ch5
        (
          .i_data_type(i_data_type        )
       ,  .i_din      (i_ch5[i*32+:32]    )
       ,  .o_dout     (o_ch5[i*32+:32]    )
        );
        end
    endgenerate
endmodule
module hash_hp_A000Yc (
    input    wire  [5:0]          A001Yc          ,
    input    wire  [511:0]        A002Yc          ,
    input    wire  [127:0]        A003Yc          ,
    output   wire  [127:0]        A004Yc          
);
    wire  [31:0]     A005Yc[0:15];
    wire  [31:0]     A006Yc[0:3] ;
    wire  [31:0]     A007Yc[0:3] ;
    wire             A008Yc      ;
    wire             A009Yc      ;
    wire             A010Yc      ;
    wire  [31:0]     A011Yc      ;
    wire  [31:0]     A012Yc      ;
    wire  [31:0]     A013Yc         ;
    genvar           A014Yc,A015Yc         ;
    generate
    for (A014Yc=0;A014Yc<16;A014Yc=A014Yc+1) begin : A016Yc    
       for (A015Yc=0;A015Yc<4;A015Yc=A015Yc+1) begin : A017Yc    
          assign A005Yc[A014Yc][A015Yc*8 +: 8] = A002Yc[((3-A015Yc)*8+(15-A014Yc)*32) +: 8];
       end
    end
    endgenerate
    generate
    for (A014Yc=0;A014Yc<4;A014Yc=A014Yc+1) begin : A018Yc      
       for (A015Yc=0;A015Yc<4;A015Yc=A015Yc+1) begin : A019Yc     
          assign A006Yc[A014Yc][A015Yc*8 +: 8] = A003Yc[((3-A015Yc)*8+(3-A014Yc)*32) +: 8];
       end
    end
    endgenerate
    assign A008Yc      = A001Yc[5:4] == 2'b00 ;
    assign A009Yc      = A001Yc[5:4] == 2'b01 ;
    assign A010Yc      = A001Yc[5:4] == 2'b10 ;
    assign A011Yc      = 
            A008Yc      ? A020Yc        (A006Yc[1],A006Yc[2],A006Yc[3]) : 
            A009Yc      ? A021Yc        (A006Yc[1],A006Yc[2],A006Yc[3]) : 
            A010Yc      ? A022Yc        (A006Yc[1],A006Yc[2],A006Yc[3]) : 
                          A023Yc        (A006Yc[1],A006Yc[2],A006Yc[3]) ;
hash_hp_A024Yc      
    A025Yc       
    (
    .A026Yc(A012Yc       ),
    .A027Yc(A013Yc          ),
    .A028Yc(A001Yc[5:0]   )
    );
    assign A013Yc         = ((A006Yc[0] + A029Yc  (A001Yc[5:0])) 
                       + A005Yc[A030Yc    (A001Yc[5:4], A001Yc[3:0])])
                       + A011Yc   ;
    assign A007Yc[0] = A006Yc[3];
    assign A007Yc[3] = A006Yc[2];
    assign A007Yc[2] = A006Yc[1];
    assign A007Yc[1] = A006Yc[1] + A012Yc;
    generate
        for (A014Yc=0;A014Yc<4;A014Yc=A014Yc+1) begin : A031Yc      
            for (A015Yc=0;A015Yc<4;A015Yc=A015Yc+1) begin : A032Yc     
                assign A004Yc[((3-A014Yc)*32 + (3-A015Yc)*8)+:8] = A007Yc[A014Yc][A015Yc*8+:8];
            end
        end
    endgenerate
    function    [31:0] A020Yc        ;
        input   [31:0]      A033Yc     ;
        input   [31:0]      A034Yc     ;
        input   [31:0]      A035Yc     ;
    begin
        A020Yc          =   (A033Yc & A034Yc) | (~A033Yc & A035Yc);
    end
    endfunction
    function    [31:0] A021Yc        ;
        input   [31:0]      A033Yc     ;
        input   [31:0]      A034Yc     ;
        input   [31:0]      A035Yc     ;
    begin
        A021Yc          =   (A035Yc & A033Yc) | (~A035Yc & A034Yc);
    end
    endfunction
    function    [31:0] A022Yc        ;
        input   [31:0]      A033Yc     ;
        input   [31:0]      A034Yc     ;
        input   [31:0]      A035Yc     ;
    begin
        A022Yc          =   A033Yc ^ A034Yc ^ A035Yc;
    end
    endfunction
    function    [31:0] A023Yc        ;
        input   [31:0]      A033Yc     ;
        input   [31:0]      A034Yc     ;
        input   [31:0]      A035Yc     ;
    begin
        A023Yc          =   A034Yc ^ (A033Yc | ~A035Yc);
    end
    endfunction
    function    [3:0]   A030Yc      ;
        input   [1:0]   A036Yc      ;
        input   [3:0]   A037Yc       ;
    begin
        A030Yc          =   A036Yc    == 2'b00 ?                    A037Yc :  
                            A036Yc    == 2'b01 ? ((A037Yc<<2) + A037Yc + 1) :  
                            A036Yc    == 2'b10 ? ((A037Yc<<1) + A037Yc + 5) :  
                                                 ((A037Yc<<3) - A037Yc)     ; 
    end
    endfunction
    function    [31:0]  A029Yc      ;
        input   [5:0]   A037Yc       ;
        reg     [31:0]  A038Yc[0:63]   ;
    begin
        {A038Yc[0] , A038Yc[1] , A038Yc[2] , A038Yc[3] , A038Yc[4] , A038Yc[5] , A038Yc[6] , A038Yc[7] , 
         A038Yc[8] , A038Yc[9] , A038Yc[10], A038Yc[11], A038Yc[12], A038Yc[13], A038Yc[14], A038Yc[15],
         A038Yc[16], A038Yc[17], A038Yc[18], A038Yc[19], A038Yc[20], A038Yc[21], A038Yc[22], A038Yc[23], 
         A038Yc[24], A038Yc[25], A038Yc[26], A038Yc[27], A038Yc[28], A038Yc[29], A038Yc[30], A038Yc[31], 
         A038Yc[32], A038Yc[33], A038Yc[34], A038Yc[35], A038Yc[36], A038Yc[37], A038Yc[38], A038Yc[39], 
         A038Yc[40], A038Yc[41], A038Yc[42], A038Yc[43], A038Yc[44], A038Yc[45], A038Yc[46], A038Yc[47], 
         A038Yc[48], A038Yc[49], A038Yc[50], A038Yc[51], A038Yc[52], A038Yc[53], A038Yc[54], A038Yc[55], 
         A038Yc[56], A038Yc[57], A038Yc[58], A038Yc[59], A038Yc[60], A038Yc[61], A038Yc[62], A038Yc[63]}
        = {
        32'hd76aa478, 32'he8c7b756, 32'h242070db, 32'hc1bdceee,
        32'hf57c0faf, 32'h4787c62a, 32'ha8304613, 32'hfd469501,
        32'h698098d8, 32'h8b44f7af, 32'hffff5bb1, 32'h895cd7be,
        32'h6b901122, 32'hfd987193, 32'ha679438e, 32'h49b40821,
        32'hf61e2562, 32'hc040b340, 32'h265e5a51, 32'he9b6c7aa,
        32'hd62f105d, 32'h02441453, 32'hd8a1e681, 32'he7d3fbc8,
        32'h21e1cde6, 32'hc33707d6, 32'hf4d50d87, 32'h455a14ed,
        32'ha9e3e905, 32'hfcefa3f8, 32'h676f02d9, 32'h8d2a4c8a,
        32'hfffa3942, 32'h8771f681, 32'h6d9d6122, 32'hfde5380c,
        32'ha4beea44, 32'h4bdecfa9, 32'hf6bb4b60, 32'hbebfbc70,
        32'h289b7ec6, 32'heaa127fa, 32'hd4ef3085, 32'h04881d05,
        32'hd9d4d039, 32'he6db99e5, 32'h1fa27cf8, 32'hc4ac5665,
        32'hf4292244, 32'h432aff97, 32'hab9423a7, 32'hfc93a039,
        32'h655b59c3, 32'h8f0ccc92, 32'hffeff47d, 32'h85845dd1,
        32'h6fa87e4f, 32'hfe2ce6e0, 32'ha3014314, 32'h4e0811a1,
        32'hf7537e82, 32'hbd3af235, 32'h2ad7d2bb, 32'heb86d391};
        A029Yc      =   A038Yc[A037Yc] ;
    end
    endfunction
endmodule
module hash_hp_A024Yc      (
    output wire [31:0]      A026Yc   ,
    input  wire [31:0]      A027Yc   ,
    input  wire [5:0]       A028Yc   
);
    assign  A026Yc   =   A039Yc    (A027Yc, A040Yc(A028Yc));
    function    [4:0]   A040Yc           ;
        input   [5:0]   A037Yc       ;
        reg     [4:0]   A041Yc[0:63]   ;
    begin
        {A041Yc[0] , A041Yc[1] , A041Yc[2] , A041Yc[3] , A041Yc[4] , A041Yc[5] , A041Yc[6] , A041Yc[7] , 
         A041Yc[8] , A041Yc[9] , A041Yc[10], A041Yc[11], A041Yc[12], A041Yc[13], A041Yc[14], A041Yc[15],
         A041Yc[16], A041Yc[17], A041Yc[18], A041Yc[19], A041Yc[20], A041Yc[21], A041Yc[22], A041Yc[23], 
         A041Yc[24], A041Yc[25], A041Yc[26], A041Yc[27], A041Yc[28], A041Yc[29], A041Yc[30], A041Yc[31], 
         A041Yc[32], A041Yc[33], A041Yc[34], A041Yc[35], A041Yc[36], A041Yc[37], A041Yc[38], A041Yc[39], 
         A041Yc[40], A041Yc[41], A041Yc[42], A041Yc[43], A041Yc[44], A041Yc[45], A041Yc[46], A041Yc[47], 
         A041Yc[48], A041Yc[49], A041Yc[50], A041Yc[51], A041Yc[52], A041Yc[53], A041Yc[54], A041Yc[55], 
         A041Yc[56], A041Yc[57], A041Yc[58], A041Yc[59], A041Yc[60], A041Yc[61], A041Yc[62], A041Yc[63]}
        = {
        5'd07, 5'd12, 5'd17, 5'd22, 5'd07, 5'd12, 5'd17, 5'd22,
        5'd07, 5'd12, 5'd17, 5'd22, 5'd07, 5'd12, 5'd17, 5'd22,
        5'd05, 5'd09, 5'd14, 5'd20, 5'd05, 5'd09, 5'd14, 5'd20,
        5'd05, 5'd09, 5'd14, 5'd20, 5'd05, 5'd09, 5'd14, 5'd20,
        5'd04, 5'd11, 5'd16, 5'd23, 5'd04, 5'd11, 5'd16, 5'd23,
        5'd04, 5'd11, 5'd16, 5'd23, 5'd04, 5'd11, 5'd16, 5'd23,
        5'd06, 5'd10, 5'd15, 5'd21, 5'd06, 5'd10, 5'd15, 5'd21,
        5'd06, 5'd10, 5'd15, 5'd21, 5'd06, 5'd10, 5'd15, 5'd21};
        A040Yc   =   A041Yc[A037Yc]  ;
    end
    endfunction
    function    [31:0]  A039Yc      ;
        input   [31:0]      A042Yc     ;
        input   [4:0]       A043Yc     ;
    begin
        A039Yc      =   (A042Yc << A043Yc) | (A042Yc >> (32-A043Yc));
    end
    endfunction
endmodule
module hash_hp_A044Yc     (
    input    wire  [6:0]    A001Yc          ,
    input    wire  [511:0]  A002Yc          ,
    input    wire  [159:0]  A003Yc          ,
    output   wire  [159:0]  A004Yc          ,
    output   wire  [31:0]   A045Yc           ,
    output   wire           A046Yc         
);
    wire  [31:0]     A047Yc         ; 
    wire  [31:0]     A048Yc      ; 
    wire  [31:0]     A049Yc      ; 
    wire  [31:0]     A005Yc[0:15];
    wire  [31:0]     A006Yc[0:4] ;
    wire  [31:0]     A007Yc[0:4] ;
    genvar           A014Yc           ;
    generate
    for(A014Yc=0;A014Yc<16;A014Yc=A014Yc+1) begin : A016Yc    
        assign A005Yc[A014Yc] = A002Yc[(15-A014Yc)*32+:32];
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<5;A014Yc=A014Yc+1) begin : A018Yc      
        assign A006Yc[A014Yc] = A003Yc[(4-A014Yc)*32+:32];
    end
    endgenerate
    assign A048Yc     = A050Yc((A005Yc[13] ^ A005Yc[8] ^ A005Yc[2] ^ A005Yc[0]),6'd1);
    assign A049Yc     = A005Yc[A001Yc];
    assign A007Yc[0] = A047Yc;
    assign A007Yc[1] = A006Yc[0];
    assign A007Yc[2] = A050Yc(A006Yc[1],5'd30);
    assign A007Yc[3] = A006Yc[2];
    assign A007Yc[4] = A006Yc[3];
    assign A047Yc = 
        (A001Yc<'d16) ? A050Yc(A006Yc[0],6'd5) + A051Yc(A006Yc[1],A006Yc[2],A006Yc[3]) +
                       A006Yc[4] + A052Yc(2'd0) + A049Yc     :
        (A001Yc<'d20) ? A050Yc(A006Yc[0],6'd5) + A051Yc(A006Yc[1],A006Yc[2],A006Yc[3]) + 
                       A006Yc[4] + A052Yc(2'd0) + A048Yc     :
        (A001Yc<'d40) ? 
            A050Yc(A006Yc[0],6'd5) + A053Yc(A006Yc[1],A006Yc[2],A006Yc[3]) +
            A006Yc[4] + A052Yc(2'd1) + A048Yc     :
        (A001Yc<'d60) ? A050Yc(A006Yc[0],6'd5) + A054Yc(A006Yc[1],A006Yc[2],A006Yc[3]) + 
                       A006Yc[4] + A052Yc(2'd2) + A048Yc     : 
            A050Yc(A006Yc[0],6'd5) + A053Yc(A006Yc[1],A006Yc[2],A006Yc[3]) + 
            A006Yc[4] + A052Yc(2'd3) + A048Yc    ;
    generate
    for (A014Yc=0;A014Yc<5;A014Yc=A014Yc+1) begin : A031Yc      
        assign A004Yc[(4-A014Yc)*32+:32] = A007Yc[A014Yc];
    end
    endgenerate
    assign A045Yc = A048Yc    ;
    assign A046Yc  = A001Yc >= 'd16;
    function [31:0] A051Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        begin
            A051Yc = (A055Yc & A056Yc) ^ (~A055Yc & A057Yc);
        end
    endfunction
    function [31:0] A054Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        begin
            A054Yc = (A055Yc & A056Yc) ^ (A055Yc & A057Yc) ^ (A056Yc & A057Yc);
        end
    endfunction
    function [31:0] A053Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        begin
            A053Yc = A055Yc ^ A056Yc ^ A057Yc;
        end
    endfunction
    function [31:0] A050Yc;
        input [31:0]   A055Yc;
        input [5:0]    A058Yc;
        begin
            A050Yc = (A055Yc<<A058Yc | A055Yc>>(32-A058Yc));
        end
    endfunction
    function [31:0] A052Yc;
        input [1:0] A055Yc;
        reg [31:0] A059Yc [0:3];
        begin
            {A059Yc[0 ],A059Yc[1 ],A059Yc[2 ],A059Yc[3 ]}
            = 
            {32'h5a827999, 32'h6ed9eba1, 32'h8f1bbcdc, 32'hca62c1d6};
            A052Yc = A059Yc[A055Yc];
        end
    endfunction
endmodule
module hash_hp_A060Yc  #(
     parameter  A061Yc      =   1           
    ,parameter  A062Yc      =   1           
    ,parameter  A063Yc      =   1           
    ,parameter  A064Yc      =   1           
    ,parameter  A065Yc      =   1
    ,parameter  A066Yc      =   1
)(
    input    wire                          clk            ,
    input    wire                          reset_n        ,
    input    wire                          A067Yc           , 
    input    wire                          A068Yc         , 
    input    wire  [3:0]                   A069Yc         , 
    output   wire                          A070Yc         , 
    output   wire                          A071Yc         , 
    output   wire                          A072Yc         , 
    output   wire                          A073Yc         , 
    output   wire  [6:0]                   A074Yc          , 
    output   wire                          A075Yc         , 
    output   wire                          A076Yc         
);
    localparam  A077Yc   = 3'b000,
                A078Yc   = 3'b010,
                A079Yc    = 3'b100,
                A080Yc   = 3'b101,
                A081Yc   = 3'b110;
    reg     [2:0]   A082Yc         ,  A083Yc    ;
    reg     [6:0]   A084Yc          ;
    reg             A085Yc         ;
    wire            A086Yc         ;
    wire            A087Yc         ;
    wire            A088Yc         ;
    wire            A089Yc         ;
    wire            A090Yc         ;
    wire            A091Yc         ;
    wire            A092Yc         ;
    wire            A093Yc         ;
    wire            A094Yc         ;
    wire            A095Yc         ;
    wire            A096Yc         ;
    wire            A097Yc         ;
    wire            A098Yc         ;
    wire            A099Yc         ;
    wire            A100Yc         ;
    wire            A101Yc         ;
    wire            A102Yc         ;
    assign A098Yc         = A061Yc      && (A069Yc==4'b0001);
    assign A099Yc         = A065Yc      && (A069Yc==4'b0000);
    assign A100Yc         = A062Yc      && (A069Yc==4'b0101);
    assign A101Yc         = 
            A063Yc      && ((A069Yc==4'b0010) || (A069Yc==4'b0110));
    assign A102Yc         = 
            A064Yc      && (
                (A069Yc==4'b0100) || (A069Yc==4'b0011) ||
                (A069Yc==4'b0111) || (A069Yc==4'b1000));
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n)
            A082Yc        <= A077Yc;
        else
            A082Yc        <=  A083Yc    ;
    end
    always @*
    begin
        A083Yc     = A082Yc       ;
        case(A082Yc       )
            A077Yc      :   A083Yc     = A067Yc ? A078Yc   : A077Yc;
            A078Yc      :   A083Yc     = A068Yc    ? A081Yc : A079Yc;
            A079Yc       :   begin
                                if(A099Yc     ) 
                                   A083Yc     = 
                                    !A091Yc       ? (A068Yc    ? A081Yc : A079Yc) : 
                                                                  A080Yc  ;
                                else if(A098Yc     ) 
                                   A083Yc     =  
                                    !A091Yc       ? (A068Yc    ? A081Yc : A079Yc) :  
                                                                  A080Yc  ;
                                else if(A101Yc        ) 
                                   A083Yc     =  
                                    !A091Yc       ? (A068Yc    ? A081Yc : A079Yc) :  
                                                                  A080Yc  ;
                                else if(A102Yc        )
                                   A083Yc     =  
                                    !A091Yc       ? (A068Yc    ? A081Yc : A079Yc) :  
                                                                  A080Yc  ;
                                else if(A100Yc      )
                                   A083Yc     =  
                                    !A091Yc       ? (A068Yc    ? A081Yc : A079Yc) :  
                                                                  A080Yc  ;
                            end
            A080Yc      :   A083Yc     = A081Yc;
            A081Yc      :   A083Yc     = A077Yc;
            default     :   A083Yc     = A077Yc;
        endcase
    end
    assign A089Yc           = A082Yc       ==A078Yc  ;
    assign A090Yc           = A082Yc       ==A080Yc  ;
    assign A088Yc           = A082Yc       !=A077Yc;
    assign A086Yc           = A082Yc       ==A081Yc;
    assign A092Yc           = A082Yc       ==A079Yc;
    assign A091Yc           = A102Yc         || A100Yc       ? (A084Yc==('d80-A066Yc     )) : (A084Yc==('d64-A066Yc     ));
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A084Yc <= 7'd0;
        else if (A092Yc     ) begin
            A084Yc <=  
                (A099Yc        ) && (A084Yc==('d64-A066Yc     )) ? 7'd0 :
                (A098Yc        ) && (A084Yc==('d64-A066Yc     )) ? 7'd0 :
                (A101Yc        ) && (A084Yc==('d64-A066Yc     )) ? 7'd0 :
                (A102Yc        ) && (A084Yc==('d80-A066Yc     )) ? 7'd0 : 
                (A100Yc        ) && (A084Yc==('d80-A066Yc     )) ? 7'd0 : 
                                                   A084Yc + A066Yc     ;
            end
        else begin
            A084Yc <=  7'd0;
        end
    end
    always@(posedge clk or negedge reset_n)
    begin
        if(!reset_n)
            A085Yc         <= 1'b0;
        else
            A085Yc         <= A087Yc        ;
    end
    assign A087Yc         = (A067Yc || !A068Yc   )                                                       ? 1'b0 :
                            (((A082Yc       ==A078Yc   || A082Yc       ==A079Yc) && A083Yc    ==A081Yc) ? 1'b1 :
                                                                                                         A085Yc        );
    assign A093Yc       = A099Yc         && A092Yc     ;
    assign A094Yc       = A098Yc         && A092Yc     ;
    assign A095Yc       = A101Yc         && A092Yc     ; 
    assign A096Yc       = A102Yc         && A092Yc     ;
    assign A097Yc       = A100Yc         && A092Yc     ;
    assign   A072Yc         =  A088Yc      ;
    assign   A070Yc         =  A086Yc      ;
    assign   A071Yc         =  A085Yc        ;
    assign   A073Yc         =  A089Yc      ; 
    assign   A074Yc          =  A084Yc       ; 
    assign   A075Yc         =  A090Yc      ;    
    assign   A076Yc         =  A093Yc    || A094Yc    || A097Yc     ||
                               A095Yc       || A096Yc      ;
endmodule
module hash_hp_A103Yc        #(
     parameter  A061Yc      =   1'b1           
    ,parameter  A062Yc      =   1'b1           
    ,parameter  A063Yc      =   1'b1           
    ,parameter  A064Yc      =   1'b1           
    ,parameter  A065Yc      =   1'b1
)(
    input    wire           clk            ,
    input    wire           reset_n        ,
    input    wire           A104Yc         ,
    input    wire           A068Yc         ,
    input    wire           A105Yc         ,
    input    wire           A106Yc         ,  
    input    wire           A107Yc           ,
    input    wire           A108Yc         ,  
    input    wire           A109Yc         ,  
    input    wire           A110Yc         ,  
    input    wire  [3:0]    A111Yc         ,
    input    wire           A112Yc         ,
    input    wire  [127:0]  A113Yc          ,
    output   wire  [127:0]  A114Yc         ,
    output   wire           A115Yc        ,
    output   wire           A116Yc            ,
    output   wire           A072Yc         ,
    output   wire           A070Yc         ,
    output   wire           A117Yc         ,
    output   wire           A118Yc         ,
    input    wire           A119Yc         ,
    input    wire           A120Yc         ,
    output   wire  [1:0]    A121Yc          ,
    output   wire  [6:0]    A122Yc         ,
    output   wire  [31:0]   A123Yc         
);
    localparam [3:0]  A077Yc    = 4'h0,
                      A124Yc      = 4'h1,
                      A081Yc    = 4'h2,
                      A125Yc     = 4'h3,
                      A126Yc    = 4'h4,
                      A127Yc    = 4'h5;
    reg   [3:0]     A082Yc         ,  A083Yc            ;
    reg   [1:0]     A084Yc          ;
    reg   [7:0]     A128Yc         ;
    reg   [6:0]     A129Yc         ;
    reg   [1:0]     A130Yc         ;
    reg   [6:0]     A131Yc         ;
    reg   [31:0]    A132Yc         ;
    reg             A133Yc             ;
    reg             A134Yc             ;
    reg   [127:0]   A135Yc             ;
    wire            A136Yc             ;
    wire            A088Yc         ;
    wire            A086Yc         ;
    wire            A137Yc         ;
    wire            A138Yc         ;
    wire            A139Yc         ;
    wire            A140Yc         ;
    wire            A141Yc         ;
    wire            A142Yc         ;
    wire            A143Yc         ;
    wire            A144Yc         ;
    wire            A145Yc         ;
    wire  [7:0]     A146Yc         ;
    wire            A147Yc         ;
    wire            A148Yc         ;
    wire            A149Yc         ;
    wire  [127:0]   A150Yc         ;
    wire            A098Yc         ;
    wire            A099Yc         ;
    wire            A100Yc         ;
    wire            A101Yc         ;
    wire            A102Yc         ;
    assign A098Yc         = A061Yc      && (A111Yc==4'b0001);
    assign A099Yc         = A065Yc      && (A111Yc==4'b0000);
    assign A100Yc         = A062Yc      && (A111Yc==4'b0101);
    assign A101Yc         = 
            A063Yc      && ((A111Yc==4'b0010) || (A111Yc==4'b0110));
    assign A102Yc         = 
            A064Yc      && (
                (A111Yc==4'b0100) || (A111Yc==4'b0011) || 
                (A111Yc==4'b0111) || (A111Yc==4'b1000));
    assign A139Yc      = A104Yc ;
    assign A142Yc      = A099Yc      || A100Yc       ||
                         A101Yc         || A098Yc     ;
    assign A143Yc      = A102Yc        ;
    assign A150Yc        = {A113Yc[124:0],3'b000};
    assign A140Yc      = (A142Yc      && (A113Yc[5:0]>=6'h38)) ||
                         (A143Yc      && (A113Yc[6:0]>=7'h70));
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n)
            A082Yc        <= A077Yc;
        else
            A082Yc        <=  A083Yc    ;
    end
    always @*
    begin
        A083Yc     = A082Yc       ;
        case(A082Yc       )
            A077Yc   : A083Yc     = 
                        (A139Yc    && A137Yc   )              ? A081Yc    :
                        (A139Yc    && !A112Yc)                ? A124Yc      :
                        (A139Yc    &&  A112Yc && A140Yc     ) ? A126Yc    : 
                        (A139Yc    &&  A112Yc)                ? A125Yc     : 
                                                                A077Yc;
            A124Yc     : A083Yc     = A137Yc     ? A081Yc :
                                    A119Yc     ? A081Yc : A124Yc;
            A081Yc   : A083Yc     = A077Yc;
            A125Yc    : A083Yc     = (A137Yc    || A068Yc   )  ? A081Yc :
                                    A144Yc                    ? A124Yc   : A125Yc;
            A126Yc   : A083Yc     = (A137Yc    || A068Yc   )  ? A081Yc    :
                                    A144Yc                    ? A127Yc    : A126Yc   ;
            A127Yc   : A083Yc     = A137Yc     ? A081Yc :
                                    A119Yc     ? A125Yc : A127Yc   ;
            default  : A083Yc     = A077Yc;
        endcase
    end
    assign A088Yc = A082Yc        != A077Yc;
    assign A086Yc = A082Yc        == A081Yc;
    assign A145Yc      = A082Yc        == A125Yc;
    assign A147Yc          = A082Yc        == A126Yc   ;
    assign A148Yc          = A082Yc        == A127Yc   ;
    assign A149Yc     = A082Yc        == A124Yc;
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A084Yc <= 2'd0;
        else if (!A088Yc)
            A084Yc <=  2'd0;
        else if (A119Yc    )
            A084Yc <=  2'd0;
        else if (A149Yc     || A148Yc         ) 
            A084Yc <=  &A084Yc ? A084Yc : A084Yc+1;
    end
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A128Yc    <= 8'd0;
        else if (!A088Yc)
            A128Yc    <=  8'd0;
        else if (A119Yc    )
            A128Yc    <=  8'd0;
        else if (A145Yc     ) begin
                if(A146Yc   [1:0]=='d0) 
                    A128Yc    <=  A128Yc    + 4;
                else
                    A128Yc    <=  A128Yc    + 1;
        end
        else if (A147Yc         ) begin
                if(A146Yc   [1:0]=='d0) 
                    A128Yc    <=  A128Yc    + 4;
                else
                    A128Yc    <=  A128Yc    + 1;
        end
    end
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A129Yc    <= 7'd0;
        else if (A104Yc  && !(A088Yc)) 
            A129Yc    <=  A142Yc      ? {1'b0,A113Yc[5:0]} :
                                 A143Yc      ?       A113Yc[6:0]  : 7'd0;
        else if (A119Yc    ) 
            A129Yc    <=  7'd0;
    end
    assign A146Yc    = A142Yc      ? {2'd0,A129Yc   [5:0]} + A128Yc    : 
                       A143Yc      ? {1'd0,A129Yc   [6:0]} + A128Yc    : 7'd0;
    assign A144Yc     = A142Yc      ? A146Yc   =='h40 :
                        A143Yc      ? A146Yc   =='h80 : 1'b0;
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A130Yc     <= 2'd0;
        else if ((A104Yc  && !(A088Yc)) || A144Yc     || A086Yc)
            A130Yc     <=  2'd0;
        else if (A145Yc     ) begin
                if(A146Yc   [1:0]=='d0) 
                    A130Yc     <=  2'b10;
                else
                    A130Yc     <=  2'b01;
        end
        else if (A147Yc         ) begin
                if(A146Yc   [1:0]=='d0) 
                    A130Yc     <=  2'b10;
                else
                    A130Yc     <=  2'b01;
        end
    end
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A131Yc     <= 7'd0;
        else if ((A104Yc  && !(A088Yc)) || A144Yc    )
            A131Yc     <=  7'd0;
        else 
            A131Yc     <= 
                 (A145Yc      || A147Yc         ) ? A146Yc   [6:0] : 
                                                          7'd0;
    end
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) 
            A132Yc     <= 'd0;
        else if ((A104Yc  && !(A088Yc)) || A144Yc    )
            A132Yc     <=  'd0;
        else if (A145Yc     ) begin
                if (A146Yc   [1:0]!=2'd0) begin 
                        A132Yc     <= 
                            (A128Yc    =='h0 ) ? 'h80 :
                            A142Yc     && (A146Yc    < 'h38) ? 'd0 :
                            A098Yc     && (A146Yc    >='h38) ?
                                {A150Yc       [(  A146Yc   [  2])*32+0*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+1*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+2*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+3*8+:8]} :
                            A142Yc     && (A146Yc    >='h38) ? 
                                A150Yc       [(1-A146Yc   [  2])*32+:32] : 
                            A143Yc     && (A146Yc    < 'h70) ? 'd0 :
                            A143Yc     && (A146Yc    >='h70) ? 
                                A150Yc       [(3-A146Yc   [3:2])*32+:32] : 
                                'd0;
                end
                else begin 
                    if (A140Yc     ) begin
                        A132Yc     <=  
                            A142Yc     && (A146Yc    < 'h38) ? 'd0 :
                            A098Yc     && (A146Yc    >='h38) ?
                                {A150Yc       [(  A146Yc   [  2])*32+0*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+1*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+2*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+3*8+:8]} :
                            A142Yc     && (A146Yc    >='h38) ?  
                                A150Yc       [(1-A146Yc   [  2])*32+:32] : 
                            A143Yc     && (A146Yc    < 'h70) ? 'd0 :
                            A143Yc     && (A146Yc    >='h70) ?  
                                A150Yc       [(3-A146Yc   [3:2])*32+:32] :  
                                'd0;
                    end
                    else begin
                        A132Yc     <= 
                            (A128Yc    =='h0 ) ? 'h80000000 :
                            A142Yc     && (A146Yc    < 'h38) ? 'd0 :
                            A098Yc     && (A146Yc    >='h38) ?
                                {A150Yc       [(  A146Yc   [  2])*32+0*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+1*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+2*8+:8],
                                 A150Yc       [(  A146Yc   [  2])*32+3*8+:8]} :
                            A142Yc     && (A146Yc    >='h38) ? 
                                A150Yc       [(1-A146Yc   [  2])*32+:32] : 
                            A143Yc     && (A146Yc    < 'h70) ? 'd0 :
                            A143Yc     && (A146Yc    >='h70) ? 
                                A150Yc       [(3-A146Yc   [3:2])*32+:32] : 
                                'd0;
                    end
                end
        end
        else if (A147Yc         ) begin
                if (A146Yc   [1:0]!=2'd0) begin 
                    A132Yc     <=  (A128Yc    =='h0) ? 'h80 : 'd0;
                end
                else begin 
                    A132Yc     <=  (A128Yc    =='h0) ? 'h80000000 : 'd0;
                end
        end
    end
    always@(posedge clk or negedge reset_n)
    begin
        if(!reset_n)
            A134Yc             <= 1'b0;
        else
            A134Yc             <= A136Yc            ;
    end
    assign A136Yc             = (A104Yc  || !A068Yc   ) ? 1'b0 :
                                ((((A145Yc      || A147Yc         ) && (A083Yc    ==A081Yc)) || (A119Yc     && A120Yc         && !A107Yc           ) || A106Yc         ) ? 1'b1 : A134Yc            );
    always@(posedge clk or negedge reset_n)
    begin
        if(!reset_n)
            A133Yc         <= 1'b0;
        else if(A109Yc       && ((A112Yc && A134Yc            ) || A106Yc         ))
            A133Yc         <= 1'b1;
        else if(A110Yc )
            A133Yc         <= 1'b0;
    end
    always@(posedge clk or negedge reset_n)
    begin
        if(!reset_n)
            A135Yc        <= 128'b0;
        else if(A105Yc   ) begin
            if(A108Yc       ) begin
                if(A086Yc && (!A134Yc            )) begin
                    if(A112Yc) begin
                        A135Yc        <= A142Yc      ? A135Yc        + A113Yc[5:0] :
                                         A143Yc      ? A135Yc        + A113Yc[6:0] :
                                                       A135Yc       ;
                    end
                    else begin
                        A135Yc        <= A142Yc      ? A135Yc        + 7'h40 :
                                         A143Yc      ? A135Yc        + 8'h80 :
                                                       A135Yc       ;
                    end
                end
            end
            else if(A106Yc         ) begin
                if(!A109Yc      )
                    A135Yc        <= A142Yc      ? {A135Yc       [127:6],6'd0} :
                                     A143Yc      ? {A135Yc       [127:7],7'd0} :
                                                   A135Yc       ;
            end
            else if(A110Yc ) begin
                A135Yc        <= 128'b0;
            end
        end
        else begin
            if(A086Yc && (!A134Yc            )) begin
                if(A112Yc) begin
                    A135Yc        <= A142Yc      ? A135Yc        + A113Yc[5:0] :
                                     A143Yc      ? A135Yc        + A113Yc[6:0] :
                                                   A135Yc       ;
                end
                else begin
                    A135Yc        <= A142Yc      ? A135Yc        + 7'h40 :
                                     A143Yc      ? A135Yc        + 8'h80 :
                                                   A135Yc       ;
                end
            end
            else if(A110Yc ) begin
                A135Yc        <= 128'b0;
            end
        end
    end
    assign A137Yc           = A111Yc > 'd8;
    assign A138Yc           = A084Yc == 'd1;
    assign A114Yc             = {A135Yc        << 3};
    assign A115Yc             = A133Yc        ;
    assign A116Yc             = A134Yc              ;
    assign A072Yc             = A088Yc          ;
    assign A117Yc             = A137Yc          ;
    assign A118Yc             = A138Yc          ;
    assign A070Yc             = A086Yc          ;
    assign A121Yc              = A130Yc          ;
    assign A122Yc             = A131Yc          ;
    assign A123Yc             = A132Yc          ;
endmodule
module hash_hp_A151Yc       #(
    parameter   A061Yc                      =  1'b1           
   ,parameter   A062Yc                      =  1'b1           
   ,parameter   A063Yc                      =  1'b1           
   ,parameter   A064Yc                      =  1'b1           
   ,parameter   A065Yc                      =  1'b1           
   ,parameter   A066Yc                      =  1'b1
)(
    input   wire                            clk             
   ,input   wire                            rst_n           
   ,input   wire                            A104Yc   
   ,input   wire                            A068Yc   
   ,input   wire                            A105Yc   
   ,input   wire                            A106Yc           
   ,input   wire                            A107Yc           
   ,input   wire                            A108Yc           
   ,input   wire                            A109Yc           
   ,input   wire                            A110Yc           
   ,input   wire    [3:0]                   A111Yc   
   ,input   wire                            A112Yc   
   ,input   wire    [127:0]                 A113Yc    
   ,output  wire    [127:0]                 A114Yc            
   ,output  wire                            A115Yc            
   ,output  wire                            A116Yc            
   ,output  wire                            A072Yc   
   ,output  wire                            A070Yc   
   ,input   wire    [1023:0]                A152Yc
   ,input   wire    [31:0]                  A153Yc     
   ,input   wire    [511:0]                 A154Yc     
   ,input   wire    [15:0]                  A155Yc     
   ,output  wire    [511:0]                 A156Yc      
);    
    localparam  A157Yc      = A064Yc     ? 512 :
                              A063Yc     ? 256 :
                              A065Yc     ? 256 :
                              A062Yc     ? 160 :
                              A061Yc     ? 128 :
                                           0;
    wire                        A158Yc              ;
    wire                        A159Yc              ; 
    wire  [1:0]                 A160Yc               ;
    wire  [6:0]                 A161Yc              ;
    wire  [31:0]                A162Yc              ; 
    wire                        A089Yc              ;
    wire  [6:0]                 A163Yc               ;
    wire                        A090Yc              ;
    wire                        A164Yc              ;
    wire                        A087Yc              ;
    wire  [255:0]               A165Yc              ;
    wire  [A066Yc     *32-1:0]  A166Yc              ;
    wire  [127:0]               A167Yc              ;
    wire  [255:0]               A168Yc              ;
    wire  [A066Yc     *32-1:0]  A169Yc              ;
    wire  [511:0]               A170Yc              ;
    wire  [A066Yc     *64-1:0]  A171Yc              ;
    wire  [159:0]               A172Yc              ;
    wire  [A066Yc     *32-1:0]  A173Yc              ;
    wire                        A174Yc              ;
    wire                        A175Yc              ;
    wire  [A157Yc     -1:0]     A176Yc              ;
    wire  [511:0]               A177Yc              ;
    wire  [1023:0]              A178Yc              ;
    wire                        A179Yc              ;
    wire                        A180Yc              ;
hash_hp_A103Yc       
    A181Yc         
    (
      .clk                 (clk                )
   ,  .reset_n             (rst_n              )
   ,  .A104Yc              (A104Yc             )
   ,  .A068Yc              (A068Yc             )
   ,  .A109Yc              (A109Yc             )
   ,  .A105Yc              (A105Yc             )
   ,  .A106Yc              (A106Yc             )
   ,  .A107Yc              (A107Yc             )
   ,  .A108Yc              (A108Yc             )
   ,  .A110Yc              (A110Yc             )
   ,  .A111Yc              (A111Yc             )
   ,  .A112Yc              (A112Yc             )
   ,  .A113Yc               (A113Yc              )
   ,  .A114Yc              (A114Yc             )
   ,  .A115Yc              (A115Yc             )
   ,  .A116Yc              (A116Yc             )
   ,  .A072Yc              (A072Yc             )
   ,  .A070Yc              (A070Yc             )
   ,  .A117Yc              (A179Yc             )
   ,  .A118Yc              (A158Yc             )
   ,  .A119Yc              (A164Yc             )
   ,  .A120Yc              (A087Yc             )
   ,  .A121Yc               (A160Yc              )
   ,  .A122Yc              (A161Yc             )
   ,  .A123Yc              (A162Yc             )
    );
hash_hp_A060Yc 
    #(
      .A061Yc         (A061Yc             )
   ,  .A062Yc         (A062Yc             )
   ,  .A063Yc         (A063Yc             )
   ,  .A064Yc         (A064Yc             )
   ,  .A065Yc         (A065Yc             )
   ,  .A066Yc         (A066Yc             )
    )
    A182Yc   
    (
      .clk            (clk                )
   ,  .reset_n        (rst_n              )
   ,  .A067Yc           (A158Yc             )
   ,  .A068Yc         (A068Yc             )
   ,  .A069Yc         (A111Yc             )
   ,  .A070Yc         (A164Yc             )
   ,  .A071Yc         (A087Yc             )
   ,  .A072Yc         (A180Yc             ) 
   ,  .A073Yc         (A089Yc             )
   ,  .A074Yc          (A163Yc              )
   ,  .A075Yc         (A090Yc             )
   ,  .A076Yc         (A175Yc             )
    );
hash_hp_A183Yc   
    #(
      .A061Yc         (A061Yc             )
   ,  .A062Yc         (A062Yc             )
   ,  .A063Yc         (A063Yc             )
   ,  .A064Yc         (A064Yc             )
   ,  .A065Yc         (A065Yc             )
   ,  .A066Yc         (A066Yc             )
    )A184Yc    (
      .clk            (clk                )
   ,  .reset_n        (rst_n              )
   ,  .A155Yc         (A155Yc             )
   ,  .A154Yc         (A154Yc             )
   ,  .A156Yc         (A156Yc             )
   ,  .A185Yc         (A152Yc          )
   ,  .A186Yc         (A153Yc              )
   ,  .A187Yc         (A089Yc             )
   ,  .A188Yc         (A090Yc             )
   ,  .A189Yc          (A175Yc             )
   ,  .A069Yc         (A111Yc             )
   ,  .A190Yc         (A174Yc             )
   ,  .A191Yc         (A166Yc             )
   ,  .A192Yc          (A165Yc             )
   ,  .A193Yc          (A167Yc             )
   ,  .A194Yc         (A169Yc             )
   ,  .A195Yc         (A168Yc             )
   ,  .A196Yc         (A171Yc             )
   ,  .A197Yc         (A170Yc             )
   ,  .A198Yc         (A173Yc             )
   ,  .A199Yc         (A172Yc             )
   ,  .A004Yc          (A176Yc             )
   ,  .A200Yc         (A177Yc             )
   ,  .A201Yc         (A178Yc             )
   ,  .A202Yc          (A160Yc              )
   ,  .A203Yc         (A161Yc             )
   ,  .A204Yc         (A162Yc             )
    );
hash_hp_A205Yc            
    #(
      .A061Yc         (A061Yc             )
   ,  .A062Yc         (A062Yc             )
   ,  .A063Yc         (A063Yc             )
   ,  .A064Yc         (A064Yc             )
   ,  .A065Yc         (A065Yc             )
   ,  .A066Yc         (A066Yc             )
    )A206Yc         (
      .A111Yc         (A111Yc             )
   ,  .A001Yc          (A163Yc              )
   ,  .A207Yc         (A177Yc             )
   ,  .A208Yc         (A178Yc             )
   ,  .A003Yc          (A176Yc             )
   ,  .A209Yc         (A165Yc             )
   ,  .A210Yc         (A167Yc             )
   ,  .A211Yc         (A168Yc             )
   ,  .A212Yc         (A170Yc             )
   ,  .A213Yc         (A172Yc             )
   ,  .A214Yc         (A166Yc             )
   ,  .A215Yc         (A169Yc             )
   ,  .A216Yc         (A171Yc             )
   ,  .A217Yc         (A173Yc             )
   ,  .A046Yc         (A174Yc             )
    );
endmodule
module hash_hp_A218Yc  #(
    parameter   A061Yc                      = 1'b1           
   ,parameter   A062Yc                      = 1'b1           
   ,parameter   A063Yc                      = 1'b1           
   ,parameter   A064Yc                      = 1'b1           
   ,parameter   A065Yc                      = 1'b1   
   ,parameter   A219Yc                      = 1'b1
   ,parameter   A066Yc                      = 1'b1
   ,parameter   A220Yc                      = A219Yc    ? 1152 : 1024
   ,parameter   A221Yc                      = A220Yc      /32
   ,parameter   A222Yc                      = A219Yc    ? 1600 : 512
   ,parameter   A223Yc                      = A222Yc        /32
)(
    input   wire                            clk             
   ,input   wire                            rst_n           
   ,input   wire                            A104Yc   
   ,input   wire                            A068Yc   
   ,input   wire                            A105Yc   
   ,input   wire                            A106Yc           
   ,input   wire                            A107Yc           
   ,input   wire                            A108Yc           
   ,input   wire                            A109Yc           
   ,input   wire                            A110Yc           
   ,input   wire    [3:0]                   A111Yc   
   ,input   wire                            A112Yc   
   ,input   wire    [127:0]                 A113Yc    
   ,output  wire                            A116Yc            
   ,output  wire    [127:0]                 A114Yc            
   ,output  wire                            A115Yc            
   ,output  wire                            A072Yc   
   ,output  wire                            A070Yc   
   ,input   wire    [A220Yc      -1:0]      A152Yc
   ,input   wire    [A221Yc     -1:0]       A153Yc     
   ,input   wire    [A222Yc        -1:0]    A154Yc     
   ,input   wire    [A223Yc       -1:0]     A155Yc     
   ,output  wire    [A222Yc        -1:0]    A224Yc        
   );
    localparam A225Yc        = 0;
    wire                                    A226Yc          ;
    wire    [127:0]                         A227Yc            ;
    wire    [127:0]                         A228Yc            ;
    wire                                    A229Yc             ;
    wire                                    A230Yc             ;
    wire                                    A231Yc                 ;
    wire                                    A232Yc                 ;
    wire                                    A233Yc         ;
    wire                                    A234Yc         ;
    wire    [1023:0]                        A235Yc         ;
    wire    [511:0]                         A236Yc         ;
    wire    [511:0]                         A237Yc           ;
    wire    [31:0]                          A238Yc              ;
    wire    [15:0]                          A239Yc              ;
    wire                                    A240Yc      ;
    wire                                    A241Yc     ;
    wire                                    A242Yc     ;
    wire    [1151:0]                        A243Yc     ;
    wire    [35:0]                          A244Yc          ;
    wire    [1599:0]                        A245Yc     ;
    wire    [49:0]                          A246Yc          ;
    wire    [1599:0]                        A247Yc       ;
    wire    [1:0]                           A248Yc     ;
    wire                                    A249Yc       ;
    wire                                    A250Yc   ;
    generate
    if(A061Yc  |A062Yc   |A063Yc     |A064Yc     |A065Yc  ) begin: A251Yc       
hash_hp_A151Yc      
        #(
          .A061Yc             (A061Yc             )
       ,  .A062Yc             (A062Yc             )
       ,  .A063Yc             (A063Yc             )
       ,  .A064Yc             (A064Yc             )
       ,  .A065Yc             (A065Yc             )
       ,  .A066Yc             (A066Yc             )
        )
        A252Yc        
        (
          .clk                (clk                )
       ,  .rst_n              (rst_n              )
       ,  .A104Yc             (A226Yc             )
       ,  .A068Yc             (A068Yc             )
       ,  .A109Yc             (A109Yc             )
       ,  .A105Yc             (A105Yc             )
       ,  .A106Yc             (A106Yc             )
       ,  .A107Yc             (A107Yc             )
       ,  .A108Yc             (A108Yc             )
       ,  .A110Yc             (A110Yc             )
       ,  .A111Yc             (A111Yc             )
       ,  .A112Yc             (A112Yc             )
       ,  .A113Yc              (A113Yc              )
       ,  .A114Yc             (A227Yc             )
       ,  .A115Yc             (A229Yc             )
       ,  .A116Yc             (A231Yc                  )
       ,  .A072Yc             (A233Yc             )
       ,  .A070Yc             (A234Yc             )
       ,  .A152Yc             (A235Yc                    )
       ,  .A153Yc             (A238Yc                    )
       ,  .A154Yc             (A236Yc                    )
       ,  .A155Yc             (A239Yc                    )
       ,  .A156Yc             (A237Yc                    )
    );
    end
    else begin :A253Yc           
        assign A227Yc               = 128'd0;
        assign A229Yc               = 1'b0;
        assign A231Yc                    = 1'b0;
        assign A233Yc               = 1'b0;
        assign A234Yc               = 1'b0;
        assign A237Yc               = 512'd0;
    end
    if(A219Yc   ) begin: A254Yc   
hash_hp_A255Yc  
        #(
          .A066Yc             (A066Yc             )
        )
        A256Yc    
        (
          .clk                (clk                )
       ,  .rst_n              (rst_n              )
       ,  .A152Yc             (A243Yc             )
       ,  .A153Yc             (A244Yc             )
       ,  .A154Yc             (A245Yc             )
       ,  .A155Yc             (A246Yc             )
       ,  .A112Yc             (A112Yc             )
       ,  .A113Yc              (A113Yc[7:0]         )
       ,  .A111Yc             (A111Yc             )
       ,  .A104Yc             (A240Yc             )
       ,  .A068Yc             (A068Yc             )
       ,  .A110Yc             (A110Yc             )
       ,  .A114Yc             (A228Yc             )
       ,  .A115Yc             (A230Yc             )
       ,  .A116Yc             (A232Yc                  )
       ,  .A072Yc             (A241Yc             )
       ,  .A070Yc             (A242Yc             )
       ,  .A224Yc             (A247Yc             )
        );
    end
    else begin: A257Yc       
        assign A228Yc              = 128'd0;
        assign A230Yc              = 1'b0;
        assign A232Yc                  = 1'b0;
        assign A241Yc              = 1'b0;
        assign A242Yc              = 1'b0;
        assign A247Yc              = 1152'd0;
    end
    endgenerate
    assign A249Yc           = 
            (A061Yc  |A062Yc   |A063Yc     |A064Yc     |A065Yc  )&
               (A111Yc==4'b0001|A111Yc==4'b0101|A111Yc==4'b0000|
                A111Yc==4'b0110|A111Yc==4'b0010|A111Yc==4'b0011|
                A111Yc==4'b0100|A111Yc==4'b0111|
                A111Yc==4'b1000);
    assign A250Yc           = 
            A219Yc   &
                (A111Yc==4'b1001|A111Yc==4'b1010|
                 A111Yc==4'b1011|A111Yc==4'b1100);
    assign A226Yc           = A249Yc        & A104Yc ;
    assign A240Yc           = A250Yc        & A104Yc ;
    generate
    if(A225Yc       ) begin: A258Yc 
        assign A235Yc           = {~A152Yc[A220Yc      -1],A152Yc[A220Yc      -2-:1024]};
        assign A243Yc           = {~A152Yc[A220Yc      -1],A152Yc[A220Yc      -2:0]};
    end
    else begin : A259Yc    
        assign A235Yc           = A152Yc[A220Yc      -1-:1024];
        assign A243Yc           = A152Yc;
    end
    endgenerate
    assign A238Yc               = A249Yc        ? 
                                    A153Yc     [A221Yc     -1-:32] : 32'd0;
    assign A244Yc               = A250Yc    ? A153Yc      : {A221Yc     {1'd0}};
    assign A236Yc           = A154Yc[A222Yc        -1-:512];
    assign A245Yc           = A154Yc;
    assign A239Yc               = A249Yc        ?
                                    A155Yc     [A223Yc       -1-:16] : 16'd0;
    assign A246Yc               = A250Yc    ? A155Yc      : {A223Yc       {1'd0}};
    assign A072Yc   = A233Yc          | (A241Yc     |A242Yc     );
    assign A070Yc   = A234Yc          | A242Yc     ;
    assign A114Yc         = A249Yc        ? A227Yc              : (A250Yc    ? A228Yc              : 128'd0);
    assign A115Yc         = A249Yc        ? A229Yc              : (A250Yc    ? A230Yc              : 1'b0);
    assign A116Yc             = A231Yc                  | A232Yc                 ;
    generate
    if(A219Yc   ) begin: A260Yc     
        assign A224Yc   = A249Yc        ? {A237Yc           ,1088'd0}
                            :(A250Yc    ? A247Yc        : 1600'd0);
    end
    else begin: A261Yc    
        assign A224Yc   = A237Yc           ;
    end
    endgenerate
endmodule
module hash_hp_A262Yc  (
    input   wire    [1599:0]    A263Yc
   ,output  wire    [1599:0]    A264Yc
);
    wire    [1599:0]            A265Yc;
    generate
    genvar A055Yc,A056Yc,A057Yc;
    for(A056Yc=0;A056Yc<5;A056Yc=A056Yc+1) begin :A266Yc
        for(A055Yc=0;A055Yc<5;A055Yc=A055Yc+1) begin: A267Yc
            for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin :A268Yc
                assign A265Yc[64*(5*A056Yc+A055Yc)+A057Yc]    = A263Yc[64*(5*A056Yc+A055Yc)+A057Yc]
                                        ^ ((A263Yc[64*(5*A056Yc+((A055Yc+1)%5))+A057Yc] ^ 1'b1)
                                        & A263Yc[64*(5*A056Yc+((A055Yc+2)%5))+A057Yc]);
            end
        end
    end
    endgenerate
    assign A264Yc  = A265Yc;
 endmodule
module hash_hp_A269Yc   (
    input   wire    [1599:0]    A263Yc,
    input   wire    [4:0]       A270Yc,
    output  wire    [1599:0]    A264Yc
);
    wire    [1599:0]            A271Yc;
    wire    [63:0]              A272Yc;
    wire    [7:0]               A273Yc[6:0];
    assign A273Yc[0]   = 8'd0 + 7*A270Yc;
    assign A273Yc[1]   = 8'd1 + 7*A270Yc;
    assign A273Yc[2]   = 8'd2 + 7*A270Yc;
    assign A273Yc[3]   = 8'd3 + 7*A270Yc;
    assign A273Yc[4]   = 8'd4 + 7*A270Yc;
    assign A273Yc[5]   = 8'd5 + 7*A270Yc;
    assign A273Yc[6]   = 8'd6 + 7*A270Yc;
    assign A272Yc[62:32]  = 31'd0;
    assign A272Yc[30:16]  = 15'd0;
    assign A272Yc[14:8]   = 7'd0;
    assign A272Yc[6:4]    = 3'd0;
    assign A272Yc[2]      = 1'b0;
hash_hp_A274Yc A275Yc(.A276Yc(A273Yc[0]),.A277Yc(A272Yc[0]));
hash_hp_A274Yc A278Yc(.A276Yc(A273Yc[1]),.A277Yc(A272Yc[1]));
hash_hp_A274Yc A279Yc(.A276Yc(A273Yc[2]),.A277Yc(A272Yc[3]));
hash_hp_A274Yc A280Yc(.A276Yc(A273Yc[3]),.A277Yc(A272Yc[7]));
hash_hp_A274Yc A281Yc(.A276Yc(A273Yc[4]),.A277Yc(A272Yc[15]));
hash_hp_A274Yc A282Yc(.A276Yc(A273Yc[5]),.A277Yc(A272Yc[31]));
hash_hp_A274Yc A283Yc(.A276Yc(A273Yc[6]),.A277Yc(A272Yc[63]));
    generate
    genvar A055Yc,A056Yc,A057Yc;
    for(A056Yc=0;A056Yc<5;A056Yc=A056Yc+1) begin : A284Yc     
        for(A055Yc=0;A055Yc<5;A055Yc=A055Yc+1) begin: A285Yc     
            for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin : A286Yc     
                if((A055Yc == 0) & (A056Yc == 0))
                    assign A271Yc[64*(5*0+0)+A057Yc]   = A263Yc[64*(5*0+0)+A057Yc] ^ A272Yc[A057Yc];
                else
                    assign A271Yc[64*(5*A056Yc+A055Yc)+A057Yc]   = A263Yc[64*(5*A056Yc+A055Yc)+A057Yc];
            end
        end
    end
    endgenerate
    assign A264Yc      = A271Yc;
endmodule
module hash_hp_A274Yc(
    input   wire[7:0]           A276Yc
   ,output  wire                A277Yc
);
    wire    [167:0]             A287Yc  ;
    assign A287Yc  [167:0]  = { 1'b1,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,
                                1'b1,1'b0,1'b1,1'b1,1'b0,1'b0,1'b0,1'b1,
                                1'b1,1'b1,1'b1,1'b0,1'b1,1'b0,1'b0,1'b0,
                                1'b0,1'b1,1'b1,1'b1,1'b1,1'b1,1'b1,1'b1,
                                1'b1,1'b0,1'b0,1'b1,1'b0,1'b0,1'b0,1'b0,
                                1'b1,1'b0,1'b1,1'b0,1'b0,1'b1,1'b1,1'b1,
                                1'b1,1'b1,1'b0,1'b1,1'b0,1'b1,1'b0,1'b1,
                                1'b0,1'b1,1'b1,1'b1,1'b0,1'b0,1'b0,1'b0,
                                1'b0,1'b1,1'b1,1'b0,1'b0,1'b0,1'b1,1'b0,
                                1'b1,1'b0,1'b1,1'b1,1'b0,1'b0,1'b1,1'b1,
                                1'b0,1'b0,1'b1,1'b0,1'b1,1'b1,1'b1,1'b1,
                                1'b1,1'b1,1'b0,1'b1,1'b1,1'b1,1'b1,1'b0,
                                1'b0,1'b1,1'b1,1'b0,1'b1,1'b1,1'b1,1'b0,
                                1'b1,1'b1,1'b1,1'b0,1'b0,1'b1,1'b0,1'b1,
                                1'b0,1'b1,1'b0,1'b0,1'b1,1'b0,1'b1,1'b0,
                                1'b0,1'b0,1'b1,1'b0,1'b0,1'b1,1'b0,1'b1,
                                1'b1,1'b0,1'b1,1'b0,1'b0,1'b0,1'b1,1'b1,
                                1'b0,1'b0,1'b1,1'b1,1'b1,1'b0,1'b0,1'b1,
                                1'b1,1'b1,1'b1,1'b0,1'b0,1'b0,1'b1,1'b1,
                                1'b0,1'b1,1'b1,1'b0,1'b0,1'b0,1'b0,1'b1,
                                1'b0,1'b0,1'b0,1'b1,1'b0,1'b1,1'b1,1'b1};
    assign A277Yc = A287Yc  [167-A276Yc];
endmodule
module hash_hp_A288Yc (
    input   wire    [1599:0]    A263Yc
   ,output  wire    [1599:0]    A264Yc
);
    wire    [1599:0]            A265Yc;
    generate
    genvar A055Yc,A056Yc,A057Yc;
    for(A056Yc=0;A056Yc<5;A056Yc=A056Yc+1) begin :A266Yc
        for(A055Yc=0;A055Yc<5;A055Yc=A055Yc+1) begin: A267Yc
            for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin :A268Yc
                assign A265Yc[64*(5*A056Yc+A055Yc)+A057Yc]    = A263Yc[64*(5*A055Yc+((A055Yc+3*A056Yc)%5))+A057Yc];
            end
        end
    end
    endgenerate
    assign A264Yc  = A265Yc;
endmodule
module hash_hp_A289Yc  (
    input   wire    [1599:0]    A263Yc
   ,output  wire    [1599:0]    A264Yc
);
    wire    [1599:0]            A265Yc;
    generate
    genvar A057Yc;
    for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin : A290Yc
        assign A265Yc[64*(5*0+0)+A057Yc]    = A263Yc[64*(5*0+0)+A057Yc];
    end
    for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin : A291Yc
        if(A057Yc >= 1)
            assign A265Yc[64*(5*0+1)+A057Yc]    = A263Yc[64*(5*0+1)+(A057Yc-1)];
        else
            assign A265Yc[64*(5*0+1)+A057Yc]    = A263Yc[64*(5*0+1)+(64+A057Yc-1)];
        if(A057Yc >= 3)
            assign A265Yc[64*(5*2+0)+A057Yc]    = A263Yc[64*(5*2+0)+(A057Yc-3)];
        else
            assign A265Yc[64*(5*2+0)+A057Yc]    = A263Yc[64*(5*2+0)+(64+A057Yc-3)];
        if(A057Yc >= 6)
            assign A265Yc[64*(5*1+2)+A057Yc]    = A263Yc[64*(5*1+2)+(A057Yc-6)];
        else
            assign A265Yc[64*(5*1+2)+A057Yc]    = A263Yc[64*(5*1+2)+(64+A057Yc-6)];
        if(A057Yc >= 10)
            assign A265Yc[64*(5*2+1)+A057Yc]    = A263Yc[64*(5*2+1)+(A057Yc-10)];
        else
            assign A265Yc[64*(5*2+1)+A057Yc]    = A263Yc[64*(5*2+1)+(64+A057Yc-10)];
        if(A057Yc >= 15)
            assign A265Yc[64*(5*3+2)+A057Yc]    = A263Yc[64*(5*3+2)+(A057Yc-15)];
        else
            assign A265Yc[64*(5*3+2)+A057Yc]    = A263Yc[64*(5*3+2)+(64+A057Yc-15)];
        if(A057Yc >= 21)
            assign A265Yc[64*(5*3+3)+A057Yc]    = A263Yc[64*(5*3+3)+(A057Yc-21)];
        else
            assign A265Yc[64*(5*3+3)+A057Yc]    = A263Yc[64*(5*3+3)+(64+A057Yc-21)];
        if(A057Yc >= 28)
            assign A265Yc[64*(5*0+3)+A057Yc]    = A263Yc[64*(5*0+3)+(A057Yc-28)];
        else
            assign A265Yc[64*(5*0+3)+A057Yc]    = A263Yc[64*(5*0+3)+(64+A057Yc-28)];
        if(A057Yc >= 36)
            assign A265Yc[64*(5*1+0)+A057Yc]    = A263Yc[64*(5*1+0)+(A057Yc-36)];
        else
            assign A265Yc[64*(5*1+0)+A057Yc]    = A263Yc[64*(5*1+0)+(64+A057Yc-36)];
        if(A057Yc >= 45)
            assign A265Yc[64*(5*3+1)+A057Yc]    = A263Yc[64*(5*3+1)+(A057Yc-45)];
        else
            assign A265Yc[64*(5*3+1)+A057Yc]    = A263Yc[64*(5*3+1)+(64+A057Yc-45)];
        if(A057Yc >= 55)
            assign A265Yc[64*(5*1+3)+A057Yc]    = A263Yc[64*(5*1+3)+(A057Yc-55)];
        else
            assign A265Yc[64*(5*1+3)+A057Yc]    = A263Yc[64*(5*1+3)+(64+A057Yc-55)];
        if(A057Yc+64 >= 66)
            assign A265Yc[64*(5*4+1)+A057Yc]    = A263Yc[64*(5*4+1)+(64+A057Yc-66)];
        else
            assign A265Yc[64*(5*4+1)+A057Yc]    = A263Yc[64*(5*4+1)+(128+A057Yc-66)];
        if(A057Yc+64 >= 78)
            assign A265Yc[64*(5*4+4)+A057Yc]    = A263Yc[64*(5*4+4)+(64+A057Yc-78)];
        else
            assign A265Yc[64*(5*4+4)+A057Yc]    = A263Yc[64*(5*4+4)+(128+A057Yc-78)];
        if(A057Yc+64 >= 91)
            assign A265Yc[64*(5*0+4)+A057Yc]    = A263Yc[64*(5*0+4)+(64+A057Yc-91)];
        else
            assign A265Yc[64*(5*0+4)+A057Yc]    = A263Yc[64*(5*0+4)+(128+A057Yc-91)];
        if(A057Yc+64 >= 105)
            assign A265Yc[64*(5*3+0)+A057Yc]    = A263Yc[64*(5*3+0)+(64+A057Yc-105)];
        else
            assign A265Yc[64*(5*3+0)+A057Yc]    = A263Yc[64*(5*3+0)+(128+A057Yc-105)];
        if(A057Yc+64 >= 120)
            assign A265Yc[64*(5*4+3)+A057Yc]    = A263Yc[64*(5*4+3)+(64+A057Yc-120)];
        else
            assign A265Yc[64*(5*4+3)+A057Yc]    = A263Yc[64*(5*4+3)+(128+A057Yc-120)];
        if(A057Yc+128 >= 136)
            assign A265Yc[64*(5*3+4)+A057Yc]    = A263Yc[64*(5*3+4)+(128+A057Yc-136)];
        else
            assign A265Yc[64*(5*3+4)+A057Yc]    = A263Yc[64*(5*3+4)+(192+A057Yc-136)];
        if(A057Yc+128 >= 153)
            assign A265Yc[64*(5*2+3)+A057Yc]    = A263Yc[64*(5*2+3)+(128+A057Yc-153)];
        else
            assign A265Yc[64*(5*2+3)+A057Yc]    = A263Yc[64*(5*2+3)+(192+A057Yc-153)];
        if(A057Yc+128 >= 171)
            assign A265Yc[64*(5*2+2)+A057Yc]    = A263Yc[64*(5*2+2)+(128+A057Yc-171)];
        else
            assign A265Yc[64*(5*2+2)+A057Yc]    = A263Yc[64*(5*2+2)+(192+A057Yc-171)];
        if(A057Yc+128 >= 190)
            assign A265Yc[64*(5*0+2)+A057Yc]    = A263Yc[64*(5*0+2)+(128+A057Yc-190)];
        else
            assign A265Yc[64*(5*0+2)+A057Yc]    = A263Yc[64*(5*0+2)+(192+A057Yc-190)];
        if(A057Yc+192 >= 210)
            assign A265Yc[64*(5*4+0)+A057Yc]    = A263Yc[64*(5*4+0)+(192+A057Yc-210)];
        else
            assign A265Yc[64*(5*4+0)+A057Yc]    = A263Yc[64*(5*4+0)+(256+A057Yc-210)];
        if(A057Yc+192 >= 231)
            assign A265Yc[64*(5*2+4)+A057Yc]    = A263Yc[64*(5*2+4)+(192+A057Yc-231)];
        else
            assign A265Yc[64*(5*2+4)+A057Yc]    = A263Yc[64*(5*2+4)+(256+A057Yc-231)];
        if(A057Yc+192 >= 253)
            assign A265Yc[64*(5*4+2)+A057Yc]    = A263Yc[64*(5*4+2)+(192+A057Yc-253)];
        else
            assign A265Yc[64*(5*4+2)+A057Yc]    = A263Yc[64*(5*4+2)+(256+A057Yc-253)];
        if(A057Yc+256 >= 276)
            assign A265Yc[64*(5*1+4)+A057Yc]    = A263Yc[64*(5*1+4)+(256+A057Yc-276)];
        else
            assign A265Yc[64*(5*1+4)+A057Yc]    = A263Yc[64*(5*1+4)+(320+A057Yc-276)];
        if(A057Yc+256 >= 300)
            assign A265Yc[64*(5*1+1)+A057Yc]    = A263Yc[64*(5*1+1)+(256+A057Yc-300)];
        else
            assign A265Yc[64*(5*1+1)+A057Yc]    = A263Yc[64*(5*1+1)+(320+A057Yc-300)];
    end
    endgenerate
    assign A264Yc  = A265Yc;
endmodule
module hash_hp_A292Yc    (
    input   wire    [1599:0]    A263Yc
   ,output  wire    [1599:0]    A264Yc
);
    wire    [319:0]             A293Yc;
    wire    [319:0]             A294Yc;
    wire    [1599:0]            A265Yc;
    generate
    genvar A055Yc,A056Yc,A057Yc;
    for(A055Yc=0;A055Yc<5;A055Yc=A055Yc+1) begin: A295Yc 
        for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin: A296Yc 
            assign A293Yc[64*A055Yc+A057Yc]  = A263Yc[64*(5*0+A055Yc)+A057Yc] ^ A263Yc[64*(5*1+A055Yc)+A057Yc]
                                ^ A263Yc[64*(5*2+A055Yc)+A057Yc] ^ A263Yc[64*(5*3+A055Yc)+A057Yc]
                                ^ A263Yc[64*(5*4+A055Yc)+A057Yc];
        end
    end
    assign A294Yc[0]   = A293Yc[64*4] ^ A293Yc[64*1+63];
    for(A057Yc=1;A057Yc<64;A057Yc=A057Yc+1) begin: A297Yc  
        assign A294Yc[A057Yc]   = A293Yc[64*4+A057Yc] ^ A293Yc[64*1+((A057Yc-1)%64)];
    end
    for(A055Yc=1;A055Yc<5;A055Yc=A055Yc+1) begin: A298Yc 
        assign A294Yc[64*A055Yc]    = A293Yc[64*((A055Yc-1)%5)] ^ A293Yc[64*((A055Yc+1)%5)+63];
        for(A057Yc=1;A057Yc<64;A057Yc=A057Yc+1) begin: A299Yc 
            assign A294Yc[64*A055Yc+A057Yc]  = A293Yc[64*((A055Yc-1)%5)+A057Yc]
                                ^ A293Yc[64*((A055Yc+1)%5)+((A057Yc-1)%64)];
        end
    end
    for(A056Yc=0;A056Yc<5;A056Yc=A056Yc+1) begin: A300Yc 
        for(A055Yc=0;A055Yc<5;A055Yc=A055Yc+1) begin: A301Yc 
            for(A057Yc=0;A057Yc<64;A057Yc=A057Yc+1) begin: A302Yc 
                assign A265Yc[64*(5*A056Yc+A055Yc)+A057Yc]    =  A263Yc[64*(5*A056Yc+A055Yc)+A057Yc] ^ A294Yc[64*A055Yc+A057Yc];
            end
        end
    end
    endgenerate
    assign A264Yc  = A265Yc;
endmodule
module hash_hp_A303Yc  (
    input   wire    [1599:0]    A263Yc,
    input   wire    [4:0]       A270Yc,
    output  wire    [1599:0]    A264Yc
);
    wire    [1599:0]            A271Yc,A304Yc,A305Yc,A306Yc;
hash_hp_A292Yc     A307Yc (.A263Yc(A263Yc),.A264Yc(A271Yc));
hash_hp_A289Yc   A308Yc(.A263Yc(A271Yc),.A264Yc(A304Yc));
hash_hp_A288Yc  A309Yc(.A263Yc(A304Yc),.A264Yc(A305Yc));
hash_hp_A262Yc   A310Yc(.A263Yc(A305Yc),.A264Yc(A306Yc));
hash_hp_A269Yc    A311Yc(.A263Yc(A306Yc),.A270Yc(A270Yc),.A264Yc(A264Yc));
endmodule
module hash_hp_A312Yc        #(
    parameter   A066Yc          = 1'b1
)(
    input   wire                clk
   ,input   wire                rst_n
   ,input   wire                A104Yc 
   ,input   wire [7:0]          A113Yc  
   ,input   wire                A068Yc   
   ,input   wire                A110Yc  
   ,input   wire                A112Yc  
   ,input   wire [3:0]          A111Yc  
   ,output  wire [127:0]        A114Yc            
   ,output  wire                A115Yc            
   ,output  wire                A116Yc            
   ,output  wire                A070Yc
   ,output  wire                A072Yc
   ,output  wire                A121Yc 
   ,output  wire    [4:0]       A313Yc
);
    localparam                  A314Yc          = 2'd3;
    localparam                  A077Yc          = 3'b000,
                                A315Yc          = 3'b001,
                                A316Yc          = 3'b010,
                                A081Yc          = 3'b011,
                                A125Yc           = 3'b100;
    localparam                  A317Yc          = A066Yc     ==1 ? 24 
                                                : (A066Yc     ==2 ? 12 : 0);
    reg     [A314Yc       -1:0] A082Yc       , A083Yc    ;                            
    reg     [4:0]               A318Yc;
    reg                         A319Yc;
    reg                         A320Yc;
    reg                         A133Yc        ;
    reg                         A134Yc            ;
    reg     [127:0]             A135Yc       ;
    wire                        A087Yc        ;
    always @(posedge clk or negedge rst_n)
    begin         
        if(!rst_n) begin
            A082Yc          <= A077Yc;
        end
        else            
            A082Yc          <=  A083Yc    ;    
    end
    always @(*)    
    begin        
        A083Yc     = A082Yc       ;        
        case(A082Yc       )
            A077Yc      :   A083Yc      = A104Yc  && A112Yc ? A125Yc   : 
                                          A104Yc            ? A315Yc  : 
                                                              A077Yc;
            A125Yc       :   A083Yc      = A315Yc ;
            A315Yc      :   A083Yc      = A068Yc    ? A081Yc : A316Yc;
            A316Yc      :   A083Yc      = (A318Yc < A317Yc         -1) ? (A068Yc    ? A081Yc : A316Yc) : A081Yc;
            A081Yc      :   A083Yc      = A077Yc;
            default     :   A083Yc      = A077Yc;        
        endcase    
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A318Yc        <= 5'd0;
            A319Yc      <= 1'b0;
            A320Yc      <= 1'b0;
        end
        else begin
            case(A083Yc    )
                A077Yc      :   begin
                                    A319Yc      <=  1'b0;
                                    A320Yc      <=  1'b0;
                                end
                A125Yc       :   begin
                                    A319Yc      <=  1'b0;
                                    A320Yc      <=  1'b0;
                                end
                A315Yc      :   begin
                                    A319Yc      <=  1'b1;
                                    A318Yc        <=  5'd0;
                                end
                A316Yc      :   begin
                                    A318Yc        <=  A318Yc + 1'b1;
                                end
                A081Yc      :   begin
                                    A319Yc      <=  1'b0;
                                    A320Yc      <=  1'b1;
                                end
                default     :;
            endcase
        end
    end
    assign A087Yc         = ((A082Yc       ==A315Yc  || A318Yc < A317Yc         -1) && A083Yc    ==A081Yc) ? 1'b1 : 1'b0;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A133Yc         <= 1'b0;
        else if((A113Yc==8'd0) && A112Yc && A087Yc        )
            A133Yc         <= 1'b1;
        else if(A110Yc )
            A133Yc         <= 1'b0;
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A135Yc        <= 128'b0;
        else if((A082Yc       ==A081Yc) && (A318Yc==(A317Yc         -1))) begin
            if(A112Yc) begin
                A135Yc        <= A135Yc        + A113Yc[7:0];
            end
            else begin
                A135Yc        <= A111Yc == 4'b1001 ? A135Yc        + 8'h90 :
                                 A111Yc == 4'b1010 ? A135Yc        + 8'h88 :
                                 A111Yc == 4'b1011 ? A135Yc        + 8'h68 :
                                 A111Yc == 4'b1100 ? A135Yc        + 8'h48 :
                                                       A135Yc       ;
            end
        end
        else if(A110Yc )
            A135Yc        <= 128'b0;
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A134Yc             <= 1'b0;
        else if(A087Yc        )
            A134Yc             <= 1'b1;
        else if(A104Yc  || (!A068Yc   ))
            A134Yc             <= 1'b0;
    end
    assign A116Yc             = A134Yc            ;
    assign A115Yc             = A133Yc        ;
    assign A114Yc             = {A135Yc        << 3};
    assign A070Yc    = A320Yc;
    assign A072Yc    = A319Yc;
    assign A313Yc      = A318Yc;
    assign A121Yc     = A082Yc        == A125Yc;
endmodule
module hash_hp_A321Yc         #(
    parameter   A066Yc                      = 1'b1
)(
    input   wire                clk
   ,input   wire                rst_n
   ,input   wire    [1151:0]    A152Yc
   ,input   wire    [35:0]      A153Yc     
   ,input   wire    [1599:0]    A154Yc
   ,input   wire    [49:0]      A155Yc     
   ,input   wire    [3:0]       A111Yc
   ,input   wire                A112Yc
   ,input   wire    [7:0]       A113Yc
   ,input   wire                A322Yc
   ,input   wire    [4:0]       A270Yc
   ,output  wire    [1599:0]    A323Yc
);
    reg     [1599:0]            A324Yc;
    wire    [1599:0]            A265Yc;
    wire    [1599:0]            A325Yc ;
    wire    [1599:0]            A326Yc ;
    wire    [1599:0]            A327Yc     , A328Yc     ;
    wire    [1599:0]            A329Yc     ;
    wire    [1599:0]            A330Yc    ;
    genvar                      A014Yc;
    integer                     A015Yc;
    generate
        if(A066Yc      == 1) begin
hash_hp_A303Yc   A331Yc(.A263Yc(A324Yc),.A270Yc(A270Yc),.A264Yc(A265Yc));
        end
        else if(A066Yc      == 2) begin
hash_hp_A303Yc   A332Yc(.A263Yc(A324Yc),.A270Yc(2'd2*A270Yc),.A264Yc(A325Yc ));
hash_hp_A303Yc   A333Yc(.A263Yc(A325Yc ),.A270Yc(2'd2*A270Yc+1'b1),.A264Yc(A265Yc));
        end
    endgenerate
hash_hp_A334Yc  A335Yc   (.A152Yc(A152Yc),.A111Yc(A111Yc),
                      .A113Yc(A113Yc),.A112Yc(A112Yc),.A336Yc(A326Yc ));
    generate
        for(A014Yc=0;A014Yc<1600;A014Yc=A014Yc+1) begin : A337Yc 
            assign A327Yc     [A014Yc]   = A326Yc [1599-A014Yc];
        end
    endgenerate
    generate
        for(A014Yc=0;A014Yc<200;A014Yc=A014Yc+1) begin : A338Yc  
            assign A330Yc    [1599-8*A014Yc-:8]   = A154Yc[8*A014Yc+:8];
        end
    endgenerate
    assign A329Yc       = A324Yc ^ A327Yc     ;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A324Yc <= 1600'd0;
        end
        else begin
            if(A322Yc)
                A324Yc <=  A265Yc;
            else if(|A155Yc     ) begin
                if(!(|A153Yc     )) begin
                    for(A015Yc=0;A015Yc<50;A015Yc=A015Yc+1) begin
                        if(A155Yc     [A015Yc])
                            A324Yc[(49-A015Yc)*32+:32] <=  A330Yc    [(49-A015Yc)*32+:32];
                    end
                end
                else
                    A324Yc <=  A154Yc ^ A327Yc     ;
            end
            else if(|A153Yc     )
                for(A015Yc=0;A015Yc<36;A015Yc=A015Yc+1) begin
                    if(A153Yc     [A015Yc])
                        A324Yc[(35-A015Yc)*32+:32] <=  A329Yc     [(35-A015Yc)*32+:32];
                end
            else begin
                A324Yc <=  A324Yc;
            end
        end
    end
    generate
        for(A014Yc=0;A014Yc<200;A014Yc=A014Yc+1) begin : A339Yc    
            assign A328Yc     [1599-8*A014Yc-:8]   = A324Yc[8*A014Yc+:8];
        end
    endgenerate
    assign A323Yc = A328Yc     ;
endmodule
module hash_hp_A334Yc (
    input   wire    [1151:0]    A152Yc
   ,input   wire    [3:0]       A111Yc
   ,input   wire    [7:0]       A113Yc
   ,input   wire                A112Yc
   ,output  wire    [1599:0]    A336Yc
);
    wire    [1151:0]            A340Yc    ;
    wire    [1151:0]		A341Yc             ;
    wire    [1151:0]		A342Yc             ;
    wire    [1151:0]		A343Yc             ;
    wire    [1151:0]		A344Yc             ;
    wire    [1599:0]            A326Yc ;
    wire    [1599:0]            A327Yc     ;
    genvar                      A014Yc,A015Yc;
    generate
        for(A014Yc=0;A014Yc<144;A014Yc=A014Yc+1) begin : A334Yc 
            assign A341Yc             [1151-A014Yc*8-:8] 
				           = (A113Yc==A014Yc) ? ((A113Yc==8'd143) ? 8'b10000110 : 8'b00000110) :
                                            ((A113Yc>A014Yc)  ? A152Yc[1151-A014Yc*8-:8] : ((A014Yc==8'd143) ? 8'b10000000 : 8'd0));
            assign A342Yc             [1151-A014Yc*8-:8] 
				           = (A113Yc==A014Yc) ? ((A113Yc==8'd135) ? 8'b10000110 : 8'b00000110) :
                                            ((A113Yc>A014Yc)  ? A152Yc[1151-A014Yc*8-:8] : ((A014Yc==8'd135) ? 8'b10000000 : 8'd0));
            assign A343Yc             [1151-A014Yc*8-:8] 
				           = (A113Yc==A014Yc) ? ((A113Yc==8'd103) ? 8'b10000110 : 8'b00000110) :
                                            ((A113Yc>A014Yc)  ? A152Yc[1151-A014Yc*8-:8] : ((A014Yc==8'd103) ? 8'b10000000 : 8'd0));
            assign A344Yc             [1151-A014Yc*8-:8] 
				           = (A113Yc==A014Yc) ? ((A113Yc==8'd71) ? 8'b10000110 : 8'b00000110) :
                                            ((A113Yc>A014Yc)  ? A152Yc[1151-A014Yc*8-:8] : ((A014Yc==8'd71) ? 8'b10000000 : 8'd0));
        end
    endgenerate
    assign A340Yc     = A111Yc == 4'b1001 ? A341Yc              :
			A111Yc == 4'b1010 ? A342Yc              :
			A111Yc == 4'b1011 ? A343Yc              :
			A344Yc             ;
    assign A326Yc    = A112Yc ?
                        ((A111Yc == 4'b1001) ? {A340Yc    ,          448'd0} :
                        ((A111Yc == 4'b1010) ? {A340Yc    [1151:64], 512'd0} :
                        ((A111Yc == 4'b1011) ? {A340Yc    [1151:320],768'd0} :
                        ((A111Yc == 4'b1100) ? {A340Yc    [1151:576],1024'd0}:
                          1600'd0)))) :
                        ((A111Yc == 4'b1001) ? {A152Yc,          448'd0} :
                        ((A111Yc == 4'b1010) ? {A152Yc[1151:64], 512'd0} :
                        ((A111Yc == 4'b1011) ? {A152Yc[1151:320],768'd0} :
                        ((A111Yc == 4'b1100) ? {A152Yc[1151:576],1024'd0}:
                          1600'd0))));
    generate
        for(A014Yc=0;A014Yc<200;A014Yc=A014Yc+1) begin : A345Yc  
            for(A015Yc=0;A015Yc<8;A015Yc=A015Yc+1) begin : A346Yc
                assign A327Yc     [8*A014Yc+A015Yc] = A326Yc [8*A014Yc+7-A015Yc];
            end
        end
    endgenerate
    assign A336Yc = A327Yc     ;
endmodule
module hash_hp_A255Yc   #(
    parameter   A066Yc          = 1'b1
)(
    input   wire                clk
   ,input   wire                rst_n
   ,input   wire    [1151:0]    A152Yc
   ,input   wire    [35:0]      A153Yc     
   ,input   wire    [1599:0]    A154Yc
   ,input   wire    [49:0]      A155Yc     
   ,input   wire                A112Yc
   ,input   wire    [7:0]       A113Yc
   ,input   wire    [3:0]       A111Yc 
   ,input   wire                A104Yc 
   ,input   wire                A068Yc   
   ,input   wire                A110Yc           
   ,output  wire    [127:0]     A114Yc            
   ,output  wire                A115Yc            
   ,output  wire                A116Yc            
   ,output  wire                A072Yc
   ,output  wire                A070Yc
   ,output  wire    [1599:0]    A224Yc  
);
    wire                        A088Yc;
    wire                        A086Yc;
    wire                        A160Yc;
    wire    [4:0]               A347Yc;
    wire    [1151:0]            A348Yc;
    wire    [35:0]              A349Yc     ;
hash_hp_A321Yc        
    #(
        .A066Yc         (A066Yc     )
    )
    A350Yc          (
        .clk            (clk        ),
        .rst_n          (rst_n      ),
        .A152Yc         (A348Yc     ),
        .A153Yc         (A349Yc     ),
        .A154Yc         (A154Yc     ),
        .A155Yc         (A155Yc     ),
        .A111Yc         (A111Yc     ),
        .A112Yc         (A160Yc      ),
        .A113Yc          (A113Yc      ),
        .A322Yc         (A088Yc     ),
        .A270Yc           (A347Yc       ),
        .A323Yc         (A224Yc     )
    );
    assign A348Yc = A160Yc ? 1151'd0 : A152Yc;
    assign A349Yc      = A160Yc ? {36{1'b1}} : A153Yc     ;
hash_hp_A312Yc        
    #(
       .A066Yc         (A066Yc     )
    )
    A351Yc         (
       .clk             (clk           ),
       .rst_n           (rst_n         ),
       .A104Yc          (A104Yc        ),
       .A113Yc           (A113Yc         ),
       .A068Yc          (A068Yc        ),
       .A110Yc          (A110Yc        ),
       .A112Yc          (A112Yc        ),
       .A111Yc          (A111Yc        ),
       .A114Yc          (A114Yc        ),
       .A115Yc          (A115Yc        ),
       .A116Yc             (A116Yc            ),
       .A070Yc          (A086Yc        ),
       .A072Yc          (A088Yc        ),
       .A121Yc           (A160Yc         ),
       .A313Yc            (A347Yc          )
    );
    assign A072Yc = A088Yc || A160Yc;
    assign A070Yc = A086Yc;
endmodule
module hash_hp_A352Yc  
#(  
    parameter                               A353Yc          = 1
)(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire    [3:0]                   A111Yc
   ,input   wire                            A105Yc   
   ,input   wire                            A354Yc     
   ,input   wire                            A355Yc   
   ,output  wire                            A356Yc          
   ,output  wire                            A357Yc         
   ,output  wire                            A358Yc           
   ,output  wire                            A359Yc         
   ,input   wire                            A112Yc
   ,output  wire                            A360Yc         
   ,input   wire    [127:0]                 A113Yc
   ,output  wire    [127:0]                 A361Yc        
   ,input   wire    [1023:0]                A152Yc
   ,input   wire    [31:0]                  A362Yc    
   ,input   wire                            A104Yc 
   ,input   wire                            A068Yc   
   ,input   wire                            A363Yc            
   ,input   wire                            A364Yc           
   ,input   wire                            A365Yc    
   ,output  wire    [1023:0]                A366Yc         
   ,output  wire    [31:0]                  A367Yc             
   ,input   wire    [511:0]                 A368Yc           
   ,input   wire                            A369Yc         
   ,output  wire                            A370Yc     
   ,output  wire                            A371Yc          
   ,output  wire                            A372Yc     
   ,output  wire                            A373Yc                 
   ,input   wire                            A374Yc      
);
    localparam  [2:0]   A314Yc              = 3'd4;
    localparam  [3:0]   A077Yc              = 4'b0000,
                        A375Yc              = 4'b0001,
                        A376Yc              = 4'b0010,
                        A377Yc              = 4'b0011,
                        A378Yc              = 4'b0100,
                        A379Yc              = 4'b0101,
                        A380Yc              = 4'b0110,
                        A381Yc              = 4'b0111,
                        A382Yc              = 4'b1000,
                        A383Yc              = 4'b1001,
                        A384Yc              = 4'b1010,
                        A385Yc              = 4'b1011,
                        A386Yc              = 4'b1100,
                        A387Yc              = 4'b1101,
                        A081Yc              = 4'b1110;
    reg     [A314Yc       -1:0]         A082Yc       , A083Yc    ;
    reg     [1023:0]                    A388Yc;
    reg     [511:0]                     A389Yc     ;
    reg                                 A390Yc         ;
    reg                                 A320Yc;
    reg                                 A319Yc;
    reg                                 A391Yc     ;
    reg                                 A392Yc          ;
    reg                                 A393Yc         ;
    reg                                 A394Yc         ;
    wire                                A395Yc            ;
    wire                                A396Yc                ;
    wire    [1023:0]                    A397Yc ;
    wire                                A398Yc            ;
    wire                                A399Yc      ;
    wire                                A400Yc         ;
    wire                                A401Yc           ;
    wire    [31:0]                      A402Yc         ;
    wire    [10:0]                      A403Yc     ;
    wire    [9:0]                       A404Yc      ;
    wire                                A405Yc     , A406Yc      ;
    wire                                A407Yc      , A408Yc      ,
                                        A409Yc      , A410Yc      ,
                                        A411Yc      , A412Yc      ;
    wire    [511:0]                     A413Yc        ;
    wire    [1023:0]                    A414Yc         ;
    wire    [1023:0]                    A415Yc      ,
                                        A416Yc    , A417Yc    ;
    wire                                A418Yc                ,
                                        A419Yc           ,
                                        A420Yc           ,
                                        A421Yc                ,
                                        A422Yc                          ;
    integer                             A014Yc;
    always@(posedge clk or negedge rst_n)
    begin
        if (!rst_n)
            A082Yc        <= A077Yc;
        else if (A355Yc   |A365Yc    )
            A082Yc        <=  A077Yc;
        else
            A082Yc        <=  A083Yc    ;
    end
    always@(*)
    begin
        A083Yc      = A082Yc       ;
        case(A082Yc       )
            A077Yc          : A083Yc     = A399Yc       ? (A354Yc      ?
                                ((A112Yc&(A113Yc<={117'd0,A403Yc     }))? A377Yc           : A376Yc         )
                                                           : A382Yc         )
                                                        : A077Yc;
            A375Yc          : A083Yc     = A399Yc       ? (!A396Yc                 ? A376Yc         
                                                        : A377Yc          )
                                                        : A375Yc    ;
            A376Yc          : A083Yc     = A369Yc          ? 
                                (A112Yc ? A377Yc           : A375Yc    )
                                                           : A376Yc         ;
            A377Yc          : A083Yc     = A393Yc          ? (!A374Yc       ? A378Yc         : A377Yc          )
                                                           : A378Yc        ;
            A378Yc          : A083Yc     = A068Yc    ? A077Yc : A379Yc          ;
            A379Yc          : A083Yc     = A369Yc          ? A380Yc          
                                                           : A379Yc          ;
            A380Yc          : A083Yc     = A077Yc;
            A381Yc          : A083Yc     = A399Yc       ? A382Yc          :
                                           ((A068Yc    & !A364Yc           )  ? A077Yc          : A381Yc    );
            A382Yc          : A083Yc     = A369Yc          ?
                                (A112Yc ? A383Yc          : A381Yc    )
                                                           : A382Yc         ;
            A383Yc          : A083Yc     = A384Yc     ;
            A384Yc          : A083Yc     = A068Yc    ? A077Yc : A385Yc          ;
            A385Yc          : A083Yc     = A369Yc          ? A386Yc     
                                                           : A385Yc          ;
            A386Yc          : A083Yc     = A068Yc    ? A077Yc : A387Yc          ;
            A387Yc          : A083Yc     = A369Yc          ? A081Yc
                                                           : A387Yc          ;
            A081Yc          : A083Yc     = !A374Yc       ? A378Yc         :A081Yc;
            default         : ;
        endcase
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A392Yc              <= 1'b0;
            A393Yc              <= 1'b0;
            A320Yc              <= 1'b0;
            A319Yc              <= 1'b0;
            A389Yc              <= 512'd0;
            A394Yc              <= 1'b0;
        end
        else begin
            case(A083Yc    )
                A077Yc          :   begin
                                        A392Yc              <=  1'b0;
                                        A393Yc              <=  1'b0;
                                        A320Yc              <=  1'b0;
                                        A319Yc              <=  1'b0;
                                        A389Yc              <=  512'd0;
                                        A394Yc              <=  A398Yc            ;
                                    end
                A375Yc          :   begin
                                        A392Yc              <=  1'b0;
                                        A393Yc              <=  A395Yc            ;
                                        A319Yc              <=  1'b0;
                                        A394Yc              <=  A398Yc            ;
                                    end
                A376Yc          :   begin
                                        A392Yc              <=  1'b0;
                                        A393Yc              <=  1'b1;
                                        A319Yc              <=  1'b1;
                                    end
                A377Yc          :   begin
                                        A319Yc              <=  1'b1;
                                    end
                A378Yc          :   begin
                                        A394Yc              <=  1'b0;
                                        A393Yc              <=  1'b1;
                                        A392Yc              <=  1'b1;
                                        A320Yc              <=  1'b0;
                                        A319Yc              <=  1'b1;
                                    end
                A379Yc          :   begin
                                        A392Yc              <=  1'b0;
                                        A319Yc              <=  1'b1;
                                    end
                A380Yc          :   begin
                                        A320Yc              <=  1'b1;
                                        A319Yc              <=  1'b0;
                                    end
                A381Yc          :   begin
                                        A319Yc              <=  1'b0;
                                        A394Yc              <=  A398Yc            ;
                                    end
                A382Yc          :   begin
                                        A319Yc              <=  1'b1;
                                    end
                A383Yc          :   begin
                                        A389Yc              <= A368Yc           ;
                                        A319Yc              <=  1'b1;
                                    end
                A384Yc          :   begin
                                        A392Yc              <=  1'b1;
                                        A319Yc              <=  1'b1;
                                    end
                A385Yc          :   begin
                                        A392Yc              <=  1'b0;
                                        A319Yc              <=  1'b1;
                                    end
                A386Yc          :   begin
                                        A392Yc              <=  1'b1;
                                        A319Yc              <=  1'b1;
                                    end
                A387Yc          :   begin
                                        A392Yc              <=  1'b0;
                                        A319Yc              <=  1'b1;
                                    end
                A081Yc          :   begin
                                        A320Yc              <=  1'b1;
                                        A319Yc              <=  1'b0;
                                    end
                default         :   ;
            endcase
        end
    end
    assign A400Yc          = ((A082Yc       ==A383Yc          || A082Yc       ==A384Yc      || A082Yc       ==A386Yc     ) && (A083Yc    ==A077Yc)) || ((A083Yc    ==A081Yc) && A363Yc            );
    assign A401Yc            = A082Yc       ==A379Yc          ;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A388Yc    <= 1024'd0;
        else begin
            if(|A402Yc          & A354Yc      & !A112Yc) begin          
                if(A402Yc          == 32'h80000000) begin               
                    A388Yc <= {A152Yc[1023:992],992'd0};
                end
                else begin
                    for(A014Yc=0;A014Yc<32;A014Yc=A014Yc+1)
                        if(A402Yc         [A014Yc])
                            A388Yc[A014Yc*32+:32]  <=  A152Yc[A014Yc*32+:32];
                end
            end
            else if(A421Yc                ) begin                       
                if((A113Yc == 128'd0)&A394Yc         ) begin             
                    A388Yc    <=  A415Yc      ;
                end
                else if(!A396Yc                ) begin                  
                    if(A393Yc         ) begin                           
                        A388Yc    <=  A415Yc      ;
                    end
                    else begin                                          
                        for(A014Yc=0;A014Yc<32;A014Yc=A014Yc+1)
                            if(A402Yc         [A014Yc])
                                A388Yc[A014Yc*32+:32]  <=  A152Yc[A014Yc*32+:32];
                    end
                end
            end
        end
    end
    assign A395Yc               = A355Yc    ? 1'b0 : 1'b1;
    assign A398Yc               = A355Yc    ? 1'b0 : (A365Yc    &A105Yc    ? 1'b1 : A394Yc         );
    assign A396Yc                 = (A113Yc == {117'd0,A403Yc     });
    assign A399Yc           = A105Yc    & A104Yc ;
    assign A402Yc           = {32{A105Yc   }} & A362Yc    ;
    assign A405Yc           = (A111Yc==4'b0001)||(A111Yc==4'b0000)||(A111Yc==4'b0101)||
                              (A111Yc==4'b0110)||(A111Yc==4'b0010);
    assign A406Yc           = (A111Yc==4'b0011)||(A111Yc==4'b0100)||
                              (A111Yc==4'b0111)||(A111Yc==4'b1000);
    assign A403Yc           = A405Yc      ? 11'd64  : (A406Yc       ? 11'd128 
                                                                    : 11'd0);
    assign A407Yc           = A111Yc==4'b0001;
    assign A408Yc           = A111Yc==4'b0101;
    assign A409Yc           = (A111Yc==4'b0110)||(A111Yc==4'b0111);
    assign A410Yc           = (A111Yc==4'b0010)||(A111Yc==4'b1000)||
                              (A111Yc==4'b0000);
    assign A411Yc           = A111Yc==4'b0011;
    assign A412Yc           = A111Yc==4'b0100;
    assign A404Yc           = A407Yc        ? 10'd128 :
                             (A408Yc        ? 10'd160 :
                             (A409Yc        ? 10'd224 :
                             (A410Yc        ? 10'd256 :
                             (A411Yc        ? 10'd384 :
                             (A412Yc        ? 10'd512 : 10'd0)))));
    assign A413Yc           = 
                        A407Yc       ? {A368Yc           [511:384],384'd0} :
                       (A408Yc       ? {A368Yc           [511:352],352'd0} :
                       (A409Yc       ? {A368Yc           [511:288],288'd0} :
                       (A410Yc       ? {A368Yc           [511:256],256'd0}
                                     : 512'd0)));
    assign A414Yc           =
                        A409Yc       ? {A368Yc           [511:288],800'd0} :
                       (A410Yc       ? {A368Yc           [511:256],768'd0} :
                       (A411Yc       ? {A368Yc           [511:128],640'd0} :
                       (A412Yc       ? {A368Yc           [511:0],512'd0} 
                                     : 1024'd0)));
    assign A415Yc           = A405Yc      ? {A413Yc        ,512'd0}
                                          : (A406Yc       ? A414Yc          
                                                          : 1024'd0);
    assign A416Yc           = A353Yc          ? (A388Yc ^ {128{8'h36}}) 
                                    : {(A388Yc[1023:512] ^ {64{8'h36}}),512'd0};
    assign A417Yc           = A353Yc          ? (A388Yc ^ {128{8'h5C}}) 
                                    : {(A388Yc[1023:512] ^ {64{8'h5C}}),512'd0};
    assign A418Yc                   = ((A083Yc    ==A375Yc    )&(!A396Yc                ))||
                                      (A083Yc    ==A376Yc         )||
                                      (A083Yc    ==A382Yc         );
    assign A419Yc                   = (A082Yc       ==A386Yc     )||
                                      (A082Yc       ==A387Yc          );
    assign A420Yc                   = (A082Yc       ==A378Yc        )||
                                      (A082Yc       ==A384Yc     )||
                                      (A082Yc       ==A386Yc     );
    assign A421Yc                   = (A083Yc    ==A377Yc          );
    assign A422Yc                          
                                    = (A083Yc    ==A380Yc          )||
                                      (A083Yc    ==A383Yc         )||
                                      (A083Yc    ==A386Yc     );
    assign A357Yc          = A400Yc         ;
    assign A358Yc            = A401Yc           ;
    assign A359Yc          = (A082Yc       ==A382Yc         );
    assign A356Yc           = A418Yc                ? A104Yc 
                                                    : (A392Yc           & (!A068Yc   ));
    assign A360Yc           = A419Yc                ? 1'b1
                                                    : A112Yc;
    assign A361Yc           = (A083Yc    ==A382Yc         )&A112Yc ?
        (A113Yc+{117'd0,(A403Yc     )}) : 
        (A419Yc            ? 
        ({117'd0,A403Yc     +(({1'b0,A404Yc      })>>3)}) : A113Yc);
    assign A366Yc          = (A082Yc       ==A378Yc        ) ? A416Yc     :
                             ((A082Yc       ==A384Yc     )   ? A417Yc     :
                             ((A082Yc       ==A386Yc     )   ? 
                                            {A389Yc     ,512'd0}: A152Yc));
    assign A367Yc               = A420Yc            ? 32'hFFFFFFFF
                                                    : A362Yc    ;
    assign A370Yc           = A320Yc;
    assign A371Yc           = (A083Yc    ==A081Yc);
    assign A372Yc           = A319Yc;
    assign A373Yc                   
                            = A369Yc         &!A422Yc                          ;
endmodule
module hash_hp_A423Yc             
#(
    parameter   A219Yc                      = 1'b1
   ,parameter   A220Yc                      = A219Yc    ? 1152 : 1024
   ,parameter   A221Yc                      = A220Yc      /32
   ,parameter   A222Yc                      = A219Yc    ? 1600 : 512
   ,parameter   A223Yc                      = A222Yc        /32
)(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire    [3:0]                   A111Yc
   ,input   wire                            A105Yc   
   ,input   wire                            A424Yc         
   ,input   wire                            A355Yc   
   ,input   wire                            A104Yc  
   ,input   wire                            A425Yc      
   ,output  wire                            A356Yc          
   ,input   wire                            A112Yc
   ,input   wire                            A426Yc     
   ,output  wire                            A360Yc         
   ,input   wire    [127:0]                 A113Yc
   ,input   wire    [127:0]                 A427Yc    
   ,output  wire    [127:0]                 A361Yc        
   ,input   wire    [A220Yc      -1:0]      A152Yc
   ,input   wire    [1023:0]                A428Yc     
   ,input   wire    [A221Yc     -1:0]       A362Yc    
   ,input   wire    [31:0]                  A429Yc         
   ,output  wire    [A220Yc      -1:0]      A366Yc         
   ,output  wire    [A221Yc     -1:0]       A367Yc             
   ,input   wire    [A222Yc        -1:0]    A154Yc
   ,input   wire    [A223Yc       -1:0]     A365Yc    
   ,output  wire    [A222Yc        -1:0]    A430Yc         
   ,input   wire    [A222Yc        -1:0]    A368Yc           
   ,output  wire    [A222Yc        -1:0]    A224Yc  
   ,output  wire    [A223Yc       -1:0]     A431Yc             
   ,output  wire                            A432Yc      
   ,output  wire                            A433Yc
   ,output  wire                            A434Yc    
   ,input   wire                            A435Yc         
   ,input   wire                            A369Yc         
   ,input   wire                            A363Yc            
   ,input   wire                            A436Yc     
   ,input   wire                            A437Yc          
   ,input   wire                            A438Yc                 
   ,input   wire                            A439Yc     
   ,input   wire                            A440Yc   
   ,output  wire                            A441Yc
   ,output  wire                            A442Yc
   ,output  wire                            A443Yc     
   ,output  wire    [127:0]                 A444Yc    
   ,output  wire                            A445Yc 
   ,output  wire    [A221Yc     -1:0]       A446Yc    
   ,output  wire                            A447Yc   
);
    localparam  [1:0]   A314Yc              = 2'd2;
    localparam  [A314Yc       -1:0] A077Yc       = 2'b00,
                                    A448Yc       = 2'b01,
                                    A449Yc       = 2'b10;
    reg     [A314Yc       -1:0]         A082Yc       , A083Yc    ;
    reg                                 A450Yc    ;
    reg                                 A451Yc          ;
    reg                                 A392Yc          ;
    reg                                 A452Yc      ;
    reg                                 A453Yc;
    reg     [127:0]                     A454Yc;
    reg                                 A455Yc     ;
    reg                                 A456Yc     ;
    wire                                A457Yc             ;
    wire                                A458Yc       ;
    wire    [127:0]                     A459Yc  ;
    wire                                A460Yc   ;
    wire                                A461Yc         ;
    wire                                A462Yc        ;
    wire                                A463Yc        ;
    wire                                A464Yc   ;
    wire                                A465Yc ;
    wire    [A221Yc     -1:0]           A466Yc    ;
    wire                                A467Yc, A468Yc;
    wire                                A469Yc      ;
    wire                                A226Yc          ;
    wire                                A470Yc     ;
    wire    [127:0]                     A471Yc    ;
    wire                                A472Yc    ;
    wire    [511:0]                     A473Yc     ;
    wire    [511:0]                     A474Yc    ,     A475Yc     ,
                                        A476Yc    ,     A477Yc       ,
                                        A478Yc       ,  A479Yc       ,
                                        A480Yc       ,  A481Yc           ,
                                        A482Yc           ;
    wire                                A483Yc      ,
                                        A484Yc          ;
    always@(posedge clk or negedge rst_n)
    begin
        if (!rst_n)
            A082Yc        <= A077Yc;
        else
            A082Yc        <=  A083Yc    ;
    end
    always@(*)
    begin
        A083Yc      = A082Yc       ;
        case(A082Yc       )
            A077Yc      : A083Yc     = A469Yc       ? (!A451Yc           ?
                                                    A448Yc       : A449Yc    )
                                                  : A077Yc;
            A448Yc      : A083Yc     = A449Yc    ;
            A449Yc      : A083Yc     = A369Yc          ? A077Yc : A449Yc    ;
            default     : A083Yc     = A077Yc;
        endcase
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A451Yc          <= 1'b0;
            A392Yc          <= 1'b0;
        end
        else begin
            case(A083Yc    )
                A077Yc      :   begin
                                    A392Yc          <=  1'b0;
                                    A451Yc          <=  A457Yc             ;
                                end
                A448Yc      :   begin
                                    A451Yc          <=  1'b1;
                                    A392Yc          <=  1'b1;
                                end
                A449Yc      :   begin
                                    A392Yc          <=  1'b0;
                                end
                default     :   ;
            endcase
        end
    end
    assign A474Yc       = {128'h0123456789ABCDEFFEDCBA9876543210,384'd0};
    assign A475Yc       = {160'h67452301EFCDAB8998BADCFE10325476C3D2E1F0,352'd0};
    assign A476Yc       = {256'h7380166F4914B2B9172442D7DA8A0600A96F30BC163138AAE38DEE4DB0FB0E4E,256'd0};
    assign A477Yc       = {256'hC1059ED8367CD5073070DD17F70E5939FFC00B316858151164F98FA7BEFA4FA4,256'd0};
    assign A478Yc       = {256'h6A09E667BB67AE853C6EF372A54FF53A510E527F9B05688C1F83D9AB5BE0CD19,256'd0};
    assign A479Yc       = 512'hCBBB9D5DC1059ED8629A292A367CD5079159015A3070DD17152FECD8F70E593967332667FFC00B318EB44A8768581511DB0C2E0D64F98FA747B5481DBEFA4FA4;
    assign A480Yc       = 512'h6A09E667F3BCC908BB67AE8584CAA73B3C6EF372FE94F82BA54FF53A5F1D36F1510E527FADE682D19B05688C2B3E6C1F1F83D9ABFB41BD6B5BE0CD19137E2179;
    assign A481Yc           = 512'h8C3D37C819544DA273E1996689DCD4D61DFAB7AE32FF9C82679DD514582F9FCF0F6D2B697BD44DA877E36F7304C489423F9D85A86A1D36C81112E6AD91D692A1;
    assign A482Yc           = 512'h22312194FC2BF72C9F555FA3C84C64C22393B86B6F53B151963877195940EABD96283EE2A88EFFE3BE5E1E25538639922B0199FC2C85B8AA0EB72DDC81C52CA2;
    assign A473Yc          = (A111Yc==4'b0001)        ? A474Yc            : 
                            ((A111Yc==4'b0101)       ? A475Yc            :
                            ((A111Yc==4'b0000)        ? A476Yc            :
                            ((A111Yc==4'b0110)     ? A477Yc            :
                            ((A111Yc==4'b0010)     ? A478Yc            :
                            ((A111Yc==4'b0011)     ? A479Yc            :
                            ((A111Yc==4'b0100)     ? A480Yc            :
                            ((A111Yc==4'b0111  ? A481Yc            :
                            ((A111Yc==4'b1000  ? A482Yc            : 
                                                       512'd0))))))))));
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A450Yc      <= 1'b0;
            A454Yc       <= 128'd0;
            A453Yc      <= 1'b0;
        end
        else begin
            A450Yc      <=  A458Yc       ;
            A454Yc       <=  A459Yc  ;
            A453Yc      <=  A460Yc   ;
        end
    end
    assign A457Yc                 = A453Yc|A437Yc          |A464Yc    ? 1'b0 : A451Yc          ;
    assign A458Yc       = ((|A365Yc    )&A467Yc) ? 1'b1 : (A484Yc           ? 1'b0 : A450Yc    );
    assign A459Yc       = A465Yc  ? A113Yc>>3 : (A483Yc       ? 128'd0 : A454Yc);
    assign A460Yc       = A465Yc  ? A112Yc   : (A483Yc       ? 1'b0   : A453Yc);
    assign A465Yc               = A104Yc  & A468Yc;
    assign A466Yc               = A362Yc     & {A221Yc     {A468Yc}};
    assign A469Yc               = A105Yc    ? A425Yc       : A465Yc ;
    assign A483Yc               = (A082Yc       ==A077Yc);
    assign A484Yc               = (A082Yc       ==A449Yc    );
    assign A472Yc               = ((|A365Yc    ) | (A450Yc    ))&A467Yc;
    assign A226Yc               = A105Yc    ? A425Yc       : 
                                ((A465Yc  & A451Yc          ) ? A465Yc 
                                :  A392Yc          );
    assign A356Yc               = A226Yc          ;
    assign A360Yc               = A105Yc        ? A426Yc     
                                                : A470Yc     ;
    assign A361Yc               = A105Yc        ? A427Yc    
                                                : A471Yc    ;
    assign A366Yc               = A105Yc        ? A428Yc      << (A220Yc      -1024) : A152Yc;
    assign A367Yc               = A105Yc        ? A429Yc          <<  (A221Yc     -32) 
                                                : A466Yc    ;
    assign A430Yc               = !A472Yc       ? A473Yc      << (A222Yc        -512) : A154Yc;
    assign A431Yc               = !A472Yc       ? (A451Yc           ? {A223Yc       {1'b0}} 
                                                                : {A223Yc       {A469Yc      }})
                                                : A365Yc    ;
    assign A470Yc               = A465Yc  ? A112Yc : A453Yc;
    assign A471Yc               = A465Yc  ? A113Yc>>3 : A454Yc;
    assign A443Yc               = A470Yc     ;
    assign A444Yc               = A470Yc      ? A471Yc     : 128'd0;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A452Yc          <= 1'b0;
            A455Yc          <= 1'b0;
            A456Yc          <= 1'b0;
        end
        else begin
            A452Yc          <=  A461Yc         ;
            A455Yc          <=  A462Yc        ;
            A456Yc          <=  A463Yc        ;
        end
    end
    assign A461Yc               = (A452Yc      &A440Yc   ) ? 1'b0 
                                : (A424Yc         & 
                                (A105Yc    ? A438Yc                  
                                : A369Yc         ) ? (A363Yc             ? 1'b0 : 1'b1)
                                : (A105Yc    ? (A437Yc           ? (A363Yc             ? 1'b0 : 1'b1) 
                                : A452Yc      )
                                : ((A369Yc         &A453Yc) ? (A363Yc             ? 1'b0 : 1'b1) 
                                : A452Yc      )));
    assign A462Yc               = (A455Yc     &A440Yc   ) ? 1'b0
                                    : (A437Yc           ? 1'b1 : A455Yc     );
    assign A463Yc               = (A456Yc     &A440Yc   ) ? 1'b0
                                : ((A369Yc         &A453Yc) ? 1'b1 : A456Yc     ); 
    assign A224Yc               = A452Yc       ? A368Yc            : {A222Yc        {1'b0}};
    assign A432Yc               = A452Yc      ;
    assign A433Yc               = A105Yc    ? A455Yc      : A456Yc     ;
    assign A467Yc                = A468Yc;
    assign A468Yc                = A105Yc    ? !(A439Yc     | A436Yc     |A452Yc      )
                            : !(A435Yc         |A392Yc          |A452Yc      );
    assign A441Yc                = A467Yc;
    assign A442Yc                = A468Yc;
    assign A445Yc               = A465Yc ;
    assign A446Yc               = A466Yc    ;
    assign A464Yc               = A355Yc   &A467Yc;
    assign A447Yc               = A464Yc   ;
    assign A434Yc               = (|A365Yc    )&A467Yc;
endmodule
module hash_hp_A485Yc              
#(
    parameter   A219Yc                      = 1'b1
   ,parameter   A220Yc                      = A219Yc    ? 1152 : 1024
   ,parameter   A221Yc                      = A220Yc      /32
   ,parameter   A222Yc                      = A219Yc    ? 1600 : 512
   ,parameter   A223Yc                      = A222Yc        /32
)(
    input   wire    [3:0]                   A111Yc
   ,input   wire                            A105Yc   
   ,input   wire                            A424Yc         
   ,input   wire    [2:0]                   A486Yc  
   ,input   wire                            A355Yc   
   ,input   wire    [A222Yc        -1:0]    A154Yc
   ,input   wire    [A223Yc       -1:0]     A365Yc    
   ,input   wire                            A104Yc 
   ,input   wire                            A112Yc
   ,input   wire    [127:0]                 A113Yc
   ,input   wire    [A220Yc      -1:0]      A152Yc
   ,input   wire    [A221Yc     -1:0]       A362Yc    
   ,input   wire                            A354Yc     
   ,input   wire                            A440Yc   
   ,output  wire                            A442Yc
   ,output  wire    [3:0]                   A487Yc
   ,output  wire                            A488Yc   
   ,output  wire                            A489Yc         
   ,output  wire    [2:0]                   A490Yc  
   ,output  wire                            A441Yc
   ,output  wire    [A222Yc        -1:0]    A156Yc
   ,output  wire    [A223Yc       -1:0]     A434Yc    
   ,output  wire                            A445Yc 
   ,output  wire                            A433Yc
   ,output  wire    [127:0]                 A491Yc
   ,output  wire    [A220Yc      -1:0]      A336Yc
   ,output  wire    [A221Yc     -1:0]       A446Yc    
   ,output  wire                            A492Yc     
);
    assign A442Yc            = A440Yc   ;
    assign A487Yc           = A111Yc;
    assign A488Yc           = A105Yc   ;
    assign A489Yc           = A424Yc         ;
    assign A490Yc           = A486Yc  ;
    assign A441Yc            = A355Yc   ;
    assign A156Yc           = A154Yc;
    assign A434Yc           = A365Yc    ;
    assign A445Yc           = A104Yc ;
    assign A433Yc           = A112Yc;
    assign A491Yc            = A113Yc;
    assign A336Yc           = A152Yc;
    assign A446Yc           = A362Yc    ;
    assign A492Yc           = A354Yc     ;
endmodule
module hash_hp_A493Yc               
(
    input   wire                        A440Yc   
   ,input   wire                        A374Yc      
   ,input   wire                        A112Yc
   ,output  wire                        A432Yc      
   ,output  wire                        A433Yc
   ,output  wire                        A442Yc
);
   assign A432Yc        = A374Yc      ;
   assign A433Yc        = A112Yc;
   assign A442Yc         = A440Yc   ;
endmodule
module hash_hp_A494Yc                 
(
    input   wire                            clk                 
   ,input   wire                            rst_n
   ,input   wire    [3:0]                   A111Yc          
   ,input   wire                            A105Yc          
   ,input   wire                            A424Yc          
   ,input   wire    [2:0]                   A486Yc   
   ,input   wire                            A495Yc        
   ,input   wire                            A355Yc   
   ,output  wire    [3:0]                   A487Yc          
   ,output  wire                            A488Yc          
   ,output  wire                            A489Yc          
   ,output  wire    [2:0]                   A490Yc   
);
    wire                        A464Yc              ;
    reg     [3:0]               A496Yc              ;
    reg                         A497Yc              ;
    reg                         A498Yc              ;
    reg     [2:0]               A499Yc              ;
    wire    [3:0]               A500Yc              ;
    wire                        A501Yc              ;
    wire                        A502Yc              ;
    wire    [2:0]               A503Yc              ;
    assign A464Yc    = A355Yc    && A495Yc;
    assign A500Yc    = A464Yc    ? A111Yc : 
                                   A496Yc;
    assign A501Yc       = A464Yc    ? A105Yc    : 
                                      A497Yc   ;
    assign A502Yc             = A464Yc    ? A424Yc          : 
                                            A498Yc         ;
    assign A503Yc      = A464Yc    ? A486Yc   : 
                                     A499Yc  ;
    always@(posedge clk or negedge rst_n)
    begin
        if (!rst_n) begin 
            A496Yc          <=  4'd0;
            A497Yc          <=  1'd0;
            A498Yc          <=  1'd0;
            A499Yc          <=  3'd0;
        end
        else begin
            A496Yc          <=   A500Yc             ;
            A497Yc          <=   A501Yc             ;
            A498Yc          <=   A502Yc             ;
            A499Yc          <=   A503Yc             ;
        end
    end
    assign A487Yc          = A496Yc         ; 
    assign A488Yc          = A497Yc         ; 
    assign A489Yc          = A498Yc         ; 
    assign A490Yc          = A499Yc         ; 
endmodule
module hash_hp_A504Yc             
#(
    parameter   A505Yc                      = 1'b1           
   ,parameter   A061Yc                      = 1'b1           
   ,parameter   A062Yc                      = 1'b1           
   ,parameter   A063Yc                      = 1'b1           
   ,parameter   A064Yc                      = 1'b1           
   ,parameter   A065Yc                      = 1'b1
   ,parameter   A219Yc                      = 1'b1
   ,parameter   A066Yc                      = 1'b1
   ,parameter   A220Yc                      = A219Yc    ? 1152 : 1024
   ,parameter   A221Yc                      = A220Yc      /32
   ,parameter   A222Yc                      = A219Yc    ? 1600 : 512
   ,parameter   A223Yc                      = A222Yc        /32
   ,parameter   A506Yc                      = 16'h00CD
   ,parameter   A507Yc                       = 4'h1
   ,parameter   A508Yc                       = 4'h1
)(
    input   wire                            clk             
   ,input   wire                            rst_n           
   ,input   wire    [3:0]                   A111Yc          
   ,input   wire                            A105Yc          
   ,input   wire                            A424Yc          
   ,input   wire    [2:0]                   A486Yc   
   ,output  wire                            A441Yc        
   ,input   wire                            A355Yc   
   ,input   wire    [A222Yc        -1:0]    A154Yc   
   ,input   wire    [A223Yc       -1:0]     A365Yc    
   ,input   wire                            A104Yc 
   ,input   wire                            A068Yc   
   ,input   wire                            A364Yc           
   ,input   wire                            A110Yc 
   ,input   wire                            A109Yc          
   ,input   wire                            A112Yc
   ,input   wire    [127:0]                 A113Yc 
   ,output  wire                            A442Yc 
   ,input   wire    [A220Yc      -1:0]      A152Yc
   ,output  wire    [A222Yc        -1:0]    A224Yc    
   ,input   wire    [A221Yc     -1:0]       A362Yc    
   ,input   wire                            A354Yc     
   ,input   wire                            A440Yc            
   ,output  wire                            A433Yc            
   ,output  wire                            A432Yc            
   ,output  wire    [3:0]                   A509Yc
   ,output  wire    [3:0]                   A510Yc
   ,output  wire    [127:0]                 A114Yc            
   ,output  wire                            A115Yc            
   ,output  wire                            A116Yc               
);
    localparam          A353Yc          = A064Yc      ? 1'b1 : 1'b0;
    wire    [3:0]                   A511Yc              ;
    wire                            A512Yc              ;
    wire                            A513Yc              ;
    wire    [2:0]                   A514Yc              ;
    wire                            A515Yc              ;
    wire                            A226Yc              ;
    wire                            A516Yc              ;
    wire    [127:0]                 A517Yc              ;
    wire                            A233Yc              ;
    wire                            A234Yc              ;
    wire    [A220Yc      -1:0]      A235Yc              ;
    wire    [A222Yc        -1:0]    A236Yc              ;
    wire    [A222Yc        -1:0]    A237Yc              ;
    wire    [A221Yc     -1:0]       A518Yc              ;
    wire    [A223Yc       -1:0]     A519Yc              ;
    wire                            A464Yc              ;
    wire                            A399Yc              ;
    wire                            A400Yc              ;
    wire                            A401Yc              ;
    wire                            A136Yc              ;
    wire                            A520Yc              ;
    wire                            A521Yc              ;
    wire    [127:0]                 A522Yc              ;
    wire    [1023:0]                A523Yc              ;
    wire    [31:0]                  A402Yc              ;
    wire                            A524Yc              ;
    wire                            A525Yc              ;
    wire                            A526Yc              ;
    wire                            A470Yc              ;
    wire    [127:0]                 A471Yc              ;
    wire                            A465Yc              ;
    wire    [A221Yc     -1:0]       A466Yc              ;
    wire                            A527Yc                  ;
    wire                            A528Yc              ;
    wire    [3:0]                   A529Yc              ;
    wire                            A530Yc              ;
    wire                            A531Yc                  ;
    wire    [2:0]                   A532Yc              ;
    wire                            A533Yc              ; 
    wire    [A222Yc        -1:0]    A534Yc              ;
    wire    [A223Yc       -1:0]     A535Yc              ; 
    wire                            A536Yc              ; 
    wire                            A537Yc              ; 
    wire    [127:0]                 A538Yc              ; 
    wire    [A220Yc      -1:0]      A539Yc              ; 
    wire    [A221Yc     -1:0 ]      A540Yc              ; 
    wire                            A541Yc              ; 
    wire                            A542Yc              ;
    wire                            A543Yc              ;
    wire                            A544Yc              ;
    wire                            A472Yc              ;
hash_hp_A485Yc              
    #(
       .A219Yc             (A219Yc             )
    )
    A545Yc        
    (
      .A111Yc             (A111Yc             )
   ,  .A105Yc             (A105Yc             )
   ,  .A424Yc             (A424Yc             )
   ,  .A486Yc             (A486Yc             )
   ,  .A355Yc             (A355Yc             )
   ,  .A154Yc             (A154Yc             )
   ,  .A365Yc             (A365Yc             )
   ,  .A104Yc             (A104Yc             )
   ,  .A112Yc             (A112Yc             )
   ,  .A113Yc              (A113Yc              )
   ,  .A152Yc             (A152Yc             )
   ,  .A362Yc             (A362Yc             )
   ,  .A354Yc             (A354Yc             )
   ,  .A440Yc             (A440Yc             )
   ,  .A442Yc              (A528Yc             )
   ,  .A487Yc             (A529Yc             )
   ,  .A488Yc             (A530Yc             )
   ,  .A489Yc             (A531Yc                 )
   ,  .A490Yc             (A532Yc             )
   ,  .A441Yc              (A533Yc             )
   ,  .A156Yc             (A534Yc             )
   ,  .A434Yc             (A535Yc             )
   ,  .A445Yc             (A536Yc             )
   ,  .A433Yc             (A537Yc             )
   ,  .A491Yc              (A538Yc             )
   ,  .A336Yc             (A539Yc             )
   ,  .A446Yc             (A540Yc             )
   ,  .A492Yc             (A541Yc             )
    );
hash_hp_A493Yc               
    A546Yc         
    (
      .A440Yc             (A542Yc             )
   ,  .A374Yc             (A543Yc              )
   ,  .A112Yc             (A544Yc             )
   ,  .A432Yc             (A432Yc             )
   ,  .A433Yc             (A433Yc             )
   ,  .A442Yc              (A442Yc              )
   );
hash_hp_A494Yc                 
    A547Yc    
    (
      .clk                (clk                )
   ,  .rst_n              (rst_n              )
   ,  .A111Yc             (A529Yc             )
   ,  .A105Yc             (A530Yc             )
   ,  .A424Yc             (A531Yc                 )
   ,  .A486Yc             (A532Yc             )
   ,  .A495Yc              (A515Yc  )
   ,  .A355Yc             (A533Yc             )
   ,  .A487Yc             (A511Yc             )
   ,  .A488Yc             (A512Yc             )
   ,  .A489Yc             (A513Yc             )
   ,  .A490Yc             (A514Yc             )
    );
   generate
   if(A505Yc   ) begin : A548Yc        
hash_hp_A423Yc              
        #(
           .A219Yc             (A219Yc             )
        )
        A549Yc               (
          .clk                (clk                )
       ,  .rst_n              (rst_n              )
       ,  .A111Yc             (A511Yc             )
       ,  .A105Yc             (A512Yc             )
       ,  .A424Yc             (A513Yc             )
       ,  .A355Yc             (A533Yc             )
       ,  .A104Yc             (A536Yc             )
       ,  .A425Yc             (A399Yc             )
       ,  .A356Yc             (A226Yc             )
       ,  .A112Yc             (A537Yc             )
       ,  .A426Yc             (A521Yc             )
       ,  .A360Yc             (A516Yc             )
       ,  .A113Yc              (A538Yc             )
       ,  .A427Yc             (A522Yc             )
       ,  .A361Yc             (A517Yc             )
       ,  .A152Yc             (A539Yc             )
       ,  .A428Yc             (A523Yc             )
       ,  .A362Yc             (A540Yc             )
       ,  .A429Yc             (A402Yc             )
       ,  .A366Yc             (A235Yc             )
       ,  .A367Yc             (A518Yc             )
       ,  .A154Yc             (A534Yc                 )
       ,  .A365Yc             (A535Yc                 )
       ,  .A430Yc             (A236Yc             )
       ,  .A368Yc             (A237Yc             )
       ,  .A224Yc             (A224Yc             )
       ,  .A431Yc             (A519Yc             )
       ,  .A432Yc             (A543Yc              )
       ,  .A433Yc             (A544Yc             )
       ,  .A435Yc             (A233Yc             )
       ,  .A369Yc             (A234Yc             )
       ,  .A363Yc             (A136Yc             )
       ,  .A436Yc             (A524Yc             )
       ,  .A437Yc             (A525Yc             )
       ,  .A438Yc                 (A527Yc                 )
       ,  .A439Yc             (A526Yc             )
       ,  .A440Yc             (A528Yc             )
       ,  .A441Yc              (A515Yc             )
       ,  .A442Yc              (A542Yc             )
       ,  .A443Yc             (A470Yc             )
       ,  .A444Yc             (A471Yc             )
       ,  .A445Yc             (A465Yc             )
       ,  .A446Yc             (A466Yc             )
       ,  .A447Yc             (A464Yc             )
       ,  .A434Yc             (A472Yc             )
        );
hash_hp_A352Yc  
        #(
          .A353Yc             (A353Yc                 )
        )
        A550Yc    
        (
          .clk                (clk                    )
       ,  .rst_n              (rst_n                  )
       ,  .A111Yc             (A511Yc                 )
       ,  .A105Yc             (A512Yc                 )
       ,  .A354Yc             (A541Yc                 )
       ,  .A355Yc             (A464Yc                 )
       ,  .A356Yc             (A399Yc                 )
       ,  .A357Yc             (A400Yc                 )
       ,  .A358Yc             (A401Yc                 )
       ,  .A359Yc             (A520Yc                 )
       ,  .A112Yc             (A470Yc                 )
       ,  .A068Yc             (A068Yc                 )
       ,  .A363Yc             (A136Yc                 )
       ,  .A364Yc             (A364Yc                 )
       ,  .A360Yc             (A521Yc                 )
       ,  .A113Yc              (A471Yc                 )
       ,  .A361Yc             (A522Yc                 )
       ,  .A104Yc             (A465Yc                 )
       ,  .A152Yc             (A539Yc        [A220Yc      -1-:1024])
       ,  .A362Yc             (A466Yc    [A221Yc     -1-:32])
       ,  .A365Yc             (A472Yc                 )
       ,  .A366Yc             (A523Yc                 )
       ,  .A368Yc             (A237Yc           [A222Yc        -1-:512])
       ,  .A367Yc             (A402Yc                 )
       ,  .A369Yc             (A234Yc                 )
       ,  .A370Yc             (A524Yc                 )
       ,  .A371Yc             (A525Yc                 )
       ,  .A372Yc             (A526Yc                 )
       ,  .A373Yc                 (A527Yc                 )
       ,  .A374Yc             (A543Yc                 )
        );
    end
    else begin : A551Yc           
        assign A527Yc                  = 1'b0;
hash_hp_A423Yc              
        #(
           .A219Yc             (A219Yc             )
        )
        A549Yc               (
          .clk                (clk                )
       ,  .rst_n              (rst_n              )
       ,  .A111Yc             (A511Yc             )
       ,  .A105Yc             (A512Yc             )
       ,  .A424Yc             (A513Yc             )
       ,  .A355Yc             (A533Yc             )
       ,  .A104Yc             (A536Yc             )
       ,  .A425Yc             (A399Yc             )
       ,  .A356Yc             (A226Yc             )
       ,  .A112Yc             (A537Yc             )
       ,  .A426Yc             (A521Yc             )
       ,  .A360Yc             (A516Yc             )
       ,  .A113Yc              (A538Yc             )
       ,  .A427Yc             (A522Yc             )
       ,  .A361Yc             (A517Yc             )
       ,  .A152Yc             (A539Yc             )
       ,  .A428Yc             (A523Yc             )
       ,  .A362Yc             (A540Yc             )
       ,  .A429Yc             (A402Yc             )
       ,  .A366Yc             (A235Yc                 )
       ,  .A367Yc             (A518Yc                 )
       ,  .A154Yc             (A534Yc                 )
       ,  .A365Yc             (A535Yc                 )
       ,  .A430Yc             (A236Yc             )
       ,  .A368Yc             (A237Yc             )
       ,  .A224Yc             (A224Yc             )
       ,  .A431Yc             (A519Yc             )
       ,  .A432Yc             (A543Yc              )
       ,  .A433Yc             (A544Yc             )
       ,  .A435Yc             (A233Yc             )
       ,  .A369Yc             (A234Yc             )
       ,  .A363Yc             (A136Yc             )
       ,  .A436Yc             (A524Yc             )
       ,  .A437Yc             (A525Yc             )
       ,  .A438Yc                 (A527Yc                 )
       ,  .A439Yc             (A526Yc             )
       ,  .A440Yc             (A528Yc             )
       ,  .A441Yc              (A515Yc             )
       ,  .A442Yc              (A542Yc             )
       ,  .A443Yc             (A470Yc             )
       ,  .A444Yc             (A471Yc             )
       ,  .A445Yc             (A465Yc             )
       ,  .A446Yc             (A466Yc             )
       ,  .A447Yc             (A464Yc             )
       ,  .A434Yc             (A472Yc             )
        );
        assign A400Yc           = 1'b0;
        assign A401Yc            = 1'b0;
        assign A520Yc           = 1'b0;
        assign A399Yc           = 1'b0;
        assign A521Yc           = 1'b0;
        assign A522Yc           = 128'd0;
        assign A523Yc           = 1024'd0;
        assign A402Yc           = 1'b0;
        assign A524Yc           = 1'b0;
        assign A525Yc           = 1'b0;
        assign A526Yc           = 1'b0;
    end
    endgenerate
hash_hp_A218Yc 
    #(
      .A061Yc             (A061Yc                 )
   ,  .A062Yc             (A062Yc                 )
   ,  .A063Yc             (A063Yc                 )
   ,  .A064Yc             (A064Yc                 )
   ,  .A065Yc             (A065Yc                 )
   ,  .A219Yc             (A219Yc                 )
   ,  .A066Yc             (A066Yc                 )
    )
    A552Yc   
    (
      .clk                (clk                    )
   ,  .rst_n              (rst_n                  )
   ,  .A104Yc             (A226Yc                 )
   ,  .A105Yc             (A512Yc                 )
   ,  .A106Yc             (A400Yc                 )
   ,  .A107Yc             (A401Yc                 )
   ,  .A108Yc             (A520Yc                 )
   ,  .A068Yc             (A068Yc                 )
   ,  .A109Yc             (A109Yc                 )
   ,  .A110Yc             (A110Yc                 )
   ,  .A111Yc             (A511Yc                 )
   ,  .A112Yc             (A516Yc                 )
   ,  .A113Yc              (A517Yc                 )
   ,  .A116Yc             (A136Yc                 )
   ,  .A114Yc             (A114Yc                 )
   ,  .A115Yc             (A115Yc                 )
   ,  .A072Yc             (A233Yc                 )
   ,  .A070Yc             (A234Yc                 )
   ,  .A152Yc             (A235Yc                 )
   ,  .A153Yc             (A518Yc                 )
   ,  .A154Yc             (A236Yc                 )
   ,  .A155Yc             (A519Yc                 )
   ,  .A224Yc             (A237Yc                 )
    );
    assign A441Yc = A515Yc  ;
    assign A509Yc = A507Yc;
    assign A510Yc = A508Yc;
    assign A116Yc             = A136Yc            ;
endmodule
module hash_hp_A553Yc       (
    input    wire  [6:0]    A001Yc          ,
    input    wire  [511:0]  A002Yc          ,
    input    wire  [255:0]  A003Yc          ,
    output   wire  [255:0]  A004Yc          ,
    output   wire  [31:0]   A045Yc           ,
    output   wire           A046Yc         
);
    wire  [31:0]     A554Yc        ; 
    wire  [31:0]     A555Yc        ; 
    wire  [31:0]     A048Yc      ; 
    wire  [31:0]     A049Yc      ; 
    wire  [31:0]     A005Yc[0:15];
    wire  [31:0]     A006Yc[0:7] ;
    wire  [31:0]     A007Yc[0:7] ;
    genvar           A014Yc           ;
    generate
    for(A014Yc=0;A014Yc<16;A014Yc=A014Yc+1) begin : A016Yc    
        assign A005Yc[A014Yc] = A002Yc[(15-A014Yc)*32+:32];
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<8;A014Yc=A014Yc+1) begin : A018Yc      
        assign A006Yc[A014Yc] = A003Yc[(7-A014Yc)*32+:32];
    end
    endgenerate
    assign A048Yc     = 
            A556Yc (A005Yc[14]) + A005Yc[9] + A557Yc (A005Yc[1]) + A005Yc[0];
    assign A049Yc     = A005Yc[A001Yc];
    assign A007Yc[0] = A554Yc + A555Yc;
    assign A007Yc[1] = A006Yc[0];
    assign A007Yc[2] = A006Yc[1];
    assign A007Yc[3] = A006Yc[2];
    assign A007Yc[4] = A006Yc[3] + A554Yc;
    assign A007Yc[5] = A006Yc[4];
    assign A007Yc[6] = A006Yc[5];
    assign A007Yc[7] = A006Yc[6];
    assign A554Yc = 
        (A001Yc>='d16) ? 
            A006Yc[7] + A558Yc       (A006Yc[4]) + 
            A051Yc(A006Yc[4],A006Yc[5],A006Yc[6]) + A052Yc(A001Yc[5:0]) + A048Yc     : 
            A006Yc[7] + A558Yc       (A006Yc[4]) + 
            A051Yc(A006Yc[4],A006Yc[5],A006Yc[6]) + A052Yc(A001Yc[5:0]) + A049Yc     ; 
    assign A555Yc = A559Yc       (A006Yc[0]) + A054Yc(A006Yc[0],A006Yc[1],A006Yc[2]);
    generate
    for (A014Yc=0;A014Yc<8;A014Yc=A014Yc+1) begin : A031Yc      
        assign A004Yc[(7-A014Yc)*32+:32] = A007Yc[A014Yc];
    end
    endgenerate
    assign A045Yc = A048Yc    ;
    assign A046Yc  = A001Yc >= 'd16;
    function [31:0] A051Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        begin
            A051Yc = (A055Yc & A056Yc) ^ (~A055Yc & A057Yc);
        end
    endfunction
    function [31:0] A054Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        begin
            A054Yc = (A055Yc & A056Yc) ^ (A055Yc & A057Yc) ^ (A056Yc & A057Yc);
        end
    endfunction
    function [31:0] A557Yc ;
        input [31:0]   A055Yc;
        begin
            A557Yc  = (A055Yc>>7 | A055Yc<<25) ^ (A055Yc>>18 | A055Yc<<14) ^ (A055Yc>>3);
        end
    endfunction
    function [31:0] A556Yc ;
        input [31:0]   A055Yc;
        begin
            A556Yc  = (A055Yc>>17 | A055Yc<<15) ^ (A055Yc>>19 | A055Yc<<13) ^ (A055Yc>>10);
        end
    endfunction
    function [31:0] A559Yc       ;
        input [31:0]   A055Yc;
        begin
            A559Yc        = (A055Yc>>2 | A055Yc<<30) ^ (A055Yc>>13 | A055Yc<<19) ^ (A055Yc>>22 | A055Yc<<10);
        end
    endfunction
    function [31:0] A558Yc       ;
        input [31:0]   A055Yc;
        begin
            A558Yc        = (A055Yc>>6 | A055Yc<<26) ^ (A055Yc>>11 | A055Yc<<21) ^ (A055Yc>>25 | A055Yc<<7);
        end
    endfunction
    function [31:0] A052Yc;
        input [5:0] A055Yc;
        reg [31:0] A059Yc [0:63];
        begin
            {A059Yc[0 ],A059Yc[1 ],A059Yc[2 ],A059Yc[3 ],A059Yc[4 ],A059Yc[5 ],A059Yc[6 ],A059Yc[7 ],
             A059Yc[8 ],A059Yc[9 ],A059Yc[10],A059Yc[11],A059Yc[12],A059Yc[13],A059Yc[14],A059Yc[15],
             A059Yc[16],A059Yc[17],A059Yc[18],A059Yc[19],A059Yc[20],A059Yc[21],A059Yc[22],A059Yc[23],
             A059Yc[24],A059Yc[25],A059Yc[26],A059Yc[27],A059Yc[28],A059Yc[29],A059Yc[30],A059Yc[31],
             A059Yc[32],A059Yc[33],A059Yc[34],A059Yc[35],A059Yc[36],A059Yc[37],A059Yc[38],A059Yc[39],
             A059Yc[40],A059Yc[41],A059Yc[42],A059Yc[43],A059Yc[44],A059Yc[45],A059Yc[46],A059Yc[47],
             A059Yc[48],A059Yc[49],A059Yc[50],A059Yc[51],A059Yc[52],A059Yc[53],A059Yc[54],A059Yc[55],
             A059Yc[56],A059Yc[57],A059Yc[58],A059Yc[59],A059Yc[60],A059Yc[61],A059Yc[62],A059Yc[63]}
            = 
            {32'h428a2f98, 32'h71374491, 32'hb5c0fbcf, 32'he9b5dba5, 
             32'h3956c25b, 32'h59f111f1, 32'h923f82a4, 32'hab1c5ed5,
             32'hd807aa98, 32'h12835b01, 32'h243185be, 32'h550c7dc3, 
             32'h72be5d74, 32'h80deb1fe, 32'h9bdc06a7, 32'hc19bf174,
             32'he49b69c1, 32'hefbe4786, 32'h0fc19dc6, 32'h240ca1cc, 
             32'h2de92c6f, 32'h4a7484aa, 32'h5cb0a9dc, 32'h76f988da,
             32'h983e5152, 32'ha831c66d, 32'hb00327c8, 32'hbf597fc7, 
             32'hc6e00bf3, 32'hd5a79147, 32'h06ca6351, 32'h14292967,
             32'h27b70a85, 32'h2e1b2138, 32'h4d2c6dfc, 32'h53380d13, 
             32'h650a7354, 32'h766a0abb, 32'h81c2c92e, 32'h92722c85,
             32'ha2bfe8a1, 32'ha81a664b, 32'hc24b8b70, 32'hc76c51a3, 
             32'hd192e819, 32'hd6990624, 32'hf40e3585, 32'h106aa070,
             32'h19a4c116, 32'h1e376c08, 32'h2748774c, 32'h34b0bcb5, 
             32'h391c0cb3, 32'h4ed8aa4a, 32'h5b9cca4f, 32'h682e6ff3,
             32'h748f82ee, 32'h78a5636f, 32'h84c87814, 32'h8cc70208, 
             32'h90befffa, 32'ha4506ceb, 32'hbef9a3f7, 32'hc67178f2};
            A052Yc = A059Yc[A055Yc];
        end
    endfunction
endmodule
module hash_hp_A560Yc       (
    input    wire  [6:0]    A001Yc          ,
    input    wire  [1023:0] A002Yc          ,
    input    wire  [511:0]  A003Yc          ,
    output   wire  [511:0]  A004Yc          ,
    output   wire  [63:0]   A045Yc           ,
    output   wire           A046Yc         
);
    wire  [63:0]     A554Yc        ; 
    wire  [63:0]     A555Yc        ; 
    wire  [63:0]     A048Yc      ; 
    wire  [63:0]     A049Yc      ; 
    wire  [63:0]     A005Yc[0:15];
    wire  [63:0]     A006Yc[0:7] ;
    wire  [63:0]     A007Yc[0:7] ;
    genvar           A014Yc           ;
    generate
    for(A014Yc=0;A014Yc<16;A014Yc=A014Yc+1) begin : A016Yc    
        assign A005Yc[A014Yc] = A002Yc[(15-A014Yc)*64+:64];
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<8;A014Yc=A014Yc+1) begin : A018Yc      
        assign A006Yc[A014Yc] = A003Yc[(7-A014Yc)*64+:64];
    end
    endgenerate
    assign A048Yc     = 
        A556Yc (A005Yc[14]) + A005Yc[9] + A557Yc (A005Yc[1]) + A005Yc[0];
    assign A049Yc     = A005Yc[A001Yc];
    assign A007Yc[0] = A554Yc + A555Yc;
    assign A007Yc[1] = A006Yc[0];
    assign A007Yc[2] = A006Yc[1];
    assign A007Yc[3] = A006Yc[2];
    assign A007Yc[4] = A006Yc[3] + A554Yc;
    assign A007Yc[5] = A006Yc[4];
    assign A007Yc[6] = A006Yc[5];
    assign A007Yc[7] = A006Yc[6];
    assign A554Yc = 
        (A001Yc>='d16) ? 
            A006Yc[7] + A558Yc       (A006Yc[4]) + 
            A051Yc(A006Yc[4],A006Yc[5],A006Yc[6]) + A052Yc(A001Yc) + A048Yc     : 
            A006Yc[7] + A558Yc       (A006Yc[4]) + 
            A051Yc(A006Yc[4],A006Yc[5],A006Yc[6]) + A052Yc(A001Yc) + A049Yc     ; 
    assign A555Yc = A559Yc       (A006Yc[0]) + A054Yc(A006Yc[0],A006Yc[1],A006Yc[2]);
    generate
    for (A014Yc=0;A014Yc<8;A014Yc=A014Yc+1) begin : A031Yc      
        assign A004Yc[(7-A014Yc)*64+:64] = A007Yc[A014Yc];
    end
    endgenerate
    assign A045Yc = A048Yc    ;
    assign A046Yc  = A001Yc >= 'd16;
    function [63:0] A051Yc;
        input [63:0]   A055Yc,A056Yc,A057Yc;
        begin
            A051Yc = (A055Yc & A056Yc) ^ (~A055Yc & A057Yc);
        end
    endfunction
    function [63:0] A054Yc;
        input [63:0]   A055Yc,A056Yc,A057Yc;
        begin
            A054Yc = (A055Yc & A056Yc) ^ (A055Yc & A057Yc) ^ (A056Yc & A057Yc);
        end
    endfunction
    function [63:0] A557Yc ;
        input [63:0]   A055Yc;
        begin
            A557Yc  = (A055Yc>>1 | A055Yc<<63) ^ (A055Yc>>8 | A055Yc<<56) ^ (A055Yc>>7);
        end
    endfunction
    function [63:0] A556Yc ;
        input [63:0]   A055Yc;
        begin
            A556Yc  = (A055Yc>>19 | A055Yc<<45) ^ (A055Yc>>61 | A055Yc<<3) ^ (A055Yc>>6);
        end
    endfunction
    function [63:0] A559Yc       ;
        input [63:0]   A055Yc;
        begin
            A559Yc        = (A055Yc>>28 | A055Yc<<36) ^ (A055Yc>>34 | A055Yc<<30) ^ (A055Yc>>39 | A055Yc<<25);
        end
    endfunction
    function [63:0] A558Yc       ;
        input [63:0]   A055Yc;
        begin
            A558Yc        = (A055Yc>>14 | A055Yc<<50) ^ (A055Yc>>18 | A055Yc<<46) ^ (A055Yc>>41 | A055Yc<<23);
        end
    endfunction
    function [63:0] A052Yc;
        input [6:0] A055Yc;
        reg [63:0] A059Yc [0:79];
        begin
            {A059Yc[0 ],A059Yc[1 ],A059Yc[2 ],A059Yc[3 ],A059Yc[4 ],A059Yc[5 ],A059Yc[6 ],A059Yc[7 ],
             A059Yc[8 ],A059Yc[9 ],A059Yc[10],A059Yc[11],A059Yc[12],A059Yc[13],A059Yc[14],A059Yc[15],
             A059Yc[16],A059Yc[17],A059Yc[18],A059Yc[19],A059Yc[20],A059Yc[21],A059Yc[22],A059Yc[23],
             A059Yc[24],A059Yc[25],A059Yc[26],A059Yc[27],A059Yc[28],A059Yc[29],A059Yc[30],A059Yc[31],
             A059Yc[32],A059Yc[33],A059Yc[34],A059Yc[35],A059Yc[36],A059Yc[37],A059Yc[38],A059Yc[39],
             A059Yc[40],A059Yc[41],A059Yc[42],A059Yc[43],A059Yc[44],A059Yc[45],A059Yc[46],A059Yc[47],
             A059Yc[48],A059Yc[49],A059Yc[50],A059Yc[51],A059Yc[52],A059Yc[53],A059Yc[54],A059Yc[55],
             A059Yc[56],A059Yc[57],A059Yc[58],A059Yc[59],A059Yc[60],A059Yc[61],A059Yc[62],A059Yc[63],
             A059Yc[64],A059Yc[65],A059Yc[66],A059Yc[67],A059Yc[68],A059Yc[69],A059Yc[70],A059Yc[71],
             A059Yc[72],A059Yc[73],A059Yc[74],A059Yc[75],A059Yc[76],A059Yc[77],A059Yc[78],A059Yc[79]}
            = 
            {64'h428a2f98d728ae22,64'h7137449123ef65cd,
             64'hb5c0fbcfec4d3b2f,64'he9b5dba58189dbbc,
             64'h3956c25bf348b538,64'h59f111f1b605d019,
             64'h923f82a4af194f9b,64'hab1c5ed5da6d8118,
             64'hd807aa98a3030242,64'h12835b0145706fbe,
             64'h243185be4ee4b28c,64'h550c7dc3d5ffb4e2,
             64'h72be5d74f27b896f,64'h80deb1fe3b1696b1,
             64'h9bdc06a725c71235,64'hc19bf174cf692694,
             64'he49b69c19ef14ad2,64'hefbe4786384f25e3,
             64'h0fc19dc68b8cd5b5,64'h240ca1cc77ac9c65,
             64'h2de92c6f592b0275,64'h4a7484aa6ea6e483,
             64'h5cb0a9dcbd41fbd4,64'h76f988da831153b5,
             64'h983e5152ee66dfab,64'ha831c66d2db43210,
             64'hb00327c898fb213f,64'hbf597fc7beef0ee4,
             64'hc6e00bf33da88fc2,64'hd5a79147930aa725,
             64'h06ca6351e003826f,64'h142929670a0e6e70,
             64'h27b70a8546d22ffc,64'h2e1b21385c26c926,
             64'h4d2c6dfc5ac42aed,64'h53380d139d95b3df,
             64'h650a73548baf63de,64'h766a0abb3c77b2a8,
             64'h81c2c92e47edaee6,64'h92722c851482353b,
             64'ha2bfe8a14cf10364,64'ha81a664bbc423001,
             64'hc24b8b70d0f89791,64'hc76c51a30654be30,
             64'hd192e819d6ef5218,64'hd69906245565a910,
             64'hf40e35855771202a,64'h106aa07032bbd1b8,
             64'h19a4c116b8d2d0c8,64'h1e376c085141ab53,
             64'h2748774cdf8eeb99,64'h34b0bcb5e19b48a8,
             64'h391c0cb3c5c95a63,64'h4ed8aa4ae3418acb,
             64'h5b9cca4f7763e373,64'h682e6ff3d6b2b8a3,
             64'h748f82ee5defb2fc,64'h78a5636f43172f60,
             64'h84c87814a1f0ab72,64'h8cc702081a6439ec,
             64'h90befffa23631e28,64'ha4506cebde82bde9,
             64'hbef9a3f7b2c67915,64'hc67178f2e372532b,
             64'hca273eceea26619c,64'hd186b8c721c0c207,
             64'heada7dd6cde0eb1e,64'hf57d4f7fee6ed178,
             64'h06f067aa72176fba,64'h0a637dc5a2c898a6,
             64'h113f9804bef90dae,64'h1b710b35131c471b,
             64'h28db77f523047d84,64'h32caab7b40c72493,
             64'h3c9ebe0a15c9bebc,64'h431d67c49c100d4c,
             64'h4cc5d4becb3e42b6,64'h597f299cfc657e2a,
             64'h5fcb6fab3ad6faec,64'h6c44198c4a475817};
            A052Yc = A059Yc[A055Yc];
        end
    endfunction
endmodule
module hash_hp_A561Yc   (
    input    wire  [6:0]    A001Yc          ,
    input    wire  [511:0]  A002Yc          ,
    input    wire  [255:0]  A003Yc          ,
    output   wire  [255:0]  A004Yc          ,
    output   wire  [31:0]   A045Yc           ,
    output   wire           A046Yc 
);
    localparam [31:0] A562Yc = 32'h79cc4519,
                      A563Yc = 32'h7a879d8a;
    ////synopsys keep_signal_name "A564Yc" 
    ////synopsys keep_signal_name "A565Yc"       
    ////synopsys keep_signal_name "A566Yc"      
    ////synopsys keep_signal_name "A567Yc"      
    ////synopsys keep_signal_name "A568Yc"       
    ////synopsys keep_signal_name "A569Yc    " 
    wire  [31:0]     A564Yc       ; 
    wire  [31:0]     A565Yc       ; 
    wire  [31:0]     A566Yc       ; 
    wire  [31:0]     A567Yc       ; 
    wire  [31:0]     A568Yc        ; 
    wire  [31:0]     A569Yc      ; 
    wire  [31:0]     A005Yc[0:15];
    wire  [31:0]     A006Yc[0:7] ;
    wire  [31:0]     A007Yc[0:7] ;
    genvar           A014Yc           ;
    generate
    for(A014Yc=0;A014Yc<16;A014Yc=A014Yc+1) begin : A016Yc    
        assign A005Yc[A014Yc] = A002Yc[(15-A014Yc)*32+:32];
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<8;A014Yc=A014Yc+1) begin : A018Yc      
        assign A006Yc[A014Yc] = A003Yc[(7-A014Yc)*32+:32];
    end
    endgenerate
    assign A569Yc     =  A001Yc >= 6'hC ? A005Yc[12] ^ A568Yc       : 
                         A001Yc == 6'hB ? A005Yc[11] ^ A005Yc[15] : 
                         A001Yc == 6'hA ? A005Yc[10] ^ A005Yc[14] : 
                         A001Yc == 6'h9 ? A005Yc[ 9] ^ A005Yc[13] : 
                         A001Yc == 6'h8 ? A005Yc[ 8] ^ A005Yc[12] : 
                         A001Yc == 6'h7 ? A005Yc[ 7] ^ A005Yc[11] : 
                         A001Yc == 6'h6 ? A005Yc[ 6] ^ A005Yc[10] : 
                         A001Yc == 6'h5 ? A005Yc[ 5] ^ A005Yc[ 9] : 
                         A001Yc == 6'h4 ? A005Yc[ 4] ^ A005Yc[ 8] : 
                         A001Yc == 6'h3 ? A005Yc[ 3] ^ A005Yc[ 7] : 
                         A001Yc == 6'h2 ? A005Yc[ 2] ^ A005Yc[ 6] : 
                         A001Yc == 6'h1 ? A005Yc[ 1] ^ A005Yc[ 5] : A005Yc[0] ^ A005Yc[4]; 
    assign A568Yc = A570Yc(A005Yc[0] ^ A005Yc[7] ^ {A005Yc[13][16:0],A005Yc[13][31:17]}) ^ {A005Yc[3][24:0],A005Yc[3][31:25]} ^ A005Yc[10];
    assign A007Yc[0] = A564Yc                  ;
    assign A007Yc[1] = A006Yc[0]              ;
    assign A007Yc[2] = {A006Yc[1][22:0],A006Yc[1][31:23]}    ;
    assign A007Yc[3] = A006Yc[2]              ;
    assign A007Yc[4] = A571Yc(A565Yc)             ;
    assign A007Yc[5] = A006Yc[4]              ;
    assign A007Yc[6] = {A006Yc[5][12:0],A006Yc[5][31:13]}   ;
    assign A007Yc[7] = A006Yc[6]              ;
    assign A566Yc = |A001Yc[5:4] ? A050Yc(({A006Yc[0][19:0],A006Yc[0][31:20]}+A006Yc[4]+A050Yc(A563Yc,A001Yc[5:0])),7) : 
                                 A050Yc(({A006Yc[0][19:0],A006Yc[0][31:20]}+A006Yc[4]+A050Yc(A562Yc,A001Yc[5:0])),7);
    assign A567Yc = A566Yc ^ {A006Yc[0][19:0],A006Yc[0][31:20]};
    assign A564Yc = A572Yc(A006Yc[0],A006Yc[1],A006Yc[2],|A001Yc[5:4]) + A006Yc[3] + A567Yc + A569Yc    ;
    assign A565Yc = (A001Yc>=6'd12) ? A573Yc(A006Yc[4],A006Yc[5],A006Yc[6],|A001Yc[5:4]) + A006Yc[7] + A566Yc + A005Yc[12]     : {
                                    A573Yc(A006Yc[4],A006Yc[5],A006Yc[6],|A001Yc[5:4]) + A006Yc[7] + A566Yc + A005Yc[A001Yc]};
    generate
    for (A014Yc=0;A014Yc<8;A014Yc=A014Yc+1) begin : A031Yc      
        assign A004Yc[(7-A014Yc)*32+:32] = A007Yc[A014Yc];
    end
    endgenerate
    assign A045Yc    = A568Yc;
    assign A046Yc  = A001Yc >= 6'd12;
    function [31:0] A050Yc;
        input [31:0]   A055Yc;
        input [5:0]    A574Yc;
        begin
            A050Yc = (A055Yc << A574Yc[4:0] | (A055Yc >> (6'd32 - A574Yc[4:0])));
        end
    endfunction
    function [31:0] A572Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        input          A575Yc;
        begin
            if (A575Yc)
                A572Yc = (A055Yc & A056Yc) | (A055Yc & A057Yc) | (A056Yc & A057Yc);
            else
                A572Yc = A055Yc ^ A056Yc ^ A057Yc;
        end
    endfunction
    function [31:0] A573Yc;
        input [31:0]   A055Yc,A056Yc,A057Yc;
        input          A575Yc;
        begin
            if (A575Yc)
                A573Yc = (A055Yc & A056Yc) | (~A055Yc & A057Yc);
            else 
                A573Yc = A055Yc ^ A056Yc ^ A057Yc;
        end
    endfunction
    function [31:0] A571Yc;
        input [31:0]   A055Yc;
        begin
            A571Yc = A055Yc ^ {A055Yc[22:0],A055Yc[31:23]} ^ {A055Yc[16:0],A055Yc[31:15]};
        end
    endfunction
    function [31:0] A570Yc;
        input [31:0]   A055Yc;
        begin
            A570Yc = A055Yc ^ {A055Yc[16:0],A055Yc[31:17]} ^ {A055Yc[8:0],A055Yc[31:9]};
        end
    endfunction
endmodule
module hash_hp_A205Yc             #(
     parameter  A061Yc      =   1           
    ,parameter  A062Yc      =   1           
    ,parameter  A063Yc      =   1           
    ,parameter  A064Yc      =   1           
    ,parameter  A065Yc      =   1
    ,parameter  A066Yc      =   1
    ,parameter  A157Yc      =   (A064Yc     ? 512 :
                                 A063Yc     ? 256 :
                                 A065Yc     ? 256 :
                                 A062Yc     ? 160 :
                                 A061Yc     ? 128 :
                                              0)
    ,parameter  A576Yc      =   (1+A064Yc     )*32
)(
     input      wire    [3:0]               A111Yc
    ,input      wire    [6:0]               A001Yc
    ,input      wire    [511:0]             A207Yc   
    ,input      wire    [1023:0]            A208Yc    
    ,input      wire    [A157Yc     -1:0]   A003Yc
    ,output     wire    [255:0]             A209Yc      
    ,output     wire    [127:0]             A210Yc      
    ,output     wire    [255:0]             A211Yc      
    ,output     wire    [511:0]             A212Yc      
    ,output     wire    [159:0]             A213Yc      
    ,output     wire    [A066Yc     *32-1:0]A214Yc     
    ,output     wire    [A066Yc     *32-1:0]A215Yc     
    ,output     wire    [A066Yc     *64-1:0]A216Yc     
    ,output     wire    [A066Yc     *32-1:0]A217Yc     
    ,output     wire                        A046Yc 
);
    genvar A014Yc;
    wire    [A157Yc     -1:0]   A577Yc                           ;
    wire    [6:0]               A163Yc       [0:A066Yc     -1]   ;
    wire    [511:0]             A578Yc      [0:A066Yc     ]     ;
    wire    [127:0]             A579Yc      [0:A066Yc     ]     ;
    wire    [127:0]             A167Yc      [0:A066Yc     -1]   ;
    wire    [511:0]             A580Yc      [0:A066Yc     ]     ;
    wire    [159:0]             A581Yc      [0:A066Yc     ]     ;
    wire    [159:0]             A172Yc      [0:A066Yc     -1]   ;
    wire    [31:0]              A173Yc      [0:A066Yc     -1]   ;
    wire                        A582Yc      [0:A066Yc     -1]   ;
    wire    [511:0]             A583Yc        [0:A066Yc     ]   ;
    wire    [255:0]             A584Yc        [0:A066Yc     ]   ;
    wire    [255:0]             A168Yc        [0:A066Yc     -1] ;
    wire    [31:0]              A169Yc        [0:A066Yc     -1] ;
    wire                        A585Yc        [0:A066Yc     -1] ;
    wire    [1023:0]            A586Yc        [0:A066Yc     ]   ;
    wire    [511:0]             A587Yc        [0:A066Yc     ]   ;
    wire    [511:0]             A170Yc        [0:A066Yc     -1] ;
    wire    [63:0]              A171Yc        [0:A066Yc     -1] ;
    wire                        A588Yc        [0:A066Yc     -1] ;
    wire    [511:0]             A589Yc     [0:A066Yc     ]      ;
    wire    [255:0]             A590Yc     [0:A066Yc     ]      ;
    wire    [255:0]             A165Yc     [0:A066Yc     -1]    ;
    wire    [31:0]              A166Yc     [0:A066Yc     -1]    ;
    wire                        A591Yc     [0:A066Yc     -1]    ;
    assign A577Yc = A003Yc;
    generate
        if(A061Yc  ) begin : A592Yc  
            assign A578Yc   [0]     = A207Yc   ;
            assign A579Yc    [0]    = A577Yc[A157Yc     -128+:128];
        end
        else begin : A593Yc     
            assign A578Yc   [0]     = 512'd0;
            assign A579Yc    [0]    = 128'd0;
        end
        if(A062Yc   ) begin : A594Yc   
            assign A580Yc    [0]    = A207Yc   ;
            assign A581Yc     [0]   = A577Yc[A157Yc     -160+:160];
        end
        else begin : A595Yc      
            assign A580Yc    [0]    = 512'd0;
            assign A581Yc     [0]   = 160'd0;
        end
        if(A063Yc     ) begin : A596Yc     
            assign A583Yc      [0]  = A207Yc   ;
            assign A584Yc       [0] = A577Yc[A157Yc     -256+:256];
        end
        else begin : A597Yc        
            assign A583Yc      [0]  = 512'd0;
            assign A584Yc       [0] = 256'd0;
        end
        if(A064Yc     ) begin : A598Yc     
            assign A586Yc      [0]  = A208Yc    ;
            assign A587Yc       [0] = A577Yc;
        end
        else begin : A599Yc       
            assign A586Yc      [0]  = 1024'd0;
            assign A587Yc       [0] = {A157Yc     {1'b0}};
        end
        if(A065Yc  ) begin : A600Yc  
            assign A589Yc   [0]     = A207Yc   ;
            assign A590Yc    [0]    = A577Yc[A157Yc     -256+:256];
        end
        else begin : A601Yc     
            assign A589Yc   [0]     = 512'd0;
            assign A590Yc    [0]    = 256'd0;
        end
    endgenerate
    generate 
        for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A602Yc
            assign A163Yc[A014Yc] = A001Yc + A014Yc[6:0];
        end
    endgenerate
    generate
        if(A061Yc  ) begin : A603Yc   
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A604Yc       
hash_hp_A000Yc A605Yc(
                     .A001Yc      (A163Yc[A014Yc][5:0]  ),
                     .A002Yc      (A578Yc   [A014Yc]   ),
                     .A003Yc      (A579Yc    [A014Yc]  ),
                     .A004Yc      (A167Yc    [A014Yc]  )
                );
                assign A578Yc   [A014Yc+1]  = A578Yc   [A014Yc]; 
                assign A579Yc    [A014Yc+1] = A167Yc    [A014Yc];
            end
        end
        else begin : A606Yc      
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A604Yc       
                assign A167Yc    [A014Yc]   = 128'd0;
                assign A578Yc   [A014Yc+1]  = 512'd0; 
                assign A579Yc    [A014Yc+1] = A167Yc    [A014Yc];
            end
        end
        if(A062Yc   ) begin : A044Yc    
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A607Yc        
hash_hp_A044Yc     A608Yc      (
                     .A001Yc      (A163Yc[A014Yc]       ),
                     .A002Yc      (A580Yc    [A014Yc]  ),
                     .A003Yc      (A581Yc     [A014Yc] ),
                     .A004Yc      (A172Yc     [A014Yc] ),
                     .A045Yc       (A173Yc   [A014Yc]   ),
                     .A046Yc     (A582Yc      [A014Yc])
                );
                assign A580Yc    [A014Yc+1] = 
                        A001Yc < 16 ? A580Yc    [0] : 
                                     {A580Yc    [A014Yc][479:0],A173Yc   [A014Yc]};
                assign A581Yc     [A014Yc+1] = A172Yc     [A014Yc];
            end
        end
        else begin : A609Yc       
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A607Yc        
                assign A172Yc     [A014Yc]  = 160'd0;
                assign A173Yc   [A014Yc]    = 32'd0;
                assign A582Yc      [A014Yc] = 1'd0;
                assign A580Yc    [A014Yc+1] = 512'd0;
                assign A581Yc     [A014Yc+1] = A172Yc     [A014Yc];
            end
        end
        if(A063Yc     ) begin : A553Yc      
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A610Yc          
hash_hp_A553Yc       A611Yc         (             
                     .A001Yc      (A163Yc[A014Yc]           ),
                     .A002Yc      (A583Yc      [A014Yc]    ),
                     .A003Yc      (A584Yc       [A014Yc]   ),
                     .A004Yc      (A168Yc       [A014Yc]   ),
                     .A045Yc       (A169Yc     [A014Yc]     ),
                     .A046Yc     (A585Yc        [A014Yc]  )
                );
                assign A583Yc      [A014Yc+1] =  
                        A001Yc < 16 ? A583Yc      [0] : 
                                     {A583Yc      [A014Yc][479:0],A169Yc     [A014Yc]};
                assign A584Yc       [A014Yc+1] = A168Yc       [A014Yc];
            end
        end
        else begin : A612Yc         
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A610Yc          
                assign A168Yc       [A014Yc]  = 256'd0;
                assign A169Yc     [A014Yc]    = 32'd0;
                assign A585Yc        [A014Yc] = 1'd0;
                assign A583Yc      [A014Yc+1] = 512'd0; 
                assign A584Yc       [A014Yc+1] = A168Yc       [A014Yc];
            end
        end
        if(A064Yc     ) begin : A560Yc      
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A613Yc          
hash_hp_A560Yc       A614Yc         (
                     .A001Yc      (A163Yc[A014Yc]           ),
                     .A002Yc      (A586Yc      [A014Yc]    ),
                     .A003Yc      (A587Yc       [A014Yc]   ),
                     .A004Yc      (A170Yc       [A014Yc]   ),
                     .A045Yc       (A171Yc     [A014Yc]     ),
                     .A046Yc     (A588Yc        [A014Yc]  )
                );
                assign A586Yc      [A014Yc+1] =  
                        A001Yc < 16 ? A586Yc      [0] : 
                                     {A586Yc      [A014Yc][959:0],A171Yc     [A014Yc]};
                assign A587Yc       [A014Yc+1] = A170Yc       [A014Yc];
            end
        end
        else begin : A615Yc         
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A613Yc          
                assign A170Yc       [A014Yc]  = 512'd0;
                assign A171Yc     [A014Yc]    = 64'd0;
                assign A588Yc        [A014Yc] = 1'd0;
                assign A586Yc      [A014Yc+1] = 1024'd0; 
                assign A587Yc       [A014Yc+1] = A170Yc       [A014Yc];
            end
        end
        if(A065Yc  ) begin : A561Yc   
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A613Yc          
hash_hp_A561Yc    A616Yc     (
                     .A001Yc      (A163Yc[A014Yc]           ),
                     .A002Yc      (A589Yc   [A014Yc]       ),
                     .A003Yc      (A590Yc    [A014Yc]      ),
                     .A004Yc      (A165Yc    [A014Yc]      ),
                     .A045Yc       (A166Yc  [A014Yc]        ),
                     .A046Yc     (A591Yc     [A014Yc]     )
                );
                assign A589Yc   [A014Yc+1] =  
                        A001Yc < 12 ? A589Yc   [0] : 
                                     {A589Yc   [A014Yc][479:0],A166Yc  [A014Yc]};
                assign A590Yc    [A014Yc+1] = A165Yc    [A014Yc];
            end
        end
        else begin : A617Yc      
            for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A613Yc          
                assign A165Yc    [A014Yc]  = 256'd0;
                assign A166Yc  [A014Yc]    = 32'd0;
                assign A591Yc     [A014Yc] = 1'd0;
                assign A589Yc   [A014Yc+1] = 512'd0; 
                assign A590Yc    [A014Yc+1] = A165Yc    [A014Yc];
            end
        end
    endgenerate
    assign A046Yc  = 
        (A111Yc==4'b0000   )     ? A065Yc      & A591Yc     [A066Yc     -1]    :
        (A111Yc==4'b0010)     ? A063Yc      & A585Yc        [A066Yc     -1] :
        (A111Yc==4'b0110)     ? A063Yc      & A585Yc        [A066Yc     -1] :
        (A111Yc==4'b0011)     ? A064Yc      & A588Yc        [A066Yc     -1] :
        (A111Yc==4'b0100)     ? A064Yc      & A588Yc        [A066Yc     -1] :
        (A111Yc==4'b0111) ? A064Yc      & A588Yc        [A066Yc     -1] :
        (A111Yc==4'b1000) ? A064Yc      & A588Yc        [A066Yc     -1] :
        (A111Yc==4'b0101)       ? A062Yc      & A582Yc      [A066Yc     -1]   :
                                1'b0;
    assign A209Yc      = A065Yc      ? A165Yc    [A066Yc     -1]   : 256'd0;
    assign A210Yc      = A061Yc      ? A167Yc    [A066Yc     -1]   : 128'd0;
    assign A211Yc      = A063Yc      ? A168Yc       [A066Yc     -1]: 256'd0;
    assign A212Yc      = A064Yc      ? A170Yc       [A066Yc     -1]: 512'd0;
    assign A213Yc      = A062Yc      ? A172Yc     [A066Yc     -1]  : 160'd0;
    generate 
        for(A014Yc=0;A014Yc<A066Yc     ;A014Yc=A014Yc+1) begin : A618Yc 
            assign A214Yc  [(A066Yc     -1-A014Yc)*32+:32]    = 
                                A065Yc      ? A166Yc  [A014Yc]     : 32'd0;
            assign A215Yc     [(A066Yc     -1-A014Yc)*32+:32] = 
                                A063Yc      ? A169Yc     [A014Yc]  : 32'd0;
            assign A216Yc     [(A066Yc     -1-A014Yc)*64+:64] = 
                                A064Yc      ? A171Yc     [A014Yc]  : 64'd0;
            assign A217Yc   [(A066Yc     -1-A014Yc)*32+:32]   = 
                                A062Yc      ? A173Yc   [A014Yc]    : 32'd0;
        end
    endgenerate
endmodule
module hash_hp_A183Yc   #(
     parameter  A061Yc      =   1           
    ,parameter  A062Yc      =   1           
    ,parameter  A063Yc      =   1           
    ,parameter  A064Yc      =   1           
    ,parameter  A065Yc      =   1
    ,parameter  A066Yc      =   1
    ,parameter  A157Yc      =   (A064Yc     ? 512 :
                                 A063Yc     ? 256 :
                                 A065Yc     ? 256 :
                                 A062Yc     ? 160 :
                                 A061Yc     ? 128 :
                                              0)
)(
    input   wire                            clk         ,
    input   wire                            reset_n     ,
    input   wire    [15:0]                  A155Yc      ,
    input   wire    [511:0]                 A154Yc      ,
    output  wire    [511:0]                 A156Yc      ,
    input   wire    [1023:0]                A185Yc      ,  
    input   wire    [31:0]                  A186Yc      , 
    input   wire                            A187Yc      ,
    input   wire                            A188Yc      ,
    input   wire                            A189Yc       ,
    input   wire    [3:0]                   A069Yc      ,
    input   wire                            A190Yc      ,
    input   wire    [A066Yc     *32-1:0]    A191Yc      ,
    input   wire    [255:0]                 A192Yc       ,
    input   wire    [127:0]                 A193Yc       ,
    input   wire    [A066Yc     *32-1:0]    A194Yc      ,
    input   wire    [255:0]                 A195Yc      ,
    input   wire    [A066Yc     *64-1:0]    A196Yc      ,
    input   wire    [511:0]                 A197Yc      ,
    input   wire    [A066Yc     *32-1:0]    A198Yc      ,
    input   wire    [159:0]                 A199Yc      ,
    output  wire    [A157Yc     -1:0]       A004Yc       ,
    output  wire    [511:0]                 A200Yc      ,
    output  wire    [1023:0]                A201Yc      ,
    input   wire    [1:0]                   A202Yc       ,
    input   wire    [6:0]                   A203Yc      ,
    input   wire    [31:0]                  A204Yc      
);
    localparam  A619Yc      = A064Yc      ? 32 :
                              A063Yc      ? 16 :
                              A065Yc      ? 16 :
                              A062Yc      ? 16 :
                              A061Yc      ? 16 :
                                            1  ;
    localparam  A620Yc      = A064Yc      ? 16 :
                              A063Yc      ? 8  :
                              A065Yc      ? 8  :
                              A062Yc      ? 5  :
                              A061Yc      ? 4  :
                                            1  ;
    localparam  [31:0] A621Yc          = (A620Yc      - 32'd1);
    localparam  [31:0] A622Yc          = (A619Yc      - 32'd1);
    integer         A014Yc,A058Yc                        ;
    genvar          A015Yc,A623Yc                        ;
    reg   [31:0]    A624Yc  [0:A619Yc     -1]       ;
    reg   [31:0]    A625Yc  [0:A620Yc     -1]       ;
    reg   [31:0]    A626Yc  [0:A620Yc     -1]       ;
    wire  [1023:0]  A627Yc                          ;
    wire  [511:0]   A628Yc                          ;
    wire  [31:0]    A629Yc      [0:3]               ;
    wire  [31:0]    A630Yc      [0:3]               ;
    wire  [31:0]    A631Yc      [0:3]               ;
    wire            A098Yc                          ;
    wire            A099Yc                          ;
    wire            A100Yc                          ;
    wire            A101Yc                          ;
    wire            A102Yc                          ;
    assign A098Yc         = A061Yc   && (A069Yc==4'b0001);
    assign A099Yc         = A065Yc   && (A069Yc==4'b0000);
    assign A100Yc         = A062Yc    && (A069Yc==4'b0101);
    assign A101Yc         = 
            A063Yc      && ((A069Yc==4'b0010) || (A069Yc==4'b0110));
    assign A102Yc         = 
            A064Yc      && (
                (A069Yc==4'b0100) || (A069Yc==4'b0011) || 
                (A069Yc==4'b0111) || (A069Yc==4'b1000));
    generate
        if(A061Yc  ) begin : A632Yc 
            for (A623Yc=0;A623Yc<4;A623Yc=A623Yc+1) begin : A633Yc     
               for (A015Yc=0;A015Yc<4;A015Yc=A015Yc+1) begin : A019Yc     
                  assign A630Yc      [A623Yc][A015Yc*8 +: 8] = A625Yc  [A623Yc][(3-A015Yc)*8 +: 8];
               end
            end
            for (A623Yc=0;A623Yc<4;A623Yc=A623Yc+1) begin : A634Yc   
               for (A015Yc=0;A015Yc<4;A015Yc=A015Yc+1) begin : A635Yc   
                  assign A629Yc    [A623Yc][A015Yc*8 +: 8] = A626Yc[A623Yc][(3-A015Yc)*8 +: 8];
               end
            end
            for (A623Yc=0;A623Yc<4;A623Yc=A623Yc+1) begin : A636Yc
               assign A631Yc   [A623Yc] = A630Yc      [A623Yc] + A629Yc    [A623Yc];
            end
        end
        else begin : A637Yc
            for (A623Yc=0;A623Yc<4;A623Yc=A623Yc+1) begin : A633Yc     
               assign A630Yc      [A623Yc]   = 32'd0;
               assign A629Yc    [A623Yc]     = 32'd0;
               assign A631Yc   [A623Yc]      = 32'd0;
            end
        end
    endgenerate
    generate
        for(A015Yc=0;A015Yc<32;A015Yc=A015Yc+1) begin: A638Yc      
            assign A627Yc       [1023-A015Yc*32:992-A015Yc*32] = A185Yc   [A015Yc*32+:32];
        end
        for(A015Yc=0;A015Yc<16;A015Yc=A015Yc+1) begin: A639Yc      
            assign A628Yc       [511-A015Yc*32:480-A015Yc*32] = A154Yc[A015Yc*32+:32];
        end
    endgenerate
    wire [15:0]        A640Yc      ;
    assign A640Yc      = A064Yc      ? A155Yc      : A155Yc     >>8 ;
    wire [31:0]        A641Yc      ;
    assign A641Yc      = A064Yc      ? A186Yc     : A186Yc    >>16 ;
    always@(posedge clk or negedge reset_n)
    begin
        if (!reset_n) begin
           for (A014Yc=0;A014Yc<A620Yc     ;A014Yc=A014Yc+1) begin
              A626Yc[A014Yc] <= 'd0;
           end
        end
        else if (|(A640Yc     )) begin
            for(A014Yc=0;A014Yc<A620Yc     ;A014Yc=A014Yc+1) begin 
                if (A640Yc     [A014Yc])
                    A626Yc[A621Yc      -A014Yc]   <= 
                         A628Yc       [(A621Yc      -A014Yc)*32+:32];
            end
        end
        else begin
            if (A099Yc     ) begin
                if(A188Yc    ) begin
                    for(A014Yc=0;A014Yc<A065Yc  *8;A014Yc=A014Yc+1) begin
                        A626Yc[A014Yc] <=  A625Yc  [A014Yc] ^ A626Yc[A014Yc];
                    end
                end
            end
            if (A098Yc     ) begin
                if (A188Yc    ) begin
                    for(A014Yc=0;A014Yc<A061Yc  *4;A014Yc=A014Yc+1) begin
                        for(A058Yc=0;A058Yc<4;A058Yc=A058Yc+1) begin
                            A626Yc[A014Yc][A058Yc*8+:8] <=  A631Yc   [A014Yc][(3-A058Yc)*8+:8];
                        end
                    end
                end
            end
            if (A101Yc        ) begin
                if (A188Yc    ) begin
                    for(A014Yc=0;A014Yc<A063Yc     *8;A014Yc=A014Yc+1) begin
                        A626Yc[A014Yc] <=  A625Yc  [A014Yc] + A626Yc[A014Yc];
                    end
                end
            end
            if (A102Yc        ) begin
                if (A188Yc    ) begin
                    for(A014Yc=0;A014Yc<A064Yc     *16;A014Yc=A014Yc+2) begin
                        {A626Yc[A014Yc],A626Yc[A014Yc+1]} <= 
                             {A625Yc  [A014Yc],A625Yc  [A014Yc+1]} + 
                                   {A626Yc[A014Yc],A626Yc[A014Yc+1]};
                    end
                end
            end
            if (A100Yc      ) begin
                if (A188Yc    ) begin
                    for(A014Yc=0;A014Yc<A062Yc   *5;A014Yc=A014Yc+1) begin
                        A626Yc[A014Yc] <=  A625Yc  [A014Yc] + A626Yc[A014Yc];
                    end
                end
            end
        end
    end
    always@(posedge clk or negedge reset_n) 
    begin
        if (!reset_n) begin
           for (A014Yc=0;A014Yc<A620Yc     ;A014Yc=A014Yc+1) begin
              A625Yc  [A014Yc] <= 'd0;
           end
        end
        else if (A099Yc     ) begin
            if (A187Yc  ) begin
                for(A014Yc=0;A014Yc<A065Yc  *8;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A626Yc[A014Yc];
                end
            end
            else if (A189Yc) begin
                for(A014Yc=0;A014Yc<A065Yc  *8;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A192Yc[(7-A014Yc)*32+:32];
                end
            end
        end
        else if (A098Yc     ) begin
            if (A187Yc  ) begin
                for(A014Yc=0;A014Yc<A061Yc  *4;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A626Yc[A014Yc];
                end
            end
            else if (A189Yc) begin
                for(A014Yc=0;A014Yc<A061Yc  *4;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A193Yc[(3-A014Yc)*32+:32];
                end
            end
        end
        else if (A101Yc        ) begin
            if (A187Yc  ) begin
                for(A014Yc=0;A014Yc<A063Yc     *8;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A626Yc[A014Yc];
                end
            end
            else if (A189Yc) begin
                for(A014Yc=0;A014Yc<A063Yc     *8;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A195Yc  [(7-A014Yc)*32+:32];
                end
            end
        end
        else if (A102Yc        ) begin
            if (A187Yc  ) begin
                for(A014Yc=0;A014Yc<A064Yc     *16;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A626Yc[A014Yc];
                end
            end
            else if (A189Yc) begin
                for(A014Yc=0;A014Yc<A064Yc     *16;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A197Yc  [(15-A014Yc)*32+:32];
                end
            end
        end
        else if (A100Yc      ) begin
            if (A187Yc  ) begin
                for(A014Yc=0;A014Yc<A062Yc   *5;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A626Yc[A014Yc];
                end
            end
            else if (A189Yc) begin
                for(A014Yc=0;A014Yc<A062Yc   *5;A014Yc=A014Yc+1) begin
                    A625Yc  [A014Yc] <=  A199Yc[(4-A014Yc)*32+:32];
                end
            end
        end
    end
    generate 
        if(A066Yc     ==1) begin : A642Yc         
            always@(posedge clk or negedge reset_n)
            begin
                if (!reset_n) begin
                   for(A014Yc=0;A014Yc<A619Yc     ;A014Yc=A014Yc+1)
                      A624Yc[A014Yc] <= 'd0;
                end
                else if (A190Yc ) begin
                    if (A099Yc     ) begin
                        A624Yc[A065Yc  *15] <=  A191Yc  ;
                        for(A014Yc=0;A014Yc<A065Yc  *15;A014Yc=A014Yc+1)
                            A624Yc[A014Yc] <=  A624Yc[A014Yc+1];
                    end
                    else if (A101Yc        ) begin
                        A624Yc[A063Yc     *15] <=  A194Yc     ;
                        for(A014Yc=0;A014Yc<A063Yc     *15;A014Yc=A014Yc+1)
                            A624Yc[A014Yc] <=  A624Yc[A014Yc+1];
                    end
                    else if (A102Yc        ) begin
                        A624Yc[A064Yc     *30] <=  A196Yc     [63:32];
                        A624Yc[A064Yc     *31] <=  A196Yc     [31:0];
                        for(A014Yc=0;A014Yc<A064Yc     *30;A014Yc=A014Yc+2) begin
                            A624Yc[A014Yc] <=  A624Yc[A014Yc+2];
                            A624Yc[A014Yc+1] <=  A624Yc[A014Yc+3];
                        end
                    end
                    else if (A100Yc      ) begin
                        A624Yc[A062Yc   *15] <=  A198Yc   ;
                        for(A014Yc=0;A014Yc<A062Yc   *15;A014Yc=A014Yc+1)
                            A624Yc[A014Yc] <=  A624Yc[A014Yc+1];
                    end
                end
                else if(|(A641Yc    )) begin
                      for(A014Yc=0;A014Yc<A619Yc     ;A014Yc=A014Yc+1) begin 
                        if (A641Yc    [A014Yc])
                            A624Yc[A622Yc      -A014Yc]   <= 
                                 A627Yc       [(A622Yc      -A014Yc)*32+:32];
                      end
                end
                else if(|(A202Yc)) begin
                    if(A202Yc==2'b01) begin 
                        A624Yc[A203Yc    [6:2]][(3-A203Yc    [1:0])*8+:8] <= 
                                 A204Yc    [7:0];
                    end
                    else if(A202Yc==2'b10) begin 
                        A624Yc[A203Yc    [6:2]] <=  A204Yc    ;
                    end
                end
            end
        end
        if(A066Yc     ==2) begin : A643Yc         
            always@(posedge clk or negedge reset_n)
            begin
                if (!reset_n) begin
                   for(A014Yc=0;A014Yc<A619Yc     ;A014Yc=A014Yc+1)
                      A624Yc[A014Yc] <= 'd0;
                end
                else if (A190Yc ) begin
                    if (A099Yc     ) begin
                        A624Yc[A065Yc  *14] <=  A191Yc  [63:32];
                        A624Yc[A065Yc  *15] <=  A191Yc  [31:0 ];
                        for(A014Yc=0;A014Yc<A065Yc  *14;A014Yc=A014Yc+2) begin
                            A624Yc[A014Yc  ] <=  A624Yc[A014Yc+2];
                            A624Yc[A014Yc+1] <=  A624Yc[A014Yc+3];
                        end
                    end
                    else if (A101Yc        ) begin
                        A624Yc[A063Yc     *14] <=  A194Yc     [63:32];
                        A624Yc[A063Yc     *15] <=  A194Yc     [31:0 ];
                        for(A014Yc=0;A014Yc<A063Yc     *14;A014Yc=A014Yc+2) begin
                            A624Yc[A014Yc  ] <=  A624Yc[A014Yc+2];
                            A624Yc[A014Yc+1] <=  A624Yc[A014Yc+3];
                        end
                    end
                    else if (A102Yc        ) begin
                        A624Yc[A064Yc     *28] <=  A196Yc     [127:96];
                        A624Yc[A064Yc     *29] <=  A196Yc     [95 :64];
                        A624Yc[A064Yc     *30] <=  A196Yc     [63 :32];
                        A624Yc[A064Yc     *31] <=  A196Yc     [31 :0 ];
                        for(A014Yc=0;A014Yc<A064Yc     *28;A014Yc=A014Yc+4) begin
                            A624Yc[A014Yc  ] <=  A624Yc[A014Yc+4];
                            A624Yc[A014Yc+1] <=  A624Yc[A014Yc+5];
                            A624Yc[A014Yc+2] <=  A624Yc[A014Yc+6];
                            A624Yc[A014Yc+3] <=  A624Yc[A014Yc+7];
                        end
                    end
                    else if (A100Yc      ) begin
                        A624Yc[A062Yc   *14] <=  A198Yc   [63:32];
                        A624Yc[A062Yc   *15] <=  A198Yc   [31:0 ];
                        for(A014Yc=0;A014Yc<A062Yc   *14;A014Yc=A014Yc+2) begin
                            A624Yc[A014Yc  ] <=  A624Yc[A014Yc+2];
                            A624Yc[A014Yc+1] <=  A624Yc[A014Yc+3];
                        end
                    end
                end
                else if(|(A641Yc    )) begin
                      for(A014Yc=0;A014Yc<A619Yc     ;A014Yc=A014Yc+1) begin 
                        if (A641Yc    [A014Yc])
                            A624Yc[A619Yc     -1-A014Yc]   <= 
                                 A627Yc       [(A622Yc      -A014Yc)*32+:32];
                      end
                end
                else if(|(A202Yc)) begin
                    if(A202Yc==2'b01) begin 
                        A624Yc[A203Yc    [6:2]][(3-A203Yc    [1:0])*8+:8] <= 
                                 A204Yc    [7:0];
                    end
                    else if(A202Yc==2'b10) begin 
                        A624Yc[A203Yc    [6:2]] <=  A204Yc    ;
                    end
                end
            end
        end
    endgenerate
    generate
        for (A015Yc=0;A015Yc<A620Yc     ;A015Yc=A015Yc+1) begin : A644Yc    
            assign A004Yc[(A620Yc     -1-A015Yc)*32 +: 32] = A625Yc  [A015Yc];
        end
    endgenerate
    generate
        for (A015Yc=0;A015Yc<16;A015Yc=A015Yc+1) begin : A645Yc    
            assign A200Yc   [(15-A015Yc)*32 +: 32] = A624Yc[A015Yc];
            assign A201Yc    [(15-A015Yc)*64 +: 64] = 
                    {A624Yc[A064Yc     *2*A015Yc],A624Yc[A064Yc     *2*A015Yc+1]};
        end
        for (A015Yc=0;A015Yc<4;A015Yc=A015Yc+1) begin : A646Yc     
            if(A061Yc  |A062Yc   |A063Yc     |A064Yc     |A065Yc  ) 
            begin : A647Yc   
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = A626Yc[A015Yc];
            end
            else begin : A648Yc    
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = 32'd0;
            end
        end
        for (A015Yc=4;A015Yc<5;A015Yc=A015Yc+1) begin : A649Yc     
            if(A062Yc    || A063Yc      || A064Yc      || A065Yc  ) 
            begin : A650Yc   
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = A626Yc[A015Yc];
            end
            else begin : A651Yc    
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = 32'd0;
            end
        end
        for (A015Yc=5;A015Yc<8;A015Yc=A015Yc+1) begin : A652Yc     
            if(A063Yc      || A064Yc      || A065Yc  ) begin : A653Yc   
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = A626Yc[A015Yc];
            end
            else begin : A654Yc    
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = 32'd0;
            end
        end
        for (A015Yc=8;A015Yc<16;A015Yc=A015Yc+1) begin : A655Yc     
            if(A064Yc     ) begin  : A656Yc   
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = A626Yc[A015Yc];
            end
            else begin : A657Yc    
                assign A156Yc[511-A015Yc*32:480-A015Yc*32] = 32'd0;
            end
        end
    endgenerate
endmodule
module hash_hp_A658Yc        
#(
    parameter   A659Yc                      =  12       
   ,parameter   A660Yc                      =  32
)(
    input    wire                           i_s_hclk      
   ,input    wire                           i_s_hresetn   
   ,input    wire                           A661Yc        
   ,input    wire  [A659Yc          -1:0]   A662Yc        
   ,input    wire                           A663Yc        
   ,input    wire  [2:0]                    A664Yc        
   ,input    wire  [2:0]                    A665Yc        
   ,input    wire  [3:0]                    A666Yc        
   ,input    wire  [1:0]                    A667Yc        
   ,input    wire                           A668Yc        
   ,input    wire                           A669Yc        
   ,input    wire  [A660Yc          -1:0]   A670Yc        
   ,output   wire                           A671Yc        
   ,output   wire                           A672Yc        
   ,output   wire  [31:0]                   A673Yc        
   ,output   wire  [A659Yc          -1:0]   A674Yc      
   ,output   wire  [A660Yc          -1:0]   A675Yc      
   ,output   wire                           A676Yc      
   ,output   wire                           A677Yc      
   ,output   wire  [3:0]                    A678Yc      
   ,output   wire                           A679Yc      
   ,input    wire  [A660Yc          -1:0]   A680Yc      
   ,input    wire                           A681Yc      
);
    reg     [A659Yc          -1:0]  A682Yc              ;
    reg                             A683Yc              ;
    reg                             A684Yc              ;
    reg     [3:0]                   A685Yc              ;
    reg     [3:0]                   A686Yc              ;
    reg                             A687Yc              ;
    reg                             A688Yc              ;
    wire    [A659Yc          -1:0]  A689Yc              ;
    wire                            A690Yc              ;
    wire                            A691Yc              ;
    wire    [3:0]                   A692Yc              ;
    wire                            A693Yc              ;
    wire                            A694Yc              ;
    wire                            A695Yc              ;
    wire                            A696Yc              ;
    wire                            A697Yc              ;
    wire                            A698Yc              ;
    wire                            A699Yc              ;
    wire                            A700Yc              ;
    assign A700Yc           =       A668Yc              ;
    wire    [2:0]                   A701Yc              ;
    assign A701Yc           =       A665Yc              ;
    assign A694Yc         = A661Yc   && A667Yc    [1] && A669Yc    ;
    assign A695Yc      = A694Yc         && A663Yc     ;
    assign A696Yc     = A694Yc         && !A663Yc    ;
    assign A689Yc     = A694Yc         ? A662Yc    : A682Yc;
    assign A697Yc           = A696Yc     || (A683Yc    && A669Yc    );
    assign A690Yc        = A697Yc           ? A696Yc     : A683Yc   ;
    assign A698Yc            = A695Yc      || (A684Yc     && A669Yc    );
    assign A691Yc         = A695Yc          ? 1'b1 :
                            !A671Yc         ? A684Yc    :
                                              1'b0;
    always@(A662Yc    or A664Yc   )
    begin
        if(A664Yc    == 3'b000) begin
            case (A662Yc   [1:0])
                2'b00   : A686Yc        = 4'b0001;
                2'b01   : A686Yc        = 4'b0010;
                2'b10   : A686Yc        = 4'b0100;
                default : A686Yc        = 4'b1000;
            endcase
        end
        else if (A664Yc    == 3'b001) begin
                A686Yc        = A662Yc   [1] ? 4'b1100 : 4'b0011;
        end
        else begin
                A686Yc        = 4'b1111;
        end
    end
    assign A692Yc        = A697Yc           | A698Yc            ? 
                                    A686Yc        : A685Yc   ;
    assign A693Yc         = A697Yc           | A698Yc            ?
                                    A666Yc   [1] : A687Yc    ;
    assign A699Yc    = A694Yc         ? 1'b1 :
                       A681Yc         ? 1'b0 :
                       A688Yc   ;
    always@(posedge i_s_hclk or negedge i_s_hresetn)
    begin
        if(!i_s_hresetn) begin
            A682Yc      <=  {A659Yc          {1'b0}}  ;
            A683Yc      <=  1'b0                      ;
            A684Yc      <=  1'b0                      ;
            A685Yc      <=  4'd0                      ;
            A687Yc      <=  1'b0                      ;
            A688Yc      <=  1'b0                      ;
        end
        else begin 
            A682Yc      <=  A689Yc                    ;
            A683Yc      <=  A690Yc                    ;
            A684Yc      <=  A691Yc                    ;
            A685Yc      <=  A692Yc                    ;
            A687Yc      <=  A693Yc                    ;
            A688Yc      <=  A699Yc                    ;
        end
    end
    assign A674Yc         = A682Yc      ;
    assign A675Yc         = A670Yc      ;
    assign A676Yc         = A683Yc      ;
    assign A677Yc         = A684Yc      ;
    assign A678Yc         = A685Yc      ; 
    assign A679Yc         = A687Yc      ;
    assign A673Yc         = A680Yc      ;
    assign A671Yc         = A688Yc    ? A681Yc     :1'b1;
    assign A672Yc         = 1'b0        ;
endmodule
module hash_hp_A702Yc         
#(
    parameter   A703Yc      =   10
)
(
    input   wire                        clk                 
   ,input   wire                        rst_n
   ,input   wire    [A703Yc   -1:0]     A704Yc   
   ,input   wire    [A703Yc   -1:0]     A705Yc   
   ,output  wire    [A703Yc   -1:0]     A706Yc
);
    genvar A014Yc;
    reg     [A703Yc   -1:0] A707Yc          ;
    reg     [A703Yc   -1:0] A708Yc          ;
    wire    [A703Yc   -1:0] A709Yc          ;
    wire    [A703Yc   -1:0] A710Yc          ;
    generate
    for(A014Yc=0;A014Yc<A703Yc   ;A014Yc=A014Yc+1) begin : A711Yc
        assign A710Yc         [A014Yc] = A704Yc   [A014Yc] && !A708Yc      [A014Yc];
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<A703Yc   ;A014Yc=A014Yc+1) begin : A712Yc
        assign A709Yc       [A014Yc] = A710Yc         [A014Yc]    ? 1'b1 :
                                  A705Yc   [A014Yc]          ? 1'b0 :
                                                          A707Yc   [A014Yc];
    end
    endgenerate
    always@(posedge clk or negedge rst_n)
    begin
        if (!rst_n) begin 
            A707Yc      <= {A703Yc   {1'd0}};
            A708Yc      <= {A703Yc   {1'd0}};
        end
        else begin
            A707Yc      <=  A709Yc            ;
            A708Yc      <=  A704Yc            ;
        end
    end
    assign A706Yc    = A707Yc    ;
endmodule
module hash_hp_A713Yc          
#(
    parameter   A714Yc                          = 2  
   ,parameter   A715Yc                          = 512
   ,parameter   A716Yc                          = 1  
   ,parameter   A717Yc                          = 1'b1    
   ,parameter   A061Yc                          = 1'b1    
   ,parameter   A062Yc                          = 1'b1    
   ,parameter   A063Yc                          = 1'b1    
   ,parameter   A064Yc                          = 1'b1    
   ,parameter   A219Yc                          = 1'b1    
   ,parameter   A065Yc                          = 1'b1    
   ,parameter   A220Yc                          = A219Yc      ? 1152 : 1024    
   ,parameter   A221Yc                          = A220Yc      /32
   ,parameter   A222Yc                          = A219Yc      ? 1600 : 512
   ,parameter   A223Yc                          = A222Yc        /32
)
(
    input   wire                                clk_axi
   ,input   wire                                rst_n_axi
   ,input   wire                                clk_core                 
   ,input   wire                                rst_n_core
   ,input   wire                                A718Yc  
   ,output  wire                                A719Yc      
   ,output  wire                                A720Yc       
   ,output  wire                                A721Yc      
   ,input   wire    [3:0]                       A111Yc 
   ,input   wire                                A722Yc  
   ,input   wire                                A723Yc
   ,input   wire                                A724Yc  
   ,input   wire                                A725Yc 
   ,input   wire                                A726Yc      
   ,input   wire                                A727Yc      
   ,input   wire                                A728Yc          
   ,input   wire    [127:0]                     A729Yc   
   ,input   wire    [127:0]                     A730Yc   
   ,input   wire                                A731Yc        
   ,output  wire    [127:0]                     A732Yc   
   ,input   wire    [31:0]                      A733Yc   
   ,input   wire    [31:0]                      A734Yc   
   ,input   wire                                A735Yc        
   ,output  wire    [31:0]                      A736Yc   
   ,input   wire                                A737Yc    
   ,input   wire    [511:0]                     A738Yc  
   ,input   wire                                A112Yc
   ,input   wire    [31:0]                      A739Yc  
   ,input   wire                                A740Yc     
   ,input   wire    [31:0]                      A741Yc   
   ,input   wire                                A155Yc     
   ,input   wire    [5:0]                       A742Yc        
   ,output  wire    [A222Yc        -1:0]        A743Yc    
   ,output  wire                                A744Yc       
   ,output  wire    [3:0]                       A745Yc     
   ,output  wire                                A746Yc      
   ,input   wire                                A747Yc      
   ,output  wire                                A748Yc       
   ,output  wire    [A223Yc       -1:0]         A749Yc        
   ,output  wire                                A750Yc           
   ,output  wire                                A751Yc      
   ,output  wire                                A752Yc 
   ,output  wire                                A753Yc       
   ,output  wire                                A754Yc     
   ,output  wire                                A755Yc    
   ,output  wire    [127:0]                     A756Yc   
   ,input   wire                                A757Yc    
   ,output  wire                                A758Yc       
   ,output  wire    [A220Yc      -1:0]          A759Yc    
   ,output  wire    [A222Yc        -1:0]        A760Yc    
   ,input   wire    [A222Yc        -1:0]        A761Yc      
   ,output  wire    [A221Yc     -1:0]           A762Yc        
   ,output  wire                                A763Yc         
   ,input   wire                                A764Yc          
   ,input   wire                                A068Yc   
   ,input   wire                                A363Yc            
   ,input   wire                                A765Yc          
   ,output  wire                                A766Yc          
   ,output  wire                                A767Yc       
   ,output  wire                                A768Yc       
   ,input   wire                                A769Yc        
   ,input   wire    [A715Yc            -1:0]    A770Yc              
   ,input   wire                                A771Yc         
   ,output  wire                                A772Yc           
   ,output  wire    [A716Yc           :0]       A773Yc                
   ,output  wire                                A774Yc       
   ,input   wire                                A775Yc        
   ,input   wire                                A776Yc              
   ,output  wire    [A715Yc            -1:0]    A777Yc              
   ,input   wire                                A778Yc         
   ,output  wire                                A779Yc            
   ,output  wire    [A716Yc           :0]       A780Yc                
   ,input   wire                                A781Yc         
   ,input   wire                                A782Yc        
   ,input   wire                                A783Yc            
   ,output  wire                                A784Yc    
   ,output  wire                                A785Yc   
   ,output  wire                                A786Yc       
   ,output  wire    [A714Yc            -1:0]    A787Yc   
   ,output  wire    [A714Yc            -1:0]    A788Yc   
   ,input   wire    [A714Yc            -1:0]    A789Yc
   ,output  wire                                A706Yc
   ,output  wire                                A790Yc       
);
    genvar A014Yc;
    integer A015Yc;
    localparam A077Yc           =   5'd0;
    localparam A791Yc             =   5'd1;
    localparam A792Yc             =   5'd2;
    localparam A793Yc             =   5'd3;
    localparam A794Yc             =   5'd4;
    localparam A795Yc             =   5'd5;
    localparam A796Yc             =   5'd6;
    localparam A797Yc             =   5'd7;
    localparam A798Yc             =   5'd8;
    localparam A799Yc             =   5'd9;
    localparam A800Yc             =   5'd10;
    localparam A801Yc            =   5'd11;
    localparam A802Yc            =   5'd12;
    localparam A803Yc            =   5'd13;
    localparam A804Yc            =   5'd14;
    localparam A805Yc            =   5'd15;
    localparam A806Yc            =   5'd16;
    localparam A807Yc            =   5'd17;
    localparam A808Yc            =   5'd18;
    localparam A809Yc            =   5'd19;
    localparam A810Yc            =   5'd20;
    localparam A811Yc            =   5'd21;
    localparam A812Yc            =   5'd22;
    localparam A813Yc            =   5'd23;
    localparam A814Yc            =   5'd24;
    localparam A815Yc            =   5'd25;
    localparam A816Yc            =   5'd26;
    localparam A817Yc            =   5'd27;
    localparam A818Yc            =   5'd28;
    localparam A819Yc            =   5'd29;
    localparam A820Yc            =   5'd30;
    localparam A821Yc            =   5'd31;  
    localparam A822Yc               = A715Yc            /32;
    localparam [4:0] A823Yc                = (A715Yc            /32)-5'b1;
    localparam A824Yc          = {A822Yc              {1'b1}};
    localparam A825Yc               = A715Yc            /8;
    localparam [A221Yc     -1:0] A826Yc       = ({( 512/32){1'b1}} << ((A220Yc      - 512)/32)) +{A221Yc     {1'b0}};
    localparam [A221Yc     -1:0] A827Yc       = ({( 576/32){1'b1}} << ((A220Yc      - 576)/32)) +{A221Yc     {1'b0}};
    localparam [A221Yc     -1:0] A828Yc       = ({( 832/32){1'b1}} << ((A220Yc      - 832)/32)) +{A221Yc     {1'b0}};
    localparam [A221Yc     -1:0] A829Yc       = ({(1024/32){1'b1}} << ((A220Yc      -1024)/32)) +{A221Yc     {1'b0}};
    localparam [A221Yc     -1:0] A830Yc       = ({(1088/32){1'b1}} << ((A220Yc      -1088)/32)) +{A221Yc     {1'b0}};
    localparam [A221Yc     -1:0] A831Yc       = ({(1152/32){1'b1}} << ((A220Yc      -1152)/32)) +{A221Yc     {1'b0}};
    localparam A832Yc           = A715Yc            *(
                                    (A220Yc       + A715Yc             - 1) / A715Yc            );
    localparam A833Yc           = A832Yc     /32;
    localparam          A834Yc    = 4'd8;
    localparam          A835Yc      = A834Yc    *76;
    localparam          A836Yc     = A834Yc    *144;
    reg     [4:0]         A082Yc       ,A083Yc                  ;
    wire                                A837Yc                  ;
    wire                                A838Yc                  ;
    wire                                A839Yc                  ;
    wire                                A840Yc                  ;
    wire                                A841Yc                  ;
    wire                                A842Yc                  ;
    wire                                A843Yc                  ;
    wire                                A844Yc                  ;
    wire                                A845Yc                  ;
    wire                                A846Yc                  ;
    wire                                A847Yc                  ;
    wire                                A848Yc                  ;
    wire                                A849Yc                  ;
    wire                                A850Yc                  ;
    wire                                A851Yc                  ;
    wire                                A852Yc                  ;
    wire                                A853Yc                  ;
    wire                                A854Yc                  ;
    wire                                A855Yc                  ;
    wire                                A856Yc                  ;
    wire                                A857Yc                  ;
    wire                                A858Yc                  ;
    wire                                A859Yc                  ;
    wire                                A860Yc                  ;
    wire                                A861Yc                  ;
    wire                                A862Yc                  ;
    wire                                A863Yc                  ;
    wire                                A864Yc                  ;
    wire                                A865Yc                  ;
    wire                                A866Yc                  ;
    wire                                A867Yc                  ;
    wire                                A868Yc                  ;
    wire                                A869Yc                  ;
    wire                                A870Yc                  ;
    reg     [127:0]                     A871Yc                  ;
    wire    [127:0]                     A872Yc                  ;
    reg     [127:0]                     A873Yc                  ;
    wire    [127:0]                     A874Yc                  ;
    wire                                A875Yc                  ;
    reg     [31:0]                      A876Yc                  ;
    wire    [31:0]                      A877Yc                  ;
    reg     [31:0]                      A878Yc                  ;
    wire    [31:0]                      A879Yc                  ;
    wire                                A880Yc                  ;
    reg     [15:0]                      A881Yc                  ;
    wire    [15:0]                      A882Yc                  ;
    reg                                 A883Yc                  ;
    wire                                A884Yc                  ;
    reg     [10:0]                      A885Yc                  ;
    wire    [10:0]                      A886Yc                  ;
    wire    [10:0]                      A887Yc                  ;
    wire                                A888Yc                  ;
    wire                                A889Yc                  ;
    reg                                 A890Yc                  ;
    reg                                 A891Yc                  ;
    wire                                A892Yc                  ;
    wire                                A893Yc                  ;
    reg                                 A894Yc                  ;
    wire                                A895Yc                  ;
    wire                                A896Yc                  ;
    wire    [A222Yc        -1:0]        A897Yc                  ;
    wire    [A222Yc        -1:0]        A898Yc                  ;
    reg     [A222Yc        -1:0]        A899Yc                  ;
    wire    [A222Yc        -1:0]        A900Yc                  ;
    wire                                A901Yc                  ;
    wire    [A221Yc     -1:0]           A902Yc                  ;
    wire                                A903Yc                  ;
    wire                                A904Yc                  ;
    reg                                 A905Yc                  ;
    wire                                A906Yc                  ;
    reg                                 A907Yc                  ;
    wire                                A908Yc                  ;
    wire                                A909Yc                  ; 
    reg     [4:0]                       A910Yc                  ;
    wire    [4:0]                       A911Yc                  ;
    wire    [3:0]                       A912Yc                  ; 
    wire                                A913Yc                  ;
    wire                                A914Yc                  ;
    wire    [A223Yc       -1:0]         A915Yc                  ;
    wire                                A916Yc                  ;
    wire                                A917Yc                  ;
    wire                                A918Yc                  ;
    wire    [127:0]                     A919Yc                  ;
    wire                                A920Yc                  ;
    wire                                A921Yc                  ;
    wire                                A922Yc                  ;
    reg                                 A923Yc                  ;
    reg                                 A924Yc                  ;
    reg                                 A925Yc                  ;
    wire                                A926Yc                  ;
    wire                                A927Yc                  ;
    wire                                A928Yc                  ;
    wire                                A929Yc                  ;
    wire    [A714Yc            -1:0]    A930Yc                  ;
    wire    [A714Yc            -1:0]    A931Yc                  ;
    wire    [A714Yc            -1:0]    A932Yc                  ;
    wire    [A714Yc            -1:0]    A933Yc                   ;
    wire                                A934Yc                  ;
    wire                                A935Yc                  ;
    wire                                A936Yc                  ;
    wire    [A715Yc            -1:0]    A937Yc                  ;
    wire                                A938Yc                  ;
    wire                                A939Yc                  ;
    reg                                 A940Yc                  ;
    wire                                A941Yc                  ;
    wire    [A715Yc            -1:0]    A942Yc                  ;
    wire                                A943Yc                  ;
    wire    [A715Yc            -1:0]    A944Yc                  ;
    wire                                A945Yc                  ;
    wire                                A946Yc                  ;
    wire    [5:0]                       A947Yc                  ;
    reg     [5:0]                       A948Yc                  ;
    wire    [5:0]                       A949Yc                  ;
    wire    [A715Yc            -1:0]    A950Yc                  ;
    wire                                A951Yc                  ;
    reg     [A832Yc     -1:0]           A952Yc                  ;
    wire    [A832Yc     -1:0]           A953Yc                  ;
    wire    [A220Yc      -1:0]          A954Yc                  ;
    wire    [A222Yc        -1:0]        A955Yc                  ;
    wire    [511:0]                     A956Yc                  ;
    always @(posedge clk_core or negedge rst_n_core)
    begin
        if(!rst_n_core) begin
            A082Yc        <= A077Yc;
        end
        else begin
            A082Yc        <=  A083Yc    ;
        end
    end 
    always @*
    begin
        A083Yc     = A082Yc       ;
        case(A082Yc       )
            A077Yc  : begin
                        A083Yc     = 
                            A893Yc         && A724Yc   ? A807Yc :
                            A893Yc         ? 
                            (
                            A861Yc     ? 
                                (
                                A722Yc    && A870Yc      ? 
                                    (
                                    A903Yc          ? A791Yc :
                                                      A793Yc
                                    ) :
                                A722Yc    && !A870Yc     ? 
                                    (
                                    A903Yc          ? A791Yc :
                                                      A797Yc
                                    ) :
                                !A722Yc   && !A870Yc     ? 
                                    (
                                    A903Yc          ? A791Yc :
                                                      A797Yc
                                    ) :
                                                      A077Yc 
                                ) :
                            A862Yc     ? 
                                (
                                A903Yc          ? A804Yc :
                                                  A817Yc
                                ) :
                                A077Yc
                            ) :
                            ((A896Yc          && A725Yc ) ? A821Yc : A077Yc);
                      end
            A791Yc    : begin
                        A083Yc     = A792Yc;
                      end
            A792Yc    : begin
                        A083Yc     = A927Yc          ? A811Yc :
                                                       A792Yc;
                      end
            A793Yc    : begin
                        A083Yc     = 
                            A951Yc                     ? A794Yc : 
                            A884Yc       && A880Yc     ? A794Yc :
                            A884Yc                     ? A795Yc :
                                                         A793Yc;
                      end
            A794Yc    : begin
                        A083Yc     = A802Yc;
                      end
            A802Yc   : begin
                        A083Yc     = 
                            A927Yc           && A906Yc       ? A791Yc  :
                            A927Yc                           ? A811Yc :
                                                               A802Yc;
                      end
            A795Yc    : begin
                        A083Yc     = A796Yc;
                      end
            A796Yc    : begin
                        A083Yc     = 
                            A927Yc          ? A793Yc :
                                              A796Yc;
                      end
            A797Yc    : begin
                        A083Yc     = 
                            A884Yc       && A875Yc     ? A798Yc :
                            A884Yc                     ? A799Yc :
                                                         A797Yc;
                      end
            A798Yc    : begin
                        A083Yc     = A801Yc;
                      end
            A801Yc   : begin
                        A083Yc     = 
                            A927Yc          && A906Yc       ? A791Yc  :
                            A927Yc                          ? A811Yc :
                                                              A801Yc;
                      end
            A799Yc    : begin
                        A083Yc     = A800Yc;
                      end
            A800Yc    : begin
                        A083Yc     = 
                            A927Yc          ? A797Yc :
                                              A800Yc;
                      end
            A807Yc   : begin
                        A083Yc     = A808Yc;
                      end
            A808Yc   : begin
                        A083Yc     = A747Yc    ? A811Yc :
                                                 A808Yc;
                      end
            A804Yc   : begin
                        A083Yc     = A805Yc;
                      end
            A805Yc   : begin
                        A083Yc     = A927Yc          && A112Yc && !A363Yc             ? A806Yc :
                                     A927Yc                                           ? A811Yc :
                                                                                        A805Yc;
                      end
            A806Yc   : begin
                        A083Yc     = A809Yc;
                      end
            A809Yc   : begin
                        A083Yc     = A949Yc          ==A947Yc        ? A810Yc :
                                                    A809Yc;
                      end
            A810Yc   : begin
                        A083Yc     = !A775Yc       ? A811Yc :
                                                     A810Yc;
                      end
            A812Yc   : begin
                        A083Yc     = 
                            A757Yc    && A068Yc    ? ((!A769Yc       || A925Yc            ) ? A811Yc : A812Yc) :
                            A757Yc    && A884Yc       && A875Yc          ? A813Yc:
                            A757Yc    && A884Yc                          ? A815Yc:
                                                                           A812Yc;
                      end
            A813Yc   : begin
                        A083Yc     = A814Yc;
                      end
            A814Yc   : begin
                        A083Yc     = 
                            A927Yc          && A906Yc       && (!A363Yc            ) ? A804Yc :
                            A927Yc          && A112Yc && (!A363Yc            )       ? A806Yc :
                            A927Yc                                                   ? A811Yc :
                                                                                       A814Yc;
                      end
            A815Yc   : begin
                        A083Yc     = A816Yc;
                      end
            A816Yc   : begin
                        A083Yc     = A812Yc;
                      end
            A817Yc   : A083Yc     = A068Yc    ? A811Yc : A812Yc;
            A811Yc   : begin
                        A083Yc     = A077Yc;
                      end
            A821Yc   : begin
                        A083Yc     = A077Yc;
                      end
            default : A083Yc     = A077Yc; 
        endcase
    end
    assign A837Yc      = A082Yc        == A077Yc;      
    assign A838Yc      = A082Yc        == A791Yc ;       
    assign A839Yc      = A082Yc        == A793Yc ;       
    assign A840Yc      = A082Yc        == A794Yc ;       
    assign A841Yc      = A082Yc        == A795Yc ;       
    assign A842Yc      = A082Yc        == A797Yc ;       
    assign A843Yc      = A082Yc        == A798Yc ;       
    assign A844Yc      = A082Yc        == A799Yc ;       
    assign A845Yc      = A082Yc        == A801Yc;       
    assign A846Yc      = A082Yc        == A802Yc;       
    assign A847Yc      = A082Yc        == A804Yc;       
    assign A848Yc      = A082Yc        == A806Yc;       
    assign A849Yc      = A082Yc        == A808Yc;       
    assign A850Yc      = A082Yc        == A809Yc;       
    assign A851Yc      = A082Yc        == A810Yc;       
    assign A852Yc      = A082Yc        == A811Yc;       
    assign A853Yc      = A082Yc        == A812Yc;       
    assign A854Yc      = A082Yc        == A813Yc;       
    assign A855Yc      = A082Yc        == A814Yc;       
    assign A856Yc      = A082Yc        == A815Yc;       
    assign A857Yc      = A082Yc        == A816Yc;       
    assign A858Yc      = A082Yc        == A817Yc;       
    assign A859Yc      = A082Yc        == A821Yc;       
    assign A861Yc     = A718Yc   && !A725Yc ;
    assign A862Yc     = A718Yc   &&  A725Yc ;
    assign A863Yc       = (A111Yc == 4'b0001)          || 
                          (A111Yc == 4'b0000)          ||
                          (A111Yc == 4'b0101)         || 
                          (A111Yc == 4'b0010)       ||
                          (A111Yc == 4'b0110);
    assign A866Yc       = (A111Yc == 4'b0100)       || 
                          (A111Yc == 4'b0011)       ||
                          (A111Yc == 4'b0111)   || 
                          (A111Yc == 4'b1000);
    assign A864Yc       = (A111Yc == 4'b1100);
    assign A865Yc       = (A111Yc == 4'b1011);
    assign A867Yc       = (A111Yc == 4'b1010);
    assign A868Yc       = (A111Yc == 4'b1001);
    assign A869Yc        = 
            A861Yc      ? A740Yc      && A901Yc       :
            A862Yc      ? (A940Yc             && A901Yc       && (!A883Yc   )) :
                          1'b0;
    assign A870Yc     = A861Yc     ? A722Yc   && A737Yc     :
                                     1'b0;
    assign A872Yc       = 
            A731Yc                                     ? A730Yc    :
            A861Yc     && A869Yc        && !A870Yc     ? A871Yc    + 6'd32 :
            A862Yc     && A869Yc                       ? 
                A871Yc    + A887Yc    :
                                                         A871Yc   ;
    assign A875Yc    = A872Yc       >= A729Yc   ;
    assign A874Yc            = 
            A861Yc     ?
                (
                    A722Yc   && !A737Yc     && A893Yc                   ? 
                                            A729Yc     - A871Yc    :
                    !A722Yc   && !A737Yc     && A893Yc                  ?
                                            A729Yc     - A871Yc    :
                    A722Yc   && !A737Yc     && A883Yc                   ?
                                            A873Yc         - {112'd0,A881Yc   } :
                    !A722Yc   && !A737Yc     && A883Yc                  ?
                                            A873Yc         - {112'd0,A881Yc   } :
                                            A873Yc        
                ) :
            A862Yc     ? 
                (
                    A893Yc         ? A729Yc     - A871Yc    :
                    A916Yc         ? A873Yc         - {112'd0,A881Yc   } :
                                     A873Yc        
                ) :
                         128'd0;
    assign A887Yc     = 11'd32;
    assign A886Yc        = 
            A837Yc       ? 
                (
                    A863Yc       ? 11'd512  :
                    A864Yc       ? 11'd576  :
                    A865Yc       ? 11'd832  :
                    A866Yc       ? 11'd1024 :
                    A867Yc       ? 11'd1088 :
                    A868Yc       ? 11'd1152 :
                                   11'd0
                ) :
            A862Yc     ? 
                (
                    A916Yc      ? 
                        (
                            A863Yc       ? 11'd512  :
                            A864Yc       ? 11'd576  :
                            A865Yc       ? 11'd832  :
                            A866Yc       ? 11'd1024 :
                            A867Yc       ? 11'd1088 :
                            A868Yc       ? 11'd1152 :
                                           11'd0
                        ) :
                    A869Yc        ? A885Yc     - 8'd64 :
                        A885Yc    
                ) : 
                11'd0;
    assign A951Yc             = A082Yc        == A793Yc && A737Yc     && A722Yc   && A723Yc &&
                                A861Yc    ;
    assign A956Yc       = A733Yc    >=512 ? A738Yc   : 
                         ({512{1'b1}} << (512 - A733Yc   [8:0])) & A738Yc  ;
    assign A877Yc       = 
            A735Yc                                   ? A734Yc    :
            A861Yc     && A869Yc        && A870Yc     ? A876Yc    + 6'd32 :
                                                        A876Yc   ;
    assign A880Yc    = A877Yc       >= A733Yc   ;
    assign A879Yc            = 
            A861Yc     ?
                (
                A722Yc   && A737Yc     && A893Yc                    ? 
                                         A733Yc    - A876Yc         :
                A722Yc   && A737Yc     && A883Yc                    ?
                                         A878Yc         - {16'd0,A881Yc   } :
                                         A878Yc        
                ) :
            A862Yc     ? 32'd0 :
                         32'd0;
    assign A882Yc       = 
            A837Yc                      ? 16'd0 :
            A916Yc                      ? 16'd0 :
            A861Yc     && A869Yc        ? A881Yc    + 6'd32 :
            A862Yc     && A869Yc        ? A881Yc    + {5'd0,A887Yc    } :
                                          A881Yc   ;
    assign A884Yc       = 
            (A863Yc       && (A882Yc       >= 'd512 )) |
            (A864Yc       && (A882Yc       >= 'd576 )) |
            (A865Yc       && (A882Yc       >= 'd832 )) |
            (A866Yc       && (A882Yc       >= 'd1024)) |
            (A867Yc       && (A882Yc       >= 'd1088)) |
            (A868Yc       && (A882Yc       >= 'd1152)) |
            (A922Yc          && (A877Yc       >= A733Yc   )) |
            (!A922Yc          && (A872Yc       >= A729Yc   ));
    assign A909Yc     =
            A861Yc     ? 
                (
                    (A722Yc   && A737Yc     && (
                    (A863Yc       && ((A878Yc         < 'd512 ))) | 
                    (A864Yc       && ((A878Yc         < 'd576 ))) | 
                    (A865Yc       && ((A878Yc         < 'd832 ))) | 
                    (A866Yc       && ((A878Yc         < 'd1024))) | 
                    (A867Yc       && ((A878Yc         < 'd1088))) | 
                    (A868Yc       && ((A878Yc         < 'd1152))))) |
                    (A722Yc   && !A737Yc     && (
                    (A863Yc       && ((A873Yc         < 'd512 ))) | 
                    (A864Yc       && ((A873Yc         < 'd576 ))) | 
                    (A865Yc       && ((A873Yc         < 'd832 ))) | 
                    (A866Yc       && ((A873Yc         < 'd1024))) | 
                    (A867Yc       && ((A873Yc         < 'd1088))) | 
                    (A868Yc       && ((A873Yc         < 'd1152))))) |
                    (!A722Yc   && (
                    (A863Yc       && ((A873Yc         < 'd512 ))) | 
                    (A864Yc       && ((A873Yc         < 'd576 ))) | 
                    (A865Yc       && ((A873Yc         < 'd832 ))) | 
                    (A866Yc       && ((A873Yc         < 'd1024))) | 
                    (A867Yc       && ((A873Yc         < 'd1088))) | 
                    (A868Yc       && ((A873Yc         < 'd1152)))))
                ) :
                (
                    (A863Yc       && ((A873Yc         < 'd512 ))) | 
                    (A864Yc       && ((A873Yc         < 'd576 ))) | 
                    (A865Yc       && ((A873Yc         < 'd832 ))) | 
                    (A866Yc       && ((A873Yc         < 'd1024))) | 
                    (A867Yc       && ((A873Yc         < 'd1088))) | 
                    (A868Yc       && ((A873Yc         < 'd1152)))
                );
    assign A920Yc        = 1'b1;
    assign A892Yc         = A718Yc  ;
    assign A893Yc         = A892Yc         && (!A891Yc     );
    assign A895Yc          = A068Yc   ;
    assign A896Yc          = A895Yc          && (!A894Yc      );
    assign A900Yc        = A764Yc           ? A897Yc     : A899Yc    ;
    generate
        for(A014Yc=0;A014Yc<A223Yc       ;A014Yc=A014Yc+1) begin : A957Yc      
            assign A898Yc    [A014Yc*32+:32] = A899Yc    [(A223Yc       -1-A014Yc)*32+:32];
        end
    endgenerate
    wire [A221Yc     -1:0] A958Yc;
    assign A958Yc = ((A869Yc        << A221Yc     -1) >> A881Yc   [15:5]) +{A221Yc     {1'b0}};
    assign A902Yc      = 
            A861Yc     ? 
                A951Yc             ? {A221Yc     {1'b1}} : 
                (
                    A722Yc    && A737Yc     && A723Yc ? {A221Yc     {1'b0}} : A958Yc[A221Yc     -1:0]
                ) :
            A862Yc     ? 
                (
                    A863Yc      ? 
                    ({A221Yc     {A854Yc      || A856Yc     }} & A826Yc    ) :
                    A864Yc      ? 
                    ({A221Yc     {A854Yc      || A856Yc     }} & A827Yc    ) :
                    A865Yc      ? 
                    ({A221Yc     {A854Yc      || A856Yc     }} & A828Yc    ) :
                    A866Yc      ? 
                    ({A221Yc     {A854Yc      || A856Yc     }} & A829Yc     ):
                    A867Yc      ? 
                    ({A221Yc     {A854Yc      || A856Yc     }} & A830Yc     ):
                    A868Yc      ? 
                    ({A221Yc     {A854Yc      || A856Yc     }} & A831Yc     ):
                                                            {A221Yc     {1'b0}}
                ) :
                         {A221Yc     {1'b0}};
    wire [A220Yc      -1:0] A959Yc;
    wire [A832Yc     -1:0]  A960Yc;
    assign A959Yc = ((A739Yc << (A221Yc     -1)*32) >> (A881Yc   [15:5]*32)) + {A220Yc      {1'b0}};
    assign A960Yc = A952Yc        << (A220Yc       - A881Yc   );
    assign A954Yc     = 
            A861Yc     ? 
                A951Yc             ? {A956Yc      ,{A220Yc      -512{1'b0}}} : 
                (
                     A959Yc[A220Yc      -1:0]
                ) :
                (
                     A960Yc[A220Yc      -1:0] 
                );
    assign A901Yc       = 
            A861Yc     ? A837Yc       | A839Yc     | A842Yc     | A845Yc      :
            A862Yc     ? A837Yc       | A853Yc      | A855Yc      :
                         1'b1;
    assign A926Yc          = A757Yc    && A747Yc   ;
    assign A927Yc          = A926Yc          && (!A923Yc      );
    assign A929Yc              = A935Yc        && (!A924Yc          );
    assign A914Yc        = A849Yc     ;
    assign A917Yc            = A855Yc      & A906Yc       & (!A363Yc            );
    assign A916Yc      =
            A861Yc     ? 
                (
                    A838Yc     | A841Yc     | A844Yc     | A843Yc     | 
                    A840Yc     | A847Yc      
                ) :
            A862Yc     ? 
                (
                    A847Yc      | A854Yc      | A856Yc     
                ) :
                    1'b0;
    assign A918Yc     = 
            A861Yc     ? 
                (
                    A722Yc      && A737Yc        ? A723Yc ? 1 : A112Yc && A909Yc     :
                    A722Yc      && (!A737Yc    ) ? A112Yc && A909Yc     :
                    (!A722Yc  ) && (!A737Yc    ) ? A112Yc && A909Yc     :
                                                   1'b0 
                ) :
            A862Yc     ? 
                (
                    A112Yc && A909Yc    
                ) :
                    1'b0;
    assign A919Yc    = 
            A861Yc     ? 
                (
                A722Yc      && A737Yc        ? {96'd0,A733Yc   } :
                A722Yc      && (!A737Yc    ) ? A729Yc    :
                (!A722Yc  ) && (!A737Yc    ) ? 
                    (
                     !(A863Yc      | A866Yc      ) ? A873Yc         : 
                                                     A729Yc   
                    ) :
                         1'b0 
                ) :
            A862Yc     ? 
                (
                A863Yc      | A866Yc       ? A729Yc    :
                                             A873Yc        
                ) :
                         1'b0;
    assign A922Yc          = 
            A861Yc     ? 
                (
                A722Yc      && A737Yc        ? 1'b1 :
                A722Yc      && (!A737Yc    ) ? 1'b0 :
                (!A722Yc  ) && (!A737Yc    ) ? 1'b0 :
                                               1'b0 
                ) :
            A862Yc     ? 1'b0 :
                         1'b0;
    assign A903Yc          = 
            A861Yc     ? 
                (
                A722Yc      && A737Yc        ? A733Yc    == A876Yc    :
                A722Yc      && (!A737Yc    ) ? A729Yc    == A871Yc    :
                (!A722Yc  ) && (!A737Yc    ) ? A729Yc    == A871Yc    :
                                               1'b0 
                ) :
            A862Yc     ? A729Yc    == A871Yc    :
                         1'b0;
    assign A904Yc              = (A082Yc        == A077Yc) ? (A729Yc    == A871Yc   ) : A905Yc             ;                       
    assign A906Yc      = 
            A861Yc     && A112Yc ? 
                (
                A722Yc      && A737Yc        ? 
                    (A863Yc       && (A733Yc    > 'd0) && (A733Yc   [8:0] == 'd0)) |
                    (A866Yc       && (A733Yc    > 'd0) && (A733Yc   [9:0] == 'd0)) :
                A722Yc      && (!A737Yc    ) ? 
                    (A863Yc       && (A729Yc    > 'd0) && (A729Yc   [8:0] == 'd0)) |
                    (A866Yc       && (A729Yc    > 'd0) && (A729Yc   [9:0] == 'd0)) :
                (!A722Yc  ) && (!A737Yc    ) ? 
                    (
                     !(A863Yc      | A866Yc      ) ? 
                        (A729Yc    > 'd0) && A907Yc            : 
                        (A863Yc       && (A729Yc    > 'd0) && (A729Yc   [8:0] == 'd0)) |
                        (A866Yc       && (A729Yc    > 'd0) && (A729Yc   [9:0] == 'd0)) 
                    ) :
                    1'b0
                ) :
            A862Yc     && A112Yc ?
                (
                    !(A863Yc      | A866Yc      ) ? 
                        (A729Yc    > 'd0) && A907Yc            : 
                        (A863Yc       && (A729Yc    > 'd0) && (A729Yc   [8:0] == 'd0)) |
                        (A866Yc       && (A729Yc    > 'd0) && (A729Yc   [9:0] == 'd0)) 
                ) :
                         1'b0;
    assign A908Yc               = 
            A861Yc     ? 
                (
                    A916Yc      ? 
                    (
                        (A864Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd576)) |
                        (A865Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd832)) |
                        (A867Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd1088)) |
                        (A868Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd1152)) 
                    ) :
                    A907Yc           
                ) :
            A862Yc     ? 
                (
                    A916Yc      ? 
                    (
                        (A864Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd576)) |
                        (A865Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd832)) |
                        (A867Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd1088)) |
                        (A868Yc       && (A729Yc    > 'd0) && (A873Yc         == 'd1152)) 
                    ) :
                    A907Yc           
                ) :
                         1'b0;
    assign A888Yc        = A852Yc      || A859Yc     ;
    assign A889Yc        = (A852Yc      && A068Yc   ) || A859Yc     ;
    always @(posedge clk_core or negedge rst_n_core)
    begin
        if(!rst_n_core) begin
            A890Yc       <= 1'b0;
        end
        else if(A852Yc      && A068Yc   ) begin
           A890Yc       <= 1'b1;
        end
        else if(A859Yc     ) begin
           A890Yc       <= 1'b0;
        end
    end
    assign A934Yc        = A068Yc    ? 1'b0 : A858Yc     ;
    assign A943Yc        = A848Yc     ;
    assign A935Yc        = A068Yc    && A769Yc       && (A853Yc      || A856Yc      || A857Yc     );
    assign A936Yc             = A776Yc             ? 1'b1 :
                                A852Yc             ? 1'b0 :
                                                     A925Yc            ;
    assign A938Yc          = 
            A861Yc     ? 1'b0 :
            A862Yc     ? A853Yc      && (A910Yc       == 5'd0) && (!A939Yc            ) && 
                         (!A940Yc            ) && (!A884Yc      ) :
                         1'b0;
    assign A941Yc                = 
            A837Yc                                 ? 1'b0 :
            A938Yc          && !A939Yc             ? 1'b1 :
            A869Yc        && (A910Yc       == A823Yc               )
                                                   ? 1'b0 :
                                                     A940Yc            ;
    assign A945Yc          = 
            A861Yc     ? 1'b0 :
            A862Yc     ? A850Yc      :
                         1'b0;
    assign A949Yc           = 
            A837Yc                            ? 6'd0 :
            A850Yc      && !A946Yc            ? A948Yc        + 1'b1 :
                                                A948Yc       ;
    assign A947Yc        = 
            (A111Yc == 4'b0001         ) ? (128+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0000         ) ? (256+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0101        ) ? (160+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0010      ) ? (256+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0110      ) ? (224+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0100      ) ? (512+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0011      ) ? (384+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b0111  ) ? (224+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b1000  ) ? (256+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b1100    ) ? (512+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b1011    ) ? (384+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b1010    ) ? (256+A715Yc            -1)/A715Yc             :
            (A111Yc == 4'b1001    ) ? (224+A715Yc            -1)/A715Yc             :
                                        6'd0;
    assign A944Yc               = 
            A899Yc    [(A222Yc        -A948Yc       *A715Yc            -1)
                       -:A715Yc            ];
    assign A953Yc          [0*32+:32] = 
                A869Yc        ? A937Yc              [(A715Yc            -1)-A910Yc      *32-:32] :
                                A952Yc       [0*32+:32];
    generate
    for(A014Yc=1;A014Yc<A833Yc     ;A014Yc=A014Yc+1) begin : A961Yc      
        assign A953Yc          [A014Yc*32+:32] = 
                A869Yc        ? A952Yc       [(A014Yc-1)*32+:32] :
                                A952Yc       [A014Yc*32+:32];
    end
    endgenerate
    assign A911Yc          = 
            A861Yc     ? 5'd0 :
            A862Yc     ? 
                (
                    A853Yc      && A869Yc        && 
                    (A910Yc       == A823Yc               ) ? 5'd0 :
                    A853Yc      && A869Yc               ? A910Yc       + 1'b1 : 
                                                          A910Yc      
                ) :
                         5'd0;
    assign A930Yc    = {
                        A852Yc      && (!A725Yc ) && (A875Yc    || A880Yc   ),
                        ((A852Yc      && A725Yc ) || A859Yc     ), 
                        ((A852Yc      && A725Yc  && A068Yc   ) || A859Yc     )
                        };
    assign A931Yc    = {
                        A781Yc         ,
                        A782Yc        ,
                        A783Yc            
                        };
    assign {
            A784Yc    ,
            A785Yc   ,
            A786Yc       
           }         = A789Yc;
    assign A932Yc    = {
                        A726Yc      ,
                        A727Yc     ,
                        A728Yc         
                        };
    assign A933Yc = A932Yc  & A789Yc;
    always@(posedge clk_core or negedge rst_n_core)
    begin
        if (!rst_n_core) begin
            A871Yc              <= 128'd0;
            A876Yc              <= 32'd0;
            A891Yc              <= 1'd0;
            A894Yc              <= 1'd0;
            A881Yc              <= 16'd0;
            A899Yc              <= {A222Yc        {1'd0}};
            A883Yc              <= 1'd0;
            A923Yc              <= 1'd0;
            A924Yc              <= 1'd0;
            A925Yc              <= 1'd0;
            A873Yc              <= 128'd0;
            A878Yc              <= 32'd0;
            A907Yc              <= 1'd0;
            A940Yc              <= 1'd0;
            A948Yc              <= 6'd0;
            A885Yc              <= 11'd0;
            A952Yc              <= {A220Yc      {1'b0}};
            A910Yc              <= {5{1'b0}};
            A905Yc              <= 1'd0;
        end
        else begin
            A871Yc              <=  A872Yc      ;
            A876Yc              <=  A877Yc      ;
            A891Yc              <=  A892Yc        ;
            A894Yc              <=  A895Yc         ;
            A881Yc              <=  A882Yc      ;
            A899Yc              <=  A900Yc       ;
            A883Yc              <=  A884Yc      ;
            A923Yc              <=  A926Yc         ;
            A924Yc              <=  A935Yc       ;
            A925Yc              <=  A936Yc            ;
            A873Yc              <=  A874Yc           ;
            A878Yc              <=  A879Yc           ;
            A907Yc              <=  A908Yc              ;
            A940Yc              <=  A941Yc               ;
            A948Yc              <=  A949Yc          ;
            A885Yc              <=  A886Yc       ;
            A952Yc              <=  A953Yc          ;
            A910Yc              <=  A911Yc         ;
            A905Yc              <=  A904Yc             ;
        end
    end
    assign A860Yc       = A852Yc      && A862Yc    ;
    generate 
    if(A717Yc  ==1) begin
hash_hp_A962Yc             #(
      .A963Yc         (A716Yc                 )
   ,  .A964Yc         (A715Yc                 ) 
    )
    A965Yc       
    (
      .i_rd_clk       (clk_core               )
   ,  .i_rd_rst_n     (rst_n_core             )
   ,  .A966Yc           (A938Yc                 )
   ,  .A967Yc         (A937Yc                 )
   ,  .A968Yc         ()
   ,  .A969Yc         (A939Yc                 )
   ,  .A970Yc         (A860Yc                 )
   ,  .i_wr_clk       (clk_axi                )
   ,  .i_wr_rst_n     (rst_n_axi              )
   ,  .A971Yc           (A771Yc                 )
   ,  .A972Yc         (A942Yc                 )
   ,  .A973Yc         (A773Yc                 )
   ,  .A974Yc         (A772Yc                 )
   ,  .A975Yc         (A765Yc                 )
    );
    end
    else begin
hash_hp_A976Yc            
    #(
      .A963Yc         (A716Yc                 )
   ,  .A964Yc         (A715Yc                 )
    )
    A965Yc       
    (
      .i_clk          (clk_core               )
   ,  .i_rst_n        (rst_n_core             )
   ,  .A971Yc           (A771Yc                 )
   ,  .A972Yc         (A942Yc                 )
   ,  .A973Yc         (A773Yc                 )
   ,  .A974Yc         (A772Yc                 )
   ,  .A966Yc           (A938Yc                 )
   ,  .A967Yc         (A937Yc                 )
   ,  .A968Yc         ()
   ,  .A969Yc         (A939Yc                 )
   ,  .A110Yc         (A765Yc                 )
    );
    wire                         A977Yc        ;
    wire                         A978Yc        ;
    assign A977Yc         =      clk_axi       ;
    assign A978Yc         =      rst_n_axi     ;
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<A825Yc              ;A014Yc=A014Yc+1) begin : A979Yc      
        assign A942Yc              [A014Yc*8+:8] = 
                A770Yc              [(A825Yc              -A014Yc-1)*8+:8];
    end
    endgenerate
    generate 
    if(A717Yc  ==1) begin
hash_hp_A962Yc             #(
      .A963Yc         (A716Yc                 )
   ,  .A964Yc         (A715Yc                 ) 
    )
    A980Yc       
    (
      .i_rd_clk       (clk_axi                )
   ,  .i_rd_rst_n     (rst_n_axi              )
   ,  .A966Yc           (A778Yc                 )
   ,  .A967Yc         (A950Yc                 )
   ,  .A968Yc         (A780Yc                 )
   ,  .A969Yc         (A779Yc                 )
   ,  .A970Yc         (1'b0                   )
   ,  .i_wr_clk       (clk_core               )
   ,  .i_wr_rst_n     (rst_n_core             )
   ,  .A971Yc           (A945Yc                 )
   ,  .A972Yc         (A944Yc                 )
   ,  .A973Yc         ()
   ,  .A974Yc         (A946Yc                 )
   ,  .A975Yc         (1'b0                   )
    );
    end
    else begin
hash_hp_A976Yc            
    #(
      .A963Yc         (A716Yc                 )
   ,  .A964Yc         (A715Yc                 )
    )
    A980Yc       
    (
      .i_clk          (clk_core               )
   ,  .i_rst_n        (rst_n_core             )
   ,  .A971Yc           (A945Yc                 )
   ,  .A972Yc         (A944Yc                 )
   ,  .A973Yc         ()
   ,  .A974Yc         (A946Yc                 )
   ,  .A966Yc           (A778Yc                 )
   ,  .A967Yc         (A950Yc                 )
   ,  .A968Yc         (A780Yc                 )
   ,  .A969Yc         (A779Yc                 )
   ,  .A110Yc         (1'b0                   )
    );
    end
    endgenerate
    generate
    for(A014Yc=0;A014Yc<A825Yc              ;A014Yc=A014Yc+1) begin : A981Yc      
        assign A777Yc              [A014Yc*8+:8] = 
                A950Yc              [(A825Yc              -A014Yc-1)*8+:8];
    end
    endgenerate
    assign A912Yc           = A111Yc;
    assign A913Yc           = A722Yc  ;
    assign A915Yc         = {A223Yc       {A155Yc     }} &
            ({{(A223Yc       -1){1'b0}},1'b1} << (A223Yc       -1)) >> A742Yc        ;
    assign A955Yc     = ((A741Yc    << (A223Yc       -1)*32) >> (A742Yc        *32)) + {A222Yc        {1'b0}};
    assign A928Yc               = A068Yc    && A725Yc   ;
    assign A897Yc     = A761Yc      ;    
    assign A719Yc               = A888Yc                ;
    assign A720Yc               = A889Yc                ;
    assign A721Yc               = A890Yc                ;
    assign A732Yc               = A871Yc                ;
    assign A736Yc               = A876Yc                ;
    assign A744Yc               = A901Yc                ;
    assign A743Yc               = A898Yc                ;
    assign A750Yc               = A917Yc                ;
    assign A753Yc               = A928Yc                ;
    assign A752Yc               = A893Yc                ;
    assign A751Yc               = A906Yc                ;
    assign A745Yc               = A912Yc                ;
    assign A746Yc               = A913Yc                ;
    assign A748Yc               = A914Yc                ;
    assign A749Yc               = A915Yc                ;
    assign A754Yc               = A916Yc                ;
    assign A755Yc               = A918Yc                ;
    assign A756Yc               = A919Yc                ;
    assign A758Yc               = A920Yc                ;
    assign A759Yc               = A954Yc                ;
    assign A760Yc               = A955Yc                ;
    assign A762Yc               = A902Yc                ;
    assign A763Yc               = A922Yc                ;
    assign A766Yc               = A860Yc                ;
    assign A767Yc               = A929Yc                ;
    assign A768Yc               = A934Yc                ;
    assign A774Yc               = A943Yc                ;
    assign A787Yc               = A930Yc                ;
    assign A788Yc               = A931Yc                ;
    assign A706Yc                = |A933Yc                ;
    assign A790Yc               = !A851Yc                ;
endmodule
module hash_hp_A982Yc             
(
    input  wire     clk
   ,input  wire     rst_n
   ,input  wire     A152Yc
   ,output wire     A336Yc
);
    reg A624Yc;
    always @(posedge clk or negedge rst_n)
    if (!rst_n) begin
        A624Yc <= 1'b0;
    end
    else begin
        A624Yc <=  A152Yc;
    end
    assign A336Yc = A152Yc && !A624Yc;
endmodule 
module hash_hp_A983Yc                 (
    input  wire      clk_a
   ,input  wire      A984Yc
   ,input  wire      A985Yc 
   ,output wire      A986Yc 
   ,input  wire      clk_b
   ,input  wire      A987Yc
   ,output wire      A988Yc 
   ,input  wire      A989Yc 
);
   reg  A990Yc ;
   wire A991Yc ;
   wire A992Yc ;
   assign A991Yc  = A992Yc  ? 1'b0 :
                    A985Yc  ? 1'b1 :
                    A990Yc ;
   always @(posedge clk_a or negedge A984Yc)
   if (!A984Yc) begin
        A990Yc  <= 1'b0;
   end
   else begin
        A990Yc  <=  A991Yc ;
   end
   wire A993Yc  ;
   reg A994Yc  ;
hash_hp_A995Yc           A996Yc       (
          .clk        (clk_b        )
         ,.rst_n      (A987Yc        )    
         ,.A152Yc     (A990Yc       )
         ,.A336Yc     (A993Yc       )
   );
   always @(posedge clk_b or negedge A987Yc)
   if (!A987Yc) begin
        A994Yc   <= 1'b0;
   end
   else begin
        A994Yc   <= A993Yc  ;
   end 
   reg  A997Yc ;
   wire A998Yc ;
   assign A998Yc  = A993Yc   && !A994Yc   ? 1'b1 :
                    A989Yc                ? 1'b0 :
                    A997Yc ;
   assign A988Yc  = A997Yc ;
   reg   A999Yc ;
   wire  Y000Zc ;
   assign Y000Zc  =  A989Yc  && A997Yc  ? 1'b1 :
                    !A993Yc   ? 1'b0 :
                     A999Yc ;
   always @(posedge clk_b or negedge A987Yc)
   if (!A987Yc) begin
        A999Yc  <= 1'b0;
        A997Yc  <= 1'b0;
   end
   else begin
        A999Yc  <=  Y000Zc ;
        A997Yc  <=  A998Yc ;
   end
   wire  Y001Zc  ;
   reg   Y002Zc  ;
hash_hp_A995Yc           Y003Zc       (
          .clk        (clk_a        )
         ,.rst_n      (A984Yc        )    
         ,.A152Yc     (A999Yc       )
         ,.A336Yc     (Y001Zc       )
   );
   always @(posedge clk_a or negedge A984Yc)
   if (!A984Yc) begin
        Y002Zc   <= 1'b0; 
   end
   else begin
        Y002Zc   <=  Y001Zc  ;
   end
   assign A992Yc  = Y001Zc  ;
   assign A986Yc  = Y002Zc   && !Y001Zc  ;
endmodule 
module hash_hp_A995Yc          (
    input  wire clk
   ,input  wire rst_n 
   ,input  wire A152Yc
   ,output wire A336Yc
);
    reg Y004Zc ;
    reg Y005Zc ;
    always @(posedge clk or negedge rst_n)
    if (!rst_n) begin
        Y004Zc  <= 1'b0;
        Y005Zc  <= 1'b0;
    end
    else begin
        Y004Zc  <=  A152Yc;
        Y005Zc  <=  Y004Zc ;
    end
    assign A336Yc = Y005Zc ;
endmodule 
module hash_hp_Y006Zc         #(
    parameter A963Yc   = 10 ,       
    parameter A964Yc   = 32
    )(
    input  wire                  i_wr_clk          ,
    input  wire                  i_wr_rst_n        ,    
    input  wire                  A971Yc              , 
    input  wire [A963Yc  -1:0]   Y007Zc            , 
    input  wire [A964Yc  -1:0]   A972Yc            , 
    input  wire                  i_rd_clk          ,
    input  wire                  i_rd_rst_n        ,   
    input  wire                  A966Yc              , 
    input  wire [A963Yc  -1:0]   Y008Zc            , 
    output wire [A964Yc  -1:0]   A967Yc             
    );
    localparam Y009Zc          = 2**(A963Yc  )     ;
    integer                      A014Yc;
    integer                      A015Yc;
    genvar                       A623Yc;
    genvar                       Y010Zc;
    reg  [A964Yc  -1:0]          Y011Zc        [Y009Zc -1:0];
    wire [A964Yc  -1:0]          Y012Zc     [Y009Zc -1:0];
    wire [A964Yc  -1:0]          Y013Zc     [Y009Zc -1:0];
    wire                         Y014Zc              ;
    wire [A963Yc  -1:0]          Y015Zc              ;
    wire [A964Yc  -1:0]          Y016Zc              ;
    wire                         Y017Zc              ;
    wire [A963Yc  -1:0]          Y018Zc              ;
    reg  [A964Yc  -1:0]          Y019Zc              ;
    wire [A964Yc  -1:0]          Y020Zc              ;
    always@(posedge i_wr_clk or negedge i_wr_rst_n)
    begin
        if(~i_wr_rst_n) begin
            for(A014Yc=0;A014Yc<Y009Zc ;A014Yc=A014Yc+1) begin
                    Y011Zc[A014Yc] <= {(A964Yc  ){1'b0}};
            end
        end
        else begin
            for(A014Yc=0;A014Yc<Y009Zc ;A014Yc=A014Yc+1) begin
                    Y011Zc[A014Yc] <=   Y012Zc[A014Yc];
            end
        end
    end
    generate
        for(A623Yc=0;A623Yc<Y009Zc ;A623Yc=A623Yc+1) begin:Y021Zc         
                assign Y012Zc[A623Yc] = (A623Yc==Y015Zc     )&Y014Zc ? Y016Zc      : Y011Zc[A623Yc];
        end
    endgenerate
    generate
        for(A623Yc=0;A623Yc<Y009Zc ;A623Yc=A623Yc+1) begin:Y022Zc          
            assign Y013Zc [A623Yc] =  Y011Zc[A623Yc];
        end
    endgenerate
    always@(posedge i_rd_clk or negedge i_rd_rst_n)
    begin
        if(~i_rd_rst_n) begin
            Y019Zc     <= {(A964Yc  ){1'b0}};
        end
        else begin
            Y019Zc     <=  Y020Zc         ;
        end
    end
    assign Y020Zc        = (Y017Zc) ? Y013Zc [Y018Zc     ] : Y019Zc      ;
    assign Y014Zc        = A971Yc     ; 
    assign Y015Zc        = Y007Zc   ; 
    assign Y016Zc        = A972Yc   ; 
    assign Y017Zc        = A966Yc     ; 
    assign Y018Zc        = Y008Zc   ; 
    assign A967Yc        = Y019Zc       ; 
endmodule
module hash_hp_A962Yc             #(
    parameter A963Yc   = 10 ,       
    parameter A964Yc   = 32
    )(
    input  wire                  i_rd_clk            ,
    input  wire                  i_rd_rst_n          ,
    input  wire                  A966Yc                ,
    output wire [A964Yc  -1:0]   A967Yc              ,
    output wire [A963Yc    :0]   A968Yc              ,
    output wire                  A969Yc              ,
    input  wire                  A970Yc              ,
    input  wire                  i_wr_clk            ,
    input  wire                  i_wr_rst_n          ,
    input  wire                  A971Yc                ,
    input  wire [A964Yc  -1:0]   A972Yc              ,
    output wire [A963Yc    :0]   A973Yc              ,
    output wire                  A974Yc              ,
    input  wire                  A975Yc              
    );
    integer                      A014Yc;
    wire                         Y017Zc;
    wire [A963Yc  -1:0]          Y018Zc     ;
    wire [A964Yc  -1:0]          Y019Zc     ;
    reg  [A963Yc    :0]          Y023Zc    ;
    wire [A963Yc    :0]          Y024Zc       ;
    reg  [A963Yc    :0]          Y025Zc         ;
    wire [A963Yc    :0]          Y026Zc            ;
    reg  [A963Yc    :0]          Y027Zc            ;
    reg  [A963Yc    :0]          Y028Zc            ;
    reg  [A963Yc    :0]          Y029Zc         ;
    wire [A963Yc    :0]          Y030Zc    ;
    wire                         Y031Zc       ;
    integer                      A015Yc;
    wire                         Y014Zc;
    wire [A963Yc  -1:0]          Y015Zc     ;
    wire [A964Yc  -1:0]          Y016Zc     ;
    reg  [A963Yc    :0]          Y032Zc    ;
    wire [A963Yc    :0]          Y033Zc       ;
    reg  [A963Yc    :0]          Y034Zc         ;
    wire [A963Yc    :0]          Y035Zc            ;
    reg  [A963Yc    :0]          Y036Zc            ;
    reg  [A963Yc    :0]          Y037Zc            ;
    reg  [A963Yc    :0]          Y038Zc         ;
    wire [A963Yc    :0]          Y039Zc    ;
    wire                         Y040Zc      ;
hash_hp_Y006Zc        #(
        .A963Yc        (A963Yc              ),       
        .A964Yc        (A964Yc              )
    ) Y041Zc          (
        .i_wr_clk      (i_wr_clk            ),
        .i_wr_rst_n    (i_wr_rst_n          ),    
        .A971Yc          (Y014Zc              ), 
        .Y007Zc        (Y015Zc              ), 
        .A972Yc        (Y016Zc              ), 
        .i_rd_clk      (i_rd_clk            ),
        .i_rd_rst_n    (i_rd_rst_n          ),   
        .A966Yc          (Y017Zc              ), 
        .Y008Zc        (Y018Zc              ), 
        .A967Yc        (Y019Zc              ) 
    );
    always@(posedge i_rd_clk or negedge i_rd_rst_n) 
    begin
        if(~i_rd_rst_n) begin
            Y023Zc              <= {(A963Yc  +1){1'b0}};
            Y025Zc              <= {(A963Yc  +1){1'b0}};
            Y027Zc              <= {(A963Yc  +1){1'b0}};
            Y028Zc              <= {(A963Yc  +1){1'b0}};
        end
        else begin
            Y023Zc              <=   Y024Zc       ;
            Y025Zc              <=   Y026Zc             ;
            Y027Zc              <=   Y034Zc          ;
            Y028Zc              <=   Y027Zc             ;
        end 
    end
    assign Y024Zc        = (A970Yc    ) ? {(A963Yc  +1){1'b0}} :
                           (Y017Zc)     ? Y023Zc    +1'b1      : Y023Zc     ;
    assign Y030Zc        = (Y023Zc    [A963Yc  -1:0]==
                            Y029Zc         [A963Yc  -1:0])  ? {(Y023Zc    [A963Yc  ] !=
                                                                Y029Zc         [A963Yc  ]),{(A963Yc  ){1'b0}}} :
                           (Y023Zc    [A963Yc  -1:0] <
                            Y029Zc         [A963Yc  -1:0])  ? {1'b0,(Y029Zc         [A963Yc  -1:0]-Y023Zc    [A963Yc  -1:0])} : 
                                                               Y029Zc         [A963Yc  -1:0]+({{1'b1},{(A963Yc  ){1'b0}}}-Y023Zc    [A963Yc  -1:0]) ;
    assign Y026Zc               = Y024Zc        ^ {1'b0,Y024Zc        [A963Yc  :1]};
    always@(*) begin
        for(A014Yc=0;A014Yc<A963Yc  +1;A014Yc=A014Yc+1) begin
            Y029Zc         [A014Yc]    = ^(Y028Zc            >>A014Yc);
        end
    end
    assign Y031Zc          = (Y029Zc         ==Y023Zc    );
    always@(posedge i_wr_clk or negedge i_wr_rst_n) 
    begin
        if(~i_wr_rst_n) begin
            Y032Zc              <= {(A963Yc  +1){1'b0}};
            Y034Zc              <= {(A963Yc  +1){1'b0}};
            Y036Zc              <= {(A963Yc  +1){1'b0}};
            Y037Zc              <= {(A963Yc  +1){1'b0}};
        end
        else begin
            Y032Zc              <=   Y033Zc       ;
            Y034Zc              <=   Y035Zc             ;
            Y036Zc              <=   Y025Zc          ;
            Y037Zc              <=   Y036Zc             ;
        end
    end
    assign Y033Zc        = (A975Yc    ) ? {(A963Yc  +1){1'b0}} :
                           (Y014Zc)     ? Y032Zc    +1'b1      : Y032Zc     ;
    assign Y039Zc        = (Y032Zc    [A963Yc  -1:0]==
                            Y038Zc         [A963Yc  -1:0])  ? {(Y032Zc    [A963Yc  ] ==
                                                                Y038Zc         [A963Yc  ]),{(A963Yc  ){1'b0}}} :
                           (Y032Zc    [A963Yc  -1:0] <
                            Y038Zc         [A963Yc  -1:0])  ? {1'b0,(Y038Zc         [A963Yc  -1:0]-Y032Zc    [A963Yc  -1:0])} : 
                                                               Y038Zc         [A963Yc  -1:0]+({{1'b1},{(A963Yc  ){1'b0}}}-Y032Zc    [A963Yc  -1:0]) ;
    assign Y035Zc               = Y033Zc        ^ {1'b0,Y033Zc        [A963Yc  :1]};
    always@(*) begin
        for(A015Yc=0;A015Yc<A963Yc  +1;A015Yc=A015Yc+1) begin
            Y038Zc         [A015Yc]    = ^(Y037Zc            >>A015Yc);
        end
    end
    assign Y040Zc          = (Y032Zc    [A963Yc  ]!=Y038Zc         [A963Yc  ])&(Y032Zc    [A963Yc  -1:0]==Y038Zc         [A963Yc  -1:0]);
    assign Y014Zc          = A971Yc&(~Y040Zc      )     ;
    assign Y016Zc          = A972Yc                   ;
    assign Y015Zc          = Y032Zc    [A963Yc  -1:0] ;
    assign Y017Zc          = A966Yc&(~Y031Zc       )    ;
    assign Y018Zc          = Y023Zc    [A963Yc  -1:0] ;
    assign A974Yc          = Y040Zc                   ;
    assign A973Yc          = Y039Zc                   ;
    assign A967Yc          = Y019Zc                   ;
    assign A969Yc          = Y031Zc                   ;
    assign A968Yc          = Y030Zc                   ;
endmodule
module hash_hp_A976Yc           (i_clk, i_rst_n, A971Yc, A972Yc   , A973Yc        , A974Yc   ,
	A966Yc, A967Yc   , A968Yc        , A969Yc    , A110Yc );
	parameter [31:0]	A963Yc  	= 32'd10;
	parameter [31:0]	A964Yc  	= 32'd32;
	input  wire                        i_clk;
	input  wire                        i_rst_n;
	input  wire                        A971Yc;
	input  wire [(A964Yc   - 1):0]     A972Yc   ;
	output wire [A963Yc  :0]           A973Yc        ;
	output wire                        A974Yc   ;
	input  wire                        A966Yc;
	output wire [(A964Yc   - 1):0]     A967Yc   ;
	output wire [A963Yc  :0]           A968Yc        ;
	output wire                        A969Yc    ;
	input  wire                        A110Yc ;
	wire                               Y014Zc;
	wire    [(A963Yc   - 1):0]         Y015Zc     ;
	wire    [(A964Yc   - 1):0]         Y016Zc     ;
	wire                               Y017Zc;
	wire    [(A963Yc   - 1):0]         Y018Zc     ;
	wire    [(A964Yc   - 1):0]         Y019Zc     ;
	reg     [A963Yc  :0]               Y032Zc    ;
	wire    [A963Yc  :0]               Y033Zc       ;
	reg     [A963Yc  :0]               Y023Zc    ;
	wire    [A963Yc  :0]               Y024Zc       ;
	reg     [A963Yc  :0]               Y030Zc    ;
	wire    [A963Yc  :0]               Y042Zc       ;
	reg     [A963Yc  :0]               Y039Zc    ;
	wire    [A963Yc  :0]               Y043Zc       ;
	wire                               Y040Zc      ;
	wire                               Y031Zc       ;
	assign Y024Zc        = (A110Yc  ? {(A963Yc   + 1) {1'b0}} : (Y017Zc ? (
		Y023Zc     + 1'b1) : Y023Zc    ));
	assign Y042Zc        = (A110Yc  ? {(A963Yc   + 1) {1'b0}} : ((Y017Zc & 
		Y014Zc) ? Y030Zc     : (Y014Zc ? (Y030Zc     + 1'b1) : (Y017Zc ?
		(Y030Zc     - 1'b1) : Y030Zc    ))));
	assign Y031Zc        = (Y032Zc     == Y023Zc    );
	assign Y033Zc        = (A110Yc  ? {(A963Yc   + 1) {1'b0}} : (Y014Zc ? (
		Y032Zc     + 1'b1) : Y032Zc    ));
	assign Y043Zc        = (A110Yc  ? {1'b1, {A963Yc   {1'b0}}} : ((Y017Zc &
		Y014Zc) ? Y039Zc     : (Y017Zc ? (Y039Zc     + 1'b1) : (Y014Zc ?
		(Y039Zc     - 1'b1) : Y039Zc    ))));
	assign Y040Zc       = ((Y032Zc    [A963Yc  ] != Y023Zc    [A963Yc  ]) & 
		(Y032Zc    [(A963Yc   - 1):0] == Y023Zc    [(A963Yc   - 1):0]));
	assign Y014Zc = (A971Yc & (~Y040Zc      ));
	assign Y016Zc      = A972Yc   ;
	assign Y015Zc      = Y032Zc    [(A963Yc   - 1):0];
	assign Y017Zc = (A966Yc & (~Y031Zc       ));
	assign Y018Zc      = Y023Zc    [(A963Yc   - 1):0];
	assign A974Yc    = Y040Zc      ;
	assign A973Yc         = Y039Zc    ;
	assign A967Yc    = Y019Zc     ;
	assign A969Yc     = Y031Zc       ;
	assign A968Yc         = Y030Zc    ;
hash_hp_Y006Zc         #(.A963Yc  (A963Yc  ), .A964Yc  (A964Yc  )) Y044Zc      (
		.i_wr_clk			(i_clk), 
		.i_wr_rst_n			(i_rst_n), 
		.A971Yc				(Y014Zc), 
		.Y007Zc   			(Y015Zc     ), 
		.A972Yc   			(Y016Zc     ), 
		.i_rd_clk			(i_clk), 
		.i_rd_rst_n			(i_rst_n), 
		.A966Yc				(Y017Zc), 
		.Y008Zc   			(Y018Zc     ), 
		.A967Yc   			(Y019Zc     ));
	always @(posedge i_clk or negedge i_rst_n) begin
	  if (~i_rst_n) begin
	    Y032Zc     <= {(A963Yc   + 1) {1'b0}};
	    Y023Zc     <= {(A963Yc   + 1) {1'b0}};
	    Y030Zc     <= {(A963Yc   + 1) {1'b0}};
	    Y039Zc     <= {1'b1, {A963Yc   {1'b0}}};
	  end
	  else
	    begin
	      Y032Zc     <=  Y033Zc       ;
	      Y023Zc     <=  Y024Zc       ;
	      Y030Zc     <=  Y042Zc       ;
	      Y039Zc     <=  Y043Zc       ;
	    end
	end
endmodule
module hash_hp_Y045Zc           
(
    input   wire    i_a_clk  
   ,input   wire    i_a_rst_n
   ,input   wire    A042Yc   
   ,input   wire    i_b_clk  
   ,input   wire    i_b_rst_n
   ,output  wire    Y046Zc           
);
    reg     Y047Zc      ;
    reg     Y048Zc      ;
    reg     Y049Zc      ;
    wire    Y050Zc      ;
    wire    Y051Zc      ;
    wire    Y052Zc      ;
    assign Y050Zc     = A042Yc;
    assign Y051Zc     = Y047Zc ;
    assign Y052Zc     = Y048Zc ;
    always@(posedge i_b_clk or negedge i_b_rst_n)
    begin
        if(!i_b_rst_n) begin
            Y047Zc  <= 1'b0;
            Y048Zc  <= 1'b0;
            Y049Zc  <= 1'b0;
        end
        else begin
            Y047Zc  <=  Y050Zc    ;
            Y048Zc  <=  Y051Zc    ;
            Y049Zc  <=  Y052Zc    ;
        end
    end
    assign Y046Zc = Y048Zc ;
endmodule
module hash_hp_Y053Zc           
(
    input   wire    i_a_clk  
   ,input   wire    i_a_rst_n
   ,input   wire    A042Yc   
   ,input   wire    i_b_clk  
   ,input   wire    i_b_rst_n
   ,output  wire    Y046Zc           
);
    reg     Y047Zc      ;
    reg     Y048Zc      ;
    reg     Y049Zc      ;
    reg     Y054Zc      ;
    reg     Y055Zc      ;
    wire    Y050Zc      ;
    wire    Y051Zc      ;
    wire    Y052Zc      ;
    wire    Y056Zc      ;
    wire    Y057Zc      ;
    assign Y056Zc      = A042Yc ? ~Y054Zc   :
                                Y054Zc  ;
    assign Y050Zc     = Y054Zc  ;
    assign Y051Zc     = Y047Zc ;
    assign Y052Zc     = Y048Zc ;
    assign Y057Zc     = Y049Zc  ^ Y048Zc ;
    always@(posedge i_a_clk or negedge i_a_rst_n)
    begin
        if(!i_a_rst_n) begin
            Y054Zc   <= 1'b0;
        end
        else begin
            Y054Zc   <=  Y056Zc     ;
        end
    end
    always@(posedge i_b_clk or negedge i_b_rst_n)
    begin
        if(!i_b_rst_n) begin
            Y047Zc  <= 1'b0;
            Y048Zc  <= 1'b0;
            Y049Zc  <= 1'b0;
            Y055Zc  <= 1'b0;
        end
        else begin
            Y047Zc  <=  Y050Zc    ;
            Y048Zc  <=  Y051Zc    ;
            Y049Zc  <=  Y052Zc    ;
            Y055Zc  <=  Y057Zc    ;
        end
    end
    assign Y046Zc = Y055Zc ;
endmodule
module hash_hp_Y058Zc     
#(
parameter                               A220Yc       = 32
)
(
    input   wire                        clk_core        
   ,input   wire                        rst_n_core      
   ,input   wire                        clk_ahb         
   ,input   wire                        rst_n_ahb       
   ,input   wire                        A718Yc  
   ,output  wire                        Y059Zc  
   ,input   wire                        A068Yc   
   ,output  wire                        Y060Zc   
   ,input   wire                        A781Yc         
   ,output  wire                        Y061Zc         
   ,input   wire                        A782Yc        
   ,output  wire                        Y062Zc        
   ,input   wire                        A783Yc            
   ,output  wire                        Y063Zc            
   ,input   wire                        A731Yc        
   ,output  wire                        Y064Zc        
   ,input   wire                        A735Yc        
   ,output  wire                        Y065Zc        
   ,input   wire                        Y066Zc      
   ,output  wire                        A719Yc      
   ,input   wire                        Y067Zc       
   ,output  wire                        A720Yc       
   ,input   wire                        Y068Zc      
   ,output  wire                        A721Yc      
   ,input   wire                        Y069Zc             
   ,output  wire                        Y070Zc             
   ,input   wire                        Y071Zc    
   ,output  wire                        Y072Zc    
   ,input   wire                        Y073Zc   
   ,output  wire                        Y074Zc   
   ,input   wire                        Y075Zc       
   ,output  wire                        Y076Zc       
   ,input   wire                        A789Yc
   ,output  wire                        A706Yc
   ,input   wire    [A220Yc      -1:0]  A739Yc
   ,input   wire                        A740Yc     
   ,output  wire                        A744Yc      
   ,output  wire    [A220Yc      -1:0]  Y077Zc
   ,output  wire                        Y078Zc     
   ,input   wire                        Y079Zc     
   ,input   wire    [31:0]              Y080Zc
   ,input   wire                        Y081Zc    
   ,input   wire    [5:0]               Y082Zc    
   ,output  wire    [31:0]              Y083Zc
   ,output  wire                        Y084Zc    
   ,output  wire    [5:0]               Y085Zc    
);
    wire Y086Zc        ;
    wire Y087Zc       ;
    wire Y088Zc           ;
    wire Y089Zc           ;
    wire Y090Zc           ;
    wire Y091Zc        ;
    wire Y092Zc       ;
    wire Y093Zc               ;
    wire Y094Zc              ;
    wire Y095Zc                  ;
    wire Y096Zc              ;
    wire Y097Zc              ;
    wire Y098Zc           ;
    wire Y099Zc          ;
hash_hp_A982Yc              Y100Zc            (
        .clk        (clk_ahb                )
       ,.rst_n      (rst_n_ahb              )
       ,.A152Yc     (A781Yc                 )
       ,.A336Yc     (Y093Zc                 )
    );
hash_hp_A982Yc              Y101Zc           (
        .clk        (clk_ahb                )
       ,.rst_n      (rst_n_ahb              )
       ,.A152Yc     (A782Yc                 )
       ,.A336Yc     (Y094Zc                 )
    );
hash_hp_A982Yc              Y102Zc               (
        .clk        (clk_ahb                  )
       ,.rst_n      (rst_n_ahb                )
       ,.A152Yc     (A783Yc                   )
       ,.A336Yc     (Y095Zc                   )
    );
hash_hp_A982Yc              Y103Zc          (
        .clk        (clk_ahb                )
       ,.rst_n      (rst_n_ahb              )
       ,.A152Yc     (A731Yc                 )
       ,.A336Yc     (Y096Zc                 )
    );
hash_hp_A982Yc              Y104Zc          (
        .clk        (clk_ahb                )
       ,.rst_n      (rst_n_ahb              )
       ,.A152Yc     (A735Yc                 )
       ,.A336Yc     (Y097Zc                 )
    );
hash_hp_A982Yc              Y105Zc            (
        .clk        (clk_ahb                )
       ,.rst_n      (rst_n_ahb              )
       ,.A152Yc     (A740Yc                 )
       ,.A336Yc     (Y098Zc                 )
    );
hash_hp_A982Yc              Y106Zc           (
        .clk        (clk_ahb                )
       ,.rst_n      (rst_n_ahb              )
       ,.A152Yc     (Y081Zc                 )
       ,.A336Yc     (Y099Zc                 )
    );
hash_hp_A995Yc           Y107Zc          (
        .clk        (clk_core       )
       ,.rst_n      (rst_n_core     )
       ,.A152Yc     (A718Yc         )
       ,.A336Yc     (Y059Zc         )
    );
hash_hp_A995Yc           Y108Zc           (
        .clk        (clk_core       )
       ,.rst_n      (rst_n_core     )
       ,.A152Yc     (A068Yc         )
       ,.A336Yc     (Y060Zc         )
    );
hash_hp_A983Yc                  Y109Zc                       
    (
        .clk_a      (clk_ahb        )
       ,.A984Yc      (rst_n_ahb      )
       ,.A985Yc     (Y093Zc               )
       ,.A986Yc     (Y086Zc         )
       ,.clk_b      (clk_core       )
       ,.A987Yc      (rst_n_core     )
       ,.A988Yc     (Y061Zc         )
       ,.A989Yc     (Y061Zc         )
    );
hash_hp_A983Yc                  Y110Zc                      
    (
        .clk_a      (clk_ahb        )
       ,.A984Yc      (rst_n_ahb      )
       ,.A985Yc     (Y094Zc              )
       ,.A986Yc     (Y087Zc         )
       ,.clk_b      (clk_core       )
       ,.A987Yc      (rst_n_core     )
       ,.A988Yc     (Y062Zc         )
       ,.A989Yc     (Y062Zc         )
    );
hash_hp_A983Yc                  Y111Zc                          
    (
        .clk_a      (clk_ahb        )
       ,.A984Yc      (rst_n_ahb      )
       ,.A985Yc     (Y095Zc                  )
       ,.A986Yc     (Y088Zc           )
       ,.clk_b      (clk_core       )
       ,.A987Yc      (rst_n_core     )
       ,.A988Yc     (Y063Zc             )
       ,.A989Yc     (Y063Zc             )
    );
hash_hp_A983Yc                  Y112Zc                      
    (
        .clk_a      (clk_ahb            )
       ,.A984Yc      (rst_n_ahb          )
       ,.A985Yc     (Y096Zc              )
       ,.A986Yc     (Y089Zc             )
       ,.clk_b      (clk_core           )
       ,.A987Yc      (rst_n_core         )
       ,.A988Yc     (Y064Zc             )
       ,.A989Yc     (Y064Zc             )
    );
hash_hp_A983Yc                  Y113Zc                      
    (
        .clk_a      (clk_ahb            )
       ,.A984Yc      (rst_n_ahb          )
       ,.A985Yc     (Y097Zc              )
       ,.A986Yc     (Y090Zc             )
       ,.clk_b      (clk_core           )
       ,.A987Yc      (rst_n_core         )
       ,.A988Yc     (Y065Zc             )
       ,.A989Yc     (Y065Zc             )
    );
hash_hp_A983Yc                  Y114Zc                    
    (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y066Zc             )
       ,.A986Yc     ()
       ,.clk_b      (clk_ahb            )
       ,.A987Yc      (rst_n_ahb          )
       ,.A988Yc     (A719Yc             )
       ,.A989Yc     (A719Yc             )
    );
hash_hp_A983Yc                  Y115Zc                     
    (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y067Zc             )
       ,.A986Yc     ()
       ,.clk_b      (clk_ahb            )
       ,.A987Yc      (rst_n_ahb          )
       ,.A988Yc     (A720Yc             )
       ,.A989Yc     (A720Yc             )
    );
hash_hp_A995Yc           Y116Zc                      
    (
        .clk        (clk_ahb            )
       ,.rst_n      (rst_n_ahb          )
       ,.A152Yc     (Y069Zc             )
       ,.A336Yc     (Y070Zc             )
    );
hash_hp_A995Yc           Y117Zc                    
    (
        .clk        (clk_ahb            )
       ,.rst_n      (rst_n_ahb          )
       ,.A152Yc     (Y068Zc             )
       ,.A336Yc     (A721Yc             )
    );
hash_hp_A995Yc           Y118Zc                  
    (
        .clk        (clk_ahb            )
       ,.rst_n      (rst_n_ahb          )
       ,.A152Yc     (Y071Zc             )
       ,.A336Yc     (Y072Zc             )
    );
hash_hp_A995Yc           Y119Zc                 
    (
        .clk        (clk_ahb            )
       ,.rst_n      (rst_n_ahb          )
       ,.A152Yc     (Y073Zc             )
       ,.A336Yc     (Y074Zc             )
    );
hash_hp_A995Yc           Y120Zc                     
    (
        .clk        (clk_ahb            )
       ,.rst_n      (rst_n_ahb          )
       ,.A152Yc     (Y075Zc             )
       ,.A336Yc     (Y076Zc             )
    );
hash_hp_A983Yc                  Y121Zc                   
    (
        .clk_a      (clk_ahb        )
       ,.A984Yc      (rst_n_ahb      )
       ,.A985Yc     (Y098Zc           )
       ,.A986Yc     (Y091Zc         )
       ,.clk_b      (clk_core       )
       ,.A987Yc      (rst_n_core     )
       ,.A988Yc     (Y078Zc         )
       ,.A989Yc     (Y079Zc      &Y078Zc     )
    );
hash_hp_A983Yc                  Y122Zc                  
    (
        .clk_a      (clk_ahb        )
       ,.A984Yc      (rst_n_ahb      )
       ,.A985Yc     (Y099Zc          )
       ,.A986Yc     (Y092Zc         )
       ,.clk_b      (clk_core       )
       ,.A987Yc      (rst_n_core     )
       ,.A988Yc     (Y084Zc         )
       ,.A989Yc     (Y084Zc         )
    );
hash_hp_A995Yc           Y123Zc       (
        .clk        (clk_ahb        )
       ,.rst_n      (rst_n_ahb      )
       ,.A152Yc     (A789Yc          )
       ,.A336Yc     (A706Yc          )
    );
    assign Y077Zc       = A739Yc;
    assign Y083Zc        = Y080Zc;
    assign Y085Zc       = Y082Zc    ;
    assign A744Yc       = Y086Zc         | Y087Zc        | Y088Zc            |
                          Y089Zc            | Y090Zc            | 
                          Y091Zc         | Y092Zc       ;
endmodule
module hash_hp_Y124Zc              
#(
    parameter   Y125Zc                  =  12
   ,parameter   Y126Zc                  =  10
   ,parameter   A220Yc                  =  32
   ,parameter   [1:0] Y127Zc            =  2'h0
   ,parameter   [1:0] Y128Zc            =  2'h3
)
(
    input   wire    [Y125Zc      -1:0]  Y129Zc         
   ,input   wire    [A220Yc      -1:0]  Y130Zc         
   ,input   wire                        Y131Zc         
   ,input   wire                        Y132Zc         
   ,output  wire    [A220Yc      -1:0]  Y133Zc         
   ,output  wire    [Y125Zc      -1:0]  Y134Zc             
   ,output  wire    [A220Yc      -1:0]  Y135Zc             
   ,output  wire                        Y136Zc             
   ,output  wire                        Y137Zc             
   ,input   wire    [A220Yc      -1:0]  Y138Zc             
   ,output  wire    [Y125Zc      -1:0]  Y139Zc             
   ,output  wire    [A220Yc      -1:0]  Y140Zc             
   ,output  wire                        Y141Zc             
   ,output  wire                        Y142Zc             
   ,input   wire    [A220Yc      -1:0]  Y143Zc             
);
    wire    [1:0]   Y144Zc    ;
    assign Y144Zc    [0] = (Y129Zc[Y125Zc      -1:Y126Zc          ] >= Y127Zc         ) && 
                           (Y129Zc[Y125Zc      -1:Y126Zc          ] <  Y128Zc         );
    assign Y144Zc    [1] = Y129Zc[Y125Zc      -1:Y126Zc          ] == Y128Zc         ;
    assign Y133Zc        = Y144Zc    [0] ? Y138Zc      :
                           Y144Zc    [1] ? Y143Zc      :
                                           {A220Yc      {1'b0}};
    assign Y134Zc        = {{Y125Zc      -Y126Zc          {1'b0}},Y129Zc[Y126Zc          -1:0]};
    assign Y135Zc        = Y130Zc ;
    assign Y136Zc        = Y131Zc    & Y144Zc    [0];
    assign Y137Zc        = Y132Zc     & Y144Zc    [0];
    assign Y139Zc        = {{Y125Zc      -Y126Zc          {1'b0}},Y129Zc[Y126Zc          -1:0]};
    assign Y140Zc        = Y130Zc ;
    assign Y141Zc        = Y131Zc    & Y144Zc    [1];
    assign Y142Zc        = Y132Zc     & Y144Zc    [1];
endmodule
module hash_hp_Y145Zc        
#(
   parameter   Y146Zc                   =  "Y147Zc"
  ,parameter   Y148Zc                   =  32 
)(
    input   wire                        A486Yc     
   ,input   wire    [Y148Zc   -1:0]     A152Yc 
   ,output  wire    [Y148Zc   -1:0]     Y149Zc    
);
    localparam Y150Zc       = Y148Zc   /8;
    localparam Y151Zc       = Y148Zc   /32;
    genvar A014Yc;
    generate
        if (Y146Zc  == "Y147Zc") begin : Y152Zc
            for (A014Yc=0;A014Yc<Y150Zc    ;A014Yc=A014Yc+1) begin : Y153Zc   
                assign Y149Zc    [A014Yc*8+:8] = !A486Yc   ? 
                        A152Yc[A014Yc*8+:8] : A152Yc[Y148Zc   -1-A014Yc*8-:8];
            end
        end
        else if (Y146Zc  == "Y154Zc") begin : Y155Zc
            for (A014Yc=0;A014Yc<Y151Zc    ;A014Yc=A014Yc+1) begin : Y156Zc   
                assign Y149Zc    [A014Yc*32+:32] = !A486Yc   ? 
                        A152Yc[A014Yc*32+:32] : A152Yc[Y148Zc   -1-A014Yc*32-:32];
            end
        end
        else begin : Y157Zc   
//                // synopsys translate_off 
//                // synopsys translate_on
        end
    endgenerate
endmodule
module hash_hp_Y158Zc             
#(
    parameter   Y159Zc                          =   256
   ,parameter   Y160Zc                          =   2 
)
(
    input   wire                                clk       
   ,input   wire                                rst_n   
   ,input   wire                                Y161Zc 
   ,input   wire    [Y159Zc     -1:0]           A152Yc 
   ,output  wire    [511:0]                     Y162Zc 
);
    localparam  Y163Zc      =  Y160Zc  ; 
    integer A014Yc;
    genvar A015Yc;
    reg     [Y159Zc     -1:0]   Y164Zc              [Y163Zc    -1:0]    ;
    wire    [Y159Zc     -1:0]   Y165Zc              [Y163Zc    -1:0]    ;
    reg     [Y159Zc     -1:0]   Y166Zc              [Y163Zc    -1:0]    ;
    wire    [Y159Zc     -1:0]   Y167Zc              [Y163Zc    -1:0]    ;
    assign Y165Zc   [0] = Y161Zc                 ? A152Yc : 
                          Y164Zc [0];
    generate
        for(A015Yc=1;A015Yc<Y163Zc    ;A015Yc=A015Yc+1) begin : Y168Zc
            assign  Y165Zc   [A015Yc] = Y161Zc  ? Y164Zc [A015Yc-1]:
                                   Y164Zc [A015Yc];
        end
    endgenerate
    always  @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            for(A014Yc=0;A014Yc<Y163Zc    ;A014Yc=A014Yc+1) begin
                Y164Zc [A014Yc] <= 256'd0;
            end
        end
        else begin
            for(A014Yc=0;A014Yc<Y163Zc    ;A014Yc=A014Yc+1) begin
                Y164Zc [A014Yc] <= Y165Zc   [A014Yc];
            end
        end
    end
    assign Y162Zc = {Y164Zc [1],Y164Zc [0]};
endmodule
module hash_hp_Y169Zc          #(
    parameter Y125Zc       = 64
)(
    input  wire           clk_axi
   ,input  wire           rst_n_axi
   ,input  wire           clk_core
   ,input  wire           rst_n_core
   ,input  wire [Y125Zc      -1:0]    Y170Zc         
   ,input  wire [Y125Zc      -1:0]    Y171Zc         
   ,input  wire [31:0]    Y172Zc        
   ,input  wire [31:0]    Y173Zc        
   ,input  wire           Y174Zc        
   ,input  wire           Y175Zc           
   ,input  wire           Y176Zc           
   ,input  wire           Y177Zc           
   ,input  wire           Y178Zc             
   ,output wire           Y179Zc                 
   ,output wire           Y180Zc          
   ,output wire           Y181Zc          
   ,input  wire           Y182Zc         
   ,input  wire           Y183Zc         
   ,input  wire           Y184Zc                
   ,output wire  [Y125Zc      -1:0]   Y185Zc         
   ,output wire  [Y125Zc      -1:0]   Y186Zc         
   ,output wire  [31:0]   Y187Zc        
   ,output wire  [31:0]   Y188Zc        
   ,output wire           Y189Zc          
   ,output wire           Y190Zc          
   ,output wire           Y191Zc            
   ,output wire           Y192Zc       
   ,output wire           Y193Zc          
);
   wire             Y194Zc       ;
   wire             Y195Zc          ;
   wire             Y196Zc          ;
   wire             Y197Zc          ;
   wire             Y198Zc            ;
hash_hp_A983Yc                  Y199Zc                    
   (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y175Zc             )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_axi            )
       ,.A987Yc      (rst_n_axi          )
       ,.A988Yc     (Y195Zc            )
       ,.A989Yc     (Y195Zc            )
    ); 
hash_hp_A983Yc                  Y200Zc                 
   (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y174Zc             )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_axi            )
       ,.A987Yc      (rst_n_axi          )
       ,.A988Yc     (Y194Zc             )
       ,.A989Yc     (Y194Zc             )
    ); 
hash_hp_A983Yc                  Y201Zc                    
   (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y176Zc              )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_axi            )
       ,.A987Yc      (rst_n_axi          )
       ,.A988Yc     (Y196Zc             )
       ,.A989Yc     (Y196Zc             )
    ); 
hash_hp_A983Yc                  Y202Zc                    
   (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y177Zc              )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_axi            )
       ,.A987Yc      (rst_n_axi          )
       ,.A988Yc     (Y197Zc             )
       ,.A989Yc     (Y197Zc             )
    );
hash_hp_A983Yc                  Y110Zc                      
   (
        .clk_a      (clk_core           )
       ,.A984Yc      (rst_n_core         )
       ,.A985Yc     (Y178Zc             )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_axi            )
       ,.A987Yc      (rst_n_axi          )
       ,.A988Yc     (Y198Zc             )
       ,.A989Yc     (Y198Zc             )
    );
    reg [Y125Zc      -1:0] Y203Zc         ;
    reg [Y125Zc      -1:0] Y204Zc         ;
    reg [31:0] Y205Zc        ;
    reg [31:0] Y206Zc        ;
    reg        Y207Zc          ;
    reg        Y208Zc          ;
    reg        Y209Zc            ;
    reg        Y210Zc       ;
    reg        Y211Zc          ;
    always @(posedge clk_axi or negedge rst_n_axi)
    if (!rst_n_axi) begin
        Y203Zc          <= {Y125Zc      {1'b0}};
        Y204Zc          <= {Y125Zc      {1'b0}};
        Y205Zc          <= 32'b0;
        Y206Zc          <= 32'b0;
        Y207Zc          <= 1'b0;
        Y208Zc          <= 1'b0;
        Y209Zc            <= 1'b0;
        Y210Zc          <= 1'b0;
        Y211Zc             <= 1'b0;
    end
    else begin
        Y203Zc          <=  Y196Zc           ? Y170Zc          : Y203Zc         ;
        Y204Zc          <=  Y197Zc           ? Y171Zc          : Y204Zc         ;
        Y205Zc          <=  Y196Zc           ? Y172Zc          : Y205Zc        ;
        Y206Zc          <=  Y197Zc           ? Y173Zc          : Y206Zc        ; 
        Y207Zc          <=  Y197Zc          ;
        Y208Zc          <=  Y196Zc          ;
        Y209Zc            <=  Y198Zc            ;      
        Y210Zc          <=  Y194Zc       ;
        Y211Zc            <=  Y195Zc          ;
    end
    assign Y185Zc          = Y203Zc         ;
    assign Y186Zc          = Y204Zc         ;
    assign Y187Zc          = Y205Zc        ;
    assign Y188Zc          = Y206Zc        ;
    assign Y189Zc          = Y207Zc          ;
    assign Y190Zc          = Y208Zc          ;
    assign Y191Zc             = Y209Zc            ;
    assign Y192Zc          = Y210Zc       ;
    assign Y193Zc             = Y211Zc          ;
    wire   Y212Zc                 ;
    wire   Y213Zc          ;
    wire   Y214Zc          ;
hash_hp_A983Yc                  Y215Zc                    
   (
        .clk_a      (clk_axi            )
       ,.A984Yc      (rst_n_axi          )
       ,.A985Yc     (Y182Zc             )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_core           )
       ,.A987Yc      (rst_n_core         )
       ,.A988Yc     (Y213Zc              )
       ,.A989Yc     (Y213Zc              )
    );
hash_hp_A983Yc                  Y216Zc                    
   (
        .clk_a      (clk_axi            )
       ,.A984Yc      (rst_n_axi          )
       ,.A985Yc     (Y183Zc             )
       ,.A986Yc     (                   )
       ,.clk_b      (clk_core           )
       ,.A987Yc      (rst_n_core         )
       ,.A988Yc     (Y214Zc              )
       ,.A989Yc     (Y214Zc              )
    );
hash_hp_A983Yc                  Y217Zc                           
   (
        .clk_a      (clk_axi                    )
       ,.A984Yc      (rst_n_axi                  )
       ,.A985Yc     (Y184Zc                     )
       ,.A986Yc     (                           )
       ,.clk_b      (clk_core                   )
       ,.A987Yc      (rst_n_core                 )
       ,.A988Yc     (Y212Zc                     )
       ,.A989Yc     (Y212Zc                     )
    );
    reg Y218Zc          ;
    reg Y219Zc          ;
    always @(posedge clk_core or negedge rst_n_core)
    if (!rst_n_core) begin
        Y218Zc           <= 1'b0;
        Y219Zc           <= 1'b0;
    end
    else begin
        Y218Zc           <=  Y177Zc            ? 1'b1 :
                            Y213Zc           ? 1'b0 : 
                            Y218Zc          ;
        Y219Zc           <=  Y176Zc            ? 1'b1 :
                            Y214Zc           ? 1'b0 : 
                            Y219Zc          ;
    end
    assign Y179Zc                  = Y212Zc                 ;
    assign Y180Zc           = Y218Zc          ;
    assign Y181Zc           = Y219Zc          ;
endmodule
