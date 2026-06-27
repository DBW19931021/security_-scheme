//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dp_mem #(
    parameter [31:0] p_AWIDTH = 10 ,       

    parameter [31:0] p_DWIDTH = 32
    )(

    input  wire                  i_wr_clk          ,
    input  wire                  i_wr_rst_n        ,    

    input  wire                  i_wr              , 
    input  wire [p_AWIDTH-1:0]   i_wr_addr         , 
    input  wire [p_DWIDTH-1:0]   i_wr_data         , 

    input  wire                  i_rd_clk          ,
    input  wire                  i_rd_rst_n        ,   

    input  wire                  i_rd              , 
    input  wire [p_AWIDTH-1:0]   i_rd_addr         , 
    output wire [p_DWIDTH-1:0]   o_rd_data          
    );

    localparam [31:0] MEM_NUM         = 2**(p_AWIDTH) +32'b0     ;

    integer                      i;
    integer                      j;
    genvar                       k;
    genvar                       l;

    reg  [p_DWIDTH-1:0]          mem        [MEM_NUM-1:0];
    wire [p_DWIDTH-1:0]          mem_in     [MEM_NUM-1:0];
    wire [p_DWIDTH-1:0]          mem_out    [MEM_NUM-1:0];

    wire                         mem_wr              ;
    wire [p_AWIDTH-1:0]          mem_wr_addr         ;
    wire [p_DWIDTH-1:0]          mem_wr_data         ;

    wire                         mem_rd              ;
    wire [p_AWIDTH-1:0]          mem_rd_addr         ;
    reg  [p_DWIDTH-1:0]          mem_rd_data         ;
    wire [p_DWIDTH-1:0]          mem_rd_data_in      ;

    always@(posedge i_wr_clk or negedge i_wr_rst_n)
    begin
        if(~i_wr_rst_n) begin
            for(i=0;i<MEM_NUM;i=i+1) begin
                    mem[i] <= {(p_DWIDTH){1'b0}};
            end
        end
        else begin
            for(i=0;i<MEM_NUM;i=i+1) begin
                    mem[i] <=  mem_in[i];
            end
        end
    end

    generate
        for(k=0;k<MEM_NUM;k=k+1) begin:dp_mem_in_array
           assign mem_in[k]= (k==mem_wr_addr)&mem_wr ? mem_wr_data : mem[k];
        end
    endgenerate

    generate
        for(k=0;k<MEM_NUM;k=k+1) begin:dp_mem_out_logic
            assign mem_out[k] =  mem[k];
        end
    endgenerate

    always@(posedge i_rd_clk or negedge i_rd_rst_n)
    begin
        if(~i_rd_rst_n) begin
            mem_rd_data<= {(p_DWIDTH){1'b0}};
        end
        else begin
            mem_rd_data<=  mem_rd_data_in ;
        end
    end

    assign mem_rd_data_in= (mem_rd) ? mem_out[mem_rd_addr] : mem_rd_data ;

    assign mem_wr        = i_wr     ; 
    assign mem_wr_addr   = i_wr_addr; 
    assign mem_wr_data   = i_wr_data; 

    assign mem_rd        = i_rd     ; 
    assign mem_rd_addr   = i_rd_addr; 
    assign o_rd_data     = mem_rd_data  ; 

endmodule
