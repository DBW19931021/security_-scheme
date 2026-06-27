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

module wing_cbb_dft_mux2 (
  input  logic din0    ,
  input  logic din1    ,
  input  logic sel     ,
  output logic dout    
);
//always_comb begin
//    case (sel)
//        1'b0 : dout = din0;
//        1'b1 : dout = din1;
//        default : dout = 1'bx;
//    endcase
//end

osr_mux2 u_wing_cbb_dft_mux2 (.A(din0), .B(din1), .S(sel), .Z(dout));

endmodule
