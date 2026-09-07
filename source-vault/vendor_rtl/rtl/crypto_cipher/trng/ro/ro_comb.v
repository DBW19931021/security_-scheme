//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_comb(
    input   wire            clk     ,
    input   wire            resetn  ,
    input   wire            i_rngen ,
    input   wire            i_alarm ,
    input   wire    [ 3: 0] i_rosen ,

    input   wire            i_sflag1,
    output  wire            o_sflag1,
    input   wire    [31: 0] i_rand1 ,

    input   wire            i_sflag2,
    output  wire            o_sflag2,
    input   wire    [31: 0] i_rand2 ,

    input   wire            i_sflag3,
    output  wire            o_sflag3,
    input   wire    [31: 0] i_rand3 ,

    input   wire            i_sflag4,
    output  wire            o_sflag4,
    input   wire    [31: 0] i_rand4 ,

    input   wire            i_full  ,
    output  wire            o_we    ,
    output  wire    [31: 0] o_wdata  
);

reg             r_we;
reg             r_sflag;
reg             r_stat1,r_stat2,r_stat3,r_stat4;
reg     [31: 0] r_rand1,r_rand2,r_rand3,r_rand4;
wire            w_we;
wire            w_stat1,w_stat2,w_stat3,w_stat4;

assign  w_stat1 = r_stat1 | !i_rosen[3];
assign  w_stat2 = r_stat2 | !i_rosen[2];
assign  w_stat3 = r_stat3 | !i_rosen[1];
assign  w_stat4 = r_stat4 | !i_rosen[0];
assign  w_we = w_stat1&w_stat2&w_stat3&w_stat4&!i_full&!i_alarm&i_rngen&(|i_rosen);
assign  o_sflag1 = r_sflag;
assign  o_sflag2 = r_sflag;
assign  o_sflag3 = r_sflag;
assign  o_sflag4 = r_sflag;
assign  o_we = r_we;
assign  o_wdata = r_rand1^r_rand2^r_rand3^r_rand4;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_sflag <= 'b0;
    else if(!i_rngen)   r_sflag <= 'b0;
    else if(w_we)       r_sflag <= ~r_sflag;
    else                r_sflag <= r_sflag;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_we <= 'b0;
    else if(w_we)       r_we <= 'b1;
    else                r_we <= 'b0;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_stat1 <= 'b0;
    else if(!i_rngen)   r_stat1 <= 'b0;
    else if(i_sflag1)   r_stat1 <= 'b1;
    else if(w_we)       r_stat1 <= 'b0;
    else                r_stat1 <= r_stat1;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_rand1 <= 'h0;
    else if(i_sflag1)   r_rand1 <= i_rand1;
    else                r_rand1 <= r_rand1;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_stat2 <= 'b0;
    else if(!i_rngen)   r_stat2 <= 'b0;
    else if(i_sflag2)   r_stat2 <= 'b1;
    else if(w_we)       r_stat2 <= 'b0;
    else                r_stat2 <= r_stat2;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_rand2 <= 'h0;
    else if(i_sflag2)   r_rand2 <= i_rand2;
    else                r_rand2 <= r_rand2;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_stat3 <= 'b0;
    else if(!i_rngen)   r_stat3 <= 'b0;
    else if(i_sflag3)   r_stat3 <= 'b1;
    else if(w_we)       r_stat3 <= 'b0;
    else                r_stat3 <= r_stat3;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_rand3 <= 'h0;
    else if(i_sflag3)   r_rand3 <= i_rand3;
    else                r_rand3 <= r_rand3;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_stat4 <= 'b0;
    else if(!i_rngen)    r_stat4 <= 'b0;
    else if(i_sflag4)   r_stat4 <= 'b1;
    else if(w_we)       r_stat4 <= 'b0;
    else                r_stat4 <= r_stat4;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_rand4 <= 'h0;
    else if(i_sflag4)   r_rand4 <= i_rand4;
    else                r_rand4 <= r_rand4;

endmodule
