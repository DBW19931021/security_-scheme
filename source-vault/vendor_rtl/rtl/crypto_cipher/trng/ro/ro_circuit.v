//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_circuit
#(
    parameter INV_N =17          ,
    parameter p_IDX =0           ,
    parameter delay =860
)
(
    input   wire        en      ,
    input   wire        clk     ,
    input   wire        rstn    ,
    input   wire        scan    ,
    output  wire        o_rout    
);

    genvar      inv_idx;
    (*dont_touch = "true"*)    wire [INV_N-1:0] inv_n    /* synthesis syn_keep=1 */;
    (*dont_touch = "true"*)    wire    inv_00   /* synthesis syn_keep=1 */;
    (*dont_touch = "true"*)    wire    inv_01   /* synthesis syn_keep=1 */;
    (*dont_touch = "true"*)    wire    inv_sel  /* synthesis syn_keep=1 */;
    (*dont_touch = "true"*)    wire    inv_in   /* synthesis syn_keep=1 */;
    (*dont_touch = "true"*)    wire    rout     /* synthesis syn_keep=1 */;
    (*dont_touch = "true"*)    reg     dft_reg  /* synthesis syn_keep=1 */;   

        osr_nand2   OSR_DONTTOUCH_n01(.Z(inv_01),.A(en),.B(inv_in));

    `ifdef OSR_TRNG_RAND_SIM
        reg ro_data ;

        initial begin
            forever begin
                if(en) 
                   ro_data = #7 ($urandom / (INV_N*delay)); 
                else
                   ro_data = #2 1'b0 ;
            end
        end
        assign rout = ro_data ;

    `else 
        `ifdef OSR_TRNG_SIM
            assign      #(delay*1ps) inv_n[0] = inv_01;
        `else
            assign      #(delay/1000.0) inv_n[0] = inv_01;
        `endif
      generate
        for (inv_idx = 0; inv_idx < (INV_N-1); inv_idx = inv_idx+1)
        begin: INV_CELL
            osr_ro_inv OSR_DONTTOUCH_n(.Z(inv_n[inv_idx+1]),.A(inv_n[inv_idx]));
        end
      endgenerate

        assign  inv_00  = inv_n[INV_N-1]  ;
        assign  inv_sel = inv_n[INV_N>>1] ;

        osr_inv   OSR_DONTTOUCH_nout(.Z(rout),.A(inv_sel)); 

        osr_mux2  OSR_DONTTOUCH_mux(.A(inv_00),.B(dft_reg),.S(scan),.Z(inv_in));
    `endif

        always @(posedge clk or negedge rstn)
            if(!rstn)     dft_reg <= 1'b0   ;
            else if(scan) dft_reg <= inv_00 ;
            else          dft_reg <= 1'b0   ;

        osr_sync #(.p_DWIDTH(1))
        sample_sync(
            .i_clk  (clk    ), 
            .i_rst_n(rstn   ), 
            .i_in   (rout   ), 
            .o_out  (o_rout ) 
        );

endmodule
