//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_to_ram #(
    parameter  P_BUS_AW         = 19    ,
    parameter  P_BUS_NO_WORD    = 1     ,
    parameter  P_AHB2RAM_TIMING = 0     ,
    parameter  P_AW_DEL         = 2      
    )(

    input  wire                           i_hclk              ,
    input  wire                           i_hresetn           ,

    input  wire [7:0]                     i_ecc_err_rsp_en    , 
    input  wire                           i_ecc_dec_sec       ,
    input  wire                           i_ecc_dec_ded       ,

    input  wire                           i_hsel              ,
    input  wire                           i_hready            ,
    input  wire [P_BUS_AW-1:0]            i_haddr             ,
    input  wire [1:0]                     i_htrans            ,
    input  wire                           i_hwrite            ,
    input  wire [2:0]                     i_hsize             ,
    input  wire [2:0]                     i_hburst            ,
    input  wire [3:0]                     i_hprot             ,
    input  wire [3:0]                     i_hmaster           ,
    input  wire [31:0]                    i_hwdata            ,
    input  wire                           i_hmastlock         ,

    output wire [31:0]                    o_hrdata            ,
    output wire                           o_hreadyout         ,
    output wire [1:0]                     o_hresp             ,

    output wire                           o_ram_cs            , 
    output wire [3:0]                     o_ram_wen           ,  
    output wire [P_BUS_AW-P_AW_DEL-1:0]   o_ram_addr          , 
    output wire [31:0]                    o_ram_wdata         , 
    input  wire [31:0]                    i_ram_rdata          

);

wire       ahb_access ; 
wire       ahb_write  ; 
wire       ahb_read   ; 
wire       ram_write  ;
wire       tx_byte    ;
wire       tx_half    ;
wire       tx_word    ;
wire       byte_at_00 ;
wire       byte_at_01 ;
wire       byte_at_10 ;
wire       byte_at_11 ;
wire       half_at_00 ;
wire       half_at_10 ;
wire       word_at_00 ;
wire       byte_sel_0 ;
wire       byte_sel_1 ;
wire       byte_sel_2 ;
wire       byte_sel_3 ;              
wire [7:0] buf_data0  ;                             
wire [7:0] buf_data1  ;                             
wire [7:0] buf_data2  ;                             
wire [7:0] buf_data3  ;
wire [7:0] buf_wr_pend_data0;                             
wire [7:0] buf_wr_pend_data1;                             
wire [7:0] buf_wr_pend_data2;                             
wire [7:0] buf_wr_pend_data3;
wire [3:0] merge      ;

reg [31:0]                  ram_rdata_dly          ;
reg                         buf_wr_pend            ;
reg                         buf_wr_word_flag       ;
reg                         buf_wr_pend_lck        ;
reg                         buf_pend               ;
reg                         buf_pend_dly           ;
reg                         wr_ready               ;
reg [3:0]                   buf_we                 ;
reg                         buf_data_en            ;
reg                         buf_byte_sel0          ;
reg                         buf_byte_sel1          ;
reg                         buf_byte_sel2          ;
reg                         buf_byte_sel3          ;
reg [31:0]                  buf_data               ;
reg [31:0]                  buf_wr_pend_data       ;
reg [P_BUS_AW-P_AW_DEL-1:0] buf_addr               ;
reg                         buf_hit                ;
reg                         buf_wr_ready           ;
reg                         ecc_dec_ded            ;
reg                         buf_ecc_dec_ded        ;
reg                         buf_ahb_access         ;
reg                         buf_ahb_write          ;
reg                         write_done             ;
reg                         buf_ahb_read           ;
reg [3:0]                   buf_merge              ;

wire [31:0] ram_rdata_dly_next = i_ram_rdata; 
wire buf_wr_pend_next = (ahb_write & !tx_word & P_BUS_NO_WORD);
wire buf_wr_word_flag_next = (buf_data_en & !buf_wr_pend); 
wire buf_wr_pend_lck_next = (buf_data_en & buf_wr_pend & (buf_wr_word_flag || buf_pend_dly )); 
wire buf_pend_next = (buf_pend | buf_data_en) & (ahb_read);
wire buf_pend_dly_next = buf_pend;
wire wr_ready_next = ram_write & (buf_wr_pend || buf_wr_pend_lck);
wire [3:0] buf_we_next = ahb_write ? { byte_sel_3 & ahb_write,byte_sel_2 & ahb_write,byte_sel_1 & ahb_write,byte_sel_0 & ahb_write } : buf_we;
wire buf_data_en_next = ahb_write;   
wire buf_byte_sel0_next = buf_wr_pend_next ? byte_sel_0 : buf_byte_sel0;
wire buf_byte_sel1_next = buf_wr_pend_next ? byte_sel_1 : buf_byte_sel1;
wire buf_byte_sel2_next = buf_wr_pend_next ? byte_sel_2 : buf_byte_sel2;
wire buf_byte_sel3_next = buf_wr_pend_next ? byte_sel_3 : buf_byte_sel3;
wire [31:0] buf_data_next = {buf_data3,buf_data2,buf_data1,buf_data0};
wire [31:0] buf_wr_pend_data_next = {buf_wr_pend_data3,buf_wr_pend_data2,buf_wr_pend_data1,buf_wr_pend_data0};
wire [P_BUS_AW-P_AW_DEL-1:0] buf_addr_next = ahb_write ? i_haddr[(P_BUS_AW-1):P_AW_DEL] : buf_addr;
wire buf_hit_next = i_haddr[(P_BUS_AW-1):P_AW_DEL] == buf_addr[P_BUS_AW-P_AW_DEL-1:0];

