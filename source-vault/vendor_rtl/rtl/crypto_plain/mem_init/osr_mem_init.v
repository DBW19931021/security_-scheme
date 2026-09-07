//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_mem_init
    #(
     parameter    P_MEM_DATA_WIDTH       = 32
    ,parameter    P_MEM_ADDR_WIDTH       = 14
    ,parameter    P_MEM_INIT_DEEP        = 64*1024/4
    )
    (
     input    wire                            clk
    ,input    wire                            rst_n
    ,output   wire                            o_ram_wren
    ,output   wire    [P_MEM_ADDR_WIDTH-1:0]  o_ram_waddr
    ,output   wire    [P_MEM_DATA_WIDTH-1:0]  o_ram_wdata

    ,output   wire                            o_init_done
    );

    localparam                 ADDR_MAX    = P_MEM_INIT_DEEP;

    reg   [P_MEM_ADDR_WIDTH:0]        r_addr_cnt   ;
    wire                              w_init_done  ;
    reg                               r_valid      ;
    wire                              w_ram_wren   ;

    reg                               r_init_done  ;

    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            r_addr_cnt <= {(P_MEM_ADDR_WIDTH+1){1'b0}};
        end else if(w_ram_wren) begin
            r_addr_cnt <= r_addr_cnt + 1'b1;
        end
    end

    always@(posedge clk or negedge rst_n)
    begin
        if(!rst_n) begin
            r_valid <= 1'b0;
            r_init_done <= 1'h0 ;             
        end else begin
            r_valid <= 1'b1;
            r_init_done <= w_init_done ;             
        end
    end

    assign w_init_done = r_addr_cnt[P_MEM_ADDR_WIDTH:0] == ADDR_MAX;
    assign w_ram_wren  = !w_init_done & r_valid;

    assign o_init_done = r_init_done;
    assign o_ram_wdata = {P_MEM_DATA_WIDTH{1'b0}};
    assign o_ram_wren  = w_ram_wren;
    assign o_ram_waddr = r_addr_cnt[P_MEM_ADDR_WIDTH-1:0];

endmodule
