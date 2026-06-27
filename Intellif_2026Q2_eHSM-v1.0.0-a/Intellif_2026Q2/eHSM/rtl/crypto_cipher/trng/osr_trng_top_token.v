//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                         Token      
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_trng_top # (
    parameter   p_CONFIG_DRBG_AES_EN    =   1                   ,
    parameter   p_CONFIG_DRBG_SM4_EN    =   1                   ,
    parameter   p_CONFIG_DRBG_LFSR_EN   =   0                   ,
    parameter   p_CONFIG_TRBG_HT_EN     =   1                   ,
    parameter   p_CONFIG_DRBG_HT_EN     =   1                   ,
    parameter   pAHB_ADDR_WIDTH         =   12                  ,
    parameter   pAHB_DATA_WIDTH         =   32                  ,             
    parameter   p_CONFIG_FPGA           =   0
    )(
    input  wire                          clk            ,
`ifdef OSR_TRNG_FPGA            
    input  wire [1:0]                    i_osc_en_clk   ,
`endif
    input  wire                          rst_n          ,
    input  wire                          i_s_hsel       ,
    input  wire [1:0]                    i_s_htrans     ,
    input  wire [2:0]                    i_s_hburst     ,
    input  wire [2:0]                    i_s_hsize      ,
    input  wire [pAHB_ADDR_WIDTH-1:0]    i_s_haddr      ,
    input  wire                          i_s_hmastlock  ,
    input  wire [3:0]                    i_s_hprot      ,
    input  wire [pAHB_DATA_WIDTH-1:0]    i_s_hwdata     ,
    input  wire                          i_s_hwrite     ,
    input  wire                          i_s_hready     ,
    output wire [pAHB_DATA_WIDTH-1:0]    o_s_hrdata     ,
    output wire                          o_s_hresp      ,
    output wire                          o_s_hreadyout  ,
    output wire                          o_irq          ,
    input  wire                          i_trng_pop     ,
    output wire                          o_trng_drdy    ,
    output wire [pAHB_DATA_WIDTH-1:0]    o_trng_data    ,
    input  wire                          i_scan_mode    ,
    input  wire                          i_skip_startup ,
    input  wire                          i_drbg_mode    ,
    input  wire                          i_cbc_sm4      ,
    input  wire                          i_sclk_sel     ,
    input  wire [63:0]                   i_ro_src_en    ,
    input  wire [3:0]                    i_ro_clk_en    ,
    input  wire [1:0]                    i_ro_src_fsel  ,
    output wire                          o_trng_rdy     ,
    output wire                          o_alarm        ,
    output wire [3:0]                    o_ro_clk       ,
    output wire [3:0]                    o_ro_out        
    );
    localparam  Y479Zc                  =   32'h000D0202        ;
    localparam  A001Yc                  =   1                   ; 
    localparam  A180Yc                  =   pAHB_DATA_WIDTH     ;
    localparam  Y700Zc                  =   128                 ;
    localparam  Y701Zc                  =   128                 ;
    localparam  Y568Zc                  =   3                   ;
    localparam  Y569Zc                  =   3                   ;
    wire                                    Y663Zc              ;
    wire                                    Y702Zc              ;
    wire                                    Y703Zc              ;
    wire                                    Y704Zc              ;
    wire                                    Y705Zc              ;
    wire                                    Y706Zc              ;
    wire                                    Y707Zc              ;
    wire        [ 3: 0]                     Y708Zc              ;
    wire                                    Y709Zc              ;
    wire        [pAHB_DATA_WIDTH-1:0]       Y710Zc              ;
    wire                                    Y711Zc              ;
    wire                                    Y712Zc              ;
    wire                                    Y713Zc              ;
    wire                                    Y714Zc              ;
    wire                                    Y715Zc              ;
    wire                                    Y716Zc              ;
    wire                                    Y717Zc              ;
    wire        [63: 0]                     Y718Zc              ;
    wire        [1:0]                       Y719Zc              ;
    wire                                    Y720Zc              ;
    wire                                    Y721Zc              ;
    wire    [9:0]                           Y722Zc              ;
    wire    [9:0]                           Y723Zc              ;
    wire    [15:0]                          Y724Zc              ;
    wire    [31:0]                          Y725Zc              ;
    wire    [11:0]                          Y726Zc              ;
    wire    [23:0]                          Y727Zc              ;
    wire    [9:0]                           Y728Zc              ;
    wire    [9:0]                           Y729Zc              ;
    wire    [Y569Zc          :0]            Y730Zc              ;
    wire    [3:0]                           Y731Zc              ;
    wire    [3:0]                           Y732Zc              ;
    wire                                    Y733Zc              ;
    wire        [A180Yc     -1:0]           Y734Zc              ;
    wire                                    Y735Zc              ;
    wire                                    Y736Zc              ;
    wire                                    Y737Zc              ;
    wire                                    Y738Zc              ;
    wire                                    Y739Zc              ;
    wire        [Y568Zc          -1:0]      Y740Zc              ;
    wire        [A180Yc     -1:0]           Y741Zc              ;
    wire                                    Y742Zc              ;
    wire                                    Y743Zc              ;
    wire                                    Y744Zc              ;
    wire                                    Y745Zc              ;
    wire        [Y568Zc          :0]        Y746Zc              ;
    wire                                    Y747Zc              ;
    wire                                    Y748Zc              ;
    wire                                    Y749Zc              ;
    wire                                    Y750Zc              ;
    wire                                    Y751Zc              ;
    wire                                    Y752Zc              ;
    wire                                    Y753Zc              ;
    wire                                    Y754Zc              ;
    wire                                    Y755Zc              ;
    wire                                    Y756Zc              ;
    wire                                    Y757Zc              ;
    wire                                    Y758Zc              ;
    wire                                    Y759Zc              ;
    wire                                    Y760Zc              ;
    wire                                    Y761Zc              ;
    wire                                    Y762Zc              ;
    wire                                    Y763Zc              ;
    wire                                    Y764Zc              ;
    wire                                    Y765Zc              ;
    wire                                    Y766Zc              ;
    wire                                    Y767Zc              ;
    wire                                    Y768Zc              ;
    wire                                    Y769Zc              ;
    wire                                    Y770Zc              ;
    wire                                    Y771Zc              ;
    wire                                    Y772Zc              ;    
    wire                                    Y773Zc              ;    
    wire                                    Y774Zc              ;
    wire        [Y569Zc          -1:0]      Y775Zc              ;
    wire                                    Y776Zc              ;
    wire                                    Y777Zc              ;
    wire                                    Y778Zc              ;
    wire        [Y569Zc          :0]        Y779Zc              ;
    wire        [9:0]                       Y780Zc              ;
    wire        [A180Yc     -1:0]           Y781Zc              ;
    wire                                    Y782Zc              ;
    wire        [A180Yc     -1:0]           Y783Zc              ;
    wire                                    Y784Zc              ;
    wire                                    Y785Zc              ;
    wire                                    Y786Zc              ;
    wire                                    Y787Zc              ;
    wire                                    Y788Zc              ;
    wire                                    Y789Zc              ;
    wire                                    Y790Zc              ;
    wire                                    Y791Zc              ;
    wire                                    Y792Zc              ;
    wire                                    Y793Zc              ;
    wire                                    Y794Zc              ;
    wire                                    Y795Zc              ;
    wire                                    Y796Zc              ;
    wire                                    Y797Zc              ;
    wire        [A180Yc     -1:0]           Y798Zc              ;
    wire        [A180Yc     -1:0]           Y799Zc              ;
    wire                                    Y800Zc              ;
    wire                                    Y801Zc              ;
    wire                                    Y802Zc              ;
    wire                                    Y803Zc              ;
    wire                                    Y804Zc              ;
    wire        [A180Yc     -1:0]           Y805Zc              ;
    wire                                    Y806Zc              ;
    wire                                    Y807Zc              ;
    wire                                    Y808Zc              ;
    wire                                    Y809Zc              ;
    wire                                    Y810Zc              ;
    wire                                    Y811Zc              ;
    wire                                    Y812Zc              ;
    wire                                    Y813Zc              ;
        wire                                    Y814Zc          = 1'b0                                          ;
        wire                                    Y815Zc          = 1'b0                                          ;
        wire                                    Y816Zc          = 1'b0                                                              ;
        wire                                    Y817Zc          =  1'b0                                         ;
        wire                                    Y818Zc          =  1'b0                                         ;
        wire                                    Y819Zc          = 1'b0                                          ;
osr_trng_Y567Zc   # (
        .A180Yc                 (A180Yc                 ),
        .Y568Zc                 (Y568Zc                 ),
        .Y569Zc                 (Y569Zc                 )
    )
    Y820Zc    
    (
        .clk                    (clk                    ),
        .resetn                 (rst_n                  ),
        .A487Yc                 (i_skip_startup         ),
        .Y570Zc                 (Y703Zc                 ),
        .Y571Zc                 (Y704Zc                 ),
        .Y572Zc                 (Y705Zc                 ),
        .Y573Zc                 (Y706Zc                 ),
        .Y574Zc                 (Y707Zc                 ),
        .Y575Zc                 (Y709Zc                 ),
        .Y576Zc                 (Y710Zc                 ),
        .Y577Zc                 (Y730Zc                 ),
        .Y578Zc                 (Y728Zc                 ),
        .Y579Zc                 (Y711Zc                 ),
        .Y580Zc                 (Y712Zc                 ),
        .Y581Zc                 (Y713Zc                 ),
        .Y582Zc                 (Y714Zc                 ),
        .Y583Zc                 (Y715Zc                 ),
        .Y584Zc                 (Y716Zc                 ),
        .Y585Zc                 (Y717Zc                 ),
        .Y586Zc                 (Y733Zc                 ),
        .Y587Zc                 (Y734Zc                 ),
        .Y588Zc                 (Y735Zc                 ),
        .Y589Zc                 (Y736Zc                 ),
        .Y590Zc                 (Y737Zc                 ),
        .Y591Zc                 (Y738Zc                 ),
        .Y592Zc                 (Y739Zc                 ),
        .Y593Zc                 (Y740Zc                 ),
        .Y594Zc                 (Y741Zc                 ),
        .Y595Zc                 (Y742Zc                 ),
        .Y596Zc                 (Y743Zc                 ),
        .Y597Zc                 (Y744Zc                 ),
        .Y598Zc                 (Y745Zc                 ),
        .Y599Zc                 (Y746Zc                 ),
        .Y600Zc                 (Y729Zc                 ),
        .Y601Zc                 (Y747Zc                 ),
        .Y602Zc                 (Y748Zc                 ),
        .Y603Zc                 (Y749Zc                 ),
        .Y604Zc                 (Y750Zc                 ),
        .Y605Zc                 (Y751Zc                 ),
        .Y606Zc                 (Y752Zc                 ),
        .Y607Zc                 (Y753Zc                 ),
        .Y608Zc                 (Y754Zc                 ),
        .Y609Zc                 (Y755Zc                 ),
        .Y610Zc                 (Y756Zc                 ),
        .Y611Zc                 (Y758Zc                 ),
        .Y612Zc                 (Y759Zc                 ),
        .Y613Zc                 (Y760Zc                 ),
        .Y614Zc                 (Y761Zc                 ),
        .Y615Zc                 (Y762Zc                 ),
        .Y616Zc                 (Y764Zc                 ),
        .Y617Zc                 (Y765Zc                 ),
        .Y618Zc                 (Y766Zc                 ),
        .Y619Zc                 (Y767Zc                 ),
        .Y620Zc                 (Y768Zc                 ),
        .Y621Zc                 (Y769Zc                 ),
        .Y622Zc                 (Y770Zc                 ),
        .Y623Zc                 (Y771Zc                 ),
        .Y624Zc                 (Y774Zc                 ),
        .Y625Zc                 (Y775Zc                 ),
        .Y626Zc                 (Y776Zc                 ),
        .Y627Zc                 (Y777Zc                 ),
        .Y628Zc                 (Y778Zc                 ),
        .Y629Zc                 (Y779Zc                 ),
        .Y630Zc                 (Y780Zc                 ),
        .Y631Zc                 (Y781Zc                 ),
        .Y632Zc                 (Y782Zc                 ),
        .Y633Zc                 (Y783Zc                 ),
        .Y634Zc                 (Y784Zc                 ),
        .Y635Zc                 (Y788Zc                 ),
        .Y636Zc                 (Y787Zc                 ),
        .Y637Zc                 (Y789Zc                 ),
        .Y638Zc                 (Y790Zc                 ),
        .Y639Zc                 (Y791Zc                 ),
        .Y640Zc                 (Y792Zc                 ),
        .Y641Zc                 (Y793Zc                 ),
        .Y642Zc                 (Y794Zc                 ),
        .Y643Zc                 (Y795Zc                 ),
        .Y644Zc                 (Y798Zc                 ),
        .Y645Zc                 (Y799Zc                 ),
        .Y646Zc                 (Y800Zc                 ),
        .Y647Zc                 (Y801Zc                 ),
        .Y648Zc                 (Y802Zc                 ),
        .Y649Zc                 (Y803Zc                 ),
        .Y650Zc                 (Y804Zc                 ),
        .Y651Zc                 (Y805Zc                 ),
        .Y652Zc                 (Y806Zc                 ),
        .Y653Zc                 (Y807Zc                 ),
        .Y654Zc                 (Y813Zc                 ),
        .Y655Zc                 (o_alarm                )
    );
    assign o_trng_rdy   = Y813Zc    ;
osr_trng_Y475Zc       # (
        .A411Yc                         (p_CONFIG_DRBG_AES_EN),
        .A412Yc                         (p_CONFIG_DRBG_SM4_EN),  
        .A573Yc                         (p_CONFIG_DRBG_LFSR_EN),              
        .Y434Zc                         (pAHB_ADDR_WIDTH    ),
        .Y435Zc                         (pAHB_DATA_WIDTH    ),
        .Y476Zc                         (Y568Zc             ),
        .Y477Zc                         (Y569Zc             ),
        .Y478Zc                         (8                  ),
        .Y479Zc                         (Y479Zc             )
    )
    Y821Zc        
    (
        .i_s_hclk                       (clk                ),
        .i_s_hresetn                    (rst_n              ),
        .Y436Zc                         (i_s_hsel           ),
        .Y437Zc                         (i_s_haddr          ),
        .Y438Zc                         (i_s_hwrite         ),
        .Y439Zc                         (i_s_hsize          ),
        .Y440Zc                         (i_s_hburst         ),
        .Y441Zc                         (i_s_hprot          ),
        .Y442Zc                         (i_s_htrans         ),
        .Y443Zc                         (i_s_hmastlock      ),
        .Y444Zc                         (i_s_hready         ),
        .Y445Zc                         (i_s_hwdata         ),
        .Y446Zc                         (o_s_hreadyout      ),
        .Y447Zc                         (o_s_hresp          ),
        .Y448Zc                         (o_s_hrdata         ),
        .Y483Zc                         (Y707Zc             ),
        .Y484Zc                         (Y706Zc             ),
        .Y485Zc                         (Y705Zc             ),
        .A067Yc                         (Y813Zc             ),
        .Y486Zc                         (Y772Zc             ), 
        .Y487Zc                         (Y710Zc             ),
        .Y488Zc                         (Y763Zc             ),
        .Y489Zc                         (Y711Zc             ),
        .Y490Zc                         (Y712Zc             ),
        .Y491Zc                         (Y713Zc             ),
        .Y492Zc                         (Y714Zc             ),
        .Y493Zc                         (Y715Zc             ),
        .Y494Zc                         (Y716Zc             ),
        .Y495Zc                         (Y717Zc             ),
        .Y496Zc                         (Y809Zc             ),     
        .Y497Zc                         (Y808Zc             ), 
        .Y498Zc                         (Y810Zc             ), 
        .Y499Zc                         (Y811Zc             ), 
        .Y500Zc                         (Y812Zc             ),
        .Y501Zc                         (Y752Zc             ), 
        .Y502Zc                         (Y751Zc             ), 
        .Y503Zc                         (Y750Zc             ), 
        .Y504Zc                         (Y749Zc             ), 
        .Y505Zc                         (Y748Zc             ), 
        .Y506Zc                         (Y785Zc             ),
        .Y507Zc                         (Y786Zc             ),
        .Y508Zc                         (Y787Zc             ),
        .Y509Zc                         (Y788Zc             ),
        .Y480Zc                         (Y703Zc             ),
        .Y482Zc                         (Y704Zc             ),
        .Y481Zc                         (Y708Zc             ),
        .Y510Zc                         (Y718Zc             ),
        .Y511Zc                         (Y719Zc             ),
        .Y512Zc                         (i_drbg_mode         ),
        .Y514Zc                         (Y720Zc              ),
        .A720Yc                         (i_cbc_sm4           ),
        .Y513Zc                         (Y721Zc              ),
        .Y515Zc                         (Y722Zc              ),
        .Y516Zc                         (Y723Zc              ),
        .Y517Zc                         (Y724Zc              ),
        .Y518Zc                         (Y725Zc              ),
        .Y519Zc                         (Y726Zc              ),
        .Y520Zc                         (Y727Zc              ),
        .Y521Zc                         (Y728Zc              ),
        .Y522Zc                         (Y730Zc              ),        
        .Y524Zc                         (Y744Zc             ),
        .Y525Zc                         (Y776Zc             ),
        .Y455Zc                         (Y709Zc             ),
        .A172Yc                         (i_ro_src_en        ),
        .A173Yc                         (i_ro_clk_en        ),
        .Y523Zc                         (i_ro_src_fsel      ),
        .Y526Zc                          (o_irq              )
    );
osr_trng_A000Yc   #(
        .A001Yc                 (A001Yc             ),
        .A002Yc                 (p_CONFIG_TRBG_HT_EN),
        .A007Yc                 (A180Yc             ),
        .A008Yc                 (Y568Zc             ),
        .A009Yc                 (32                 ),
        .A010Yc                 (1024               ),    
        .A011Yc                 (624                ),
        .A012Yc                 (512                ),    
        .A013Yc                 (77                 )
       ,.A004Yc                 (p_CONFIG_FPGA      )    
    )
    u_trbg_top
    (
        .i_clk                  (clk                ),
    `ifdef OSR_TRNG_FPGA         
        .i_osc_en_clk           (i_osc_en_clk       ),
    `endif                
        .i_rstn                 (rst_n              ),
        .i_scan_mode            (i_scan_mode        ),
        .A014Yc                 (i_sclk_sel         ),
        .A015Yc                 (Y758Zc             ),
        .A016Yc                 (Y759Zc             ),
        .A017Yc                 (Y760Zc             ),
        .A018Yc                 (Y761Zc             ),
        .A019Yc                 (Y762Zc             ),
        .A021Yc                 (Y718Zc             ),
        .A020Yc                 (Y719Zc             ),
        .A022Yc                 (Y708Zc             ),
        .A023Yc                 (o_ro_out           ),
        .o_ro_roclk             (o_ro_clk           ),
        .A024Yc                 (Y734Zc             ),    
        .A025Yc                 (Y735Zc             ),
        .A026Yc                 (Y736Zc             ),
        .A027Yc                 (Y737Zc             ),
        .A028Yc                 (Y738Zc             ),
        .A029Yc                 (Y722Zc             ),
        .A030Yc                 (Y724Zc             ),
        .A031Yc                 (Y725Zc             ),
        .A032Yc                 (Y723Zc             ),
        .A033Yc                 (Y726Zc             ),
        .A034Yc                 (Y727Zc             ),
        .A035Yc                 (Y739Zc             ),
        .A036Yc                 (Y740Zc             ),
        .A037Yc                 (Y741Zc             ),
        .A038Yc                 (Y742Zc             ),
        .A039Yc                 (Y743Zc             ),
        .A040Yc                 (Y744Zc             ),
        .A041Yc                 (Y745Zc             ),
        .A042Yc                 (Y746Zc             ),
        .A043Yc                 (Y729Zc              ),
        .A044Yc                 (Y747Zc              ),
        .A045Yc                 (Y748Zc             ),
        .A046Yc                 (Y752Zc             ),
        .A047Yc                 (Y751Zc             ),
        .A048Yc                 (Y750Zc             ),
        .A049Yc                 (Y749Zc             ),
        .A050Yc                 (Y808Zc             ),
        .A051Yc                 (Y810Zc             ),
        .A052Yc                 (Y811Zc             ),
        .A053Yc                 (Y809Zc             ),
        .A054Yc                 (Y812Zc             ),
        .A055Yc                 (Y753Zc             ),
        .A056Yc                 (Y754Zc             ),
        .A057Yc                 (Y733Zc             ),
        .A058Yc                 (Y807Zc             ), 
        .A059Yc                 (Y755Zc             ),
        .A060Yc                 (Y756Zc             ),
        .A061Yc                 (Y791Zc             ),
        .A062Yc                 (Y773Zc             ),
        .A063Yc                 (Y768Zc             ),
        .A064Yc                 (i_trng_pop         ),
        .A065Yc                 (o_trng_data        ),
        .A066Yc                 (o_trng_drdy        ),
        .A067Yc                 (Y813Zc             )
    );
    generate 
    if(p_CONFIG_DRBG_AES_EN | p_CONFIG_DRBG_SM4_EN) begin: Y822Zc     
osr_trng_A572Yc   # 
        (
                .A411Yc                 (p_CONFIG_DRBG_AES_EN   ),
                .A412Yc                 (p_CONFIG_DRBG_SM4_EN   ),
                .A573Yc                 (p_CONFIG_DRBG_LFSR_EN  ),
                .A574Yc                 (p_CONFIG_DRBG_HT_EN    ),
                .A180Yc                 (A180Yc                 ),
                .A179Yc                 (Y569Zc                 ),
                .A294Yc                 (Y700Zc                 ),
                .A295Yc                 (Y701Zc                 )
        )
        Y823Zc    
        (
                .clk                    (clk                    ),
                .resetn                 (rst_n                  ),
                .A299Yc                 (Y764Zc                 ),
                .A487Yc                 (Y765Zc                 ),
                .A015Yc                 (Y789Zc                 ),
                .A575Yc                 (Y790Zc                 ),
                .A491Yc                 (Y763Zc                 ),
                .A488Yc                 (Y766Zc                 ),
                .A490Yc                 (Y768Zc                 ),
                .A576Yc                 (Y720Zc                 ),
                .A307Yc                 (Y721Zc                 ),
                .A489Yc                 (Y767Zc                 ),
                .A503Yc                 (Y770Zc                 ),
                .A577Yc                 (Y771Zc                 ),
                .A578Yc                 (Y772Zc                 ),
                .A579Yc                 (Y773Zc                 ),
                .A492Yc                 (Y769Zc                 ),
                .A493Yc                 (Y774Zc                 ),
                .A494Yc                 (Y775Zc                 ),
                .A495Yc                 (Y776Zc                 ),
                .A496Yc                 (Y777Zc                 ),
                .A497Yc                 (Y778Zc                 ),
                .A498Yc                 (Y779Zc                 ),
                .A499Yc                 (Y781Zc                 ),
                .A500Yc                 (Y782Zc                 ),
                .A501Yc                 (Y783Zc                 ),
                .A502Yc                 (Y784Zc                 ),
                .A580Yc                 (Y729Zc                 ),
                .A581Yc                   (Y780Zc               ),
                .A582Yc                 (Y788Zc                 ),
                .A583Yc                 (Y787Zc                 ),
                .A584Yc                 (Y786Zc                 ),
                .A585Yc                 (Y785Zc                 )
    );
    end
    else if(p_CONFIG_DRBG_LFSR_EN) begin: Y824Zc  
osr_trng_A572Yc   # 
        (
                .A411Yc                 (p_CONFIG_DRBG_AES_EN   ),
                .A412Yc                 (p_CONFIG_DRBG_SM4_EN   ),
                .A573Yc                 (p_CONFIG_DRBG_LFSR_EN  ),
                .A574Yc                 (p_CONFIG_DRBG_HT_EN    ),
                .A180Yc                 (A180Yc                 ),
                .A179Yc                 (Y569Zc                 ),
                .A294Yc                 (Y700Zc                 ),
                .A295Yc                 (Y701Zc                 )
        )
        Y823Zc    
        (
                .clk                    (clk                    ),
                .resetn                 (rst_n                  ),
                .A299Yc                 (Y764Zc                 ),
                .A487Yc                 (Y765Zc                 ),
                .A015Yc                 (Y789Zc                 ),
                .A575Yc                 (Y790Zc                 ),
                .A491Yc                 (Y763Zc                 ),
                .A488Yc                 (Y766Zc                 ),
                .A490Yc                 (Y768Zc                 ),
                .A576Yc                 (Y720Zc                 ),
                .A307Yc                 (Y721Zc                 ),
                .A489Yc                 (Y767Zc                 ),
                .A503Yc                 (Y770Zc                 ),
                .A577Yc                 (Y771Zc                 ),
                .A578Yc                 (Y772Zc                 ),                
                .A579Yc                 (Y773Zc                 ),
                .A492Yc                 (Y769Zc                 ),
                .A493Yc                 (Y774Zc                 ),
                .A494Yc                 (Y775Zc                 ),
                .A495Yc                 (Y776Zc                 ),
                .A496Yc                 (Y777Zc                 ),
                .A497Yc                 (Y778Zc                 ),
                .A498Yc                 (Y779Zc                 ),
                .A499Yc                 (Y781Zc                 ),
                .A500Yc                 (Y782Zc                 ),
                .A501Yc                 (Y783Zc                 ),
                .A502Yc                 (Y784Zc                 ),
                .A580Yc                 (Y729Zc                 ),
                .A581Yc                   (Y780Zc               ),
                .A582Yc                 (Y788Zc                 ),
                .A583Yc                 (Y787Zc                 ),
                .A584Yc                 (Y786Zc                 ),
                .A585Yc                 (Y785Zc                 )
        );
    end
    endgenerate
osr_trng_Y372Zc   #
    (
        .A411Yc                     (p_CONFIG_DRBG_AES_EN),
        .A412Yc                     (p_CONFIG_DRBG_SM4_EN),
    .A007Yc           (A180Yc                )
    )
    Y825Zc    
    (
    .i_clk            (clk                   ),
    .i_rstn           (rst_n                 ),
    .Y373Zc           (Y720Zc                ),
    .A720Yc           (Y721Zc                ),
    .A721Yc           (Y792Zc                ),
    .Y374Zc           (Y793Zc                ),
    .A722Yc           (Y794Zc                ),
    .A723Yc           (Y795Zc                ),
    .Y375Zc           (Y798Zc                ),
    .Y376Zc           (Y799Zc                ),
    .Y377Zc           (Y800Zc                ),
    .Y378Zc           (Y801Zc                ),
    .Y379Zc            (Y802Zc               ),
    .Y380Zc           (Y803Zc                ),
    .Y381Zc           (Y804Zc                ),
    .Y382Zc           (Y805Zc                ),
    .Y383Zc           (Y806Zc                ) 
    );
endmodule
module osr_trng_ahb_reg_bank
#(
    parameter   p_CONFIG_DRBG_SM4_EN                =   1
   ,parameter   p_CONFIG_DRBG_AES_EN                =   1
   ,parameter   p_CONFIG_DRBG_LFSR_EN               =   0        
   ,parameter   p_AHB_ADDR_WIDTH                    =   20               
   ,parameter   p_AHB_DATA_WIDTH                    =   32
   ,parameter   p_TFIFO_TV_WIDTH                    =   7
   ,parameter   p_DFIFO_TV_WIDTH                    =   7
   ,parameter   p_TERO_SOURCE_CNT                   =   8    
   ,parameter   p_VERSION                           =   32'h000D0100
   ,parameter   P_SM_CNT_P_WIDTH                    =   32
   ,parameter   P_SM_CNT_P_OVERFLOW                 =   32'hFFFFFFFF
)(
    input   wire                                    clk                 
   ,input   wire                                    rst_n               
   ,input   wire    [p_AHB_ADDR_WIDTH-1:0]          i_addr              
   ,input   wire    [p_AHB_DATA_WIDTH-1:0]          i_wdata   
   ,input   wire                                    i_read_en           
   ,input   wire                                    i_write_en          
   ,input   wire    [3:0]                           i_wstrobe           
   ,input   wire                                    i_priority          
   ,output  reg     [p_AHB_DATA_WIDTH-1:0]          o_rdata
   ,output  wire                                    o_rngen
   ,output  wire    [3:0]                           o_rosen
   ,output  wire                                    o_msel
   ,input   wire                                    i_htf
   ,input   wire                                    i_drdy
   ,input   wire                                    i_ererr
   ,input   wire                                    i_trng_rdy
   ,input   wire                                    i_drbg_rdy_for_cfg
   ,input   wire    [p_AHB_DATA_WIDTH-1:0]          i_rng_dr
   ,output  wire                                    o_reseed
   ,output  wire                                    o_trcten
   ,output  wire                                    o_tapten
   ,output  wire                                    o_trunsen
   ,output  wire                                    o_taptnben
   ,output  wire                                    o_tpokeren
   ,output  wire                                    o_drcten
   ,output  wire                                    o_drpten
   ,input   wire                                    i_trrunsbf    
   ,input   wire                                    i_trpokerbf
   ,input   wire                                    i_taptnbbf
   ,input   wire                                    i_taptbbf 
   ,input   wire                                    i_trctbf  
   ,input   wire                                    i_trrunsf
   ,input   wire                                    i_trpokerf
   ,input   wire                                    i_tratnbf
   ,input   wire                                    i_taptf   
   ,input   wire                                    i_trctf
   ,input   wire                                    i_drptbf   
   ,input   wire                                    i_drctbf   
   ,input   wire                                    i_drptf 
   ,input   wire                                    i_drctf 
   ,output  wire    [63:0]                          o_roen
   ,output  wire    [1:0]                           o_fsel
   ,input   wire                                    i_drbg_mode         
   ,input   wire                                    i_cbc_sm4           
   ,output  wire                                    o_cbc_sm4
   ,output  wire                                    o_drbg_mode
   ,output  wire    [9:0]                           o_tapt_win
   ,output  wire    [9:0]                           o_tpoker_win
   ,output  wire    [15:0]                          o_tapt_thld
   ,output  wire    [31:0]                          o_truns_thld
   ,output  wire    [11:0]                          o_taptnb_thld
   ,output  wire    [23:0]                          o_tpoker_thld
   ,input   wire    [9:0]                           i_trht_iteration_ptr 
   ,input   wire    [3:0]                           i_rnvld_num 
   ,output  wire                                    o_irq
   ,input   wire    [63:0]                          i_roen
   ,input   wire    [3:0]                           i_rosen 
   ,input   wire    [1:0]                           i_fsel
   ,input   wire                                    i_trfifo_empty
   ,input   wire                                    i_drfifo_empty  
   ,input   wire                                    i_fifo_pop
   ,output  wire                                    o_fifo_pop
);
    genvar  i;
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRNG_CTRL            =   12'h000 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRNG_MSEL            =   12'h004 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRNG_STATUS          =   12'h008 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRNG_DATA            =   12'h00C + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_RESEED               =   12'h010 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_RO_CLK_EN            =   12'h014 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_RO_SRC_EN1           =   12'h018 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_RO_SRC_EN2           =   12'h01C + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_HT_CTRL              =   12'h020 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_HT_STATUS            =   12'h024 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRNG_VERSION         =   12'h030 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_DRBG_ALG_HW_SR       =   12'h034 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_DRBG_ALG_MODE_SEL    =   12'h038 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRHT_INERATION_PTR   =   12'h040 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRBG_HT_WIN_CFG      =   12'h080 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TAPT_THLD_CFG        =   12'h090 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TAPTNB_THLD_CFG      =   12'h094 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TPOKER_THLD_CFG      =   12'h098 + {p_AHB_ADDR_WIDTH{1'b0}};
    localparam  [p_AHB_ADDR_WIDTH-1:0]  p_ADDR_TRUNS_THLD_CFG       =   12'h09C + {p_AHB_ADDR_WIDTH{1'b0}};
localparam     SM_CNT_P_WIDTH = P_SM_CNT_P_WIDTH;
localparam     SM_CNT_P_OVERFLOW = P_SM_CNT_P_OVERFLOW;
localparam     SM_REG_PARITY_P_REG_GROUP_NUM     = 12;
localparam     SM_REG_PARITY_P_REG_DATA_WIDTH    = 32;
localparam     SM_REG_PARITY_LP_REG_PARITY_WIDTH = (SM_REG_PARITY_P_REG_DATA_WIDTH > (SM_REG_PARITY_P_REG_DATA_WIDTH/8)*8) ? ((SM_REG_PARITY_P_REG_DATA_WIDTH/8) + 1) : (SM_REG_PARITY_P_REG_DATA_WIDTH/8);
    reg                                     r_rngen             ;
    reg     [3:0]                           r_rosen             ;
    reg                                     r_dien              ;
    reg                                     r_erien             ;
    reg                                     r_irqen             ;
    wire                                    w_rngen_pre         ;
    wire    [3:0]                           w_rosen_pre         ;
    wire                                    w_dien_pre          ;
    wire                                    w_erien_pre         ;
    wire                                    w_irqen_pre         ;
    reg                                     r_msel              ;
    wire                                    w_msel_pre          ;
    reg                                     r_htf               ;
    reg                                     r_drdy              ;
    reg                                     r_ererr             ;
    wire                                    w_htf_pre           ;
    wire                                    w_drdy_pre          ;
    wire                                    w_ererr_pre         ;
    reg                                     r_reseed            ;
    wire                                    w_reseed_pre        ;              
    reg                                     r_trrunsen          ;
    reg                                     r_trpokeren         ;
    reg                                     r_tratnben          ;
    reg                                     r_trcten            ;
    reg                                     r_tapten            ;
    reg                                     r_drcten            ;
    reg                                     r_drpten            ;
    wire                                    w_trrunsen_pre      ;
    wire                                    w_trpokeren_pre     ;
    wire                                    w_tratnben_pre      ;   
    wire                                    w_trcten_pre        ;
    wire                                    w_tapten_pre        ;
    wire                                    w_drcten_pre        ;
    wire                                    w_drpten_pre        ;
    reg                                     r_trrunsf           ;
    reg                                     r_trpokerf          ;
    reg                                     r_tratnbf           ;
    reg                                     r_trctf             ;
    reg                                     r_taptf             ;
    reg                                     r_trrunsbf          ;
    reg                                     r_trpokerbf         ;    
    reg                                     r_taptnbbf          ;
    reg                                     r_taptbbf           ;
    reg                                     r_trctbf            ;
    reg                                     r_drctf             ;
    reg                                     r_drptf             ;
    reg                                     r_drptbf            ;
    reg                                     r_drctbf            ;
    reg     [63:0]                          r_roen              ;
    wire    [63:0]                          w_roen_pre          ;
    reg     [1:0]                           r_fsel              ;
    wire    [1:0]                           w_fsel_pre          ;
    reg                                     r_drbg_sm4_mode_sel     ;
    wire                                    w_drbg_sm4_mode_sel_pre ;
    reg                                     r_drbg_alg_sel          ;
    wire                                    w_drbg_alg_sel_pre      ;
    reg     [9:0]                           r_tpoker_win_size       ;
    reg     [9:0]                           r_tapt_win_size         ;
    wire    [9:0]                           w_tpoker_win_size_pre   ;
    wire    [9:0]                           w_tapt_win_size_pre     ;    
    reg     [15:0]                          r_tapt_thld         ;
    reg     [11:0]                          r_taptnb_thld       ;
    reg     [23:0]                          r_tpoker_thld       ;
    reg     [31:0]                          r_truns_thld        ;       
    wire    [15:0]                          w_tapt_thld_pre     ;
    wire    [11:0]                          w_taptnb_thld_pre   ;
    wire    [23:0]                          w_tpoker_thld_pre   ;
    wire    [31:0]                          w_truns_thld_pre    ;  
    wire                                    w_rand_vld          ;
    wire                                    w_drbg_rdy_for_cfg  ;
    reg                                     rst_sync            ;    
    wire                                    w_fun_lock_en = 1'b1;
wire                         sm_cnt_o_r_alarm     = 1'b0;
wire                         sm_cnt_o_alarm       = 1'b0;
wire                                                                          sm_reg_parity_o_set_alarm   = 1'b0; 
wire                                                                          sm_reg_parity_o_r_alarm     = 1'b0; 
wire                                                                          sm_reg_parity_o_alarm       = 1'b0; 
    wire    w_adr_eq_trng_ctrl          = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRNG_CTRL          ;
    wire    w_adr_eq_trng_msel          = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRNG_MSEL          ;
    wire    w_adr_eq_trng_status        = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRNG_STATUS        ;
    wire    w_adr_eq_trng_data          = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRNG_DATA          ;
    wire    w_adr_eq_reseed             = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_RESEED             ;
    wire    w_adr_eq_ro_clk_en          = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_RO_CLK_EN          ;
    wire    w_adr_eq_ro_src_en1         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_RO_SRC_EN1         ;
    wire    w_adr_eq_ro_src_en2         = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_RO_SRC_EN2         ;
    wire    w_adr_eq_ht_ctrl            = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_HT_CTRL            ;
    wire    w_adr_eq_ht_status          = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_HT_STATUS          ;
    wire    w_adr_eq_trng_version       = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRNG_VERSION       ;
    wire    w_adr_eq_drbg_alg_hw_sr     = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_DRBG_ALG_HW_SR     ;
    wire    w_adr_eq_drbg_alg_mode_sel  = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_DRBG_ALG_MODE_SEL  ;
    wire    w_adr_eq_trht_ineration_ptr = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRHT_INERATION_PTR ;
    wire    w_adr_eq_trbg_ht_win_cfg    = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRBG_HT_WIN_CFG    ;
    wire    w_adr_eq_tapt_thld_cfg      = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TAPT_THLD_CFG      ;
    wire    w_adr_eq_taptnb_thld_cfg    = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TAPTNB_THLD_CFG    ;
    wire    w_adr_eq_tpoker_thld_cfg    = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TPOKER_THLD_CFG    ;
    wire    w_adr_eq_truns_thld_cfg     = {i_addr[p_AHB_ADDR_WIDTH-1:2],2'b00} == p_ADDR_TRUNS_THLD_CFG     ;
    wire    w_sel_trng_ctrl             = w_adr_eq_trng_ctrl          & i_write_en;
    wire    w_sel_trng_msel             = w_adr_eq_trng_msel          & i_write_en;
    wire    w_sel_trng_status           = w_adr_eq_trng_status        & i_write_en;
    wire    w_sel_reseed                = w_adr_eq_reseed             & i_write_en;
    wire    w_sel_ro_clk_en             = w_adr_eq_ro_clk_en          & i_write_en;
    wire    w_sel_ro_src_en1            = w_adr_eq_ro_src_en1         & i_write_en;
    wire    w_sel_ro_src_en2            = w_adr_eq_ro_src_en2         & i_write_en;
    wire    w_sel_ht_ctrl               = w_adr_eq_ht_ctrl            & i_write_en;
    wire    w_sel_drbg_alg_mode_sel     = w_adr_eq_drbg_alg_mode_sel  & i_write_en;
    wire    w_sel_trbg_ht_win_cfg       = w_adr_eq_trbg_ht_win_cfg    & i_write_en;
    wire    w_sel_tapt_thld_cfg         = w_adr_eq_tapt_thld_cfg      & i_write_en;
    wire    w_sel_taptnb_thld_cfg       = w_adr_eq_taptnb_thld_cfg    & i_write_en;
    wire    w_sel_tpoker_thld_cfg       = w_adr_eq_tpoker_thld_cfg    & i_write_en;
    wire    w_sel_truns_thld_cfg        = w_adr_eq_truns_thld_cfg     & i_write_en;
    assign  w_rngen_pre = w_sel_trng_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[0]    : r_rngen ;
    assign  w_dien_pre  = w_sel_trng_ctrl & i_wstrobe[2] & w_fun_lock_en ? i_wdata[16]   : r_dien  ;
    assign  w_erien_pre = w_sel_trng_ctrl & i_wstrobe[2] & w_fun_lock_en ? i_wdata[17]   : r_erien ;
    assign  w_irqen_pre = w_sel_trng_ctrl & i_wstrobe[3] & w_fun_lock_en ? i_wdata[24]   : r_irqen ;
    assign  w_msel_pre  = w_sel_trng_msel & i_wstrobe[0] & w_fun_lock_en ? i_wdata[0]   : r_msel ;
    assign  w_htf_pre           = w_sel_trng_status & i_wstrobe[0] ? (i_wdata[0] ? 1'b0    : r_htf)    : r_htf  ;
    assign  w_drdy_pre          = w_sel_trng_status & i_wstrobe[0] ? (i_wdata[1] ? 1'b0    : r_drdy)   : r_drdy ;
    assign  w_ererr_pre         = w_sel_trng_status & i_wstrobe[0] ? (i_wdata[2] ? 1'b0    : r_ererr)  : r_ererr;
    assign  w_rand_vld          = (r_msel ? !i_drfifo_empty : !i_trfifo_empty ) & i_trng_rdy ;
    assign  w_drbg_rdy_for_cfg  = i_drbg_rdy_for_cfg & i_trng_rdy ;
    assign  w_reseed_pre  = w_sel_reseed && i_wstrobe[0] && i_wdata[0] ? 1'b1 :1'b0; 
    assign  w_drpten_pre    = w_sel_ht_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[0]    : r_drpten;
    assign  w_drcten_pre    = w_sel_ht_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[1]    : r_drcten;
    assign  w_tapten_pre    = w_sel_ht_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[4]    : r_tapten;
    assign  w_trcten_pre    = w_sel_ht_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[5]    : r_trcten;
    assign  w_tratnben_pre  = w_sel_ht_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[6]    : r_tratnben;
    assign  w_trpokeren_pre = w_sel_ht_ctrl & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7]    : r_trpokeren;
    assign  w_trrunsen_pre  = w_sel_ht_ctrl & i_wstrobe[1] & w_fun_lock_en ? i_wdata[8]    : r_trrunsen;
    assign  w_fsel_pre[1:0]    = w_sel_ro_clk_en & i_wstrobe[2] & w_fun_lock_en ? i_wdata[17:16] : r_fsel[1:0]   ;
    assign  w_rosen_pre[3:0]   = w_sel_ro_clk_en & i_wstrobe[0] & w_fun_lock_en ? i_wdata[3:0]   : r_rosen[3:0]  ;
    assign  w_roen_pre[39:32]   = w_sel_ro_src_en1 & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]  : r_roen[39:32];
    assign  w_roen_pre[47:40]   = w_sel_ro_src_en1 & i_wstrobe[1] & w_fun_lock_en ? i_wdata[15:8] : r_roen[47:40];
    assign  w_roen_pre[55:48]   = w_sel_ro_src_en1 & i_wstrobe[2] & w_fun_lock_en ? i_wdata[23:16]: r_roen[55:48];
    assign  w_roen_pre[63:56]   = w_sel_ro_src_en1 & i_wstrobe[3] & w_fun_lock_en ? i_wdata[31:24]: r_roen[63:56];
    assign  w_roen_pre[7:0]     = w_sel_ro_src_en2 & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]  : r_roen[7:0]  ;
    assign  w_roen_pre[15:8]    = w_sel_ro_src_en2 & i_wstrobe[1] & w_fun_lock_en ? i_wdata[15:8] : r_roen[15:8] ;
    assign  w_roen_pre[23:16]   = w_sel_ro_src_en2 & i_wstrobe[2] & w_fun_lock_en ? i_wdata[23:16]: r_roen[23:16];
    assign  w_roen_pre[31:24]   = w_sel_ro_src_en2 & i_wstrobe[3] & w_fun_lock_en ? i_wdata[31:24]: r_roen[31:24];
    assign  w_drbg_sm4_mode_sel_pre = w_sel_drbg_alg_mode_sel & i_wstrobe[0] & w_fun_lock_en ? i_wdata[4] : r_drbg_sm4_mode_sel ;
    assign  w_drbg_alg_sel_pre      = w_sel_drbg_alg_mode_sel & i_wstrobe[0] & w_fun_lock_en ? i_wdata[0] : r_drbg_alg_sel      ;
    assign  w_tpoker_win_size_pre[9:8]   = w_sel_trbg_ht_win_cfg & i_wstrobe[3] & w_fun_lock_en ? i_wdata[25:24]    : r_tpoker_win_size[9:8]    ;
    assign  w_tpoker_win_size_pre[7:0]   = w_sel_trbg_ht_win_cfg & i_wstrobe[2] & w_fun_lock_en ? i_wdata[23:16]    : r_tpoker_win_size[7:0]    ;
    assign  w_tapt_win_size_pre[9:8]     = w_sel_trbg_ht_win_cfg & i_wstrobe[1] & w_fun_lock_en ? i_wdata[9:8]      : r_tapt_win_size[9:8]      ;   
    assign  w_tapt_win_size_pre[7:0]     = w_sel_trbg_ht_win_cfg & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]      : r_tapt_win_size[7:0]      ;
    assign  w_tapt_thld_pre[15:8]   = w_sel_tapt_thld_cfg & i_wstrobe[1] & w_fun_lock_en ? i_wdata[15:8]    : r_tapt_thld[15:8] ;   
    assign  w_tapt_thld_pre[7:0]    = w_sel_tapt_thld_cfg & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]     : r_tapt_thld[7:0]  ;
    assign  w_taptnb_thld_pre[11:8]   = w_sel_taptnb_thld_cfg & i_wstrobe[1] & w_fun_lock_en ? i_wdata[11:8]    : r_taptnb_thld[11:8] ;   
    assign  w_taptnb_thld_pre[7:0]    = w_sel_taptnb_thld_cfg & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]     : r_taptnb_thld[7:0]  ;
    assign  w_tpoker_thld_pre[23:16]    = w_sel_tpoker_thld_cfg & i_wstrobe[2] & w_fun_lock_en ? i_wdata[23:16] : r_tpoker_thld[23:16]  ;   
    assign  w_tpoker_thld_pre[15:8]     = w_sel_tpoker_thld_cfg & i_wstrobe[1] & w_fun_lock_en ? i_wdata[15:8]  : r_tpoker_thld[15:8]   ;   
    assign  w_tpoker_thld_pre[7:0]      = w_sel_tpoker_thld_cfg & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]   : r_tpoker_thld[7:0]    ;
    assign  w_truns_thld_pre[31:24]  = w_sel_truns_thld_cfg & i_wstrobe[3] & w_fun_lock_en ? i_wdata[31:24]    : r_truns_thld[31:24]  ;   
    assign  w_truns_thld_pre[23:16]  = w_sel_truns_thld_cfg & i_wstrobe[2] & w_fun_lock_en ? i_wdata[23:16]    : r_truns_thld[23:16]  ;   
    assign  w_truns_thld_pre[15:8]   = w_sel_truns_thld_cfg & i_wstrobe[1] & w_fun_lock_en ? i_wdata[15:8]    : r_truns_thld[15:8]  ;   
    assign  w_truns_thld_pre[7:0]    = w_sel_truns_thld_cfg & i_wstrobe[0] & w_fun_lock_en ? i_wdata[7:0]     : r_truns_thld[7:0]   ;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            rst_sync         <= 1'b0;
        end
        else begin
            rst_sync         <= 1'b1;
        end
    end
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            r_rngen             <= 1'b1;
            r_fsel              <= 2'b11;
            r_dien              <= 1'b0;
            r_erien             <= 1'b0;
            r_irqen             <= 1'd0;
            r_msel              <= 1'b1;
            r_htf               <= 1'b0;
            r_drdy              <= 1'b0;
            r_ererr             <= 1'b0;
            r_reseed            <= 1'b0;
            r_trcten            <= 1'b1;
            r_tapten            <= 1'b1;
            r_trrunsen          <= 1'b0;
            r_tratnben          <= 1'b1;
            r_trpokeren         <= 1'b0;
            r_drcten            <= 1'b1;
            r_drpten            <= 1'b1;
            r_trrunsf           <= 1'b0;
            r_trpokerf          <= 1'b0;
            r_tratnbf           <= 1'b0;            
            r_trctf             <= 1'b0;
            r_taptf             <= 1'b0;
            r_trrunsbf          <= 1'b0;
            r_trpokerbf         <= 1'b0;               
            r_taptnbbf          <= 1'b0;
            r_taptbbf           <= 1'b0;
            r_trctbf            <= 1'b0;
            r_drctf             <= 1'b0;
            r_drptf             <= 1'b0;
            r_drptbf            <= 1'b0;
            r_drctbf            <= 1'b0;                 
            r_rosen             <= 4'hF;
            r_roen[63:32]       <= 32'hFFFFFFFF;
            r_roen[31:0]        <= 32'hFFFFFFFF;
            r_drbg_sm4_mode_sel <= 1'b0;
            r_drbg_alg_sel      <= 1'b0;
            r_tpoker_win_size   <= 10'h40;
            r_tapt_win_size     <= 10'h20;
            r_tapt_thld         <= 16'h270;
            r_taptnb_thld       <= 12'd77;
            r_tpoker_thld       <= 24'h180D8D;
            r_truns_thld        <= 32'h6600_6BC0;
        end            
        else begin
            r_rngen             <=  w_rngen_pre;
            r_dien              <=  w_dien_pre;
            r_erien             <=  w_erien_pre;
            r_irqen             <=  w_irqen_pre;
            r_msel              <=  w_msel_pre;
            if(i_htf)
                r_htf               <=  1'b1;
            else
                r_htf               <=  w_htf_pre;
            if(r_reseed | !r_rngen)
                r_drdy              <=  1'b0;
            else if(i_drdy)
                r_drdy              <=  1'b1;
            else
                r_drdy              <=  w_drdy_pre;
            if(i_ererr)
                r_ererr             <= 1'b1;
            else
                r_ererr             <=  w_ererr_pre;
            r_reseed                <=  w_reseed_pre;
            r_trrunsen              <=  w_trrunsen_pre;
            r_trpokeren             <=  w_trpokeren_pre;
            r_tratnben              <=  w_tratnben_pre;                        
            r_trcten                <=  w_trcten_pre;
            r_tapten                <=  w_tapten_pre;
            r_drcten                <=  w_drcten_pre;
            r_drpten                <=  w_drpten_pre;
            if (!r_rngen)begin
                r_trrunsf   <=  1'b0;
                r_trpokerf  <=  1'b0;
                r_tratnbf   <=  1'b0;                         
                r_trctf     <=  1'b0;
                r_taptf     <=  1'b0;
                r_trrunsbf  <=  1'b0;
                r_trpokerbf <=  1'b0;                    
                r_taptnbbf  <=  1'b0;
                r_taptbbf   <=  1'b0;
                r_trctbf    <=  1'b0;
                r_drctf     <=  1'b0;
                r_drptf     <=  1'b0;
                r_drptbf    <=  1'b0;
                r_drctbf    <=  1'b0;
            end    
            else begin
                if(i_trrunsf)   r_trrunsf   <=  1'b1    ;
                if(i_trpokerf)  r_trpokerf  <=  1'b1    ;
                if(i_tratnbf)   r_tratnbf   <=  1'b1    ;                         
                if(i_trctf)     r_trctf     <=  1'b1    ;
                if(i_taptf)     r_taptf     <=  1'b1    ;
                if(i_trrunsbf)  r_trrunsbf  <=  1'b1    ;
                if(i_trpokerbf) r_trpokerbf <=  1'b1    ;                  
                if(i_taptnbbf)  r_taptnbbf  <=  1'b1    ;
                if(i_taptbbf)   r_taptbbf   <=  1'b1    ;
                if(i_trctbf)    r_trctbf    <=  1'b1    ;
                if(i_drctf)     r_drctf     <=  1'b1    ;
                if(i_drptf)     r_drptf     <=  1'b1    ;
                if(i_drptbf)    r_drptbf    <=  1'b1    ;
                if(i_drctbf)    r_drctbf    <=  1'b1    ;
            end    
            if (!rst_sync) begin
                r_roen              <= i_roen;        
                r_rosen             <= i_rosen;        
                r_fsel              <= i_fsel;      
                r_drbg_sm4_mode_sel <= i_cbc_sm4;
                r_drbg_alg_sel      <= i_drbg_mode;                   
            end else begin          
                r_roen              <= w_roen_pre;      
                r_rosen             <= w_rosen_pre;      
                r_fsel              <= w_fsel_pre;      
                r_drbg_sm4_mode_sel <= w_drbg_sm4_mode_sel_pre;
                r_drbg_alg_sel      <= w_drbg_alg_sel_pre;                   
            end               
            r_tpoker_win_size   <=  w_tpoker_win_size_pre ;
            r_tapt_win_size     <=  w_tapt_win_size_pre ;
            r_tapt_thld         <=  w_tapt_thld_pre ;
            r_taptnb_thld       <=  w_taptnb_thld_pre ;
            r_tpoker_thld       <=  w_tpoker_thld_pre ;
            r_truns_thld        <=  w_truns_thld_pre ;       
        end
    end
    always@(*) begin
        o_rdata = 32'h0;
        case(1'b1)
            w_adr_eq_trng_ctrl      : o_rdata =  {7'd0,r_irqen,6'd0,r_erien,r_dien,15'b0,r_rngen}                                    ;
            w_adr_eq_trng_msel      : o_rdata =  {31'd0,r_msel}                                                                      ;
            w_adr_eq_trng_status    : o_rdata =  {23'd0,w_drbg_rdy_for_cfg,3'b0,w_rand_vld,i_trng_rdy,r_ererr,r_drdy,r_htf}          ;
            w_adr_eq_trng_data      : o_rdata =  i_rng_dr                                                                            ;
            w_adr_eq_ro_clk_en      : o_rdata =  {14'h0,r_fsel,12'h0,r_rosen[3:0]}                                                   ;                               
            w_adr_eq_ro_src_en1     : o_rdata =  r_roen[63:32]                                                                       ;
            w_adr_eq_ro_src_en2     : o_rdata =  r_roen[31:0]                                                                        ;
            w_adr_eq_ht_ctrl        : o_rdata =  {23'd0,r_trrunsen,r_trpokeren,r_tratnben,r_trcten,r_tapten,2'd0,r_drcten,r_drpten}  ;
            w_adr_eq_ht_status      : o_rdata =  {3'd0,r_trrunsf,r_trpokerf,r_tratnbf,r_trctf,r_taptf,6'd0,r_drctf,r_drptf,
                                                  3'd0,r_trrunsbf,r_trpokerbf,r_trctbf,r_taptbbf,r_taptnbbf,6'd0,r_drctbf,r_drptbf}          ;
            w_adr_eq_trng_version   : o_rdata =  p_VERSION                                                                                   ;
            w_adr_eq_drbg_alg_hw_sr     : o_rdata =  {23'b0, (1'b1 && p_CONFIG_DRBG_LFSR_EN),3'b0,(1'b1 && p_CONFIG_DRBG_AES_EN),3'b0,(1'b1 && p_CONFIG_DRBG_SM4_EN)} ;
            w_adr_eq_drbg_alg_mode_sel  : o_rdata =  {27'b0,r_drbg_sm4_mode_sel,3'b0,r_drbg_alg_sel}                                  ;
            w_adr_eq_trht_ineration_ptr : o_rdata =  {12'b0,i_rnvld_num,6'b0,i_trht_iteration_ptr}                                    ;
            w_adr_eq_trbg_ht_win_cfg    : o_rdata =  {6'b0,r_tpoker_win_size,6'b0,r_tapt_win_size}                                    ;
            w_adr_eq_tapt_thld_cfg      : o_rdata =  {16'b0,r_tapt_thld   }                                                           ;
            w_adr_eq_taptnb_thld_cfg    : o_rdata =  {20'b0,r_taptnb_thld }                                                           ;
            w_adr_eq_tpoker_thld_cfg    : o_rdata =  {8'b0, r_tpoker_thld }                                                           ;
            w_adr_eq_truns_thld_cfg     : o_rdata =  {r_truns_thld  }                                                           ;            
        endcase
    end
    assign  o_rngen         = r_rngen;
    assign  o_rosen         = r_rosen;
    assign  o_msel          = r_msel;
    assign  o_reseed        = r_reseed;
    assign  o_trcten        = r_trcten;
    assign  o_tapten        = r_tapten;
    assign  o_trunsen       = r_trrunsen;
    assign  o_taptnben      = r_tratnben;
    assign  o_tpokeren      = r_trpokeren;
    assign  o_drcten        = r_drcten;
    assign  o_drpten        = r_drpten;
    assign  o_roen[63:32]   = r_roen[63:32];
    assign  o_roen[31:0]    = r_roen[31:0];
    assign  o_fsel          = r_fsel;
    assign  o_drbg_mode     = r_drbg_alg_sel;
    assign  o_cbc_sm4       = r_drbg_sm4_mode_sel;
    assign  o_tapt_win      = r_tapt_win_size ;
    assign  o_tpoker_win    = r_tpoker_win_size;
    assign  o_tapt_thld     = r_tapt_thld   ;
    assign  o_truns_thld    = r_truns_thld  ;
    assign  o_taptnb_thld   = r_taptnb_thld ;
    assign  o_tpoker_thld   = r_tpoker_thld ;
    assign  o_fifo_pop      =  i_fifo_pop;
    assign  o_irq           =  r_irqen & (r_htf | (r_drdy & r_dien) | (r_ererr & r_erien));  
endmodule
module osr_trng_A000Yc  
 #(
    parameter                   A001Yc                  = 1             ,
    parameter                   A002Yc                  = 1             ,
    parameter                   A003Yc                  = 0             ,
    parameter                   A004Yc                  = 0             ,
    parameter                   A005Yc                  = 32            ,
    parameter                   A006Yc                  = 32'hFFFFFFFF  ,
    parameter                   A007Yc                  = 32            ,
    parameter                   A008Yc                  = 3             ,    
    parameter   [ 5: 0]         A009Yc                  = 41            ,
    parameter   [15: 0]         A010Yc                  = 16'd1024      ,	
    parameter   [15: 0]         A011Yc                  = 16'd624       ,
    parameter   [15: 0]         A012Yc                  = 16'd512       ,	
    parameter   [15: 0]         A013Yc                  = 16'd77
)(
    input   wire                i_clk           ,
`ifdef OSR_TRNG_FPGA      
    input   wire   [ 1: 0]      i_osc_en_clk    ,
`endif            
    input   wire                i_rstn          ,
    input   wire                i_scan_mode     ,
    input   wire                A014Yc          ,
    input   wire                A015Yc          ,
    input   wire                A016Yc          ,
    input   wire                A017Yc          ,
    input   wire                A018Yc          ,
    input   wire                A019Yc          ,
    input   wire    [ 1: 0]     A020Yc          ,
    input   wire    [63: 0]     A021Yc          ,
    input   wire    [ 3: 0]     A022Yc          ,
    output  wire    [3:0]       A023Yc    		,
    output  wire    [3:0]       o_ro_roclk		,
    input   wire    [31: 0]     A024Yc          ,	
    input   wire                A025Yc          ,
    input   wire                A026Yc          ,
    input   wire                A027Yc          ,
    input   wire                A028Yc          ,	
    input    wire   [ 9:0]      A029Yc                         ,
    input    wire   [15:0]      A030Yc                         ,
    input    wire   [31:0]      A031Yc                         ,
    input    wire   [ 9:0]      A032Yc                         ,
    input    wire   [11:0]      A033Yc                         ,
    input    wire   [23:0]      A034Yc                         ,    
    input	wire    			        A035Yc                  ,	
    input   wire    [A008Yc       -1:0] A036Yc                  ,	
    output  wire    [A007Yc      -1:0]  A037Yc                  ,	
    input   wire                        A038Yc                  ,	
    output  wire                        A039Yc                  ,
    output  wire                        A040Yc                  ,
    output  wire                        A041Yc                  ,
    output  wire    [A008Yc       :0]   A042Yc                  ,
    output  wire    [9:0]               A043Yc                  ,    
    output  wire                        A044Yc                  ,    
    output  wire                        A045Yc                  ,
    output  wire                        A046Yc                  ,
    output  wire                        A047Yc                  ,
    output  wire                        A048Yc                  ,
    output  wire                        A049Yc                  ,
    output  wire                        A050Yc                  ,
    output  wire                        A051Yc                  ,
    output  wire                        A052Yc                  ,
    output  wire                        A053Yc                  ,
    output  wire                        A054Yc                  ,
    input   wire                        A055Yc                  ,
    input   wire                        A056Yc                  ,
    input   wire                        A057Yc                  ,
    input   wire                        A058Yc                  ,
    output  wire                        A059Yc                  ,
    output  wire                        A060Yc                  ,
    input   wire                        A061Yc                  ,
    input   wire                        A062Yc                  ,
    input   wire                        A063Yc                  ,
    input   wire                        A064Yc                  ,
    output  wire    [A007Yc      -1:0]  A065Yc                  ,
    output  wire                        A066Yc                  ,
    input   wire                        A067Yc    
);
localparam     A068Yc         = A005Yc          ;
localparam     A069Yc            = A006Yc             ;
    wire                        A070Yc                     ;	
    wire                        A071Yc                     ;	
    wire	[A007Yc      -1:0]	A072Yc                     ;	
    wire                        A073Yc                     ;	
    wire                        A074Yc                     ;
    wire    [A007Yc      -1:0]  A075Yc                     ;	
    wire                        A076Yc                     ;
    wire    [A007Yc      -1:0]  A077Yc                     ;
    wire                        A078Yc                     ;
    wire                        A079Yc                     ;
    wire                        A080Yc                     ;
    wire                        A081Yc                     ;
    wire                        A082Yc                     ;
    wire                        A083Yc                     ;
    wire                        A084Yc                     ;
    wire    [A008Yc       :0]   A085Yc                     ;
    wire    [A007Yc      -1:0]  A086Yc                     ;
    reg                         A087Yc                     ;
    wire                        A088Yc                     ;
    wire                        A089Yc                     ;
    wire                        A090Yc                     ;
    wire                        A091Yc                     ;
    wire                        A092Yc                     ;
    wire                        A093Yc                     ;
    wire                        A094Yc                     ;
    wire                        A095Yc                     ;
    wire                        A096Yc                     ;
    wire                        A097Yc                     ;
    wire                        A098Yc                     ;
    wire                        A099Yc                     ;
    wire                        A100Yc                     ;			
    wire                        A101Yc                     ;
    wire                        A102Yc                     ;
    wire                        A103Yc                     ;
    wire                        A104Yc                     ;		
    wire                        A105Yc                     ;	
    wire                        A106Yc                     ;	
    wire                        A107Yc                     ;	
    wire                        A108Yc                     ;
    wire                        A109Yc                     ;
    wire                        A110Yc                     ;
    wire                        A111Yc                     ;
    wire                        A112Yc                     ;
    wire	[A007Yc      -1:0]	A113Yc                     ;
    wire                        A114Yc                     ;
    wire                        A115Yc                     ;
    wire                        A116Yc                      ;
    wire    [A007Yc      -1:0]  A117Yc                      ;
    wire                        A118Yc                      ;
    wire                        A119Yc                      ;
    wire    [A007Yc      -1:0]  A120Yc                      ;
    wire                        A121Yc                      ;
    wire                        A122Yc                      ;
    wire    [A007Yc      -1:0]  A123Yc                      ;
    wire                        A124Yc                      ;
    reg                         A125Yc                      ;
    reg     [3:0]               A126Yc                      ;
wire        A127Yc                       = 1'b0;
wire        A128Yc                       = 1'b0;
wire        A129Yc                        = 1'b0;
wire        A130Yc                        = 1'b0;
wire        A131Yc                        = 1'b0;
wire        A132Yc                        = 1'b0;
wire                         A133Yc               = 1'b0;
wire                         A134Yc               = 1'b0;
osr_trng_A135Yc      A136Yc    (
    .i_clk                      (i_clk                       ),
    .i_rstn                     (i_rstn                      ),
    .A015Yc                     (A015Yc                      ),
    .A016Yc                     (A016Yc                      ),
    .A017Yc                     (A017Yc                      ),
    .A018Yc                     (A018Yc                      ),
    .A019Yc                     (A019Yc                      ),
    .A137Yc                     (A055Yc                      ),
    .A138Yc                     (A056Yc                      ),
    .A139Yc                     (A057Yc                      ),
    .A058Yc                     (A058Yc                      ),
    .A140Yc                     (A059Yc                      ),
    .A141Yc                     (A060Yc                      ),
    .A142Yc                     (A071Yc                      ),
    .A143Yc                     (A093Yc                      ),
    .A144Yc                     (A094Yc                      ),
    .A145Yc                     (A095Yc                      ),
    .A146Yc                     (A096Yc                      ),
    .A147Yc                     (A097Yc                      ),
    .A148Yc                     (A098Yc                      ),
    .A149Yc                     (A103Yc                      ),
    .A150Yc                     (A104Yc                      ),
    .A151Yc                     (A105Yc                      ),
    .A152Yc                     (A106Yc                      ),
    .A153Yc                     (A107Yc                      ),
    .A154Yc                     (A108Yc                      ),
    .A155Yc                     (A109Yc                      ),
    .A156Yc                     (A110Yc                      ),
    .A157Yc                     (A088Yc                      ),
    .A158Yc                     (A089Yc                      ),
    .A159Yc                     (A090Yc                      ),
    .A160Yc                     (A091Yc                      ),
    .A161Yc                     (A092Yc                      ),
    .A162Yc                     (A100Yc                      ),
    .A163Yc                     (A101Yc                      ),
    .A164Yc                     (A102Yc                      ),
    .A165Yc                     (A112Yc                      ),
    .A166Yc                     (A082Yc                      ),
    .A167Yc                     (A111Yc                      )
);
ro_top 
# ( 
.p_CONFIG_FPGA (A004Yc        ) 
) 
u_ro ( 
.clk (i_clk ), 
`ifdef OSR_TRNG_FPGA 
.osc_en_clk (i_osc_en_clk ), 
`endif 
.resetn (i_rstn ), 
.i_scan (i_scan_mode ), 
.i_sclk_sel (A014Yc     ), 
.i_rngen (A071Yc      ), 
.i_alarm (A070Yc  ), 
.i_conf (A020Yc    ), 
.i_roen (A021Yc    ), 
.i_rosen (A022Yc     ), 
.i_full (A115Yc         ), 
.o_we (A073Yc     ), 
.o_wdata (A072Yc         ), 
.o_rornd (A023Yc     ), 
.o_roclk (o_ro_roclk ) 
); 
osr_trng_A178Yc    #(
    .A179Yc                     ('d1                        ),
    .A180Yc                     (A007Yc                     )
) A181Yc          (
    .clk                        (i_clk                      ),
    .resetn                     (i_rstn                     ),
    .A182Yc                     (1'b1                       ),
    .A183Yc                     (A075Yc                     ),
    .A184Yc                     (A116Yc                     ),
    .A185Yc                     (A076Yc        & A067Yc     ),
    .A186Yc                     (A074Yc                     ),
    .A187Yc                     (A117Yc                     ),
    .A188Yc                     (A118Yc                     ),
    .A189Yc                     (A119Yc                     ),
    .A190Yc                     (),
    .A191Yc                     ()
);
generate
if(A003Yc              ) begin: A192Yc 
osr_trng_A193Yc   A194Yc    (
        .clk                        (i_clk                      ),
        .rst_n                      (i_rstn                     ),
        .A139Yc                     (A057Yc                     ),
        .A195Yc                     (!A119Yc                    ),
        .A196Yc                     (A117Yc                     ),
        .A197Yc                     (A116Yc                     ),
        .A198Yc                     (A122Yc                     ),
        .A199Yc                     (A120Yc                     )
        );
osr_trng_A178Yc    #(
        .A179Yc                     (A008Yc                     ),
        .A180Yc                     (A007Yc                     )
    ) A200Yc     (
        .clk                        (i_clk                      ),
        .resetn                     (i_rstn                     ),
        .A182Yc                     (A036Yc                     ),
        .A183Yc                     (A120Yc                     ),
        .A184Yc                     (A121Yc                     ),
        .A185Yc                     (A122Yc           & A067Yc      ),
        .A186Yc                     (A074Yc                     ),
        .A187Yc                     (A123Yc                     ),
        .A188Yc                     (),
        .A189Yc                     (A124Yc                     ),
        .A190Yc                     ()
    );
    assign A121Yc           = A064Yc    ; 
    assign A065Yc       = A123Yc         ;
    assign A066Yc       = !A124Yc          ;
end
else begin : A201Yc    
    assign A116Yc                = A064Yc     && (!A119Yc               ); 
    assign A065Yc       = A117Yc              ;
    assign A066Yc       = !A119Yc                && !A099Yc            ;
end
endgenerate
osr_trng_A178Yc    #(
    .A179Yc                     (A008Yc                     ),
    .A180Yc                     (A007Yc                     )
) A202Yc       (
    .clk                        (i_clk                      ),
    .resetn                     (i_rstn                     ),
    .A182Yc                     (A036Yc                     ),
    .A183Yc                     (A075Yc                     ),
    .A184Yc                     (A079Yc                     ),
    .A185Yc                     (A080Yc                     ),
    .A186Yc                     (A074Yc                     ),
    .A187Yc                     (A037Yc                     ),
    .A188Yc                     (A082Yc                     ),
    .A189Yc                     (A083Yc                     ),
    .A190Yc                     (A084Yc                     ),
    .A191Yc                     (A085Yc                     )
);
generate
if(A002Yc             ) begin: A203Yc
osr_trng_A204Yc              #(
        .A001Yc                     (A001Yc                     ),
        .A009Yc                     (A009Yc                     ),
        .A010Yc                     (A010Yc                     ),
        .A011Yc                     (A011Yc                     ),
        .A012Yc                     (A012Yc                     ),
        .A013Yc                     (A013Yc                     )
    ) A205Yc    (
        .i_clk                      (i_clk                      ),
        .i_rstn                     (i_rstn                     ),
        .A206Yc                     (A088Yc                     ),
        .A207Yc                     (A089Yc                     ),
        .A208Yc                     (A090Yc                     ),
        .A209Yc                     (A091Yc                     ),
        .A210Yc                     (A092Yc                     ),
        .A067Yc                     (A067Yc                     ),        
        .A211Yc                     (A075Yc                     ),
        .A212Yc                     (A076Yc                     ),
        .A213Yc                     (A081Yc                     ),
        .A214Yc                     (A009Yc                     ),
        .A029Yc                     (A029Yc                     ),
        .A030Yc                     (A030Yc                     ),
        .A031Yc                     (A031Yc                     ),
        .A032Yc                     (A032Yc                     ),
        .A033Yc                     (A033Yc                     ),
        .A034Yc                     (A034Yc                     ),
        .A043Yc                     (A043Yc                     ),        
        .A215Yc                     (A093Yc                     ),
        .A216Yc                     (A094Yc                     ),
        .A217Yc                     (A095Yc                     ),
        .A218Yc                     (A096Yc                     ),
        .A219Yc                     (A097Yc                     ),
        .A220Yc                     (A098Yc                     ),
        .A221Yc                     (A099Yc                     ),
        .A222Yc                     (A100Yc                     ),
        .A223Yc                     (A101Yc                     ),
        .A224Yc                     (A102Yc                     ),
        .A225Yc                     (A103Yc                     ),
        .A226Yc                     (A104Yc                     ),
        .A227Yc                     (A105Yc                     ),
        .A228Yc                     (A106Yc                     ),
        .A229Yc                     (A107Yc                     ),
        .A230Yc                     (A108Yc                     ),
        .A231Yc                     (A109Yc                     ),
        .A232Yc                     (A110Yc                     )
    );
end
else begin: A233Yc   
    assign  A043Yc                      = 10'b0;        
    assign  A093Yc                      = 1'b0;
    assign  A094Yc                      = 1'b0;
    assign  A095Yc                      = 1'b0;
    assign  A096Yc                      = 1'b0;
    assign  A097Yc                      = 1'b0;
    assign  A098Yc                      = 1'b0;   
    assign  A099Yc                      = 1'b0;   
    assign  A103Yc                      = 1'b1; 
    assign  A104Yc                      = 1'b0;
    assign  A105Yc                      = 1'b1;
    assign  A106Yc                      = 1'b0;
    assign  A107Yc                      = 1'b0;
    assign  A108Yc                      = 1'b1;
    assign  A109Yc                      = 1'b0;
    assign  A110Yc                      = 1'b0;
end
endgenerate
   always @(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            A125Yc                  <=  1'b0; 
            A126Yc                  <=  4'h0;
        end else if (A063Yc       ) begin  
            if(A062Yc      && A067Yc    ) begin
                A125Yc                  <=  1'b1;
                A126Yc                  <=  4'h8;
            end        
            else if(!(|A126Yc    )) begin
                A125Yc                  <=  1'b0;
                A126Yc                  <=  A126Yc    ;
            end        
            else if (A080Yc       )begin 
                A125Yc                  <=  A125Yc           ;
                A126Yc                  <=  A126Yc     - 1'b1;
            end
        end else begin
            A125Yc                  <=  1'b0; 
            A126Yc                  <=  4'h0;
        end        
    end
assign  A113Yc              = A072Yc            ;
assign  A114Yc              = A073Yc            ;
assign  A075Yc              = A028Yc            ? A024Yc            : A113Yc               ;
assign  A076Yc              = A028Yc            ? A025Yc            : A114Yc               ;
assign  A080Yc              = A076Yc        & (A118Yc               | !A067Yc    )         ;
assign  A081Yc              = A080Yc        && (!A061Yc           ) && (!A125Yc           );
assign  A079Yc              = A027Yc            ? A026Yc            : A038Yc               ;
assign  A070Yc              = A093Yc        || A094Yc        || A095Yc         || A096Yc          || A097Yc         ;
assign  A074Yc              = A112Yc        || A035Yc      ;
assign  A115Yc              = A111Yc     ? 1'b0 : (A082Yc       & A118Yc              );
assign  A039Yc               = A082Yc                                                   ;
assign  A040Yc               = A083Yc                                                   ;
assign  A041Yc               = A084Yc                                                   ;
assign  A042Yc               = A085Yc                                                 ;
assign  A044Yc               = A098Yc                                                   ;
assign  A045Yc               = A093Yc                                                   ;
assign  A049Yc               = A094Yc                                                   ;
assign  A048Yc               = A095Yc                                                   ;
assign  A047Yc               = A096Yc                                                   ;
assign  A046Yc               = A097Yc                                                   ;
assign  A052Yc               = A105Yc                 && A106Yc                         ;
assign  A051Yc               = A105Yc                 && A107Yc                         ;
assign  A053Yc               = A108Yc                     && A109Yc                     ;
assign  A050Yc               = A108Yc                     && A110Yc                     ;
assign  A054Yc               = A103Yc             && A104Yc                             ;
endmodule
module osr_trng_A135Yc  (
    input   wire            i_clk                       ,
    input   wire            i_rstn                      ,
    input   wire            A015Yc                      ,
    input   wire            A016Yc                      ,
    input   wire            A017Yc                      ,
    input   wire            A018Yc                      ,
    input   wire            A019Yc                      ,
    input   wire            A137Yc                      ,
    input   wire            A138Yc                      ,
    input   wire            A139Yc                      ,
    input   wire            A058Yc                      ,
    output  wire            A140Yc                      ,
    output  wire            A141Yc                      ,
    output  wire            A142Yc                      ,
    input   wire            A143Yc                      ,
    input   wire            A144Yc                      ,
    input   wire            A145Yc                      ,
    input   wire            A146Yc                      ,
    input   wire            A147Yc                      ,
    input   wire            A148Yc                      ,
    input   wire            A149Yc                      ,
    input   wire            A150Yc                      ,
    input   wire            A151Yc                      ,
    input   wire            A152Yc                      ,
    input   wire            A153Yc                      ,
    input   wire            A154Yc                      ,
    input   wire            A155Yc                      ,
    input   wire            A156Yc                      ,
    output  wire            A157Yc                      ,
    output  wire            A158Yc                      ,
    output  wire            A159Yc                      ,
    output  wire            A160Yc                      ,
    output  wire            A161Yc                      ,
    output  wire            A162Yc                      ,
    output  wire            A163Yc                      ,
    output  wire            A164Yc                      ,
    output  wire            A165Yc                      ,
    input   wire            A166Yc                      ,
    output  wire            A167Yc    
);
    localparam	A234Yc          =   4       ;
    localparam	A235Yc          =   4'b0001 ;
    localparam	A236Yc          =   4'b0010 ;
    localparam	A237Yc          =   4'b0011 ;
    localparam	A238Yc          =   4'b0100 ;
    localparam	A239Yc          =   4'b0101 ;
    localparam	A240Yc          =   4'b0110 ;
    localparam	A241Yc          =   4'b0111 ;
    localparam	A242Yc          =   4'b1000 ;
    localparam	A243Yc          =   4'b1001 ;
    reg [A234Yc       -1:0] A244Yc                  ,A245Yc                     ;
    reg                     A246Yc                  ,A247Yc                     ;
    reg                     A248Yc                  ,A249Yc                     ;
    reg                     A250Yc                  ,A251Yc                     ;
    reg                     A252Yc                  ,A253Yc                     ;
    reg                     A254Yc                  ,A255Yc                     ;
    reg                     A256Yc                  ,A257Yc                     ;
    reg                     A258Yc                  ,A259Yc                     ;
    reg                     A260Yc                  ,A261Yc                     ;
    reg                     A262Yc                  ,A263Yc                     ;
    reg                     A264Yc                  ,A265Yc                     ;
    reg                     A266Yc                  ,A267Yc                     ;
    reg [3:0]               A268Yc                  ,A269Yc                     ;
    reg [3:0]               A270Yc                  ,A271Yc                     ;
    reg                     A272Yc                  ,A273Yc                     ;
    reg                     A274Yc                  ,A275Yc                     ;
always @(posedge i_clk or negedge i_rstn)
    if(!i_rstn) begin
        A244Yc                  <= A235Yc   ;
        A246Yc                  <= 1'b0     ;
        A248Yc                  <= 1'b0     ;
        A250Yc                  <= 1'b0     ;
        A252Yc                  <= 1'b0     ;
        A254Yc                  <= 1'b0     ;
        A256Yc                  <= 1'b0     ;
        A258Yc                  <= 1'b0     ;
        A260Yc                  <= 1'b0     ;
        A262Yc                  <= 1'b0     ;
        A264Yc                  <= 1'b0     ;
        A266Yc                  <= 1'b0     ;
        A268Yc                  <= 4'd0     ;
        A270Yc                  <=  'd0     ;
        A272Yc                  <= 1'b0     ;
        A274Yc                  <= 1'b0     ;
    end
    else begin
        A244Yc                  <= A139Yc   ? A245Yc                        : A235Yc    ;
        A246Yc                  <= A139Yc   ? A247Yc                        : 1'b0      ;
        A248Yc                  <= A139Yc   ? A249Yc                        : 1'b0      ;
        A250Yc                  <= A139Yc   ? A251Yc                        : 1'b0      ;
        A252Yc                  <= A139Yc   ? A253Yc                        : 1'b0      ;
        A254Yc                  <= A139Yc   ? A255Yc                        : 1'b0      ;
        A256Yc                  <= A139Yc   ? A257Yc                        : 1'b0      ;
        A258Yc                  <= A139Yc   ? A259Yc                        : 1'b0      ;
        A260Yc                  <= A139Yc   ? A261Yc                        : 1'b0      ;
        A262Yc                  <= A139Yc   ? A263Yc                        : 1'b0      ;
        A264Yc                  <= A139Yc   ? A265Yc                        : 1'b1      ;
        A266Yc                  <= A139Yc   ? A267Yc                        : 1'b0      ;
        A268Yc                  <= A139Yc   ? A269Yc                        : 4'd0      ;
        A270Yc                  <= A139Yc   ? A271Yc                        :  'd0      ;
        A272Yc                  <= A139Yc   ? A273Yc                        : 1'b0      ;
        A274Yc                  <= A139Yc   ? A275Yc                        : 1'b0      ;
    end
always@(*)
    begin
        A245Yc                      = A244Yc                ;
        A247Yc                      = A246Yc                ;
        A249Yc                      = A248Yc                ;
        A251Yc                      = A250Yc                ;
        A253Yc                      = A252Yc                ;
        A255Yc                      = A254Yc                ;
        A257Yc                      = A256Yc                ;
        A259Yc                      = A258Yc                ;
        A261Yc                      = A260Yc                ;
        A263Yc                      = A262Yc                ;
        A265Yc                      = A264Yc                ;
        A267Yc                      = A266Yc                ;
        A269Yc                      = A268Yc                ;
        A271Yc                      = A270Yc                ;
        A273Yc                      = A272Yc                ;
        A275Yc                      = A274Yc                ;
        case(A244Yc)
            A235Yc  : begin
                A247Yc                      = 1'b0  ;	
                A249Yc                      = 1'b0  ;	
                A251Yc                      = 1'b0  ;	
                A253Yc                      = 1'b0  ;	
                A255Yc                      = 1'b0  ;	
                A257Yc                      = 1'b0  ;	
                A259Yc                      = 1'b0  ;	
                A261Yc                      = 1'b0  ;	
                A263Yc                      = 1'b0  ;	
                A265Yc                      = 1'b1  ;	
                A267Yc                      = 1'b0  ;	
                A269Yc                      = 4'd0  ;	
                A271Yc                      =  'd0  ;
                A273Yc                      = 1'b0  ;
                A275Yc                      = 1'b0  ;
                if(A137Yc        ) begin
                    A245Yc = A236Yc     ;
                end
                else begin
                    A245Yc              = A238Yc        ;
                    A249Yc              = 1'b1          ;
                    A251Yc              = 1'b1          ;
                    A253Yc              = 1'b0          ;
                    A255Yc              = 1'b1          ;
                    A257Yc              = 1'b0          ;
                    A265Yc              = 1'b1          ;
                end
            end 
            A238Yc      : begin
                A265Yc       = 1'b0;
                A269Yc     = A268Yc + 1'b1;
                A259Yc             = 1'b0;
                if(A268Yc==4'd7) begin
                    A259Yc             = 1'b1;
                end
                else if (A149Yc         ) begin
                    A245Yc              = !A150Yc          ? A239Yc      : A237Yc ;
                    A275Yc              = !A150Yc          ? 1'b0        : 1'b1;
                    A271Yc     [0]      = !A150Yc           ;
                    A269Yc              = 'd0               ;
                end
                else if(A268Yc>7) begin
                    A269Yc     = A268Yc;
                end
            end
            A239Yc      : begin
                A269Yc     = A268Yc + 1'b1;
                A261Yc                 = 1'b0;
                if(A268Yc==4'd7) begin
                    A261Yc                 = 1'b1;
                end
                else if (A151Yc             ) begin
                    A245Yc              = !(A152Yc              || A153Yc          )  ? A240Yc        : A237Yc ;
                    A275Yc              = !(A152Yc              || A153Yc          )  ? 1'b0          : 1'b1;
                    A271Yc     [1]      = !(A152Yc              || A153Yc          );
                    A269Yc              = 'd0;
                end
                else if(A268Yc>7) begin
                    A269Yc     = A268Yc;
                end
            end
            A240Yc      : begin
                A269Yc     = A268Yc + 1'b1;
                A263Yc                     = 1'b0;
                if(A268Yc==4'd7) begin
                    A263Yc                     = 1'b1;
                end
                else if (A154Yc                 ) begin
                    A245Yc              = !(A155Yc                  || A156Yc           ) ? A241Yc      : A237Yc ;
                    A275Yc              = !(A155Yc                  || A156Yc           ) ? 1'b0        : 1'b1;
                    A271Yc     [2]      = !(A155Yc                  || A156Yc           );
                    A269Yc              = 'd0;
                end
                else if(A268Yc>7) begin
                    A269Yc     = A268Yc;
                end
            end
            A241Yc      : begin
                A247Yc              = 1'b1 ;	
                A249Yc              = 1'b1 ;	
                A251Yc              = 1'b1 ;
                A253Yc              = 1'b0 ;
                A255Yc              = 1'b1 ;
                A257Yc              = 1'b0 ;
                A267Yc              = 1'b0 ;	
                A265Yc              = 1'b0 ;
                A245Yc              = (!A143Yc     && !A144Yc     && !A146Yc       )   ? A244Yc : A237Yc ;	
                A275Yc              = (!A143Yc     && !A144Yc     && !A146Yc       )   ? 1'b0 : 1'b1;
                if((A268Yc==4'd7)&&A166Yc     ) begin
                    A269Yc      = 'd0;
                    A245Yc      = A242Yc     ;
                end
                else if(A166Yc     ) begin
                    A265Yc       = 1'b1;
                end
                else if(A264Yc ) begin
                    A269Yc     = A268Yc + 1'b1;
                end
            end
            A242Yc      : begin
                if(!A148Yc   ) begin
                    A245Yc              = (!A143Yc     && !A144Yc     && !A146Yc       )   ? A243Yc          : A237Yc ;
                    A275Yc              = (!A143Yc     && !A144Yc     && !A146Yc       )   ? 1'b0            : 1'b1;
                    A271Yc     [3]      = (!A143Yc     && !A144Yc     && !A146Yc       );
                    A265Yc              = 1'b0  ;
                end
            end
            A243Yc          : begin
                A273Yc                  = 1'b1  ;	
                A249Yc                  = 1'b0  ;	
                A251Yc                  = 1'b0  ;
                A253Yc                   = 1'b0  ;
                A255Yc                  = 1'b0  ;
                A257Yc               = 1'b0  ;
                A267Yc                  = 1'b1  ;	
                A275Yc                  = !(&A271Yc     );
                A265Yc              = 1'b0  ;
                if(A058Yc           )
                    A265Yc              = 1'b1  ;
                if(A138Yc         ) begin
                    A267Yc              = 1'b0  ;	
                    A245Yc              = A236Yc       ;	
                    A265Yc              = 1'b1 ;	
                end
            end
            A236Yc      : begin
                A247Yc              = 1'b1;
                A249Yc              = A015Yc     ;
                A251Yc              = A016Yc     ;
                A253Yc              = A017Yc      ;
                A255Yc              = A018Yc          ;
                A257Yc              = A019Yc       ;
                A265Yc              = 1'b0;
                if(A143Yc     || A144Yc    ) begin
                    A245Yc = A237Yc ;
                end
            end
            A237Yc  : begin
                A247Yc              = 1'b0;
                A249Yc              = 1'b0;
                A251Yc              = 1'b0;
                A253Yc              = 1'b0;
                A255Yc              = 1'b0;
                A257Yc              = 1'b0;
                A275Yc              = 1'b0;
            end
            default : A245Yc = A235Yc ;
        endcase
    end
    assign  A142Yc                   = A246Yc                ;
    assign  A157Yc                   = A248Yc                ;
    assign  A158Yc                   = A250Yc                ;
    assign  A159Yc                   = A252Yc                ;
    assign  A160Yc                   = A254Yc                ;
    assign  A161Yc                   = A256Yc                ;
    assign  A162Yc                   = A258Yc                ;
    assign  A163Yc                   = A260Yc                ;
    assign  A164Yc                   = A262Yc                ;
    assign  A165Yc                   = A264Yc                ;
    assign  A167Yc                   = A266Yc                ;
    assign  A140Yc                   = A272Yc                ;
    assign  A141Yc                   = A274Yc                ;
endmodule
module osr_trng_A193Yc  
(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire                            A139Yc  
   ,input   wire                            A195Yc    
   ,input   wire    [31:0]                  A196Yc   
   ,output  wire                            A197Yc    
   ,output  wire                            A198Yc
   ,output  wire    [31:0]                  A199Yc
);
    localparam                              A234Yc              = 2'd3;
    localparam                              A276Yc              = 3'b000,
                                            A277Yc              = 3'b001,
                                            A278Yc              = 3'b010,
                                            A279Yc              = 3'b011,
                                            A280Yc              = 3'b100,
                                            A281Yc              = 3'b101;
    genvar                                  A282Yc;
    reg     [A234Yc       -1:0]             A283Yc       , A284Yc    ;
    reg     [127:0]                         A285Yc;
    wire                                    A286Yc;
    reg                                     A287Yc;
    reg                                     A288Yc    ;
    reg     [31:0]                          A289Yc;
    reg     [3:0]                           A290Yc   ;
    reg     [14:0]                          A291Yc      ;
    reg     [5:0]                           A292Yc    ;
    assign  A286Yc  = A285Yc[127] ^ A285Yc[125] ^ A285Yc[100] ^ A285Yc[98];
    always @(posedge clk or negedge rst_n)
    begin 
        if(!rst_n)
            A283Yc          <=  A276Yc;
        else
            A283Yc          <=  A284Yc    ;
    end
    always @(*)
    begin
        A284Yc     = A283Yc       ;
        case(A283Yc       )
            A276Yc      :   A284Yc     = A139Yc     ? (A195Yc     ? A277Yc : A276Yc)
                                                    : A276Yc;
            A277Yc      :   A284Yc     = A139Yc     ? A278Yc
                                                    : A276Yc;
            A278Yc      :   A284Yc     = A139Yc     ? A280Yc
                                                    : A276Yc;
            A279Yc      :   A284Yc     = A139Yc     ? (A195Yc     ? A277Yc : A279Yc)
                                                    : A276Yc;
            A280Yc      :   A284Yc     = A139Yc     ? ((A290Yc   [3:0] == 4'h4) 
                                                        ? A281Yc : A279Yc)
                                                    : A276Yc;
            A281Yc      :   A284Yc     = A139Yc     ? ((A291Yc      [14] == 1'b1)
                                                        ? A276Yc : A281Yc)
                                                    : A276Yc;
            default     :   A284Yc     = A276Yc;
        endcase
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A285Yc      <=  128'h0;
            A287Yc      <=  1'b0;
            A288Yc      <=  1'b0;
            A289Yc      <=  32'h0;
            A290Yc      <=  3'd0;
            A292Yc      <=  6'd1;
        end
        else begin
            case(A284Yc    )
                A276Yc  :   begin                    
                                A285Yc      <=  128'h0;
                                A287Yc      <=  1'b0;
                                A288Yc      <=  1'b0;
                                A289Yc      <=  32'h0;
                                A290Yc      <=  3'd0;
                                A292Yc      <=  6'd1;
                            end
                A277Yc  :   begin
                                A288Yc      <=  1'b1;
                            end
                A278Yc  :   begin
                                A290Yc      <=  A290Yc    + 3'd1;
                                A288Yc      <=  1'b0;
                            end
                A280Yc  :   begin
                                case(A290Yc   )
                                    3'd1    :   A285Yc[31:0]    <=  A196Yc   ;
                                    3'd2    :   A285Yc[63:32]   <=  A196Yc   ;
                                    3'd3    :   A285Yc[95:64]   <=  A196Yc   ;
                                    3'd4    :   A285Yc[127:96]  <=  A196Yc   ;
                                    default :   ;
                                endcase
                            end
                A281Yc  :   begin
                                A285Yc      <=  {A285Yc[126:0],A286Yc};
                                A290Yc      <=  3'd0;
                                if(A292Yc    [5] == 0) begin
                                    A292Yc      <=  A292Yc     + 6'd1;
                                    A287Yc      <=  1'b0;
                                end
                                else begin
                                    A292Yc      <=  6'd1;
                                    A287Yc      <=  1'b1;
                                end
                            end
                default :   ;
            endcase
        end
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A291Yc          <=  15'd1;
        else if(A283Yc        == A281Yc) begin
            if(A292Yc     == 6'd32)
                A291Yc          <=  A291Yc       + 15'd1;
            else
                A291Yc       <= A291Yc      ;
        end
        else
            A291Yc          <=  15'd1;
    end
    assign  A197Yc      = A288Yc    ;
    assign  A198Yc  = A287Yc;
    assign  A199Yc  = A285Yc[31:0];
endmodule
module osr_trng_A293Yc       # 
    (
    parameter   A294Yc                  =   128             ,
    parameter   A295Yc                  =   128             ,
    parameter   A296Yc                  =   128             ,
    parameter   A297Yc                  =   256             ,
    parameter   A180Yc                  =   32              ,
    parameter   A298Yc                  =   8               
    )
    (
    input  wire                             clk                 ,
    input  wire                             resetn              ,
    input  wire                             A299Yc              ,
    input  wire                             A300Yc              ,
    input  wire                             A301Yc              ,
    input  wire                             A302Yc              ,
    output wire                             A303Yc              ,
    input  wire                             A304Yc              ,
    input  wire [A180Yc     -1:0]           A305Yc              ,
    input  wire                             A306Yc              ,
    input  wire                             A307Yc              ,
    output wire                             A308Yc              ,
    output wire                             A309Yc              ,
    input  wire [A180Yc     -1:0]           A310Yc              ,
    input  wire [A297Yc   -1:0]             A311Yc              ,
    input  wire [A297Yc   -1:0]             A312Yc              ,
    output wire [A294Yc  -1:0]              A313Yc              ,
    output wire [A295Yc  -1:0]              A314Yc              ,
    output wire [A180Yc     -1:0]           A315Yc              ,
    output wire [A180Yc     -1:0]           A316Yc              ,
    output wire                             A317Yc              ,
    output wire                             A318Yc              ,
    input  wire                             A319Yc              ,
    output wire                             A320Yc              ,
    input  wire                             A321Yc              ,
    output wire                             A322Yc              ,
    input  wire                             A323Yc              ,
    input  wire                             A324Yc              ,
    input  wire                             A325Yc              ,
    input  wire                             A326Yc              ,
    input  wire                             A327Yc              ,
    input  wire                             A328Yc              ,
    input  wire                             A329Yc              ,
    output wire                             A330Yc              ,
    input  wire                             A331Yc              ,
    input  wire                             A332Yc              ,
    output wire                             A333Yc              ,
    input  wire                             A334Yc              ,
    input  wire [A180Yc     -1:0]           A335Yc              ,
    input  wire [A298Yc         -1:0]       A336Yc          
    );
    localparam  A337Yc                  =   A180Yc     /A298Yc         ;
    localparam  A338Yc                  =   A337Yc   == 2 ? 1 : 0
                                        |   A337Yc   == 4 ? 2 : 0
                                        |   A337Yc   == 8 ? 3 : 0;
    localparam  A339Yc                  =   A295Yc  /A180Yc     ;
    localparam  A340Yc                  =   A296Yc /A180Yc     ;
    localparam  A341Yc                  =   A339Yc      <=  2 ? 1 : 0
                                        |   A339Yc      <=  4 ? 2 : 0
                                        |   A339Yc      <=  8 ? 3 : 0
                                        |   A339Yc      <= 16 ? 4 : 0;
    localparam  A342Yc                  =   3               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   3'd0            ,
                A344Yc                  =   3'd1            ,
                A345Yc                  =   3'd2            ,
                A346Yc                  =   3'd3            ,
                A347Yc                  =   3'd4            ,
                A348Yc                  =   3'd5            ,
                A349Yc                  =   3'd6            ,
                A350Yc                  =   3'd7            ;
    reg         [A342Yc     -1:0]           A283Yc          ;
    reg         [A342Yc     -1:0]           A284Yc          ;
    reg         [A294Yc  -1:0]              A351Yc          ;
    reg         [A294Yc  -1:0]              A352Yc          ;
    reg         [A295Yc  -1:0]              A353Yc          ;
    reg         [A180Yc     -1:0]           A354Yc          ;
    wire                                    A355Yc          ;
    reg                                     A356Yc          ;
    wire                                    A357Yc          ;
    reg         [A338Yc        -1:0]        A358Yc          ;
    reg                                     A359Yc          ;
    reg         [A180Yc     -1:0]           A360Yc          ;
    reg         [A180Yc     -1:0]           A361Yc          ;
    reg                                     A362Yc          ;
    wire                                    A363Yc          ;
    wire                                    A364Yc          ;
    wire                                    A365Yc          ;
    wire                                    A366Yc          ;
    wire                                    A367Yc          ;
    wire                                    A368Yc          ;
    wire        [A297Yc   -1:0]             A369Yc          ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A283Yc                      <=  A343Yc          ;
        else if (~A299Yc  )
            A283Yc                      <= A343Yc        ;
        else
            A283Yc                      <= A284Yc        ;
    always @ (*)
        begin
        A284Yc                          =   A343Yc          ;
        case (A283Yc       )
            A343Yc:
                if (A300Yc   )
                    A284Yc              =   A344Yc          ;
                else if (A301Yc   )
                    A284Yc              =   A346Yc          ;
                else if (A302Yc   )
                    A284Yc              =   A348Yc          ;
                else
                    A284Yc              =   A343Yc          ;
            A344Yc  :
                A284Yc                  =   A345Yc          ;
            A345Yc:
                if (A319Yc     )
                    A284Yc              =   A350Yc          ;
                else
                    A284Yc              =   A345Yc          ;
            A346Yc  :
                A284Yc                  =   A347Yc          ;
            A347Yc:
                if (A321Yc     )
                    A284Yc              =   A350Yc          ;
                else
                    A284Yc              =   A347Yc          ;
            A348Yc  :
                A284Yc                  =   A349Yc          ;
            A349Yc:
                if (A323Yc     )
                    begin
                    if (A355Yc  )
                        A284Yc          =   A346Yc          ;
                    else
                        A284Yc          =   A343Yc          ;
                    end
                else
                    A284Yc              =   A349Yc          ;
            A350Yc:
                A284Yc                  =   A343Yc          ;
            endcase
        end
    assign  A308Yc          =   A328Yc        | A329Yc        |A331Yc      ;
    assign  A365Yc          =   (A283Yc        == A345Yc) & A325Yc     ;
    assign  A366Yc          =   ((A283Yc        == A347Yc)
                            |   (A283Yc        == A349Yc)) & A325Yc     ;
    assign  A330Yc          =   A306Yc        & A308Yc       ;
    always @ (posedge clk or negedge resetn)
        if (!resetn) begin
            A362Yc                      <=  1'b0            ;
        end
        else if (A306Yc       ) begin
            A362Yc                      <= A330Yc        ;
        end
        else  begin             
            A362Yc                      <= 1'b0          ;
        end
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A351Yc                      <=  {A294Yc  {1'b0}};
            A352Yc                      <=  {A294Yc  {1'b0}};
            end
        else if (A300Yc   )
            begin
            A351Yc                      <= {A294Yc  {1'b0}};
            end
        else if (A362Yc          & A363Yc      )
            begin
            A351Yc                      <= {A351Yc [A294Yc  -1-A180Yc     :0],
                                A310Yc   ^A351Yc [A294Yc  -1:A294Yc  -A180Yc     ]};
            end
        else if (A326Yc        )
            begin
            A352Yc                  <= {A352Yc      [A294Yc  -1-A298Yc         :0]
                                            ,A336Yc    }    ;
            end
        else if (A365Yc    )
                A351Yc                      <= A352Yc       
                                            ^   A311Yc   [A297Yc   -1:A297Yc   -A294Yc  ];
        else if (A366Yc    )                
                A351Yc                      <= A352Yc       
                                            ^   A312Yc   [A297Yc   -1:A297Yc   -A294Yc  ];
        else
                A351Yc                      <= A351Yc ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A353Yc                      <=  {A295Yc  {1'b0}};
            end
        else if (A300Yc   )
            A353Yc                      <= {A295Yc  {1'b0}};
        else if (A324Yc      )
            A353Yc                      <= {A335Yc       
                                            ,A353Yc[A295Yc  -1:A180Yc     ]};
        else if (A334Yc        && A307Yc   )
            A353Yc                      <= {A353Yc[A295Yc  -1-A298Yc         :0]
                                            ,A336Yc    }    ;
        else if (A362Yc          & A364Yc    )
                A353Yc                      <= {A353Yc[A295Yc  -1-A180Yc     :0],
                                    A310Yc   ^A353Yc[A295Yc  -1:A295Yc  -A180Yc     ]};
        else if (A327Yc      )
            A353Yc                      <= {A353Yc[A295Yc  -1-A298Yc         :0]
                                            ,A336Yc    }    ;
        else if (A365Yc    )
                A353Yc                      <= A353Yc
                                            ^   A311Yc   [A295Yc  -1:0];
        else if (A366Yc    )
            A353Yc                          <= A353Yc
                                            ^   A312Yc   [A295Yc  -1:0];
        else
            A353Yc                          <= A353Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A354Yc                      <=  {A180Yc     {1'b0}};
            end
        else if (A301Yc    && A299Yc  )
            A354Yc                      <= A305Yc          + 1'b1;
        else if (A319Yc     )
            begin
            A354Yc                      <= 'd1           ;
            end
        else if (A321Yc     )
            A354Yc                      <= 'd1           ;
        else if (A322Yc   )
            A354Yc                      <= A354Yc       + 1'b1;
        else;
    assign  A355Yc      =   (A354Yc       == A305Yc         ) | (A354Yc       == (A305Yc          + 1'b1));
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A361Yc                      <=  0               ;
            end
        else if (A362Yc         )
            begin
            A361Yc                      <= A361Yc     + 1'b1;
            end
        else if (A334Yc        & A307Yc   )
            A361Yc                      <= 0             ;
        else if (A303Yc)
            begin
            A361Yc                      <= A361Yc        ;
            end
        else
            begin
            A361Yc                      <= 0             ;
            end
    assign  A364Yc        =   A303Yc & (A361Yc     < A339Yc      +{A180Yc     {1'b0}});
    assign  A363Yc        =   A303Yc & (~A364Yc    )    ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A358Yc                      <=  0               ;
            end
        else if (A334Yc       )
            begin
            A358Yc                      <= A358Yc      + 1'b1;
            end
        else
            begin
            A358Yc                      <= 0             ;
            end
    assign  A357Yc          =   A358Yc      == (A337Yc  -1);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A359Yc                      <=  1'b0            ;
            end
        else if (A357Yc      )
            begin
            A359Yc                      <= 1'b1          ;
            end
        else
            begin
            A359Yc                      <= 1'b0          ;
            end
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            A360Yc                      <=  0               ;
            end
        else if (A334Yc       )
            begin
            A360Yc                  <= {A360Yc     [A180Yc     -1-A298Yc         :0]
                                            ,A336Yc    }    ;
            end
        else;
    assign  A318Yc      =   A283Yc        == A344Yc     ;
    assign  A320Yc      =   A283Yc        == A346Yc     ;
    assign  A322Yc      =   A283Yc        == A348Yc     ;
    assign  A313Yc      =   A351Yc                      ;
    assign  A314Yc      =   A353Yc                      ; 
    assign  A333Yc          =   A304Yc                  ;
    assign  A309Yc          =   A359Yc                  ;
    assign  A316Yc          =   {A180Yc     {A309Yc       }} 
                            &   A360Yc                  ;
    assign  A315Yc      =   A354Yc                      ;
    assign  A303Yc      =   A283Yc        != A343Yc     ;
    assign  A317Yc      = A283Yc        == A346Yc     ;
endmodule
module osr_trng_A370Yc            #
    (
    parameter   A371Yc                  =   256             ,
    parameter   A295Yc                  =   128             ,
    parameter   A180Yc                  =   32              ,
    parameter   A372Yc                  =   128             
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A307Yc          ,
    output wire                             A373Yc          ,
    input  wire                             A374Yc          ,
    output wire                             A375Yc          ,    
    input  wire                             A302Yc          ,
    output wire                             A376Yc          ,
    input  wire                             A377Yc          ,
    output wire                             A378Yc          ,
    output wire                             A379Yc          ,
    input  wire                             A380Yc          ,
    output wire                             A381Yc          ,
    input  wire                             A325Yc          ,
    output wire                             A382Yc          ,
    input  wire                             A383Yc          ,
    input  wire                             A384Yc          
    );
    localparam  A385Yc                  =   A295Yc   <= 2**1 ? 1 : 0
                                        |   A295Yc   <= 2**2 ? 2 : 0
                                        |   A295Yc   <= 2**3 ? 3 : 0
                                        |   A295Yc   <= 2**4 ? 4 : 0
                                        |   A295Yc   <= 2**5 ? 5 : 0
                                        |   A295Yc   <= 2**6 ? 6 : 0
                                        |   A295Yc   <= 2**7 ? 7 : 0
                                        |   A295Yc   <= 2**8 ? 8 : 0;
    localparam  A340Yc                  =   A295Yc  /A180Yc     ;
    localparam  A386Yc                  =   A371Yc   <=  1*A372Yc     ?  1 : 0
                                        |   A371Yc   <=  2*A372Yc     ?  2 : 0
                                        |   A371Yc   <=  3*A372Yc     ?  3 : 0
                                        |   A371Yc   <=  4*A372Yc     ?  4 : 0
                                        |   A371Yc   <=  5*A372Yc     ?  5 : 0
                                        |   A371Yc   <=  6*A372Yc     ?  6 : 0
                                        |   A371Yc   <=  7*A372Yc     ?  7 : 0
                                        |   A371Yc   <=  8*A372Yc     ?  8 : 0
                                        |   A371Yc   <=  9*A372Yc     ?  9 : 0
                                        |   A371Yc   <= 10*A372Yc     ? 10 : 0
                                        |   A371Yc   <= 11*A372Yc     ? 11 : 0
                                        |   A371Yc   <= 12*A372Yc     ? 12 : 0
                                        |   A371Yc   <= 13*A372Yc     ? 13 : 0
                                        |   A371Yc   <= 14*A372Yc     ? 14 : 0
                                        |   A371Yc   <= 15*A372Yc     ? 15 : 0
                                        |   A371Yc   <= 16*A372Yc     ? 16 : 0;
    localparam  A387Yc                  =   A386Yc   <=  1 ? 1 : 0
                                        |   A386Yc   <=  3 ? 2 : 0
                                        |   A386Yc   <=  7 ? 3 : 0
                                        |   A386Yc   <= 15 ? 4 : 0;
    localparam  A342Yc                  =   4               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   4'd0            ,
                A388Yc                  =   4'd1            ,
                A389Yc                  =   4'd2            ,
                A390Yc                  =   4'd3            ,
                A391Yc                  =   4'd4            ,
                A392Yc                  =   4'd5            ,
                A393Yc                  =   4'd6            ,
                A394Yc                  =   4'd7            ,
                A395Yc                  =   4'd8            ,
                A396Yc                  =   4'd9            ,
                A397Yc                  =   4'd10           ;
    reg         [A385Yc       -1:0]         A398Yc          ;
    wire                                    A399Yc          ;
    reg         [A342Yc     -1:0]           A283Yc          ;
    reg         [A342Yc     -1:0]           A284Yc          ;
    reg         [A387Yc        -1:0]        A400Yc          ;
    wire                                    A401Yc          ;
    wire                                    A402Yc          ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A283Yc                      <=  A343Yc          ;
        else if (~A299Yc  )
            A283Yc                      <= A343Yc        ;
        else
            A283Yc                      <= A284Yc        ;
    always @ (*)
        begin
        A284Yc                          =   A343Yc          ;
        case (A283Yc       )
            A343Yc:
                if (A302Yc   )
                    if (A377Yc        )
                        A284Yc          =   A390Yc          ;
                    else
                        A284Yc          =   A388Yc          ;  
                else
                    A284Yc              =   A343Yc          ;
            A394Yc  :
                if (A399Yc      )
                    A284Yc              =   A395Yc          ;
                else
                    A284Yc              =   A394Yc          ;
            A395Yc       :
                A284Yc                  =   A392Yc          ;    
            A388Yc       :
                A284Yc                  =   A389Yc          ;
            A389Yc    :
               if (A325Yc     )begin
                   if (A307Yc   )
                       A284Yc      =   A394Yc          ;
                   else
                       A284Yc      =   A390Yc          ;
                end    
                else
                    A284Yc              =   A389Yc          ;
            A390Yc    :
                A284Yc                  =   A391Yc          ;
            A391Yc :
                if (A380Yc       )
                    A284Yc              =   A392Yc          ;
                else
                    A284Yc              =   A391Yc          ;
            A392Yc     :
                A284Yc                  =   A393Yc          ;
            A393Yc  :
                if (A383Yc    )
                    begin
                        if (A402Yc  )
                            A284Yc          =   A396Yc          ;
                        else begin
                            if(A307Yc   )
                                A284Yc      =   A394Yc         ; 
                            else   
                            A284Yc          =   A390Yc          ;
                        end
                    end
                else
                    A284Yc              =   A393Yc          ;
            A396Yc          :
                A284Yc                  =   A397Yc          ;
            A397Yc       :
                if (A325Yc     )
                    A284Yc              =   A343Yc          ;
                else
                    A284Yc              =   A397Yc          ;
            default:    ;
        endcase
        end
    always @ (posedge clk or negedge resetn)
        begin
        if (!resetn)
            A400Yc                      <=  0               ;
        else if (A302Yc   )
            A400Yc                      <= 0             ;
        else if (A401Yc      )
            A400Yc                      <= A400Yc          + 1'b1;
        else;
        end
    always @ (posedge clk or negedge resetn)
        begin
        if (!resetn)
            A398Yc                      <=  0               ;
        else if ((A284Yc     == A343Yc)|(A284Yc     == A392Yc     ))
            A398Yc                      <= 0             ;
        else if (A307Yc    &(A374Yc   & A373Yc  ))
            A398Yc                      <= A398Yc          + 1'b1;
        else;
        end
    assign  A399Yc      =   A398Yc          == A340Yc      +{A385Yc       {1'b0}};
    assign  A373Yc      =   A284Yc     == A394Yc            ;
    assign  A401Yc      =   (A283Yc        == A393Yc  ) & A383Yc    ;
    assign  A402Yc      =   (A400Yc          +5'b0) == (A386Yc   - 1 +{A387Yc        {1'b0}}+5'b0);
    assign  A379Yc      =   A283Yc        == A390Yc         ;
    assign  A381Yc      =   A283Yc        == A388Yc         
                        |   A283Yc        == A396Yc          ;
    assign  A382Yc      =   A283Yc        == A392Yc         ;
    assign  A378Yc          =   (A283Yc        == A393Yc  ) & A384Yc        ;
    assign  A375Yc          =   A399Yc                          ;
    assign  A376Yc      =   A283Yc        == A397Yc        & A325Yc     ;
endmodule
module osr_trng_A403Yc               # 
    (
    parameter   A297Yc                  =   256             ,
    parameter   A180Yc                  =   32              
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A300Yc          ,
    output wire                             A404Yc          ,
    output wire                             A373Yc          ,
    input  wire                             A374Yc          ,
    output wire                             A381Yc          ,
    input  wire                             A325Yc          
    );
    localparam  A341Yc                  =   A297Yc    <= 2**1 ? 1 : 0
                                        |   A297Yc    <= 2**2 ? 2 : 0
                                        |   A297Yc    <= 2**3 ? 3 : 0
                                        |   A297Yc    <= 2**4 ? 4 : 0
                                        |   A297Yc    <= 2**5 ? 5 : 0
                                        |   A297Yc    <= 2**6 ? 6 : 0
                                        |   A297Yc    <= 2**7 ? 7 : 0
                                        |   A297Yc    <= 2**8 ? 8 : 0;
    localparam  A339Yc                  =   A297Yc   /A180Yc     ;
    localparam  A342Yc                  =   3               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   3'd0            ,
                A388Yc                  =   3'd1            ,
                A389Yc                  =   3'd2            ,
                A394Yc                  =   3'd4            ,
                A350Yc                  =   3'd5            ;
    reg         [A342Yc     -1:0]           A283Yc          ;
    reg         [A342Yc     -1:0]           A284Yc          ;
    reg         [A341Yc       -1:0]         A405Yc          ;
    wire                                    A399Yc          ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A283Yc                      <=  A343Yc          ;
        else if (~A299Yc  )
            A283Yc                      <= A343Yc        ;
        else
            A283Yc                      <= A284Yc        ;
    always @ (*)
        begin
        A284Yc                          =   A343Yc          ;
        case (A283Yc       )
            A343Yc:
                if (A300Yc   )begin
                    A284Yc              =   A388Yc          ;
                end
                else
                    A284Yc              =   A343Yc          ;
            A388Yc       :
                A284Yc                  =   A389Yc          ;
            A389Yc    :
                if (A325Yc     )
                    A284Yc              =   A394Yc          ;
                else
                    A284Yc              =   A389Yc          ;
            A394Yc  :
                if (A399Yc      )
                    A284Yc              =   A350Yc          ;
                else
                    A284Yc              =   A394Yc          ;
            A350Yc:
                A284Yc                  =   A343Yc          ;
            default:    ;
        endcase
        end
    always @ (posedge clk or negedge resetn)
        begin
        if (!resetn)
            A405Yc                      <=  0               ;
        else if (A284Yc     == A343Yc)
            A405Yc                      <= 0             ;
        else if (A374Yc   & A373Yc  )
            A405Yc                      <= A405Yc          + 1'b1;
        else;
        end
    assign  A399Yc      =   A405Yc          == A339Yc      +{A341Yc       {1'b0}} ;
    assign  A373Yc      =   A284Yc     == A394Yc            ;
    assign  A381Yc      =   A283Yc        == A388Yc         ;
    assign  A404Yc      =   A283Yc        == A350Yc         ;
endmodule
module osr_trng_A406Yc          # 
    (
    parameter   A297Yc                  =   256             ,
    parameter   A180Yc                  =   32              
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A301Yc          ,
    output wire                             A407Yc          ,
    output wire                             A373Yc          ,
    input  wire                             A374Yc          ,
    output wire                             A381Yc          ,
    input  wire                             A325Yc          
    );
    localparam  A408Yc                  =   A297Yc    <= 2**1 ? 1 : 0
                                        |   A297Yc    <= 2**2 ? 2 : 0
                                        |   A297Yc    <= 2**3 ? 3 : 0
                                        |   A297Yc    <= 2**4 ? 4 : 0
                                        |   A297Yc    <= 2**5 ? 5 : 0
                                        |   A297Yc    <= 2**6 ? 6 : 0
                                        |   A297Yc    <= 2**7 ? 7 : 0
                                        |   A297Yc    <= 2**8 ? 8 : 0;
    localparam  A409Yc                  =   A297Yc   /A180Yc     ;
    localparam  A342Yc                  =   3               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   3'd0            ,
                A394Yc                  =   3'd2            ,
                A388Yc                  =   3'd3            ,
                A389Yc                  =   3'd4            ,
                A350Yc                  =   3'd5            ;
    reg         [A342Yc     -1:0]           A283Yc          ;
    reg         [A342Yc     -1:0]           A284Yc          ;
    reg         [A408Yc    -1:0]            A405Yc          ;
    wire                                    A399Yc          ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A283Yc                      <=  A343Yc          ;
        else if (~A299Yc  )
            A283Yc                      <= A343Yc        ;
        else
            A283Yc                      <= A284Yc        ;
    always @ (*)
        begin
        A284Yc                          =   A343Yc          ;        
        case (A283Yc       )
            A343Yc:
                if (A301Yc   )begin
                    A284Yc              =   A388Yc          ;
                end
                else
                    A284Yc              =   A343Yc          ;
            A388Yc       :
                A284Yc                  =   A389Yc          ;
            A389Yc    :
                if (A325Yc     )
                    A284Yc              =   A394Yc          ;
                else
                    A284Yc              =   A389Yc          ;
            A394Yc  :
                if (A399Yc      )
                    A284Yc              =   A350Yc          ;
                else
                    A284Yc              =   A394Yc          ;
            A350Yc:
                A284Yc                  =   A343Yc          ;
            default:    ;
        endcase
        end
    always @ (posedge clk or negedge resetn)
        begin
        if (!resetn)
            A405Yc                      <=  0               ;
        else if (A284Yc     == A343Yc)
            A405Yc                      <= 0             ;
        else if (A374Yc   & A373Yc  )
            A405Yc                      <= A405Yc          + 1'b1;
        else;
        end
    assign  A399Yc          =   A405Yc          == A409Yc      +{A408Yc    {1'b0}};
    assign  A373Yc          =   A284Yc     == A394Yc            ;
    assign  A381Yc          =   A283Yc        == A388Yc         ;
    assign  A407Yc          =   A283Yc        == A350Yc         ;
endmodule
module osr_trng_A410Yc       # 
    (
    parameter   A411Yc                  =   1               ,
    parameter   A412Yc                  =   0               ,
    parameter   A294Yc                  =   128             ,
    parameter   A295Yc                  =   128             ,
    parameter   A296Yc                  =   128             ,   
    parameter   A297Yc                  =   256             ,
    parameter   A180Yc                  =   32              ,
    parameter   A371Yc                  =   256             ,
    parameter   A298Yc                  =   8               
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A300Yc          ,
    input  wire                             A301Yc          ,
    input  wire                             A302Yc          ,
    input  wire                             A307Yc          ,
    input  wire                             A413Yc          ,
    output wire                             A303Yc          ,
    input  wire                             A304Yc          ,
    input  wire [A180Yc     -1:0]           A305Yc          ,
    input  wire                             A306Yc          ,
    output wire                             A308Yc          ,
    output wire                             A414Yc          ,
    input  wire [A180Yc     -1:0]           A310Yc          ,
    input  wire [A297Yc   -1:0]             A311Yc          ,
    input  wire [A297Yc   -1:0]             A312Yc          ,
    output wire [31:0]                      A313Yc          ,
    output wire [31:0]                      A314Yc          ,
    output wire [A180Yc     -1:0]           A315Yc          ,
    output wire                             A317Yc          ,
    output wire                             A415Yc          ,
    output wire [A180Yc     -1:0]           A416Yc          
    );
    wire                                    A417Yc          ;
    wire                                    A418Yc          ;
    wire                                    A419Yc          ;
    wire                                    A420Yc          ;
    wire                                    A421Yc          ;
    wire                                    A422Yc          ;
    wire                                    A423Yc          ;
    wire                                    A424Yc          ;
    wire                                    A425Yc          ;
    wire                                    A426Yc          ;
    wire                                    A427Yc          ;
    wire                                    A428Yc          ;
    wire        [A295Yc  -1:0]              A429Yc          ;
    wire        [A294Yc  -1:0]              A430Yc          ;
    wire                                    A431Yc          ;    
    wire                                    A432Yc             ;
    wire                                    A433Yc          ;
    wire                                    A434Yc          ;
    wire        [A180Yc     -1:0]           A435Yc          ;
    wire                                    A436Yc          ;
    wire                                    A437Yc          ;
    wire                                    A438Yc          ;
    wire                                    A439Yc          ;
    wire                                    A440Yc          ;
    wire                                    A441Yc          ;
    wire                                    A442Yc          ;
    wire                                    A443Yc          ;
    wire                                    A444Yc          ;
    wire                                    A445Yc          ;
    wire        [A298Yc         -1:0]       A446Yc          ;
    wire                                    A447Yc          ;
    wire                                    A448Yc          ;
    wire                                    A449Yc              ;
    wire                                    A450Yc              ;
    wire                                    A451Yc              ;
osr_trng_A293Yc       # 
    (
    .A294Yc          (A294Yc           ),
    .A295Yc          (A295Yc           ),
    .A296Yc          (A296Yc           ),
    .A180Yc          (A180Yc           )
    )
    A452Yc        
    (
    .clk             (clk              ),
    .resetn          (resetn           ),
    .A299Yc          (A299Yc           ),
    .A307Yc          (A307Yc           ),
    .A300Yc          (A300Yc           ),
    .A301Yc          (A301Yc           ),
    .A302Yc          (A302Yc           ),
    .A303Yc          (A303Yc           ),
    .A304Yc          (A304Yc           ),
    .A305Yc          (A305Yc           ),
    .A309Yc          (A415Yc           ),
    .A308Yc          (A308Yc           ),
    .A306Yc          (A306Yc           ),
    .A310Yc          (A310Yc           ),
    .A311Yc          (A311Yc           ),
    .A312Yc          (A312Yc           ),
    .A313Yc          (A430Yc           ),
    .A314Yc          (A429Yc           ),
    .A315Yc          (A315Yc           ),
    .A316Yc          (A416Yc           ),
    .A317Yc          (A317Yc           ),
    .A318Yc          (A417Yc           ),
    .A319Yc          (A418Yc           ),
    .A320Yc          (A419Yc           ),
    .A321Yc          (A420Yc           ),
    .A322Yc          (A421Yc           ),
    .A323Yc          (A422Yc           ),
    .A324Yc          (A423Yc           ),
    .A325Yc          (A439Yc           ),
    .A326Yc          (A424Yc           ),
    .A327Yc          (A425Yc           ),
    .A329Yc          (A427Yc           ),
    .A328Yc          (A426Yc           ),
    .A331Yc          (A431Yc           ),
    .A332Yc                (A432Yc             ),
    .A330Yc          (A428Yc           ),   
    .A333Yc          (A433Yc           ),
    .A334Yc          (A434Yc           ),
    .A335Yc          (A435Yc           ),
    .A336Yc      (A446Yc           )
    );
osr_trng_A403Yc               #
    (
    .A297Yc          (A297Yc           ),
    .A180Yc          (A180Yc           )
    )
    A453Yc                
    (
    .clk             (clk              ),
    .resetn          (resetn           ),
    .A299Yc          (A299Yc           ),
    .A300Yc          (A417Yc           ),
    .A404Yc          (A418Yc           ),
    .A373Yc          (A426Yc           ),
    .A374Yc          (A428Yc           ),
    .A381Yc          (A436Yc           ),
    .A325Yc          (A439Yc           )
    );
osr_trng_A406Yc          # 
    (
    .A297Yc          (A297Yc           ),
    .A180Yc          (A180Yc           )
    )
    A454Yc           
    (
    .clk             (clk              ),
    .resetn          (resetn           ),
    .A299Yc          (A299Yc           ),
    .A301Yc          (A419Yc           ),
    .A407Yc          (A420Yc           ),
    .A373Yc          (A427Yc           ),
    .A374Yc          (A428Yc           ),
    .A381Yc          (A437Yc           ),
    .A325Yc          (A439Yc           )
    );
osr_trng_A370Yc            #
    (
    .A371Yc          (A371Yc           ),    
    .A295Yc          (A295Yc           ),
    .A372Yc          (A295Yc           )
    )
    A455Yc             
    (
    .clk             (clk              ),
    .resetn          (resetn           ),
    .A299Yc          (A299Yc           ),
    .A307Yc          (A307Yc           ),
    .A302Yc          (A421Yc           ),
    .A376Yc          (A422Yc           ),
    .A377Yc          (A433Yc           ),
    .A378Yc          (A434Yc           ),
    .A379Yc          (A441Yc           ),
    .A380Yc          (A443Yc           ),
    .A381Yc          (A438Yc           ),
    .A325Yc          (A439Yc           ),
    .A373Yc          (A431Yc           ),
    .A374Yc          (A428Yc           ),
    .A375Yc          (A432Yc             ),
    .A382Yc          (A445Yc           ),
    .A383Yc          (A448Yc           ),
    .A384Yc          (A447Yc           )
    );
osr_trng_A456Yc          #
    (
    .A294Yc          (A294Yc           ),
    .A295Yc          (A295Yc           ),
    .A372Yc          (A295Yc           ),
    .A180Yc          (A180Yc           )
    )
    A457Yc           
    (
    .clk             (clk              ),
    .resetn          (resetn           ),
    .A299Yc          (A299Yc           ),
    .A458Yc          (A436Yc           ),
    .A459Yc          (A437Yc           ),
    .A460Yc          (A438Yc           ),
    .A461Yc          (A439Yc           ),
    .A462Yc          (A424Yc           ),
    .A463Yc          (A425Yc           ),
    .A379Yc          (A440Yc           ),
    .A380Yc          (A443Yc           ),
    .A382Yc          (A444Yc           ),
    .A383Yc          (A448Yc           ),
    .A384Yc          (A447Yc           )
    );
osr_trng_A464Yc A465Yc  
    (
    .clk             (clk              ),
    .resetn          (resetn           ),
    .A466Yc          (A440Yc           ),
    .A467Yc          (A441Yc           ),
    .A468Yc          (A423Yc           ),
    .A469Yc          (A443Yc           ),
    .A470Yc          (A314Yc           ),
    .A471Yc          (A435Yc           )
    );
osr_trng_A472Yc   #
    (       .A411Yc                 (A411Yc                 ),
            .A412Yc                 (A412Yc                 )
    )
    A473Yc    
    (
    .clk            (clk              ),
    .resetn         (resetn           ),
    .A299Yc         (A299Yc           ),
    .A474Yc         (A444Yc           ),
    .A475Yc         (A445Yc           ),
    .A336Yc       (A429Yc           ),
    .A476Yc         (A430Yc           ),
    .A413Yc         (A413Yc           ),
    .A477Yc      (A446Yc           ),
    .A478Yc         (A447Yc           ),
    .A479Yc         (A448Yc           )
    );
    assign A314Yc      = A429Yc  [31:0]  ;
    assign A313Yc      = A430Yc    [31:0];
    assign A414Yc       = A431Yc    ;
endmodule
module osr_trng_A456Yc          #
    (
    parameter   A294Yc                  =   128             ,
    parameter   A295Yc                  =   128             ,
    parameter   A372Yc                  =   128             ,
    parameter   A180Yc                  =   32              
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A458Yc          ,
    input  wire                             A459Yc          ,
    input  wire                             A460Yc          ,
    output wire                             A461Yc          ,
    output wire                             A462Yc          ,
    output wire                             A463Yc          ,
    output wire                             A379Yc          ,
    input  wire                             A380Yc          ,
    output wire                             A382Yc          ,
    input  wire                             A383Yc          ,
    input  wire                             A384Yc          
    );
    localparam  A297Yc                  =   A294Yc   + A295Yc  ;
    localparam  A480Yc                  =   A294Yc   <=   A372Yc     ? 1 : 0
                                        |   A294Yc   <= 2*A372Yc     ? 2 : 0;
    localparam  A481Yc                  =   A295Yc   <=   A372Yc     ? 1 : 0
                                        |   A295Yc   <= 2*A372Yc     ? 2 : 0;
    localparam  A482Yc                  =   A480Yc       + A481Yc       - 1;
    localparam  A387Yc                  =   A482Yc         < 2 ? 1 : 0
                                        |   A482Yc         < 4 ? 2 : 0
                                        |   A482Yc         < 8 ? 3 : 0;
    localparam  A342Yc                  =   3               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   3'd0            ,
                A390Yc                  =   3'd1            ,
                A391Yc                  =   3'd2            ,
                A392Yc                  =   3'd3            ,
                A393Yc                  =   3'd4            ,
                A350Yc                  =   3'd5            ;
    reg         [A342Yc     -1:0]           A283Yc          ;
    reg         [A342Yc     -1:0]           A284Yc          ;
    reg         [A387Yc        -1:0]        A400Yc          ;
    wire                                    A483Yc          ;
    wire                                    A402Yc          ;
    wire                                    A484Yc          ;
    wire                                    A485Yc          ;
    always @ (posedge clk or negedge resetn)
        begin
        if (!resetn)
            A283Yc                      <=  A343Yc          ;
        else if (~A299Yc  )
            A283Yc                      <= A343Yc        ;
        else
            A283Yc                      <= A284Yc        ;
        end
    always @ (*)
        begin
        A284Yc                          =   A343Yc          ;        
            case (A283Yc       )
                A343Yc:
                    if (A483Yc )
                        A284Yc          =   A390Yc          ;
                    else
                        A284Yc          =   A343Yc          ;
                A390Yc    :
                    A284Yc              =   A391Yc          ;
                A391Yc :
                    if (A380Yc       )
                        A284Yc          =   A392Yc          ;
                    else
                        A284Yc          =   A391Yc          ;
                A392Yc     :
                    A284Yc              =   A393Yc          ;
                A393Yc  :
                    if (A383Yc    )
                        begin
                        if (A402Yc  )
                            A284Yc      =   A350Yc          ;
                        else
                            A284Yc      =   A390Yc          ;
                        end
                    else
                        A284Yc          =   A393Yc          ;
                A350Yc:
                    A284Yc              =   A343Yc          ;
                default:    ;
            endcase
        end
    always @ (posedge clk or negedge resetn)
        begin
        if (!resetn)
            A400Yc                      <=  0               ;
        else if (A483Yc )
            A400Yc                      <= 0             ;
        else if (A383Yc    )
            A400Yc                      <= A400Yc          + 1'b1;
        else;
        end
    assign  A483Yc      =   A458Yc         | A459Yc         | A460Yc        ;
    assign  A402Yc      =   (A400Yc          +3'b0)== (A482Yc         +{A387Yc        {1'b0}}+3'b0);
    assign  A484Yc      =   ((A400Yc          +3'b0)< (A480Yc       +{A387Yc        {1'b0}}+3'b0)) 
                        &&  (A283Yc        != A343Yc)        ;
    assign  A485Yc      =   ((A400Yc          +3'b0)>= (A480Yc       +{A387Yc        {1'b0}}+3'b0)) 
                        &&  (A283Yc        != A343Yc)        ;
    assign  A382Yc          =   A283Yc        == A392Yc     ;
    assign  A379Yc          =   A283Yc        == A390Yc     ;
    assign  A461Yc          =   A283Yc        == A350Yc     ;
    assign  A463Yc          =   A384Yc         & A485Yc     
                            &   (A283Yc        == A393Yc  ) ;
    assign  A462Yc          =   A384Yc         & A484Yc   
                            &   (A283Yc        == A393Yc  ) ;
endmodule
module osr_trng_A486Yc   # 
    (
    parameter   A179Yc                  =   10              ,
    parameter   A294Yc                  =   128             ,
    parameter   A295Yc                  =   128             ,
    parameter   A296Yc                  =   128             ,
    parameter   A297Yc                  =   256             ,
    parameter   A180Yc                  =   32              
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A487Yc          ,
    output wire                             A488Yc          ,
    output wire                             A489Yc          ,
    input  wire                             A490Yc          ,
    input   wire                            A491Yc          ,
    input  wire                             A492Yc          ,
    input  wire                             A493Yc          ,
    input  wire [A179Yc      -1:0]          A494Yc          ,
    output wire                             A495Yc           ,
    output wire                             A496Yc          ,
    output wire                             A497Yc           ,
    output wire [A179Yc      :0]            A498Yc             ,
    output wire [A180Yc     -1:0]           A499Yc          ,
    input  wire                             A500Yc          ,
    input  wire [A180Yc     -1:0]           A501Yc          ,
    output wire                             A502Yc          ,
    output wire                             A503Yc          ,
    output wire                             A504Yc           ,
    output wire                             A505Yc          ,
    output wire                             A506Yc          ,
    output wire                             A507Yc          ,
    output wire                             A508Yc          ,
    input  wire                             A509Yc          ,
    output wire                             A510Yc          ,
    output wire [A180Yc     -1:0]           A511Yc            ,
    output wire                             A512Yc          ,
    input  wire                             A513Yc          ,
    output wire [A180Yc     -1:0]           A514Yc          ,
    output wire [A297Yc   -1:0]             A515Yc          ,
    output wire [A297Yc   -1:0]             A516Yc          ,
    input  wire [31:0]                      A517Yc          ,
    input  wire [31:0]                      A518Yc          ,
    input  wire [A180Yc     -1:0]           A519Yc          ,
    input  wire                             A520Yc          ,
    input  wire [A180Yc     -1:0]           A521Yc          ,
    output wire                             A522Yc          ,
    input  wire                             A523Yc          ,
    input  wire                             A524Yc          ,
    output wire                             A525Yc          ,
    input  wire                             A526Yc          ,
    input  wire                             A527Yc          ,
    input  wire                             A528Yc          ,
    input  wire                             A529Yc          ,
    input  wire [A180Yc     -1:0]           A530Yc           ,
    output wire                             A531Yc          ,
    output wire [31:0]                      A532Yc          ,
    output wire [31:0]                      A533Yc          ,
    output wire [A180Yc     -1:0]           A534Yc          ,
    output wire                             A535Yc          ,
    output wire [A180Yc     -1:0]           A536Yc          ,
    input  wire                             A537Yc          ,
    input  wire [A180Yc     -1:0]           A538Yc          ,
    input  wire [A297Yc   -1:0]             A539Yc          ,
    input  wire [A297Yc   -1:0]             A540Yc          ,
    input   wire                            A541Yc          ,
    input   wire                            A542Yc          ,
    input   wire                            A543Yc          ,
    input  wire [A180Yc     -1:0]           A544Yc          ,
    input  wire                             A545Yc          ,
    input  wire                             A546Yc           ,
    input  wire                             A547Yc          ,
    input  wire                             A548Yc          ,
    input  wire [A179Yc      :0]            A549Yc          ,
    output wire [A179Yc      -1:0]          A550Yc           ,
    output wire [A180Yc     -1:0]           A551Yc          ,
    output wire                             A552Yc          ,
    output wire                             A553Yc          ,
    output wire                             A554Yc          
    );
    localparam  A342Yc                  =   4               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   4'd0            ,
                A555Yc                  =   4'd1            ,
                A556Yc                  =   4'd2            ,
                A557Yc                  =   4'd3            ,
                A558Yc                  =   4'd4            ,
                A559Yc                  =   4'd5            ,
                A560Yc                  =   4'd6            ,
                A561Yc                  =   4'd7            ,
                A562Yc                  =   4'd8            ,
                A563Yc                  =   4'd9            ,
                A564Yc                  =   4'd10           ,
                A565Yc                  =   4'd11           ,
                A566Yc                  =   4'd12           ,
                A567Yc                  =   4'd13           ,
                A568Yc                  =   4'd14           ;
    localparam  A569Yc                  =   32'd1000        ;
    reg         [A342Yc     -1:0]           A283Yc          ;
    reg         [A342Yc     -1:0]           A284Yc          ;
    wire                                    A570Yc          ;
    reg         [1:0]                      A571Yc           ; 
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A283Yc                      <=  A343Yc          ;
        else if (~A299Yc  )
            A283Yc                      <= A343Yc        ;
        else
            A283Yc                      <= A284Yc        ;
    always @ (*)
        begin
        A284Yc                          =   A343Yc          ;
        case (A283Yc       )
            A343Yc:
                begin
                if (A299Yc  )
                    begin
                    if (A487Yc     )
                        A284Yc          =   A566Yc          ;
                    else
                        A284Yc          =   A555Yc          ;
                    end
                else
                    A284Yc              =   A343Yc          ;
                end
            A555Yc  :
                A284Yc                  =   A556Yc          ;
            A556Yc:
                if (A523Yc     )
                    begin
                    if (A524Yc     )
                        A284Yc          =   A568Yc          ;
                    else
                        A284Yc          =   A557Yc           ;
                    end
                else
                    A284Yc              =   A556Yc          ;
            A557Yc           :
                A284Yc                  =   A558Yc          ;
            A558Yc        :
                if (~A509Yc      )
                    A284Yc              =   A559Yc           ;
                else
                    A284Yc              =   A558Yc          ;
            A559Yc           :
                A284Yc                  =   A560Yc          ;
            A560Yc        :
                if (~A509Yc      )
                    A284Yc              =   A561Yc             ;
                else
                    A284Yc              =   A560Yc          ;
            A561Yc             :
                A284Yc                  =   A562Yc          ;
            A562Yc        :
                if (A492Yc         )
                    A284Yc              =   A563Yc          ;
                else
                    A284Yc              =   A562Yc          ;
            A563Yc     :
                    A284Yc              =   A564Yc          ;
            A564Yc       :
                A284Yc                  =   A565Yc          ;
            A565Yc    :
                if (~A509Yc      )
                    A284Yc              =   A566Yc          ;
                else
                    A284Yc              =   A565Yc          ;
            A566Yc:
                if (!A509Yc      )
                    begin
                    if ((~A546Yc           ) & A490Yc       )
                        A284Yc          =   A567Yc          ;
                    else
                        A284Yc          =   A566Yc          ;
                    end
                else
                    A284Yc              =   A566Yc          ;
            A567Yc       :
                A284Yc                  =   A566Yc          ;
            A568Yc    :
                A284Yc                  =   A568Yc          ;
            default:    ;
        endcase
        end
    assign  A505Yc          =   A299Yc                      ;
    assign  A506Yc          =   A570Yc     ? A526Yc         :
                                            ((A283Yc        == A564Yc       ) 
                                            | (A283Yc        == A557Yc           ));
    assign  A507Yc          =   A570Yc     ? A527Yc         : A491Yc  ;
    assign  A508Yc          =   A570Yc     ? A528Yc         :
        ((A283Yc        == A567Yc       ) | (A283Yc        == A559Yc           ));
    assign  A510Yc          =   A570Yc     ? A529Yc         :
                                             1'b0           ;
    assign  A511Yc            = A570Yc     ? A530Yc           :
                                             A569Yc         ;
    assign  A512Yc          =   A570Yc     ? A537Yc         :
                                             ~A500Yc        ;
    assign  A514Yc          =   A570Yc     ? A538Yc         :
                                             A501Yc         ;
    assign  A515Yc          =   A570Yc     ? A539Yc         : 'd0;
    assign  A516Yc          =   A570Yc     ? A540Yc         : 'd0;
    assign  A522Yc          =   A283Yc        == A555Yc     ;
    assign  A525Yc          =   A509Yc                      ;
    assign  A531Yc          =   A513Yc                      ;
    assign  A532Yc          =   A517Yc                      ;
    assign  A533Yc          =   A518Yc                      ;
    assign  A534Yc          =   A519Yc                      ;
    assign  A535Yc          =   A520Yc                      ;
    assign  A536Yc          =   A521Yc                      ;
    assign  A550Yc           =  A494Yc                      ;
    assign  A551Yc          =   A521Yc                      ;
    assign  A552Yc          =   A493Yc                      ;
    assign  A553Yc          =   A520Yc                      ;
    assign  A554Yc          =   A492Yc          | A523Yc      | (~A299Yc  ) ;
    assign  A570Yc          =   A283Yc        == A556Yc     ;
    assign  A488Yc          =   (A524Yc      & A523Yc     ) | A541Yc     | A542Yc    ;
    always @(posedge clk or negedge resetn)
    begin
        if(!resetn)
            A571Yc              <=  2'b0; 
        else if(A523Yc     )
                A571Yc          [0] <=  1'b1;
        else if(A543Yc        )
                A571Yc          [1] <=  1'b1;
        else if(&A571Yc          )
                A571Yc              <=  2'b00;
    end
    assign  A489Yc          =   &A571Yc                      ;
    assign  A495Yc           =  A547Yc                      ;
    assign  A496Yc          =   A545Yc                      ;
    assign  A497Yc           =  A548Yc                      ;
    assign  A498Yc             =A549Yc                      ;
    assign  A499Yc          =   A544Yc                      ;
    assign  A502Yc          =   A513Yc                      ;
    assign  A503Yc          =   A283Yc        == A566Yc     ;
    assign  A504Yc           =  A283Yc        == A561Yc             ;
endmodule
module osr_trng_A572Yc   # 
    (
    parameter   A411Yc                  =   1               ,
    parameter   A412Yc                  =   0               ,
    parameter   A573Yc                  =   0               ,
    parameter   A574Yc                  =   1               ,
    parameter   A180Yc                  =   32              ,
    parameter   A179Yc                  =   5               ,
    parameter   A294Yc                  =   128             ,
    parameter   A296Yc                  =   128             ,
    parameter   A295Yc                  =   128  
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc              ,
    input  wire                             A487Yc              ,
    input   wire                            A015Yc              ,
    input   wire                            A575Yc              ,
    input   wire                            A491Yc              ,
    output wire                             A488Yc              ,
    output wire                             A489Yc              ,
    input  wire                             A490Yc              ,
    input  wire                             A576Yc              , 
    input  wire                             A307Yc              ,
    input  wire                             A492Yc              ,
    output wire                             A503Yc              ,
    output wire                             A577Yc              ,
    output wire                             A578Yc              ,
    output wire                             A579Yc              ,
    input  wire                             A493Yc              ,
    input  wire [A179Yc      -1:0]          A494Yc              ,
    output wire                             A495Yc              ,
    output wire                             A496Yc              ,
    output wire                             A497Yc              ,
    output wire [A179Yc      :0]            A498Yc              ,
    output wire [A180Yc     -1:0]           A499Yc              ,
    input  wire                             A500Yc              ,
    input  wire [A180Yc     -1:0]           A501Yc              ,
    output wire                             A502Yc              ,
    input  wire [9:0]                       A580Yc              ,
    output wire [9:0]                       A581Yc                   ,
    output  wire                            A582Yc              ,
    output  wire                            A583Yc              ,
    output  wire                            A584Yc              ,
    output  wire                            A585Yc    
    );
    wire                                    A586Yc        ;
    assign      A586Yc         = A307Yc    ? ((A411Yc               && A412Yc               && !A576Yc         ) ? 1'b1 :
                                              ((A412Yc               && !A411Yc              ) ? 1'b1 : 1'b0)) : 1'b0;
    localparam  A297Yc                  =   A294Yc   + A295Yc  ;
    wire                                    A587Yc          ;
    wire                                    A588Yc          ;
    wire                                    A589Yc          ;
    wire                                    A590Yc          ;
    wire                                    A591Yc          ;
    wire                                    A592Yc          ;
    wire        [A180Yc     -1:0]           A593Yc             ;
    wire                                    A594Yc          ;
    wire                                    A595Yc          ;
    wire        [A180Yc     -1:0]           A596Yc          ;
    wire        [A297Yc   -1:0]             A597Yc          ;
    wire        [A297Yc   -1:0]             A598Yc          ;
    wire        [31:0]                      A599Yc           ;
    wire        [31:0]                      A600Yc          ;
    wire                                    A601Yc              ;
    wire        [A180Yc     -1:0]           A602Yc           ;
    wire                                    A603Yc           ;
    wire        [A180Yc     -1:0]           A604Yc          ;
    wire                                    A605Yc          ;
    wire                                    A606Yc          ;
    wire                                    A607Yc          ;
    wire                                    A608Yc          ;
    wire                                    A609Yc          ;
    wire                                    A610Yc          ;
    wire                                    A611Yc          ;
    wire        [A180Yc     -1:0]           A612Yc            ;
    wire                                    A613Yc          ;
    wire        [31:0]                      A614Yc          ;
    wire        [31:0]                      A615Yc          ;
    wire        [31:0]                      A616Yc          ;
    wire        [A180Yc     -1:0]           A617Yc          ;
    wire                                    A618Yc            ;
    wire        [A180Yc     -1:0]           A619Yc          ;
    wire                                    A620Yc          ;
    wire        [A180Yc     -1:0]           A621Yc          ;
    wire        [A297Yc   -1:0]             A622Yc          ;
    wire        [A297Yc   -1:0]             A623Yc          ;
    wire        [A180Yc     -1:0]           A624Yc          ;
    wire                                    A625Yc          ;
    wire                                    A626Yc          ;
    wire                                    A627Yc          ;
    wire        [A179Yc      :0]            A628Yc          ;
    wire        [A179Yc      -1:0]          A629Yc          ;
    wire                                    A630Yc          ;
    wire                                    A631Yc          ;
    wire                                    A632Yc          ;
    wire                                    A633Yc          ;
    wire                                    A634Yc          ;
    wire    [31:0]                          A635Yc          ;
    wire                                    A636Yc          ;
    wire                                    A637Yc          ;
    wire    [31:0]                          A638Yc          ;
    wire                                    A639Yc          ;
    wire                                    A640Yc          ;  
    wire                                    A641Yc          ;  
    wire                                    A642Yc          ;  
    wire                                    A643Yc          ;  
    wire    [31:0]                          A644Yc          ;  
    wire                                    A645Yc          ;  
    wire                                    A646Yc          ;  
    wire                                    A647Yc          ;  
    wire    [31:0]                          A648Yc          ;  
    wire    [A180Yc     -1:0]               A649Yc          ;  
    wire                                    A650Yc          ; 
    wire                                    A651Yc          ;
    wire                                    A652Yc          ;
    wire    [A179Yc      -1:0]              A653Yc              ;
    wire    [A179Yc      -1:0]              A654Yc               ;
    wire    [A180Yc     -1:0]               A655Yc         ;
    wire                                    A656Yc          ;
    wire                                    A657Yc          ; 
    wire                                    A658Yc          ;
    wire    [A180Yc     -1:0]               A659Yc            ;
    wire                                    A660Yc            ;    
    wire                                    A661Yc            ;    
    wire                                    A662Yc            ;    
    wire                                    A663Yc          ;
    wire                                    A664Yc          ;
    wire    [9:0]                           A665Yc                    ;
    reg                                     A666Yc         ;
    reg                                     A667Yc           ;    
wire        A668Yc                        = 1'b0;
wire        A669Yc                        = 1'b0;
wire        A670Yc                        = 1'b0;
wire        A671Yc                        = 1'b0;
    generate
    if(A411Yc               | A412Yc              ) begin: A672Yc    
osr_trng_A486Yc   # 
        (
            .A179Yc           (A179Yc             ),
            .A294Yc           (128                ),
            .A295Yc           (128                ),
            .A296Yc           (128                ), 
            .A297Yc           (256                ),
            .A180Yc           (32                 )
        )
        A673Yc    
        (
            .clk              (clk                ),
            .resetn           (resetn             ),
            .A299Yc           (A299Yc             ),
            .A487Yc           (A487Yc             ),
            .A488Yc           (A488Yc             ),
            .A489Yc           (A489Yc             ),
            .A490Yc           (A490Yc             ),
            .A491Yc           (A491Yc             ),
            .A492Yc           (A492Yc             ),
            .A493Yc           (A493Yc             ),
            .A494Yc           (A494Yc             ),
            .A495Yc           (A495Yc             ),
            .A496Yc           (A496Yc             ),
            .A497Yc           (A497Yc             ),
            .A498Yc             (A498Yc             ),
            .A499Yc           (A499Yc             ),
            .A500Yc           (A500Yc             ),
            .A501Yc           (A501Yc             ),
            .A502Yc           (A502Yc             ),
            .A503Yc           (A503Yc             ),
            .A504Yc           (A577Yc             ),
            .A505Yc           (A587Yc             ),
            .A506Yc           (A588Yc             ),
            .A507Yc           (A589Yc             ),
            .A508Yc           (A590Yc             ),
            .A509Yc           (A591Yc             ),
            .A510Yc           (A592Yc             ),
            .A511Yc            (A593Yc             ),
            .A512Yc           (A594Yc             ),
            .A513Yc           (A595Yc             ),
            .A514Yc           (A596Yc             ),
            .A515Yc           (A597Yc             ),
            .A516Yc           (A598Yc             ),
            .A517Yc           (A599Yc           [31:0]),
            .A518Yc           (A600Yc         [31:0]  ),
            .A519Yc           (A602Yc             ),
            .A520Yc           (A603Yc             ),
            .A521Yc           (A604Yc             ),
            .A522Yc           (A645Yc             ),
            .A523Yc           (A605Yc             ),
            .A524Yc           (A606Yc             ),
            .A525Yc           (A607Yc             ),
            .A526Yc           (A608Yc             ),
            .A527Yc           (A609Yc             ),
            .A528Yc           (A610Yc             ),
            .A529Yc           (A611Yc             ),
            .A530Yc           (A612Yc             ),
            .A531Yc           (A613Yc             ),
            .A532Yc           (A614Yc             ),
            .A533Yc           (A615Yc             ),
            .A534Yc           (A617Yc             ),
            .A535Yc           (A618Yc             ),
            .A536Yc           (A619Yc             ),
            .A537Yc           (A620Yc             ),
            .A538Yc           (A621Yc             ),
            .A539Yc           (A622Yc             ),
            .A540Yc           (A623Yc             ),
            .A541Yc             (A663Yc             ),
            .A542Yc             (A664Yc             ),
            .A543Yc             (A632Yc             ),
            .A544Yc           (A624Yc             ),
            .A545Yc           (A625Yc             ),
            .A546Yc           (A661Yc             ),
            .A547Yc           (A626Yc             ),
            .A548Yc           (A627Yc             ),
            .A549Yc           (A628Yc             ),
            .A550Yc           (A629Yc             ),
            .A551Yc           (A659Yc             ),
            .A552Yc           (A630Yc             ),
            .A553Yc           (A660Yc             ),
            .A554Yc           (A631Yc             )
        );
osr_trng_A410Yc       # 
        (
            .A411Yc                 (A411Yc              ),
            .A412Yc                 (A412Yc              ),
            .A294Yc                 (128                ),
            .A295Yc                 (128                ),
            .A296Yc                 (128                ),
            .A297Yc                 (256                ),
            .A180Yc                 (32                 ),
            .A371Yc                 (256                ),
            .A298Yc                 (8                  )
        )
        A674Yc        
        (
            .clk                    (clk                ),
            .resetn                 (resetn             ),
            .A299Yc                 (A587Yc             ),
            .A300Yc                 (A588Yc             ),
            .A301Yc                 (A589Yc             ),
            .A302Yc                 (A590Yc             ),
            .A413Yc                 (A576Yc             ),
            .A307Yc                 (A586Yc             ),
            .A414Yc                 (A601Yc             ),
            .A303Yc                 (A591Yc             ),
            .A304Yc                 (A592Yc             ),
            .A305Yc                 (A593Yc             ),
            .A306Yc                 (A594Yc             ),
            .A308Yc                 (A595Yc             ),
            .A310Yc                 (A596Yc             ),
            .A311Yc                 (A597Yc             ),
            .A312Yc                 (A598Yc             ),
            .A313Yc                 (A599Yc             ),
            .A314Yc                 (A600Yc             ),
            .A315Yc                 (A602Yc             ),
            .A317Yc                 (A579Yc             ),
            .A415Yc                 (A603Yc             ),
            .A416Yc                 (A604Yc             )
        );
    if(A411Yc               & A412Yc              ) begin: A675Yc       
	    wire                                    A676Yc                   ;
	    wire                                    A677Yc                   ;
   	    wire                                    A678Yc                   ;
	    wire                                    A679Yc                   ;
	    wire                                    A680Yc                   ;
	    wire                                    A681Yc                   ;
	    wire                                    A682Yc                   ;
	    wire  [A180Yc     -1:0]                 A683Yc                   ;
	    wire                                    A684Yc                   ;
	    wire  [A180Yc     -1:0]                 A685Yc                   ;
	    wire  [A297Yc   -1:0]                   A686Yc                   ;
	    wire  [A297Yc   -1:0]                   A687Yc                   ;
	    wire                                    A688Yc                   ;
	    wire                                    A689Yc                   ;
   	    wire                                    A690Yc                   ;
	    wire                                    A691Yc                   ;
	    wire                                    A692Yc                   ;
	    wire                                    A693Yc                   ;
	    wire                                    A694Yc                   ;
	    wire  [A180Yc     -1:0]                 A695Yc                   ;
	    wire                                    A696Yc                   ;
	    wire  [A180Yc     -1:0]                 A697Yc                   ;
	    wire  [A297Yc   -1:0]                   A698Yc                   ;
	    wire  [A297Yc   -1:0]                   A699Yc                   ;
        wire                                    A700Yc                   ;
    	wire                                    A701Yc                   ;
   	    wire                                    A702Yc                   ;
	    wire                                    A703Yc                   ;
	    wire  [31:0]                            A704Yc                   ;
	    wire  [31:0]                            A705Yc                   ;
	    wire  [A180Yc     -1:0]                 A706Yc                   ;
	    wire                                    A707Yc                   ;
	    wire  [A180Yc     -1:0]                 A708Yc                   ;
        wire                                    A709Yc                   ;
	    wire                                    A710Yc                   ;
	    wire                                    A711Yc                   ;
	    wire                                    A712Yc                   ;
	    wire  [31:0]                            A713Yc                   ;
	    wire  [31:0]                            A714Yc                   ;
	    wire  [A180Yc     -1:0]                 A715Yc                   ;
	    wire                                    A716Yc                   ;
	    wire  [A180Yc     -1:0]                 A717Yc                   ;
osr_trng_A718Yc            # (
	            .A294Yc           (128                ),
	            .A295Yc           (128                ),
	            .A296Yc           (128                ),
	            .A297Yc           (256                ),
	            .A180Yc           (32                 )
            )
            A719Yc             
            (
	            .i_clk            (clk                      ),
	            .i_rstn           (resetn                   ),
                .A139Yc           (A700Yc                   ),
                .A720Yc           (A586Yc                   ),
	            .A721Yc           (A701Yc                   ),
	            .A722Yc           (A676Yc                   ),
	            .A723Yc           (A677Yc                   ),
	            .A724Yc           (A702Yc                   ),
	            .A725Yc           (A679Yc                   ),
	            .A726Yc           (A680Yc                   ),
	            .A727Yc           (A681Yc                   ),
	            .A728Yc           (A682Yc                   ),
	            .A729Yc           (A683Yc                   ),
	            .A730Yc           (A703Yc                   ),
	            .A731Yc           (A704Yc                   ),
	            .A732Yc           (A705Yc                   ),
	            .A733Yc           (A706Yc                   ),
	            .A734Yc           (A707Yc                   ),
	            .A735Yc           (A708Yc                   ),
	            .A736Yc           (A684Yc                   ),
	            .A737Yc           (A685Yc                   ),
	            .A738Yc           (A686Yc                   ),
	            .A739Yc           (A687Yc                   )
            );
osr_trng_A740Yc            # (
	            .A294Yc           (128                ),
	            .A295Yc           (128                ),
	            .A297Yc           (256                ),
	            .A180Yc           (32                 )
            )
            A741Yc             
            (
	            .i_clk            (clk                      ),
	            .i_rstn           (resetn                   ),
                .A139Yc           (A709Yc                   ),
	            .A721Yc           (A710Yc                   ),
	            .A722Yc           (A688Yc                   ),
	            .A723Yc           (A689Yc                   ),
	            .A724Yc           (A711Yc                   ),
	            .A725Yc           (A691Yc                   ),
	            .A726Yc           (A692Yc                   ),
	            .A727Yc           (A693Yc                   ),
	            .A728Yc           (A694Yc                   ),
	            .A729Yc           (A695Yc                   ),
	            .A730Yc           (A712Yc                   ),
	            .A731Yc           (A713Yc                   ),
	            .A732Yc           (A714Yc                   ),
	            .A733Yc           (A715Yc                   ),
	            .A734Yc           (A716Yc                   ),
	            .A735Yc           (A717Yc                   ),
	            .A736Yc           (A696Yc                   ),
	            .A737Yc           (A697Yc                   ),
	            .A738Yc           (A698Yc                   ),
	            .A739Yc           (A699Yc                   )
            );
	    assign    A605Yc             = A576Yc          ? A688Yc                   : A676Yc                   ;
	    assign    A606Yc             = A576Yc          ? A689Yc                   : A677Yc                   ;
	    assign    A608Yc             = A576Yc          ? A691Yc                   : A679Yc                   ;
	    assign    A609Yc             = A576Yc          ? A692Yc                   : A680Yc                   ;
	    assign    A610Yc             = A576Yc          ? A693Yc                   : A681Yc                   ;
	    assign    A611Yc             = A576Yc          ? A694Yc                   : A682Yc                   ;
	    assign    A612Yc             = A576Yc          ? A695Yc                   : A683Yc                   ;
	    assign    A620Yc             = A576Yc          ? A696Yc                   : A684Yc                   ;
	    assign    A621Yc             = A576Yc          ? A697Yc                   : A685Yc                   ;
	    assign    A622Yc             = A576Yc          ? A698Yc                   : A686Yc                   ;
	    assign    A623Yc             = A576Yc          ? A699Yc                   : A687Yc                   ;
        assign       A700Yc                   = A576Yc          ? 'd0 : A587Yc             ;
    	assign       A701Yc                   = A576Yc          ? 'd0 : A645Yc             ;
   	    assign       A702Yc                   = A576Yc          ? 'd0 : A607Yc             ;
	    assign       A703Yc                   = A576Yc          ? 'd0 : A613Yc             ;
	    assign       A704Yc                   = A576Yc          ? 'd0 : A614Yc             ;
	    assign       A705Yc                   = A576Yc          ? 'd0 : A615Yc             ;
	    assign       A706Yc                   = A576Yc          ? 'd0 : A617Yc             ;
	    assign       A707Yc                   = A576Yc          ? 'd0 : A618Yc             ;
	    assign       A708Yc                   = A576Yc          ? 'd0 : A619Yc             ;
        assign       A709Yc                   = A576Yc          ? A587Yc             : 'd0 ;
	    assign       A710Yc                   = A576Yc          ? A645Yc             : 'd0 ;
	    assign       A711Yc                   = A576Yc          ? A607Yc             : 'd0 ;
	    assign       A712Yc                   = A576Yc          ? A613Yc             : 'd0 ;
	    assign       A713Yc                   = A576Yc          ? A614Yc             : 'd0 ;
	    assign       A714Yc                   = A576Yc          ? A615Yc             : 'd0 ;
	    assign       A715Yc                   = A576Yc          ? A617Yc             : 'd0 ;
	    assign       A716Yc                   = A576Yc          ? A618Yc             : 'd0 ;
	    assign       A717Yc                   = A576Yc          ? A619Yc             : 'd0 ;
    end
    else begin: A742Yc       
            if(A412Yc              ) begin: A743Yc 
osr_trng_A718Yc            # (
	            .A294Yc           (128                ),
	            .A295Yc           (128                ),
	            .A297Yc           (256                ),
	            .A180Yc           (32                 )
            )
            A744Yc         
            (
	            .i_clk            (clk                ),
	            .i_rstn           (resetn             ),
                .A139Yc           (A587Yc             ),
	            .A720Yc           (A586Yc             ),
	            .A721Yc           (A645Yc             ),
	            .A722Yc           (A605Yc             ),
	            .A723Yc           (A606Yc             ),
	            .A724Yc           (A607Yc             ),
	            .A725Yc           (A608Yc             ),
	            .A726Yc           (A609Yc             ),
	            .A727Yc           (A610Yc             ),
	            .A728Yc           (A611Yc             ),
	            .A729Yc           (A612Yc             ),
	            .A730Yc           (A613Yc             ),
	            .A731Yc           (A614Yc             ),
	            .A732Yc           (A615Yc             ),
	            .A733Yc           (A617Yc             ),
	            .A734Yc           (A618Yc             ),
	            .A735Yc           (A619Yc             ),
	            .A736Yc           (A620Yc             ),
	            .A737Yc           (A621Yc             ),
	            .A738Yc           (A622Yc             ),
	            .A739Yc           (A623Yc             )
            );
        end
        else if(A411Yc              ) begin: A745Yc 
osr_trng_A740Yc            # (
	            .A294Yc           (128                ),
	            .A295Yc           (128                ),
	            .A297Yc           (256                ),
	            .A180Yc           (32                 )
            )
            A744Yc         
            (
	            .i_clk            (clk                ),
	            .i_rstn           (resetn             ),
                .A139Yc           (A587Yc             ),
	            .A721Yc           (A645Yc             ),
	            .A722Yc           (A605Yc             ),
	            .A723Yc           (A606Yc             ),
	            .A724Yc           (A607Yc             ),
	            .A725Yc           (A608Yc             ),
	            .A726Yc           (A609Yc             ),
	            .A727Yc           (A610Yc             ),
	            .A728Yc           (A611Yc             ),
	            .A729Yc           (A612Yc             ),
	            .A730Yc           (A613Yc             ),
	            .A731Yc           (A614Yc             ),
	            .A732Yc           (A615Yc             ),
	            .A733Yc           (A617Yc             ),
	            .A734Yc           (A618Yc             ),
	            .A735Yc           (A619Yc             ),
	            .A736Yc           (A620Yc             ),
	            .A737Yc           (A621Yc             ),
	            .A738Yc           (A622Yc             ),
	            .A739Yc           (A623Yc             )
            );
        end
    end    
    wire        A746Yc            = A666Yc          && !A662Yc         ;
    reg [9:0]   A747Yc                     ;
    reg [9:0]   A748Yc                   ;
    assign A657Yc           = A667Yc           ;    
        always @(posedge clk or negedge resetn)begin
            if(!resetn)begin
                A666Yc              <= 1'b0;
                A667Yc              <= 1'b0;
            end
            else begin
                A667Yc              <= A746Yc           ;
                if(A626Yc  )
                    A666Yc          <=  1'b1;
                else if (A625Yc  )
                    A666Yc          <=  1'b0;
                else
                    A666Yc          <= A666Yc         ;
            end  
        end 
        always @(posedge clk or negedge resetn)begin
            if(!resetn)begin
                A747Yc                          <= 10'b0;
            end
            else if (A601Yc             )begin
                A747Yc                          <= A580Yc              ;
            end 
            else 
                A747Yc                          <= A747Yc                     ;
        end 
        always @(posedge clk or negedge resetn)begin
            if(!resetn)begin
                A748Yc                          <= 10'b0;
            end
            else if (A657Yc          )begin
                A748Yc                          <= A747Yc                     ;       
            end
            else
                A748Yc                          <= A748Yc                   ;       
        end         
        assign A665Yc                    = A748Yc                   ;
osr_trng_A178Yc    # (
        .A179Yc           (A179Yc             ),
        .A180Yc           (A180Yc             )
        )
        A749Yc     
        (
        .A187Yc           (A624Yc                             ),
        .A188Yc           (A625Yc                             ),
        .A189Yc           (A626Yc                             ),
        .A190Yc           (A627Yc                             ),
        .A191Yc           (A628Yc                             ),
        .A182Yc           (A629Yc                             ),
        .A183Yc           (A655Yc                             ),
        .A184Yc           (A630Yc                             ),
        .A185Yc           (A657Yc                             ),
        .A186Yc           (A631Yc                             ),
        .clk              (clk                                ),
        .resetn           (resetn                             )
        );
osr_trng_A178Yc    # (
        .A179Yc           (A179Yc             ),
        .A180Yc           (A180Yc             )
        )
        A750Yc       
        (
        .A187Yc           (A655Yc                             ),
        .A188Yc           (A661Yc                             ),
        .A189Yc           (A662Yc                             ),
        .A190Yc           (                                   ),
        .A191Yc           (                                   ),
        .A182Yc           (A629Yc                             ),
        .A183Yc           (A659Yc                             ),
        .A184Yc           (A746Yc                             ),
        .A185Yc           (A660Yc                             ),
        .A186Yc           (A631Yc                             ),
        .clk              (clk                                ),
        .resetn           (resetn                             )
        );
    end
    else if(A573Yc               ) begin: A751Yc 
osr_trng_A752Yc   #(
            .A008Yc                 (A179Yc             )
           ,.A007Yc                 (A180Yc             )
        )
        A753Yc    
        (
            .clk                    (clk                ) 
           ,.rst_n                  (resetn             )
           ,.A139Yc                 (A299Yc             )
           ,.A754Yc                 (A487Yc             )
           ,.A063Yc                 (A490Yc             )
           ,.A755Yc                 (A489Yc             )
           ,.A138Yc                 (A492Yc             )
           ,.A756Yc                 (A503Yc             )
           ,.A757Yc                 (A500Yc             )
           ,.A758Yc                 (A501Yc             )
           ,.A759Yc                 (A494Yc             )
           ,.A760Yc                 (A493Yc             )
           ,.A761Yc                 (A502Yc             )
           ,.A762Yc                 (A499Yc             )
           ,.A763Yc                 (A496Yc             )
           ,.A764Yc                 (A495Yc             )
           ,.A765Yc                 (A497Yc             )
           ,.A766Yc                 (A633Yc             )
           ,.A767Yc                 (A634Yc             )
           ,.A768Yc                 (A635Yc             )
           ,.A769Yc                 (A639Yc             )
           ,.A770Yc                 (A636Yc             )
           ,.A771Yc                 (A637Yc             )
           ,.A772Yc                 (A638Yc             )
           ,.A541Yc                 (A663Yc             )
           ,.A542Yc                 (A664Yc             )
           ,.A543Yc                 (A632Yc             )
           ,.A773Yc                 (A488Yc             )
           ,.A774Yc                 (A640Yc             )
           ,.A775Yc                 (A641Yc             )
           ,.A776Yc                 (A642Yc             )
           ,.A777Yc                 (A643Yc             )
           ,.A778Yc                 (A644Yc             )
           ,.A779Yc                 (A645Yc             )
           ,.A780Yc                 (A646Yc             )
           ,.A781Yc                 (A647Yc             )
           ,.A782Yc                 (A648Yc             )
           ,.A783Yc                 (A649Yc             )
           ,.A166Yc                 (A650Yc             )
           ,.A784Yc                 (A651Yc             )
           ,.A785Yc                 (A652Yc             )
           ,.A786Yc                 (A653Yc              )
           ,.A787Yc                 (A654Yc               )
           ,.A037Yc                 (A655Yc            )
           ,.A788Yc                 (A656Yc             )
           ,.A789Yc                 (A657Yc             )
           ,.A790Yc                 (A658Yc             )
           ,.A791Yc                 (A577Yc             )
        );
osr_trng_A792Yc  A793Yc   
        (
            .clk                    (clk                ) 
           ,.rst_n                  (resetn             )
           ,.A139Yc                 (A639Yc             )
           ,.A195Yc                 (A637Yc             )
           ,.A491Yc                 (A491Yc             )
           ,.A196Yc                 (A638Yc             )        
           ,.A197Yc                 (A633Yc             )
           ,.A198Yc                 (A634Yc             )
           ,.A199Yc                 (A635Yc             )
           ,.A790Yc                 (A636Yc             )
        );
osr_trng_A794Yc    A795Yc     
        (
            .clk                    (clk                )
           ,.rst_n                  (resetn             )
           ,.A139Yc                 (A299Yc             )
           ,.A721Yc                 (A645Yc             )
           ,.A722Yc                 (A640Yc             )
           ,.A723Yc                 (A641Yc             )
           ,.A796Yc                 (A642Yc             )
           ,.A797Yc                 (A646Yc             )
           ,.A798Yc                 (A647Yc             )
           ,.A799Yc                 (A648Yc             )
           ,.A800Yc                 (A643Yc             )
           ,.A801Yc                 (A644Yc             )
        );
osr_trng_A178Yc    #(
            .A179Yc                 (A179Yc             )
           ,.A180Yc                 (A180Yc             )
        )
        A749Yc     
        (
            .A187Yc                 (A649Yc             )
           ,.A188Yc                 (A650Yc             )
           ,.A189Yc                 (A651Yc             )
           ,.A190Yc                 (A652Yc             )
           ,.A191Yc                 (A628Yc             )
           ,.A182Yc                 (A653Yc               )
           ,.A183Yc                 (A655Yc             )
           ,.A184Yc                 (A656Yc             )
           ,.A185Yc                 (A657Yc             )
           ,.A186Yc                 (A658Yc             )
           ,.clk                    (clk                )
           ,.resetn                 (resetn             )
        );
        assign A665Yc                    = 10'b0;
    end
        if(A574Yc             ) begin: A802Yc
osr_trng_A803Yc      #(
                .A804Yc                 (A180Yc             )
            )
            A805Yc       
            (
                .clk                    (clk                )
               ,.rst_n                  (resetn             )
               ,.A206Yc                 (A015Yc             )
               ,.A806Yc                 (A575Yc             )
               ,.A211Yc                 (A655Yc             )
               ,.A212Yc                 (A657Yc             )
               ,.A214Yc                 (6'd41              )
               ,.A721Yc                 (A645Yc             )
               ,.A722Yc                 (A632Yc             )
               ,.A226Yc                 (A663Yc             )
               ,.A807Yc                 (A664Yc             )
               ,.A215Yc                 (A582Yc             )
               ,.A808Yc                 (A583Yc             )
            );
        end
        else begin: A809Yc   
            assign  A632Yc          = 1'b1;
            assign  A663Yc          = 1'b0;
            assign  A664Yc          = 1'b0;
            assign  A582Yc          = 1'b0;
            assign  A583Yc          = 1'b0;
        end
    endgenerate
    assign  A578Yc                      = !A591Yc        && A625Yc   && A661Yc         ;
    assign  A581Yc                      = A665Yc                   ;
    assign  A584Yc     = A663Yc    ;
    assign  A585Yc     = A664Yc    ;
endmodule
module osr_trng_A472Yc   #
    (
    parameter   A411Yc                  =   1               ,
    parameter   A412Yc                  =   0               ,
    parameter   A810Yc                  =   128             ,
    parameter   A811Yc                  =   128             ,
    parameter   A298Yc                  =   8               
    )
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A299Yc          ,
    input  wire                             A474Yc          ,
    input  wire                             A475Yc          ,
    input  wire [A810Yc        -1:0]        A336Yc          ,
    input  wire [A811Yc    -1:0]            A476Yc          ,
    input  wire                             A413Yc          ,
    output wire [A298Yc         -1:0]       A477Yc          ,
    output wire                             A478Yc          ,
    output wire                             A479Yc          
    );
    wire                                    A812Yc          ;
    generate
    if(A411Yc               & A412Yc              ) begin: A813Yc      
        wire [7:0]                              A814Yc              ;   
        wire                                    A815Yc              ;  
        wire                                    A816Yc              ;
        wire [7:0]                              A817Yc              ;
        wire                                    A818Yc              ;
        wire                                    A819Yc              ;
        wire                                    A820Yc          ;
        wire                                    A821Yc          ;
        wire [127:0]                            A822Yc          ;      
        wire [127:0]                            A823Yc          ; 
        wire                                    A824Yc          ;
        wire                                    A825Yc          ;
        wire [127:0]                            A826Yc          ;
        wire [127:0]                            A827Yc          ;
osr_trng_A828Yc   A829Yc    
        (
        .A830Yc         (A814Yc               ),
        .A831Yc         (A815Yc               ),
        .A832Yc         (A816Yc               ),
        .A833Yc         (A820Yc               ),
        .A834Yc         (A821Yc               ),
        .A835Yc         (A822Yc               ),
        .A836Yc         (A823Yc               ),
        .clk            (clk                  ),
        .resetn         (resetn               )
        );
osr_trng_A837Yc A838Yc
        (
        .A830Yc         (A817Yc               ),
        .A831Yc         (A818Yc               ),
        .A832Yc         (A819Yc               ),
        .A833Yc         (A824Yc               ),
        .A834Yc         (A825Yc               ),
        .A835Yc         (A826Yc               ),
        .A836Yc         (A827Yc               ),
        .clk            (clk                  ),
        .resetn         (resetn               )
        );
        assign  A820Yc         = A413Yc ? A299Yc     : 1'd0   ;
        assign  A821Yc         = A413Yc ? A812Yc     : 1'd0   ;
        assign  A822Yc         = A413Yc ? A336Yc     : 128'd0 ;
        assign  A823Yc         = A413Yc ? A476Yc     : 128'd0 ;
        assign  A824Yc         = A413Yc ? 1'd0   : A299Yc     ;
        assign  A825Yc         = A413Yc ? 1'd0   : A812Yc     ;
        assign  A826Yc         = A413Yc ? 128'd0 : A336Yc     ;
        assign  A827Yc         = A413Yc ? 128'd0 : A476Yc     ;
        assign  A477Yc          =   A413Yc ? A814Yc                 :   A817Yc             ; 
        assign  A478Yc          =   A413Yc ? A815Yc                 :   A818Yc             ;
        assign  A479Yc          =   A413Yc ? A816Yc                 :   A819Yc             ;
    end
    else begin: A839Yc      
        if(A411Yc              ) begin: A840Yc
osr_trng_A828Yc   A829Yc    
            (
            .A830Yc         (A477Yc           ),
            .A831Yc         (A478Yc           ),
            .A832Yc         (A479Yc           ),
            .A833Yc         (A299Yc           ),
            .A834Yc         (A812Yc           ),
            .A835Yc         (A336Yc           ),
            .A836Yc         (A476Yc           ),
            .clk            (clk              ),
            .resetn         (resetn           )
            );
        end
        else if(A412Yc              ) begin: A841Yc
osr_trng_A837Yc A838Yc
            (
            .A830Yc         (A477Yc           ),
            .A831Yc         (A478Yc           ),
            .A832Yc         (A479Yc           ),
            .A833Yc         (A299Yc           ),
            .A834Yc         (A812Yc           ),
            .A835Yc         (A336Yc           ),
            .A836Yc         (A476Yc           ),
            .clk            (clk              ),
            .resetn         (resetn           )
            );
        end    
    end
    assign  A812Yc  =   A474Yc        | A475Yc              ;
endgenerate
endmodule
module osr_trng_A464Yc
    (
    input  wire                             clk             ,
    input  wire                             resetn          ,
    input  wire                             A466Yc          ,
    input  wire                             A467Yc          ,
    output wire                             A468Yc          ,
    output wire                             A469Yc          ,
    input  wire [31:0]                      A470Yc          ,
    output wire [31:0]                      A471Yc          
    );
    reg         [2:0]                       A842Yc          ;
    reg                                     A843Yc          ;
    reg         [31:0]                      A844Yc          ;
    wire        [31:0]                      A845Yc          ;
    wire                                    A846Yc          ;
    wire                                    A847Yc          ;
    wire                                    A848Yc          ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A842Yc                      <=  3'b000          ;
        else if (A848Yc   )
            A842Yc                      <= 3'b100        ;
        else if (A847Yc    )
            A842Yc                      <= A842Yc  + 1'b1;
        else
            A842Yc                      <= 3'b000        ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A843Yc                      <=  0               ;
        else if (A847Yc    )
            A843Yc                      <= A846Yc        ;
        else
            A843Yc                      <= 1             ;
    always @ (*)
        begin
        A844Yc                          =   0               ;
        case (A842Yc )
            3'b100:  A844Yc             =   A470Yc     [31: 0];
            3'b101:  A844Yc             =   A470Yc     [31: 0];
            3'b110:  A844Yc             =   A470Yc     [31: 0];
            3'b111:  A844Yc             =   A470Yc     [31: 0];
            default:    ;
        endcase
        end
    assign  A848Yc      =   A466Yc           | A467Yc          ;
    assign  A847Yc      =   A842Yc [2]          ;
    assign  {A846Yc,A845Yc}      =   {1'b0,A844Yc} + {32'd0,A843Yc};
    assign  A471Yc      =   A845Yc              ;
    assign  A468Yc          =   A847Yc          ;
    assign  A469Yc      =   A842Yc  == 3'b111   ;
endmodule
module osr_trng_A849Yc       (clk, resetn, A835Yc, A830Yc, A850Yc, A851Yc, A852Yc, A853Yc        , A854Yc     );
    input clk;
    input resetn;
    input [7:0] A835Yc;
    output [7:0] A830Yc;
    input A850Yc;
    input [1:0] A851Yc;
    input [7:0] A852Yc;
    input [7:0] A853Yc        , A854Yc     ;
    wire [7:0] A855Yc, A856Yc, A857Yc, A858Yc , A859Yc , A860Yc , A861Yc , A862Yc;
    wire [31:0] A863Yc;
    assign A863Yc = {A858Yc , A859Yc , A860Yc , A861Yc };
    assign A855Yc = A853Yc         ^ A857Yc;
    assign A830Yc = A862Yc ^ A854Yc     ;
osr_trng_A864Yc           A865Yc (A855Yc, A856Yc, A851Yc, clk, resetn);
osr_trng_A866Yc  A867Yc (A835Yc, A857Yc, A863Yc, A850Yc, clk, resetn);
osr_trng_A868Yc      A869Yc (A862Yc, A852Yc, A858Yc , A859Yc , A860Yc , A861Yc , clk, resetn);
osr_trng_A870Yc A871Yc (A856Yc, A862Yc);
endmodule
module osr_trng_A828Yc   (
	output		[7:0]						A830Yc				,
	output									A831Yc				,
    output                                  A832Yc              ,
    input                                   A833Yc              ,
	input									A834Yc				,
	input		[127:0]						A835Yc				,
	input		[127:0]						A836Yc				,
	input									clk					,
	input									resetn	
	);
	parameter	A872Yc						= 3'b000;	
	parameter	A873Yc						= 3'b001;			
	parameter	A874Yc						= 3'b010;       	
	parameter	A875Yc						= 3'b011;       	
	parameter	A876Yc						= 3'b100;       	
	parameter	A877Yc						= 3'b101;       	
	parameter	A878Yc						= 3'b110;       	
	reg			[2:0]						A283Yc       , A284Yc    ;
	wire									A879Yc     			;
	wire									A880Yc          	;
	wire									A881Yc          	;
	wire									A882Yc          	;
	wire									A883Yc   	    	;
	wire									A884Yc  	    	;
	wire									A885Yc          	;
	wire									A886Yc     	    	;
	wire		[7:0]						A887Yc 		    	;
	wire		[7:0]						A853Yc        , A854Yc     ;
    reg			[1:0]						A851Yc					;
    wire 									A850Yc					;
    reg			[7:0]						A888Yc   			;
    reg 									A889Yc 				;
    wire		[7:0]						A852Yc				;
    reg			[7:0]						A890Yc 				;
    wire 		[7:0] 						A891Yc 				;
	reg 		[7:0] 						A892Yc   			;
	reg 		[3:0] 						A893Yc					;
	wire									A894Yc      		;
	reg										A895Yc 				;
	wire		[3:0]						A896Yc     			;
	reg			[7:0]						A897Yc  			;
	reg			[7:0]						A898Yc				;
osr_trng_A899Yc        A900Yc (
	.clk									(	clk				), 
	.resetn									(	resetn			),
	.A836Yc									(	A897Yc  		),
	.A853Yc        							(	A853Yc        	),
	.A892Yc   								(	A896Yc     		),
	.A854Yc     							(	A854Yc     		),
	.A883Yc   								(	A883Yc   		),
	.A884Yc  								(	A884Yc  		),
	.A885Yc      							(	A885Yc      	),
	.A886Yc     							(	A886Yc     		),
	.A887Yc 								(	A887Yc 			)
	);
osr_trng_A849Yc        A901Yc    (
	.clk									(	clk				),
	.resetn									(	resetn			),
	.A835Yc									(	A898Yc			),
	.A830Yc									(	A891Yc 			),
	.A850Yc									(	A850Yc				),
	.A851Yc										(	A851Yc				),
	.A852Yc									(	A852Yc			),
	.A853Yc        							(	A853Yc        	),
	.A854Yc     							(	A854Yc     		)
	);
	always @ (*)
        if (A283Yc        == A873Yc)
			case (A893Yc[3:0])
				4'h0: 						A897Yc  				= A836Yc[127:120];
				4'h1: 						A897Yc  				= A836Yc[119:112];
				4'h2: 						A897Yc  				= A836Yc[111:104];
				4'h3: 						A897Yc  				= A836Yc[103: 96];
				4'h4: 						A897Yc  				= A836Yc[ 95: 88];
				4'h5: 						A897Yc  				= A836Yc[ 87: 80];
				4'h6: 						A897Yc  				= A836Yc[ 79: 72];
				4'h7: 						A897Yc  				= A836Yc[ 71: 64];
				4'h8: 						A897Yc  				= A836Yc[ 63: 56];
				4'h9: 						A897Yc  				= A836Yc[ 55: 48];
				4'hA: 						A897Yc  				= A836Yc[ 47: 40];
				4'hB: 						A897Yc  				= A836Yc[ 39: 32];
				4'hC: 						A897Yc  				= A836Yc[ 31: 24];
				4'hD: 						A897Yc  				= A836Yc[ 23: 16];
				4'hE: 						A897Yc  				= A836Yc[ 15:  8];
				4'hF: 						A897Yc  				= A836Yc[  7:  0];
			endcase
		else 								A897Yc  				= 0;
	always @ (*)
        if (A283Yc        == A873Yc)
			case (A893Yc[3:0])
				4'h0: 						A898Yc					= A835Yc[127:120];
				4'h1: 						A898Yc					= A835Yc[119:112];
				4'h2: 						A898Yc					= A835Yc[111:104];
				4'h3: 						A898Yc					= A835Yc[103: 96];
				4'h4: 						A898Yc					= A835Yc[ 95: 88];
				4'h5: 						A898Yc					= A835Yc[ 87: 80];
				4'h6: 						A898Yc					= A835Yc[ 79: 72];
				4'h7: 						A898Yc					= A835Yc[ 71: 64];
				4'h8: 						A898Yc					= A835Yc[ 63: 56];
				4'h9: 						A898Yc					= A835Yc[ 55: 48];
				4'hA: 						A898Yc					= A835Yc[ 47: 40];
				4'hB: 						A898Yc					= A835Yc[ 39: 32];
				4'hC: 						A898Yc					= A835Yc[ 31: 24];
				4'hD: 						A898Yc					= A835Yc[ 23: 16];
				4'hE: 						A898Yc					= A835Yc[ 15:  8];
				4'hF: 						A898Yc					= A835Yc[  7:  0];
			endcase
		else								A898Yc					= 0;
	always @ (posedge clk or negedge resetn)
		if (!resetn)						A283Yc       			<= A872Yc;
        else if (~A833Yc)                   A283Yc                  <= A872Yc;
		else								A283Yc       			<= A284Yc    ;
	always @ (*)
		case (A283Yc       )
			A872Yc:
				if (A834Yc)					A284Yc    				= A873Yc;
				else						A284Yc    				= A872Yc;
			A873Yc:
				if (A879Yc     )			A284Yc    				= A874Yc;
				else						A284Yc    				= A873Yc;
			A874Yc:				
				if (A894Yc      )			A284Yc    				= A872Yc;
				else						A284Yc    				= A875Yc;
			A875Yc:
				if (A880Yc     )			A284Yc    				= A876Yc;
				else						A284Yc    				= A875Yc;
			A876Yc:							A284Yc    				= A877Yc;
			A877Yc:
				if (A881Yc     )			A284Yc    				= A878Yc;
				else						A284Yc    				= A877Yc;
			A878Yc:
				if (A882Yc     )			A284Yc    				= A874Yc;
				else						A284Yc    				= A878Yc;
			default:						A284Yc    				= A872Yc;
		endcase
	always @ (posedge clk or negedge resetn)
		if (!resetn)						A893Yc						<= 4'h0;
		else case (A283Yc       )
			A872Yc:							A893Yc						<= 4'h0;
			A873Yc:
				if (A879Yc     )			A893Yc						<= 4'h0;
				else						A893Yc						<= A893Yc + 1'b1;
			A874Yc:							A893Yc						<= 4'h0;
			A875Yc:
				if (A880Yc     )			A893Yc						<= 4'h0;
				else						A893Yc						<= A893Yc + 1'b1;
			A876Yc:							A893Yc						<= 4'h0;
			A877Yc:
				if (A881Yc     )			A893Yc						<= 4'h0;
				else						A893Yc						<= A893Yc + 1'b1;
			A878Yc:
				if (A882Yc     )			A893Yc						<= 4'h0;
				else						A893Yc						<= A893Yc + 1'b1;
			endcase
	assign		A879Yc     	= (A893Yc == 4'hF);
	assign		A880Yc      = (A893Yc == 4'h1);
	assign		A881Yc      = (A893Yc == 4'h7);
	assign		A882Yc      = (A893Yc == 4'h3);
	assign		A883Yc   	= ((A283Yc        == A873Yc) || (A283Yc        == A872Yc)) ? 1'b0 : 1'b1;	
	assign		A884Yc  	= A283Yc       [2] 		  ? 1'b0 : 1'b1;
	assign		A885Yc      = ((A283Yc        == A877Yc) || (A283Yc        == A878Yc)) ? 1'b1 : 1'b0;
	assign		A886Yc     	= ((A283Yc        == A873Yc) || (A283Yc        == A878Yc) || (A283Yc        == A872Yc)) ? 1'b0 : 1'b1;
	assign		A887Yc 		= (A283Yc        == A874Yc) ? 8'hFF : 8'h00;
    assign      A832Yc      = (A283Yc        == A874Yc) && A894Yc      ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)						A892Yc   				<= 8'h00;
        else if (A893Yc == 4'hF)				A892Yc   				<= 8'h00;
        else if (A896Yc      == 4'hA)		A892Yc   				<= 8'h00;
		else if (A283Yc        == A872Yc)		A892Yc   				<= 8'h00;
        else								A892Yc   				<= A892Yc    + 6'h01;
    always @ (posedge clk or negedge resetn)
        if (!resetn)						A851Yc 						<= 2'h0;
        else if (A283Yc        == A873Yc) 	A851Yc 						<= 2'h3;
        else begin
			case (A892Yc   [3:0])
				4'h0: 						A851Yc 						<= 2'h2;
				4'h1: 						A851Yc 						<= 2'h1;
				4'h2: 						A851Yc 						<= 2'h0;
				4'h3: 						A851Yc 						<= 2'h3;
				4'h4: 						A851Yc 						<= 2'h2;
				4'h5: 						A851Yc 						<= 2'h1;
				4'h6: 						A851Yc 						<= 2'h1;
				4'h7: 						A851Yc 						<= 2'h3;
				4'h8: 						A851Yc 						<= 2'h2;
				4'h9: 						A851Yc 						<= 2'h3;
				4'hA: 						A851Yc 						<= 2'h2;
				4'hB: 						A851Yc 						<= 2'h3;
				4'hC: 						A851Yc 						<= 2'h3;
				4'hD: 						A851Yc 						<= 2'h3;
				4'hE: 						A851Yc 						<= 2'h3;
				4'hF: 						A851Yc 						<= 2'h3;
			endcase
        end
    always @ (posedge clk or negedge resetn)
        if (!resetn)						A888Yc   				<= 8'h00;
        else if (A892Yc   [1:0] == 2'b11)	A888Yc    				<= 8'h00;
        else								A888Yc   				<= 8'hFF;
    always @ (posedge clk or negedge resetn)
        if (!resetn)						A889Yc  				<= 1'b0;
        else if (A283Yc        == A873Yc)		A889Yc  				<= 1'b0;
        else if (A892Yc   [1:0] == 2'b11)	A889Yc  				<= 1'b1;
        else								A889Yc 					<= 1'b0;
    always @(posedge clk or negedge resetn)
        if (!resetn)						A895Yc 					<= 1'b0;
        else if (A892Yc    == 8'h90)		A895Yc 					<= 1'b1;
		else if (A894Yc      )				A895Yc 					<= 1'b0;
		else;
	always @ (posedge clk or negedge resetn)
        if (!resetn)						A890Yc  				<= 8'h00;
        else								A890Yc  				<= A891Yc ;
	assign		A894Yc       = (A892Yc    == 8'ha0);
    assign 		A850Yc 	= A889Yc ;
    assign 		A852Yc 	= A888Yc   ;
    assign 		A896Yc      = A892Yc   [7:4];
	assign		A830Yc	= A890Yc ;
	assign		A831Yc	= A895Yc ;
endmodule	
module osr_trng_A864Yc           (A902Yc, A903Yc, A851Yc, clk, resetn);
    input [7:0] A902Yc;
    output [7:0] A903Yc;
    input [1:0] A851Yc;
    input clk;
    input resetn;
    wire A904Yc, A905Yc, A906Yc;
    wire [7:0] A907Yc, A908Yc, A909Yc;
    reg [7:0] A910Yc, A911Yc, A912Yc, A913Yc, A914Yc, A915Yc, A916Yc, A917Yc, A918Yc, A919Yc, A920Yc, A921Yc;
    assign A904Yc = ~ (A851Yc[1] | A851Yc[0]);
    assign A905Yc = ~ (A851Yc[1] | (~ A851Yc[0])); 
    assign A906Yc = ~ ((~ A851Yc[1]) | A851Yc[0]);
osr_trng_A922Yc A923Yc (A921Yc, A902Yc, A907Yc, A904Yc);
osr_trng_A922Yc A924Yc (A921Yc, A913Yc, A908Yc, A905Yc);
osr_trng_A922Yc A925Yc (A921Yc, A917Yc, A909Yc, A906Yc);
osr_trng_A926Yc A927Yc (A902Yc, A913Yc, A917Yc, A921Yc, A903Yc, A851Yc);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A910Yc <= 8'h00;
        else
            A910Yc <= A907Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A911Yc <= 8'h00;
        else
            A911Yc <= A910Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A912Yc <= 8'h00;
        else
            A912Yc <= A911Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A913Yc <= 8'h00;
        else
            A913Yc  <= A912Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A914Yc <= 8'h00;
        else
            A914Yc  <= A908Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A915Yc <= 8'h00;
        else
            A915Yc  <= A914Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A916Yc <= 8'h00;
        else
            A916Yc  <= A915Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A917Yc <= 8'h00;
        else
            A917Yc  <= A916Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A918Yc <= 8'h00;
        else
            A918Yc  <= A909Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A919Yc <= 8'h00;
        else
            A919Yc  <= A918Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A920Yc <= 8'h00;
        else
            A920Yc  <= A919Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A921Yc <= 8'h00;
        else
            A921Yc  <= A920Yc;
endmodule
module osr_trng_A899Yc       (clk, resetn, A836Yc, A853Yc        , A892Yc   , A854Yc     , A883Yc   , A884Yc  , A885Yc      , A886Yc     , A887Yc );
    input clk;
    input resetn;
    input [7:0] A836Yc;
    output [7:0] A853Yc        ;
    output [7:0] A854Yc     ;
    input [3:0] A892Yc   ;
    input A883Yc   , A884Yc  , A885Yc      , A886Yc     ; 
    input [7:0] A887Yc ;
    reg [7:0] A928Yc, A929Yc, A930Yc, A931Yc, A932Yc, A933Yc, A934Yc, A935Yc, A936Yc, A937Yc, A938Yc, A939Yc, A940Yc, A941Yc, A942Yc, A943Yc, A944Yc ;
    wire [7:0] A945Yc     , A862Yc, A946Yc, A947Yc , A948Yc  , A949Yc   , A950Yc  ;
    function [7:0] A951Yc;
      input [3:0] A952Yc;
        casex (A952Yc)
          4'b0000: A951Yc = 8'h01;
          4'b0001: A951Yc = 8'h02;
          4'b0010: A951Yc = 8'h04;
          4'b0011: A951Yc = 8'h08;
          4'b0100: A951Yc = 8'h10;
          4'b0101: A951Yc = 8'h20;
          4'b0110: A951Yc = 8'h40;
          4'b0111: A951Yc = 8'h80;
          4'b1000: A951Yc = 8'h1b;
          4'b1001: A951Yc = 8'h36;
          default: A951Yc = 8'h01;
        endcase
    endfunction
    assign A950Yc   = A951Yc(A892Yc   );
    assign A945Yc      = A862Yc ^ A946Yc;
    assign A946Yc = A887Yc  & A950Yc  ;
    assign A853Yc         = A931Yc;
osr_trng_A922Yc A953Yc (A854Yc     , A836Yc, A948Yc  , A883Yc   );
osr_trng_A922Yc A954Yc   (A930Yc, A944Yc , A947Yc , A884Yc  ); 
osr_trng_A922Yc A955Yc  ((A939Yc ^ A854Yc     ), A939Yc, A949Yc   , A886Yc     ); 
osr_trng_A922Yc A956Yc       (A943Yc, ( A943Yc ^ A945Yc     ),  A854Yc     , A885Yc      );
osr_trng_A870Yc A957Yc (A947Yc , A862Yc);
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A928Yc <= 8'h00;
        else
            A928Yc <= A948Yc  ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A929Yc <= 8'h00;
        else
            A929Yc <= A928Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A930Yc <= 8'h00;
        else
            A930Yc <= A929Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A931Yc <= 8'h00;
        else
            A931Yc <= A930Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A932Yc <= 8'h00;
        else
            A932Yc <= A931Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A933Yc <= 8'h00;
        else
            A933Yc <= A932Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A934Yc <= 8'h00;
        else
            A934Yc <= A933Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A935Yc <= 8'h00;
        else
            A935Yc <= A934Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A936Yc <= 8'h00;
        else
            A936Yc <= A935Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A937Yc <= 8'h00;
        else
            A937Yc <= A936Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A938Yc <= 8'h00;
        else
            A938Yc <= A937Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A939Yc <= 8'h00;
        else
            A939Yc <= A938Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A940Yc <= 8'h00;
        else
            A940Yc <= A949Yc   ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A941Yc <= 8'h00;
        else
            A941Yc <= A940Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A942Yc <= 8'h00;
        else
            A942Yc <= A941Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)    
            A943Yc <= 8'h00;
        else
            A943Yc <= A942Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A944Yc  <= 8'h00;
        else if (A887Yc  == 8'hff)
            A944Yc  <= A931Yc;
        else
            A944Yc  <= A944Yc ;
endmodule
module osr_trng_A868Yc      (A902Yc, A958Yc, A959Yc, A960Yc, A961Yc, A962Yc, clk, resetn);
    input [7:0] A902Yc;
    input [7:0] A958Yc;
    output [7:0] A959Yc, A960Yc, A961Yc, A962Yc;
    input clk;
    input resetn;
    reg [7:0] A963Yc, A921Yc, A920Yc, A919Yc;
    wire [7:0] A964Yc, A965Yc;
    assign A964Yc = {A902Yc[6:4], A902Yc[3] ^ A902Yc[7], A902Yc[2] ^ A902Yc[7] , A902Yc[1], A902Yc[0] ^ A902Yc[7], A902Yc[7]};
    assign A965Yc = {A902Yc[7] ^ A902Yc[6], A902Yc[6] ^ A902Yc[5], A902Yc[5] ^ A902Yc[4], A902Yc[4] ^ A902Yc[3] ^ A902Yc[7], A902Yc[3] ^ A902Yc[2] ^ A902Yc[7] , A902Yc[2] ^ A902Yc[1], A902Yc[1] ^ A902Yc[0] ^ A902Yc[7], A902Yc[0] ^ A902Yc[7]};
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A963Yc <= 8'h00;
        else
            A963Yc <= A902Yc ^ (A921Yc & A958Yc);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A921Yc <= 8'h00;
        else
            A921Yc <= A902Yc ^ (A920Yc & A958Yc);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A920Yc <= 8'h00;
        else
            A920Yc <= A965Yc ^ (A919Yc & A958Yc);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A919Yc <= 8'h00;
        else
            A919Yc <= A964Yc ^ (A963Yc & A958Yc);
    assign A959Yc = A963Yc;
    assign A960Yc = A921Yc;
    assign A961Yc = A920Yc;
    assign A962Yc = A919Yc;
endmodule
module osr_trng_A922Yc(A966Yc, A967Yc, A968Yc, A969Yc);
    input [7:0] A966Yc, A967Yc;
    output [7:0] A968Yc;
    input A969Yc;
    assign A968Yc = A969Yc ? A966Yc : A967Yc;
endmodule
module osr_trng_A926Yc(A966Yc, A967Yc, A970Yc, A971Yc, A968Yc, A969Yc);
    input [7:0] A966Yc, A967Yc, A970Yc, A971Yc;
    output [7:0] A968Yc;
    input [1:0] A969Yc;
    reg [7:0] A972Yc ;
    assign A968Yc = A972Yc ;
    always@(*)
    begin
        case(A969Yc)
            2'b00: A972Yc  = A966Yc;
            2'b01: A972Yc  = A967Yc;
            2'b10: A972Yc  = A970Yc;
            default: A972Yc  = A971Yc;
        endcase
    end
endmodule
module osr_trng_A866Yc (A902Yc, A903Yc, A863Yc, A850Yc, clk, resetn);
    input [7:0] A902Yc;
    output [7:0] A903Yc; 
    input [31:0] A863Yc; 
    input A850Yc; 
    input clk;
    input resetn;
    reg [7:0] A919Yc, A920Yc, A921Yc, A963Yc;
    wire [7:0] A908Yc, A909Yc, A973Yc;
osr_trng_A922Yc A923Yc (A863Yc[31:24], A963Yc, A903Yc, A850Yc);
osr_trng_A922Yc A924Yc (A863Yc[23:16], A921Yc, A908Yc, A850Yc);
osr_trng_A922Yc A925Yc (A863Yc[15: 8], A920Yc, A909Yc, A850Yc);
osr_trng_A922Yc A927Yc (A863Yc[ 7: 0], A919Yc, A973Yc, A850Yc);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A919Yc <= 8'h00;
        else
            A919Yc <= A902Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A920Yc <= 8'h00;
        else
            A920Yc <= A973Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A921Yc <= 8'h00;
        else
            A921Yc <= A909Yc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A963Yc <= 8'h00;
        else
            A963Yc <= A908Yc;
endmodule
module osr_trng_A974Yc  ( A, A975Yc );
    input [1:0] A;
    output [1:0] A975Yc;
    assign A975Yc = { A[0], A[1] };
endmodule
module osr_trng_A976Yc    ( A, A977Yc, B, A978Yc, A979Yc );
    input [1:0] A;
    input A977Yc;
    input [1:0] B;
    input A978Yc;
    output [1:0] A979Yc;
    wire A980Yc, A981Yc, A982Yc;
    assign A980Yc = ~(A977Yc & A978Yc); 
    assign A981Yc = (~(A[1] & B[1])) ^ A980Yc;
    assign A982Yc = (~(A[0] & B[0])) ^ A980Yc;
    assign A979Yc = { A981Yc, A982Yc };
endmodule
module osr_trng_A983Yc        ( A, A977Yc, B, A978Yc, A979Yc );
    input [1:0] A;
    input A977Yc;
    input [1:0] B;
    input A978Yc;
    output [1:0] A979Yc;
    wire A984Yc, A981Yc, A982Yc;
    assign A984Yc = ~(A[0] & B[0]); 
    assign A981Yc = (~(A977Yc & A978Yc)) ^ A984Yc;
    assign A982Yc = (~(A[1] & B[1])) ^ A984Yc;
    assign A979Yc = { A981Yc, A982Yc };
endmodule
module osr_trng_A985Yc   ( A986Yc, A979Yc );
input [3:0] A986Yc;
output [3:0] A979Yc;
    wire [1:0] A987Yc, A988Yc, A989Yc, A990Yc, A981Yc, A982Yc;
    wire A991Yc, A992Yc, A993Yc; 
    assign A987Yc = A986Yc[3:2];
    assign A988Yc = A986Yc[1:0];
    assign A991Yc = A987Yc[1] ^ A987Yc[0];
    assign A992Yc = A988Yc[1] ^ A988Yc[0];
    assign A989Yc = { 
    ~(A987Yc[1] | A988Yc[1]) ^ (~(A991Yc & A992Yc)) ,
    ~(A991Yc | A992Yc) ^ (~(A987Yc[0] & A988Yc[0])) };
osr_trng_A974Yc  A994Yc( A989Yc, A990Yc);
    assign A993Yc = A990Yc[1] ^ A990Yc[0];
osr_trng_A976Yc    A995Yc(A990Yc, A993Yc, A988Yc, A992Yc, A981Yc);
osr_trng_A976Yc    A996Yc(A990Yc, A993Yc, A987Yc, A991Yc, A982Yc);
    assign A979Yc = { A981Yc, A982Yc };
endmodule
module osr_trng_A997Yc    ( A, A998Yc, A999Yc, Y000Zc, Y001Zc, B, Y002Zc, Y003Zc, Y004Zc, Y005Zc, A975Yc );
    input [3:0] A;
    input [1:0] A998Yc;
    input A999Yc;
    input Y000Zc;
    input Y001Zc;
    input [3:0] B;
    input [1:0] Y002Zc;
    input Y003Zc;
    input Y004Zc;
    input Y005Zc;
    output [3:0] A975Yc;
    wire [1:0] Y006Zc, Y007Zc, A981Yc;
osr_trng_A976Yc    Y008Zc(A[3:2], Y000Zc, B[3:2], Y004Zc, Y006Zc);
osr_trng_A976Yc    Y009Zc(A[1:0], A999Yc, B[1:0], Y003Zc, Y007Zc);
osr_trng_A983Yc        Y010Zc( A998Yc, Y001Zc, Y002Zc, Y005Zc, A981Yc);
    assign A975Yc = { (Y006Zc ^ A981Yc), (Y007Zc ^ A981Yc) };
endmodule
module osr_trng_Y011Zc   ( A986Yc, A979Yc );
    input [7:0] A986Yc;
    output [7:0] A979Yc;
    wire [3:0] A987Yc, A988Yc, A989Yc, A990Yc, A981Yc, A982Yc;
    wire [1:0] A991Yc, A992Yc, A993Yc; 
    wire Y012Zc, Y013Zc, Y001Zc, Y014Zc, Y015Zc, Y005Zc, Y016Zc, Y017Zc, Y018Zc; 
    wire A905Yc, A906Yc, A851Yc; 
    assign A987Yc = A986Yc[7:4];
    assign A988Yc = A986Yc[3:0];
    assign A991Yc = A987Yc[3:2] ^ A987Yc[1:0];
    assign A992Yc = A988Yc[3:2] ^ A988Yc[1:0];
    assign Y012Zc = A987Yc[1] ^ A987Yc[0];
    assign Y013Zc = A987Yc[3] ^ A987Yc[2];
    assign Y001Zc = A991Yc[1] ^ A991Yc[0];
    assign Y014Zc = A988Yc[1] ^ A988Yc[0];
    assign Y015Zc = A988Yc[3] ^ A988Yc[2];
    assign Y005Zc = A992Yc[1] ^ A992Yc[0];
    assign A905Yc = ~(Y013Zc & Y015Zc);
    assign A906Yc = ~(A991Yc[0] & A992Yc[0]);
    assign A851Yc = ~(Y001Zc & Y005Zc);
    assign A989Yc = { 
    (~(A991Yc[0] | A992Yc[0]) ^ (~(A987Yc[3] & A988Yc[3]))) ^ A905Yc ^ A851Yc ,
    (~(A991Yc[1] | A992Yc[1]) ^ (~(A987Yc[2] & A988Yc[2]))) ^ A905Yc ^ A906Yc ,
    (~(Y012Zc | Y014Zc) ^ (~(A987Yc[1] & A988Yc[1]))) ^ A906Yc ^ A851Yc ,
    (~(A987Yc[0] | A988Yc[0]) ^ (~(Y012Zc & Y014Zc))) ^ (~(A991Yc[1] & A992Yc[1])) ^ A906Yc };
osr_trng_A985Yc   A994Yc( A989Yc, A990Yc);
    assign A993Yc = A990Yc[3:2] ^ A990Yc[1:0];
    assign Y016Zc = A990Yc[1] ^ A990Yc[0];
    assign Y017Zc = A990Yc[3] ^ A990Yc[2];
    assign Y018Zc = A993Yc[1] ^ A993Yc[0];
osr_trng_A997Yc    A995Yc(A990Yc, A993Yc, Y016Zc, Y017Zc, Y018Zc, A988Yc, A992Yc, Y014Zc, Y015Zc, Y005Zc, A981Yc);
osr_trng_A997Yc    A996Yc(A990Yc, A993Yc, Y016Zc, Y017Zc, Y018Zc, A987Yc, A991Yc, Y012Zc, Y013Zc, Y001Zc, A982Yc);
    assign A979Yc = { A981Yc, A982Yc };
endmodule
module osr_trng_A870Yc ( A, A975Yc );
    input [7:0] A;
    output [7:0] A975Yc;
    wire [7:0] B, Y019Zc;
    wire Y020Zc, Y021Zc, Y022Zc, Y023Zc, Y024Zc, Y025Zc, Y026Zc, Y027Zc, Y028Zc;
    wire Y029Zc, Y030Zc, Y031Zc, Y032Zc, Y033Zc, Y034Zc, Y035Zc, Y036Zc, Y037Zc;
    assign Y020Zc = A[7] ^ A[5] ;
    assign Y021Zc = A[7] ^ A[4] ;
    assign Y022Zc = A[6] ^ A[0] ;
    assign Y023Zc = A[5] ^ Y022Zc ;
    assign Y024Zc = A[4] ^ Y023Zc ;
    assign Y025Zc = A[3] ^ A[0] ;
    assign Y026Zc = A[2] ^ Y020Zc ;
    assign Y027Zc = A[1] ^ Y022Zc ;
    assign Y028Zc = A[3] ^ Y027Zc ;
    assign B[7] = Y026Zc ^ Y027Zc ;
    assign B[6] = Y024Zc ;
    assign B[5] = A[1] ^ Y023Zc ;
    assign B[4] = Y020Zc ^ Y022Zc ;
    assign B[3] = A[1] ^ Y021Zc ^ Y025Zc ;
    assign B[2] = A[0] ;
    assign B[1] = Y023Zc ;
    assign B[0] = A[2] ^ Y028Zc ;
osr_trng_Y011Zc   Y038Zc( B, Y019Zc );
    assign Y029Zc = Y019Zc[7] ^ Y019Zc[3] ;
    assign Y030Zc = Y019Zc[6] ^ Y019Zc[4] ;
    assign Y031Zc = Y019Zc[6] ^ Y019Zc[0] ;
    assign Y032Zc = Y019Zc[5] ^ Y019Zc[3] ;
    assign Y033Zc = Y019Zc[5] ^ Y029Zc ;
    assign Y034Zc = Y019Zc[5] ^ Y019Zc[1] ;
    assign Y035Zc = Y019Zc[4] ^ Y034Zc ;
    assign Y036Zc = Y019Zc[2] ^ Y032Zc ;
    assign Y037Zc = Y019Zc[1] ^ Y030Zc ;
    assign A975Yc[7] = Y032Zc ;
    assign A975Yc[6] = ~Y029Zc ;
    assign A975Yc[5] = ~Y031Zc ;
    assign A975Yc[4] = Y033Zc ;
    assign A975Yc[3] = Y030Zc ^ Y033Zc ;
    assign A975Yc[2] = Y031Zc ^ Y036Zc ;
    assign A975Yc[1] = ~Y035Zc ;
    assign A975Yc[0] = ~Y037Zc ;
endmodule
module osr_trng_A837Yc (
    input  wire                 clk
   ,input  wire                 resetn
   ,output wire [7:0]           A830Yc
   ,output wire                 A831Yc
   ,output wire                 A832Yc
   ,input  wire                 A834Yc
   ,input  wire                 A833Yc
   ,input  wire [127:0]         A835Yc
   ,input  wire [127:0]         A836Yc
    );
    reg     [127:0]             Y039Zc;
    reg     [127:0]             Y040Zc ;
    reg     [7:0]               A268Yc;
    reg                         Y041Zc;
    wire    [7:0]               Y042Zc    ;
    wire    [7:0]               Y043Zc   ;
    reg     [23:0]              Y044Zc;
    wire                        Y045Zc    ;
    wire                        Y046Zc       ;
    wire    [31:0]              Y047Zc;
    wire    [31:0]              Y048Zc;
    wire                        Y049Zc    ;
    wire                        Y050Zc       ;
    wire    [31:0]              Y051Zc;
    wire    [31:0]              Y052Zc;
    wire    [7:0]               Y053Zc;
    wire    [7:0]               Y054Zc;
    wire    [7:0]               Y055Zc;
    reg     [3:0]               Y056Zc   ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            Y041Zc <= 1'b0;
        else if (A834Yc)
            Y041Zc <=  1'b1;
        else
            Y041Zc <=  1'b0;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A268Yc <= 'h0;
        else if (~A833Yc)
            A268Yc <=  'd0;
        else if (Y041Zc)
            A268Yc <=  8'b1;
        else if (A268Yc != 0)
            A268Yc <=  A268Yc + 1'b1;
        else
            A268Yc <=  'h0;
    always @ (posedge clk or negedge resetn)
    		if (!resetn)
    				Y039Zc <=  'h0 ;
        else if (A834Yc)
            Y039Zc <=  A836Yc ^ 128'ha3b1bac6_56aa3350_677d9197_b27022dc;
        else if (Y045Zc    )
            Y039Zc <=  {Y039Zc[119:24], Y048Zc};
        else if (Y046Zc       )
            Y039Zc <=  {Y039Zc[119:0], Y039Zc[127:120]};
        else
            Y039Zc <=  Y039Zc;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
    				Y040Zc  <=  'h0 ;
        else if (A834Yc)
            Y040Zc  <=  A835Yc;
        else if (Y049Zc    )
            Y040Zc  <=  {Y040Zc [119:24], Y052Zc};
        else if (Y050Zc       )
            Y040Zc  <=  {Y040Zc [119:0], Y040Zc [127:120]};
        else if (Y056Zc   [1:0] == 2'b11)
            Y040Zc  <=  {Y040Zc [31:0],Y040Zc [127:32]};
        else
            Y040Zc  <=  Y040Zc ;
    always @ (posedge clk or negedge resetn)
    		if (!resetn)
        	Y044Zc <=  'h0 ;
        else 
        	Y044Zc <=  {Y044Zc[15:0], Y042Zc    };
    assign Y047Zc = {Y044Zc, Y042Zc    };
    assign Y048Zc = Y047Zc ^ {Y047Zc[18:0],Y047Zc[31:19]} 
                  ^ {Y047Zc[8:0],Y047Zc[31:9]} ^ {Y039Zc[23:0],Y039Zc[127:120]};
    assign Y045Zc     = A268Yc[2:0] == 3'b011;
    assign Y046Zc        = A268Yc[2] == 1'b0;
    assign Y051Zc = {Y044Zc, Y042Zc    };
    assign Y052Zc = Y051Zc ^ {Y051Zc[29:0],Y051Zc[31:30]} 
                  ^ {Y051Zc[21:0],Y051Zc[31:22]} ^ {Y051Zc[13:0],Y051Zc[31:14]} 
                  ^ {Y051Zc[7:0],Y051Zc[31:8]} ^ {Y040Zc [23:0],Y040Zc [127:120]};
    assign Y049Zc     = A268Yc[2:0] == 3'b111;
    assign Y050Zc        = A268Yc[2] == 1'b1;
    assign Y055Zc = A268Yc[1:0] == 2'b00 ? Y039Zc[31:24] :
                  A268Yc[1:0] == 2'b01 ? Y039Zc[23:16] :
                  A268Yc[1:0] == 2'b10 ? Y039Zc[15: 8] :
                  A268Yc[1:0] == 2'b11 ? Y039Zc[ 7: 0] : 0;
    assign Y043Zc    = A268Yc[2] ? 
                       (Y040Zc [31:24] ^ Y040Zc [63:56] ^ Y040Zc [95:88] ^ Y055Zc) 
                     : (Y039Zc[31:24] ^ Y039Zc[63:56] ^ Y039Zc[95:88] ^ Y054Zc);
osr_trng_Y057Zc   Y058Zc     (.A862Yc(Y042Zc    ), .Y059Zc(Y043Zc   ));
    assign Y053Zc = (A268Yc[7:3]<<2) + {6'b0,A268Yc[1:0]};
    assign Y054Zc = (Y053Zc<<3) + ~Y053Zc + 1;
    reg     Y060Zc    ;
    always @ (posedge clk or negedge resetn)
    		if (!resetn)
    			Y060Zc     <=  'h0 ;
    		else
        	Y060Zc     <=  (A268Yc == 8'hff);
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            Y056Zc    <=  'h0;
        else if (!A833Yc)
            Y056Zc    <=  'd0;
        else if (Y060Zc    )
            Y056Zc    <=  'd1;
        else if (Y056Zc    == 4'hf)
            Y056Zc    <=  'd0;
        else if (Y056Zc    != 0)
            Y056Zc    <=  Y056Zc    + 1'b1;
        else
            Y056Zc    <=  'd0;
    assign A831Yc = (|Y056Zc   ) | Y060Zc    ;
    assign A832Yc = Y056Zc    == 4'hf;
    assign A830Yc = Y056Zc   [1:0] == 2'b00 ? Y040Zc [31:24] :
                   Y056Zc   [1:0] == 2'b01 ? Y040Zc [23:16] :
                   Y056Zc   [1:0] == 2'b10 ? Y040Zc [15: 8] :
                   Y056Zc   [1:0] == 2'b11 ? Y040Zc [ 7: 0] : 0;
endmodule
module osr_trng_Y057Zc   (
    output wire [7:0]          A862Yc
   ,input  wire [7:0]          Y059Zc
    );
    assign      A862Yc                          = Y061Zc(Y059Zc);
    function    [7:0]          Y061Zc;
    input       [7:0]          Y062Zc;
    reg         [7:0]          A992Yc[0:255];
    begin
        { A992Yc[  0],A992Yc[  1],A992Yc[  2],A992Yc[  3],A992Yc[  4],A992Yc[  5],A992Yc[  6],A992Yc[  7],A992Yc[  8],A992Yc[  9],A992Yc[ 10],A992Yc[ 11],A992Yc[ 12],A992Yc[ 13],A992Yc[ 14],A992Yc[ 15],
          A992Yc[ 16],A992Yc[ 17],A992Yc[ 18],A992Yc[ 19],A992Yc[ 20],A992Yc[ 21],A992Yc[ 22],A992Yc[ 23],A992Yc[ 24],A992Yc[ 25],A992Yc[ 26],A992Yc[ 27],A992Yc[ 28],A992Yc[ 29],A992Yc[ 30],A992Yc[ 31],
          A992Yc[ 32],A992Yc[ 33],A992Yc[ 34],A992Yc[ 35],A992Yc[ 36],A992Yc[ 37],A992Yc[ 38],A992Yc[ 39],A992Yc[ 40],A992Yc[ 41],A992Yc[ 42],A992Yc[ 43],A992Yc[ 44],A992Yc[ 45],A992Yc[ 46],A992Yc[ 47],
          A992Yc[ 48],A992Yc[ 49],A992Yc[ 50],A992Yc[ 51],A992Yc[ 52],A992Yc[ 53],A992Yc[ 54],A992Yc[ 55],A992Yc[ 56],A992Yc[ 57],A992Yc[ 58],A992Yc[ 59],A992Yc[ 60],A992Yc[ 61],A992Yc[ 62],A992Yc[ 63],
          A992Yc[ 64],A992Yc[ 65],A992Yc[ 66],A992Yc[ 67],A992Yc[ 68],A992Yc[ 69],A992Yc[ 70],A992Yc[ 71],A992Yc[ 72],A992Yc[ 73],A992Yc[ 74],A992Yc[ 75],A992Yc[ 76],A992Yc[ 77],A992Yc[ 78],A992Yc[ 79],
          A992Yc[ 80],A992Yc[ 81],A992Yc[ 82],A992Yc[ 83],A992Yc[ 84],A992Yc[ 85],A992Yc[ 86],A992Yc[ 87],A992Yc[ 88],A992Yc[ 89],A992Yc[ 90],A992Yc[ 91],A992Yc[ 92],A992Yc[ 93],A992Yc[ 94],A992Yc[ 95],
          A992Yc[ 96],A992Yc[ 97],A992Yc[ 98],A992Yc[ 99],A992Yc[100],A992Yc[101],A992Yc[102],A992Yc[103],A992Yc[104],A992Yc[105],A992Yc[106],A992Yc[107],A992Yc[108],A992Yc[109],A992Yc[110],A992Yc[111],
          A992Yc[112],A992Yc[113],A992Yc[114],A992Yc[115],A992Yc[116],A992Yc[117],A992Yc[118],A992Yc[119],A992Yc[120],A992Yc[121],A992Yc[122],A992Yc[123],A992Yc[124],A992Yc[125],A992Yc[126],A992Yc[127],
          A992Yc[128],A992Yc[129],A992Yc[130],A992Yc[131],A992Yc[132],A992Yc[133],A992Yc[134],A992Yc[135],A992Yc[136],A992Yc[137],A992Yc[138],A992Yc[139],A992Yc[140],A992Yc[141],A992Yc[142],A992Yc[143],
          A992Yc[144],A992Yc[145],A992Yc[146],A992Yc[147],A992Yc[148],A992Yc[149],A992Yc[150],A992Yc[151],A992Yc[152],A992Yc[153],A992Yc[154],A992Yc[155],A992Yc[156],A992Yc[157],A992Yc[158],A992Yc[159],
          A992Yc[160],A992Yc[161],A992Yc[162],A992Yc[163],A992Yc[164],A992Yc[165],A992Yc[166],A992Yc[167],A992Yc[168],A992Yc[169],A992Yc[170],A992Yc[171],A992Yc[172],A992Yc[173],A992Yc[174],A992Yc[175],
          A992Yc[176],A992Yc[177],A992Yc[178],A992Yc[179],A992Yc[180],A992Yc[181],A992Yc[182],A992Yc[183],A992Yc[184],A992Yc[185],A992Yc[186],A992Yc[187],A992Yc[188],A992Yc[189],A992Yc[190],A992Yc[191],
          A992Yc[192],A992Yc[193],A992Yc[194],A992Yc[195],A992Yc[196],A992Yc[197],A992Yc[198],A992Yc[199],A992Yc[200],A992Yc[201],A992Yc[202],A992Yc[203],A992Yc[204],A992Yc[205],A992Yc[206],A992Yc[207],
          A992Yc[208],A992Yc[209],A992Yc[210],A992Yc[211],A992Yc[212],A992Yc[213],A992Yc[214],A992Yc[215],A992Yc[216],A992Yc[217],A992Yc[218],A992Yc[219],A992Yc[220],A992Yc[221],A992Yc[222],A992Yc[223],
          A992Yc[224],A992Yc[225],A992Yc[226],A992Yc[227],A992Yc[228],A992Yc[229],A992Yc[230],A992Yc[231],A992Yc[232],A992Yc[233],A992Yc[234],A992Yc[235],A992Yc[236],A992Yc[237],A992Yc[238],A992Yc[239],
          A992Yc[240],A992Yc[241],A992Yc[242],A992Yc[243],A992Yc[244],A992Yc[245],A992Yc[246],A992Yc[247],A992Yc[248],A992Yc[249],A992Yc[250],A992Yc[251],A992Yc[252],A992Yc[253],A992Yc[254],A992Yc[255]
        } =
        { 
        8'hd6, 8'h90, 8'he9, 8'hfe, 8'hcc, 8'he1, 8'h3d, 8'hb7, 8'h16, 8'hb6, 8'h14, 8'hc2, 8'h28, 8'hfb, 8'h2c, 8'h05, 
        8'h2b, 8'h67, 8'h9a, 8'h76, 8'h2a, 8'hbe, 8'h04, 8'hc3, 8'haa, 8'h44, 8'h13, 8'h26, 8'h49, 8'h86, 8'h06, 8'h99, 
        8'h9c, 8'h42, 8'h50, 8'hf4, 8'h91, 8'hef, 8'h98, 8'h7a, 8'h33, 8'h54, 8'h0b, 8'h43, 8'hed, 8'hcf, 8'hac, 8'h62, 
        8'he4, 8'hb3, 8'h1c, 8'ha9, 8'hc9, 8'h08, 8'he8, 8'h95, 8'h80, 8'hdf, 8'h94, 8'hfa, 8'h75, 8'h8f, 8'h3f, 8'ha6, 
        8'h47, 8'h07, 8'ha7, 8'hfc, 8'hf3, 8'h73, 8'h17, 8'hba, 8'h83, 8'h59, 8'h3c, 8'h19, 8'he6, 8'h85, 8'h4f, 8'ha8, 
        8'h68, 8'h6b, 8'h81, 8'hb2, 8'h71, 8'h64, 8'hda, 8'h8b, 8'hf8, 8'heb, 8'h0f, 8'h4b, 8'h70, 8'h56, 8'h9d, 8'h35, 
        8'h1e, 8'h24, 8'h0e, 8'h5e, 8'h63, 8'h58, 8'hd1, 8'ha2, 8'h25, 8'h22, 8'h7c, 8'h3b, 8'h01, 8'h21, 8'h78, 8'h87, 
        8'hd4, 8'h00, 8'h46, 8'h57, 8'h9f, 8'hd3, 8'h27, 8'h52, 8'h4c, 8'h36, 8'h02, 8'he7, 8'ha0, 8'hc4, 8'hc8, 8'h9e, 
        8'hea, 8'hbf, 8'h8a, 8'hd2, 8'h40, 8'hc7, 8'h38, 8'hb5, 8'ha3, 8'hf7, 8'hf2, 8'hce, 8'hf9, 8'h61, 8'h15, 8'ha1, 
        8'he0, 8'hae, 8'h5d, 8'ha4, 8'h9b, 8'h34, 8'h1a, 8'h55, 8'had, 8'h93, 8'h32, 8'h30, 8'hf5, 8'h8c, 8'hb1, 8'he3, 
        8'h1d, 8'hf6, 8'he2, 8'h2e, 8'h82, 8'h66, 8'hca, 8'h60, 8'hc0, 8'h29, 8'h23, 8'hab, 8'h0d, 8'h53, 8'h4e, 8'h6f, 
        8'hd5, 8'hdb, 8'h37, 8'h45, 8'hde, 8'hfd, 8'h8e, 8'h2f, 8'h03, 8'hff, 8'h6a, 8'h72, 8'h6d, 8'h6c, 8'h5b, 8'h51, 
        8'h8d, 8'h1b, 8'haf, 8'h92, 8'hbb, 8'hdd, 8'hbc, 8'h7f, 8'h11, 8'hd9, 8'h5c, 8'h41, 8'h1f, 8'h10, 8'h5a, 8'hd8, 
        8'h0a, 8'hc1, 8'h31, 8'h88, 8'ha5, 8'hcd, 8'h7b, 8'hbd, 8'h2d, 8'h74, 8'hd0, 8'h12, 8'hb8, 8'he5, 8'hb4, 8'hb0, 
        8'h89, 8'h69, 8'h97, 8'h4a, 8'h0c, 8'h96, 8'h77, 8'h7e, 8'h65, 8'hb9, 8'hf1, 8'h09, 8'hc5, 8'h6e, 8'hc6, 8'h84, 
        8'h18, 8'hf0, 8'h7d, 8'hec, 8'h3a, 8'hdc, 8'h4d, 8'h20, 8'h79, 8'hee, 8'h5f, 8'h3e, 8'hd7, 8'hcb, 8'h39, 8'h48
        };
        Y061Zc = A992Yc[Y062Zc];
    end
    endfunction
endmodule
module osr_trng_A752Yc   #(
    parameter                               A008Yc          = 10
   ,parameter                               A007Yc          = 32
)(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire                            A139Yc  
   ,input   wire                            A754Yc              
   ,input   wire                            A063Yc              
   ,output  wire                            A755Yc              
   ,output  wire                            A791Yc              
   ,input   wire                            A138Yc              
   ,output  wire                            A756Yc              
   ,input   wire                            A757Yc              
   ,input   wire    [A007Yc      -1:0]      A758Yc              
   ,input   wire    [A008Yc       -1:0]     A759Yc         
   ,input   wire                            A760Yc         
   ,output  wire                            A761Yc              
   ,output  wire    [A007Yc      -1:0]      A762Yc          
   ,output  wire                            A763Yc          
   ,output  wire                            A764Yc           
   ,output  wire                            A765Yc           
   ,input   wire                            A766Yc         
   ,input   wire                            A767Yc     
   ,input   wire    [31:0]                  A768Yc     
   ,input   wire                            A770Yc         
   ,output  wire                            A771Yc         
   ,output  wire    [31:0]                  A772Yc        
   ,output  wire                            A769Yc       
   ,input   wire                            A541Yc    
   ,input   wire                            A542Yc    
   ,input   wire                            A543Yc        
   ,output  wire                            A773Yc     
   ,input   wire                            A774Yc     
   ,input   wire                            A775Yc     
   ,input   wire                            A776Yc     
   ,input   wire                            A777Yc         
   ,input   wire    [31:0]                  A778Yc        
   ,output  wire                            A779Yc   
   ,output  wire                            A780Yc         
   ,output  wire                            A781Yc     
   ,output  wire    [31:0]                  A782Yc     
   ,input   wire    [A007Yc      -1:0]      A783Yc     
   ,input   wire                            A166Yc     
   ,input   wire                            A784Yc      
   ,input   wire                            A785Yc        
   ,output  wire    [A008Yc       -1:0]     A786Yc              
   ,output  wire    [A008Yc       -1:0]     A787Yc               
   ,output  wire    [A007Yc      -1:0]      A037Yc     
   ,output  wire                            A788Yc          
   ,output  wire                            A789Yc          
   ,output  wire                            A790Yc    
);
    localparam                              A234Yc              = 3'd3;
    localparam                              A276Yc              = 3'b000;
    localparam                              Y063Zc              = 3'b001;
    localparam                              Y064Zc              = 3'b010;
    localparam                              Y065Zc              = 3'b011;
    localparam                              Y066Zc              = 3'b100;
    localparam                              Y067Zc              = 3'b101;
    localparam                              A281Yc              = 3'b110;
    localparam                              A237Yc              = 3'b111;
    reg     [A234Yc       -1:0]             A283Yc       , A284Yc    ;
    reg                                     Y068Zc   ;
    reg                                     Y069Zc       ;
    wire                                    Y070Zc           ;
    wire                                    Y071Zc          ;
    wire                                    Y072Zc       ;
    reg     [1:0]                           A571Yc          ;
    reg                                     Y073Zc             ;
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A283Yc          <=  A276Yc;
        else if(A775Yc      | A541Yc     | A542Yc    )
            A283Yc          <=  A237Yc ;
        else
            A283Yc          <=  A284Yc    ;
    end
    always @( * )
    begin
        A284Yc     = A283Yc       ;
        case(A283Yc       )
            A276Yc          :   A284Yc     = A139Yc   ? (A754Yc      ? A281Yc : Y063Zc         ) : A276Yc;
            Y063Zc          :   A284Yc     = A139Yc   ? Y064Zc      : A276Yc;
            Y064Zc          :   A284Yc     = A139Yc   ? (A774Yc      ? Y065Zc     : Y064Zc     ) : A276Yc;
            Y065Zc          :   A284Yc     = A139Yc   ? Y066Zc              : A276Yc;
            Y066Zc          :   A284Yc     = A139Yc   ? (A138Yc          ? Y067Zc  : Y066Zc          ) : A276Yc;
            Y067Zc          :   A284Yc     = A139Yc   ? (A063Yc        ? A281Yc : Y067Zc ) : A276Yc;
            A281Yc          :   A284Yc     = A139Yc   ? A281Yc      : A276Yc;
            A237Yc          :   A284Yc     = A139Yc   ? A237Yc      : A276Yc;
            default         :   A284Yc     = A276Yc;
        endcase
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            Y068Zc          <=  1'b0  ;
            Y069Zc          <=  1'b1  ; 
        end
        else begin
            case(A283Yc       )
                A276Yc          :   begin
                                        Y068Zc          <=  1'b0;
                                        Y069Zc          <=  1'b1;
                                    end
                Y063Zc          :   begin
                                        Y068Zc          <=  1'b1;
                                    end
                Y064Zc          :   begin
                                        Y068Zc          <=  1'b0;
                                    end
                Y065Zc          :   begin
                                        Y069Zc          <=  1'b0;
                                    end
                A281Yc          :   begin
                                        Y069Zc          <=  1'b1;
                                    end
                default         : ;
            endcase
        end
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A571Yc              <=  2'b0;
            Y073Zc              <=  1'b0;
       end 
        else begin
            if(A774Yc     ) begin
                A571Yc          [0] <=  1'b1;
                Y073Zc              <=  1'b1;
            end
            if(A543Yc        )
                A571Yc          [1] <=  1'b1;
            if(&A571Yc          )
                A571Yc              <=  2'b00;
        end
    end
    assign  Y070Zc              = A283Yc        == Y064Zc     ;
    assign  Y071Zc              = A283Yc        == Y067Zc ;
    assign  Y072Zc              = A283Yc        == A237Yc ;
    assign  A756Yc              = Y071Zc          ;
    assign  A761Yc              = Y070Zc            ? 1'b0  : A766Yc         ;
    assign  A762Yc              = A783Yc     ;
    assign  A763Yc              = A166Yc     ;
    assign  A764Yc              = A784Yc      ;
    assign  A765Yc              = A785Yc        ;
    assign  A771Yc              = Y070Zc            ? A777Yc          : !A757Yc      ;
    assign  A772Yc              = Y070Zc            ? A778Yc          : A758Yc     ;
    assign  A769Yc              = (A283Yc        == A281Yc) ? A063Yc        : (A139Yc   & Y069Zc       );
    assign  A773Yc              = Y072Zc       ;
    assign  A791Yc              = Y073Zc             ;
    assign  A755Yc              = &A571Yc          ;
    assign  A779Yc              = Y068Zc   ;
    assign  A780Yc              = Y070Zc            ? A766Yc          : 1'b0;
    assign  A781Yc              = Y070Zc            ? A767Yc          : 1'b0;
    assign  A782Yc              = Y070Zc            ? A768Yc          : 32'h0;
    assign  A786Yc              = A759Yc         ;
    assign  A787Yc               ={A008Yc       {1'b0}};
    assign  A037Yc              = A768Yc     ;
    assign  A788Yc              = A760Yc         ;
    assign  A789Yc              = Y070Zc            ? 1'b0  : A767Yc     ;
    assign  A790Yc              = !(A139Yc  &Y069Zc       ) | A770Yc         ;
endmodule
module osr_trng_A792Yc 
(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire                            A139Yc  
   ,input   wire                            A195Yc              
   ,input   wire                            A491Yc              
   ,input   wire    [31:0]                  A196Yc              
   ,output  wire                            A197Yc              
   ,output  wire                            A198Yc              
   ,output  wire    [31:0]                  A199Yc              
   ,output  wire                            A790Yc              
);
    localparam                              A234Yc              = 2'd3  ;
    localparam                              A276Yc              = 3'b000,
                                            A277Yc              = 3'b001,
                                            A278Yc              = 3'b010,
                                            A279Yc              = 3'b011,
                                            A280Yc              = 3'b100,
                                            A281Yc              = 3'b101;
    genvar                                  A282Yc;
    reg     [A234Yc       -1:0]             A283Yc       , A284Yc    ;
    reg     [127:0]                         A285Yc;
    wire    [31:0]                          A286Yc;
    reg                                     A287Yc;
    reg                                     A288Yc    ;
    reg     [31:0]                          A289Yc;
    reg     [2:0]                           A290Yc   ;
    reg     [11:0]                          A268Yc;
    generate 
    for(A282Yc=0;A282Yc<32;A282Yc=A282Yc+1) begin: Y074Zc   
        assign  A286Yc[0+A282Yc] = A285Yc[96+A282Yc] ^ A285Yc[94+A282Yc] ^ A285Yc[69+A282Yc] ^ A285Yc[67+A282Yc];
    end
    endgenerate
    always @(posedge clk or negedge rst_n)
    begin 
        if(!rst_n) begin
            A283Yc          <=  A276Yc;
        end
        else if(A491Yc  )
            A283Yc          <=  A276Yc;
        else
            A283Yc          <=  A284Yc    ;
    end
    wire Y075Zc   = A139Yc   & !A491Yc   ;
    always @( * )
    begin
        A284Yc     = A283Yc       ;
        case(A283Yc       )
            A276Yc      :   A284Yc     = Y075Zc   ? (A195Yc     ? A277Yc : A276Yc) : A276Yc ;
            A277Yc      :   A284Yc     = Y075Zc   ? A278Yc : A276Yc ;
            A278Yc      :   A284Yc     = Y075Zc   ? A280Yc : A276Yc ;
            A279Yc      :   A284Yc     = Y075Zc   ? (A195Yc     ? A277Yc : A279Yc) : A276Yc ;
            A280Yc      :   A284Yc     = Y075Zc   ? ((A290Yc   [2:0] == 3'h4) ? A281Yc : A279Yc) : A276Yc ;
            A281Yc      :   A284Yc     = Y075Zc   ? ((A268Yc[11:0] >=12'd4000) ? A276Yc : A281Yc) : A276Yc ;
            default     :   A284Yc     = A276Yc ;
        endcase
    end
     wire  Y076Zc          =   A283Yc         ==  A276Yc ;
     wire  Y077Zc          =   A283Yc         ==  A277Yc ;
     wire  Y078Zc          =   A283Yc         ==  A278Yc ;
     wire  Y079Zc          =   A283Yc         ==  A279Yc ;
     wire  Y080Zc          =   A283Yc         ==  A280Yc ;
     wire  Y081Zc          =   A283Yc         ==  A281Yc ;
     wire                   Y082Zc         = A290Yc    == 3'd1 ;
     wire                   Y083Zc         = A290Yc    == 3'd2 ;
     wire                   Y084Zc         = A290Yc    == 3'd3 ;
     wire                   Y085Zc         = A290Yc    == 3'd4 ;
     wire [31:0]            Y086Zc      =  Y076Zc       ? 31'd0 : (Y081Zc      ? A286Yc[31:0]   : (Y082Zc        ? A196Yc    : A285Yc[31:0]   )) ;    
     wire [31:0]            Y087Zc      =  Y076Zc       ? 31'd0 : (Y081Zc      ? A285Yc[31:0]   : (Y083Zc        ? A196Yc    : A285Yc[63:32]  )) ;    
     wire [31:0]            Y088Zc      =  Y076Zc       ? 31'd0 : (Y081Zc      ? A285Yc[63:32]  : (Y084Zc        ? A196Yc    : A285Yc[95:64]  )) ;    
     wire [31:0]            Y089Zc      =  Y076Zc       ? 31'd0 : (Y081Zc      ? A285Yc[95:64]  : (Y085Zc        ? A196Yc    : A285Yc[127:96] )) ;    
	 wire [127:0]           Y090Zc      =  {Y089Zc   , Y088Zc   , Y087Zc   , Y086Zc    } ;      
     wire                   A634Yc      =  Y081Zc       ? 1'b1   : 1'b0  ;
     wire                   A633Yc      =  Y077Zc       ? 1'b1   : 1'b0  ;
     wire [31:0]            A635Yc      =  Y081Zc       ? A286Yc[31:0] : 32'b0 ;
     wire [11:0]            Y091Zc      =  Y081Zc       ? A268Yc + 1    : 12'b0 ;
     wire [2:0]             Y092Zc      =  
                           (Y081Zc      | Y076Zc      ) ? 3'd0 : 
                                           Y078Zc       ? A290Yc    + 1'b1 : 
                                                          A290Yc     ;
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            A285Yc      <=  128'h0 ;
            A287Yc      <=  1'b0   ;
            A288Yc      <=  1'b0   ;
            A289Yc      <=  32'h0  ;
            A290Yc      <=  3'd0   ;
            A268Yc      <=  12'h0  ;
        end
        else begin
            A285Yc      <=    Y090Zc     ; 
            A287Yc      <=    A634Yc     ;
            A288Yc      <=    A633Yc     ;
            A289Yc      <=    A635Yc     ;
            A290Yc      <=    Y092Zc     ;
            A268Yc      <=    Y091Zc     ;
        end
    end
    assign  A197Yc      = A288Yc     ;
    assign  A198Yc      = A287Yc     ;
    assign  A199Yc      = A289Yc     ;
    assign  A790Yc      = A491Yc     ;    
endmodule
module osr_trng_Y093Zc                       #(
	parameter	Y094Zc         	    =	15		,        
	parameter	Y095Zc             	=	32		,	
	parameter	Y096Zc      			=	1024	,	
	parameter	Y097Zc      			=	624		
)(
    input   wire                                i_clk				,
    input   wire                                i_rstn				,
    input   wire                                Y098Zc              , 
    input   wire                                Y099Zc              , 
    input   wire    [Y095Zc             -1:0]   A211Yc        		,
    input   wire                                A212Yc         	    , 
    input   wire    [14:0]                      Y100Zc  			,
    input   wire    [15:0]                      Y101Zc      		,
    input   wire    [2*(Y094Zc         -1)+9:0] Y102Zc              ,
    input   wire    [2*(Y094Zc         -1)+9:0] Y103Zc              ,
    output  wire    [13:0]                      Y104Zc              ,
    output  wire    [13:0]                      Y105Zc              ,
    output  wire    [1:0]                       Y106Zc              ,
    output  wire                                Y107Zc              ,
    output  wire                                Y108Zc         		, 
    output  wire                                Y109Zc          	, 
    output  wire                                A220Yc     
);
    localparam  A234Yc              =   5		    ;
    localparam  A235Yc              =   5'b00001    ;
    localparam  Y110Zc              =   5'b00010    ;
    localparam  Y111Zc              =   5'b00100    ;
    localparam  Y112Zc              =   5'b01000    ;
    localparam  Y113Zc              =   5'b10000    ;
    localparam  Y114Zc              =   2           ;
    reg                             Y115Zc         ,Y116Zc              ;
    wire[Y095Zc             -1:0]   Y117Zc    ;	
    reg	[Y095Zc             -1:0]   Y118Zc     ,Y119Zc          ;	
    reg	[Y095Zc             -1:0]   Y120Zc          ,Y121Zc               ;	
    reg	[Y094Zc         -1:0]       Y122Zc    ,Y123Zc         ;	
    reg	[Y094Zc         -1:0]       Y124Zc     ,Y125Zc          ; 
    reg	[Y094Zc         -1:0]       Y126Zc       ,Y127Zc            ; 
    reg	[A234Yc       -1:0]         A244Yc,A245Yc; 
    reg	[Y094Zc         -1:0]       Y128Zc ,Y129Zc ,Y130Zc      ,Y131Zc      ; 
    reg [Y094Zc         +8:0]       Y132Zc;
    reg [Y094Zc         -2:0]       Y133Zc;
    reg [2*Y094Zc         +7:0]     Y134Zc;
    reg [2*(Y094Zc         -1)+9:0] Y135Zc      , Y136Zc           ;
    reg [2*(Y094Zc         -1)+9:0] Y137Zc      , Y138Zc           ;
    wire                            Y139Zc          ;
    reg                             Y140Zc         ,Y141Zc              ;
    reg                             Y142Zc          ,Y143Zc               ;
    reg                             Y144Zc     ,Y145Zc          ;
    wire                            Y146Zc  ;
    always @(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            A244Yc              <=  A235Yc ;
            Y118Zc      	    <=  'd0;
            Y120Zc              <=  'd0;
            Y115Zc              <=  'd0;
            Y124Zc      	    <=  'd0;
            Y126Zc       	    <=  'd0;
            Y128Zc              <=  'd0;
            Y129Zc              <=  'd0;
            Y122Zc              <=  'd0;
            Y140Zc              <=  'b0;
            Y142Zc              <=  'b0;
            Y144Zc              <=  'b0;
        end
        else begin 
            A244Yc              <=  Y146Zc   ? A245Yc                 : A235Yc ;
            Y118Zc      	    <=  Y146Zc   ? Y119Zc                 : 'd0;
            Y120Zc              <=  Y146Zc   ? Y121Zc                 : 'd0;
            Y115Zc              <=  Y146Zc   ? Y116Zc                 : 'd0;
            Y124Zc      	    <=  Y146Zc   ? Y125Zc                 : 'd0;
            Y126Zc       	    <=  Y146Zc   ? Y127Zc                 : 'd0;
            Y128Zc              <=  Y146Zc   ? Y130Zc                 : 'd0;
            Y129Zc              <=  Y146Zc   ? Y131Zc                 : 'd0;
            Y122Zc              <=  Y146Zc   ? Y123Zc                 : 'd0;
            Y140Zc              <=  Y146Zc   ? Y141Zc                 : 'b0;
            Y142Zc              <=  Y146Zc   ? Y143Zc                 : 'b0;
            Y144Zc      	    <=  Y146Zc   ? Y145Zc                 : 'b0;
        end
    end
always@(*)
begin
    A245Yc                  = A244Yc            ;
    Y119Zc                  = Y118Zc            ;
    Y121Zc                  = Y120Zc            ;
    Y116Zc                  = Y115Zc            ;
    Y125Zc                  = Y124Zc            ;
    Y127Zc                  = Y126Zc            ;
    Y130Zc                  = Y128Zc            ;
    Y131Zc                  = Y129Zc            ;
    Y123Zc                  = Y122Zc            ;
    Y141Zc                  = Y140Zc            ;
    Y143Zc                  = Y142Zc            ;
    Y145Zc                  = Y144Zc            ;
    case(A244Yc)
        A235Yc 		:	A245Yc = Y110Zc   ;
        Y110Zc   	:	begin
                            if (Y146Zc  &&A212Yc         ) begin
                            	A245Yc = Y111Zc ;
                            	Y119Zc                  = A211Yc        ;
                            	Y121Zc                  = A211Yc         ^ Y117Zc    ;
                            	Y145Zc                  = 1'b1;
                            end
                        end
        Y112Zc 		:	begin
        						Y119Zc           = {Y118Zc     [Y095Zc             -5:0],4'd0};
        						Y121Zc                = {Y120Zc          [Y095Zc             -5:0],4'd0};
        						if ((Y126Zc        +16'b0) == (Y100Zc   >> 2)) begin 
                                    if(Y099Zc   )begin
                                        A245Yc = Y113Zc;
                                        Y125Zc           = 'd0;
                                    end
                                    else begin        
        						        A245Yc = Y110Zc   ;
        						        Y145Zc           = 1'b0;
        						        Y125Zc           = 'd0;
        						        Y127Zc             = 'd0;	
        						        Y130Zc       = 'd0;
        						        Y131Zc       = 'd0;
                                    end
        						end
        						else if ((Y124Zc     ) == (Y095Zc              >> 2)) begin 
        						    A245Yc = Y110Zc   ;
        						    Y145Zc           = 1'b0;
        						    Y125Zc           = 'd0;
                                    Y116Zc               = Y118Zc     [28];
        						end
        						else
        						    A245Yc = Y111Zc ;
        					end
        Y111Zc 		: 	begin
                                A245Yc = Y112Zc ;
                                Y125Zc           = Y124Zc      + 1;
                                Y127Zc             = Y126Zc        + 1;
                                case (Y118Zc     [Y095Zc             -1:Y095Zc             -4])
                                    4'h0:   begin 
                                                Y130Zc       = Y128Zc  + 'd4;
                                                if ((Y129Zc  +16'b0> Y101Zc      ) || (Y128Zc  + 16'd4 > Y101Zc      )) begin
                                                    Y141Zc               = 'b1;
                                                end
                                            end
                                    4'h1,
                                    4'h2,
                                    4'h4,
                                    4'h8:   begin
                                                Y130Zc       = Y128Zc  + 'd3;
                                                Y131Zc       = Y129Zc  + 'd1;
                                                if ((Y129Zc  + 16'd1 > Y101Zc      ) || (Y128Zc  + 16'd3 > Y101Zc      )) begin
                                                    Y141Zc               = 'b1;
                                                end
                                            end
                                    4'h3,
                                    4'h5,
                                    4'h6,
                                    4'h9,
                                    4'hA,
                                    4'hC:   begin 
                                                Y130Zc       = Y128Zc  + 'd2; 
                                                Y131Zc       = Y129Zc  + 'd2; 
                                                if ((Y129Zc  + 16'd2 > Y101Zc      ) || (Y128Zc  + 16'd2 > Y101Zc      )) begin
                                                    Y141Zc               = 'b1;
                                                end
                                            end
                                    4'h7,
                                    4'hB,
                                    4'hD,
                                    4'hE:   begin 
                                                Y130Zc       = Y128Zc  + 'd1; 
                                                Y131Zc       = Y129Zc  + 'd3; 
                                                if ((Y129Zc  + 16'd3 > Y101Zc      ) || (Y128Zc  + 16'd1 > Y101Zc      )) begin
                                                    Y141Zc               = 'b1;
                                                end
                                            end
                                    4'hF:   begin 
                                                Y131Zc       = Y129Zc  + 'd4; 
                                                if ((Y129Zc  + 16'd4 > Y101Zc      ) || (Y128Zc  +16'b0> Y101Zc      )) begin
                                                    Y141Zc               = 'b1;
                                                end
                                            end
                                    default : ;
                                endcase
                                case (Y120Zc          [Y095Zc             -1:Y095Zc             -4])
                                    4'h1,
                                    4'h2,
                                    4'h4,
                                    4'h8:   begin
                                                Y123Zc          = Y122Zc     + 'd1;
                                            end
                                    4'h3,
                                    4'h5,
                                    4'h6,
                                    4'h9,
                                    4'hA,
                                    4'hC:   begin 
                                                Y123Zc          = Y122Zc     + 'd2; 
                                            end
                                    4'h7,
                                    4'hB,
                                    4'hD,
                                    4'hE:   begin 
                                                Y123Zc          = Y122Zc     + 'd3; 
                                            end
                                    4'hF:   begin 
                                                Y123Zc          = Y122Zc     + 'd4; 
                                            end
                                    default:   Y123Zc          = Y122Zc    ;
                                endcase
        					end
            Y113Zc      :   begin   
                                    Y125Zc           = Y124Zc      + 1'b1;
                                    if(Y124Zc      == 'd4) begin
                                        if(!(({1'b0,Y122Zc    } < Y102Zc      [37:20+Y114Zc   ]) || (({1'b0,Y122Zc    } == Y102Zc      [37:20+Y114Zc   ]) && (|Y102Zc      [20+Y114Zc   -1:0]))) || ({1'b0,Y122Zc    } < Y103Zc      [37:20+Y114Zc   ])) begin
                                            Y143Zc                = 'b1;
                                        end 
                                            Y123Zc          = 'd0;                           
        						            A245Yc = Y110Zc   ;
        						            Y145Zc           = 1'b0;
        						            Y125Zc           = 'd0;
        						            Y127Zc             = 'd0;	
        						            Y130Zc       = 'd0;
        						            Y131Zc       = 'd0;                                                                        
                                    end                              
                                    else
                                        A245Yc = Y113Zc;
                            end                              
        default		: ;
    endcase
end
    assign Y139Zc              = (A244Yc == Y113Zc);
    assign  Y146Zc          = Y098Zc   || Y099Zc   ;
    assign  Y117Zc          = {Y115Zc         , A211Yc        [Y095Zc             -1:1]};
    assign  Y104Zc              = Y128Zc [13:0];
    assign  Y105Zc              = Y129Zc [13:0];
    assign  Y106Zc              = Y124Zc     [1:0];
    assign  Y107Zc              = Y139Zc          ;    
    assign  Y108Zc              = Y098Zc   && Y140Zc         ;
    assign  Y109Zc              = Y142Zc          ;
    assign  A220Yc              = Y144Zc     ;
endmodule
module osr_trng_Y147Zc                           #(
	parameter	Y094Zc         	    =	15		,	
	parameter	Y095Zc             	=	32		,	
	parameter	Y096Zc      			=	512	,	
	parameter	Y097Zc      			=	77			
)(
	input	wire									i_clk					    ,
	input	wire									i_rstn				        ,
	input	wire									Y148Zc     		            , 
	input	wire									Y149Zc                      , 
	input	wire    [Y095Zc             -1:0]	    A211Yc        		        ,
	input	wire    								A212Yc         	            , 
	input	wire	[12:0]							Y100Zc  				    ,
	input	wire	[11:0]							Y150Zc         				,
	input	wire	[23:0]							A034Yc        				,
    output  wire    [9:0]                           A043Yc                      ,
    input   wire    [15:0]                          Y151Zc         		        ,
    input   wire    [15:0]                          Y152Zc                      ,
    input   wire    [13:0]                          Y153Zc                      ,
    input   wire    [13:0]                          Y154Zc                      ,
    output  wire    [2*(Y094Zc         -1)+9:0]     Y155Zc                      ,
    output  wire    [2*(Y094Zc         -1)+9:0]     Y156Zc                      ,
    input   wire    [1:0]                           Y157Zc                      ,    
    input   wire                                    Y158Zc                      ,    
	output	wire								    Y159Zc            			, 
	output	wire								    Y160Zc           			, 
	output	wire								    A220Yc     
);
	localparam	A234Yc       		=	5			;
	localparam	A235Yc 				=	5'b00001	;
	localparam	Y110Zc   			=	5'b00010	;
	localparam	Y111Zc 				=	5'b00100	;
	localparam	Y112Zc 				=	5'b01000	;
	localparam	Y113Zc              =   5'b10000    ;
	reg	[Y095Zc             -1:0]	Y118Zc     ,Y119Zc          ;	
	reg	[Y094Zc         -1:0]		Y124Zc     ,Y125Zc          ; 
	reg	[Y094Zc         -1:0]		Y126Zc       ,Y127Zc            ; 
	reg	[A234Yc       -1:0]			A244Yc,A245Yc; 
	reg	[Y094Zc         -1:0]		Y128Zc , 
								    Y129Zc ,
								    Y161Zc ,
								    Y162Zc ,
								    Y163Zc ,
								    Y164Zc ,
								    Y165Zc ,
								    Y166Zc ,
								    Y167Zc ,
								    Y168Zc ,
								    Y169Zc ,
								    Y170Zc ,
								    Y171Zc ,
								    Y172Zc ,
								    Y173Zc ,
								    Y174Zc ;
	reg	[Y094Zc         -1:0]		Y130Zc      ,
								    Y131Zc      ,
								    Y175Zc      ,
								    Y176Zc      ,
								    Y177Zc      ,
								    Y178Zc      ,
								    Y179Zc      ,
								    Y180Zc      ,
								    Y181Zc      ,
								    Y182Zc      ,
								    Y183Zc      ,
								    Y184Zc      ,
								    Y185Zc      ,
								    Y186Zc      ,
								    Y187Zc      ,
								    Y188Zc      ; 
	reg	[Y094Zc         -1:0]	    Y189Zc       ;                                        
    reg [29:0]                      Y190Zc      ;
    reg [2*Y094Zc         -3:0]     Y191Zc        ;    
    wire                            Y192Zc           ;
	reg								Y193Zc            ,Y194Zc                 ;
	reg								Y195Zc           ,Y196Zc                ;
	reg								Y144Zc     ,Y145Zc          ;
    wire                            Y146Zc  ;  
    reg  [2:0]                      Y197Zc     ;
    reg                             Y198Zc          ;
    reg [9:0]                       Y199Zc           ;
    reg [Y094Zc         +8:0]       Y132Zc;
    reg [Y094Zc         -2:0]       Y133Zc;
    wire [2*Y094Zc         +7:0]     Y134Zc;
    reg [2*(Y094Zc         -1)+9:0] Y135Zc      , Y136Zc           ;
    reg [2*(Y094Zc         -1)+9:0] Y137Zc      , Y138Zc           ;
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
			A244Yc 				<=  A235Yc    ;
			Y118Zc      	<=  'd0       ;
			Y124Zc      	<=  'd0       ;
			Y126Zc       	<=  'd0       ;
			Y128Zc  			<=  'd0       ;
			Y129Zc  			<=  'd0       ;
			Y161Zc  			<=  'd0       ;
			Y162Zc  			<=  'd0       ;
			Y163Zc  			<=  'd0       ;
			Y164Zc  			<=  'd0       ;
			Y165Zc  			<=  'd0       ;
			Y166Zc  			<=  'd0       ;
			Y167Zc  			<=  'd0       ;
			Y168Zc  			<=  'd0       ;
			Y169Zc  			<=  'd0       ;
			Y170Zc  			<=  'd0       ;
			Y171Zc  			<=  'd0       ;
			Y172Zc  			<=  'd0       ;
			Y173Zc  			<=  'd0       ;
			Y174Zc  			<=  'd0       ;
			Y193Zc             	<=  'b0       ;
			Y195Zc            	<=  'b0       ;
			Y144Zc      	<=  'b0       ;
		end
		else begin 
			A244Yc 				<=  Y146Zc   ? A245Yc 					: A235Yc    ;
			Y118Zc      	    <=  Y146Zc   ? Y119Zc          	    : 'd0       ;
			Y124Zc      	    <=  Y146Zc   ? Y125Zc          	    : 'd0       ;
			Y126Zc       	    <=  Y146Zc   ? Y127Zc            	    : 'd0       ;
			Y128Zc  			<=  Y146Zc   ? Y130Zc      			: 'd0       ;
			Y129Zc  			<=  Y146Zc   ? Y131Zc      			: 'd0       ;
			Y161Zc  			<=  Y146Zc   ? Y175Zc      			: 'd0       ;
			Y162Zc  			<=  Y146Zc   ? Y176Zc      			: 'd0       ;
			Y163Zc  			<=  Y146Zc   ? Y177Zc      			: 'd0       ;
			Y164Zc  			<=  Y146Zc   ? Y178Zc      			: 'd0       ;
			Y165Zc  			<=  Y146Zc   ? Y179Zc      			: 'd0       ;
			Y166Zc  			<=  Y146Zc   ? Y180Zc      			: 'd0       ;
			Y167Zc  			<=  Y146Zc   ? Y181Zc      			: 'd0       ;
			Y168Zc  			<=  Y146Zc   ? Y182Zc      			: 'd0       ;
			Y169Zc  			<=  Y146Zc   ? Y183Zc      			: 'd0       ;
			Y170Zc  			<=  Y146Zc   ? Y184Zc      			: 'd0       ;
			Y171Zc  			<=  Y146Zc   ? Y185Zc      			: 'd0       ;
			Y172Zc  			<=  Y146Zc   ? Y186Zc      			: 'd0       ;
			Y173Zc  			<=  Y146Zc   ? Y187Zc      			: 'd0       ;
			Y174Zc  			<=  Y146Zc   ? Y188Zc      			: 'd0       ;
            Y193Zc              <=  Y146Zc   ? Y194Zc                 : 'b0       ;
            Y195Zc              <=  Y146Zc   ? Y196Zc                 : 'b0       ;
			Y144Zc      	    <=  Y146Zc   ? Y145Zc          	    : 'b0       ;
		end
	end
	always@(*)
	begin
		A245Yc                  = A244Yc				;
		Y119Zc          		= Y118Zc     		;
		Y125Zc          		= Y124Zc     		;
		Y127Zc            	    = Y126Zc       	    ;
		Y130Zc      			= Y128Zc 			;
		Y131Zc      			= Y129Zc 			;
		Y175Zc      			= Y161Zc 			;
		Y176Zc      			= Y162Zc 			;
		Y177Zc      			= Y163Zc 			;
		Y178Zc      			= Y164Zc 			;
		Y179Zc      			= Y165Zc 			;
		Y180Zc      			= Y166Zc 			;
		Y181Zc      			= Y167Zc 			;
		Y182Zc      			= Y168Zc 			;
		Y183Zc      			= Y169Zc 			;
		Y184Zc      			= Y170Zc 			;
		Y185Zc      			= Y171Zc 			;
		Y186Zc      			= Y172Zc 			;
		Y187Zc      			= Y173Zc 			;
		Y188Zc      			= Y174Zc 			;
		Y194Zc                 	= Y193Zc            ;
        Y196Zc                  = Y195Zc            ;       
		Y145Zc          		= Y144Zc     		;
		case(A244Yc)
			A235Yc 		:	A245Yc = Y110Zc   ;
			Y110Zc   	:	begin
									if (Y146Zc  &&A212Yc         ) begin
										A245Yc = Y111Zc ;
										Y119Zc           = A211Yc        ;
										Y145Zc           = 1'b1;
									end
								end
			Y112Zc 		:	begin
									Y119Zc           = {Y118Zc     [Y095Zc             -5:0],4'd0};
									if ((Y126Zc        +16'b0) == (Y100Zc  )) begin 
                                        if (Y149Zc    )begin
										    A245Yc = Y113Zc;
										    Y125Zc           = 'd0;
                                        end                                    
                                        else begin        
										    A245Yc = Y110Zc   ;
										    Y145Zc           = 1'b0;
										    Y125Zc           = 'd0;
										    Y127Zc             = 'd0;	
										    Y130Zc       = 'd0;
										    Y131Zc       = 'd0;
										    Y175Zc       = 'd0;
										    Y176Zc       = 'd0;
										    Y177Zc       = 'd0;
										    Y178Zc       = 'd0;
										    Y179Zc       = 'd0;
										    Y180Zc       = 'd0;
										    Y181Zc       = 'd0;
										    Y182Zc       = 'd0;
										    Y183Zc       = 'd0;
										    Y184Zc       = 'd0;
										    Y185Zc       = 'd0;
										    Y186Zc       = 'd0;
										    Y187Zc       = 'd0;
										    Y188Zc       = 'd0;
                                        end
									end
									else if ((Y124Zc     ) == (Y095Zc              >> 2)) begin 
										A245Yc = Y110Zc   ;
										Y145Zc           = 1'b0;
										Y125Zc           = 'd0;
									end
									else
										A245Yc = Y111Zc ;
                            end
			Y111Zc 		: 	begin
									A245Yc = Y112Zc ;
									Y125Zc           = Y124Zc      + 1;
									Y127Zc             = Y126Zc        + 1;
									case (Y118Zc     [Y095Zc             -1:Y095Zc             -4])
										4'h0: begin
													Y130Zc       = Y128Zc  + 'd1;
													if (Y128Zc  + 16'd1> Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h1: begin
													Y131Zc       = Y129Zc  + 'd1;
													if (Y129Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h2:  begin
													Y175Zc       = Y161Zc  + 'd1;
													if (Y161Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h3: begin
													Y176Zc       = Y162Zc  + 'd1;
													if (Y162Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h4: begin
													Y177Zc       = Y163Zc  + 'd1;
													if (Y163Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h5: begin
													Y178Zc       = Y164Zc  + 'd1;
													if (Y164Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h6: begin
													Y179Zc       = Y165Zc  + 'd1;
													if (Y165Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h7: begin
													Y180Zc       = Y166Zc  + 'd1;
													if (Y166Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h8: begin
													Y181Zc       = Y167Zc  + 'd1;
													if (Y167Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'h9: begin
													Y182Zc       = Y168Zc  + 'd1;
													if (Y168Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'hA: begin
													Y183Zc       = Y169Zc  + 'd1; 
													if (Y169Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'hB: begin
													Y184Zc       = Y170Zc  + 'd1;
													if (Y170Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'hC: begin
													Y185Zc       = Y171Zc  + 'd1;
													if (Y171Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'hD: begin
													Y186Zc       = Y172Zc  + 'd1;
													if (Y172Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'hE: begin
													Y187Zc       = Y173Zc  + 'd1; 
													if (Y173Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										4'hF: begin
													Y188Zc       = Y174Zc  + 'd1; 
													if (Y174Zc  + 16'd1 > Y150Zc         ) begin
														Y194Zc                  = 'b1;
													end
												end
										default : ;
									endcase
								end
            Y113Zc      :   begin   
                                    Y125Zc           = Y124Zc      + 1'b1;
                                    if(Y124Zc      == 'd20) begin
                                        if((Y190Zc      [23:0] >= A034Yc        ) || (|Y190Zc      [29:24])) begin
                                            Y196Zc                 = 'b1;
                                        end    
                                            A245Yc = Y110Zc   ;
										    Y145Zc           = 1'b0;
										    Y125Zc           = 'd0;
										    Y127Zc             = 'd0;	
										    Y130Zc       = 'd0;
										    Y131Zc       = 'd0;
										    Y175Zc       = 'd0;
										    Y176Zc       = 'd0;
										    Y177Zc       = 'd0;
										    Y178Zc       = 'd0;
										    Y179Zc       = 'd0;
										    Y180Zc       = 'd0;
										    Y181Zc       = 'd0;
										    Y182Zc       = 'd0;
										    Y183Zc       = 'd0;
										    Y184Zc       = 'd0;
										    Y185Zc       = 'd0;
										    Y186Zc       = 'd0;
										    Y187Zc       = 'd0;
										    Y188Zc       = 'd0;                                       
                                    end                              
                                    else
                                        A245Yc = Y113Zc;
                            end            
			default		: 	begin 
									A245Yc = {A234Yc       {1'b0}};
								end
		endcase
	end
    assign Y192Zc               = (A244Yc == Y113Zc);
    assign Y134Zc               = Y132Zc * Y133Zc;        
    always @( * )
    begin      
        Y138Zc              =Y137Zc      ;
        Y136Zc              =Y135Zc      ; 
        Y191Zc              =28'b0; 
      case(({8'b0, Y157Zc          } | Y124Zc     [9:0]) & {10{Y158Zc           || Y192Zc           }})
        10'd0:  begin
                    Y132Zc = {14'b0, Y151Zc         [15:6]};
                    Y133Zc = Y153Zc      [13:0];
                    Y136Zc            = Y134Zc;
                end 
        10'd1:  begin
                    Y132Zc = Y135Zc      [23:0];
                    Y133Zc = Y154Zc      [13:0];
                    Y138Zc            = Y134Zc;
                end
        10'd2:  begin 
                    Y132Zc = {14'b0, Y152Zc         [15:6]};
                    Y133Zc = Y153Zc      [13:0];        
                    Y136Zc            = Y134Zc;
                end
        10'd3:  begin 
                    Y132Zc = Y135Zc      [23:0];
                    Y133Zc = Y154Zc      [13:0];
                    Y136Zc            = Y134Zc;
                end         
        10'd4:  begin 
                    Y132Zc = {9'b0, Y128Zc };
                    Y133Zc = Y128Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end   
        10'd5:  begin 
                    Y132Zc = {9'b0, Y129Zc };
                    Y133Zc = Y129Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd6:  begin 
                    Y132Zc = {9'b0, Y161Zc };
                    Y133Zc = Y161Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd7:  begin 
                    Y132Zc = {9'b0, Y162Zc };
                    Y133Zc = Y162Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd8:  begin 
                    Y132Zc = {9'b0, Y163Zc };
                    Y133Zc = Y163Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd9:  begin 
                    Y132Zc = {9'b0, Y164Zc };
                    Y133Zc = Y164Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd10:  begin 
                    Y132Zc = {9'b0, Y165Zc };
                    Y133Zc = Y165Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd11:  begin 
                    Y132Zc = {9'b0, Y166Zc };
                    Y133Zc = Y166Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd12:  begin 
                    Y132Zc = {9'b0, Y167Zc };
                    Y133Zc = Y167Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd13:  begin 
                    Y132Zc = {9'b0, Y168Zc };
                    Y133Zc = Y168Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd14:  begin 
                    Y132Zc = {9'b0, Y169Zc };
                    Y133Zc = Y169Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd15:  begin 
                    Y132Zc = {9'b0, Y170Zc };
                    Y133Zc = Y170Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd16:  begin 
                    Y132Zc = {9'b0, Y171Zc };
                    Y133Zc = Y171Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd17:  begin 
                    Y132Zc = {9'b0, Y172Zc };
                    Y133Zc = Y172Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd18:  begin 
                    Y132Zc = {9'b0, Y173Zc };
                    Y133Zc = Y173Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        10'd19:  begin 
                    Y132Zc = {9'b0, Y174Zc };
                    Y133Zc = Y174Zc [13:0];
                    Y191Zc         = Y134Zc[27:0];
                end 
        default:begin
                    Y132Zc = {(Y094Zc         +9){1'b0}};
                    Y133Zc = {(Y094Zc         -1){1'b0}};              
                end
      endcase 
    end      
    always @(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            Y135Zc          <=  {38{1'b0}};
            Y137Zc          <=  {38{1'b0}}; 
            Y190Zc          <=  {30{1'b0}};            
        end
        else if(Y158Zc           || Y192Zc           ) begin
            Y135Zc          <=  Y136Zc           ;
            Y137Zc          <=  Y138Zc           ;                    
            Y190Zc          <=  Y190Zc       + Y191Zc        ;
        end        
        else begin
            Y135Zc          <= Y135Zc      ;
            Y137Zc          <= Y137Zc      ;           
            Y190Zc          <= 30'b0;            
        end        
    end 
    always @(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            Y197Zc              <=  {3{1'b1}};
        end else if (!Y146Zc  ) begin
            Y197Zc              <=  {3{1'b1}};
        end else if(A212Yc         ) begin
            Y197Zc          <=  Y197Zc      - 1'b1;
        end        
        else begin
            Y197Zc          <=  Y197Zc     ;
        end        
    end 
    always @(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            Y199Zc              <=  10'b0;
        end else if (!Y146Zc  ) begin
            Y199Zc              <=  10'b0;
        end        
        else if(A212Yc          && (Y197Zc     ==3'h0)) begin
            Y199Zc              <=  Y126Zc       [Y094Zc         -1:3] + 1'b1;
        end
        else begin
            Y199Zc              <=  Y199Zc           ;
        end        
    end 
    assign  Y146Zc       = Y148Zc      || Y149Zc    ;
    assign  A043Yc              = Y199Zc           ;
    assign  Y159Zc              = Y148Zc      && Y193Zc            ;
    assign  Y160Zc              = Y195Zc           ;
    assign  A220Yc      = Y144Zc     ;
    assign  Y155Zc       = Y137Zc      ;
    assign  Y156Zc       = Y135Zc      ;
endmodule
module osr_trng_Y200Zc      # (
	parameter	Y094Zc         	    =	15		,          
	parameter	Y095Zc             	=	32		,
	parameter	Y096Zc      			=	1024	,
	parameter	Y097Zc      			=	624	
)(
	input   wire									i_clk					,
	input	wire									i_rstn				    ,
	input	wire									Y098Zc  				, 
	input	wire									Y099Zc   				, 
	input	wire									A067Yc    			    , 
	input	wire    [Y095Zc             -1:0]	    A211Yc        		    ,
	input	wire    								A212Yc         	        , 
	input	wire	[9:0]							Y100Zc  				,
	input	wire	[15:0]							Y101Zc                  ,
	input	wire									A721Yc   			    ,
    input   wire    [2*(Y094Zc         -1)+9:0]     Y102Zc                  ,
    input   wire    [2*(Y094Zc         -1)+9:0]     Y103Zc                  ,
    output  wire    [13:0]                          Y104Zc                  ,
    output  wire    [13:0]                          Y105Zc                  ,
    output  wire    [1:0]                           Y106Zc                  ,    
    output  wire                                    Y107Zc                  ,
	output	wire    								Y108Zc                  ,
	output	wire    								Y109Zc                  ,
	output	wire									A220Yc     			    ,
	output	wire									A722Yc     			    ,  	
	output	wire    								Y201Zc                  ,   
	output	wire    								A229Yc                      
);
	wire										Y202Zc         ;
	wire										Y203Zc          ;
	wire										Y204Zc     ;
	wire	[Y095Zc             -1:0]	        Y205Zc       ;
	wire	[Y095Zc             -1:0]	        Y206Zc        ;
	wire										Y207Zc        ;
	wire										Y208Zc         ;
	wire	[Y095Zc             -1:0]	        Y209Zc           ;
	wire										Y210Zc            ;
	wire										Y211Zc           ;
	wire										Y212Zc        ;
    wire                                        Y213Zc       ;    
	wire	[15:0]							    Y214Zc           ;
	wire	[14:0]							    Y215Zc             ;
	wire										A642Yc     ;
osr_trng_Y093Zc                       # (
					.Y095Zc             	(Y095Zc             		),
                    .Y094Zc                 (Y094Zc                     ),
					.Y096Zc      			(Y096Zc      				),
					.Y097Zc      			(Y097Zc      				)
	) Y216Zc (
					.i_clk					(i_clk						),
					.i_rstn					(i_rstn						),
					.Y098Zc                 (Y211Zc           		    ),
					.Y099Zc                 (Y212Zc        		        ),
					.A211Yc        		    (Y209Zc           			),
					.A212Yc         		(Y210Zc            			),
					.Y100Zc  				(Y215Zc             		),
					.Y101Zc                 (Y214Zc           		    ),
                    .Y102Zc                 (Y102Zc                     ),
                    .Y103Zc                 (Y103Zc                     ),
                    .Y104Zc                 (Y104Zc                     ),
                    .Y105Zc                 (Y105Zc                     ),   
                    .Y106Zc                 (Y106Zc                     ),     
                    .Y107Zc                 (Y107Zc                     ),
					.Y108Zc                 (Y202Zc         			),
					.Y109Zc                 (Y203Zc          			),
					.A220Yc     			(Y204Zc     				)
	);
osr_trng_Y217Zc   # (
					.Y095Zc             	(Y095Zc             		),
					.Y218Zc        		    ('d33					    ),
					.Y219Zc         		('d9					    ),
					.Y220Zc             	('d5					    )
	) Y221Zc         ( 
					.i_clk					(i_clk						),
					.i_rstn					(i_rstn						),
					.A139Yc  				(Y213Zc                     ),
					.A721Yc   				(A721Yc   					),
					.Y222Zc     			(Y202Zc         		    ),
					.Y223Zc      			(Y203Zc                     ),
					.Y224Zc        		    (Y206Zc                     ),
					.Y225Zc         		(Y205Zc       		        ),
					.Y226Zc         		(Y207Zc        	            ),
					.Y227Zc       			(Y208Zc         	        ),
					.A796Yc     			(A642Yc     				),
					.A722Yc     			(A722Yc     				),
					.A723Yc     			(Y201Zc         			),
					.Y228Zc                 (A229Yc          			)
	)	;
	assign  Y209Zc            	= A642Yc      ? Y205Zc        	: A211Yc        ;
	assign  Y210Zc            	= A642Yc      ? Y207Zc        	: A212Yc         ;
	assign  Y211Zc           	= A642Yc      ? Y208Zc         	: Y098Zc  ;
	assign  Y212Zc        	    = A642Yc      ? Y208Zc         	: Y099Zc   ;
    assign  Y213Zc              = Y098Zc   || Y099Zc   ;
	assign  Y214Zc           	= (A642Yc      || !A067Yc    )? 16'd624			: Y101Zc      ;
	assign  Y215Zc             	= (A642Yc      || !A067Yc    )? 15'd1024		: {Y100Zc  ,5'b0};
	assign  Y108Zc              = A642Yc      ? 1'b0 : Y202Zc         ;
	assign  Y109Zc              = A642Yc      ? 1'b0 : Y203Zc          ;
	assign  A220Yc              = A642Yc      ? 1'b0 : Y204Zc     ;
endmodule
module osr_trng_Y217Zc   # (
	parameter	Y095Zc             	=	32	,
	parameter	Y218Zc        		=	32	, 
	parameter	Y219Zc         		=	9	, 
	parameter	Y220Zc             	=	8	  
)(
	input	wire								i_clk					,
	input	wire								i_rstn				,
    input   wire                                A139Yc            ,
	input	wire								A721Yc   			,
	input	wire								Y222Zc     			,
	input	wire								Y223Zc      			,
	output	wire    [Y095Zc             -1:0]	Y224Zc        		,
	output	wire    [Y095Zc             -1:0]	Y225Zc         		,
	output	wire								Y226Zc         	, 
	output	wire								Y227Zc       		,
	output  wire								A796Yc     			,
	output	wire								A722Yc     			,
	output	wire								A723Yc              ,
	output	wire								Y228Zc      
);
	localparam	A234Yc       	=	5			;
	localparam	Y110Zc   		=	5'b00001	;
	localparam	Y229Zc  			=	5'b00010	;
	localparam	Y230Zc  			=	5'b00100	;
	localparam	Y231Zc  			=	5'b01000	;
	localparam	Y232Zc				=	5'b10000	;
	localparam	Y233Zc    		=	3			; 
	reg	[A234Yc       -1:0]			A244Yc,A245Yc;
	reg	[Y233Zc    -1:0]				Y234Zc     ,Y235Zc          ;
	reg	[Y233Zc    -1:0]				Y236Zc      ,Y237Zc           ;
	reg	[Y219Zc         -1:0]		Y238Zc   ,Y239Zc        ;
	reg	[Y220Zc             -1:0]	Y240Zc       ,Y241Zc            ;
	reg										Y242Zc,Y243Zc     ;
	reg										Y244Zc      ,Y245Zc           ;
	reg	[Y095Zc             -1:0]	Y246Zc        ,Y247Zc             ;
	reg	[Y095Zc             -1:0]	Y248Zc         ,Y249Zc              ;
	reg										Y250Zc         ,Y251Zc              ;
	reg										Y252Zc     ,Y253Zc          ;
	reg										Y254Zc     ,Y255Zc          ;
	reg										Y256Zc      ,Y257Zc           ;
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
			A244Yc 				<=  Y110Zc       ;
			Y234Zc     		<=  'd0          ;
			Y236Zc      		<=  'd0          ;
			Y238Zc   		<=  'd0          ;
			Y240Zc       	<=  'd0          ;
			Y242Zc			<=  'b0          ;
			Y244Zc      	<=  'b0          ;
			Y246Zc         <=  'd0          ;
			Y248Zc          <=  'd0          ;
			Y250Zc         <=  'b0          ;
			Y254Zc     		<=  'b0          ;
			Y256Zc      		<=  'b0          ;
			Y252Zc     		<=  'b0          ;
		end
		else begin
			A244Yc			<=  A139Yc   ? A245Yc                 : Y110Zc     ;
			Y234Zc     		<=  A139Yc   ? Y235Zc                 : 'd0        ;
			Y236Zc      	<=  A139Yc   ? Y237Zc                 : 'd0        ;
			Y238Zc   		<=  A139Yc   ? Y239Zc                 : 'd0        ;
			Y240Zc       	<=  A139Yc   ? Y241Zc                 : 'd0        ;
			Y242Zc			<=  A139Yc   ? Y243Zc                 : 'b0        ;
			Y244Zc      	<=  A139Yc   ? Y245Zc                 : 'b0        ;
			Y246Zc        	<=  A139Yc   ? Y247Zc                 : 'd0        ;
			Y248Zc         	<=  A139Yc   ? Y249Zc                  : 'd0        ;
			Y250Zc         <=  A139Yc   ? Y251Zc                 : 'b0        ;
			Y254Zc     		<=  A139Yc   ? Y255Zc                 : 'b0        ;
			Y256Zc          <=  A139Yc   ? Y257Zc                  : 'b0        ;
			Y252Zc     		<=  A139Yc   ? Y253Zc                 : 'b0        ;
		end
	end
	always@(*)
	begin
		A245Yc						= A244Yc				;
		Y235Zc          		= Y234Zc     		;
		Y237Zc           		= Y236Zc      		;
		Y239Zc        			= Y238Zc   			;
		Y241Zc            	= Y240Zc       	;
		Y243Zc     				= Y242Zc				;
		Y245Zc           		= Y244Zc      		;
		Y247Zc             	= Y246Zc        	;
		Y249Zc              	= Y248Zc         	;
		Y251Zc              	= Y250Zc         	;
		Y255Zc          		= Y254Zc     		;
		Y257Zc           		= Y256Zc      		;
		Y253Zc          		= Y252Zc     		;
		case(A244Yc)
			Y110Zc    	:  begin
									if(A721Yc   ) begin
										A245Yc 						= Y229Zc  ;
										Y243Zc      			= 1'b1;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {(Y095Zc             >>3){8'h03}};
										Y249Zc               	= {(Y095Zc             >>3){8'h03}};
									end
									Y253Zc           		= 'b0;
									Y255Zc           		= 'b0;
									Y257Zc            		= 'b0;
									Y235Zc           		= 'd0;
									Y237Zc            		= 'd0;
									Y239Zc        	  		= 'd0;
									Y241Zc            	= 'd0;
								end
			Y229Zc  	:	begin
									Y241Zc             	= Y240Zc        + 1'b1;
									Y251Zc               = 1'b0;
									Y245Zc            	= 1'b1;
									if(Y238Zc   ==Y218Zc         +{Y219Zc         {1'b0}}) begin
										A245Yc 						= Y230Zc  ;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {(Y095Zc             >>3){8'hf1}};
										Y249Zc               	= {(Y095Zc             >>3){8'hf1}};
										Y239Zc         		= 'd0;
										Y241Zc            	= 'd0;
										Y235Zc          [0] 	= Y222Zc     ;
										Y237Zc           [0] 	= Y223Zc      ;
									end
									else if(Y240Zc       ==((Y095Zc             >>1)+ 'd1)) begin
											Y241Zc            	= 'd0;
											Y239Zc        			= Y238Zc    + 1'b1;
											end
									else if((Y240Zc       =='d1)) begin
										Y251Zc               = 1'b1;
									end
							end
			Y230Zc  	:	begin
									Y241Zc             	= Y240Zc        + 1'b1;
									Y251Zc               = 1'b0;
									Y245Zc            	= 1'b1;
									if(Y238Zc   ==Y218Zc         +{Y219Zc         {1'b0}}) begin
										A245Yc 						= Y231Zc  ;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {(Y095Zc             >>3){8'h5a}};
										Y249Zc               	= {(Y095Zc             >>3){8'h5a}};
										Y239Zc         		= 'd0;
										Y241Zc            	= 'd0;
										Y235Zc          [1] 	= Y222Zc     ;
										Y237Zc           [1] 	= Y223Zc      ;
									end
									else if(Y240Zc       ==((Y095Zc             >>1)+ 'd1)) begin
											Y241Zc            	= 'd0;
											Y239Zc        			= Y238Zc    + 1'b1;
											end
									else if((Y240Zc       =='d1)) begin
										Y251Zc               = 1'b1;
									end
							end
			Y231Zc  	:	begin
									Y241Zc             	= Y240Zc        + 1'b1;
									Y251Zc               = 1'b0;
									Y245Zc            	= 1'b1;
									if(Y238Zc   ==Y218Zc         +{Y219Zc         {1'b0}}) begin
										A245Yc 						= Y232Zc;
										Y245Zc            	= 1'b0;
										Y247Zc              		= {(Y095Zc             >>3){8'h00}};
										Y249Zc               		= {(Y095Zc             >>3){8'h00}};
										Y239Zc         		= 'd0;
										Y241Zc            	= 'd0;
										Y235Zc          [2] 	= !Y222Zc     ;
										Y237Zc           [2] 	= !Y223Zc      ;
									end
									else if(Y240Zc       ==((Y095Zc             >>1)+ 'd1)) begin
											Y241Zc            	= 'd0;
											Y239Zc        			= Y238Zc    + 1'b1;
											end
									else if((Y240Zc       =='d1)) begin
										Y251Zc               = 1'b1;
										Y247Zc              	= {(Y095Zc             >>2){Y238Zc   [3:0]}};
										Y249Zc               	= {(Y095Zc             ){Y238Zc   [0]}};
									end
							end
			Y232Zc		:	begin
								A245Yc 					= Y110Zc   ;
								Y253Zc           	= 'b1;
								Y255Zc           	= !(& Y234Zc     ); 
								Y257Zc            	= !(& Y236Zc      ); 
								Y243Zc      		= 1'b0;
							end
			default	:	A245Yc = Y110Zc   ;
		endcase
	end	
	assign Y224Zc         	= Y246Zc        ;
	assign Y225Zc          	= Y248Zc         ;
	assign Y226Zc         	= Y250Zc         ;
	assign Y227Zc       		= Y244Zc      ;
	assign A796Yc     			= Y242Zc;
	assign A722Yc     			= Y252Zc     ;
	assign A723Yc     			= Y254Zc     ;
	assign Y228Zc      			= Y256Zc      ;
endmodule
module osr_trng_Y258Zc          # (
	parameter	Y094Zc         	        =	15	,          
	parameter	Y095Zc             	    =	32	,
	parameter	Y096Zc      			=	512	,
	parameter	Y097Zc      			=	77	
)(
	input	wire									i_clk				,
	input	wire									i_rstn				,
	input	wire									Y259Zc              , 
	input	wire									Y149Zc    			, 
	input	wire									A067Yc    			,    
	input	wire    [Y095Zc             -1:0]	    A211Yc        		,
	input	wire									A212Yc         	    , 
	input	wire	[9:0]							Y100Zc  			,
	input	wire    [11:0]							Y150Zc         		,
	input	wire    [23:0]							A034Yc        		,
	input   wire								    A721Yc              ,
	input	wire	[31:0]							A031Yc              ,
    input   wire    [13:0]                          Y153Zc              ,
    input   wire    [13:0]                          Y154Zc              ,
    output  wire    [2*(Y094Zc         -1)+9:0]     Y155Zc              ,
    output  wire    [2*(Y094Zc         -1)+9:0]     Y156Zc              ,
    input   wire    [1:0]                           Y157Zc              ,     
    input   wire                                    Y158Zc              ,     
    output  wire    [9:0]                           A043Yc              ,
	output  wire									Y159Zc            	,
	output  wire									Y160Zc           	,
	output  wire									A220Yc     			,
	output  wire									A722Yc     			,	
	output  wire									Y260Zc              ,   
	output  wire									A232Yc                  
);
	wire										Y261Zc            ;
	wire										Y262Zc           ;
	wire										Y204Zc     ;
	wire	[Y095Zc             -1:0]	        Y205Zc       ;
	wire	[Y095Zc             -1:0]	        Y206Zc        ;
	wire										Y207Zc        ;
	wire										Y208Zc         ;
	wire	[Y095Zc             -1:0]	        Y209Zc           ;
	wire										Y210Zc            ;
	wire										Y263Zc               ;
	wire										Y264Zc         ;
    wire                                        Y213Zc       ;
	wire	[11:0]							    Y265Zc               ;
	wire	[23:0]							    Y266Zc         ;
	wire	[12:0]							    Y215Zc             ;
	wire	[15:0]							    Y267Zc          ;
	wire	[15:0]							    Y268Zc          ;
	wire										A642Yc     ;
osr_trng_Y147Zc                           # (
					.Y095Zc             	(Y095Zc             			),
                    .Y094Zc                 (Y094Zc                         ),                    
					.Y096Zc      			(Y096Zc      					),
					.Y097Zc      			(Y097Zc      					)
	) Y216Zc (
					.i_clk					(i_clk							),
					.i_rstn					(i_rstn							),
					.Y148Zc                 (Y263Zc               		    ),
					.Y149Zc                 (Y264Zc         		        ),
					.A211Yc        		    (Y209Zc           		        ),
					.A212Yc         		(Y210Zc            		        ),
					.Y100Zc  				(Y215Zc             		    ),
					.Y150Zc         		(Y265Zc               		    ),
					.A034Yc        			(Y266Zc             		    ),
					.A043Yc                 (A043Yc                         ),
                    .Y151Zc         		(Y267Zc                         ),
                    .Y152Zc                 (Y268Zc                         ),
                    .Y153Zc                 (Y153Zc                         ),
                    .Y154Zc                 (Y154Zc                         ),
                    .Y155Zc                 (Y155Zc                         ),
                    .Y156Zc                 (Y156Zc                         ),
                    .Y157Zc                 (Y157Zc                         ),   
                    .Y158Zc                 (Y158Zc                         ),
					.Y159Zc            		(Y261Zc                         ),
					.Y160Zc           		(Y262Zc                         ),
					.A220Yc     			(Y204Zc     				    )
	);
osr_trng_Y217Zc   # (
					.Y095Zc             	(Y095Zc             			),
					.Y218Zc        		    ('d128							),
					.Y219Zc         		('d9							),
					.Y220Zc             	('d5							)
	) Y221Zc         ( 
					.i_clk					(i_clk							),
					.i_rstn					(i_rstn							),
					.A139Yc  				(Y213Zc                         ),
   				    .A721Yc   				(A721Yc   						),
					.Y222Zc     	        (Y261Zc            			    ),
					.Y223Zc      	        (Y262Zc           			    ),
					.Y224Zc        		    (Y205Zc       	                ),
					.Y225Zc         		(Y206Zc                         ),
					.Y226Zc         		(Y207Zc        	                ),
					.Y227Zc       			(Y208Zc                         ),
					.A796Yc     			(A642Yc     					),
					.A722Yc     			(A722Yc     					),
					.A723Yc     			(Y260Zc                         ),
					.Y228Zc      			(A232Yc                         )
	)	;
    assign Y209Zc               = A642Yc      ? Y205Zc        	: A211Yc        ;
    assign Y210Zc               = A642Yc      ? Y207Zc        	: A212Yc         ;
    assign Y263Zc               = A642Yc      ? Y208Zc         	: Y259Zc          ;
    assign Y264Zc               = A642Yc      ? Y208Zc         	: Y149Zc    ;
    assign Y213Zc               = Y259Zc           || Y149Zc    ;
    assign Y265Zc               = (A642Yc      || !A067Yc    ) ? 16'd77	: Y150Zc         ;
    assign Y266Zc               = (A642Yc      || !A067Yc    ) ? 24'd17801: A034Yc        ;
    assign Y215Zc               = (A642Yc      || !A067Yc    ) ? 13'd512	: {Y100Zc  ,3'b0};
	assign Y267Zc              = (A642Yc      || !A067Yc    )? 16'hFFC0		: A031Yc       [15:0];
	assign Y268Zc              = (A642Yc      || !A067Yc    )? 16'h0		: A031Yc       [31:16];
	assign Y159Zc               = A642Yc      ? 1'b0 : Y261Zc            ;
	assign Y160Zc               = A642Yc      ? 1'b0 : Y262Zc           ;
	assign A220Yc               = A642Yc      ? 1'b0 : Y204Zc     ;
endmodule
module osr_trng_Y269Zc   # (
	parameter	Y270Zc             	=	32	,
	parameter	Y219Zc         		=	8	, 
	parameter	Y218Zc        		=	2	, 
	parameter	Y220Zc             	=	8	  
)(
	input	wire									i_clk					,
	input	wire    								i_rstn				,
    input   wire                                    A139Yc            ,
	input	wire									A721Yc   			,
	input	wire									A143Yc    			,
	output	wire    [Y270Zc             -1:0]	    Y224Zc        		,
	output	wire								    Y226Zc         	, 
	output	wire								    Y271Zc      		,
	output	wire								A796Yc     			,
	output	wire								A722Yc     			,
	output	wire									A723Yc     
);
	localparam	A234Yc       	=	5			;
	localparam	Y110Zc   		=	5'b00001	;
	localparam	Y229Zc  			=	5'b00010	;
	localparam	Y230Zc  			=	5'b00100	;
	localparam	Y231Zc  			=	5'b01000	;
	localparam	Y232Zc				=	5'b10000	;
	localparam	Y233Zc    		=	3			; 
	reg	[A234Yc       -1:0]			A244Yc,A245Yc;
	reg	[Y233Zc    -1:0]				Y234Zc     ,Y235Zc          ;
	reg	[Y219Zc         -1:0]		Y238Zc   ,Y239Zc        ;
	reg	[Y220Zc             -1:0]	Y240Zc       ,Y241Zc            ;
	reg										Y242Zc,Y243Zc     ;
	reg										Y244Zc      ,Y245Zc           ;
	reg	[Y270Zc             -1:0]	Y246Zc        ,Y247Zc             ;
	reg										Y250Zc         ,Y251Zc              ;
	reg										Y252Zc     ,Y253Zc          ;
	reg										Y254Zc     ,Y255Zc          ;
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
			A244Yc 				<=  Y110Zc    ;
			Y234Zc     		<=  'd0       ;
			Y238Zc   		<=  'd0       ;
			Y240Zc       	<=  'd0       ;
			Y242Zc			<=  'b0       ;
			Y244Zc      	<=  'b0       ;
			Y246Zc          <=  'd0       ;
			Y250Zc          <=  'b0       ;
			Y254Zc     		<=  'b0       ;
			Y252Zc     		<=  'b0       ;
		end
		else begin
			A244Yc				<=  A139Yc   ? A245Yc                 : Y110Zc    ;
			Y234Zc     		<=  A139Yc   ? Y235Zc                 : 'd0       ;
			Y238Zc   		<=  A139Yc   ? Y239Zc                 : 'd0       ;
			Y240Zc       	<=  A139Yc   ? Y241Zc                 : 'd0       ;
			Y242Zc			<=  A139Yc   ? Y243Zc                 : 'b0       ;
			Y244Zc      	<=  A139Yc   ? Y245Zc                 : 'b0       ;
			Y246Zc        	<=  A139Yc   ? Y247Zc                 : 'd0       ;
			Y250Zc         <=  A139Yc   ? Y251Zc                 : 'b0       ;
			Y254Zc     		<=  A139Yc   ? Y255Zc                 : 'b0       ;
			Y252Zc     		<=  A139Yc   ? Y253Zc                 : 'b0       ;
		end
	end
	always@(*)
	begin
		A245Yc						= A244Yc				;
		Y235Zc          		= Y234Zc     		;
		Y239Zc        			= Y238Zc   			;
		Y241Zc            	= Y240Zc       	;
		Y243Zc     				= Y242Zc				;
		Y245Zc           		= Y244Zc      		;
		Y247Zc             	= Y246Zc        	;
		Y251Zc              	= Y250Zc         	;
		Y255Zc          		= Y254Zc     		;
		Y253Zc          		= Y252Zc     		;
		case(A244Yc)
			Y110Zc    	:  begin
									if(A721Yc   ) begin
										A245Yc 						= Y229Zc  ;
										Y243Zc      			= 1'b1;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {Y270Zc             {1'b0}};
									end
									Y253Zc           		= 'b0;
									Y255Zc           		= 'b0;
									Y235Zc           		= 'd0;
									Y239Zc        	  		= 'd0;
									Y241Zc            	= 'd0;
								end
			Y229Zc  	:	begin
									Y241Zc             	= Y240Zc        + 1'b1;
									Y251Zc               = 1'b0;
									Y245Zc            	= 1'b1;
									if(Y238Zc   ==(Y218Zc         +{Y219Zc         {1'b0}})) begin
										A245Yc 						= Y230Zc  ;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {Y270Zc             {1'b1}};
										Y239Zc         		= 'd0;
										Y241Zc            	= 'd0;
										Y235Zc          [0] 	= A143Yc    ;
									end
									else if(Y240Zc       ==((Y270Zc             >>1)+ 'd1)) begin
											Y241Zc            	= 'd0;
											Y239Zc        			= Y238Zc    + 1'b1;
											end
									else if((Y240Zc       =='d1)) begin
										Y251Zc               = 1'b1;
									end
								end
			Y230Zc  	:	begin
									Y241Zc             	= Y240Zc        + 1'b1;
									Y251Zc               = 1'b0;
									Y245Zc            	= 1'b1;
									if(Y238Zc   ==(Y218Zc         +{Y219Zc         {1'b0}})) begin
										A245Yc 						= Y231Zc  ;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {Y270Zc             >>3{8'h5a}};
										Y239Zc         		= 'd0;
										Y241Zc            	= 'd0;
										Y235Zc          [1] 	= A143Yc    ;
									end
									else if(Y240Zc       ==((Y270Zc             >>1)+ 'd1)) begin
											Y241Zc            	= 'd0;
											Y239Zc        			= Y238Zc    + 1'b1;
											end
									else if((Y240Zc       =='d1)) begin
										Y251Zc               = 1'b1;
									end
							end
			Y231Zc  	:	begin
									Y241Zc             	= Y240Zc        + 1'b1;
									Y251Zc               = 1'b0;
									Y245Zc            	= 1'b1;
									if(Y238Zc   ==(Y218Zc         +{Y219Zc         {1'b0}})) begin
										A245Yc 						= Y232Zc;
										Y245Zc            	= 1'b0;
										Y247Zc              	= {Y270Zc             {1'b0}};
										Y239Zc         		= 'd0;
										Y241Zc            	= 'd0;
										Y235Zc          [2] 	= !A143Yc    ;
									end
									else if(Y240Zc       ==((Y270Zc             >>1)+ 'd1)) begin
											Y241Zc            	= 'd0;
											Y239Zc        			= Y238Zc    + 1'b1;
											end
									else if((Y240Zc       =='d1)) begin
										Y251Zc               = 1'b1;
									end
							end
			Y232Zc			:	begin
									A245Yc 					= Y110Zc   ;
									Y253Zc           	= 'b1;
									Y255Zc           	= !(& Y234Zc     ); 
									Y243Zc      		= 1'b0;
								end
			default		:	A245Yc = Y110Zc   ;
		endcase
	end	
	assign Y224Zc         	= Y246Zc        ;
	assign Y226Zc         	= Y250Zc         ;
	assign Y271Zc      		= Y244Zc      ;
	assign A796Yc     			= Y242Zc;
	assign A722Yc     			= Y252Zc     ;
	assign A723Yc     			= Y254Zc     ;
endmodule
module osr_trng_Y272Zc  # (
	parameter	Y270Zc             	=	32	,
	parameter	A009Yc      			=	41	
)(
	input	wire									i_clk					,
	input	wire									i_rstn				,
	input	wire									A139Yc  				, 
	input	wire    [Y270Zc             -1:0]	    A211Yc        		,
	input	wire									A212Yc         	, 
	input	wire    [5:0]							Y273Zc  				,
	input	wire									A721Yc   			,
	output	wire									Y274Zc     			,
	output	wire									A220Yc     			,
	output	wire									A722Yc     			,	
	output	wire									A723Yc     				
);
	wire										Y275Zc     ;
	wire										Y204Zc     ;
	wire	[Y270Zc             -1:0]	Y276Zc         ;
	wire										Y277Zc          ;
	wire										Y278Zc           ;
	wire	[Y270Zc             -1:0]	Y279Zc     ;
	wire										Y280Zc      ;
	wire										Y281Zc       ;
	wire	[5:0]								Y282Zc       ;
	wire										A642Yc     ;
osr_trng_Y283Zc                # (
					.Y270Zc             	(Y270Zc             	),
					.A009Yc      			(A009Yc      			)
	) Y284Zc (
					.i_clk					(i_clk					),
					.i_rstn					(i_rstn					),
					.A139Yc  				(Y281Zc       			),
					.A211Yc        		(Y279Zc     			),
					.A212Yc         		(Y280Zc      			),
					.Y273Zc  				(Y282Zc       			),
					.Y274Zc     			(Y275Zc     			),
					.A220Yc     			(Y204Zc     			)
	);
osr_trng_Y269Zc   # (
					.Y270Zc             	(Y270Zc             	),
					.Y219Zc         		('d8						),
					.Y218Zc        		('d2						),
					.Y220Zc             	('d8						)
	) Y285Zc     ( 
					.i_clk					(i_clk					),
					.i_rstn					(i_rstn					),
					.A139Yc  				(A139Yc              ),
					.A721Yc   				(A721Yc   				),
					.A143Yc    				(Y275Zc     			),
					.Y224Zc        		(Y276Zc         		),
					.Y226Zc         		(Y277Zc          		),
					.Y271Zc      			(Y278Zc           	),
					.A796Yc     			(A642Yc     			),
					.A722Yc     			(A722Yc     			),
					.A723Yc     			(A723Yc     			)
	)	;
	assign  Y279Zc      	= A642Yc      ? Y276Zc          		: A211Yc        ;
	assign  Y280Zc      	= A642Yc      ? Y277Zc          		: A212Yc         ;
	assign  Y282Zc       	= A642Yc      ? 'd41						: Y273Zc  ;
	assign  Y281Zc       	= A642Yc      ? Y278Zc           	: A139Yc  ;
	assign  Y274Zc      = A642Yc      ? 1'b0 : Y275Zc     ;
	assign  A220Yc      = A642Yc      ? 1'b0 : Y204Zc     ;
endmodule
module osr_trng_Y283Zc                #(
	parameter	Y270Zc             	=	32	,	
	parameter	A009Yc      			=	41		
)(
	input	wire										i_clk					,
	input	wire										i_rstn				,
	input	wire										A139Yc  				, 
	input	wire	[Y270Zc             -1:0]	A211Yc        		,
	input	wire										A212Yc         	, 
	input	wire	[5:0]								Y273Zc  				, 
	output	wire									Y274Zc     			, 
	output	wire									A220Yc     
);
	localparam	A234Yc       	=	4			;
	localparam	A235Yc 			=	4'b0001	;
	localparam	Y110Zc   		=	4'b0010	;
	localparam	Y111Zc 			=	4'b0100	;
	localparam	Y112Zc 			=	4'b1000	;
	reg	[Y270Zc             -1:0]	Y118Zc     ,Y119Zc          ;
	reg	[7:0]								Y124Zc     ,Y125Zc          ;
	reg	[A234Yc       -1:0]			A244Yc,A245Yc;
	reg	[7:0]								Y128Zc ,Y129Zc ,Y130Zc      ,Y131Zc      ;
	reg										Y286Zc     ,Y287Zc          ;
	reg										Y144Zc     ,Y145Zc          ;
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
			A244Yc 			<=  A235Yc ;
			Y118Zc      <=  'd0;
			Y124Zc      <=  'd0;
			Y128Zc  		<=  'd0;
			Y129Zc  		<=  'd0;
			Y286Zc      <=  'b0;
			Y144Zc      <=  'b0;
		end
		else begin 
			A244Yc 			<=  A139Yc   ? A245Yc 				: A235Yc ;
			Y118Zc      <=  A139Yc   ? Y119Zc          	: 'd0;
			Y124Zc      <=  A139Yc   ? Y125Zc          	: 'd0;
			Y128Zc  		<=  A139Yc   ? Y130Zc      		: 'd0;
			Y129Zc  		<=  A139Yc   ? Y131Zc      		: 'd0;
			Y286Zc      <=  A139Yc   ? Y287Zc          	: 'b0;
			Y144Zc      <=  A139Yc   ? Y145Zc          	: 'b0;
		end
	end
	always@(*)
	begin
		A245Yc					= A244Yc			;
		Y119Zc          	= Y118Zc     	;
		Y125Zc          	= Y124Zc     	;
		Y130Zc      		= Y128Zc 		;
		Y131Zc      		= Y129Zc 		;
		Y287Zc          	= Y286Zc     	;
		Y145Zc          	= Y144Zc     	;
		case(A244Yc)
			A235Yc 		:	A245Yc = Y110Zc   ;
			Y110Zc   	:	begin
									if (A139Yc  &&A212Yc         ) begin
										A245Yc = Y111Zc ;
										Y119Zc           = A211Yc        ;
										Y145Zc           = 1'b1;
									end
								end
			Y112Zc 		:	begin
									Y119Zc           = {Y118Zc     [Y270Zc             -5:0],4'd0};
									Y125Zc           = Y124Zc      + 1'b1;
									if ((Y124Zc      + 1'b1) == (Y270Zc              >> 2)) begin
										A245Yc = Y110Zc   ;
										Y145Zc           = 1'b0;
										Y125Zc           = 'd0;
									end
									else
										A245Yc = Y111Zc ;
								end
			Y111Zc 		:	begin
									A245Yc = Y112Zc ;
									case (Y118Zc     [Y270Zc             -1:Y270Zc             -4])
										4'h0: begin
													Y130Zc       = Y128Zc  + 8'd4;
													Y131Zc       = 8'd0;
													if ((Y128Zc  + 8'd4) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h1: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd1;
													if ((Y128Zc  + 8'd3) > (Y273Zc   +8'd0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h2: begin
													Y130Zc       = 8'd1;
													Y131Zc       = 8'd0;
													if ((Y128Zc  + 8'd2) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h3: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd2;
													if ((Y128Zc  + 8'd2) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h4: begin
													Y130Zc       = 8'd2;
													Y131Zc       = 8'd0;
													if ((Y128Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h5: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd1;
													if ((Y128Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h6: begin
													Y130Zc       = 8'd1;
													Y131Zc       = 8'd0;
													if ((Y128Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h7: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd3;
													if ((Y128Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h8: begin
													Y130Zc       = 8'd3;
													Y131Zc       = 8'd0;
													if ((Y129Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'h9: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd1;
													if ((Y129Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'hA: begin
													Y130Zc       = 8'd1;
													Y131Zc       = 8'd0;
													if ((Y129Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'hB: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd2;
													if ((Y129Zc  + 8'd1) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'hC: begin
													Y130Zc       = 8'd2;
													Y131Zc       = 8'd0;
													if ((Y129Zc  + 8'd2) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'hD: begin
													Y130Zc       = 8'd0;
													Y131Zc       = 8'd1;
													if ((Y129Zc  + 8'd2) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'hE: begin
													Y130Zc       = 8'd1;
													Y131Zc       = 8'd0;
													if ((Y129Zc  + 8'd3) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										4'hF: begin
													Y130Zc       = 8'd0;
													Y131Zc       = Y129Zc  + 8'd4;
													if ((Y129Zc  + 8'd4) > (Y273Zc   +8'b0)) begin
														Y287Zc           = 'b1;
													end
												end
										default: ;
									endcase
								end
				default: ;
			endcase
	end
	assign Y274Zc     	= Y286Zc     ;
	assign A220Yc     	= Y144Zc     ;
endmodule
module osr_trng_A204Yc              #(
    parameter   Y288Zc                  =   1       ,
    parameter   Y289Zc                  =   1       ,
    parameter   A001Yc                  =   1       ,       
    parameter   A804Yc                  =   32      ,
	parameter	Y094Zc         	        =	15		,      
    parameter   A009Yc                  =   41      ,
    parameter   A010Yc                  =   1024    ,    
    parameter   A011Yc                  =   624     ,
    parameter   A012Yc                  =   512     ,    
    parameter   A013Yc                  =   77      
)(
    input    wire                            i_clk                          ,
    input    wire                            i_rstn                         ,
    input    wire                            A206Yc                         , 
    input    wire                            A207Yc                         , 
    input    wire                            A208Yc                         , 
    input    wire                            A209Yc                         , 
    input    wire                            A210Yc                         , 
    input    wire                            A067Yc                         , 
    input    wire    [A804Yc         -1:0]   A211Yc                         ,
    input    wire                            A212Yc                         , 
    input    wire                            A213Yc                         , 
    input    wire    [ 5:0]                  A214Yc                         , 
    input    wire    [ 9:0]                  A029Yc                         ,
    input    wire    [15:0]                  A030Yc                         ,
    input    wire    [31:0]                  A031Yc                         ,
    input    wire    [ 9:0]                  A032Yc                         ,
    input    wire    [11:0]                  A033Yc                         ,
    input    wire    [23:0]                  A034Yc                         ,
    output  wire    [9:0]                    A043Yc                         ,    
    output   wire                            A215Yc                         ,
    output   wire                            A216Yc                         ,
    output   wire                            A217Yc                         ,
    output   wire                            A218Yc                         ,
    output   wire                            A219Yc                         ,
    output   wire                            A220Yc                         ,
    output   wire                            A221Yc                         ,
    input    wire                            A222Yc                         ,
    input    wire                            A223Yc                         ,
    input    wire                            A224Yc                         ,
    output   wire                            A225Yc                         ,
    output   wire                            A226Yc                         ,
    output   wire                            A227Yc                         ,
    output   wire                            A228Yc                         ,
    output   wire                            A229Yc                         ,
    output   wire                            A230Yc                         ,
    output   wire                            A231Yc                         , 
    output   wire                            A232Yc               
);
    wire    Y290Zc    ;
    wire    Y291Zc        ;
    wire    Y292Zc             ;
    wire    [13:0]                          Y293Zc          ;
    wire    [13:0]                          Y294Zc          ;
    wire    [1:0]                           Y295Zc          ;
    wire                                    Y139Zc          ;
    wire    [2*(Y094Zc         -1)+9:0]     Y296Zc          ; 
    wire    [2*(Y094Zc         -1)+9:0]     Y297Zc          ;     
osr_trng_Y272Zc  #(
        .Y270Zc                     (A804Yc                 ),
        .A009Yc                     (A009Yc                 )
    ) Y284Zc (
        .i_clk                      (i_clk                  ),
        .i_rstn                     (i_rstn                 ),
        .A139Yc                     (A206Yc                 ),
        .A211Yc                     (A211Yc                 ),
        .A212Yc                     (A212Yc                 ),
        .Y273Zc                     (A214Yc                 ),
        .A721Yc                     (A222Yc                 ),
        .Y274Zc                     (A215Yc                 ),
        .A220Yc                     (Y290Zc                 ),
        .A722Yc                     (A225Yc                 ),
        .A723Yc                     (A226Yc                 )
    );
osr_trng_Y200Zc      #(
        .Y095Zc                     (A804Yc                 ),
        .Y096Zc                     (A010Yc                 ),
        .Y097Zc                     (A011Yc                 )
    ) Y298Zc    (
        .i_clk                      (i_clk                  ),
        .i_rstn                     (i_rstn                 ),
        .Y098Zc                     (A207Yc                 ),
        .Y099Zc                     (A208Yc                 ),
        .A067Yc                     (A067Yc                 ),
        .A211Yc                     (A211Yc                 ),
        .A212Yc                     (A213Yc                 ),
        .Y100Zc                     (A029Yc                 ),
        .Y101Zc                     (A030Yc                 ),
        .A721Yc                     (A223Yc                 ),
        .Y102Zc                     (Y296Zc                 ),
        .Y103Zc                     (Y297Zc                 ),
        .Y104Zc                     (Y293Zc                 ),
        .Y105Zc                     (Y294Zc                 ),       
        .Y106Zc                     (Y295Zc                 ),          
        .Y107Zc                     (Y139Zc                 ),
        .Y108Zc                     (A216Yc                 ),
        .Y109Zc                     (A217Yc                 ),
        .A220Yc                     (Y291Zc                 ),
        .A722Yc                     (A227Yc                 ),
        .Y201Zc                     (A228Yc                 ),
        .A229Yc                     (A229Yc                 )
    );
generate
if( A001Yc         ) begin : Y299Zc       
osr_trng_Y258Zc          #(
        .Y095Zc                     (A804Yc                     ),
        .Y096Zc                     (A012Yc                     ),
        .Y097Zc                     (A013Yc                     )
    ) Y300Zc        (
        .i_clk                      (i_clk                      ),
        .i_rstn                     (i_rstn                     ),
        .Y259Zc                     (A209Yc                     ),
        .Y149Zc                     (A210Yc                     ),
        .A067Yc                     (A067Yc                     ),
        .A211Yc                     (A211Yc                     ),
        .A212Yc                     (A213Yc                     ),
        .Y100Zc                     (A032Yc                     ),
        .Y150Zc                     (A033Yc                     ),
        .A034Yc                     (A034Yc                     ),
        .A721Yc                     (A224Yc                     ),
        .A031Yc                     (A031Yc                     ),
        .Y153Zc                     (Y293Zc                     ),
        .Y154Zc                     (Y294Zc                     ),
        .Y155Zc                     (Y296Zc                     ),
        .Y156Zc                     (Y297Zc                     ),       
        .Y157Zc                     (Y295Zc                     ),           
        .Y158Zc                     (Y139Zc                     ),
        .A043Yc                     (A043Yc                     ),     
        .Y159Zc                     (A218Yc                     ),
        .Y160Zc                     (A219Yc                     ),
        .A220Yc                     (Y292Zc                     ),
        .A722Yc                     (A230Yc                     ),
        .Y260Zc                     (A231Yc                     ),
        .A232Yc                     (A232Yc                     )
    );
end
else begin:Y301Zc          
    assign  A218Yc                  =  1'b0 ;
    assign  A219Yc                  =  1'b0 ;
    assign  Y292Zc                  =  1'b0 ;
    assign  A230Yc                  =  1'b1 ;
    assign  A231Yc                  =  1'b0 ;
    assign  A232Yc                  =  1'b0 ;
    assign  A043Yc                  =  10'b0;
end
endgenerate
    assign A220Yc         = Y290Zc     || Y291Zc         || Y292Zc             ;
    assign A221Yc         = Y290Zc     ;
endmodule
module osr_trng_A803Yc      #(
    parameter           A804Yc                  =   32
)(
    input   wire                                clk
   ,input   wire                                rst_n
   ,input   wire                                A206Yc      
   ,input   wire                                A806Yc      
   ,input   wire    [A804Yc         -1:0]       A211Yc        
   ,input   wire                                A212Yc         
   ,input   wire    [5:0]                       A214Yc      
   ,input   wire                                A721Yc   
   ,output  wire                                A722Yc     
   ,output  wire                                A226Yc         
   ,output  wire                                A807Yc         
   ,output  wire                                A215Yc    
   ,output  wire                                A808Yc    
);
    wire                                        Y302Zc         ;
    wire                                        Y303Zc         ;
    wire                                        Y304Zc         ;
    wire                                        Y305Zc         ;
    reg     [1:0]                               Y252Zc     ;
    wire                                        A642Yc     ;
osr_trng_Y306Zc       #
    (    
        .Y270Zc                 (A804Yc             )
    )
    Y307Zc        
    (
        .clk                    (clk                )
       ,.rst_n                  (rst_n              )
       ,.A139Yc                 (A206Yc             )
       ,.A211Yc                 (A211Yc             )
       ,.Y308Zc                 (A212Yc             )
       ,.Y273Zc                 (A214Yc             )
       ,.Y274Zc                 (A215Yc             )
       ,.A721Yc                 (A721Yc             )
       ,.A722Yc                 (Y302Zc             )
       ,.A723Yc                 (A226Yc             )
    );
osr_trng_Y309Zc       #
    (
        .Y310Zc                 (A804Yc             )
    )
    Y311Zc        
    (
        .clk                    (clk                )
       ,.rst_n                  (rst_n              )
       ,.A139Yc                 (A806Yc             )
       ,.A211Yc                 (A211Yc             )
       ,.Y308Zc                 (A212Yc             )
       ,.Y274Zc                 (A808Yc             )
       ,.A721Yc                 (A721Yc             )
       ,.A722Yc                 (Y303Zc             )
       ,.A723Yc                 (A807Yc             )
    );
    always @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            Y252Zc     [0] <= 1'b0;
        end
        else begin
            if(A206Yc       && Y302Zc         ) begin
                Y252Zc     [0]  <=  1'b1;
            end
            else if (!A206Yc      ) begin
                Y252Zc     [0]  <=  1'b1;
            end
            else if(&Y252Zc     ) begin
                Y252Zc     [0]  <=  1'b0;
            end
            else begin
                Y252Zc     [0]  <=  Y252Zc     [0];
            end
        end
    end
    always @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            Y252Zc     [1] <= 1'b0;
        end
        else begin            
            if(A806Yc       && Y303Zc         ) begin
                Y252Zc     [1]  <=  1'b1;
            end
            else if (!A806Yc      ) begin
                Y252Zc     [1]  <=  1'b1;
            end
            else if (&Y252Zc     ) begin
                Y252Zc     [1]  <=  1'b0;
            end
            else begin
                Y252Zc     [1]  <=  Y252Zc     [1];
            end
        end
    end
    assign  A722Yc      = &Y252Zc     ;
endmodule
module osr_trng_Y306Zc       #(
    parameter                               Y270Zc              = 32
)(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire                            A139Yc  
   ,input   wire    [Y270Zc             -1:0]   A211Yc        
   ,input   wire                            Y308Zc       
   ,input   wire    [5:0]                   Y273Zc  
   ,output  wire                            Y274Zc      
   ,input   wire                            A721Yc   
   ,output  wire                            A722Yc     
   ,output  wire                            A723Yc     
);
    wire                                    Y281Zc       ;
    wire    [Y270Zc             -1:0]       Y312Zc             ;
    wire                                    Y313Zc            ;
    wire                                    Y278Zc           ;
    wire    [Y270Zc             -1:0]       Y314Zc                 ;
    wire                                    Y315Zc                ;
    wire                                    Y275Zc     ;
    wire                                    A642Yc     ;
osr_trng_Y316Zc   #(
        .Y270Zc                 (Y270Zc                 )
    )
    Y317Zc    (
        .i_clk                  (clk                    )
       ,.i_rstn                 (rst_n                  )
       ,.A139Yc                 (Y281Zc                      )
       ,.A211Yc                 (Y312Zc                 )
       ,.A212Yc                 (Y313Zc                 )
       ,.Y273Zc                 (Y273Zc                 )
       ,.Y274Zc                 (Y275Zc                 )
    );
osr_trng_Y318Zc        #(
        .Y270Zc                 (Y270Zc                 )
    )
    Y319Zc         (
        .clk                    (clk                    )
       ,.rst_n                  (rst_n                  )
       ,.A139Yc                 (A139Yc                 )
       ,.A721Yc                 (A721Yc                 )
       ,.A722Yc                 (A722Yc                 )
       ,.A723Yc                 (A723Yc                 )
       ,.A796Yc                 (A642Yc                 )
       ,.Y271Zc                 (Y278Zc                 )
       ,.Y224Zc                 (Y314Zc                 )
       ,.Y320Zc                 (Y315Zc                 )
       ,.Y222Zc                 (Y275Zc                 )
    );
    assign  Y281Zc              = A642Yc      ? Y278Zc                  : A139Yc  ;
    assign  Y312Zc              = A642Yc      ? Y314Zc                  : A211Yc        ;
    assign  Y313Zc              = A642Yc      ? Y315Zc                  : Y308Zc       ;
    assign  Y274Zc              = A642Yc      ? 1'b0 : Y275Zc     ;
endmodule
module osr_trng_Y316Zc   #(
    parameter   Y270Zc              =   32     
)(
    input   wire                           i_clk           ,
    input   wire                            i_rstn          ,
    input   wire                            A139Yc          , 
    input   wire    [Y270Zc             -1:0]   A211Yc          ,
    input   wire                            A212Yc          , 
    input   wire[5:0]                       Y273Zc          , 
    output  wire                            Y274Zc            
);
    reg     [5:0]   Y128Zc ;
    reg     [5:0]   Y129Zc ;
    wire    [5:0]   Y321Zc   ;
    wire    [5:0]   Y322Zc   ;
    wire    [5:0]   Y323Zc  [31:0];
    wire    [5:0]   Y324Zc  [31:0];
    wire    [5:0]   Y325Zc   ;
    wire    [5:0]   Y326Zc   ;
    wire    [5:0]   Y327Zc       [31:0];
    wire    [5:0]   Y328Zc       [31:0];
    wire    [6:0]   Y329Zc;
    wire    [6:0]   Y330Zc;
    wire            Y331Zc;
    wire            Y332Zc ;
    wire            Y333Zc ;
    genvar A282Yc;
    generate
        for(A282Yc=0;A282Yc<31;A282Yc=A282Yc+1) begin : Y334Zc 
            assign Y327Zc      [A282Yc] = A211Yc        [31-A282Yc] ? A282Yc[5:0] : Y327Zc      [A282Yc+1];
        end
    endgenerate
    assign Y327Zc      [31] = A211Yc        [0] ? 6'd31 : 6'd32;
    assign Y325Zc    = Y327Zc      [0];
    generate
        for(A282Yc=0;A282Yc<31;A282Yc=A282Yc+1) begin : Y335Zc 
            assign Y328Zc      [A282Yc] = !A211Yc        [31-A282Yc] ? A282Yc[5:0] : Y328Zc      [A282Yc+1];
        end
    endgenerate
    assign Y328Zc      [31] = !A211Yc        [0] ? 6'd31 : 6'd32;
    assign Y326Zc    = Y328Zc      [0];
    generate
        for(A282Yc=0;A282Yc<31;A282Yc=A282Yc+1) begin : Y336Zc 
            assign Y323Zc [A282Yc] = A211Yc        [A282Yc] ? A282Yc[5:0] : Y323Zc [A282Yc+1];
        end
    endgenerate
    assign Y323Zc [31] = A211Yc        [31] ? 6'd31 : 6'd32;
    assign Y321Zc    = Y323Zc [0];
    generate
        for(A282Yc=0;A282Yc<31;A282Yc=A282Yc+1) begin : Y337Zc 
            assign Y324Zc [A282Yc] = !A211Yc        [A282Yc] ? A282Yc[5:0] : Y324Zc [A282Yc+1];
        end
    endgenerate
    assign Y324Zc [31] = !A211Yc        [31] ? 6'd31 : 6'd32;
    assign Y322Zc    = Y324Zc [0];
    assign Y332Zc  = !(|A211Yc        );
    assign Y333Zc  = &A211Yc        ;
    always@(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            Y128Zc  <= 'd0;
            Y129Zc  <= 'd0;
        end
        else if(!A139Yc  ) begin
            Y128Zc  <=  'd0;
            Y129Zc  <=  'd0;
        end
        else if(A212Yc         ) begin
            Y128Zc  <=  Y332Zc  ? Y128Zc  + Y321Zc    : Y321Zc   ;
            Y129Zc  <=  Y333Zc  ? Y129Zc  + Y322Zc    : Y322Zc   ;
        end
    end
    assign Y329Zc = Y128Zc  + Y325Zc    +7'b0;
    assign Y330Zc = Y129Zc  + Y326Zc    +7'b0;
    assign Y331Zc = A212Yc          && (Y329Zc > Y273Zc   +7'b0) ? 1'b1 :
                    A212Yc          && (Y330Zc > Y273Zc   +7'b0) ? 1'b1 :
                                                          1'b0;
    assign Y274Zc      = Y331Zc;
endmodule
module osr_trng_Y318Zc        #(
    parameter                                   Y270Zc              = 32
)(
    input   wire                                clk
   ,input   wire                                rst_n
   ,input   wire                                A139Yc  
   ,input   wire                                A721Yc   
   ,output  wire                                A722Yc     
   ,output  wire                                A723Yc     
   ,output  wire                                A796Yc     
   ,output  wire                                Y271Zc       
   ,output  wire    [Y270Zc             -1:0]   Y224Zc        
   ,output  wire                                Y320Zc       
   ,input   wire                                Y222Zc     
);
    localparam                                  A234Yc              = 2'd3;
    localparam                                  A276Yc              = 3'b000;
    localparam                                  Y338Zc              = 3'b001;
    localparam                                  Y339Zc              = 3'b010;
    localparam                                  Y340Zc              = 3'b011;
    localparam                                  Y341Zc              = 3'b100;
    localparam                                  Y342Zc              = 3'b101;
    localparam                                  Y343Zc              = 3'b110;
    localparam                                  A237Yc              = 3'b111;
    reg     [A234Yc       -1:0]                     A283Yc       , A284Yc    ;
    reg     [31:0]                              Y344Zc;
    reg                                         Y345Zc       ;
    reg                                         Y346Zc     ;
    reg                                         Y244Zc      ;
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A283Yc          <=  A276Yc;
        else
            A283Yc          <=  A284Yc    ;
    end
    always @(*)
    begin
        A284Yc     = A283Yc       ;
        case(A283Yc       )
            A276Yc          :   A284Yc     = A139Yc   ? (A721Yc    ? Y338Zc           : A276Yc) : A276Yc;
            Y338Zc          :   A284Yc     = A139Yc   ? Y339Zc           : A276Yc;
            Y339Zc          :   A284Yc     = A139Yc   ? (Y222Zc      ? Y340Zc           : A237Yc ) : A276Yc;
            Y340Zc          :   A284Yc     = A139Yc   ? Y341Zc           : A276Yc;
            Y341Zc          :   A284Yc     = A139Yc   ? (Y222Zc      ? Y342Zc    : A237Yc  ) : A276Yc;
            Y342Zc          :   A284Yc     = A139Yc   ? Y343Zc :A276Yc;
            Y343Zc          :   A284Yc     = A139Yc   ? A276Yc : A276Yc;
            A237Yc          :   A284Yc     = A139Yc   ? A237Yc  : A276Yc;
            default         :   A284Yc     = A276Yc;
        endcase
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            Y344Zc          <=  32'h00000000;
            Y345Zc          <=  1'b0;
            Y346Zc          <=  1'b0;
            Y244Zc          <=  1'b0;
        end
        else begin
            case(A284Yc    )
                A276Yc          :   begin
                                        Y344Zc          <=  32'h00000000;
                                        Y345Zc          <=  1'b0;
                                        Y346Zc          <=  1'b0;
                                    end
                Y338Zc          :   begin
                                        Y345Zc          <=  1'b1;
                                        Y344Zc          <=  32'h55555000;
                                        Y346Zc          <=  1'b1;
                                        Y244Zc          <=  1'b1;
                                    end
                Y339Zc          :   begin
                                        Y344Zc          <=  32'h00000000;
                                    end
                Y340Zc          :   begin
                                        Y344Zc          <=  32'hAAAAAFFF;
                                    end
                Y341Zc          :   begin
                                        Y344Zc          <=  32'hFFFFFFFF;
                                    end
                Y342Zc          :   begin
                                        Y244Zc          <=  1'b0;
                                        Y345Zc          <=  1'b0;
                                    end
                Y343Zc          :   begin
                                        Y346Zc          <=  1'b0;
                                    end
                A237Yc          :   begin
                                        Y346Zc          <=  1'b0;
                                        Y244Zc          <=  1'b0;
                                        Y345Zc          <=  1'b0;
                                    end
                default         :   ;
            endcase
        end
    end
    assign  A722Yc      = A283Yc        == Y343Zc;
    assign  A796Yc      = Y346Zc     ;
    assign  A723Yc      = A283Yc        == A237Yc ;
    assign  Y271Zc       = Y244Zc      ;
    assign  Y224Zc         = Y344Zc;
    assign  Y320Zc        = Y345Zc       ;
endmodule
module osr_trng_Y309Zc       #(
    parameter                               Y310Zc              = 32
)(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire                            A139Yc  
   ,input   wire    [Y310Zc             -1:0]   A211Yc        
   ,input   wire                            Y308Zc       
   ,output  wire                            Y274Zc      
   ,input   wire                            A721Yc   
   ,output  wire                            A722Yc     
   ,output  wire                            A723Yc     
);
    wire                                    Y347Zc       ;
    wire    [Y310Zc             -1:0]       Y348Zc             ;
    wire                                    Y349Zc            ;
    wire                                    Y350Zc           ;
    wire    [Y310Zc             -1:0]       Y351Zc                 ;
    wire                                    Y352Zc                ;
    wire                                    Y275Zc     ;
    wire                                    A642Yc     ;
osr_trng_Y353Zc   #(
        .Y310Zc                 (Y310Zc                 )
    )
    Y354Zc    (
        .clk                    (clk                    )
       ,.rst_n                  (rst_n                  )
       ,.A139Yc                 (Y347Zc                      )
       ,.A211Yc                 (Y348Zc                 )
       ,.A212Yc                 (Y349Zc                 )
       ,.Y274Zc                 (Y275Zc                 )
    );
osr_trng_Y355Zc        #(
        .Y310Zc                 (Y310Zc                 )
    )
    Y356Zc         (
        .clk                    (clk                    )
       ,.rst_n                  (rst_n                  )
       ,.A139Yc                 (A139Yc                 )
       ,.A721Yc                 (A721Yc                 )
       ,.A722Yc                 (A722Yc                 )
       ,.A723Yc                 (A723Yc                 )
       ,.A796Yc                 (A642Yc                 )
       ,.Y357Zc                 (Y350Zc                 )
       ,.Y224Zc                 (Y351Zc                 )
       ,.Y320Zc                 (Y352Zc                 )
       ,.Y222Zc                 (Y275Zc                 )
    );
    assign  Y347Zc              = A642Yc      ? Y350Zc                  : A139Yc  ;
    assign  Y348Zc              = A642Yc      ? Y351Zc                  : A211Yc        ;
    assign  Y349Zc              = A642Yc      ? Y352Zc                  : Y308Zc       ;
    assign  Y274Zc              = A642Yc      ? 1'b0 : Y275Zc     ;
endmodule
module osr_trng_Y353Zc   #(
    parameter       Y310Zc              = 32
)(
    input   wire                                clk
   ,input   wire                                rst_n
   ,input   wire                                A139Yc  
   ,input   wire    [Y310Zc             -1:0]   A211Yc        
   ,input   wire                                A212Yc         
   ,output  wire                                Y274Zc     
);
    reg     [2:0]                               Y358Zc              ;
    reg     [127:0]                             Y359Zc              ;
    reg     [127:0]                             Y360Zc              ;
    wire                                        Y275Zc              ;
    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            Y358Zc              <=  3'd0;
            Y359Zc              <=  128'h00000000_00000000_00000000_00000000;
            Y360Zc              <=  128'hFFFFFFFF_FFFFFFFF_FFFFFFFF_FFFFFFFF;
        end
        else if(!A139Yc  ) begin
            Y358Zc              <=  3'd0;
            Y359Zc              <=  128'h00000000_00000000_00000000_00000000;
            Y360Zc              <=  128'hFFFFFFFF_FFFFFFFF_FFFFFFFF_FFFFFFFF;
        end
        else begin
            if(A212Yc         ) begin
                Y358Zc              <=  Y358Zc        + 1;
                case(Y358Zc       )
                    3'd0    :   begin
                                    Y359Zc           [31:0]     <=  A211Yc        ;
                                end
                    3'd1    :   begin
                                    Y359Zc           [63:32]    <=  A211Yc        ;
                                end
                    3'd2    :   begin
                                    Y359Zc           [95:64]    <=  A211Yc        ;
                                end
                    3'd3    :   begin
                                    Y359Zc           [127:96]   <=  A211Yc        ;
                                end
                    3'd4    :   begin
                                    Y360Zc           [31:0]     <=  A211Yc        ;
                                end
                    3'd5    :   begin
                                    Y360Zc           [63:32]    <=  A211Yc        ;
                                end
                    3'd6    :   begin
                                    Y360Zc           [95:64]    <=  A211Yc        ;
                                end
                    3'd7    :   begin
                                    Y360Zc           [127:96]   <=  A211Yc        ;
                                end
                endcase
            end
        end
    end
    assign  Y275Zc      = (A212Yc          & (Y358Zc        == 3'd4 | Y358Zc        == 3'd0)) ? (Y359Zc            == Y360Zc           ) : 1'b0;
    assign  Y274Zc      = Y275Zc     ;
endmodule
module osr_trng_Y355Zc        #(
    parameter                                   Y310Zc              = 32
)(
    input   wire                                clk
   ,input   wire                                rst_n
   ,input   wire                                A139Yc  
   ,input   wire                                A721Yc   
   ,output  wire                                A722Yc     
   ,output  wire                                A723Yc     
   ,output  wire                                A796Yc     
   ,output  wire                                Y357Zc       
   ,output  wire    [Y310Zc             -1:0]   Y224Zc        
   ,output  wire                                Y320Zc       
   ,input   wire                                Y222Zc     
);
    localparam                                  A234Yc              = 3'd4;
    localparam                                  A276Yc              = 4'b0000;
    localparam                                  Y361Zc              = 4'b0001;
    localparam                                  Y362Zc              = 4'b0010;
    localparam                                  Y363Zc              = 4'b0011;
    localparam                                  Y364Zc              = 4'b0100;
    localparam                                  Y365Zc              = 4'b0101;
    localparam                                  Y366Zc              = 4'b0110;
    localparam                                  Y367Zc              = 4'b0111;
    localparam                                  Y368Zc              = 4'b1000;
    localparam                                  Y369Zc              = 4'b1001;
    localparam                                  Y370Zc              = 4'b1010;
    localparam                                  Y343Zc              = 4'b1011;
    localparam                                  A237Yc              = 4'b1100;
    reg     [A234Yc       -1:0]                 A283Yc       , A284Yc    ;
    reg     [31:0]                              Y344Zc;
    reg                                         Y345Zc       ;
    reg                                         Y346Zc     ;
    reg                                         Y371Zc      ;
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A283Yc          <=  A276Yc;
        else
            A283Yc          <=  A284Yc    ;
    end
    always @(*)
    begin
        A284Yc     = A283Yc       ;
        case(A283Yc       )
            A276Yc          :   A284Yc     = A139Yc   ? (A721Yc    ? Y361Zc          : A276Yc) : A276Yc;
            Y361Zc          :   A284Yc     = A139Yc   ? Y362Zc          : A276Yc;
            Y362Zc          :   A284Yc     = A139Yc   ? Y363Zc          : A276Yc;
            Y363Zc          :   A284Yc     = A139Yc   ? Y364Zc          : A276Yc;
            Y364Zc          :   A284Yc     = A139Yc   ? Y365Zc          : A276Yc;
            Y365Zc          :   A284Yc     = A139Yc   ? Y366Zc          : A276Yc;
            Y366Zc          :   A284Yc     = A139Yc   ? Y367Zc          : A276Yc;
            Y367Zc          :   A284Yc     = A139Yc   ? Y368Zc          : A276Yc;
            Y368Zc          :   A284Yc     = A139Yc   ? Y369Zc      : A276Yc;
            Y369Zc          :   A284Yc     = A139Yc   ? (Y222Zc      ? Y370Zc    : A237Yc ) : A276Yc;
            Y370Zc          :   A284Yc     = A139Yc   ? Y343Zc : A276Yc;
            Y343Zc          :   A284Yc     = A139Yc   ? A276Yc : A276Yc;
            A237Yc          :   A284Yc     = A139Yc   ? A237Yc  : A276Yc;
            default         :   A284Yc     = A276Yc;
        endcase
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            Y344Zc          <=  32'h00000000;
            Y345Zc          <=  1'b0;
            Y346Zc          <=  1'b0;
            Y371Zc          <=  1'b0;
        end
        else begin
            case(A284Yc    )
                A276Yc          :   begin
                                        Y344Zc          <=  32'h00000000;
                                        Y345Zc          <=  1'b0;
                                        Y346Zc          <=  1'b0;
                                    end
                Y361Zc          :   begin
                                        Y345Zc          <=  1'b1;
                                        Y371Zc          <=  1'b1;
                                        Y346Zc          <=  1'b1;
                                        Y344Zc          <=  32'h01234567;
                                    end
                Y362Zc          :   begin
                                        Y344Zc          <=  32'h89abcdef;
                                    end
                Y363Zc          :   begin
                                        Y344Zc          <=  32'hfedcba98;
                                    end
                Y364Zc          :   begin
                                        Y344Zc          <=  32'h76543210;
                                    end
                Y365Zc          :   begin
                                        Y344Zc          <=  32'h01234567;
                                    end
                Y366Zc          :   begin
                                        Y344Zc          <=  32'h89abcdef;
                                    end
                Y367Zc          :   begin
                                        Y344Zc          <=  32'hfedcba98;
                                    end
                Y368Zc          :   begin
                                        Y344Zc          <=  32'h76543210;
                                    end
                Y370Zc          :   begin
                                        Y345Zc          <=  1'b0;
                                        Y371Zc          <=  1'b0;
                                    end
                Y343Zc          :   begin
                                        Y346Zc          <=  1'b0;
                                    end
                A237Yc          :   begin
                                        Y346Zc          <=  1'b0;
                                        Y371Zc          <=  1'b0;
                                        Y345Zc          <=  1'b0;
                                    end
                default         :   ;
            endcase
        end
    end
    assign  A722Yc      = A283Yc        == Y343Zc;
    assign  A796Yc      = Y346Zc     ;
    assign  A723Yc      = A283Yc        == A237Yc ;
    assign  Y357Zc       = Y371Zc      ;
    assign  Y224Zc         = Y344Zc;
    assign  Y320Zc        = Y345Zc       ;
endmodule
module osr_trng_Y372Zc   #(
    parameter               A411Yc                  =   1                   ,
    parameter               A412Yc                  =   0                   ,
	parameter			    A007Yc      			=	32						
)(
	input	wire							i_clk						,
	input	wire							i_rstn					,
    input   wire                            Y373Zc                  ,
    input   wire                            A720Yc                  ,
	input	wire							A721Yc   				,
    input   wire                            Y374Zc                  ,
	output	wire						    A722Yc     				,
	output	wire						    A723Yc     				,
	input	wire    [A007Yc      -1:0]	    Y375Zc          		,
	output  wire	[A007Yc      -1:0]	    Y376Zc          		,
	output	wire							Y377Zc           		,
	output	wire						    Y378Zc           		,
    output  wire                            Y379Zc               ,
    input   wire                            Y380Zc               ,  
	input	wire							Y381Zc           		,
	input	wire	[A007Yc      -1:0]	    Y382Zc          		,
	output	wire							Y383Zc           		
);
	localparam	Y233Zc    		=	2			; 
	localparam	A234Yc       	=	3			;
	localparam	Y110Zc   		=	3'b001	;
	localparam	Y384Zc     		=	3'b010	;
	localparam	Y385Zc     		=	3'b011	;
	localparam	Y386Zc     		=	3'b100	;
	localparam	Y387Zc     		=	3'b101	;
	localparam	Y232Zc				=	3'b110	;
	reg	[A234Yc       -1:0]			A244Yc					,A245Yc						;
	reg	[Y233Zc    -1:0]				Y234Zc     			,Y235Zc          		;
	reg										Y252Zc     			,Y253Zc          		;
	reg										Y254Zc     			,Y255Zc          		;
	reg	[6:0]								A268Yc					,A269Yc    				;
	reg	[A007Yc      -1:0]			Y388Zc   			,Y389Zc        		;
	reg										Y390Zc    			,Y391Zc         		;
	reg										Y392Zc    			,Y393Zc         		;
   reg                              Y394Zc            ,Y395Zc              ;
	reg										Y396Zc    			,Y397Zc         		;
   reg                              Y398Zc                                 ; 
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
			A244Yc 				<=  Y110Zc   ;
			Y234Zc     		<=  'd0;
			Y252Zc     		<=  'b0;
			Y254Zc     		<=  'b0;
			A268Yc				<=  'd0;
			Y388Zc   		<=  'd0;
         Y390Zc    		<=  'b0;
         Y392Zc    		<=  'b0;
         Y396Zc    		<=  'b0;
         Y394Zc     		<=  'b0;
		end
		else begin
			A244Yc				<=  Y374Zc          ?  A245Yc					: Y110Zc    ;
			Y234Zc     		<=  Y374Zc          ?  Y235Zc          	: 'd0       ;
			Y252Zc     		<=  Y374Zc          ?  Y253Zc          	: 'b0       ;
			Y254Zc     		<=  Y374Zc          ?  Y255Zc          	: 'b0       ;
			A268Yc				<=  Y374Zc          ?  A269Yc    			: 'd0       ;
			Y388Zc   		<=  Y374Zc          ?  Y389Zc        		: 'd0       ;
         Y390Zc    		<=  Y374Zc          ?  Y391Zc         	: 'b0       ;
         Y392Zc    		<=  Y374Zc          ?  Y393Zc            : 'b0       ;
         Y396Zc    		<=  Y374Zc          ?  Y397Zc         	: 'b0       ;
         Y394Zc     		<=  Y374Zc          ?  Y395Zc            : 'b0       ;
		end
	end
	always@(*)
	begin
		A245Yc						= A244Yc				;
		Y235Zc          		= Y234Zc     		;
		Y253Zc          		= Y252Zc     		;
		Y255Zc          		= Y254Zc     		;
		A269Yc    				= A268Yc				;
		Y389Zc        			= Y388Zc   			;
      Y391Zc         		= Y390Zc    		;
      Y393Zc               = Y392Zc          ;
      Y397Zc         		= Y396Zc    		;
      Y395Zc               = Y394Zc          ;
		case(A244Yc)
			Y110Zc    	:  begin
									Y253Zc           		= 'b0;
									Y255Zc           		= 'b0;
									Y235Zc           		= 'd0;
									A269Yc    				= 'd0;
									Y389Zc        			= 'd0;
                           Y391Zc         		= 'b0;
                           Y393Zc         		= 'b0;
                           Y397Zc         		= 'b0;
                           Y395Zc               = 'b0;
									if(A721Yc   ) begin
										A245Yc = Y384Zc     ;
									end
								end
			Y384Zc     	:	begin
									A269Yc     = A268Yc + 1'b1;
									Y391Zc          = 1'b1;
									Y389Zc         = 	A268Yc[2:0]==3'd0 ? 32'h5555_5555 : {
															A268Yc[2:0]==3'd1 ? 32'haaaa_aaaa : {
															A268Yc[2:0]==3'd2 ? 32'h9999_9999 : {
															A268Yc[2:0]==3'd3 ? 32'h6666_6666 : {
															A268Yc[2:0]==3'd4 ? 32'hcccc_cccc : {
															A268Yc[2:0]==3'd5 ? 32'h3333_3333 : {
															A268Yc[2:0]==3'd6 ? 32'h4444_4444 : 32'h2222_2222}}}}}};
									if(A268Yc=='d87) begin
										A245Yc = Y385Zc     ;
										Y391Zc          = 1'b0;
										Y389Zc         = 'd0;
										A269Yc     = 'd0;
									end
								end
			Y385Zc     	:	begin
									if(Y380Zc          ) begin
										A245Yc = Y386Zc     ;
									end
								end
         Y386Zc      :  begin
									A269Yc     = A268Yc + 1'b1;
                                    Y393Zc          = 'b1;
                                    Y395Zc           = 'b1;
                                    if(A268Yc=='d4) begin
                                       Y393Zc          = 'b0;
                                       Y395Zc           = 'b0;
                                       A245Yc = Y387Zc     ;
                                        if(A411Yc               & A412Yc              )begin
                                            if(Y373Zc || !A720Yc   )
                                                Y235Zc          [0] = Y375Zc          ==32'h9999_9999;
                                            else
                                                Y235Zc          [0] = Y375Zc          ==32'h2222_2222;
                                        end 
                                        else if (A412Yc               && A720Yc   )    
                                                Y235Zc          [0] = Y375Zc          ==32'h2222_2222;
                                        else
                                        Y235Zc          [0] = Y375Zc          ==32'h9999_9999;
                                    end
                        end
			Y387Zc     	:	begin
									Y397Zc          = 1'b1;
									Y389Zc         = Y388Zc    ^ Y382Zc          ;
									if(Y398Zc    ) begin
										A245Yc = Y232Zc;
                                        Y389Zc         = Y388Zc   ;
                                        Y397Zc          = 1'b0;
                                        if(A411Yc               | A412Yc              ) begin
                                            if(A411Yc               && Y373Zc || (!A412Yc              )) 
                                                Y235Zc          [1] = Y388Zc   ==32'hd483_9394;
                                            else if ((A412Yc               & !Y373Zc) || !A411Yc              ) begin 
                                                if (A720Yc   )
                                                    Y235Zc          [1] = Y388Zc   ==32'h6c2b_7440;
                                                else
                                                    Y235Zc          [1] = Y388Zc   ==32'h0da4_811d;
                                            end
                                        end    
                                        else
                                        Y235Zc          [1] = 1'b1; 
									end
								end
			Y232Zc			:	begin
									A245Yc = Y110Zc   ;
									Y253Zc           = 1'b1;
                           Y255Zc           = !(&Y234Zc     );
								end
			default		:	A245Yc = Y110Zc   ;
		endcase
	end
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
	      Y398Zc     <=  'b0;
      end
      else begin
         Y398Zc     <=  Y381Zc           ;
      end
   end
	assign A722Yc     			   = Y252Zc     		                  ;
	assign A723Yc     			   = Y254Zc     		                  ;
	assign Y376Zc          	   = Y388Zc   			                  ;	
	assign Y377Zc           	   = Y390Zc    		                  ;		
   assign Y378Zc               = Y392Zc                            ;
   assign Y379Zc               = Y394Zc                            ;
	assign Y383Zc           	   = Y396Zc      && !Y381Zc            ;		
endmodule
module osr_trng_A740Yc            # (
	parameter   A294Yc                  =   128          	,
	parameter   A295Yc                  =   128          	,
	parameter   A297Yc                  =   256          	,
	parameter   A180Yc                  =   32           
)(
	input   wire                          i_clk            	,
	input  	wire                         i_rstn          	,
    input   wire                          A139Yc            ,
	input	wire									A721Yc   			,
	output	wire									A722Yc     			,
	output	wire									A723Yc     			,
	input 	wire                           A724Yc          	,
	output  wire	                           A725Yc          	,
	output  wire	                           A726Yc          	,
	output  wire	                           A727Yc          	,
	output  wire	                           A728Yc          	,
	output  wire	[A180Yc     -1:0]          A729Yc          	,
	input 	wire                        A730Yc          	,
	input 	wire[31:0]                   A731Yc          	,
	input 	wire[31:0]                   A732Yc          	,
	input 	wire[A180Yc     -1:0]          A733Yc          	,
	input 	wire                          A734Yc          	,
	input 	wire[A180Yc     -1:0]          A735Yc          	,
	output  wire	                           A736Yc          	,
	output  wire	[A180Yc     -1:0]          A737Yc          	,
	output  wire	[A297Yc   -1:0]            A738Yc          	,
	output	wire[A297Yc   -1:0]            A739Yc          	
);
	localparam	Y233Zc    		=	3			; 
	localparam	A234Yc       	=	4			;
	localparam	Y110Zc   		=	4'b0001	;
	localparam	Y399Zc    		=	4'b0010	;
	localparam	Y400Zc    		=	4'b0011	;
	localparam	Y401Zc    		=	4'b0100	;
	localparam	Y402Zc    		=	4'b0101	;
	localparam	Y403Zc    		=	4'b0110	;
	localparam	Y404Zc    		=	4'b0111	;
	localparam	Y405Zc    		=	4'b1000	;
	localparam	Y406Zc    		=	4'b1001	;
	localparam	Y407Zc    		=	4'b1010	;
	localparam	Y232Zc				=	4'b1011	;
	reg	[A234Yc       -1:0]			A244Yc,A245Yc;
	reg	[Y233Zc    -1:0]				Y234Zc     ,Y235Zc          ;
	reg										Y252Zc     ,Y253Zc          ;
	reg										Y254Zc     ,Y255Zc          ;
	reg	[3:0]								A268Yc,A269Yc    ;
	reg										Y408Zc   ,Y409Zc        ;
	reg										Y410Zc   ,Y411Zc        ;
	reg										Y412Zc   ,Y413Zc        ;
	reg										Y414Zc   ,Y415Zc        ;
	reg										Y416Zc       ,Y417Zc            ;
	reg  	[A180Yc     -1:0]          Y418Zc   ,Y419Zc        ;
	reg	[A180Yc     -1:0]				Y420Zc,Y421Zc    ;
	always @(posedge i_clk or negedge i_rstn)
	begin
		if(!i_rstn) begin
			A244Yc 				<=  Y110Zc   ;
			Y234Zc     		<=  'd0;
			Y252Zc     		<=  'b0;
			Y254Zc     		<=  'b0;
			A268Yc				<=  'd0;
			Y408Zc   		<=  'b0;
			Y410Zc   		<=  'b0;
			Y412Zc   		<=  'b0;
			Y414Zc   		<=  'b0;
			Y416Zc       	<=  'b0;
			Y418Zc   		<=  'd0;
			Y420Zc				<=  'd0;
		end
		else begin
			A244Yc				<=  A139Yc   ? A245Yc                 : Y110Zc       ;
			Y234Zc     		<=  A139Yc   ? Y235Zc                 : 'd0          ;
			Y252Zc     		<=  A139Yc   ? Y253Zc                 : 'b0          ;
			Y254Zc     		<=  A139Yc   ? Y255Zc                 : 'b0          ;
			A268Yc				<=  A139Yc   ? A269Yc                 : 'd0          ;
			Y408Zc   		<=  A139Yc   ? Y409Zc                 : 'b0          ;
			Y410Zc   		<=  A139Yc   ? Y411Zc                 : 'b0          ;
			Y412Zc   		<=  A139Yc   ? Y413Zc                 : 'b0          ;
			Y414Zc   		<=  A139Yc   ? Y415Zc                 : 'b0          ;
			Y416Zc       	<=  A139Yc   ? Y417Zc                 : 'b0          ;
			Y418Zc   		<=  A139Yc   ? Y419Zc                 : 'd0          ;
			Y420Zc				<=  A139Yc   ? Y421Zc                 : 'd0          ;
		end
	end
	always@(*)
	begin
		A245Yc						= A244Yc				;
		Y235Zc          		= Y234Zc     		;
		Y253Zc          		= Y252Zc     		;
		Y255Zc          		= Y254Zc     		;
		A269Yc    				= A268Yc				;
		Y409Zc        			= Y408Zc   			;
		Y411Zc        			= Y410Zc   			;
		Y413Zc        			= Y412Zc   			;
		Y415Zc        			= Y414Zc   			;
		Y417Zc            	= Y416Zc       	;
		Y419Zc        			= Y418Zc   			;
		Y421Zc    				= Y420Zc				;
		case(A244Yc)
			Y110Zc    	:  begin
									Y253Zc           		= 'b0;
									Y255Zc           		= 'b0;
									Y235Zc           		= 'd0;
									A269Yc    				= 'd0;
									Y421Zc    				= 'd0;
									if(A721Yc   ) begin
										A245Yc 						= Y399Zc    ;
									end
								end
			Y399Zc    	:	begin
									A269Yc     		= A268Yc + 1'b1;
									Y409Zc         = 1'b0;
									if(A730Yc       ) begin
										A245Yc = Y400Zc    ;
										Y417Zc             = 1'b1;
										Y419Zc         = {(A180Yc     >>2){4'h5}};
										A269Yc     = 'd0;
									end
									else if(A724Yc) begin
										A269Yc      = A268Yc;
									end
									else if(A268Yc=='d0) begin
										Y409Zc         = 1'b1;
									end
								end
			Y400Zc    	:	begin
                           if (!A724Yc) begin
										A245Yc = Y401Zc    ;
                           end
									else if(!A730Yc       ) begin
										Y417Zc             = 1'b0;
									end
								end
			Y401Zc    	:	begin
									A245Yc = Y402Zc    ;
									Y235Zc          [0] = 		(A731Yc    [31:0]==32'h97d4_7669)
																	&& (A732Yc  [31:0]==32'h4281_cd4b)
																	&&	(A733Yc    ==32'h0000_0001);
								end
			Y402Zc    	:	begin
									A269Yc     		= A268Yc + 1'b1;
									Y413Zc        	= 1'b0;
									if(A734Yc         ) begin
										A245Yc = Y403Zc    ;
										A269Yc     = 'd0;
										Y421Zc     = A735Yc     ;
									end
									else if(A724Yc) begin
										A269Yc      = A268Yc;
									end
									else if(A268Yc=='d0) begin
										Y413Zc         = 1'b1;
									end
								end
			Y403Zc    	:	begin
									if(A734Yc         ) begin
										Y421Zc     = Y420Zc ^ A735Yc     ;
									end
									else if (!A724Yc) begin
										A245Yc = Y404Zc    ;
									end
								end
			Y404Zc    	:	begin
									A245Yc = Y405Zc    ;
									Y235Zc          [1] = 		(Y420Zc==32'h3f5a_b944)
																	&&	(A731Yc    [31:0]==32'hf572_2f2d)
																	&& (A732Yc  [31:0]==32'hfeb2_dbb6)
																	&&	(A733Yc    ==32'h0000_0002);
								end
			Y405Zc    	:	begin
									A269Yc     		= A268Yc + 1'b1;
									Y411Zc        	= 1'b0;
									if(A730Yc       ) begin
										A245Yc = Y406Zc    ;
										A269Yc     = 'd0;
										Y417Zc             = 1'b1;
										Y419Zc         = {(A180Yc     >>2){4'ha}};
									end
									else if(A724Yc) begin
										A269Yc      = A268Yc;
									end
									else if(A268Yc=='d0) begin
										Y411Zc         = 1'b1;
									end
								end
			Y406Zc    	:	begin
                           if (!A724Yc) begin
										A245Yc = Y407Zc    ;
                           end
									else if(!A730Yc       ) begin
										Y417Zc             = 1'b0;
									end
								end
			Y407Zc    	:	begin
									A245Yc = Y232Zc;
									Y235Zc          [2] = 		(A731Yc    [31:0]==32'hbb75_0ef2)
																	&& (A732Yc  [31:0]==32'h6aac_0330)
																	&&	(A733Yc    ==32'h0000_0001);
								end
			Y232Zc			:	begin
									Y253Zc           	= 1'b1;
									Y255Zc          	= !(&Y235Zc          );
									A245Yc = Y110Zc   ;
								end
			default		:	A245Yc = Y110Zc   ;
		endcase
	end
	assign  A722Yc     			= Y252Zc     				;
	assign  A723Yc     			= Y254Zc     				;
	assign  A725Yc   			= Y408Zc   					;
	assign  A726Yc   			= Y410Zc   					;
	assign  A727Yc   			= Y412Zc   					;
	assign  A728Yc   			= Y414Zc   					;
	assign  A729Yc         	    = 32'd5							;
	assign  A736Yc       		= Y416Zc       			;
	assign  A737Yc   			= Y418Zc   					;
	assign  A738Yc   			= {A297Yc   >>2{4'h6}}	;
	assign  A739Yc   			= {A297Yc   >>2{4'h9}}	;
endmodule
module osr_trng_A718Yc            # (
    parameter   A294Yc                  =   128         ,
    parameter   A295Yc                  =   128         ,
    parameter   A296Yc                  =   128         ,
    parameter   A297Yc                  =   256         ,
    parameter   A180Yc                  =   32           
)(
    input   wire                           i_clk            ,
    input   wire                           i_rstn           ,
    input   wire                           A139Yc           ,
    input   wire                           A720Yc           ,
    input   wire                           A721Yc           ,
    output  wire                           A722Yc           ,
    output  wire                           A723Yc           ,
    input   wire                           A724Yc           ,
    output  wire                           A725Yc           ,
    output  wire                           A726Yc           ,
    output  wire                           A727Yc           ,
    output  wire                           A728Yc           ,
    output  wire[A180Yc     -1:0]          A729Yc           ,
    input   wire                           A730Yc           ,
    input   wire[31:0]                     A731Yc           ,
    input   wire[31:0]                     A732Yc           ,
    input   wire[A180Yc     -1:0]          A733Yc           ,
    input   wire                           A734Yc           ,
    input   wire[A180Yc     -1:0]          A735Yc           ,
    output  wire                           A736Yc           ,
    output  wire[A180Yc     -1:0]          A737Yc           ,
    output  wire[A297Yc   -1:0]            A738Yc           ,
    output  wire[A297Yc   -1:0]            A739Yc           
);
    localparam  Y233Zc          =   3       ; 
    localparam  A234Yc          =   4       ;
    localparam  Y110Zc          =   4'b0001 ;
    localparam  Y399Zc          =   4'b0010 ;
    localparam  Y400Zc          =   4'b0011 ;
    localparam  Y401Zc          =   4'b0100 ;
    localparam  Y402Zc          =   4'b0101 ;
    localparam  Y403Zc          =   4'b0110 ;
    localparam  Y404Zc          =   4'b0111 ;
    localparam  Y405Zc          =   4'b1000 ;
    localparam  Y406Zc          =   4'b1001 ;
    localparam  Y407Zc          =   4'b1010 ;
    localparam  Y232Zc          =   4'b1011 ;
    reg [A234Yc       -1:0]         A244Yc,A245Yc;
    reg [Y233Zc    -1:0]            Y234Zc     ,Y235Zc          ;
    reg                             Y252Zc     ,Y253Zc          ;
    reg                             Y254Zc     ,Y255Zc          ;
    reg [3:0]                       A268Yc,A269Yc    ;
    reg                             Y408Zc   ,Y409Zc        ;
    reg                             Y410Zc   ,Y411Zc        ;
    reg                             Y412Zc   ,Y413Zc        ;
    reg                             Y414Zc   ,Y415Zc        ;
    reg                             Y416Zc       ,Y417Zc            ;
    reg [A180Yc     -1:0]           Y418Zc   ,Y419Zc        ;
    reg [A180Yc     -1:0]           Y420Zc,Y421Zc    ;
    always @(posedge i_clk or negedge i_rstn)
    begin
        if(!i_rstn) begin
            A244Yc          <=  Y110Zc   ;
            Y234Zc          <=  'd0;
            Y252Zc          <=  'b0;
            Y254Zc          <=  'b0;
            A268Yc          <=  'd0;
            Y408Zc          <=  'b0;
            Y410Zc          <=  'b0;
            Y412Zc          <=  'b0;
            Y414Zc          <=  'b0;
            Y416Zc          <=  'b0;
            Y418Zc          <=  'd0;
            Y420Zc          <=  'd0;
        end
        else begin
            A244Yc          <=  A139Yc   ? A245Yc                 : Y110Zc       ;
            Y234Zc          <=  A139Yc   ? Y235Zc                 : 'd0          ;
            Y252Zc          <=  A139Yc   ? Y253Zc                 : 'b0          ;
            Y254Zc          <=  A139Yc   ? Y255Zc                 : 'b0          ;
            A268Yc          <=  A139Yc   ? A269Yc                 : 'd0          ;
            Y408Zc          <=  A139Yc   ? Y409Zc                 : 'b0          ;
            Y410Zc          <=  A139Yc   ? Y411Zc                 : 'b0          ;
            Y412Zc          <=  A139Yc   ? Y413Zc                 : 'b0          ;
            Y414Zc          <=  A139Yc   ? Y415Zc                 : 'b0          ;
            Y416Zc          <=  A139Yc   ? Y417Zc                 : 'b0          ;
            Y418Zc          <=  A139Yc   ? Y419Zc                 : 'd0          ;
            Y420Zc          <=  A139Yc   ? Y421Zc                 : 'd0          ;
        end
    end
    always@(*)
    begin
        A245Yc                  = A244Yc            ;
        Y235Zc                  = Y234Zc            ;
        Y253Zc                  = Y252Zc            ;
        Y255Zc                  = Y254Zc            ;
        A269Yc                  = A268Yc            ;
        Y409Zc                  = Y408Zc            ;
        Y411Zc                  = Y410Zc            ;
        Y413Zc                  = Y412Zc            ;
        Y415Zc                  = Y414Zc            ;
        Y417Zc                  = Y416Zc            ;
        Y419Zc                  = Y418Zc            ;
        Y421Zc                  = Y420Zc            ;
        case(A244Yc)
            Y110Zc      :  begin
                               Y253Zc                  = 'b0;
                               Y255Zc                  = 'b0;
                               Y235Zc                  = 'd0;
                               A269Yc                  = 'd0;
                               Y421Zc                  = 'd0;
                               if(A721Yc   ) begin
                                   A245Yc                  = Y399Zc    ;
                               end
                           end
            Y399Zc      :   begin
                                A269Yc         = A268Yc + 1'b1;
                                Y409Zc         = 1'b0;
                                if(A730Yc       ) begin
                                    A245Yc = Y400Zc    ;
                                    Y417Zc             = 1'b1;
                                    Y419Zc         = {(A180Yc     >>2){4'h5}};
                                    A269Yc     = 'd0;
                                end
                                else if(A724Yc) begin
                                    A269Yc      = A268Yc;
                                end
                                else if(A268Yc=='d0) begin
                                    Y409Zc         = 1'b1;
                                end
                            end
            Y400Zc      :   begin
                            if (!A724Yc) begin
                                         A245Yc = Y401Zc    ;
                            end
                                else if(!A730Yc       ) begin
                                    Y417Zc             = 1'b0;
                                end
                            end
            Y401Zc      :   begin
                                A245Yc = Y402Zc    ;
                                Y235Zc          [0] =  (A731Yc    [31:0]==32'h86ef_b6be)
                                                    && (A732Yc  [31:0]==32'h8cd8_5eba)
                                                    && (A733Yc    ==32'h0000_0001);
                            end
            Y402Zc      :   begin
                                A269Yc          = A268Yc + 1'b1;
                                Y413Zc          = 1'b0;
                                if(A734Yc         ) begin
                                    A245Yc = Y403Zc    ;
                                    A269Yc     = 'd0;
                                    Y421Zc     = A735Yc     ;
                                end
                                else if(A724Yc) begin
                                    A269Yc      = A268Yc;
                                end
                                else if(A268Yc=='d0) begin
                                    Y413Zc         = 1'b1;
                                end
                                if (A730Yc       )begin
                                    Y417Zc             = 1'b1;
                                    Y419Zc         = {(A180Yc     >>2){4'hc}};
                                end
                                else begin
                                    Y417Zc             = 1'b0;
                                end
                            end
            Y403Zc      :   begin
                                if(A734Yc         ) begin
                                    Y421Zc     = Y420Zc ^ A735Yc     ;
                                end
                                else if (!A724Yc) begin
                                    A245Yc = Y404Zc    ;
                                end
                                if (A730Yc       )begin
                                    Y417Zc             = 1'b1;
                                    Y419Zc         = {(A180Yc     >>2){4'hd}};
                                end
                                else begin
                                    Y417Zc             = 1'b0;
                                end
                            end
            Y404Zc      :   begin
                                A245Yc = Y405Zc    ;
                                if (A720Yc   )
                                    Y235Zc          [1] =  (A731Yc    [31:0]==32'h4a29_28dc)
                                                        && (Y420Zc==32'h554a_fe01)
                                                        && (A732Yc  [31:0]==32'h6605_a5ae)
                                                        && (A733Yc    ==32'h0000_0002);
                                else
                                    Y235Zc          [1] =       (Y420Zc==32'h128f_dacf)
                                                        &&  (A731Yc    [31:0]==32'h5505_a785)
                                                        &&  (A732Yc  [31:0]==32'h77a2_301e)
                                                        &&  (A733Yc    ==32'h0000_0002);
                            end
            Y405Zc      :   begin
                                A269Yc          = A268Yc + 1'b1;
                                Y411Zc          = 1'b0;
                                if(A730Yc       ) begin
                                    A245Yc = Y406Zc    ;
                                    A269Yc     = 'd0;
                                    Y417Zc             = 1'b1;
                                    Y419Zc         = {(A180Yc     >>2){4'ha}};
                                end
                                else if(A724Yc) begin
                                    A269Yc      = A268Yc;
                                end
                                else if(A268Yc=='d0) begin
                                    Y411Zc         = 1'b1;
                                end
                            end
            Y406Zc      :   begin
                            if (!A724Yc) begin
                                         A245Yc = Y407Zc    ;
                            end
                                else if(!A730Yc       ) begin
                                    Y417Zc             = 1'b0;
                                end
                            end
            Y407Zc      :   begin
                                A245Yc = Y232Zc;
                                if (A720Yc   )
                                    Y235Zc          [2] =  (A731Yc    [31:0]==32'hb0a2_7c5f)
                                                        && (A732Yc  [31:0]==32'hc6bf_7fb8)
                                                        && (A733Yc    ==32'h0000_0001);
                                else
                                    Y235Zc          [2] =      (A731Yc    [31:0]==32'h1484_1a15)
                                                            && (A732Yc  [31:0]==32'h991c_b65c)
                                                            && (A733Yc    ==32'h0000_0001);
                            end
            Y232Zc      :   begin
                                Y253Zc              = 1'b1;
                                Y255Zc              = !(&Y235Zc          );
                                A245Yc = Y110Zc   ;
                            end
            default     :   A245Yc = Y110Zc   ;
        endcase
    end
    assign A722Yc               = Y252Zc                    ;
    assign A723Yc               = Y254Zc                    ;
    assign A725Yc               = Y408Zc                    ;
    assign A726Yc               = Y410Zc                    ;
    assign A727Yc               = Y412Zc                    ;
    assign A728Yc               = Y414Zc                    ;
    assign A729Yc               = 3'd5 + {A180Yc     {1'b0}};
    assign A736Yc               = Y416Zc                ;
    assign A737Yc               = Y418Zc                    ;
    assign A738Yc               = {A297Yc   >>2{4'h6}}  ;
    assign A739Yc               = {A297Yc   >>2{4'h9}}  ;
endmodule
module osr_trng_A794Yc   
(
    input   wire                            clk
   ,input   wire                            rst_n
   ,input   wire                            A139Yc   
   ,input   wire                            A721Yc   
   ,output  wire                            A722Yc     
   ,output  wire                            A723Yc     
   ,output  wire                            A796Yc     
   ,input   wire                            A797Yc          
   ,input   wire                            A798Yc          
   ,input   wire    [31:0]                  A799Yc          
   ,output  wire                            A800Yc          
   ,output  wire    [31:0]                  A801Yc          
);
    localparam                              A234Yc              = 3'd4;
    localparam                              A276Yc              = 4'b0000;
    localparam                              Y422Zc              = 4'b0001;
    localparam                              Y423Zc              = 4'b0010;
    localparam                              Y424Zc              = 4'b0011;
    localparam                              Y425Zc              = 4'b0100;
    localparam                              Y426Zc              = 4'b0101;
    localparam                              Y427Zc              = 4'b0110;
    localparam                              Y428Zc              = 4'b0111;
    localparam                              Y429Zc              = 4'b1000;
    localparam                              Y430Zc              = 4'b1001;
    localparam                              Y343Zc              = 4'b1010;
    localparam                              A237Yc              = 4'b1011;
    reg     [A234Yc       -1:0]             A283Yc       , A284Yc    ;
    reg     [31:0]                          Y344Zc;
    reg                                     Y431Zc    ;
    reg                                     Y346Zc     ;
    reg                                     Y432Zc      ;
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n)
            A283Yc          <=  A276Yc;
        else if(Y432Zc      )
            A283Yc          <=  A237Yc ;
        else
            A283Yc          <=  A284Yc    ;
    end
    always @(*)
    begin
        A284Yc     = A283Yc       ;
        case(A283Yc       )
            A276Yc      :   A284Yc      = A139Yc   ? (A721Yc     ? Y422Zc       : A276Yc)       : A276Yc;
            Y422Zc      :   A284Yc      = A139Yc   ? (A797Yc     ? Y423Zc       : Y422Zc      ) : A276Yc; 
            Y423Zc      :   A284Yc      = A139Yc   ? (A797Yc     ? Y424Zc       : Y423Zc      ) : A276Yc; 
            Y424Zc      :   A284Yc      = A139Yc   ? (A797Yc     ? Y425Zc       : Y424Zc      ) : A276Yc; 
            Y425Zc      :   A284Yc      = A139Yc   ? (A797Yc     ? Y426Zc       : Y425Zc      ) : A276Yc;
            Y426Zc      :   A284Yc      = A139Yc   ? (A798Yc? Y427Zc       : Y426Zc     )  : A276Yc;
            Y427Zc      :   A284Yc      = A139Yc   ? Y428Zc     : A276Yc;
            Y428Zc      :   A284Yc      = A139Yc   ? Y429Zc     : A276Yc;
            Y429Zc      :   A284Yc      = A139Yc   ? Y430Zc     : A276Yc;
            Y430Zc      :   A284Yc      = A139Yc   ? Y343Zc     : A276Yc;
            Y343Zc      :   A284Yc      = A139Yc   ? A276Yc     : A276Yc;
            A237Yc      :   A284Yc      = A139Yc   ? A237Yc     : A276Yc;
            default     :   A284Yc      = A276Yc;
        endcase
    end
    always @(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            Y344Zc      <=  32'h00000000;
            Y431Zc      <=  1'b0;
            Y432Zc      <=  1'b0;
            Y346Zc      <=  1'b0;
        end
        else begin
            case(A284Yc    )
                A276Yc      :   begin
                                    Y344Zc      <=  32'h00000000;
                                    Y431Zc      <=  1'b0;
                                    Y432Zc      <=  1'b0;
                                    Y346Zc      <=  1'b0;
                                end
                Y422Zc       :   begin
                                    Y431Zc      <=  1'b1;
                                    Y346Zc      <=  1'b1;
                                end
                Y423Zc      :   begin
                                    Y431Zc      <=  1'b1;
                                    Y344Zc      <=  32'h55555555;
                                end
                Y424Zc      :   begin
                                    Y431Zc      <=  1'b1;
                                    Y344Zc      <=  32'hAAAAAAAA;
                                end
                Y425Zc      :   begin
                                    Y431Zc      <=  1'b1;
                                    Y344Zc      <=  32'h99999999;
                                end
                Y426Zc      :   begin
                                    Y431Zc      <=  1'b0;
                                    Y344Zc      <=  32'h66666666;
                                end
                Y427Zc      :   begin
                                    if(A799Yc != 32'h18000003)
                                        Y432Zc      <=  1'b1;
                                end
                Y428Zc      :   begin
                                    if(A799Yc != 32'h07FFFFFF)
                                        Y432Zc      <=  1'b1;
                                end
                Y429Zc      :   begin
                                    if(A799Yc != 32'h18000003)
                                        Y432Zc      <=  1'b1;
                                end
                Y430Zc      :   begin
                                    if(A799Yc != 32'h0BC00001)
                                        Y432Zc      <=  1'b1;
                                end
                Y343Zc      :   begin
                                    Y346Zc      <=  1'b0;
                                end
                A237Yc      :   begin
                                    Y346Zc      <=  1'b0;
                                end
                default     :   ;
            endcase
        end
    end
    assign  A722Yc      = A283Yc        == Y343Zc;
    assign  A796Yc      = Y346Zc     ;
    assign  A723Yc      = A283Yc        == A237Yc ;
    assign  A801Yc      = Y344Zc;
    assign  A800Yc      = Y431Zc    ;
endmodule
module osr_trng_Y433Zc     
#(
    parameter   Y434Zc                      =  20       
   ,parameter   Y435Zc                      =  32
)(
    input   wire                            i_s_hclk      
   ,input   wire                            i_s_hresetn   
   ,input   wire                            Y436Zc        
   ,input   wire    [Y434Zc          -1:0]  Y437Zc        
   ,input   wire                            Y438Zc        
   ,input   wire    [2:0]                   Y439Zc        
   ,input   wire    [2:0]                   Y440Zc        
   ,input   wire    [3:0]                   Y441Zc        
   ,input   wire    [1:0]                   Y442Zc        
   ,input   wire                            Y443Zc        
   ,input   wire                            Y444Zc        
   ,input   wire    [Y435Zc          -1:0]  Y445Zc        
   ,output  wire                            Y446Zc        
   ,output  wire                            Y447Zc        
   ,output  wire    [31:0]                  Y448Zc        
   ,output  wire    [Y434Zc          -1:0]  Y449Zc      
   ,output  wire    [Y435Zc          -1:0]  A176Yc      
   ,output  wire                            Y450Zc      
   ,output  wire                            Y451Zc      
   ,output  wire    [3:0]                   Y452Zc      
   ,output  wire                            Y453Zc      
   ,input   wire    [Y435Zc          -1:0]  Y454Zc    
   ,output	wire							Y455Zc    
);
    localparam  [Y434Zc          -1:0]  Y456Zc           = 8'h0C + {Y434Zc          {1'b0}};
    localparam  [Y434Zc          -1:0]  Y457Zc           = 8'hD4 + {Y434Zc          {1'b0}};
    reg     [Y434Zc          -1:0]  Y458Zc              ;
    reg                             Y459Zc              ;
    reg                             Y460Zc              ;
    reg     [3:0]                   Y461Zc              ;
    reg     [3:0]                   Y462Zc              ;
    reg                             Y463Zc              ;
    wire    [Y434Zc          -1:0]  Y464Zc              ;
    wire                            Y465Zc              ;
    wire                            Y466Zc              ;
    wire    [3:0]                   Y467Zc              ;
    wire                            Y468Zc              ;
    wire                            Y469Zc              ;
    wire                            Y470Zc              ;
    wire                            Y471Zc              ;
    wire                            Y472Zc              ;
    wire                            Y473Zc              ;
    wire                            Y474Zc              ;
    assign Y469Zc         = Y436Zc   && Y442Zc    [1] && Y444Zc    ;
    assign Y470Zc      = Y469Zc         && Y438Zc    ;
    assign Y471Zc     = Y469Zc         && !Y438Zc    ;
    assign Y464Zc     = Y469Zc         ? Y437Zc    : Y458Zc;
    assign Y472Zc           = Y471Zc     || (Y459Zc    && Y444Zc    );
    assign Y465Zc        = Y472Zc           ? Y471Zc     : Y459Zc   ;
    assign Y473Zc            = Y470Zc      || (Y460Zc     && Y444Zc    );
    assign Y466Zc         = Y470Zc     ;
    always@(Y437Zc   [1:0] or Y439Zc   )
    begin
        if(Y439Zc    == 3'b000) begin
            case (Y437Zc   [1:0])
                2'b00   : Y462Zc        = 4'b0001;
                2'b01   : Y462Zc        = 4'b0010;
                2'b10   : Y462Zc        = 4'b0100;
                default : Y462Zc        = 4'b1000;
            endcase
        end
        else if (Y439Zc    == 3'b001) begin
                Y462Zc        = Y437Zc   [1] ? 4'b1100 : 4'b0011;
        end
        else begin
                Y462Zc        = 4'b1111;
        end
    end
    assign Y467Zc        = Y472Zc           | Y473Zc            ? 
                                    Y462Zc        : Y461Zc   ;
    assign Y468Zc         = Y472Zc           | Y473Zc            ?
                                    Y441Zc   [1] : Y463Zc    ;
    always@(posedge i_s_hclk or negedge i_s_hresetn)
    begin
        if(!i_s_hresetn) begin
            Y458Zc      <=  {Y434Zc          {1'b0}}  ;
            Y459Zc      <=  1'b0                      ;
            Y460Zc      <=  1'b0                      ;
            Y461Zc      <=  4'd0                      ;
            Y463Zc      <=  1'b0                      ;
        end
        else begin 
            Y458Zc      <=  Y464Zc                    ;
            Y459Zc      <=  Y465Zc                    ;
            Y460Zc      <=  Y466Zc                    ;
            Y461Zc      <=  Y467Zc                    ;
            Y463Zc      <=  Y468Zc                    ;
        end
    end
    assign Y474Zc        = (Y465Zc        && 
                                ({Y437Zc   [Y434Zc          -1:2],2'b00}==Y456Zc          ));
    assign Y449Zc         = Y458Zc       ;
    assign A176Yc         = Y445Zc       ;
    assign Y450Zc         = Y459Zc       ;
    assign Y451Zc         = Y460Zc       ;
    assign Y452Zc         = Y461Zc       ; 
    assign Y453Zc         = Y463Zc       ;
    assign Y448Zc         = Y454Zc       ;
    assign Y446Zc         = 1'b1         ;
    assign Y447Zc         = 1'b0         ;
    assign Y455Zc      = Y474Zc    ;
endmodule
module osr_trng_Y475Zc      
#(
    parameter    A411Yc                             =    1                   
   ,parameter    A412Yc                             =    1                   
   ,parameter    A573Yc                             =    0                           
   ,parameter    Y434Zc                             =    20
   ,parameter    Y435Zc                             =    32
   ,parameter    Y476Zc                             =    7
   ,parameter    Y477Zc                             =    7
   ,parameter    Y478Zc                             =    8
   ,parameter    Y479Zc                             =    32'h000D0100
   ,parameter    A005Yc                             =    20
   ,parameter    A006Yc                             =    20'hFFFFF   
)(
    input   wire                                    i_s_hclk     
   ,input   wire                                    i_s_hresetn  
   ,input   wire                                    Y436Zc       
   ,input   wire    [Y434Zc          -1:0]          Y437Zc       
   ,input   wire                                    Y438Zc       
   ,input   wire    [2:0]                           Y439Zc       
   ,input   wire    [2:0]                           Y440Zc       
   ,input   wire    [3:0]                           Y441Zc       
   ,input   wire    [1:0]                           Y442Zc       
   ,input   wire                                    Y443Zc       
   ,input   wire                                    Y444Zc       
   ,input   wire    [Y435Zc          -1:0]          Y445Zc       
   ,output  wire                                    Y446Zc       
   ,output  wire                                    Y447Zc       
   ,output  wire    [Y435Zc          -1:0]          Y448Zc    
   ,output  wire                                    Y480Zc 
   ,output  wire    [3:0]                           Y481Zc 
   ,output  wire                                    Y482Zc
   ,input   wire                                    Y483Zc
   ,input   wire                                    Y484Zc
   ,input   wire                                    Y485Zc 
   ,input   wire                                    A067Yc    
   ,input   wire                                    Y486Zc            
   ,input   wire    [Y435Zc          -1:0]          Y487Zc  
   ,output  wire                                    Y488Zc  
   ,output  wire                                    Y489Zc  
   ,output  wire                                    Y490Zc  
   ,output  wire                                    Y491Zc   
   ,output  wire                                    Y492Zc    
   ,output  wire                                    Y493Zc    
   ,output  wire                                    Y494Zc  
   ,output  wire                                    Y495Zc  
   ,input   wire                                    Y496Zc    
   ,input   wire                                    Y497Zc    
   ,input   wire                                    Y498Zc   
   ,input   wire                                    Y499Zc   
   ,input   wire                                    Y500Zc  
   ,input   wire                                    Y501Zc   
   ,input   wire                                    Y502Zc   
   ,input   wire                                    Y503Zc   
   ,input   wire                                    Y504Zc 
   ,input   wire                                    Y505Zc   
   ,input   wire                                    Y506Zc      
   ,input   wire                                    Y507Zc      
   ,input   wire                                    Y508Zc     
   ,input   wire                                    Y509Zc      
   ,output  wire    [63:0]                          Y510Zc
   ,output  wire    [1:0]                           Y511Zc
   ,input   wire                                    Y512Zc              
   ,input   wire                                    A720Yc              
   ,output  wire                                    Y513Zc   
   ,output  wire                                    Y514Zc     
   ,output  wire    [9:0]                           Y515Zc    
   ,output  wire    [9:0]                           Y516Zc      
   ,output  wire    [15:0]                          Y517Zc     
   ,output  wire    [31:0]                          Y518Zc      
   ,output  wire    [11:0]                          Y519Zc       
   ,output  wire    [23:0]                          Y520Zc       
   ,input   wire    [9:0]                           Y521Zc               
   ,input   wire    [3:0]                           Y522Zc         
   ,input   wire    [63:0]                          A172Yc
   ,input   wire    [3:0]                           A173Yc 
   ,input   wire    [1:0]                           Y523Zc
   ,input   wire                                    Y524Zc        
   ,input   wire                                    Y525Zc        
   ,output  wire                                    Y455Zc    
   ,output  wire                                    Y526Zc
);
    wire    [Y434Zc          -1:0]  Y527Zc               ;
    wire    [Y435Zc          -1:0]  Y528Zc               ;
    wire    [Y435Zc          -1:0]  Y529Zc               ;
    wire                            Y530Zc               ;
    wire                            Y531Zc               ;
    wire    [3:0]                   Y532Zc               ;
    wire                            Y533Zc               ;
    wire    [Y435Zc          -1:0]  Y534Zc               ;
    wire							Y535Zc           	 ;
osr_trng_Y433Zc     
    #(
        .Y434Zc                     (Y434Zc             )
       ,.Y435Zc                     (Y435Zc             )
    )
    Y536Zc       
    (
        .i_s_hclk                   (i_s_hclk           )
       ,.i_s_hresetn                (i_s_hresetn        )
       ,.Y436Zc                     (Y436Zc             )
       ,.Y437Zc                     (Y437Zc             )
       ,.Y438Zc                     (Y438Zc             )
       ,.Y439Zc                     (Y439Zc             )
       ,.Y440Zc                     (Y440Zc             )
       ,.Y441Zc                     (Y441Zc             )
       ,.Y442Zc                     (Y442Zc             )
       ,.Y443Zc                     (Y443Zc             )
       ,.Y444Zc                     (Y444Zc             )
       ,.Y445Zc                     (Y445Zc             )
       ,.Y446Zc                     (Y446Zc             )
       ,.Y447Zc                     (Y447Zc             )
       ,.Y448Zc                     (Y448Zc             )
       ,.Y449Zc                     (Y527Zc             )
       ,.A176Yc                     (Y528Zc             )
       ,.Y450Zc                     (Y530Zc             )
       ,.Y451Zc                     (Y531Zc             )
       ,.Y452Zc                     (Y532Zc             )
       ,.Y453Zc                     (Y533Zc             )
       ,.Y454Zc                     (Y534Zc             )
       ,.Y455Zc                     (Y535Zc             )
    );
osr_trng_ahb_reg_bank 
# ( 
.p_CONFIG_DRBG_AES_EN (A411Yc              ) 
, .p_CONFIG_DRBG_SM4_EN (A412Yc              ) 
, .p_CONFIG_DRBG_LFSR_EN (A573Yc               ) 
, .p_AHB_ADDR_WIDTH (Y434Zc           ) 
, .p_AHB_DATA_WIDTH (Y435Zc           ) 
, .p_TFIFO_TV_WIDTH (Y476Zc           ) 
, .p_DFIFO_TV_WIDTH (Y477Zc           ) 
, .p_TERO_SOURCE_CNT (Y478Zc            ) 
, .p_VERSION (Y479Zc    ) 
, .P_SM_CNT_P_WIDTH (A005Yc           ) 
, .P_SM_CNT_P_OVERFLOW (A006Yc             ) 
) 
Y538Zc              
( 
.clk (i_s_hclk ) 
, .rst_n (i_s_hresetn ) 
, .i_addr (Y527Zc        ) 
, .i_wdata (Y528Zc         ) 
, .i_read_en (Y530Zc           ) 
, .i_write_en (Y531Zc            ) 
, .i_wstrobe (Y532Zc           ) 
, .i_priority (Y533Zc            ) 
, .o_rdata (Y534Zc         ) 
, .o_rngen (Y480Zc  ) 
, .o_rosen (Y481Zc  ) 
, .o_msel (Y482Zc ) 
, .i_htf (Y483Zc ) 
, .i_drdy (Y484Zc ) 
, .i_ererr (Y485Zc  ) 
, .i_trng_rdy (A067Yc     ) 
, .i_drbg_rdy_for_cfg (Y486Zc             ) 
, .i_rng_dr (Y487Zc   ) 
, .o_reseed (Y488Zc   ) 
, .o_trcten (Y489Zc   ) 
, .o_tapten (Y490Zc   ) 
, .o_trunsen (Y491Zc    ) 
, .o_taptnben (Y492Zc     ) 
, .o_tpokeren (Y493Zc     ) 
, .o_drcten (Y494Zc   ) 
, .o_drpten (Y495Zc   ) 
, .i_trrunsbf (Y498Zc    ) 
, .i_trpokerbf (Y497Zc     ) 
, .i_taptnbbf (Y496Zc     ) 
, .i_taptbbf (Y499Zc    ) 
, .i_trctbf (Y500Zc   ) 
, .i_trrunsf (Y503Zc    ) 
, .i_trpokerf (Y501Zc    ) 
, .i_tratnbf (Y502Zc    ) 
, .i_taptf (Y504Zc  ) 
, .i_trctf (Y505Zc  ) 
, .i_drptbf (Y506Zc   ) 
, .i_drctbf (Y507Zc   ) 
, .i_drptf (Y508Zc  ) 
, .i_drctf (Y509Zc  ) 
, .o_roen (Y510Zc ) 
, .o_fsel (Y511Zc ) 
, .i_drbg_mode (Y512Zc      ) 
, .i_cbc_sm4 (A720Yc    ) 
, .o_cbc_sm4 (Y513Zc    ) 
, .o_drbg_mode (Y514Zc      ) 
, .o_tapt_win (Y515Zc     ) 
, .o_tpoker_win (Y516Zc       ) 
, .o_tapt_thld (Y517Zc      ) 
, .o_truns_thld (Y518Zc       ) 
, .o_taptnb_thld (Y519Zc        ) 
, .o_tpoker_thld (Y520Zc        ) 
, .i_trht_iteration_ptr (Y521Zc               ) 
, .i_rnvld_num (Y522Zc      ) 
, .i_fifo_pop (Y535Zc            ) 
, .o_irq (Y526Zc ) 
, .i_roen (A172Yc ) 
, .i_rosen (A173Yc  ) 
, .i_fsel (Y523Zc ) 
, .i_trfifo_empty (Y524Zc         ) 
, .i_drfifo_empty (Y525Zc         ) 
, .o_fifo_pop (Y455Zc     ) 
); 
endmodule
module osr_trng_A178Yc    #(
    parameter           A179Yc                      =   10                  ,       
    parameter           A180Yc                      =   32                  
    )(
    output wire [A180Yc     -1:0]                       A187Yc              ,
    output wire                                         A188Yc              ,
    output wire                                         A189Yc              ,
    output wire                                         A190Yc              ,
    output wire [A179Yc      :0]                        A191Yc              ,
    input  wire [A179Yc      -1:0]                      A182Yc              ,
    input  wire [A180Yc     -1:0]                       A183Yc              ,
    input  wire                                         A184Yc              ,
    input  wire                                         A185Yc              ,
    input  wire                                         A186Yc              ,
    input  wire                                         clk                 ,
    input  wire                                         resetn              
    );
    localparam  Y552Zc                              =   1<<A179Yc           ;
    reg         [A180Yc     -1:0]                       Y553Zc   [0:Y552Zc  -1];
    reg         [A179Yc      -1:0]                      Y554Zc              ;
    reg         [A179Yc      -1:0]                      Y555Zc              ;
    reg         [A179Yc      :0]                        Y556Zc              ;
    reg         [A179Yc      :0]                        Y557Zc              ;
    reg         [A180Yc     -1:0]                       Y558Zc              ;
    wire        [A179Yc      :0]                        Y559Zc              ;
    integer                                             Y560Zc              ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            begin
            Y558Zc                                  <=  0                   ;
            Y554Zc                                  <=  0                   ;
            Y555Zc                                  <=  0                   ;
            Y556Zc                                  <=  0                   ;
            for(Y560Zc  =0;Y560Zc  <Y552Zc  ;Y560Zc  =Y560Zc  +1)
                Y553Zc  [Y560Zc  ]                  <=  0                   ;
            end
        else if (A186Yc )
            begin
            Y558Zc                                  <= 0                 ;
            Y554Zc                                  <= 0                 ;
            Y555Zc                                  <= 0                 ;
            Y556Zc                                  <= 0                 ;
            for(Y560Zc  =0;Y560Zc  <Y552Zc  ;Y560Zc  =Y560Zc  +1)
                Y553Zc  [Y560Zc  ]                  <= 0                 ;
            end
        else if (A185Yc      && !A188Yc && !A184Yc     )
            begin
            Y553Zc  [Y555Zc  ]                      <= A183Yc            ;
            Y555Zc                                  <= Y555Zc   + 1'b1   ;
            Y556Zc                                  <= Y556Zc+ 1'b1      ;
            end
        else if (A184Yc      && !A189Yc  && !A185Yc     )
            begin
            Y558Zc                                  <= Y553Zc  [Y554Zc  ];
            Y554Zc                                  <= Y554Zc   + 1'b1   ;
            Y556Zc                                  <= Y556Zc- 1'b1      ;
            end
        else if (A184Yc      && A185Yc      && A189Yc )
            begin
            Y553Zc  [Y555Zc  ]                      <= A183Yc            ;
            Y555Zc                                  <= Y555Zc   + 1'b1   ;
            Y556Zc                                  <= Y556Zc+ 1'b1      ;
            end
        else if (A184Yc      && A185Yc      && A188Yc)
            begin
            Y558Zc                                  <= Y553Zc  [Y554Zc  ];
            Y554Zc                                  <= Y554Zc   + 1'b1   ;
            Y556Zc                                  <= Y556Zc- 1'b1      ;
            end
        else if (A184Yc      && A185Yc      && !A189Yc  && !A188Yc)
            begin
            Y553Zc  [Y555Zc  ]                      <= A183Yc            ;
            Y558Zc                                  <= Y553Zc  [Y554Zc  ];
            Y555Zc                                  <= Y555Zc   + 1'b1   ;
            Y554Zc                                  <= Y554Zc   + 1'b1   ;
            end
        else;
    always @ (posedge clk or negedge resetn) begin
        if (~resetn) begin
            Y557Zc                                  <=  {(A179Yc       +1){1'b0}};
            end
        else begin
            Y557Zc                                  <= Y556Zc            ;
            end
        end
    assign  Y559Zc              =   A182Yc         +1 + {(A179Yc       +1){1'b0}}  ;
    assign  A188Yc      =   Y556Zc == (2**A179Yc      )  ;
    assign  A189Yc      =   Y556Zc == 0                  ;
    assign  A187Yc      =   Y558Zc                      ;
    assign  A191Yc      =   Y556Zc                      ;
    assign  A190Yc      =   Y556Zc  ==  Y559Zc          
                        &&  Y557Zc  ==  {1'b0,A182Yc         } ;
endmodule
module osr_trng_Y561Zc     
#(
    parameter   Y562Zc                      =  32 
)(
    input   wire                            Y563Zc     
   ,input   wire    [Y562Zc         -1:0]   Y445Zc     
   ,input   wire    [Y562Zc         -1:0]   Y564Zc     
   ,output  wire    [Y562Zc         -1:0]   Y565Zc     
   ,output  wire    [Y562Zc         -1:0]   Y448Zc     
);
    genvar A282Yc;
    generate
        for (A282Yc=0;A282Yc<4;A282Yc=A282Yc+1) begin : Y566Zc    
            assign Y448Zc    [A282Yc*8+:8] = !Y563Zc   ? 
                    Y564Zc    [A282Yc*8+:8] : Y564Zc    [Y562Zc         -1-A282Yc*8-:8];
            assign Y565Zc    [A282Yc*8+:8] = !Y563Zc   ? 
                    Y445Zc    [A282Yc*8+:8] : Y445Zc    [Y562Zc         -1-A282Yc*8-:8];
        end
    endgenerate
endmodule
module osr_trng_Y567Zc   # (
    parameter   A180Yc                  =   32              ,
    parameter   Y568Zc                  =   11              ,
    parameter   Y569Zc                  =   5               
    )
    (
    input  wire                             clk                   ,
    input  wire                             resetn                ,
    input  wire                             A487Yc                ,
    input  wire                             Y570Zc                ,
    input  wire                             Y571Zc                ,
    output wire                             Y572Zc                ,
    output wire                             Y573Zc                ,
    output wire                             Y574Zc                ,
    input  wire                             Y575Zc                ,
    output wire [A180Yc     -1:0]           Y576Zc                ,
    output wire [Y569Zc          :0]        Y577Zc                ,
    output wire [9:0]                       Y578Zc                ,
    input   wire                            Y579Zc              ,
    input   wire                            Y580Zc              ,
    input   wire                            Y581Zc              ,
    input   wire                            Y582Zc              ,
    input   wire                            Y583Zc              ,
    input   wire                            Y584Zc              ,
    input   wire                            Y585Zc              ,
    output wire                             Y586Zc                ,
    output wire [A180Yc     -1:0]           Y587Zc                ,
    output wire                             Y588Zc                ,
    output wire                             Y589Zc                ,
    output wire                             Y590Zc                ,
    output wire                             Y591Zc                ,
    output wire                             Y592Zc                ,
    output wire [Y568Zc          -1:0]      Y593Zc                ,
    input  wire [A180Yc     -1:0]           Y594Zc                ,   
    output wire                             Y595Zc                ,   
    input  wire                             Y596Zc                ,   
    input  wire                             Y597Zc                ,   
    input  wire                             Y598Zc                ,   
    input  wire [Y568Zc          :0]        Y599Zc                ,
    input  wire [9:0]                       Y600Zc                ,
    input  wire                             Y601Zc                ,
    input  wire                             Y602Zc                ,
    input  wire                             Y603Zc                ,
    input  wire                             Y604Zc                ,
    input  wire                             Y605Zc                ,
    input  wire                             Y606Zc                ,
    output wire                             Y607Zc                ,
    output wire                             Y608Zc                , 
    input  wire                             Y609Zc                ,
    input  wire                             Y610Zc                ,
    output  wire                            Y611Zc                ,
    output  wire                            Y612Zc                ,
    output  wire                            Y613Zc                ,
    output  wire                            Y614Zc                ,
    output  wire                            Y615Zc                ,
    output wire                             Y616Zc                ,
    output wire                             Y617Zc                ,
    input  wire                             Y618Zc                ,  
    input  wire                             Y619Zc                ,
    output wire                             Y620Zc                ,
    output wire                             Y621Zc                ,
    input  wire                             Y622Zc                ,
    input  wire                             Y623Zc                ,
    output wire                             Y624Zc                ,
    output wire [Y569Zc          -1:0]      Y625Zc                ,
    input  wire                             Y626Zc                ,
    input  wire                             Y627Zc                ,
    input  wire                             Y628Zc                ,
    input  wire [Y569Zc          :0]        Y629Zc                ,
    input  wire [9:0]                       Y630Zc                ,
    input  wire [A180Yc     -1:0]           Y631Zc                ,
    output wire                             Y632Zc                ,
    output wire [A180Yc     -1:0]           Y633Zc                ,
    input  wire                             Y634Zc                ,
    input   wire                            Y635Zc                ,
    input   wire                            Y636Zc                ,
    output  wire                            Y637Zc                ,
    output  wire                            Y638Zc                ,
    output  wire                            Y639Zc                ,
    output wire                             Y640Zc                ,
    output wire                             Y641Zc                ,
    input  wire                             Y642Zc                ,
    input  wire                             Y643Zc                ,
    output wire [A180Yc     -1:0]           Y644Zc                ,
    input  wire [A180Yc     -1:0]           Y645Zc                ,
    input  wire                             Y646Zc                ,
    input  wire                             Y647Zc                ,
    input  wire                             Y648Zc                ,
    output wire                             Y649Zc                ,
    output wire                             Y650Zc                ,
    output wire [A180Yc     -1:0]           Y651Zc                ,
    input  wire                             Y652Zc                ,
    output wire                             Y653Zc                ,
    output wire                             Y654Zc                ,
    output wire                             Y655Zc                
    );
    localparam  A342Yc                  =   4               ;
    localparam  [A342Yc     -1:0]
                A343Yc                  =   4'd0  + {A342Yc     {1'b0}},
                Y656Zc                  =   4'd1  + {A342Yc     {1'b0}},
                Y657Zc                  =   4'd2  + {A342Yc     {1'b0}},
                Y658Zc                  =   4'd3  + {A342Yc     {1'b0}},
                Y659Zc                  =   4'd4  + {A342Yc     {1'b0}},
                Y660Zc                  =   4'd5  + {A342Yc     {1'b0}},
                A565Yc                  =   4'd7  + {A342Yc     {1'b0}},
                Y661Zc                  =   4'd10 + {A342Yc     {1'b0}},
                Y662Zc                  =   4'd11 + {A342Yc     {1'b0}};
    reg     [A342Yc     -1:0]           A283Yc              ;
    reg     [A342Yc     -1:0]           A284Yc              ;
    wire                                Y663Zc              ;
    wire                                Y664Zc              ;
    wire                                Y665Zc              ;
    wire                                Y666Zc              ;
    wire                                Y667Zc              ;
    wire                                Y668Zc              ;
    wire                                Y669Zc              ;
    wire                                Y670Zc              ;
    wire                                Y671Zc              ;
    wire                                Y672Zc              ;
    wire                                Y673Zc              ;
    wire                                Y674Zc              ;
    reg     [1:0]                       Y675Zc              ;
    wire                                Y676Zc              ;
    wire                                Y677Zc              ;
    wire                                Y678Zc              ;
    wire                                Y679Zc              ;
    wire                                Y680Zc              ;
    reg                                 Y681Zc              ;
    reg                                 Y682Zc              ;
    reg                                 Y683Zc              ;
    reg                                 Y684Zc              ;
    reg                                 Y685Zc              ;
    reg                                 Y686Zc              ;
    reg                                 Y687Zc              ;
    reg                                 Y688Zc              ;
    wire                                Y689Zc              ;
    reg     [1:0]                       Y690Zc              ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            A283Yc                  <=  A343Yc              ;
        else if (!Y570Zc     )
            A283Yc                  <=  A343Yc            ;
        else
            A283Yc                  <=  A284Yc            ;
    always @ (*)
        begin
        A284Yc                      =   A343Yc              ;
        case (A283Yc       )
            A343Yc:
                if (Y570Zc     )
                    A284Yc          =   Y656Zc              ;
                else
                    A284Yc          =   A343Yc              ;
            Y656Zc    :
                begin
                if (A487Yc     )
                    A284Yc          =   Y661Zc              ;
                else
                    A284Yc          =   Y657Zc              ;
                end
            Y657Zc   :
                begin
                if (Y673Zc         )
                    A284Yc          =   Y662Zc              ;
                else if (Y674Zc         )
                    A284Yc          =   Y658Zc              ;
                else
                    A284Yc          =   Y657Zc              ;
                end
            Y658Zc      :
                A284Yc              =   Y659Zc              ;
            Y659Zc   :
                begin
                if (Y643Zc        )
                    A284Yc          =   Y662Zc              ;
                else if (Y642Zc        )
                    A284Yc          =   Y660Zc              ;
                else
                    A284Yc          =   Y659Zc              ;
                end
            Y660Zc:
                A284Yc              =   A565Yc              ;
            A565Yc    :
                if (Y622Zc    )
                    A284Yc          =   Y661Zc              ;
                else
                    A284Yc          =   A565Yc              ;
            Y661Zc:
                if (Y677Zc    | Y678Zc     )
                    A284Yc          =   Y662Zc              ;
                else
                    A284Yc          =   Y661Zc              ;
            Y662Zc:
                A284Yc              =   Y662Zc              ;
        endcase
        end
    assign  Y672Zc                  =   A284Yc     == Y656Zc    ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
        begin
            Y682Zc                  <=  0                   ;
            Y683Zc                  <=  0                   ;
            Y684Zc                  <=  0                   ;
            Y685Zc                  <=  0                   ;
            Y686Zc                  <=  0                   ;
            Y687Zc                  <=  0                   ;
            Y688Zc                  <=  0                   ;
        end
        else if (Y672Zc         )
        begin
            Y682Zc                  <=  Y579Zc            ;
            Y683Zc                  <=  Y580Zc            ;
            Y684Zc                  <=  Y581Zc            ;
            Y685Zc                  <=  Y582Zc            ;
            Y686Zc                  <=  Y583Zc            ;
            Y687Zc                  <=  Y584Zc            ;
            Y688Zc                  <=  Y585Zc            ;
        end
        else;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            Y675Zc                  <=  2'b00               ;
        else if (Y669Zc          )
            begin
            if (Y609Zc              & Y619Zc          )
                Y675Zc              <=  2'b11             ;
            else if (Y609Zc             )
                Y675Zc             [0]<=  1'b1            ;
            else if (Y619Zc          )
                Y675Zc             [1]<=  1'b1            ;
            else
                Y675Zc              <=  Y675Zc             ;
            end
        else
            Y675Zc                  <=  2'b00             ;
    assign  Y664Zc                  =   Y571Zc              ;
    assign  Y677Zc                  =   Y602Zc          | Y603Zc          | Y604Zc           | Y605Zc            | Y606Zc           ;
    assign  Y678Zc                  =   Y635Zc          | Y636Zc         ;
    assign  Y674Zc                  =   & Y675Zc             ;
    assign  Y676Zc                  =   A283Yc        == Y660Zc;
    assign  Y673Zc                  =   Y610Zc              | Y618Zc             ;
    assign  Y665Zc                  =   A283Yc        == A565Yc    ;
    assign  Y666Zc                  =   A283Yc        == Y661Zc;
    assign  Y671Zc                  =   A283Yc        == Y662Zc;
    assign  Y667Zc                  =   Y666Zc    & ~Y664Zc ;
    assign  Y668Zc                  =   Y666Zc    & Y664Zc  ;
    assign  Y669Zc                  =   A284Yc     == Y657Zc   ;
    assign  Y670Zc                  =   A283Yc        == Y659Zc   ;
    assign  Y653Zc                  =   (A284Yc     == Y658Zc      ) && (A283Yc        == Y657Zc   );
    assign  Y663Zc                  =   Y570Zc      & ~Y671Zc     ;
    assign  Y576Zc                  =   (Y667Zc         ? Y594Zc           : 0)
                                    |   (Y668Zc         ? Y631Zc           : 0);
    assign  Y577Zc                  =   (Y667Zc         ? Y599Zc              : 0)
                                    |   (Y668Zc         ? Y629Zc              : 0);
    assign  Y578Zc                  =   (Y667Zc         ? Y600Zc               : 10'b0)
                                    |   (Y668Zc         ? Y630Zc               : 10'b0);
    assign  Y679Zc                  =   (Y667Zc         ? Y597Zc            : 1'b1)
                                    &   (Y668Zc         ? Y626Zc            : 1'b1);
    assign  Y572Zc                  =   Y575Zc           & Y679Zc        ;
    assign  Y680Zc                  =   (Y667Zc         ? (Y598Zc              | Y596Zc          ) : 1'b0)
                                    |   (Y668Zc         ? (Y628Zc              | Y627Zc          ): 1'b0);
    assign  Y573Zc                  =   Y680Zc            && !Y601Zc               ;
    assign  Y574Zc                  = (Y666Zc    ? (Y682Zc   | Y683Zc   | Y684Zc     | Y685Zc     | Y686Zc     | Y687Zc   | Y688Zc  ) : 1'b1) & Y671Zc     ;
    assign  Y611Zc                  =   Y682Zc      & Y663Zc    ;
    assign  Y612Zc                  =   Y683Zc      & Y663Zc    ;
    assign  Y613Zc                  =   Y684Zc      & Y663Zc    ;
    assign  Y614Zc                  =   Y685Zc      & Y663Zc    ;
    assign  Y615Zc                  =   Y686Zc      & Y663Zc    ;
    assign  Y637Zc                  =   Y687Zc      & Y663Zc    ;
    assign  Y638Zc                  =   Y688Zc      & Y663Zc    ;
    assign  Y639Zc                  =   Y665Zc         ;
    assign  Y586Zc                  =   Y663Zc              ;
    assign  Y587Zc                  =   Y645Zc              ;
    assign  Y588Zc                  =   Y646Zc               ;
    assign  Y589Zc                  =   Y647Zc               ;
    assign  Y590Zc                  =   Y648Zc                ;
    assign  Y591Zc                  =   A283Yc        == Y659Zc   ;
    assign  Y592Zc                  =   Y676Zc              ;
    assign  Y593Zc                  =   {Y568Zc          {1'b1}};
    assign  Y689Zc            = 
            (Y670Zc        ? (Y634Zc          | Y647Zc               ) : 1'b0)
                                    |   (Y667Zc         ? Y575Zc           : 1'b0)
                                    |   (Y668Zc         ? Y634Zc          : 1'b0)
                                    |   (Y665Zc          ? Y634Zc          : 1'b0); 
    assign  Y595Zc                  =   Y689Zc           ;
    assign  Y608Zc                  =   Y642Zc              ;
    assign  Y607Zc                  =   A487Yc              ;
    assign  Y616Zc                  =   Y663Zc              ;
    assign  Y617Zc                  =   A487Yc              ;
    assign  Y620Zc                  =    Y668Zc         ;
    assign  Y621Zc                  =   Y642Zc              ;
    assign  Y624Zc                  =   (Y670Zc        ? Y652Zc                : 1'b0)
                                    |   (Y668Zc         ? Y575Zc           : 1'b0);
    assign  Y625Zc                  =   {Y569Zc          {1'b1}};
    assign  Y632Zc                  =   (Y665Zc          | Y668Zc         | Y670Zc       ) ? 
                                        Y597Zc            : 1'b1;
    assign  Y633Zc                  =   Y594Zc              ;
    assign  Y640Zc                  =   A283Yc        == Y658Zc      ;
    assign  Y641Zc                  =   Y663Zc              ;
    assign  Y644Zc                  =   Y594Zc              ;
    assign  Y650Zc                  =   Y626Zc              ;
    assign  Y651Zc                  =   Y631Zc              ;
    assign  Y649Zc                  =   Y623Zc               ;
    assign  Y654Zc                  =   Y666Zc            ;
    always @ (posedge clk or negedge resetn)
        if (!resetn)
            Y681Zc                  <=  1'b0              ;
        else if (A284Yc     == Y662Zc)
            Y681Zc                  <=  1'b1              ;
        else
            Y681Zc                  <=  1'b0              ;
    assign  Y655Zc                  =   Y681Zc            ;
endmodule
