//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_sync_fifo #(
    parameter [31:0] p_AWIDTH = 10 ,       

    parameter [31:0] p_DWIDTH = 32
    )(

    input  wire                  i_clk               ,
    input  wire                  i_rst_n             ,

    input  wire                  i_wr                ,
    input  wire [p_DWIDTH-1:0]   i_wr_data           ,
    output wire [p_AWIDTH  :0]   o_wr_num_count      ,
    output wire                  o_wr_full           ,
    output wire                  o_wr_full_n         ,
    output wire                  o_wr_al_full        ,
    output wire                  o_wr_al_full_n      ,

    input  wire                  i_rd                ,
    output wire [p_DWIDTH-1:0]   o_rd_data           ,
    output wire [p_AWIDTH  :0]   o_rd_num_count      ,
    output wire                  o_rd_empty          ,
    output wire                  o_rd_empty_n        ,
    output wire                  o_rd_al_empty       ,
    output wire                  o_rd_al_empty_n     ,

    input  wire                  i_clear             
    );
    wire                         mem_wr;
    wire [p_AWIDTH-1:0]          mem_wr_addr;
    wire [p_DWIDTH-1:0]          mem_wr_data;

    wire                         mem_rd;
    wire [p_AWIDTH-1:0]          mem_rd_addr;
    wire [p_DWIDTH-1:0]          mem_rd_data;

    reg  [p_AWIDTH  :0]          mem_wr_ptr;
    wire [p_AWIDTH  :0]          mem_wr_ptr_in;

    reg  [p_AWIDTH  :0]          mem_rd_ptr;
    wire [p_AWIDTH  :0]          mem_rd_ptr_in;

    reg  [p_AWIDTH  :0]          mem_rd_cnt;
    wire [p_AWIDTH  :0]          mem_rd_cnt_in;

    reg  [p_AWIDTH  :0]          mem_wr_cnt;
    wire [p_AWIDTH  :0]          mem_wr_cnt_in;

    reg                          fifo_wr_full;
    reg                          fifo_wr_full_n;
    reg                          fifo_wr_al_full;
    reg                          fifo_wr_al_full_n;

    wire                         fifo_wr_full_in;
    wire                         fifo_wr_al_full_in;

    reg                          fifo_rd_empty;
    reg                          fifo_rd_empty_n;
    reg                          fifo_rd_al_empty;
    reg                          fifo_rd_al_empty_n;

    wire                         fifo_rd_empty_in;
    wire                         fifo_rd_al_empty_in;

    osr_axi_dp_mem #(
        .p_AWIDTH      (p_AWIDTH            ),       
        .p_DWIDTH      (p_DWIDTH            )
    ) u_axi_dp_mem(
        .i_wr_clk      (i_clk               ),
        .i_wr_rst_n    (i_rst_n             ),    
        .i_wr          (mem_wr              ), 
        .i_wr_addr     (mem_wr_addr         ), 
        .i_wr_data     (mem_wr_data         ), 

        .i_rd_clk      (i_clk               ),
        .i_rd_rst_n    (i_rst_n             ),   
        .i_rd          (mem_rd              ), 
        .i_rd_addr     (mem_rd_addr         ), 
        .o_rd_data     (mem_rd_data         ) 
    );

    always@(posedge i_clk or negedge i_rst_n) 
    begin
        if(~i_rst_n) begin
            mem_wr_ptr          <= {(p_AWIDTH+1){1'b0}};
            mem_rd_ptr          <= {(p_AWIDTH+1){1'b0}};
            mem_rd_cnt          <= {(p_AWIDTH+1){1'b0}};
            mem_wr_cnt          <= {1'b1,{(p_AWIDTH){1'b0}}};
            fifo_wr_full        <= 1'b0;
            fifo_rd_empty       <= 1'b0;
            fifo_wr_al_full     <= 1'b0;
            fifo_rd_al_empty    <= 1'b0;
            fifo_wr_full_n      <= 1'b0;
            fifo_rd_empty_n     <= 1'b0;
            fifo_wr_al_full_n   <= 1'b0;
            fifo_rd_al_empty_n  <= 1'b0;
        end
        else begin
            mem_wr_ptr          <=  mem_wr_ptr_in;
            mem_rd_ptr          <=  mem_rd_ptr_in;
            mem_rd_cnt          <=  mem_rd_cnt_in;
            mem_wr_cnt          <=  mem_wr_cnt_in;
            fifo_wr_full        <=  fifo_wr_full_in;
            fifo_wr_al_full     <=  fifo_wr_al_full_in;

            fifo_rd_empty       <=  fifo_rd_empty_in;
            fifo_rd_al_empty    <=  fifo_rd_al_empty_in;

            fifo_wr_full_n      <=  !fifo_wr_full_in;
            fifo_wr_al_full_n   <=  !fifo_wr_al_full_in;

            fifo_rd_empty_n     <=  !fifo_rd_empty_in;
            fifo_rd_al_empty_n  <=  !fifo_rd_al_empty_in;

        end
    end

    assign mem_rd_ptr_in      = (i_clear)       ? {(p_AWIDTH+1){1'b0}} :
                                (mem_rd)        ? mem_rd_ptr+1'b1      : mem_rd_ptr ;

    assign mem_rd_cnt_in      = (i_clear)       ? {(p_AWIDTH+1){1'b0}} :
                                (mem_rd&mem_wr) ? mem_rd_cnt           :
                                (mem_wr)        ? mem_rd_cnt+1'b1      :
                                (mem_rd)        ? mem_rd_cnt-1'b1      : mem_rd_cnt ;

    assign fifo_rd_empty_in   = (mem_wr_ptr_in==mem_rd_ptr_in);

    assign fifo_rd_al_empty_in= mem_rd_cnt_in<= {{(p_AWIDTH){1'b0}},1'b1} ;

    assign mem_wr_ptr_in      = (i_clear)       ? {(p_AWIDTH+1){1'b0}} :
                                (mem_wr)        ? mem_wr_ptr+1'b1      : mem_wr_ptr ;

    assign mem_wr_cnt_in      = (i_clear)       ? {1'b1,{(p_AWIDTH){1'b0}}} :
                                (mem_rd&mem_wr) ? mem_wr_cnt                :
                                (mem_rd)        ? mem_wr_cnt+1'b1           :
                                (mem_wr)        ? mem_wr_cnt-1'b1           : mem_wr_cnt ;                             

    assign fifo_wr_full_in    = (mem_wr_ptr_in[p_AWIDTH]!=mem_rd_ptr_in[p_AWIDTH])&(mem_wr_ptr_in[p_AWIDTH-1:0]==mem_rd_ptr_in[p_AWIDTH-1:0]);

    assign fifo_wr_al_full_in = mem_wr_cnt_in<= {{(p_AWIDTH){1'b0}},1'b1} ;

    assign mem_wr             = i_wr&(fifo_wr_full_n)     ;
    assign mem_wr_data        = i_wr_data                ;
    assign mem_wr_addr        = mem_wr_ptr[p_AWIDTH-1:0] ;

    assign mem_rd             = i_rd&(fifo_rd_empty_n)    ;
    assign mem_rd_addr        = mem_rd_ptr[p_AWIDTH-1:0] ;

    assign o_wr_full       = fifo_wr_full             ;
    assign o_wr_full_n     = fifo_wr_full_n           ;
    assign o_wr_al_full    = fifo_wr_al_full          ;
    assign o_wr_al_full_n  = fifo_wr_al_full_n        ;

    assign o_wr_num_count  = mem_wr_cnt               ;

    assign o_rd_data       = mem_rd_data              ;
    assign o_rd_empty      = fifo_rd_empty            ;
    assign o_rd_empty_n    = fifo_rd_empty_n          ;
    assign o_rd_al_empty   = fifo_rd_al_empty         ;
    assign o_rd_al_empty_n = fifo_rd_al_empty_n       ;

    assign o_rd_num_count  = mem_rd_cnt               ;

endmodule
