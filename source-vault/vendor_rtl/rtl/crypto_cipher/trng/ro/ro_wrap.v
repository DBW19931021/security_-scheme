//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module ro_wrap
#(
    parameter p_IDX=0    ,
    parameter INV_N=17   ,
    parameter delay=860  ,
    parameter p_CONFIG_FPGA=0
)
(
    input   wire        en      ,
    input   wire        clk     ,
    input   wire        i_en_clk  ,
    input   wire        i_dft_clk_en,
    input   wire        rstn    ,
    input   wire        scan    ,
    output  wire        o_rout    
);

function [31:0] rand32(input reg [31:0] x);
    reg [31:0] calc;
    integer i;
    begin
        calc = x;
        rand32 = {calc[30],{31{1'b0}}};
        for (i = 0; i < 1; i = i+1) begin 
            calc[15:0] = {calc[0:0],calc[15:1]} ^ (~{calc[20:16], calc[31:21]} & {calc[26:16], calc[31:27]}) ^ {calc[28:16], calc[31:29]};
            calc = calc ^ p_IDX;
            calc = {calc[15:0], calc[31:16]};
            rand32[31] = rand32[31] ^ calc[0] ^ calc[31]  ^ calc[17];
        end
            rand32 = {rand32[31], calc[30:0]};
    end
endfunction
generate
    if (!p_CONFIG_FPGA) begin : oscillator
        ro_circuit  #(.p_IDX(p_IDX),.INV_N(INV_N),.delay(delay)) ro (.en(en),.clk(clk),.rstn(rstn),.scan(scan),.o_rout(o_rout));
    end else begin : fpga_oscillator
        reg [31:0] rng;
        reg [15:0] cnt;
        always @(posedge clk or negedge rstn) begin : rand_register
            if(!rstn) begin
              rng <= p_IDX<<16 | p_IDX;
              cnt <= p_IDX   ;
            end else if(i_en_clk & i_dft_clk_en) begin
              rng <= rand32(rng) ^ {2{cnt}};
              if (cnt == (1<<16)-1-p_IDX) 
                cnt <= {16{1'b0}};
              else
                cnt <= cnt + 1;
            end
        end
        assign o_rout =  rng[31];
    end
endgenerate
endmodule
