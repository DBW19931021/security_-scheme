//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_redge_to_pulse #(
    parameter p_NUM=1
)(
    input  wire clk                  ,
    input  wire rst_n                 ,
    input  wire [p_NUM-1:0] in_edge  ,
    output wire [p_NUM-1:0] out_pulse
);

    reg [p_NUM-1:0] in_d ;

    always@(posedge clk or negedge rst_n)
    begin
        if(~rst_n) begin
            in_d  <= {(p_NUM){1'b0}};
        end
        else begin
            in_d  <=  in_edge;
        end
    end

    assign out_pulse = (in_d^in_edge)&(in_edge&{(p_NUM){1'b1}}) ;

endmodule
