/*************************************************************************
// Wingsemi Technology  Processor
// Copyright (c) 2023-2024 Wingsemi Technology Co., Ltd. 
// All rights reserved.
//
// Proprietary and Confidential Information:
// This file, with all its contents, is the sole property of Wingsemi 
// Technology and must not be copied, reproduced, modified, disclosed to 
// others, published or used, in whole or in part, without the authorized 
// consent of Wingsemi Technology.
************************************************************************/


module wing_cbb_bit_sync #(
    parameter STAGE_N  =  3,
    parameter DATA_W   =  1,
    parameter RST_VAL  =  0
)(
    input  logic                       clk   ,
    input  logic                       rst_n ,
    input  logic [DATA_W-1:0]          data_i,
    output logic [DATA_W-1:0]          data_o
);

//logic [STAGE_N:0][DATA_W-1:0] ppln_data;
//if (STAGE_N == 0 ) begin : BYPASS
//  assign data_o = data_i;
//end else begin : PPLN
//  assign ppln_data[0] = data_i; 
//  for(genvar i=0; i<STAGE_N; i++) begin : g_stage
//  always_ff @(posedge clk or negedge rst_n) begin 
//    if (!rst_n) begin                               
//        ppln_data[i+1] <= RST_VAL;                              
//    end else begin                                    
//        ppln_data[i+1] <= ppln_data[i];                                   
//    end                                               
//  end
//  end
//  assign data_o = ppln_data[STAGE_N];
//end

if (STAGE_N > 2) begin : g_sync_3
    for(genvar i = 0; i < DATA_W; i = i + 1) begin : g_sysnc_cell_3
        osr_sync_level_3 u_sync_level_3(.i_clk (clk), .i_rst_n (rst_n), .i_async (data_i[i]), .o_sync (data_o[i]));
    end
end else begin : g_sync_2
    for(genvar i = 0; i < DATA_W; i = i + 1) begin : g_sysnc_cell_2
        osr_sync_level_2 u_sync_level_2(.i_clk (clk), .i_rst_n (rst_n), .i_async (data_i[i]), .o_sync (data_o[i]));
    end
end

endmodule