wire buf_wr_ready_next = wr_ready;
wire ecc_dec_ded_next = i_ecc_dec_ded;
wire buf_ecc_dec_ded_next = ecc_dec_ded;

wire buf_ahb_access_next = ahb_access;
wire buf_ahb_write_next = ahb_write;
wire write_done_next = (buf_ahb_write & !ahb_access) ? 1'b1 : buf_ahb_access ? 1'b0 : write_done;

wire buf_ahb_read_next = P_AHB2RAM_TIMING ? ahb_read : 1'b0 ;
wire [3:0] buf_merge_next =merge;

assign ahb_access = i_hsel & i_htrans[1] & i_hready ;
assign ahb_write  = ahb_access & i_hwrite;
assign ahb_read   = (ahb_access & !i_hwrite) || (ahb_write & buf_wr_pend_next & (!buf_pend & !buf_data_en)) || buf_wr_pend_lck_next;
assign ram_write  = (buf_pend | buf_data_en)  & (~ahb_read);
assign tx_byte    = (~i_hsize[1]) & (~i_hsize[0]);
assign tx_half    = (~i_hsize[1]) &  i_hsize[0];
assign tx_word    =   i_hsize[1];
assign byte_at_00 = tx_byte & (~i_haddr[1]) & (~i_haddr[0]);
assign byte_at_01 = tx_byte & (~i_haddr[1]) &   i_haddr[0];
assign byte_at_10 = tx_byte &   i_haddr[1]  & (~i_haddr[0]);
assign byte_at_11 = tx_byte &   i_haddr[1]  &   i_haddr[0];
assign half_at_00 = tx_half & (~i_haddr[1]);
assign half_at_10 = tx_half &   i_haddr[1];
assign word_at_00 = tx_word;
assign byte_sel_0 = word_at_00 | half_at_00 | byte_at_00;
assign byte_sel_1 = word_at_00 | half_at_00 | byte_at_01;
assign byte_sel_2 = word_at_00 | half_at_10 | byte_at_10;
assign byte_sel_3 = word_at_00 | half_at_10 | byte_at_11;
assign buf_data0  = (buf_we[0] & buf_data_en) ? i_hwdata[7:0]   : buf_data[7:0];
assign buf_data1  = (buf_we[1] & buf_data_en) ? i_hwdata[15:8]  : buf_data[15:8];
assign buf_data2  = (buf_we[2] & buf_data_en) ? i_hwdata[23:16] : buf_data[23:16];
assign buf_data3  = (buf_we[3] & buf_data_en) ? i_hwdata[31:24] : buf_data[31:24];
assign buf_wr_pend_data0 = wr_ready & buf_byte_sel0 ? buf_data[7:0]   : ram_rdata_dly[7:0];
assign buf_wr_pend_data1 = wr_ready & buf_byte_sel1 ? buf_data[15:8]  : ram_rdata_dly[15:8];
assign buf_wr_pend_data2 = wr_ready & buf_byte_sel2 ? buf_data[23:16] : ram_rdata_dly[23:16];
assign buf_wr_pend_data3 = wr_ready & buf_byte_sel3 ? buf_data[31:24] : ram_rdata_dly[31:24];

assign merge      = {4{buf_hit}} & buf_we & {4{!write_done}};

wire [31:0] ram_rdata = P_AHB2RAM_TIMING ? ram_rdata_dly : i_ram_rdata;
wire [3:0] rdata_merge = P_AHB2RAM_TIMING ? buf_merge : merge;
wire ecc_err = P_AHB2RAM_TIMING ? ecc_dec_ded : (i_ecc_dec_ded | ecc_dec_ded);

