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

module wing_cbb_icg_wrap (
    input  logic       clk        ,
    input  logic       clk_en     ,
    input  logic       scan_en    ,
    output logic       gated_clk 
);


// For simulation only.
// It should NOT be used for synthesis.

//logic latch_en;
//
//always_latch begin
//    if (~clk) begin
//        latch_en <= scan_en | clk_en;
//    end
//end
//
//assign gated_clk = clk & latch_en;

osr_icg u_wing_cbb_icg(.CK(clk), .E(clk_en), .SE(scan_en), .GCK(gated_clk));

endmodule
