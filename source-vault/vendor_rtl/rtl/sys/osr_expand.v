//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_expand #(
    parameter   P_LOW_PULSE = 0 
    )(
    input   wire                i_clk           ,
    input   wire                i_rst_n         ,

    input   wire                i_1pulse        ,
    output  wire                o_2pulse         
);

reg r_pulse ;

always @(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        r_pulse <= P_LOW_PULSE ? 1'h1 : 1'h0 ;
    end else begin
        r_pulse <= i_1pulse ;
    end
end 

assign o_2pulse = P_LOW_PULSE ? (i_1pulse & r_pulse) : (i_1pulse | r_pulse);

endmodule 
