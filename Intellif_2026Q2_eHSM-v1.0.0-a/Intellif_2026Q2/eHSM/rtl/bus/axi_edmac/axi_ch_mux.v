//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_axi_ch_mux # (
    parameter [31:0] p_DWIDTH = 8,
    parameter [31:0] p_NUM    = 8
)(
input   wire [p_NUM-1:0]          i_sel,
input   wire [p_DWIDTH*p_NUM-1:0] i_data,
output  wire [p_DWIDTH-1:0]       o_data
);

integer i;
genvar j;

wire [p_DWIDTH*p_NUM-1:0] data_tmp;

assign data_tmp[p_DWIDTH-1:0] = i_data[p_DWIDTH-1:0]&{(p_DWIDTH){i_sel[0]}};

generate
    for(j=1;j<p_NUM;j=j+1) begin:mux_logic
        assign data_tmp[p_DWIDTH*(j+1)-1:p_DWIDTH*j] = data_tmp[p_DWIDTH*j-1:p_DWIDTH*(j-1)]|
                                                       {i_data[p_DWIDTH*(j+1)-1:p_DWIDTH*j]&{(p_DWIDTH){i_sel[j]}}};
    end
endgenerate

assign o_data=data_tmp[p_DWIDTH*p_NUM-1:p_DWIDTH*(p_NUM-1)];

endmodule