wire       hreadyout = (buf_wr_pend || buf_wr_pend_lck || wr_ready || buf_ahb_read || (ecc_err && (i_ecc_err_rsp_en != 8'h5a))) ? 1'b0 : 1'b1;
wire [1:0] hresp     = (i_ecc_err_rsp_en == 8'h5a) ? 2'b00 : 
                       ((ecc_dec_ded || buf_ecc_dec_ded) && !buf_wr_pend) ? 2'b01 :
                       (buf_wr_ready || wr_ready) ? {1'b0,(ecc_dec_ded || buf_ecc_dec_ded)}: 2'b00;

assign o_hrdata         = { rdata_merge[3] ? buf_data[31:24] : ram_rdata[31:24],
                            rdata_merge[2] ? buf_data[23:16] : ram_rdata[23:16],
                            rdata_merge[1] ? buf_data[15: 8] : ram_rdata[15: 8],
                            rdata_merge[0] ? buf_data[ 7: 0] : ram_rdata[ 7: 0] };
assign o_hreadyout      = hreadyout ;
assign o_hresp          = hresp     ;
assign o_ram_cs         = ahb_read || (ram_write && !wr_ready_next) || wr_ready;
assign o_ram_wen        = wr_ready_next ? 4'h0 : wr_ready ? 4'hf : {4{ram_write}} & buf_we[3:0];
assign o_ram_addr       = (ahb_read && buf_wr_pend) ? buf_addr : ahb_read ? i_haddr[P_BUS_AW-1:P_AW_DEL] : buf_addr;
assign o_ram_wdata      = !o_ram_cs ? 32'b0 :wr_ready ? buf_wr_pend_data_next : (buf_pend) ? buf_data : i_hwdata[31:0];

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        ram_rdata_dly          <= 32'h0                       ; 
        buf_wr_pend            <= 1'h0                        ; 
        buf_wr_word_flag       <= 1'b0                        ; 
        buf_wr_pend_lck        <= 1'b0                        ; 
        buf_pend               <= 1'b0                        ; 
        buf_pend_dly           <= 1'b0                        ; 
        wr_ready               <= 1'h0                        ; 
        buf_we                 <= 4'h0                        ; 
        buf_data_en            <= 1'h0                        ; 
        buf_byte_sel0          <= 1'h0                        ; 
        buf_byte_sel1          <= 1'h0                        ; 
        buf_byte_sel2          <= 1'h0                        ; 
        buf_byte_sel3          <= 1'h0                        ; 
        buf_data               <= 32'h0                       ; 
        buf_wr_pend_data       <= 32'h0                       ; 
        buf_addr               <= {{P_BUS_AW-P_AW_DEL}{1'b0}} ; 
        buf_hit                <= 1'b0                        ; 
        buf_wr_ready           <= 1'b0                        ; 
        ecc_dec_ded            <= 1'b0                        ; 
        buf_ecc_dec_ded        <= 1'b0                        ; 
        buf_ahb_access         <= 1'b0                        ; 
        buf_ahb_write          <= 1'b0                        ; 
        write_done             <= 1'b0                        ; 
        buf_ahb_read           <= 1'b0                        ; 
        buf_merge              <= 4'b0                        ; 
    end else begin
        ram_rdata_dly          <= ram_rdata_dly_next          ; 
        buf_wr_pend            <= buf_wr_pend_next            ; 
        buf_wr_word_flag       <= buf_wr_word_flag_next       ; 
        buf_wr_pend_lck        <= buf_wr_pend_lck_next        ; 
        buf_pend               <= buf_pend_next               ; 
        buf_pend_dly           <= buf_pend_dly_next           ; 
        wr_ready               <= wr_ready_next               ; 
        buf_we                 <= buf_we_next                 ; 
        buf_data_en            <= buf_data_en_next            ; 
        buf_byte_sel0          <= buf_byte_sel0_next          ; 
        buf_byte_sel1          <= buf_byte_sel1_next          ; 
        buf_byte_sel2          <= buf_byte_sel2_next          ; 
        buf_byte_sel3          <= buf_byte_sel3_next          ; 
        buf_data               <= buf_data_next               ; 
        buf_wr_pend_data       <= buf_wr_pend_data_next       ; 
        buf_addr               <= buf_addr_next               ; 
        buf_hit                <= buf_hit_next                ; 
        buf_wr_ready           <= buf_wr_ready_next           ; 
        ecc_dec_ded            <= ecc_dec_ded_next            ; 
        buf_ecc_dec_ded        <= buf_ecc_dec_ded_next        ; 
        buf_ahb_access         <= buf_ahb_access_next         ; 
        buf_ahb_write          <= buf_ahb_write_next          ; 
        write_done             <= write_done_next             ; 
        buf_ahb_read           <= buf_ahb_read_next           ; 
        buf_merge              <= buf_merge_next              ; 
    end
end

endmodule 
