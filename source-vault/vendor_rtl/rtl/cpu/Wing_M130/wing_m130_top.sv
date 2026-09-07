//=========================================================================
// Wingsemi Technology [Wing-M130] Processor
// Copyright (c) 2023-2024 Wingsemi Technology Co., Ltd. 
// All rights reserved.
//
// Proprietary and Confidential Information:
// This file, with all its contents, is the sole property of Wingsemi 
// Technology and must not be copied, reproduced, modified, disclosed to 
// others, published or used, in whole or in part, without the authorized 
// consent of Wingsemi Technology.
//=========================================================================

//-------------------------------------------------------------------------
//   Release info
//     Date    : 20250225
//     Release : [Wing-M130]-[V1.0]-[40bef22aa166df6a]
//-------------------------------------------------------------------------
//`include "wing_m130_define.sv" 
`ifndef M130_CORE_SUPPORT_ICACHE
   //`define M130_CORE_SUPPORT_ICACHE
`endif 
`ifndef M130_CORE_SUPPORT_ECC 
   //`define M130_CORE_SUPPORT_ECC
`endif 
`ifndef M130_CORE_ICACHE_SIZE 
   `define M130_CORE_ICACHE_SIZE 16*1024 
`endif 
`ifdef M130_CORE_SUPPORT_ECC 
`define IC_TAG_SRAM_RAS_W 7 
`else 
`define IC_TAG_SRAM_RAS_W 0 
`endif 
`ifdef M130_CORE_SUPPORT_ECC 
`define IC_DATA_SRAM_RAS_W 8 
`else 
`define IC_DATA_SRAM_RAS_W 0 
`endif 
`ifdef M130_CORE_SUPPORT_ECC 
`define ITCM_RAS_W 7 
`else 
`define ITCM_RAS_W 0 
`endif 
`ifdef M130_CORE_SUPPORT_ECC 
`define DTCM_RAS_W 7 
`else 
`define DTCM_RAS_W 0 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`define MSS_EN_MMR 
`endif 
`ifndef MSS_EN_MMR 
`endif

//`include "wing_m130_lib_define.sv"  
 
`ifndef __WING_CBB_DEFINE_MISC__
`define __WING_CBB_DEFINE_MISC__
`define WCBB_AOMUX(__data_arr, __sel_in_oh0, __data_sel) \
always_comb begin                                        \
  __data_sel = '0;                                       \
  for (int ii=0; ii < $bits(__sel_in_oh0); ii++) begin : g_``__data_sel \
    __data_sel = (__data_arr[ii] &                       \
                  {$bits(__data_sel){__sel_in_oh0[ii]}}) \
               | __data_sel;                             \
  end                                                    \
end                                                     
`define WCBB_BIN2ONEHOT(__bin_i, __onehot_o)          \
for (genvar ii=0; ii<$bits(__onehot_o); ii++) begin : g_``__onehot_o \
  assign __onehot_o[ii] = (__bin_i==ii);            \
end                                               
`define WCBB_ONEHOT2BIN(__onehot_i, __bin_o)          \
always_comb begin                                     \
  localparam LOC_VEC_W = $bits(__onehot_i);           \
  localparam LOC_BIN_W = $bits(__bin_o);              \
  __bin_o = '0;                                       \
  for (int ii=0; ii < LOC_VEC_W; ii++) begin : g_``__bin_o      \
    __bin_o = ({LOC_BIN_W{__onehot_i[ii]}} & ii[LOC_BIN_W-1:0]) \
            | __bin_o;                                \
  end                                                 \
end
`define WCBB_PRIORITY_QUEUE_LSB(__vec_i, __onehot_o) \
always_comb begin                                  \
  logic [$bits(__vec_i)-1:0] __vec_adj_or;         \
                                                   \
  __vec_adj_or[0] = 1'b0;                          \
  for (int ii=1; ii<$bits(__vec_i); ii++) begin : g_``__onehot_o \
    __vec_adj_or[ii] = __vec_i     [ii-1] |        \
                      __vec_adj_or[ii-1] ;         \
  end                                              \
                                                   \
  __onehot_o = __vec_i & ~__vec_adj_or;            \
end
`define WCBB_PRIORITY_QUEUE_MSB(__vec_i, __onehot_o) \
always_comb begin                                  \
  logic [$bits(__vec_i)-1:0] __vec_adj_or;         \
                                                   \
  __vec_adj_or[$bits(__vec_i)-1] = 1'b0;           \
  for (int ii=$bits(__vec_i)-2; ii>=0; ii--) begin : g_``__onehot_o \
    __vec_adj_or[ii] = __vec_i     [ii+1] |        \
                      __vec_adj_or[ii+1] ;         \
  end                                              \
                                                   \
  __onehot_o = __vec_i & ~__vec_adj_or;            \
end
`endif 
`ifndef __WING_CBB_DEFINE_DFF__
`define __WING_CBB_DEFINE_DFF__
`define WDFFR(__q, __d, __clk, __rst_n)               \
  always_ff @(posedge __clk or negedge __rst_n) begin \
    if (!__rst_n) begin                               \
        __q <= '0;                                    \
    end else begin                                    \
        __q <= __d;                                   \
    end                                               \
  end
`define WDFFER(__q, __d, __en, __clk, __rst_n)        \
  always_ff @(posedge __clk or negedge __rst_n) begin \
    if (!__rst_n) begin                               \
        __q <= '0;                                    \
    end else if (__en) begin                          \
        __q <= __d;                                   \
    end                                               \
  end
`define WDFFENR(__q, __d, __en, __clk)  \
  always_ff @(posedge __clk) begin      \
    if (__en) __q <= __d;               \
  end
`define WDFFNR(__q, __d, __clk)    \
  always_ff @(posedge __clk) begin \
        __q <= __d;                \
  end
`define WDFFRVAL(__q, __d, __clk, __rst_n, __rstval)  \
  always_ff @(posedge __clk or negedge __rst_n) begin \
    if (!__rst_n) begin                               \
        __q <= __rstval;                              \
    end else begin                                    \
        __q <= __d;                                   \
    end                                               \
  end
`define WDFFERVAL(__q, __d, __en, __clk, __rst_n, __rstval) \
  always_ff @(posedge __clk or negedge __rst_n) begin       \
    if (!__rst_n) begin                                     \
        __q <= __rstval;                                    \
    end else if (__en) begin                                \
        __q <= __d;                                         \
    end                                                     \
  end
`define WNDFFER(__q, __d, __en, __clk, __rst_n)       \
  always_ff @(negedge __clk or negedge __rst_n) begin \
    if (!__rst_n) begin                               \
        __q <= '0;                                    \
    end else if (__en) begin                          \
        __q <= __d;                                   \
    end                                               \
  end
`define WNDFFR(__q, __d,__clk, __rst_n)       \
  always_ff @(negedge __clk or negedge __rst_n) begin \
    if (!__rst_n) begin                               \
      __q <= '0;                                      \
    end else begin                                    \
      __q <= __d;                                     \
    end                                               \
  end
`endif


module wing_cbb_rr_arb #( 
parameter WIDTH = 8 
) ( 
input logic clk, 
input logic rst_n, 
input logic [WIDTH-1:0] reqest_i, 
output logic [WIDTH-1:0] grant_o 
); 
logic [WIDTH-1:0] req_masked; 
logic [WIDTH-1:0] mask_higher_pri_reqs; 
logic [WIDTH-1:0] grant_masked ; 
logic [WIDTH-1:0] unmask_higher_pri_reqs; 
logic [WIDTH-1:0] grant_unmasked ; 
logic no_req_masked ; 
logic dff_ptr_en ; 
logic [WIDTH-1:0] dff_ptr_d ; 
logic [WIDTH-1:0] dff_ptr_q ; 
assign req_masked = reqest_i & dff_ptr_q; 
assign mask_higher_pri_reqs[0] = 1'b0; 
for (genvar i=1; i<WIDTH; i++) begin : g_mask_hi_pri 
assign mask_higher_pri_reqs[i] = mask_higher_pri_reqs[i-1] | req_masked[i-1]; 
end 
assign grant_masked[WIDTH-1:0] = req_masked[WIDTH-1:0] & ~mask_higher_pri_reqs[WIDTH-1:0]; 
assign unmask_higher_pri_reqs[0] = 1'b0; 
for (genvar i=1; i<WIDTH; i++) begin : g_unmask_hi_pri 
assign unmask_higher_pri_reqs[i] = unmask_higher_pri_reqs[i-1] | reqest_i[i-1]; 
end 
assign grant_unmasked[WIDTH-1:0] = reqest_i[WIDTH-1:0] & ~unmask_higher_pri_reqs[WIDTH-1:0]; 
assign dff_ptr_d = |req_masked ? mask_higher_pri_reqs : 
(|reqest_i ? unmask_higher_pri_reqs : 
dff_ptr_q); 
`WDFFRVAL(dff_ptr_q, dff_ptr_d, clk, rst_n, {WIDTH{1'b1}}) 
assign no_req_masked = ~(|req_masked); 
assign grant_o = ({WIDTH{no_req_masked}} & grant_unmasked) | grant_masked; 
endmodule
 

 
 

 

 

 

 

 
module wing_cbb_sum_compare #( 
parameter WIDTH = 4 
) ( 
input logic [WIDTH - 1 : 0] a_comp , 
input logic [WIDTH - 1 : 0] b_comp , 
input logic [WIDTH - 1 : 0] k_comp , 
output logic [WIDTH - 1 : 0] unequal_que, 
output logic equal_re 
); 
logic [WIDTH - 1 : 0] ab_xor; 
logic [WIDTH - 1 : 0] c_req; 
logic [WIDTH - 1 : 0] c_gen; 
assign ab_xor[0] = a_comp[0] ^ b_comp[0]; 
for (genvar i = 1; i < WIDTH; i++) begin 
assign ab_xor[i] = a_comp[i] ^ b_comp[i]; 
assign c_req[i-1] = ab_xor[i] ^ k_comp[i]; 
assign unequal_que[i] = (c_req[i-1] ^ c_gen[i-1]); 
end 
for (genvar i = 0; i < WIDTH; i++) begin 
assign c_gen[i] = (ab_xor[i] & (!k_comp[i])) | (a_comp[i] & b_comp[i]); 
end 
assign unequal_que[0] = a_comp[0] ^ b_comp[0] ^ k_comp[0]; 
assign equal_re = ~(| unequal_que); 
endmodule
 

 
module wing_cbb_lfsr_8bit #( 
parameter int unsigned WIDTH = 1, 
parameter logic [7:0] SEED = 8'd0 
) ( 
input logic clk , 
input logic rst_n, 
input logic en_i , 
output logic [WIDTH-1:0] out_o 
); 
logic [7:0] shift_d, shift_q; 
logic shift_in; 
assign shift_in = !(shift_q[7] ^ shift_q[3] ^ shift_q[2] ^ shift_q[1]); 
assign shift_d = {shift_q[6:0], shift_in}; 
assign out_o = shift_q[WIDTH-1:0]; 
always_ff @(posedge clk or negedge rst_n) begin : g_shift 
if (~rst_n) begin 
shift_q <= SEED; 
end else if (en_i) begin 
shift_q <= shift_d; 
end 
end 
endmodule
 
 

 
 

 

 
 

 

 
 

 
 
 
module wxblite_mux_nx1 #( 
parameter type req_t = logic, 
parameter type rsp_t = logic, 
parameter SLV_PORTS_N = 2, 
parameter ARB_MODE = 0, 
parameter LOCK = 1, 
parameter EN_FLUSH = 0, 
parameter IDX_W = $clog2(SLV_PORTS_N) 
) ( 
input logic clk , 
input logic rst_n, 
input logic flush, 
input logic [SLV_PORTS_N-1:0] slv_req_vld_i , 
output logic [SLV_PORTS_N-1:0] slv_req_rdy_o , 
input req_t [SLV_PORTS_N-1:0] slv_req_info_i , 
output logic [SLV_PORTS_N-1:0] slv_rsp_vld_o , 
input logic [SLV_PORTS_N-1:0] slv_rsp_rdy_i , 
output rsp_t [SLV_PORTS_N-1:0] slv_rsp_info_o , 
output logic mst_req_vld_o , 
input logic mst_req_rdy_i , 
output req_t mst_req_info_o , 
output logic [IDX_W-1:0] mst_req_id_o , 
input logic mst_rsp_vld_i , 
output logic mst_rsp_rdy_o , 
input rsp_t mst_rsp_info_i , 
input logic [IDX_W-1:0] mst_rsp_id_i 
); 
if (SLV_PORTS_N == 1) begin : g_direct 
assign slv_req_rdy_o = mst_req_rdy_i; 
assign slv_rsp_vld_o = mst_rsp_vld_i ; 
assign slv_rsp_info_o = mst_rsp_info_i; 
assign mst_req_vld_o = slv_req_vld_i ; 
assign mst_req_info_o = slv_req_info_i; 
assign mst_req_id_o = 1'b0; 
assign mst_rsp_rdy_o = slv_rsp_rdy_i; 
end else begin : g_mux 
logic [SLV_PORTS_N-1:0] arb_slv_req_vld_oh; 
logic [SLV_PORTS_N-1:0] sel_onehot; 
if (ARB_MODE == 0) begin : g_arb_fix 
if (LOCK == 1) begin : w_lock 
logic dff_lock_set; 
logic dff_lock_clr; 
logic dff_lock_en; 
logic dff_lock_d ; 
logic dff_lock_q ; 
logic dff_lock_req_vld_en; 
logic [SLV_PORTS_N-1:0] dff_lock_req_vld_d ; 
logic [SLV_PORTS_N-1:0] dff_lock_req_vld_q ; 
logic [SLV_PORTS_N-1:0] arb_slv_req_vld_tmp; 
assign dff_lock_set = mst_req_vld_o & ~mst_req_rdy_i & ~dff_lock_q; 
if (EN_FLUSH == 1) begin : g_flush 
assign dff_lock_clr = mst_req_rdy_i & dff_lock_q | flush; 
end else begin : g_no_flush 
assign dff_lock_clr = mst_req_rdy_i & dff_lock_q; 
end 
assign dff_lock_en = dff_lock_set | dff_lock_clr; 
assign dff_lock_d = dff_lock_set; 
`WDFFER(dff_lock_q, dff_lock_d, dff_lock_en, clk, rst_n) 
`WCBB_PRIORITY_QUEUE_LSB(slv_req_vld_i, arb_slv_req_vld_tmp) 
assign dff_lock_req_vld_en = dff_lock_set; 
assign dff_lock_req_vld_d = arb_slv_req_vld_tmp; 
`WDFFER(dff_lock_req_vld_q, dff_lock_req_vld_d, dff_lock_req_vld_en, clk, rst_n) 
if (EN_FLUSH == 1) begin : g_arb_sel_with_lock_and_flush 
assign arb_slv_req_vld_oh = dff_lock_q & (~flush) ? dff_lock_req_vld_q : arb_slv_req_vld_tmp; 
end else begin : g_arb_sel_with_lock 
assign arb_slv_req_vld_oh = dff_lock_q ? dff_lock_req_vld_q : arb_slv_req_vld_tmp; 
end 
end else begin : wo_lock 
`WCBB_PRIORITY_QUEUE_LSB(slv_req_vld_i, arb_slv_req_vld_oh) 
end 
end else begin : g_arb_rr 
wing_cbb_rr_arb #( 
.WIDTH ( SLV_PORTS_N ) 
) u_rr_arb ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.reqest_i ( slv_req_vld_i ), 
.grant_o ( arb_slv_req_vld_oh ) 
); 
end 
assign mst_req_vld_o = |slv_req_vld_i; 
`WCBB_AOMUX(slv_req_info_i, arb_slv_req_vld_oh, mst_req_info_o) 
assign slv_req_rdy_o = arb_slv_req_vld_oh & {SLV_PORTS_N{mst_req_rdy_i}}; 
`WCBB_ONEHOT2BIN(arb_slv_req_vld_oh, mst_req_id_o) 
`WCBB_BIN2ONEHOT(mst_rsp_id_i, sel_onehot) 
for (genvar i=0; i<SLV_PORTS_N; i++) begin : g_slv_rsp 
assign slv_rsp_vld_o [i] = sel_onehot[i] & mst_rsp_vld_i; 
assign slv_rsp_info_o[i] = mst_rsp_info_i; 
end 
assign mst_rsp_rdy_o = |(slv_rsp_rdy_i & sel_onehot); 
end 
endmodule
 
 
module wxblite_dummy_slave #( 
parameter EN_ID = 0, 
parameter ID_W = 1, 
parameter EN_REG = 1 
) ( 
input logic clk , 
input logic rst_n, 
input logic wxbl_req_vld , 
output logic wxbl_req_rdy , 
input logic[ID_W-1:0] wxbl_req_id , 
output logic wxbl_rsp_vld , 
input logic wxbl_rsp_rdy , 
output logic wxbl_rsp_err , 
output logic[ID_W-1:0] wxbl_rsp_id 
); 
if (EN_REG == 1) begin : g_reg 
logic req_hdsk; 
logic rsp_hdsk; 
logic dff_rsp_vld_en; 
logic dff_rsp_vld_d; 
logic dff_rsp_vld_q; 
assign req_hdsk = wxbl_req_vld & wxbl_req_rdy; 
assign rsp_hdsk = wxbl_rsp_vld & wxbl_rsp_rdy; 
assign dff_rsp_vld_en = req_hdsk | rsp_hdsk; 
assign dff_rsp_vld_d = req_hdsk; 
`WDFFER(dff_rsp_vld_q, dff_rsp_vld_d, dff_rsp_vld_en, clk, rst_n) 
assign wxbl_req_rdy = ~dff_rsp_vld_q | wxbl_rsp_rdy; 
assign wxbl_rsp_vld = dff_rsp_vld_q; 
assign wxbl_rsp_err = 1'b1; 
if (EN_ID == 1) begin : EN_ID_REG 
`WDFFER(wxbl_rsp_id, wxbl_req_id, req_hdsk, clk, rst_n) 
end else begin : NO_ID_REG 
assign wxbl_rsp_id = 'b0 ; 
end 
end else begin : g_feedthrough 
assign wxbl_rsp_vld = wxbl_req_vld; 
assign wxbl_rsp_err = 1'b1; 
assign wxbl_rsp_id = wxbl_req_id; 
assign wxbl_req_rdy = wxbl_rsp_rdy; 
end 
endmodule
 
 
 
module wxblite_demux_1xn #( 
parameter type rsp_t = logic, 
parameter MST_PORTS_N = 2, 
parameter EN_FLUSH = 0, 
parameter EN_KEEP_ORDER = 0 
) ( 
input logic clk , 
input logic rst_n, 
input logic flush, 
input logic slv_req_vld_i , 
output logic slv_req_rdy_o , 
output logic slv_rsp_vld_o , 
input logic slv_rsp_rdy_i , 
output rsp_t slv_rsp_info_o , 
input logic [MST_PORTS_N-1:0] sel_i, 
output logic [MST_PORTS_N-1:0] mst_req_vld_o , 
input logic [MST_PORTS_N-1:0] mst_req_rdy_i , 
input logic [MST_PORTS_N-1:0] mst_rsp_vld_i , 
output logic [MST_PORTS_N-1:0] mst_rsp_rdy_o , 
input rsp_t [MST_PORTS_N-1:0] mst_rsp_info_i 
); 
logic slv_req_hdsk; 
logic mst_rsp_hdsk; 
logic dff_float_vld_set; 
logic dff_float_vld_clr; 
logic dff_float_vld_en; 
logic dff_float_vld_d; 
logic dff_float_vld_q; 
logic flush_rsp; 
logic wait_diff_dest; 
if (MST_PORTS_N == 1) begin : g_direct 
assign mst_req_vld_o = slv_req_vld_i; 
assign slv_req_rdy_o = mst_req_rdy_i; 
assign slv_rsp_vld_o = mst_rsp_vld_i & ~flush_rsp; 
assign slv_rsp_info_o = mst_rsp_info_i; 
assign mst_rsp_rdy_o = slv_rsp_rdy_i; 
end else begin : g_demux 
logic [MST_PORTS_N-1:0] mst_req_rdy_masked; 
logic mst_req_vld_masked; 
assign mst_req_vld_masked = slv_req_vld_i & ~wait_diff_dest; 
for (genvar i=0; i<MST_PORTS_N; i++) begin : g_mst_req 
assign mst_req_vld_o[i] = mst_req_vld_masked & sel_i[i]; 
assign mst_req_rdy_masked[i] = mst_req_rdy_i[i] & sel_i[i]; 
end 
assign slv_req_rdy_o = (|mst_req_rdy_masked) & ~wait_diff_dest; 
assign slv_rsp_vld_o = (|mst_rsp_vld_i) & ~flush_rsp; 
`WCBB_AOMUX(mst_rsp_info_i, mst_rsp_vld_i, slv_rsp_info_o) 
assign mst_rsp_rdy_o = {MST_PORTS_N{slv_rsp_rdy_i}}; 
end 
assign slv_req_hdsk = slv_req_vld_i & slv_req_rdy_o; 
assign mst_rsp_hdsk = |(mst_rsp_vld_i & mst_rsp_rdy_o); 
assign dff_float_vld_set = slv_req_hdsk; 
assign dff_float_vld_clr = mst_rsp_hdsk; 
assign dff_float_vld_en = dff_float_vld_set | dff_float_vld_clr; 
assign dff_float_vld_d = dff_float_vld_set; 
`WDFFER(dff_float_vld_q, dff_float_vld_d, dff_float_vld_en, clk, rst_n) 
if (EN_FLUSH == 1) begin : g_flush 
logic dff_float_flush_set; 
logic dff_float_flush_clr; 
logic dff_float_flush_en; 
logic dff_float_flush_d; 
logic dff_float_flush_q; 
assign dff_float_flush_set = dff_float_vld_q & flush; 
assign dff_float_flush_clr = mst_rsp_hdsk; 
assign dff_float_flush_en = dff_float_flush_set | dff_float_flush_clr; 
assign dff_float_flush_d = dff_float_flush_set & ~dff_float_flush_clr; 
`WDFFER(dff_float_flush_q, dff_float_flush_d, dff_float_flush_en, clk, rst_n) 
assign flush_rsp = dff_float_flush_q; 
end else begin : no_flush 
assign flush_rsp = 1'b0; 
end 
if (EN_KEEP_ORDER == 1) begin : g_buf 
logic dff_dest_en; 
logic [MST_PORTS_N-1:0] dff_dest_d; 
logic [MST_PORTS_N-1:0] dff_dest_q; 
assign dff_dest_en = slv_req_hdsk; 
assign dff_dest_d = sel_i; 
`WDFFER(dff_dest_q, dff_dest_d, dff_dest_en, clk, rst_n) 
assign wait_diff_dest = (dff_dest_q == sel_i) ? 1'b0 : dff_float_vld_q; 
end else if (EN_KEEP_ORDER == 2) begin : g_force 
logic dff_waiting_set; 
logic dff_waiting_clr; 
logic dff_waiting_en ; 
logic dff_waiting_d ; 
logic dff_waiting_q ; 
assign dff_waiting_set = slv_req_hdsk; 
assign dff_waiting_clr = mst_rsp_hdsk; 
assign dff_waiting_en = dff_waiting_set | dff_waiting_clr; 
assign dff_waiting_d = dff_waiting_set & ~dff_waiting_clr; 
`WDFFER(dff_waiting_q, dff_waiting_d, dff_waiting_en, clk, rst_n) 
assign wait_diff_dest = dff_waiting_q; 
end else begin : g_no_limit 
assign wait_diff_dest = 1'b0; 
end 
endmodule
 
 
module wxblite_ahblite_bridge #( 
parameter ADDR_W = 32, 
parameter DATA_W = 32, 
parameter PPLN_WDATA = 1, 
parameter EN_WSTRB_SPLIT = 0, 
parameter WSTRB_SPLIT_MODE = 0, 
parameter EN_ID = 0, 
parameter ID_W = 1, 
parameter MASTER_W = 2, 
parameter type user_t = logic 
)( 
input logic clk , 
input logic rst_n , 
output logic ppln_wdata_en , 
input logic wxbl_req_vld , 
output logic wxbl_req_rdy , 
input logic [ADDR_W-1:0] wxbl_req_addr , 
input logic [1:0] wxbl_req_size , 
input logic wxbl_req_write , 
input logic [DATA_W -1:0] wxbl_req_wdata , 
input logic [DATA_W/8-1:0] wxbl_req_wstrb , 
input logic [1:0] wxbl_req_prot , 
input logic [1:0] wxbl_req_memattr, 
input logic [MASTER_W-1:0] wxbl_req_master , 
input logic [ID_W-1:0] wxbl_req_id , 
input user_t wxbl_req_user , 
output logic wxbl_rsp_vld , 
output logic wxbl_rsp_err , 
output logic [DATA_W-1:0] wxbl_rsp_rdata , 
output logic [ID_W-1:0] wxbl_rsp_id , 
output logic [ADDR_W-1:0] ahb_haddr , 
output logic [2:0] ahb_hburst , 
output logic ahb_hmastlock , 
output logic [3:0] ahb_hprot , 
output logic [2:0] ahb_hsize , 
output logic [MASTER_W-1:0] ahb_master , 
output logic [1:0] ahb_htrans , 
output logic ahb_hwrite , 
output logic [DATA_W-1:0] ahb_hwdata , 
output user_t ahb_user , 
input logic [DATA_W-1:0] ahb_hrdata , 
input logic ahb_hready , 
input logic ahb_hresp 
); 
function automatic logic [1:0] wxbl_wstrb_to_size (input logic[3:0] wstrb); 
logic [1:0] f_size ; 
f_size = (2'b00 & {3{wstrb == 4'b0001 }} ) 
| (2'b00 & {3{wstrb == 4'b0010 }} ) 
| (2'b00 & {3{wstrb == 4'b0100 }} ) 
| (2'b00 & {3{wstrb == 4'b1000 }} ) 
| (2'b01 & {3{wstrb == 4'b0011 }} ) 
| (2'b01 & {3{wstrb == 4'b1100 }} ) 
| (2'b10 & {3{wstrb == 4'b1111 }} ); 
return f_size ; 
endfunction 
function automatic logic [1:0] wxbl_wstrb_to_addr_offs (input logic[3:0] wstrb); 
logic [1:0] f_addr_offs ; 
f_addr_offs = (2'b00 & {2{wstrb == 4'b0001 }} ) 
| (2'b01 & {2{wstrb == 4'b0010 }} ) 
| (2'b10 & {2{wstrb == 4'b0100 }} ) 
| (2'b11 & {2{wstrb == 4'b1000 }} ) 
| (2'b00 & {2{wstrb == 4'b0011 }} ) 
| (2'b10 & {2{wstrb == 4'b1100 }} ); 
return f_addr_offs ; 
endfunction 
logic wxbl_req_hdsk; 
logic postsplit_req_hdsk; 
logic dff_req_vld_set; 
logic dff_req_vld_clr; 
logic dff_req_vld_en ; 
logic dff_req_vld_d ; 
logic dff_req_vld_q ; 
logic postsplit_req_vld ; 
logic postsplit_req_rdy ; 
logic [ADDR_W-1:0] postsplit_req_addr ; 
logic [1:0] postsplit_req_size ; 
logic postsplit_req_write ; 
logic [1:0] postsplit_req_prot ; 
logic [1:0] postsplit_req_memattr ; 
logic [MASTER_W-1:0] postsplit_req_master ; 
user_t postsplit_req_user ; 
logic postsplit_rsp_vld ; 
logic postsplit_rsp_err ; 
logic [DATA_W-1:0] postsplit_rsp_rdata ; 
assign wxbl_req_hdsk = wxbl_req_vld & wxbl_req_rdy; 
assign postsplit_req_hdsk = postsplit_req_vld & postsplit_req_rdy; 
if (EN_WSTRB_SPLIT == 1) begin : g_wstrb_split 
logic wstrb_is_unaln; 
logic dff_next_write_set; 
logic dff_next_write_clr; 
logic dff_next_write_en ; 
logic dff_next_write_d ; 
logic dff_next_write_q ; 
logic dff_req_info_en ; 
logic [3:2] dff_req_wstrb_d ; 
logic [3:2] dff_req_wstrb_q ; 
logic [ADDR_W-1:2] dff_req_addr_d ; 
logic [ADDR_W-1:2] dff_req_addr_q ; 
logic [3:0] unaln_wstrb_1st; 
logic [3:0] unaln_wstrb_2nd; 
logic [3:0] postsplit_wstrb; 
logic [ADDR_W-1:2] postsplit_addr_base; 
logic [1:0] postsplit_addr_offs; 
logic [1:0] dff_req_prot_d ; 
logic [1:0] dff_req_prot_q ; 
logic [1:0] dff_req_memattr_d; 
logic [1:0] dff_req_memattr_q; 
logic [MASTER_W-1:0] dff_req_master_d; 
logic [MASTER_W-1:0] dff_req_master_q; 
user_t dff_req_user_d ; 
user_t dff_req_user_q ; 
logic flag_wr_align ; 
logic flag_wr_unaln_1st; 
logic flag_wr_unaln_2nd; 
logic dff_1st_rsp_err_set; 
logic dff_1st_rsp_err_clr; 
logic dff_1st_rsp_err_en ; 
logic dff_1st_rsp_err_d ; 
logic dff_1st_rsp_err_q ; 
assign wstrb_is_unaln = (wxbl_req_wstrb == 4'b0110 | 
wxbl_req_wstrb == 4'b0111 | 
wxbl_req_wstrb == 4'b1110 ); 
if(WSTRB_SPLIT_MODE == 0) begin : BACKPRESS_REQ 
assign dff_next_write_set = postsplit_req_hdsk & wxbl_req_write & wstrb_is_unaln & ~dff_next_write_q; 
assign dff_next_write_clr = dff_next_write_q & postsplit_req_rdy; 
assign dff_next_write_en = dff_next_write_set | dff_next_write_clr ; 
assign dff_next_write_d = dff_next_write_set; 
`WDFFER(dff_next_write_q, dff_next_write_d, dff_next_write_en, clk, rst_n) 
assign flag_wr_unaln_1st = wxbl_req_write & wstrb_is_unaln & ~dff_next_write_q; 
assign flag_wr_unaln_2nd = dff_next_write_q ; 
assign flag_wr_align = ~(flag_wr_unaln_1st | flag_wr_unaln_2nd); 
assign unaln_wstrb_1st = {2'b0, wxbl_req_wstrb[1:0]}; 
assign unaln_wstrb_2nd = {wxbl_req_wstrb[3:2], 2'b0}; 
assign postsplit_wstrb = {4{flag_wr_align }} & wxbl_req_wstrb 
| {4{flag_wr_unaln_1st}} & unaln_wstrb_1st 
| {4{flag_wr_unaln_2nd}} & unaln_wstrb_2nd; 
assign postsplit_addr_base = wxbl_req_addr[ADDR_W-1:2] ; 
assign postsplit_addr_offs = wxbl_req_write ? wxbl_wstrb_to_addr_offs(postsplit_wstrb) : 
wxbl_req_addr[1:0]; 
assign postsplit_req_vld = wxbl_req_vld; 
assign postsplit_req_addr = {postsplit_addr_base, postsplit_addr_offs}; 
assign postsplit_req_size = wxbl_req_write ? wxbl_wstrb_to_size(postsplit_wstrb): 
wxbl_req_size; 
assign postsplit_req_write = wxbl_req_write ; 
assign postsplit_req_prot = wxbl_req_prot ; 
assign postsplit_req_memattr = wxbl_req_memattr; 
assign postsplit_req_master = wxbl_req_master ; 
assign postsplit_req_user = wxbl_req_user ; 
assign wxbl_req_rdy = wxbl_req_vld & postsplit_req_rdy & (flag_wr_align | flag_wr_unaln_2nd); 
assign ppln_wdata_en = postsplit_req_hdsk & wxbl_req_write & ~flag_wr_unaln_2nd; 
end else begin : TAKE_UNCONDITIONALLY 
assign dff_next_write_set = wxbl_req_hdsk & wxbl_req_write & wstrb_is_unaln; 
assign dff_next_write_clr = dff_next_write_q & postsplit_req_rdy; 
assign dff_next_write_en = dff_next_write_set | dff_next_write_clr ; 
assign dff_next_write_d = dff_next_write_set ; 
`WDFFER(dff_next_write_q, dff_next_write_d, dff_next_write_en, clk, rst_n) 
assign flag_wr_unaln_1st = wxbl_req_write & wstrb_is_unaln & ~dff_next_write_q; 
assign flag_wr_unaln_2nd = dff_next_write_q ; 
assign flag_wr_align = ~(flag_wr_unaln_1st | flag_wr_unaln_2nd); 
assign dff_req_info_en = dff_next_write_set; 
assign dff_req_wstrb_d = wxbl_req_wstrb[3:2]; 
`WDFFER(dff_req_wstrb_q, dff_req_wstrb_d, dff_req_info_en, clk, rst_n) 
assign unaln_wstrb_1st = {2'b0, wxbl_req_wstrb[1:0]}; 
assign unaln_wstrb_2nd = {dff_req_wstrb_q[3:2], 2'b0}; 
assign postsplit_wstrb = {4{flag_wr_align }} & wxbl_req_wstrb 
| {4{flag_wr_unaln_1st}} & unaln_wstrb_1st 
| {4{flag_wr_unaln_2nd}} & unaln_wstrb_2nd; 
assign dff_req_addr_d = wxbl_req_addr[ADDR_W-1:2]; 
`WDFFER(dff_req_addr_q, dff_req_addr_d, dff_req_info_en, clk, rst_n) 
assign postsplit_addr_base = flag_wr_unaln_2nd ? dff_req_addr_q[ADDR_W-1:2] : 
wxbl_req_addr [ADDR_W-1:2] ; 
assign postsplit_addr_offs = postsplit_req_write ? wxbl_wstrb_to_addr_offs(postsplit_wstrb) : 
wxbl_req_addr[1:0]; 
assign dff_req_prot_d = wxbl_req_prot; 
assign dff_req_memattr_d = wxbl_req_memattr; 
assign dff_req_master_d = wxbl_req_master; 
assign dff_req_user_d = wxbl_req_user ; 
`WDFFER(dff_req_prot_q , dff_req_prot_d , dff_req_info_en, clk, rst_n) 
`WDFFER(dff_req_memattr_q, dff_req_memattr_d, dff_req_info_en, clk, rst_n) 
`WDFFER(dff_req_master_q , dff_req_master_d , dff_req_info_en, clk, rst_n) 
`WDFFER(dff_req_user_q , dff_req_user_d , dff_req_info_en, clk, rst_n) 
assign postsplit_req_vld = wxbl_req_vld | flag_wr_unaln_2nd; 
assign postsplit_req_addr = {postsplit_addr_base, postsplit_addr_offs}; 
assign postsplit_req_size = postsplit_req_write ? wxbl_wstrb_to_size(postsplit_wstrb) : 
wxbl_req_size; 
assign postsplit_req_write = wxbl_req_write | flag_wr_unaln_2nd; 
assign postsplit_req_prot = flag_wr_unaln_2nd ? dff_req_prot_q : 
wxbl_req_prot ; 
assign postsplit_req_memattr = flag_wr_unaln_2nd ? dff_req_memattr_q : 
wxbl_req_memattr ; 
assign postsplit_req_master = flag_wr_unaln_2nd ? dff_req_master_q : 
wxbl_req_master ; 
assign postsplit_req_user = flag_wr_unaln_2nd ? dff_req_user_q : 
wxbl_req_user ; 
assign wxbl_req_rdy = ~flag_wr_unaln_2nd & postsplit_req_rdy; 
assign ppln_wdata_en = 1'b0; 
end 
assign wxbl_rsp_vld = ~flag_wr_unaln_2nd & postsplit_rsp_vld; 
assign wxbl_rsp_err = postsplit_rsp_err | dff_1st_rsp_err_q; 
assign wxbl_rsp_rdata = postsplit_rsp_rdata; 
assign dff_1st_rsp_err_set = flag_wr_unaln_2nd & postsplit_rsp_vld & postsplit_rsp_err; 
assign dff_1st_rsp_err_clr = dff_1st_rsp_err_q & wxbl_rsp_vld; 
assign dff_1st_rsp_err_en = dff_1st_rsp_err_set | dff_1st_rsp_err_clr; 
assign dff_1st_rsp_err_d = dff_1st_rsp_err_set; 
`WDFFER(dff_1st_rsp_err_q, dff_1st_rsp_err_d, dff_1st_rsp_err_en, clk, rst_n ) 
end else begin : g_wstrb_ignore 
assign postsplit_req_vld = wxbl_req_vld ; 
assign postsplit_req_addr = wxbl_req_addr ; 
assign postsplit_req_size = wxbl_req_size ; 
assign postsplit_req_write = wxbl_req_write ; 
assign postsplit_req_prot = wxbl_req_prot ; 
assign postsplit_req_memattr = wxbl_req_memattr; 
assign postsplit_req_master = wxbl_req_master ; 
assign postsplit_req_user = wxbl_req_user ; 
assign wxbl_req_rdy = postsplit_req_rdy; 
assign wxbl_rsp_vld = postsplit_rsp_vld ; 
assign wxbl_rsp_err = postsplit_rsp_err ; 
assign wxbl_rsp_rdata = postsplit_rsp_rdata; 
assign ppln_wdata_en = 1'b0; 
end 
assign postsplit_req_rdy = ahb_hready; 
assign dff_req_vld_set = postsplit_req_hdsk; 
assign dff_req_vld_clr = wxbl_rsp_vld; 
assign dff_req_vld_en = dff_req_vld_set | dff_req_vld_clr; 
assign dff_req_vld_d = dff_req_vld_set; 
`WDFFER(dff_req_vld_q, dff_req_vld_d, dff_req_vld_en, clk, rst_n) 
assign postsplit_rsp_vld = dff_req_vld_q & ahb_hready; 
assign postsplit_rsp_err = ahb_hresp ; 
assign postsplit_rsp_rdata = ahb_hrdata; 
if( EN_ID == 1 ) begin : g_id 
logic dff_req_id_en; 
logic [ID_W-1:0] dff_req_id_d; 
logic [ID_W-1:0] dff_req_id_q; 
assign dff_req_id_en = wxbl_req_hdsk; 
assign dff_req_id_d = wxbl_req_id; 
`WDFFER(dff_req_id_q, dff_req_id_d, dff_req_id_en, clk ,rst_n) 
assign wxbl_rsp_id = dff_req_id_q; 
end else begin : g_tie_id 
assign wxbl_rsp_id = {ID_W{1'b0}} ; 
end 
assign ahb_haddr = postsplit_req_addr; 
assign ahb_hburst = 3'b000; 
assign ahb_hmastlock = 1'b0; 
assign ahb_hprot[1:0] = postsplit_req_prot[1:0]; 
assign ahb_hprot[3:2] = postsplit_req_memattr[1:0]; 
assign ahb_hsize = {1'b0, postsplit_req_size}; 
assign ahb_htrans = postsplit_req_vld ? 2'b10 : 2'b00; 
assign ahb_hwrite = postsplit_req_write; 
assign ahb_master = postsplit_req_master; 
assign ahb_user = postsplit_req_user ; 
if (PPLN_WDATA==1) begin : g_wdata 
logic dff_req_wdata_en ; 
logic [DATA_W-1:0] dff_req_wdata_d ; 
logic [DATA_W-1:0] dff_req_wdata_q ; 
assign dff_req_wdata_en = wxbl_req_hdsk & postsplit_req_write; 
assign dff_req_wdata_d = wxbl_req_wdata; 
`WDFFER(dff_req_wdata_q, dff_req_wdata_d, dff_req_wdata_en, clk, rst_n) 
assign ahb_hwdata = dff_req_wdata_q; 
end else begin : g_tie_wdata 
assign ahb_hwdata = wxbl_req_wdata; 
end 
endmodule
 
 

 
 
module wxblite_regslice #( 
parameter type req_t = logic, 
parameter type rsp_t = logic, 
parameter MODE = 1 
) ( 
input logic clk , 
input logic rst_n, 
input logic slv_req_vld_i , 
output logic slv_req_rdy_o , 
input req_t slv_req_info_i , 
output logic slv_rsp_vld_o , 
output rsp_t slv_rsp_info_o , 
output logic mst_req_vld_o , 
input logic mst_req_rdy_i , 
output req_t mst_req_info_o , 
input logic mst_rsp_vld_i , 
input rsp_t mst_rsp_info_i 
); 
if(MODE == 0) begin : FEEDTHROUGH 
assign slv_req_rdy_o = mst_req_rdy_i; 
assign slv_rsp_vld_o = mst_rsp_vld_i ; 
assign slv_rsp_info_o = mst_rsp_info_i; 
assign mst_req_vld_o = slv_req_vld_i ; 
assign mst_req_info_o = slv_req_info_i; 
end else if(MODE == 1) begin : FORWARD 
logic slv_req_hdsk; 
logic mst_req_hdsk; 
logic dff_float_vld_set; 
logic dff_float_vld_clr; 
logic dff_float_vld_en ; 
logic dff_float_vld_d ; 
logic dff_float_vld_q ; 
logic dff_sent_set; 
logic dff_sent_clr; 
logic dff_sent_en ; 
logic dff_sent_d ; 
logic dff_sent_q ; 
logic dff_info_en; 
req_t dff_info_d ; 
req_t dff_info_q ; 
assign slv_req_hdsk = slv_req_vld_i & slv_req_rdy_o ; 
assign mst_req_hdsk = mst_req_vld_o & mst_req_rdy_i ; 
assign dff_float_vld_set = slv_req_hdsk; 
assign dff_float_vld_clr = slv_rsp_vld_o; 
assign dff_float_vld_en = dff_float_vld_set | dff_float_vld_clr; 
assign dff_float_vld_d = dff_float_vld_set ; 
`WDFFER (dff_float_vld_q, dff_float_vld_d, dff_float_vld_en, clk, rst_n) 
assign dff_sent_set = mst_req_hdsk; 
assign dff_sent_clr = slv_rsp_vld_o; 
assign dff_sent_en = dff_sent_set | dff_sent_clr; 
assign dff_sent_d = dff_sent_set; 
`WDFFER (dff_sent_q, dff_sent_d, dff_sent_en, clk, rst_n) 
assign dff_info_en = slv_req_hdsk ; 
assign dff_info_d = slv_req_info_i; 
`WDFFER (dff_info_q, dff_info_d, dff_info_en, clk, rst_n) 
assign slv_req_rdy_o = ~dff_float_vld_q | dff_float_vld_q & dff_sent_q & slv_rsp_vld_o; 
assign mst_req_vld_o = dff_float_vld_q & ~dff_sent_q; 
assign mst_req_info_o = dff_info_q ; 
assign slv_rsp_vld_o = mst_rsp_vld_i ; 
assign slv_rsp_info_o = mst_rsp_info_i; 
end else if(MODE == 2) begin : BACKWARD 
end else if(MODE == 3) begin : BIDIRECTION 
logic slv_req_hdsk; 
logic mst_req_hdsk; 
logic dff_float_vld_set; 
logic dff_float_vld_clr; 
logic dff_float_vld_en ; 
logic dff_float_vld_d ; 
logic dff_float_vld_q ; 
logic dff_sent_set; 
logic dff_sent_clr; 
logic dff_sent_en ; 
logic dff_sent_d ; 
logic dff_sent_q ; 
logic dff_info_en; 
req_t dff_info_d ; 
req_t dff_info_q ; 
logic dff_rsp_vld_set; 
logic dff_rsp_vld_clr; 
logic dff_rsp_vld_en; 
logic dff_rsp_vld_d ; 
logic dff_rsp_vld_q ; 
logic dff_rsp_info_en; 
rsp_t dff_rsp_info_d ; 
rsp_t dff_rsp_info_q ; 
assign slv_req_hdsk = slv_req_vld_i & slv_req_rdy_o ; 
assign mst_req_hdsk = mst_req_vld_o & mst_req_rdy_i ; 
assign dff_float_vld_set = slv_req_hdsk; 
assign dff_float_vld_clr = slv_rsp_vld_o; 
assign dff_float_vld_en = dff_float_vld_set | dff_float_vld_clr; 
assign dff_float_vld_d = dff_float_vld_set ; 
`WDFFER (dff_float_vld_q, dff_float_vld_d, dff_float_vld_en, clk, rst_n) 
assign dff_sent_set = mst_req_hdsk; 
assign dff_sent_clr = slv_rsp_vld_o; 
assign dff_sent_en = dff_sent_set | dff_sent_clr; 
assign dff_sent_d = dff_sent_set; 
`WDFFER (dff_sent_q, dff_sent_d, dff_sent_en, clk, rst_n) 
assign dff_info_en = slv_req_hdsk ; 
assign dff_info_d = slv_req_info_i; 
`WDFFER (dff_info_q, dff_info_d, dff_info_en, clk, rst_n) 
assign slv_req_rdy_o = ~dff_float_vld_q | dff_float_vld_q & dff_sent_q & slv_rsp_vld_o; 
assign mst_req_vld_o = dff_float_vld_q & ~dff_sent_q; 
assign mst_req_info_o = dff_info_q ; 
assign dff_rsp_vld_set = dff_float_vld_q & dff_sent_q & mst_rsp_vld_i; 
assign dff_rsp_vld_clr = dff_rsp_vld_q; 
assign dff_rsp_vld_en = dff_rsp_vld_set | dff_rsp_vld_clr; 
assign dff_rsp_vld_d = dff_rsp_vld_set; 
`WDFFER (dff_rsp_vld_q, dff_rsp_vld_d, dff_rsp_vld_en, clk, rst_n) 
assign dff_rsp_info_en = mst_rsp_vld_i; 
assign dff_rsp_info_d = mst_rsp_info_i; 
`WDFFER (dff_rsp_info_q, dff_rsp_info_d, dff_rsp_info_en, clk, rst_n) 
assign slv_rsp_vld_o = dff_rsp_vld_q ; 
assign slv_rsp_info_o = dff_rsp_info_q; 
end 
endmodule
 

 
 
 
 

 
 
module wing_cbb_regslice #( 
parameter type info_t = logic, 
parameter MODE = 0, 
parameter RAR = 0 
) ( 
input logic clk , 
input logic rst_n , 
input logic slv_vld_i , 
output logic slv_rdy_o , 
input info_t slv_info_i, 
output logic mst_vld_o, 
input logic mst_rdy_i, 
output info_t mst_info_o 
); 
if (MODE == 0) begin : mode_ft 
assign slv_rdy_o = mst_rdy_i ; 
assign mst_info_o = slv_info_i ; 
assign mst_vld_o = slv_vld_i ; 
end else if (MODE == 1) begin : mode_forward 
logic slv_hdsk; 
logic mst_hdsk; 
logic dff_req_vld_en ; 
logic dff_req_vld_d ; 
logic dff_req_vld_q ; 
logic dff_info_en; 
info_t dff_info_d ; 
info_t dff_info_q ; 
assign slv_hdsk = slv_vld_i & slv_rdy_o ; 
assign mst_hdsk = mst_vld_o & mst_rdy_i ; 
assign dff_req_vld_en = slv_hdsk | mst_hdsk; 
assign dff_req_vld_d = slv_hdsk ; 
`WDFFER (dff_req_vld_q, dff_req_vld_d, dff_req_vld_en, clk, rst_n) 
assign dff_info_en = slv_hdsk; 
assign dff_info_d = slv_info_i ; 
`WDFFER (dff_info_q, dff_info_d, dff_info_en, clk, rst_n ) 
assign slv_rdy_o = ~dff_req_vld_q | mst_rdy_i ; 
assign mst_vld_o = dff_req_vld_q ; 
assign mst_info_o = dff_info_q ; 
end else if (MODE == 2) begin : mode_backward 
logic dff_req_vld_set; 
logic dff_req_vld_clr; 
logic dff_req_vld_en; 
logic dff_req_vld_d; 
logic dff_req_vld_q; 
logic dff_info_en; 
info_t dff_info_d ; 
info_t dff_info_q ; 
assign dff_req_vld_set = slv_vld_i & ~dff_req_vld_q & ~mst_rdy_i; 
assign dff_req_vld_clr = dff_req_vld_q & mst_rdy_i; 
assign dff_req_vld_en = dff_req_vld_set | dff_req_vld_clr; 
assign dff_req_vld_d = dff_req_vld_set; 
`WDFFER(dff_req_vld_q, dff_req_vld_d, dff_req_vld_en, clk, rst_n) 
assign dff_info_en = dff_req_vld_set; 
assign dff_info_d = slv_info_i ; 
`WDFFER (dff_info_q, dff_info_d, dff_info_en, clk, rst_n) 
assign slv_rdy_o = ~dff_req_vld_q; 
assign mst_vld_o = dff_req_vld_q ? 1'b1 : slv_vld_i ; 
assign mst_info_o = dff_req_vld_q ? dff_info_q : slv_info_i; 
end else if (MODE == 3) begin : mode_full 
logic ppbuf_push; 
logic ppbuf_pop; 
logic ppbuf_vld_lo_set; 
logic ppbuf_vld_lo_clr; 
logic ppbuf_vld_lo_en; 
logic ppbuf_vld_lo_d; 
logic ppbuf_vld_lo_q; 
logic ppbuf_vld_hi_set; 
logic ppbuf_vld_hi_clr; 
logic ppbuf_vld_hi_en; 
logic ppbuf_vld_hi_d; 
logic ppbuf_vld_hi_q; 
logic ppbuf_info_lo_en; 
info_t ppbuf_info_lo_d; 
info_t ppbuf_info_lo_q; 
logic ppbuf_info_hi_en; 
info_t ppbuf_info_hi_d; 
info_t ppbuf_info_hi_q; 
logic ppbuf_info_shift; 
logic ppbuf_full; 
assign ppbuf_full = ppbuf_vld_hi_q & ppbuf_vld_lo_q; 
assign slv_rdy_o = ~ppbuf_full; 
assign mst_vld_o = ppbuf_vld_lo_q; 
assign mst_info_o = ppbuf_info_lo_q; 
assign ppbuf_push = slv_vld_i & slv_rdy_o; 
assign ppbuf_pop = mst_vld_o & mst_rdy_i; 
assign ppbuf_vld_lo_set = ~ppbuf_vld_hi_q & ~ppbuf_vld_lo_q & ppbuf_push; 
assign ppbuf_vld_lo_clr = ~ppbuf_vld_hi_q & ppbuf_vld_lo_q & ~ppbuf_push & ppbuf_pop; 
assign ppbuf_vld_lo_en = ppbuf_vld_lo_set | ppbuf_vld_lo_clr; 
assign ppbuf_vld_lo_d = ppbuf_vld_lo_set; 
`WDFFER(ppbuf_vld_lo_q, ppbuf_vld_lo_d, ppbuf_vld_lo_en, clk, rst_n) 
assign ppbuf_vld_hi_set = ppbuf_vld_lo_q & ppbuf_push & ~ppbuf_pop; 
assign ppbuf_vld_hi_clr = ppbuf_vld_hi_q & ~ppbuf_push & ppbuf_pop; 
assign ppbuf_vld_hi_en = ppbuf_vld_hi_set | ppbuf_vld_hi_clr; 
assign ppbuf_vld_hi_d = ppbuf_vld_hi_set; 
`WDFFER(ppbuf_vld_hi_q, ppbuf_vld_hi_d, ppbuf_vld_hi_en, clk, rst_n) 
assign ppbuf_info_shift = ppbuf_vld_hi_q & ppbuf_vld_lo_q & ppbuf_pop; 
assign ppbuf_info_lo_en = ppbuf_push & (~ppbuf_vld_hi_q & ~ppbuf_vld_lo_q | 
~ppbuf_vld_hi_q & ppbuf_vld_lo_q & ppbuf_pop ) 
| ppbuf_info_shift; 
assign ppbuf_info_lo_d = ppbuf_info_shift ? ppbuf_info_hi_q : slv_info_i; 
assign ppbuf_info_hi_en = ppbuf_push & ~ppbuf_vld_hi_q & ppbuf_vld_lo_q & ~ppbuf_pop; 
assign ppbuf_info_hi_d = slv_info_i; 
if (RAR) begin : g_rar 
`WDFFER(ppbuf_info_lo_q, ppbuf_info_lo_d, ppbuf_info_lo_en, clk, rst_n) 
`WDFFER(ppbuf_info_hi_q, ppbuf_info_hi_d, ppbuf_info_hi_en, clk, rst_n) 
end else begin : g_rnr 
`WDFFENR(ppbuf_info_lo_q, ppbuf_info_lo_d, ppbuf_info_lo_en, clk) 
`WDFFENR(ppbuf_info_hi_q, ppbuf_info_hi_d, ppbuf_info_hi_en, clk) 
end 
end 
endmodule
 
 

 

 
 

 
 
 

 

 
module wing_cbb_lzc #( 
parameter WIDTH = 8, 
parameter DIRECTION = 1, 
parameter IDX_W = $clog2(WIDTH), 
parameter INVERT = 0 
) ( 
input logic [WIDTH-1:0] vec_i, 
output logic [IDX_W-1:0] cnt_o, 
output logic empty_o 
); 
if (WIDTH == 1) begin : gen_degenerate_lzc 
assign cnt_o[0] = !vec_i[0]; 
assign empty_o = !vec_i[0]; 
end else begin : gen_lzc 
localparam WIDTH_ROUND_W = 2**IDX_W; 
logic [WIDTH_ROUND_W-2:0] sel_nodes; 
logic [WIDTH_ROUND_W-2:0][IDX_W-1:0] index_nodes; 
logic [WIDTH-1:0][IDX_W-1:0] index_lut; 
logic [WIDTH-1:0] vector; 
for (genvar j = 0; j < WIDTH; j++) begin : g_index_lut 
assign index_lut[j] = j; 
end 
if(INVERT == 1 )begin: LOC 
assign vector = ~vec_i; 
end else begin: LZC 
assign vector = vec_i; 
end 
if (DIRECTION == 0) begin : trailing_cnt 
for (genvar level = 0; level < IDX_W; level++) begin : g_levels 
if (level == IDX_W - 1) begin : g_last_level 
for (genvar k = 0; k < 2 ** level; k++) begin : g_iter 
if (k * 2 < WIDTH - 1) begin : g_idx_inside 
assign sel_nodes [2 ** level - 1 + k] = vector[k * 2 ] | 
vector[k * 2 + 1] ; 
assign index_nodes[2 ** level - 1 + k] = (vector[k * 2] == 1'b1) ? index_lut[k * 2] : 
index_lut[k * 2 + 1]; 
end else if (k * 2 == WIDTH - 1) begin : g_idx_border 
assign sel_nodes [2 ** level - 1 + k] = vector[k * 2]; 
assign index_nodes[2 ** level - 1 + k] = index_lut[k * 2]; 
end else begin : g_idx_out 
assign sel_nodes [2 ** level - 1 + k] = 1'b0; 
assign index_nodes[2 ** level - 1 + k] = '0; 
end 
end 
end else begin : g_upper_level 
for (genvar l = 0; l < 2 ** level; l++) begin : g_idx_sel 
assign sel_nodes[2 ** level - 1 + l] = sel_nodes[2 ** (level + 1) - 1 + l * 2 ] | 
sel_nodes[2 ** (level + 1) - 1 + l * 2 + 1] ; 
assign index_nodes[2 ** level - 1 + l] = (sel_nodes[2 ** (level + 1) - 1 + l * 2] == 1'b1) ? 
index_nodes[2 ** (level + 1) - 1 + l * 2 ] : 
index_nodes[2 ** (level + 1) - 1 + l * 2 + 1] ; 
end 
end 
end 
end else begin: leading_cnt 
for (genvar level = 0; level < IDX_W; level++) begin : g_levels 
if (level == IDX_W - 1) begin : g_last_level 
for (genvar k = 0; k < 2 ** level; k++) begin : g_iter 
if (k * 2 < WIDTH - 1) begin : g_idx_inside 
assign sel_nodes [2 ** level - 1 + k] = vector[WIDTH-1 -k * 2 ] | 
vector[WIDTH-1 -k * 2 -1] ; 
assign index_nodes[2 ** level - 1 + k] = (vector[WIDTH-1 -k * 2] == 1'b1) ? index_lut[k * 2 ] : 
index_lut[k * 2 + 1] ; 
end else if (k * 2 == WIDTH - 1) begin : g_idx_border 
assign sel_nodes [2 ** level - 1 + k] = vector[WIDTH-1 - k * 2]; 
assign index_nodes[2 ** level - 1 + k] = index_lut[k * 2]; 
end else begin : g_idx_out 
assign sel_nodes [2 ** level - 1 + k] = 1'b0; 
assign index_nodes[2 ** level - 1 + k] = '0; 
end 
end 
end else begin : g_upper_level 
for (genvar l = 0; l < 2 ** level; l++) begin : g_iter 
assign sel_nodes[2 ** level - 1 + l] = sel_nodes[2 ** (level + 1) - 1 + l * 2 ] | 
sel_nodes[2 ** (level + 1) - 1 + l * 2 + 1] ; 
assign index_nodes[2 ** level - 1 + l] = (sel_nodes[2 ** (level + 1) - 1 + l * 2] == 1'b1) ? 
index_nodes[2 ** (level + 1) - 1 + l * 2 ] : 
index_nodes[2 ** (level + 1) - 1 + l * 2 + 1] ; 
end 
end 
end 
end : leading_cnt 
assign cnt_o = index_nodes[0]; 
assign empty_o = ~sel_nodes[0]; 
end : gen_lzc 
endmodule
 

 
module wing_cbb_ecc_enc32 ( 
input logic [31:0] data_i, 
output logic [ 6:0] enc_o 
); 
assign enc_o[0] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[3] ^ data_i[4] ^ data_i[5] ^ data_i[6] ^ data_i[7] ^ data_i[8] ^ data_i[13] ^ data_i[17] ^ data_i[26] ^ data_i[27] ^ data_i[29]; 
assign enc_o[1] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[3] ^ data_i[4] ^ data_i[12] ^ data_i[16] ^ data_i[18] ^ data_i[21] ^ data_i[22] ^ data_i[23] ^ data_i[24] ^ data_i[25] ^ data_i[28]; 
assign enc_o[2] = data_i[0] ^ data_i[5] ^ data_i[6] ^ data_i[7] ^ data_i[8] ^ data_i[11] ^ data_i[15] ^ data_i[18] ^ data_i[19] ^ data_i[21] ^ data_i[22] ^ data_i[30] ^ data_i[31]; 
assign enc_o[3] = data_i[1] ^ data_i[5] ^ data_i[10] ^ data_i[14] ^ data_i[18] ^ data_i[19] ^ data_i[20] ^ data_i[23] ^ data_i[24] ^ data_i[26] ^ data_i[27] ^ data_i[28] ^ data_i[29] ^ data_i[30]; 
assign enc_o[4] = data_i[2] ^ data_i[6] ^ data_i[9] ^ data_i[14] ^ data_i[15] ^ data_i[16] ^ data_i[17] ^ data_i[19] ^ data_i[20] ^ data_i[21] ^ data_i[23] ^ data_i[25] ^ data_i[29] ^ data_i[31]; 
assign enc_o[5] = data_i[3] ^ data_i[7] ^ data_i[9] ^ data_i[10] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[20] ^ data_i[22] ^ data_i[24] ^ data_i[25] ^ data_i[27] ^ data_i[31]; 
assign enc_o[6] = data_i[4] ^ data_i[8] ^ data_i[9] ^ data_i[10] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[14] ^ data_i[15] ^ data_i[16] ^ data_i[17] ^ data_i[26] ^ data_i[28] ^ data_i[30]; 
endmodule
 
module wing_cbb_ecc_dec32 ( 
input logic [38:0] data_i, 
output logic [31:0] data_o, 
output logic [ 6:0] syndrome_o , 
output logic error_detected, 
output logic single_error_o, 
output logic double_error_o 
); 
logic [6:0] code; 
logic [38:0] loc; 
logic [38:0] data_corrected; 
assign code = data_i[38:32]; 
assign syndrome_o[0] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[3] ^ data_i[4] ^ data_i[5] ^ data_i[6] ^ data_i[7] ^ data_i[8] ^ data_i[13] ^ data_i[17] ^ data_i[26] ^ data_i[27] ^ data_i[29] ^ code[0]; 
assign syndrome_o[1] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[3] ^ data_i[4] ^ data_i[12] ^ data_i[16] ^ data_i[18] ^ data_i[21] ^ data_i[22] ^ data_i[23] ^ data_i[24] ^ data_i[25] ^ data_i[28] ^ code[1]; 
assign syndrome_o[2] = data_i[0] ^ data_i[5] ^ data_i[6] ^ data_i[7] ^ data_i[8] ^ data_i[11] ^ data_i[15] ^ data_i[18] ^ data_i[19] ^ data_i[21] ^ data_i[22] ^ data_i[30] ^ data_i[31] ^ code[2]; 
assign syndrome_o[3] = data_i[1] ^ data_i[5] ^ data_i[10] ^ data_i[14] ^ data_i[18] ^ data_i[19] ^ data_i[20] ^ data_i[23] ^ data_i[24] ^ data_i[26] ^ data_i[27] ^ data_i[28] ^ data_i[29] ^ data_i[30] ^ code[3]; 
assign syndrome_o[4] = data_i[2] ^ data_i[6] ^ data_i[9] ^ data_i[14] ^ data_i[15] ^ data_i[16] ^ data_i[17] ^ data_i[19] ^ data_i[20] ^ data_i[21] ^ data_i[23] ^ data_i[25] ^ data_i[29] ^ data_i[31] ^ code[4]; 
assign syndrome_o[5] = data_i[3] ^ data_i[7] ^ data_i[9] ^ data_i[10] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[20] ^ data_i[22] ^ data_i[24] ^ data_i[25] ^ data_i[27] ^ data_i[31] ^ code[5]; 
assign syndrome_o[6] = data_i[4] ^ data_i[8] ^ data_i[9] ^ data_i[10] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[14] ^ data_i[15] ^ data_i[16] ^ data_i[17] ^ data_i[26] ^ data_i[28] ^ data_i[30] ^ code[6]; 
assign error_detected = |syndrome_o; 
assign single_error_o = ^syndrome_o & error_detected; 
assign double_error_o = ~^syndrome_o & error_detected; 
always_comb begin 
unique case (syndrome_o) 
7'b0000111: loc = 39'h00_0000_0001; 
7'b0001011: loc = 39'h00_0000_0002; 
7'b0010011: loc = 39'h00_0000_0004; 
7'b0100011: loc = 39'h00_0000_0008; 
7'b1000011: loc = 39'h00_0000_0010; 
7'b0001101: loc = 39'h00_0000_0020; 
7'b0010101: loc = 39'h00_0000_0040; 
7'b0100101: loc = 39'h00_0000_0080; 
7'b1000101: loc = 39'h00_0000_0100; 
7'b1110000: loc = 39'h00_0000_0200; 
7'b1101000: loc = 39'h00_0000_0400; 
7'b1100100: loc = 39'h00_0000_0800; 
7'b1100010: loc = 39'h00_0000_1000; 
7'b1100001: loc = 39'h00_0000_2000; 
7'b1011000: loc = 39'h00_0000_4000; 
7'b1010100: loc = 39'h00_0000_8000; 
7'b1010010: loc = 39'h00_0001_0000; 
7'b1010001: loc = 39'h00_0002_0000; 
7'b0001110: loc = 39'h00_0004_0000; 
7'b0011100: loc = 39'h00_0008_0000; 
7'b0111000: loc = 39'h00_0010_0000; 
7'b0010110: loc = 39'h00_0020_0000; 
7'b0100110: loc = 39'h00_0040_0000; 
7'b0011010: loc = 39'h00_0080_0000; 
7'b0101010: loc = 39'h00_0100_0000; 
7'b0110010: loc = 39'h00_0200_0000; 
7'b1001001: loc = 39'h00_0400_0000; 
7'b0101001: loc = 39'h00_0800_0000; 
7'b1001010: loc = 39'h00_1000_0000; 
7'b0011001: loc = 39'h00_2000_0000; 
7'b1001100: loc = 39'h00_4000_0000; 
7'b0110100: loc = 39'h00_8000_0000; 
7'b0000001: loc = 39'h01_0000_0000; 
7'b0000010: loc = 39'h02_0000_0000; 
7'b0000100: loc = 39'h04_0000_0000; 
7'b0001000: loc = 39'h08_0000_0000; 
7'b0010000: loc = 39'h10_0000_0000; 
7'b0100000: loc = 39'h20_0000_0000; 
7'b1000000: loc = 39'h40_0000_0000; 
default: loc = 0; 
endcase 
end 
assign data_corrected = loc ^ data_i; 
assign data_o = data_corrected[31:0]; 
endmodule
 
module wing_cbb_ecc_dec64 ( 
input logic [71:0] data_i, 
output logic [63:0] data_o, 
output logic [ 7:0] syndrome_o , 
output logic error_detected, 
output logic single_error_o, 
output logic double_error_o 
); 
logic [7:0] code; 
logic [71:0] loc; 
logic [71:0] data_corrected; 
assign code = data_i[71:64]; 
assign syndrome_o[0] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[3] ^ data_i[4] ^ data_i[5] ^ data_i[6] ^ data_i[7] ^ data_i[10] ^ data_i[13] ^ data_i[14] ^ data_i[17] ^ data_i[20] ^ data_i[23] ^ data_i[24] ^ data_i[27] ^ data_i[35] ^ data_i[43] ^ data_i[46] ^ data_i[47] ^ data_i[51] ^ data_i[52] ^ data_i[53] ^ data_i[56] ^ data_i[57] ^ data_i[58] ^ code[0]; 
assign syndrome_o[1] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[8] ^ data_i[9] ^ data_i[10] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[14] ^ data_i[15] ^ data_i[18] ^ data_i[21] ^ data_i[22] ^ data_i[25] ^ data_i[28] ^ data_i[31] ^ data_i[32] ^ data_i[35] ^ data_i[43] ^ data_i[51] ^ data_i[54] ^ data_i[55] ^ data_i[59] ^ data_i[60] ^ data_i[61] ^ code[1]; 
assign syndrome_o[2] = data_i[3] ^ data_i[4] ^ data_i[5] ^ data_i[8] ^ data_i[9] ^ data_i[10] ^ data_i[16] ^ data_i[17] ^ data_i[18] ^ data_i[19] ^ data_i[20] ^ data_i[21] ^ data_i[22] ^ data_i[23] ^ data_i[26] ^ data_i[29] ^ data_i[30] ^ data_i[33] ^ data_i[36] ^ data_i[39] ^ data_i[40] ^ data_i[43] ^ data_i[51] ^ data_i[59] ^ data_i[62] ^ data_i[63] ^ code[2]; 
assign syndrome_o[3] = data_i[3] ^ data_i[6] ^ data_i[7] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[16] ^ data_i[17] ^ data_i[18] ^ data_i[24] ^ data_i[25] ^ data_i[26] ^ data_i[27] ^ data_i[28] ^ data_i[29] ^ data_i[30] ^ data_i[31] ^ data_i[34] ^ data_i[37] ^ data_i[38] ^ data_i[41] ^ data_i[44] ^ data_i[47] ^ data_i[48] ^ data_i[51] ^ data_i[59] ^ code[3]; 
assign syndrome_o[4] = data_i[3] ^ data_i[11] ^ data_i[14] ^ data_i[15] ^ data_i[19] ^ data_i[20] ^ data_i[21] ^ data_i[24] ^ data_i[25] ^ data_i[26] ^ data_i[32] ^ data_i[33] ^ data_i[34] ^ data_i[35] ^ data_i[36] ^ data_i[37] ^ data_i[38] ^ data_i[39] ^ data_i[42] ^ data_i[45] ^ data_i[46] ^ data_i[49] ^ data_i[52] ^ data_i[55] ^ data_i[56] ^ data_i[59] ^ code[4]; 
assign syndrome_o[5] = data_i[0] ^ data_i[3] ^ data_i[11] ^ data_i[19] ^ data_i[22] ^ data_i[23] ^ data_i[27] ^ data_i[28] ^ data_i[29] ^ data_i[32] ^ data_i[33] ^ data_i[34] ^ data_i[40] ^ data_i[41] ^ data_i[42] ^ data_i[43] ^ data_i[44] ^ data_i[45] ^ data_i[46] ^ data_i[47] ^ data_i[50] ^ data_i[53] ^ data_i[54] ^ data_i[57] ^ data_i[60] ^ data_i[63] ^ code[5]; 
assign syndrome_o[6] = data_i[1] ^ data_i[4] ^ data_i[7] ^ data_i[8] ^ data_i[11] ^ data_i[19] ^ data_i[27] ^ data_i[30] ^ data_i[31] ^ data_i[35] ^ data_i[36] ^ data_i[37] ^ data_i[40] ^ data_i[41] ^ data_i[42] ^ data_i[48] ^ data_i[49] ^ data_i[50] ^ data_i[51] ^ data_i[52] ^ data_i[53] ^ data_i[54] ^ data_i[55] ^ data_i[58] ^ data_i[61] ^ data_i[62] ^ code[6]; 
assign syndrome_o[7] = data_i[2] ^ data_i[5] ^ data_i[6] ^ data_i[9] ^ data_i[12] ^ data_i[15] ^ data_i[16] ^ data_i[19] ^ data_i[27] ^ data_i[35] ^ data_i[38] ^ data_i[39] ^ data_i[43] ^ data_i[44] ^ data_i[45] ^ data_i[48] ^ data_i[49] ^ data_i[50] ^ data_i[56] ^ data_i[57] ^ data_i[58] ^ data_i[59] ^ data_i[60] ^ data_i[61] ^ data_i[62] ^ data_i[63] ^ code[7]; 
assign error_detected = |syndrome_o; 
assign single_error_o = ^syndrome_o & error_detected; 
assign double_error_o = ~^syndrome_o & error_detected; 
always_comb begin 
unique case (syndrome_o) 
8'b00100011: loc = 72'h00_0000_0000_0000_0001; 
8'b01000011: loc = 72'h00_0000_0000_0000_0002; 
8'b10000011: loc = 72'h00_0000_0000_0000_0004; 
8'b00111101: loc = 72'h00_0000_0000_0000_0008; 
8'b01000101: loc = 72'h00_0000_0000_0000_0010; 
8'b10000101: loc = 72'h00_0000_0000_0000_0020; 
8'b10001001: loc = 72'h00_0000_0000_0000_0040; 
8'b01001001: loc = 72'h00_0000_0000_0000_0080; 
8'b01000110: loc = 72'h00_0000_0000_0000_0100; 
8'b10000110: loc = 72'h00_0000_0000_0000_0200; 
8'b00000111: loc = 72'h00_0000_0000_0000_0400; 
8'b01111010: loc = 72'h00_0000_0000_0000_0800; 
8'b10001010: loc = 72'h00_0000_0000_0000_1000; 
8'b00001011: loc = 72'h00_0000_0000_0000_2000; 
8'b00010011: loc = 72'h00_0000_0000_0000_4000; 
8'b10010010: loc = 72'h00_0000_0000_0000_8000; 
8'b10001100: loc = 72'h00_0000_0000_0001_0000; 
8'b00001101: loc = 72'h00_0000_0000_0002_0000; 
8'b00001110: loc = 72'h00_0000_0000_0004_0000; 
8'b11110100: loc = 72'h00_0000_0000_0008_0000; 
8'b00010101: loc = 72'h00_0000_0000_0010_0000; 
8'b00010110: loc = 72'h00_0000_0000_0020_0000; 
8'b00100110: loc = 72'h00_0000_0000_0040_0000; 
8'b00100101: loc = 72'h00_0000_0000_0080_0000; 
8'b00011001: loc = 72'h00_0000_0000_0100_0000; 
8'b00011010: loc = 72'h00_0000_0000_0200_0000; 
8'b00011100: loc = 72'h00_0000_0000_0400_0000; 
8'b11101001: loc = 72'h00_0000_0000_0800_0000; 
8'b00101010: loc = 72'h00_0000_0000_1000_0000; 
8'b00101100: loc = 72'h00_0000_0000_2000_0000; 
8'b01001100: loc = 72'h00_0000_0000_4000_0000; 
8'b01001010: loc = 72'h00_0000_0000_8000_0000; 
8'b00110010: loc = 72'h00_0000_0001_0000_0000; 
8'b00110100: loc = 72'h00_0000_0002_0000_0000; 
8'b00111000: loc = 72'h00_0000_0004_0000_0000; 
8'b11010011: loc = 72'h00_0000_0008_0000_0000; 
8'b01010100: loc = 72'h00_0000_0010_0000_0000; 
8'b01011000: loc = 72'h00_0000_0020_0000_0000; 
8'b10011000: loc = 72'h00_0000_0040_0000_0000; 
8'b10010100: loc = 72'h00_0000_0080_0000_0000; 
8'b01100100: loc = 72'h00_0000_0100_0000_0000; 
8'b01101000: loc = 72'h00_0000_0200_0000_0000; 
8'b01110000: loc = 72'h00_0000_0400_0000_0000; 
8'b10100111: loc = 72'h00_0000_0800_0000_0000; 
8'b10101000: loc = 72'h00_0000_1000_0000_0000; 
8'b10110000: loc = 72'h00_0000_2000_0000_0000; 
8'b00110001: loc = 72'h00_0000_4000_0000_0000; 
8'b00101001: loc = 72'h00_0000_8000_0000_0000; 
8'b11001000: loc = 72'h00_0001_0000_0000_0000; 
8'b11010000: loc = 72'h00_0002_0000_0000_0000; 
8'b11100000: loc = 72'h00_0004_0000_0000_0000; 
8'b01001111: loc = 72'h00_0008_0000_0000_0000; 
8'b01010001: loc = 72'h00_0010_0000_0000_0000; 
8'b01100001: loc = 72'h00_0020_0000_0000_0000; 
8'b01100010: loc = 72'h00_0040_0000_0000_0000; 
8'b01010010: loc = 72'h00_0080_0000_0000_0000; 
8'b10010001: loc = 72'h00_0100_0000_0000_0000; 
8'b10100001: loc = 72'h00_0200_0000_0000_0000; 
8'b11000001: loc = 72'h00_0400_0000_0000_0000; 
8'b10011110: loc = 72'h00_0800_0000_0000_0000; 
8'b10100010: loc = 72'h00_1000_0000_0000_0000; 
8'b11000010: loc = 72'h00_2000_0000_0000_0000; 
8'b11000100: loc = 72'h00_4000_0000_0000_0000; 
8'b10100100: loc = 72'h00_8000_0000_0000_0000; 
8'b00000001: loc = 72'h01_0000_0000_0000_0000; 
8'b00000010: loc = 72'h02_0000_0000_0000_0000; 
8'b00000100: loc = 72'h04_0000_0000_0000_0000; 
8'b00001000: loc = 72'h08_0000_0000_0000_0000; 
8'b00010000: loc = 72'h10_0000_0000_0000_0000; 
8'b00100000: loc = 72'h20_0000_0000_0000_0000; 
8'b01000000: loc = 72'h40_0000_0000_0000_0000; 
8'b10000000: loc = 72'h80_0000_0000_0000_0000; 
default: loc = 0; 
endcase 
end 
assign data_corrected = loc ^ data_i; 
assign data_o = data_corrected[63:0]; 
endmodule
 
module wing_cbb_ecc_enc64 ( 
input logic [63:0] data_i, 
output logic [ 7:0] enc_o 
); 
assign enc_o[0] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[3] ^ data_i[4] ^ data_i[5] ^ data_i[6] ^ data_i[7] ^ data_i[10] ^ data_i[13] ^ data_i[14] ^ data_i[17] ^ data_i[20] ^ data_i[23] ^ data_i[24] ^ data_i[27] ^ data_i[35] ^ data_i[43] ^ data_i[46] ^ data_i[47] ^ data_i[51] ^ data_i[52] ^ data_i[53] ^ data_i[56] ^ data_i[57] ^ data_i[58]; 
assign enc_o[1] = data_i[0] ^ data_i[1] ^ data_i[2] ^ data_i[8] ^ data_i[9] ^ data_i[10] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[14] ^ data_i[15] ^ data_i[18] ^ data_i[21] ^ data_i[22] ^ data_i[25] ^ data_i[28] ^ data_i[31] ^ data_i[32] ^ data_i[35] ^ data_i[43] ^ data_i[51] ^ data_i[54] ^ data_i[55] ^ data_i[59] ^ data_i[60] ^ data_i[61]; 
assign enc_o[2] = data_i[3] ^ data_i[4] ^ data_i[5] ^ data_i[8] ^ data_i[9] ^ data_i[10] ^ data_i[16] ^ data_i[17] ^ data_i[18] ^ data_i[19] ^ data_i[20] ^ data_i[21] ^ data_i[22] ^ data_i[23] ^ data_i[26] ^ data_i[29] ^ data_i[30] ^ data_i[33] ^ data_i[36] ^ data_i[39] ^ data_i[40] ^ data_i[43] ^ data_i[51] ^ data_i[59] ^ data_i[62] ^ data_i[63]; 
assign enc_o[3] = data_i[3] ^ data_i[6] ^ data_i[7] ^ data_i[11] ^ data_i[12] ^ data_i[13] ^ data_i[16] ^ data_i[17] ^ data_i[18] ^ data_i[24] ^ data_i[25] ^ data_i[26] ^ data_i[27] ^ data_i[28] ^ data_i[29] ^ data_i[30] ^ data_i[31] ^ data_i[34] ^ data_i[37] ^ data_i[38] ^ data_i[41] ^ data_i[44] ^ data_i[47] ^ data_i[48] ^ data_i[51] ^ data_i[59]; 
assign enc_o[4] = data_i[3] ^ data_i[11] ^ data_i[14] ^ data_i[15] ^ data_i[19] ^ data_i[20] ^ data_i[21] ^ data_i[24] ^ data_i[25] ^ data_i[26] ^ data_i[32] ^ data_i[33] ^ data_i[34] ^ data_i[35] ^ data_i[36] ^ data_i[37] ^ data_i[38] ^ data_i[39] ^ data_i[42] ^ data_i[45] ^ data_i[46] ^ data_i[49] ^ data_i[52] ^ data_i[55] ^ data_i[56] ^ data_i[59]; 
assign enc_o[5] = data_i[0] ^ data_i[3] ^ data_i[11] ^ data_i[19] ^ data_i[22] ^ data_i[23] ^ data_i[27] ^ data_i[28] ^ data_i[29] ^ data_i[32] ^ data_i[33] ^ data_i[34] ^ data_i[40] ^ data_i[41] ^ data_i[42] ^ data_i[43] ^ data_i[44] ^ data_i[45] ^ data_i[46] ^ data_i[47] ^ data_i[50] ^ data_i[53] ^ data_i[54] ^ data_i[57] ^ data_i[60] ^ data_i[63]; 
assign enc_o[6] = data_i[1] ^ data_i[4] ^ data_i[7] ^ data_i[8] ^ data_i[11] ^ data_i[19] ^ data_i[27] ^ data_i[30] ^ data_i[31] ^ data_i[35] ^ data_i[36] ^ data_i[37] ^ data_i[40] ^ data_i[41] ^ data_i[42] ^ data_i[48] ^ data_i[49] ^ data_i[50] ^ data_i[51] ^ data_i[52] ^ data_i[53] ^ data_i[54] ^ data_i[55] ^ data_i[58] ^ data_i[61] ^ data_i[62]; 
assign enc_o[7] = data_i[2] ^ data_i[5] ^ data_i[6] ^ data_i[9] ^ data_i[12] ^ data_i[15] ^ data_i[16] ^ data_i[19] ^ data_i[27] ^ data_i[35] ^ data_i[38] ^ data_i[39] ^ data_i[43] ^ data_i[44] ^ data_i[45] ^ data_i[48] ^ data_i[49] ^ data_i[50] ^ data_i[56] ^ data_i[57] ^ data_i[58] ^ data_i[59] ^ data_i[60] ^ data_i[61] ^ data_i[62] ^ data_i[63]; 
endmodule
 
 

 
 
 

 
 
 

 
 
module ahb_regslice #( 
parameter ADDR_W = 32, 
parameter DATA_W = 32, 
parameter MODE = 0, 
parameter EN_HBURST = 0, 
parameter EN_HMLOCK = 0, 
parameter EN_MISC = 0, 
parameter type misc_t = logic 
) ( 
input logic clk , 
input logic rst_n , 
input logic [ADDR_W-1:0] slv_haddr , 
input logic [2:0] slv_hburst , 
input logic slv_hmastlock , 
input logic [3:0] slv_hprot , 
input logic [2:0] slv_hsize , 
input logic [1:0] slv_hmaster , 
input logic [1:0] slv_htrans , 
input misc_t slv_misc , 
input logic slv_hwrite , 
input logic [DATA_W-1:0] slv_hwdata , 
output logic [DATA_W-1:0] slv_hrdata , 
output logic slv_hready , 
output logic slv_hresp , 
output logic [ADDR_W-1:0] mst_haddr , 
output logic [2:0] mst_hburst , 
output logic mst_hmastlock , 
output logic [3:0] mst_hprot , 
output logic [2:0] mst_hsize , 
output logic [1:0] mst_hmaster , 
output logic [1:0] mst_htrans , 
output misc_t mst_misc , 
output logic mst_hwrite , 
output logic [DATA_W-1:0] mst_hwdata , 
input logic [DATA_W-1:0] mst_hrdata , 
input logic mst_hready , 
input logic mst_hresp 
); 
if(MODE == 0) begin : FEEDTHROUGH 
assign mst_haddr = slv_haddr ; 
assign mst_hburst = slv_hburst ; 
assign mst_hmastlock = slv_hmastlock; 
assign mst_hprot = slv_hprot ; 
assign mst_hsize = slv_hsize ; 
assign mst_hmaster = slv_hmaster ; 
assign mst_htrans = slv_htrans ; 
assign mst_misc = slv_misc ; 
assign mst_hwrite = slv_hwrite ; 
assign mst_hwdata = slv_hwdata ; 
assign slv_hrdata = mst_hrdata; 
assign slv_hready = mst_hready; 
assign slv_hresp = mst_hresp ; 
end else if(MODE == 1) begin : FORWARD 
logic dff_htrans_en; 
logic [1:0] dff_htrans_d ; 
logic [1:0] dff_htrans_q ; 
logic dff_haddr_en ; 
logic [ADDR_W-1:0] dff_haddr_q ; 
logic [3:0] dff_hprot_q ; 
logic [2:0] dff_hsize_q ; 
logic dff_hwrite_q ; 
logic [1:0] dff_hmaster_q; 
logic dff_slv_addr_sent_set; 
logic dff_slv_addr_sent_clr; 
logic dff_slv_addr_sent_en; 
logic dff_slv_addr_sent_d; 
logic dff_slv_addr_sent_q; 
logic dff_mst_addr_sent_set; 
logic dff_mst_addr_sent_clr; 
logic dff_mst_addr_sent_en; 
logic dff_mst_addr_sent_d; 
logic dff_mst_addr_sent_q; 
logic slv_htrans_non_idle; 
logic mst_htrans_non_idle; 
logic dff_hwdata_en; 
logic [DATA_W-1:0] dff_hwdata_q; 
assign slv_htrans_non_idle = |slv_htrans; 
assign mst_htrans_non_idle = |mst_htrans; 
assign dff_slv_addr_sent_set = slv_hready & slv_htrans_non_idle; 
assign dff_slv_addr_sent_clr = slv_hready & dff_slv_addr_sent_q; 
assign dff_slv_addr_sent_en = dff_slv_addr_sent_set | dff_slv_addr_sent_clr; 
assign dff_slv_addr_sent_d = dff_slv_addr_sent_set; 
`WDFFER(dff_slv_addr_sent_q, dff_slv_addr_sent_d, dff_slv_addr_sent_en, clk, rst_n) 
assign dff_mst_addr_sent_set = mst_hready & mst_htrans_non_idle; 
assign dff_mst_addr_sent_clr = mst_hready & dff_mst_addr_sent_q; 
assign dff_mst_addr_sent_en = dff_mst_addr_sent_set | dff_mst_addr_sent_clr; 
assign dff_mst_addr_sent_d = dff_mst_addr_sent_set; 
`WDFFER(dff_mst_addr_sent_q, dff_mst_addr_sent_d, dff_mst_addr_sent_en, clk, rst_n) 
assign dff_htrans_en = dff_slv_addr_sent_set | 
~slv_hready & mst_htrans_non_idle ; 
assign dff_htrans_d = dff_slv_addr_sent_set ? slv_htrans : 2'b00; 
`WDFFER(dff_htrans_q, dff_htrans_d, dff_htrans_en, clk, rst_n) 
assign dff_haddr_en = dff_slv_addr_sent_set; 
`WDFFER(dff_haddr_q , slv_haddr , dff_haddr_en, clk, rst_n) 
`WDFFER(dff_hprot_q , slv_hprot , dff_haddr_en, clk, rst_n) 
`WDFFER(dff_hsize_q , slv_hsize , dff_haddr_en, clk, rst_n) 
`WDFFER(dff_hwrite_q , slv_hwrite , dff_haddr_en, clk, rst_n) 
`WDFFER(dff_hmaster_q, slv_hmaster, dff_haddr_en, clk, rst_n) 
if (EN_HBURST == 1) begin : g_reg_hburst 
logic [2:0] dff_hburst_q; 
`WDFFER(dff_hburst_q, slv_hburst, dff_haddr_en, clk, rst_n) 
assign mst_hburst = dff_hburst_q; 
end else begin : g_const_hburst 
assign mst_hburst = 3'b000; 
end 
if (EN_HMLOCK == 1) begin : g_reg_hmlock 
logic dff_hmastlock_q; 
`WDFFER(dff_hmastlock_q, slv_hmastlock, dff_haddr_en, clk, rst_n) 
assign mst_hmastlock = dff_hmastlock_q; 
end else begin : g_const_hmlock 
assign mst_hmastlock = 1'b0; 
end 
if (EN_MISC == 1) begin : g_reg_misc 
misc_t dff_misc_q; 
`WDFFER(dff_misc_q, slv_misc, dff_haddr_en, clk, rst_n) 
assign mst_misc = dff_misc_q; 
end else begin : g_tie_misc 
assign mst_misc = '0; 
end 
assign dff_hwdata_en = dff_slv_addr_sent_q & ~dff_mst_addr_sent_q & dff_hwrite_q; 
`WDFFER(dff_hwdata_q, slv_hwdata, dff_hwdata_en, clk, rst_n) 
assign mst_haddr = dff_haddr_q ; 
assign mst_hprot = dff_hprot_q ; 
assign mst_hsize = dff_hsize_q ; 
assign mst_hmaster = dff_hmaster_q; 
assign mst_htrans = dff_htrans_q ; 
assign mst_hwrite = dff_hwrite_q ; 
assign mst_hwdata = dff_hwdata_q ; 
assign slv_hready = ~dff_slv_addr_sent_q | 
dff_mst_addr_sent_q & mst_hready; 
assign slv_hrdata = mst_hrdata; 
assign slv_hresp = mst_hresp; 
end else if(MODE == 2) begin : BACKWARD 
logic dff_float_vld_set; 
logic dff_float_vld_clr; 
logic dff_float_vld_en ; 
logic dff_float_vld_d ; 
logic dff_float_vld_q ; 
logic dff_hrdata_en; 
logic [DATA_W-1:0] dff_hrdata_d; 
logic [DATA_W-1:0] dff_hrdata_q; 
logic dff_hresp_q ; 
assign dff_float_vld_set = slv_htrans[1] & slv_hready; 
assign dff_float_vld_clr = mst_hready & dff_float_vld_q; 
assign dff_float_vld_en = dff_float_vld_set | dff_float_vld_clr; 
assign dff_float_vld_d = dff_float_vld_set; 
`WDFFER(dff_float_vld_q, dff_float_vld_d, dff_float_vld_en, clk, rst_n) 
assign dff_hrdata_en = dff_float_vld_clr; 
assign dff_hrdata_d = mst_hrdata; 
`WDFFER(dff_hrdata_q, dff_hrdata_d, dff_hrdata_en, clk, rst_n) 
`WDFFR(dff_hresp_q, mst_hresp, clk, rst_n) 
assign slv_hready = ~dff_float_vld_q; 
assign mst_htrans = dff_float_vld_q ? 2'b00 : slv_htrans; 
assign mst_haddr = slv_haddr; 
assign mst_hburst = slv_hburst; 
assign mst_hmastlock = slv_hmastlock; 
assign mst_hprot = slv_hprot; 
assign mst_hsize = slv_hsize; 
assign mst_hmaster = slv_hmaster; 
assign mst_misc = slv_misc; 
assign mst_hwrite = slv_hwrite; 
assign mst_hwdata = slv_hwdata; 
assign slv_hresp = dff_hresp_q; 
assign slv_hrdata = dff_hrdata_q; 
end else if(MODE == 3)begin :BIDIRECTION 
logic [ADDR_W-1:0] dff_haddr_q ; 
logic [3:0] dff_hprot_q ; 
logic [2:0] dff_hsize_q ; 
logic dff_hwrite_q ; 
logic [1:0] dff_hmaster_q; 
logic [DATA_W-1:0] dff_hrdata_q ; 
logic dff_hresp_q ; 
logic dff_hready_q ; 
logic dff_float_req_set; 
logic dff_float_req_clr; 
logic dff_float_req_en; 
logic dff_float_req_d; 
logic dff_float_req_q; 
logic dff_mst_addr_sent_set; 
logic dff_mst_addr_sent_clr; 
logic dff_mst_addr_sent_en; 
logic dff_mst_addr_sent_d; 
logic dff_mst_addr_sent_q; 
logic dff_float_vld_set; 
logic dff_float_vld_clr; 
logic dff_float_vld_en ; 
logic dff_float_vld_d ; 
logic dff_float_vld_q ; 
logic slv_htrans_non_idle; 
logic mst_htrans_non_idle; 
logic dff_res_en; 
logic dff_req_en; 
logic dff_hdata_en ; 
logic dff_hwdata_en; 
logic [DATA_W-1:0] dff_hwdata_q ; 
assign slv_htrans_non_idle = |slv_htrans; 
assign mst_htrans_non_idle = |mst_htrans; 
assign dff_float_req_set = slv_hready & slv_htrans_non_idle; 
assign dff_float_req_clr = mst_hready & dff_float_req_q; 
assign dff_float_req_en = dff_float_req_set | dff_float_req_clr; 
assign dff_float_req_d = dff_float_req_set; 
`WDFFER(dff_float_req_q, dff_float_req_d, dff_float_req_en, clk, rst_n) 
assign dff_mst_addr_sent_set = mst_hready & mst_htrans_non_idle; 
assign dff_mst_addr_sent_clr = mst_hready & dff_mst_addr_sent_q; 
assign dff_mst_addr_sent_en = dff_mst_addr_sent_set | dff_mst_addr_sent_clr; 
assign dff_mst_addr_sent_d = dff_mst_addr_sent_set ; 
`WDFFER(dff_mst_addr_sent_q, dff_mst_addr_sent_d, dff_mst_addr_sent_en, clk, rst_n) 
assign dff_req_en = slv_hready & slv_htrans_non_idle; 
`WDFFER(dff_haddr_q , slv_haddr , dff_req_en, clk, rst_n) 
`WDFFER(dff_hprot_q , slv_hprot , dff_req_en, clk, rst_n) 
`WDFFER(dff_hsize_q , slv_hsize , dff_req_en, clk, rst_n) 
`WDFFER(dff_hwrite_q , slv_hwrite , dff_req_en, clk, rst_n) 
`WDFFER(dff_hmaster_q, slv_hmaster, dff_req_en, clk, rst_n) 
if (EN_HBURST == 1) begin : g_reg_hburst 
logic [2:0] dff_hburst_q; 
`WDFFER(dff_hburst_q, slv_hburst, dff_req_en, clk, rst_n) 
assign mst_hburst = dff_hburst_q; 
end else begin : g_const_hburst 
assign mst_hburst = 3'b000; 
end 
if (EN_HMLOCK == 1) begin : g_reg_hmlock 
logic dff_hmastlock_q; 
`WDFFER(dff_hmastlock_q, slv_hmastlock, dff_req_en, clk, rst_n) 
assign mst_hmastlock = dff_hmastlock_q; 
end else begin : g_const_hmlock 
assign mst_hmastlock = 1'b0; 
end 
if (EN_MISC == 1) begin : g_reg_misc 
misc_t dff_misc_q; 
`WDFFER(dff_misc_q, slv_misc, dff_req_en, clk, rst_n) 
assign mst_misc = dff_misc_q; 
end else begin : g_tie_misc 
assign mst_misc = '0; 
end 
assign dff_hwdata_en = dff_float_req_q & dff_hwrite_q; 
`WDFFER(dff_hwdata_q, slv_hwdata, dff_hwdata_en, clk, rst_n) 
assign dff_res_en = dff_mst_addr_sent_q ; 
`WDFFER(dff_hrdata_q, mst_hrdata, dff_res_en, clk, rst_n) 
`WDFFER(dff_hresp_q, mst_hresp, dff_res_en, clk, rst_n) 
assign slv_hready = ~dff_float_req_q & ~dff_mst_addr_sent_q; 
assign slv_hresp = dff_hresp_q; 
assign slv_hrdata = dff_hrdata_q; 
assign mst_htrans = {dff_float_req_q, 1'b0}; 
assign mst_haddr = dff_haddr_q ; 
assign mst_hprot = dff_hprot_q ; 
assign mst_hsize = dff_hsize_q ; 
assign mst_hmaster = dff_hmaster_q; 
assign mst_hwrite = dff_hwrite_q ; 
assign mst_hwdata = dff_hwdata_q ; 
end 
endmodule
 
 

 

 
 

 
 

 
 
module wing_m130_top ( 
input logic always_on_clk , 
input logic clk , 
output logic clk_gate_en , 
input logic pwrup_rst_n , 
input logic ndm_rst_n , 
output logic dm_ndmreset , 
input logic soft_rst_n , 
output logic soft_rst_req , 
output logic core_sleeping , 
output logic core_deep_sleeping , 
input logic [32-1:0] boot_pc , 
input logic [32-1:0] hart_id , 
input logic ref_clk , 
input logic [32-1:0] core_mmr_base_addr , 
input logic [15:0] reri_bank_inst_id , 
input logic core_wait , 
input logic endianess , 
output logic halted , 
output logic lockup , 
input logic [32+1:0] timer_calibration , 
input logic trst_n , 
input logic tck , 
input logic tms , 
input logic tdi , 
output logic tdo , 
output logic tdo_en , 
input logic ext_interrupt , 
input logic [64-1:0] loc_interrupt , 
output logic [31:0] i_haddr , 
output logic [2:0] i_hburst , 
output logic [3:0] i_hprot , 
output logic [2:0] i_hsize , 
output logic [1:0] i_htrans , 
output logic i_hwrite , 
output logic [31:0] i_hwdata , 
input logic [31:0] i_hrdata , 
input logic i_hready , 
input logic i_hresp , 
output logic [1:0] i_hmaster , 
output logic [31:0] d_haddr , 
output logic [2:0] d_hburst , 
output logic [3:0] d_hprot , 
output logic [2:0] d_hsize , 
output logic [1:0] d_htrans , 
output logic d_hwrite , 
output logic [31:0] d_hwdata , 
input logic [31:0] d_hrdata , 
input logic d_hready , 
input logic d_hresp , 
output logic [1:0] d_hmaster , 
output logic [31:0] m_haddr , 
output logic [2:0] m_hburst , 
output logic [3:0] m_hprot , 
output logic [2:0] m_hsize , 
output logic [1:0] m_htrans , 
output logic m_hwrite , 
output logic [31:0] m_hwdata , 
input logic [31:0] m_hrdata , 
input logic m_hready , 
input logic m_hresp , 
output logic [1:0] m_hmaster , 
`ifdef M130_CORE_SUPPORT_ICACHE 
output logic [2-1:0] ic_sram_tag_cs , 
output logic [2-1:0] ic_sram_tag_wr , 
output logic [2-1:0][$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] ic_sram_tag_addr , 
output logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_wdata , 
input logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_rdata , 
output logic [2-1:0] ic_sram_data_cs , 
output logic [2-1:0] ic_sram_data_wr , 
output logic [2-1:0][($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64)))-1:0] ic_sram_data_addr , 
output logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_wdata , 
input logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_rdata , 
`endif 
input logic dbg_authen , 
output logic [15:0] err_bus , 
input logic dft_icg_scan_en , 
input logic dft_scan_mode , 
input logic dft_scan_rst_n 
); 
logic [32-1:0] core_itcm_base_addr ; 
assign core_itcm_base_addr = 32'b0; 
logic [32-1:0] core_dtcm_base_addr ; 
assign core_dtcm_base_addr = 32'b0; 
logic dm_active_rst_n ; 
logic dm_rst_n ; 
logic tm_rst_n ; 
logic dm_core_reset ; 
logic dm_dmactive ; 
logic dm_pcu_halt_req ; 
logic dm_pcu_halt_on_reset ; 
logic dm_pcu_resume_req ; 
logic pcu_dm_halted ; 
logic pcu_dm_havereset ; 
logic dm_pcu_ack_havereset ; 
logic pcu_dm_unavail ; 
logic pcu_dm_cmd_done ; 
logic pcu_dm_cmd_excp ; 
logic pcu_dm_bus_err ; 
logic dm_pcu_dsch0_write ; 
logic [32-1:0] dm_pcu_dsch0_wdata ; 
logic [32-1:0] pcu_dm_dsch0_rdata ; 
logic dm_ifu_inst_vld ; 
logic ifu_dm_inst_rdy ; 
logic [32-1:0] dm_ifu_inst_data ; 
logic tm_pcu_csr_done ; 
logic [32-1:0] tm_pcu_csr_rdata ; 
logic tm_pcu_csr_excp ; 
logic tm_pcu_csr_match ; 
logic [2-1:0] pcu_tm_pc_ex_vld ; 
logic [2-1:0][32-1:0] pcu_tm_pc_ex ; 
logic [2-1:0] pcu_tm_inst_size_ex ; 
logic [2-1:0] tm_pcu_trigger_pc_vld ; 
logic [2-1:0] tm_pcu_trigger_pc_debug ; 
logic [2-1:0][4-1:0] tm_pcu_trigger_hit_pc_idx ; 
logic pcu_tm_trap_vld ; 
logic [32-1:0] pcu_tm_trap_cause ; 
logic [2-1:0] pcu_tm_inst_retire ; 
logic pcu_tm_tcontrol_mte ; 
logic pcu_tm_trigger_hit ; 
logic[4-1:0] pcu_tm_trigger_hit_idx ; 
logic lsu_tm_a_vld ; 
logic[32-1:0] lsu_tm_a_addr ; 
logic[2:0] lsu_tm_a_ldst ; 
logic[2-1:0] lsu_tm_a_size ; 
logic tm_lsu_trigger_a_vld ; 
logic tm_lsu_trigger_debug_a ; 
logic[4-1:0] tm_lsu_trigger_hit_a_idx ; 
logic lsu_tm_d_vld ; 
logic[32-1:0] lsu_tm_d_data ; 
logic[2:0] lsu_tm_d_ldst ; 
logic[2-1:0] lsu_tm_d_size ; 
logic tm_lsu_trigger_d_vld ; 
logic tm_lsu_trigger_debug_d ; 
logic [15:0] top_tm_vld_vec ; 
logic gated_clk ; 
logic core_rst_n ; 
logic [1:0] core_priv_mode ; 
logic core_dbg_mode ; 
logic core_sleep_mode ; 
logic pcu_csr_vld ; 
logic [1:0] pcu_csr_opcode ; 
logic [11:0] pcu_csr_index ; 
logic [32-1:0] pcu_csr_wdata ; 
logic clic_tm_intctl_vld ; 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
logic [15:0] m130_err_bus ; 
logic pwrup_rst_n_sync ; 
logic ref_clk_sync ; 
logic pwrup_rst_n_dft ; 
wing_cbb_dft_mux2 pwrup_rst_n_dft_inst (.din0(pwrup_rst_n), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(pwrup_rst_n_dft)); 
wing_cbb_bit_sync #( 
.STAGE_N (3 ) 
) u_pwrup_rst_n_sync ( 
.clk (always_on_clk ), 
.rst_n (pwrup_rst_n_dft ), 
.data_i (1'b1 ), 
.data_o (pwrup_rst_n_sync ) 
); 
logic refclk_rst_n_dft; 
wing_cbb_dft_mux2 refclk_rst_n_dft_inst (.din0(pwrup_rst_n_sync), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(refclk_rst_n_dft)); 
wing_cbb_bit_sync #( 
.STAGE_N ( 2 ), 
.DATA_W ( 1 ) 
) u_refclk_async ( 
.clk (always_on_clk ), 
.rst_n (refclk_rst_n_dft ), 
.data_i (ref_clk ), 
.data_o (ref_clk_sync ) 
); 
wing_m130 u_wing_m130 ( 
.always_on_clk (always_on_clk ), 
.clk (clk ), 
.clk_gate_en (clk_gate_en ), 
.pwrup_rst_n (pwrup_rst_n_sync ), 
.ndm_rst_n (ndm_rst_n ), 
.soft_rst_n (soft_rst_n ), 
.soft_rst_req (soft_rst_req ), 
.core_sleeping (core_sleeping ), 
.core_deep_sleeping (core_deep_sleeping ), 
.boot_pc (boot_pc ), 
.hart_id (hart_id ), 
.ref_clk (ref_clk_sync ), 
.core_itcm_base_addr (core_itcm_base_addr ), 
.core_dtcm_base_addr (core_dtcm_base_addr ), 
.core_mmr_base_addr (core_mmr_base_addr ), 
.reri_bank_inst_id (reri_bank_inst_id ), 
.core_wait (core_wait ), 
.endianess (endianess ), 
.halted (halted ), 
.lockup (lockup ), 
.timer_calibration (timer_calibration ), 
.ext_interrupt (ext_interrupt ), 
.loc_interrupt (loc_interrupt ), 
.i_haddr (i_haddr ), 
.i_hburst (i_hburst ), 
.i_hprot (i_hprot ), 
.i_hsize (i_hsize ), 
.i_htrans (i_htrans ), 
.i_hwrite (i_hwrite ), 
.i_hwdata (i_hwdata ), 
.i_hrdata (i_hrdata ), 
.i_hready (i_hready ), 
.i_hresp (i_hresp ), 
.i_hmaster (i_hmaster ), 
.d_haddr (d_haddr ), 
.d_hburst (d_hburst ), 
.d_hprot (d_hprot ), 
.d_hsize (d_hsize ), 
.d_htrans (d_htrans ), 
.d_hwrite (d_hwrite ), 
.d_hwdata (d_hwdata ), 
.d_hrdata (d_hrdata ), 
.d_hready (d_hready ), 
.d_hresp (d_hresp ), 
.d_hmaster (d_hmaster ), 
.m_haddr (m_haddr ), 
.m_hburst (m_hburst ), 
.m_hprot (m_hprot ), 
.m_hsize (m_hsize ), 
.m_htrans (m_htrans ), 
.m_hwrite (m_hwrite ), 
.m_hwdata (m_hwdata ), 
.m_hrdata (m_hrdata ), 
.m_hready (m_hready ), 
.m_hresp (m_hresp ), 
.m_hmaster (m_hmaster ), 
.dm_active_rst_n (dm_active_rst_n ), 
.dm_rst_n (dm_rst_n ), 
.tm_rst_n (tm_rst_n ), 
.dm_core_reset (dm_core_reset ), 
.dm_dmactive (dm_dmactive ), 
.dm_pcu_halt_req (dm_pcu_halt_req ), 
.dm_pcu_halt_on_reset (dm_pcu_halt_on_reset ), 
.dm_pcu_resume_req (dm_pcu_resume_req ), 
.pcu_dm_halted (pcu_dm_halted ), 
.pcu_dm_havereset (pcu_dm_havereset ), 
.dm_pcu_ack_havereset (dm_pcu_ack_havereset ), 
.pcu_dm_unavail (pcu_dm_unavail ), 
.pcu_dm_cmd_done (pcu_dm_cmd_done ), 
.pcu_dm_cmd_excp (pcu_dm_cmd_excp ), 
.pcu_dm_bus_err (pcu_dm_bus_err ), 
.dm_pcu_dsch0_write (dm_pcu_dsch0_write ), 
.pcu_dm_dsch0_rdata (pcu_dm_dsch0_rdata ), 
.dm_pcu_dsch0_wdata (dm_pcu_dsch0_wdata ), 
.dm_ifu_inst_vld (dm_ifu_inst_vld ), 
.ifu_dm_inst_rdy (ifu_dm_inst_rdy ), 
.dm_ifu_inst_data (dm_ifu_inst_data ), 
.tm_pcu_csr_done (tm_pcu_csr_done ), 
.tm_pcu_csr_rdata (tm_pcu_csr_rdata ), 
.tm_pcu_csr_excp (tm_pcu_csr_excp ), 
.tm_pcu_csr_match (tm_pcu_csr_match ), 
.pcu_tm_pc_ex_vld (pcu_tm_pc_ex_vld ), 
.pcu_tm_pc_ex (pcu_tm_pc_ex ), 
.pcu_tm_inst_size_ex (pcu_tm_inst_size_ex ), 
.pcu_tm_trap_vld (pcu_tm_trap_vld ), 
.pcu_tm_trap_cause (pcu_tm_trap_cause ), 
.pcu_tm_inst_retire (pcu_tm_inst_retire ), 
.pcu_tm_tcontrol_mte (pcu_tm_tcontrol_mte ), 
.pcu_tm_trigger_hit (pcu_tm_trigger_hit ), 
.pcu_tm_trigger_hit_idx (pcu_tm_trigger_hit_idx ), 
.tm_pcu_trigger_pc_vld (tm_pcu_trigger_pc_vld ), 
.tm_pcu_trigger_pc_debug (tm_pcu_trigger_pc_debug ), 
.tm_pcu_trigger_hit_pc_idx(tm_pcu_trigger_hit_pc_idx), 
.lsu_tm_a_vld (lsu_tm_a_vld ), 
.lsu_tm_a_addr (lsu_tm_a_addr ), 
.lsu_tm_a_ldst (lsu_tm_a_ldst ), 
.lsu_tm_a_size (lsu_tm_a_size ), 
.tm_lsu_trigger_a_vld (tm_lsu_trigger_a_vld ), 
.tm_lsu_trigger_debug_a (tm_lsu_trigger_debug_a ), 
.tm_lsu_trigger_hit_a_idx(tm_lsu_trigger_hit_a_idx), 
.lsu_tm_d_vld (lsu_tm_d_vld ), 
.lsu_tm_d_data (lsu_tm_d_data ), 
.lsu_tm_d_ldst (lsu_tm_d_ldst ), 
.lsu_tm_d_size (lsu_tm_d_size ), 
.tm_lsu_trigger_d_vld (tm_lsu_trigger_d_vld ), 
.tm_lsu_trigger_debug_d (tm_lsu_trigger_debug_d ), 
.gated_clk (gated_clk ), 
.core_rst_n (core_rst_n ), 
.core_priv_mode (core_priv_mode ), 
.core_dbg_mode (core_dbg_mode ), 
.core_sleep_mode (core_sleep_mode ), 
.pcu_csr_vld (pcu_csr_vld ), 
.pcu_csr_opcode (pcu_csr_opcode ), 
.pcu_csr_index (pcu_csr_index ), 
.pcu_csr_wdata (pcu_csr_wdata ), 
.clic_tm_intctl_vld (clic_tm_intctl_vld ), 
`ifdef M130_CORE_SUPPORT_ICACHE 
.ic_sram_tag_cs (ic_sram_tag_cs ), 
.ic_sram_tag_wr (ic_sram_tag_wr ), 
.ic_sram_tag_addr (ic_sram_tag_addr ), 
.ic_sram_tag_wdata (ic_sram_tag_wdata ), 
.ic_sram_tag_rdata (ic_sram_tag_rdata ), 
.ic_sram_data_cs (ic_sram_data_cs ), 
.ic_sram_data_wr (ic_sram_data_wr ), 
.ic_sram_data_addr (ic_sram_data_addr ), 
.ic_sram_data_wdata (ic_sram_data_wdata ), 
.ic_sram_data_rdata (ic_sram_data_rdata ), 
`endif 
.err_bus (m130_err_bus ), 
.dft_icg_scan_en (dft_icg_scan_en ), 
.dft_scan_mode (dft_scan_mode ), 
.dft_scan_rst_n (dft_scan_rst_n ) 
); 
m130_ds_top u_m130_ds_top ( 
.clk (always_on_clk ), 
.pwrup_rst_n (dm_active_rst_n ), 
.dm_sft_rst_n (dm_rst_n ), 
.dm_top_ndmreset (dm_ndmreset ), 
.dm_top_core_reset (dm_core_reset ), 
.dm_top_dmactive (dm_dmactive ), 
.top_dm_auth_bit_unfused (dbg_authen ), 
.trst_n (trst_n ), 
.tck (tck ), 
.tms (tms ), 
.tdi (tdi ), 
.tdo (tdo ), 
.tdo_en (tdo_en ), 
.dft_icg_scan_en (dft_icg_scan_en ), 
.dft_scan_mode (dft_scan_mode ), 
.dft_scan_rst_n (dft_scan_rst_n ), 
.dm_pcu_halt_req (dm_pcu_halt_req ), 
.dm_pcu_halt_on_reset (dm_pcu_halt_on_reset ), 
.dm_pcu_resume_req (dm_pcu_resume_req ), 
.pcu_dm_halted (pcu_dm_halted ), 
.pcu_dm_havereset (pcu_dm_havereset ), 
.dm_pcu_ack_havereset (dm_pcu_ack_havereset ), 
.pcu_dm_unavail (pcu_dm_unavail ), 
.pcu_dm_cmd_done (pcu_dm_cmd_done ), 
.pcu_dm_cmd_excp (pcu_dm_cmd_excp ), 
.pcu_dm_bus_err (pcu_dm_bus_err ), 
.dm_pcu_dsch0_write (dm_pcu_dsch0_write ), 
.pcu_dm_dsch0_rdata (pcu_dm_dsch0_rdata ), 
.dm_pcu_dsch0_wdata (dm_pcu_dsch0_wdata ), 
.dm_ifu_inst_vld (dm_ifu_inst_vld ), 
.ifu_dm_inst_rdy (ifu_dm_inst_rdy ), 
.dm_ifu_inst_data (dm_ifu_inst_data ) 
); 
m130_tm u_m130_tm( 
.clk (gated_clk ), 
.rst_n (tm_rst_n ), 
.top_tm_vld_vec (top_tm_vld_vec ), 
.core_priv_mode (core_priv_mode ), 
.core_dbg_mode (core_dbg_mode ), 
.pcu_csr_vld (pcu_csr_vld ), 
.pcu_csr_opcode (pcu_csr_opcode ), 
.pcu_csr_index (pcu_csr_index ), 
.pcu_csr_wdata (pcu_csr_wdata ), 
.tm_pcu_csr_done (tm_pcu_csr_done ), 
.tm_pcu_csr_rdata (tm_pcu_csr_rdata ), 
.tm_pcu_csr_excp (tm_pcu_csr_excp ), 
.tm_pcu_csr_match (tm_pcu_csr_match ), 
.pcu_tm_pc_vld (pcu_tm_pc_ex_vld ), 
.pcu_tm_pc (pcu_tm_pc_ex ), 
.pcu_tm_inst_size (pcu_tm_inst_size_ex ), 
.pcu_tm_trap_vld (pcu_tm_trap_vld ), 
.pcu_tm_trap_cause (pcu_tm_trap_cause ), 
.pcu_tm_inst_retire (pcu_tm_inst_retire ), 
.pcu_tm_tcontrol_mte (pcu_tm_tcontrol_mte ), 
.pcu_tm_trigger_hit (pcu_tm_trigger_hit ), 
.pcu_tm_trigger_hit_idx (pcu_tm_trigger_hit_idx ), 
.tm_pcu_trigger_pc_vld (tm_pcu_trigger_pc_vld ), 
.tm_pcu_trigger_pc_debug (tm_pcu_trigger_pc_debug ), 
.tm_pcu_trigger_hit_pc_idx (tm_pcu_trigger_hit_pc_idx ), 
.lsu_tm_a_vld (lsu_tm_a_vld ), 
.lsu_tm_a_addr (lsu_tm_a_addr ), 
.lsu_tm_a_ldst (lsu_tm_a_ldst ), 
.lsu_tm_a_size (lsu_tm_a_size ), 
.tm_lsu_trigger_a_vld (tm_lsu_trigger_a_vld ), 
.tm_lsu_trigger_debug_a (tm_lsu_trigger_debug_a ), 
.tm_lsu_trigger_hit_a_idx (tm_lsu_trigger_hit_a_idx ), 
.lsu_tm_d_vld (lsu_tm_d_vld ), 
.lsu_tm_d_data (lsu_tm_d_data ), 
.lsu_tm_d_ldst (lsu_tm_d_ldst ), 
.lsu_tm_d_size (lsu_tm_d_size ), 
.tm_lsu_trigger_d_vld (tm_lsu_trigger_d_vld ), 
.tm_lsu_trigger_debug_d (tm_lsu_trigger_debug_d ), 
.clic_tm_intctl_vld (clic_tm_intctl_vld ) 
); 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
assign top_tm_vld_vec = 16'b0 ; 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ICACHE 
`endif 
assign err_bus[15:0] = m130_err_bus; 
endmodule
 
 
module wing_m130 ( 
input logic always_on_clk , 
input logic clk , 
output logic clk_gate_en , 
input logic pwrup_rst_n , 
input logic ndm_rst_n , 
input logic soft_rst_n , 
output logic soft_rst_req , 
output logic core_sleeping , 
output logic core_deep_sleeping , 
input logic [32-1:0] boot_pc , 
input logic [32-1:0] hart_id , 
input logic ref_clk , 
input logic [32-1:0] core_itcm_base_addr , 
input logic [32-1:0] core_dtcm_base_addr , 
input logic [32-1:0] core_mmr_base_addr , 
input logic [15:0] reri_bank_inst_id , 
input logic core_wait , 
input logic endianess , 
output logic halted , 
output logic lockup , 
input logic [32+1:0] timer_calibration , 
input logic ext_interrupt , 
input logic [64-1:0] loc_interrupt , 
output logic [31:0] i_haddr , 
output logic [2:0] i_hburst , 
output logic [3:0] i_hprot , 
output logic [2:0] i_hsize , 
output logic [1:0] i_htrans , 
output logic i_hwrite , 
output logic [31:0] i_hwdata , 
input logic [31:0] i_hrdata , 
input logic i_hready , 
input logic i_hresp , 
output logic [1:0] i_hmaster , 
output logic [31:0] d_haddr , 
output logic [2:0] d_hburst , 
output logic [3:0] d_hprot , 
output logic [2:0] d_hsize , 
output logic [1:0] d_htrans , 
output logic d_hwrite , 
output logic [31:0] d_hwdata , 
input logic [31:0] d_hrdata , 
input logic d_hready , 
input logic d_hresp , 
output logic [1:0] d_hmaster , 
output logic [31:0] m_haddr , 
output logic [2:0] m_hburst , 
output logic [3:0] m_hprot , 
output logic [2:0] m_hsize , 
output logic [1:0] m_htrans , 
output logic m_hwrite , 
output logic [31:0] m_hwdata , 
input logic [31:0] m_hrdata , 
input logic m_hready , 
input logic m_hresp , 
output logic [1:0] m_hmaster , 
output logic dm_active_rst_n , 
output logic dm_rst_n , 
output logic tm_rst_n , 
input logic dm_core_reset , 
input logic dm_dmactive , 
input logic dm_pcu_halt_req , 
input logic dm_pcu_halt_on_reset , 
input logic dm_pcu_resume_req , 
output logic pcu_dm_halted , 
output logic pcu_dm_havereset , 
input logic dm_pcu_ack_havereset , 
output logic pcu_dm_unavail , 
output logic pcu_dm_cmd_done , 
output logic pcu_dm_cmd_excp , 
output logic pcu_dm_bus_err , 
input logic dm_pcu_dsch0_write , 
output logic [32-1:0] pcu_dm_dsch0_rdata , 
input logic [32-1:0] dm_pcu_dsch0_wdata , 
input logic dm_ifu_inst_vld , 
output logic ifu_dm_inst_rdy , 
input logic [32-1:0] dm_ifu_inst_data , 
input logic tm_pcu_csr_done , 
input logic [32-1:0] tm_pcu_csr_rdata , 
input logic tm_pcu_csr_excp , 
input logic tm_pcu_csr_match , 
output logic [2-1:0] pcu_tm_pc_ex_vld , 
output logic [2-1:0][32-1:0] pcu_tm_pc_ex , 
output logic [2-1:0] pcu_tm_inst_size_ex , 
output logic pcu_tm_trap_vld , 
output logic [32-1:0] pcu_tm_trap_cause , 
output logic [2-1:0] pcu_tm_inst_retire , 
output logic pcu_tm_tcontrol_mte , 
output logic pcu_tm_trigger_hit , 
output logic [4-1:0] pcu_tm_trigger_hit_idx , 
input logic [2-1:0] tm_pcu_trigger_pc_vld , 
input logic [2-1:0] tm_pcu_trigger_pc_debug , 
input logic [2-1:0][4-1:0] tm_pcu_trigger_hit_pc_idx , 
output logic lsu_tm_a_vld , 
output logic [32-1:0] lsu_tm_a_addr , 
output logic [2:0] lsu_tm_a_ldst , 
output logic [2-1:0] lsu_tm_a_size , 
input logic tm_lsu_trigger_a_vld , 
input logic tm_lsu_trigger_debug_a , 
input logic [4-1:0] tm_lsu_trigger_hit_a_idx , 
output logic lsu_tm_d_vld , 
output logic [32-1:0] lsu_tm_d_data , 
output logic [2:0] lsu_tm_d_ldst , 
output logic [2-1:0] lsu_tm_d_size , 
input logic tm_lsu_trigger_d_vld , 
input logic tm_lsu_trigger_debug_d , 
output logic gated_clk , 
output logic core_rst_n , 
output logic [1:0] core_priv_mode , 
output logic core_dbg_mode , 
output logic core_sleep_mode , 
output logic pcu_csr_vld , 
output logic [1:0] pcu_csr_opcode , 
output logic [11:0] pcu_csr_index , 
output logic [32-1:0] pcu_csr_wdata , 
output logic clic_tm_intctl_vld , 
`ifdef M130_CORE_SUPPORT_ICACHE 
output logic [2-1:0] ic_sram_tag_cs , 
output logic [2-1:0] ic_sram_tag_wr , 
output logic [2-1:0][$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] ic_sram_tag_addr , 
output logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_wdata , 
input logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_rdata , 
output logic [2-1:0] ic_sram_data_cs , 
output logic [2-1:0] ic_sram_data_wr , 
output logic [2-1:0][($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64)))-1:0] ic_sram_data_addr , 
output logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_wdata , 
input logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_rdata , 
`endif 
output logic [15:0] err_bus , 
input logic dft_icg_scan_en , 
input logic dft_scan_mode , 
input logic dft_scan_rst_n 
); 
logic mss_idle ; 
logic bmu_idle ; 
logic [63:0] clic_pcu_mtime ; 
logic clic_cfg_clk ; 
logic clic_cfg_clk_en ; 
logic clic_pcu_int_vld ; 
logic [$clog2(64 + 32)-1:0] clic_pcu_int_id ; 
logic [8-1:0] clic_pcu_int_lvl ; 
logic clic_pcu_int_shv ; 
logic [1:0] clic_pcu_int_priv ; 
logic pcu_clic_rsp_clr_ip ; 
logic [$clog2(64 + 32)-1:0] pcu_clic_rsp_id ; 
logic pcu_clic_rsp_vld ; 
logic pcu_clic_stoptime ; 
logic pmp_cfg_clk ; 
logic pmp_cfg_clk_en ; 
logic core_sleep_wakeup ; 
logic mss_pcu_icache_init_busy ; 
logic ifu_mss_ar_valid ; 
logic mss_ifu_ar_ready ; 
logic [32-1:0] ifu_mss_ar_addr ; 
logic [2:0] ifu_mss_ar_size ; 
logic [7:0] ifu_mss_ar_len ; 
logic [1:0] ifu_mss_ar_burst ; 
logic [2-1:0] ifu_mss_ar_id ; 
logic [3:0] ifu_mss_ar_cache ; 
logic ifu_mss_ar_lock ; 
logic [2:0] ifu_mss_ar_prot ; 
logic [4-1:0] ifu_mss_ar_user ; 
logic ifu_mss_flush ; 
logic mss_ifu_r_valid ; 
logic ifu_mss_r_ready ; 
logic [64-1:0] mss_ifu_r_data ; 
logic [2-1:0] mss_ifu_r_id ; 
logic mss_ifu_r_last ; 
logic [1:0] mss_ifu_r_resp ; 
logic [33-1:0] mss_ifu_r_user ; 
logic lsu_mss_ar_valid ; 
logic mss_lsu_ar_ready ; 
logic [32-1:0] lsu_mss_ar_addr ; 
logic [2:0] lsu_mss_ar_size ; 
logic [7:0] lsu_mss_ar_len ; 
logic [1:0] lsu_mss_ar_burst ; 
logic [2-1:0] lsu_mss_ar_id ; 
logic [3:0] lsu_mss_ar_cache ; 
logic lsu_mss_ar_lock ; 
logic [2:0] lsu_mss_ar_prot ; 
logic [5-1:0] lsu_mss_ar_user ; 
logic lsu_mss_aw_valid ; 
logic mss_lsu_aw_ready ; 
logic [32-1:0] lsu_mss_aw_addr ; 
logic [2:0] lsu_mss_aw_size ; 
logic [7:0] lsu_mss_aw_len ; 
logic [1:0] lsu_mss_aw_burst ; 
logic [2-1:0] lsu_mss_aw_id ; 
logic [3:0] lsu_mss_aw_cache ; 
logic lsu_mss_aw_lock ; 
logic [2:0] lsu_mss_aw_prot ; 
logic [5:0] lsu_mss_aw_atop ; 
logic [5-1:0] lsu_mss_aw_user ; 
logic lsu_mss_w_valid ; 
logic mss_lsu_w_ready ; 
logic [32-1:0] lsu_mss_w_data ; 
logic [32/8-1:0] lsu_mss_w_strb ; 
logic lsu_mss_w_last ; 
logic mss_lsu_r_valid ; 
logic lsu_mss_r_ready ; 
logic [32-1:0] mss_lsu_r_data ; 
logic [2-1:0] mss_lsu_r_id ; 
logic mss_lsu_r_last ; 
logic [1:0] mss_lsu_r_resp ; 
logic [1-1:0] mss_lsu_r_user ; 
logic mss_lsu_b_valid ; 
logic lsu_mss_b_ready ; 
logic [2-1:0] mss_lsu_b_id ; 
logic [1:0] mss_lsu_b_resp ; 
logic lsu_mss_fencei_req ; 
logic mss_lsu_fencei_done ; 
logic lsu_mss_load_flush ; 
logic icache_bmu_ar_valid ; 
logic bmu_icache_ar_ready ; 
logic [32-1:0] icache_bmu_ar_addr ; 
logic [2:0] icache_bmu_ar_size ; 
logic [7:0] icache_bmu_ar_len ; 
logic [1:0] icache_bmu_ar_burst ; 
logic [1-1:0] icache_bmu_ar_id ; 
logic [3:0] icache_bmu_ar_cache ; 
logic icache_bmu_ar_lock ; 
logic [2:0] icache_bmu_ar_prot ; 
logic icache_bmu_ar_user ; 
logic bmu_icache_r_valid ; 
logic icache_bmu_r_ready ; 
logic [32-1:0] bmu_icache_r_data ; 
logic [1-1:0] bmu_icache_r_id ; 
logic bmu_icache_r_last ; 
logic [1:0] bmu_icache_r_resp ; 
logic dcache_bmu_req_vld ; 
logic bmu_dcache_req_rdy ; 
logic [32-1:0] dcache_bmu_req_addr ; 
logic [1:0] dcache_bmu_req_size ; 
logic dcache_bmu_req_write ; 
logic [32-1:0] dcache_bmu_req_wdata ; 
logic [32/8-1:0] dcache_bmu_req_wstrb ; 
logic [1-1:0] dcache_bmu_req_id ; 
logic [1:0] dcache_bmu_req_memattr ; 
logic [1:0] dcache_bmu_req_prot ; 
logic dcache_bmu_req_dm ; 
logic dcache_bmu_req_bitband ; 
logic bmu_dcache_rsp_vld ; 
logic dcache_bmu_rsp_rdy ; 
logic [32-1:0] bmu_dcache_rsp_data ; 
logic [1-1:0] bmu_dcache_rsp_id ; 
logic bmu_dcache_rsp_err ; 
logic bmu_tcm_req_vld ; 
logic tcm_bmu_req_rdy ; 
logic [32-1:0] bmu_tcm_req_addr ; 
logic bmu_tcm_req_write ; 
logic [32-1:0] bmu_tcm_req_wdata ; 
logic [32/8-1:0] bmu_tcm_req_wstrb ; 
logic [1:0] bmu_tcm_req_dest ; 
logic tcm_bmu_rsp_vld ; 
logic bmu_tcm_rsp_rdy ; 
logic [32-1:0] tcm_bmu_rsp_rdata ; 
logic tcm_bmu_rsp_err ; 
logic bmu_clic_req_vld ; 
logic clic_bmu_req_rdy ; 
logic [31:0] bmu_clic_req_addr ; 
logic [ 1:0] bmu_clic_req_size ; 
logic bmu_clic_req_write ; 
logic [32/8-1:0] bmu_clic_req_wstrb ; 
logic [32-1:0] bmu_clic_req_wdata ; 
logic [1:0] bmu_clic_req_dest ; 
logic clic_bmu_rsp_vld ; 
logic bmu_clic_rsp_rdy ; 
logic [32-1:0] clic_bmu_rsp_rdata ; 
logic clic_bmu_rsp_err ; 
logic bmu_mss_req_vld ; 
logic mss_bmu_req_rdy ; 
logic [31:0] bmu_mss_req_addr ; 
logic bmu_mss_req_write ; 
logic [32-1:0] bmu_mss_req_wdata ; 
logic mss_bmu_rsp_vld ; 
logic bmu_mss_rsp_rdy ; 
logic [32-1:0] mss_bmu_rsp_rdata ; 
logic mss_bmu_rsp_err ; 
logic bmu_reri_req_vld ; 
logic reri_bmu_req_rdy ; 
logic [31:0] bmu_reri_req_addr ; 
logic bmu_reri_req_write ; 
logic [32-1:0] bmu_reri_req_wdata ; 
logic reri_bmu_rsp_vld ; 
logic bmu_reri_rsp_rdy ; 
logic [32-1:0] reri_bmu_rsp_rdata ; 
logic reri_bmu_rsp_err ; 
logic err_valid ; 
logic err_ce ; 
logic err_uue ; 
logic [1:0] err_priority ; 
logic [2:0] err_tt ; 
logic err_scrub ; 
logic [7:0] err_ec ; 
logic [31:0] err_addr ; 
logic [3:0] err_aec ; 
logic err_tag ; 
logic ras_int ; 
`ifdef M130_CORE_SUPPORT_ICACHE 
logic ic_tagm_ecc_err ; 
logic ic_datm_ecc_err ; 
`endif 
logic clic_rst_n ; 
logic core_deep_sleep ; 
m130_crg u_m130_crg( 
.always_on_clk (always_on_clk ), 
.clk (clk ), 
.clic_cfg_clk (clic_cfg_clk ), 
.clic_cfg_clk_en (clic_cfg_clk_en ), 
.pmp_cfg_clk (pmp_cfg_clk ), 
.pmp_cfg_clk_en (pmp_cfg_clk_en ), 
.core_sleep_mode (core_sleep_mode ), 
.core_deep_sleep (core_deep_sleep ), 
.core_sleep_wakeup (core_sleep_wakeup ), 
.gated_clk (gated_clk ), 
.pwrup_rst_n (pwrup_rst_n ), 
.dm_core_reset (dm_core_reset ), 
.dm_dmactive (dm_dmactive ), 
.ndm_rst_n (ndm_rst_n ), 
.core_rst_n (core_rst_n ), 
.dm_rst_n (dm_rst_n ), 
.clic_rst_n (clic_rst_n ), 
.tm_rst_n (tm_rst_n ), 
.dm_active_rst_n (dm_active_rst_n ), 
.soft_rst_n (soft_rst_n ), 
.core_sleeping (core_sleeping ), 
.core_deep_sleeping(core_deep_sleeping), 
.clk_gate_en (clk_gate_en ), 
.dft_icg_scan_en (dft_icg_scan_en ), 
.dft_scan_mode (dft_scan_mode ), 
.dft_scan_rst_n (dft_scan_rst_n ) 
); 
m130_core_top u_m130_core_top ( 
.always_on_clk (always_on_clk ), 
.clk (gated_clk ), 
.rst_n (core_rst_n ), 
.top_ifu_boot_pc (boot_pc ), 
.itcm_base_addr (core_itcm_base_addr ), 
.dtcm_base_addr (core_dtcm_base_addr ), 
.core_wait (core_wait ), 
.mss_idle (mss_idle ), 
.bmu_idle (bmu_idle ), 
.lockup (lockup ), 
.err_bus (err_bus ), 
.top_hart_id (hart_id ), 
.mem_map_mtime (clic_pcu_mtime ), 
.pmp_cfg_clk (pmp_cfg_clk ), 
.pmp_cfg_clk_en (pmp_cfg_clk_en ), 
.clic_pcu_int_vld (clic_pcu_int_vld ), 
.clic_pcu_int_id (clic_pcu_int_id ), 
.clic_pcu_int_lvl (clic_pcu_int_lvl ), 
.clic_pcu_int_shv (clic_pcu_int_shv ), 
.clic_pcu_int_priv (clic_pcu_int_priv ), 
.pcu_clic_rsp_clr_ip (pcu_clic_rsp_clr_ip ), 
.pcu_clic_rsp_id (pcu_clic_rsp_id ), 
.pcu_clic_rsp_vld (pcu_clic_rsp_vld ), 
.pcu_clic_stoptime (pcu_clic_stoptime ), 
.dm_pcu_halt_req (dm_pcu_halt_req ), 
.dm_pcu_halt_on_reset (dm_pcu_halt_on_reset ), 
.dm_pcu_resume_req (dm_pcu_resume_req ), 
.pcu_dm_halted (pcu_dm_halted ), 
.pcu_dm_havereset (pcu_dm_havereset ), 
.pcu_dm_unavail (pcu_dm_unavail ), 
.dm_pcu_ack_havereset (dm_pcu_ack_havereset ), 
.pcu_dm_cmd_done (pcu_dm_cmd_done ), 
.pcu_dm_bus_err (pcu_dm_bus_err ), 
.pcu_dm_cmd_excp (pcu_dm_cmd_excp ), 
.dm_pcu_dsch0_write (dm_pcu_dsch0_write ), 
.dm_pcu_dsch0_wdata (dm_pcu_dsch0_wdata ), 
.pcu_dm_dsch0_rdata (pcu_dm_dsch0_rdata ), 
.dm_ifu_inst_vld (dm_ifu_inst_vld ), 
.ifu_dm_inst_rdy (ifu_dm_inst_rdy ), 
.dm_ifu_inst_data (dm_ifu_inst_data ), 
.pcu_tm_pc_ex_vld (pcu_tm_pc_ex_vld ), 
.pcu_tm_pc_ex (pcu_tm_pc_ex ), 
.pcu_tm_inst_size_ex (pcu_tm_inst_size_ex ), 
.tm_pcu_trigger_pc_vld (tm_pcu_trigger_pc_vld ), 
.tm_pcu_trigger_pc_debug (tm_pcu_trigger_pc_debug ), 
.tm_pcu_trigger_hit_pc_idx (tm_pcu_trigger_hit_pc_idx ), 
.pcu_tm_trap_vld (pcu_tm_trap_vld ), 
.pcu_tm_trap_cause (pcu_tm_trap_cause ), 
.pcu_tm_inst_retire (pcu_tm_inst_retire ), 
.pcu_tm_tcontrol_mte (pcu_tm_tcontrol_mte ), 
.pcu_tm_trigger_hit (pcu_tm_trigger_hit ), 
.pcu_tm_trigger_hit_idx (pcu_tm_trigger_hit_idx ), 
.lsu_tm_a_vld (lsu_tm_a_vld ), 
.lsu_tm_a_addr (lsu_tm_a_addr ), 
.lsu_tm_a_ldst (lsu_tm_a_ldst ), 
.lsu_tm_a_size (lsu_tm_a_size ), 
.tm_lsu_trigger_a_vld (tm_lsu_trigger_a_vld ), 
.tm_lsu_trigger_debug_a (tm_lsu_trigger_debug_a ), 
.tm_lsu_trigger_hit_a_idx (tm_lsu_trigger_hit_a_idx ), 
.lsu_tm_d_vld (lsu_tm_d_vld ), 
.lsu_tm_d_data (lsu_tm_d_data ), 
.lsu_tm_d_ldst (lsu_tm_d_ldst ), 
.lsu_tm_d_size (lsu_tm_d_size ), 
.tm_lsu_trigger_d_vld (tm_lsu_trigger_d_vld ), 
.tm_lsu_trigger_debug_d (tm_lsu_trigger_debug_d ), 
.pcu_csr_vld (pcu_csr_vld ), 
.pcu_csr_opcode (pcu_csr_opcode ), 
.pcu_csr_index (pcu_csr_index ), 
.pcu_csr_wdata (pcu_csr_wdata ), 
.pcu_csr_mprv ( ), 
.pcu_csr_mpp ( ), 
.tm_pcu_csr_done (tm_pcu_csr_done ), 
.tm_pcu_csr_rdata (tm_pcu_csr_rdata ), 
.tm_pcu_csr_excp (tm_pcu_csr_excp ), 
.tm_pcu_csr_match (tm_pcu_csr_match ), 
.core_priv_mode (core_priv_mode ), 
.core_dbg_mode (core_dbg_mode ), 
.core_sleep_mode (core_sleep_mode ), 
.core_deep_sleep (core_deep_sleep ), 
.core_sleep_wakeup (core_sleep_wakeup ), 
`ifdef M130_CORE_SUPPORT_ICACHE 
.ic_tagm_ecc_err (ic_tagm_ecc_err ), 
.ic_datm_ecc_err (ic_datm_ecc_err ), 
`endif 
.mss_pcu_icache_init_busy (mss_pcu_icache_init_busy ), 
.ifu_mss_ar_valid (ifu_mss_ar_valid ), 
.mss_ifu_ar_ready (mss_ifu_ar_ready ), 
.ifu_mss_ar_addr (ifu_mss_ar_addr ), 
.ifu_mss_ar_size (ifu_mss_ar_size ), 
.ifu_mss_ar_len (ifu_mss_ar_len ), 
.ifu_mss_ar_burst (ifu_mss_ar_burst ), 
.ifu_mss_ar_id (ifu_mss_ar_id ), 
.ifu_mss_ar_cache (ifu_mss_ar_cache ), 
.ifu_mss_ar_lock (ifu_mss_ar_lock ), 
.ifu_mss_ar_prot (ifu_mss_ar_prot ), 
.ifu_mss_ar_user (ifu_mss_ar_user ), 
.ifu_mss_flush (ifu_mss_flush ), 
.mss_ifu_r_valid (mss_ifu_r_valid ), 
.ifu_mss_r_ready (ifu_mss_r_ready ), 
.mss_ifu_r_data (mss_ifu_r_data ), 
.mss_ifu_r_id (mss_ifu_r_id ), 
.mss_ifu_r_last (mss_ifu_r_last ), 
.mss_ifu_r_resp (mss_ifu_r_resp ), 
.mss_ifu_r_user (mss_ifu_r_user ), 
.lsu_mss_ar_valid (lsu_mss_ar_valid ), 
.mss_lsu_ar_ready (mss_lsu_ar_ready ), 
.lsu_mss_ar_addr (lsu_mss_ar_addr ), 
.lsu_mss_ar_size (lsu_mss_ar_size ), 
.lsu_mss_ar_len (lsu_mss_ar_len ), 
.lsu_mss_ar_burst (lsu_mss_ar_burst ), 
.lsu_mss_ar_id (lsu_mss_ar_id ), 
.lsu_mss_ar_cache (lsu_mss_ar_cache ), 
.lsu_mss_ar_lock (lsu_mss_ar_lock ), 
.lsu_mss_ar_prot (lsu_mss_ar_prot ), 
.lsu_mss_ar_user (lsu_mss_ar_user ), 
.lsu_mss_aw_valid (lsu_mss_aw_valid ), 
.mss_lsu_aw_ready (mss_lsu_aw_ready ), 
.lsu_mss_aw_addr (lsu_mss_aw_addr ), 
.lsu_mss_aw_size (lsu_mss_aw_size ), 
.lsu_mss_aw_len (lsu_mss_aw_len ), 
.lsu_mss_aw_burst (lsu_mss_aw_burst ), 
.lsu_mss_aw_id (lsu_mss_aw_id ), 
.lsu_mss_aw_cache (lsu_mss_aw_cache ), 
.lsu_mss_aw_lock (lsu_mss_aw_lock ), 
.lsu_mss_aw_prot (lsu_mss_aw_prot ), 
.lsu_mss_aw_atop (lsu_mss_aw_atop ), 
.lsu_mss_aw_user (lsu_mss_aw_user ), 
.lsu_mss_w_valid (lsu_mss_w_valid ), 
.mss_lsu_w_ready (mss_lsu_w_ready ), 
.lsu_mss_w_data (lsu_mss_w_data ), 
.lsu_mss_w_strb (lsu_mss_w_strb ), 
.lsu_mss_w_last (lsu_mss_w_last ), 
.mss_lsu_r_valid (mss_lsu_r_valid ), 
.lsu_mss_r_ready (lsu_mss_r_ready ), 
.mss_lsu_r_data (mss_lsu_r_data ), 
.mss_lsu_r_id (mss_lsu_r_id ), 
.mss_lsu_r_last (mss_lsu_r_last ), 
.mss_lsu_r_resp (mss_lsu_r_resp ), 
.mss_lsu_r_user (mss_lsu_r_user ), 
.mss_lsu_b_valid (mss_lsu_b_valid ), 
.lsu_mss_b_ready (lsu_mss_b_ready ), 
.mss_lsu_b_id (mss_lsu_b_id ), 
.mss_lsu_b_resp (mss_lsu_b_resp ), 
.lsu_mss_fencei_req (lsu_mss_fencei_req ), 
.mss_lsu_fencei_done (mss_lsu_fencei_done ), 
.endianess (endianess ), 
.core_mmr_base_addr (core_mmr_base_addr ), 
.lsu_mss_load_flush (lsu_mss_load_flush ) 
); 
m130_mss_top u_m130_mss_top( 
.mss_pcu_icache_init_busy (mss_pcu_icache_init_busy ), 
.mss_idle (mss_idle ), 
.pcu_dm_halted (pcu_dm_halted ), 
.endianess (endianess ), 
.mmr_req_vld (bmu_mss_req_vld ), 
.mmr_req_rdy (mss_bmu_req_rdy ), 
.mmr_req_addr (bmu_mss_req_addr ), 
.mmr_req_write (bmu_mss_req_write ), 
.mmr_req_wdata (bmu_mss_req_wdata ), 
.mmr_rsp_vld (mss_bmu_rsp_vld ), 
.mmr_rsp_rdy (bmu_mss_rsp_rdy ), 
.mmr_rsp_rdata (mss_bmu_rsp_rdata ), 
.mmr_rsp_err (mss_bmu_rsp_err ), 
.ifu_mss_ar_valid (ifu_mss_ar_valid ), 
.mss_ifu_ar_ready (mss_ifu_ar_ready ), 
.ifu_mss_ar_addr (ifu_mss_ar_addr ), 
.ifu_mss_ar_size (ifu_mss_ar_size ), 
.ifu_mss_ar_len (ifu_mss_ar_len ), 
.ifu_mss_ar_burst (ifu_mss_ar_burst ), 
.ifu_mss_ar_id (ifu_mss_ar_id ), 
.ifu_mss_ar_cache (ifu_mss_ar_cache ), 
.ifu_mss_ar_lock (ifu_mss_ar_lock ), 
.ifu_mss_ar_prot (ifu_mss_ar_prot ), 
.ifu_mss_ar_user (ifu_mss_ar_user ), 
.mss_ifu_r_valid (mss_ifu_r_valid ), 
.ifu_mss_r_ready (ifu_mss_r_ready ), 
.mss_ifu_r_data (mss_ifu_r_data ), 
.mss_ifu_r_id (mss_ifu_r_id ), 
.mss_ifu_r_last (mss_ifu_r_last ), 
.mss_ifu_r_resp (mss_ifu_r_resp ), 
.mss_ifu_r_user (mss_ifu_r_user ), 
.ifu_mss_flush (ifu_mss_flush ), 
.lsu_mss_ar_valid (lsu_mss_ar_valid ), 
.mss_lsu_ar_ready (mss_lsu_ar_ready ), 
.lsu_mss_ar_addr (lsu_mss_ar_addr ), 
.lsu_mss_ar_size (lsu_mss_ar_size ), 
.lsu_mss_ar_len (lsu_mss_ar_len ), 
.lsu_mss_ar_burst (lsu_mss_ar_burst ), 
.lsu_mss_ar_id (lsu_mss_ar_id ), 
.lsu_mss_ar_cache (lsu_mss_ar_cache ), 
.lsu_mss_ar_lock (lsu_mss_ar_lock ), 
.lsu_mss_ar_prot (lsu_mss_ar_prot ), 
.lsu_mss_ar_user (lsu_mss_ar_user ), 
.lsu_mss_aw_valid (lsu_mss_aw_valid ), 
.mss_lsu_aw_ready (mss_lsu_aw_ready ), 
.lsu_mss_aw_addr (lsu_mss_aw_addr ), 
.lsu_mss_aw_size (lsu_mss_aw_size ), 
.lsu_mss_aw_len (lsu_mss_aw_len ), 
.lsu_mss_aw_burst (lsu_mss_aw_burst ), 
.lsu_mss_aw_id (lsu_mss_aw_id ), 
.lsu_mss_aw_cache (lsu_mss_aw_cache ), 
.lsu_mss_aw_lock (lsu_mss_aw_lock ), 
.lsu_mss_aw_prot (lsu_mss_aw_prot ), 
.lsu_mss_aw_user (lsu_mss_aw_user ), 
.lsu_mss_aw_atop (lsu_mss_aw_atop ), 
.lsu_mss_w_valid (lsu_mss_w_valid ), 
.mss_lsu_w_ready (mss_lsu_w_ready ), 
.lsu_mss_w_data (lsu_mss_w_data ), 
.lsu_mss_w_strb (lsu_mss_w_strb ), 
.lsu_mss_w_last (lsu_mss_w_last ), 
.mss_lsu_r_valid (mss_lsu_r_valid ), 
.lsu_mss_r_ready (lsu_mss_r_ready ), 
.mss_lsu_r_data (mss_lsu_r_data ), 
.mss_lsu_r_id (mss_lsu_r_id ), 
.mss_lsu_r_last (mss_lsu_r_last ), 
.mss_lsu_r_resp (mss_lsu_r_resp ), 
.mss_lsu_r_user (mss_lsu_r_user ), 
.mss_lsu_b_valid (mss_lsu_b_valid ), 
.lsu_mss_b_ready (lsu_mss_b_ready ), 
.mss_lsu_b_id (mss_lsu_b_id ), 
.mss_lsu_b_resp (mss_lsu_b_resp ), 
.lsu_mss_fencei_req (lsu_mss_fencei_req ), 
.mss_lsu_fencei_done (mss_lsu_fencei_done ), 
.lsu_mss_load_flush (lsu_mss_load_flush ), 
.icache_bmu_ar_valid (icache_bmu_ar_valid ), 
.bmu_icache_ar_ready (bmu_icache_ar_ready ), 
.icache_bmu_ar_addr (icache_bmu_ar_addr ), 
.icache_bmu_ar_size (icache_bmu_ar_size ), 
.icache_bmu_ar_len (icache_bmu_ar_len ), 
.icache_bmu_ar_burst (icache_bmu_ar_burst ), 
.icache_bmu_ar_id (icache_bmu_ar_id ), 
.icache_bmu_ar_cache (icache_bmu_ar_cache ), 
.icache_bmu_ar_lock (icache_bmu_ar_lock ), 
.icache_bmu_ar_prot (icache_bmu_ar_prot ), 
.icache_bmu_ar_user (icache_bmu_ar_user ), 
.bmu_icache_r_valid (bmu_icache_r_valid ), 
.icache_bmu_r_ready (icache_bmu_r_ready ), 
.bmu_icache_r_data (bmu_icache_r_data ), 
.bmu_icache_r_id (bmu_icache_r_id ), 
.bmu_icache_r_last (bmu_icache_r_last ), 
.bmu_icache_r_resp (bmu_icache_r_resp ), 
.dcache_bmu_req_vld (dcache_bmu_req_vld ), 
.bmu_dcache_req_rdy (bmu_dcache_req_rdy ), 
.dcache_bmu_req_addr (dcache_bmu_req_addr ), 
.dcache_bmu_req_size (dcache_bmu_req_size ), 
.dcache_bmu_req_write (dcache_bmu_req_write ), 
.dcache_bmu_req_wdata (dcache_bmu_req_wdata ), 
.dcache_bmu_req_wstrb (dcache_bmu_req_wstrb ), 
.dcache_bmu_req_id (dcache_bmu_req_id ), 
.dcache_bmu_req_memattr (dcache_bmu_req_memattr ), 
.dcache_bmu_req_prot (dcache_bmu_req_prot ), 
.dcache_bmu_req_dm (dcache_bmu_req_dm ), 
.dcache_bmu_req_bitband (dcache_bmu_req_bitband ), 
.bmu_dcache_rsp_vld (bmu_dcache_rsp_vld ), 
.dcache_bmu_rsp_rdy (dcache_bmu_rsp_rdy ), 
.bmu_dcache_rsp_data (bmu_dcache_rsp_data ), 
.bmu_dcache_rsp_id (bmu_dcache_rsp_id ), 
.bmu_dcache_rsp_err (bmu_dcache_rsp_err ), 
.bmu_tcm_req_vld (bmu_tcm_req_vld ), 
.tcm_bmu_req_rdy (tcm_bmu_req_rdy ), 
.bmu_tcm_req_addr (bmu_tcm_req_addr ), 
.bmu_tcm_req_write (bmu_tcm_req_write ), 
.bmu_tcm_req_wdata (bmu_tcm_req_wdata ), 
.bmu_tcm_req_wstrb (bmu_tcm_req_wstrb ), 
.bmu_tcm_req_dest (bmu_tcm_req_dest ), 
.tcm_bmu_rsp_vld (tcm_bmu_rsp_vld ), 
.bmu_tcm_rsp_rdy (bmu_tcm_rsp_rdy ), 
.tcm_bmu_rsp_rdata (tcm_bmu_rsp_rdata ), 
.tcm_bmu_rsp_err (tcm_bmu_rsp_err ), 
`ifdef M130_CORE_SUPPORT_ICACHE 
.ic_tagm_ecc_err (ic_tagm_ecc_err ), 
.ic_datm_ecc_err (ic_datm_ecc_err ), 
.ic_sram_tag_cs (ic_sram_tag_cs ), 
.ic_sram_tag_wr (ic_sram_tag_wr ), 
.ic_sram_tag_addr (ic_sram_tag_addr ), 
.ic_sram_tag_wdata (ic_sram_tag_wdata ), 
.ic_sram_tag_rdata (ic_sram_tag_rdata ), 
.ic_sram_data_cs (ic_sram_data_cs ), 
.ic_sram_data_wr (ic_sram_data_wr ), 
.ic_sram_data_addr (ic_sram_data_addr ), 
.ic_sram_data_wdata (ic_sram_data_wdata ), 
.ic_sram_data_rdata (ic_sram_data_rdata ), 
`endif 
.err_valid ( err_valid ), 
.err_ce ( err_ce ), 
.err_uue ( err_uue ), 
.err_priority ( err_priority ), 
.err_tt ( err_tt ), 
.err_scrub ( err_scrub ), 
.err_ec ( err_ec ), 
.err_addr ( err_addr ), 
.err_aec ( err_aec ), 
.err_tag ( err_tag ), 
.clk (gated_clk ), 
.rst_n (core_rst_n ) 
); 
wing_clic u_m130_clic( 
.clk (always_on_clk ), 
.cfg_clk (clic_cfg_clk ), 
.rst_n (clic_rst_n ), 
.cfg_clk_en (clic_cfg_clk_en ), 
.srcr_rst_n (core_rst_n ), 
.soft_rst_req (soft_rst_req ), 
.refclk (ref_clk ), 
.soc_clic_ext_int (ext_interrupt ), 
.mtime (clic_pcu_mtime ), 
.timer_calibration (timer_calibration ), 
.ras_int (ras_int ), 
.bmu_clic_req_vld (bmu_clic_req_vld ), 
.clic_bmu_req_rdy (clic_bmu_req_rdy ), 
.bmu_clic_req_addr (bmu_clic_req_addr ), 
.bmu_clic_req_size (bmu_clic_req_size ), 
.bmu_clic_req_write (bmu_clic_req_write ), 
.bmu_clic_req_wstrb (bmu_clic_req_wstrb ), 
.bmu_clic_req_wdata (bmu_clic_req_wdata ), 
.bmu_clic_req_dest (bmu_clic_req_dest ), 
.clic_bmu_rsp_vld (clic_bmu_rsp_vld ), 
.bmu_clic_rsp_rdy (bmu_clic_rsp_rdy ), 
.clic_bmu_rsp_rdata (clic_bmu_rsp_rdata ), 
.clic_bmu_rsp_err (clic_bmu_rsp_err ), 
.pcu_clic_stoptime (pcu_clic_stoptime ), 
.soc_clic_loc_int (loc_interrupt ), 
.clic_pcu_int_vld (clic_pcu_int_vld ), 
.clic_pcu_int_id (clic_pcu_int_id ), 
.clic_pcu_int_lvl (clic_pcu_int_lvl ), 
.clic_pcu_int_shv (clic_pcu_int_shv ), 
.clic_pcu_int_priv (clic_pcu_int_priv ), 
.pcu_clic_rsp_vld (pcu_clic_rsp_vld ), 
.pcu_clic_rsp_id (pcu_clic_rsp_id ), 
.pcu_clic_rsp_clr_ip (pcu_clic_rsp_clr_ip ), 
.clic_tm_intctl_vld (clic_tm_intctl_vld ) 
); 
m130_bmu_top u_m130_bmu_top ( 
.endianess (endianess ), 
.core_itcm_base_addr (core_itcm_base_addr ), 
.core_dtcm_base_addr (core_dtcm_base_addr ), 
.core_mmr_base_addr (core_mmr_base_addr ), 
.icache_bmu_ar_valid (icache_bmu_ar_valid ), 
.bmu_icache_ar_ready (bmu_icache_ar_ready ), 
.icache_bmu_ar_addr (icache_bmu_ar_addr ), 
.icache_bmu_ar_size (icache_bmu_ar_size ), 
.icache_bmu_ar_len (icache_bmu_ar_len ), 
.icache_bmu_ar_burst (icache_bmu_ar_burst ), 
.icache_bmu_ar_id (icache_bmu_ar_id ), 
.icache_bmu_ar_cache (icache_bmu_ar_cache ), 
.icache_bmu_ar_lock (icache_bmu_ar_lock ), 
.icache_bmu_ar_prot (icache_bmu_ar_prot ), 
.icache_bmu_ar_user (icache_bmu_ar_user ), 
.bmu_icache_r_valid (bmu_icache_r_valid ), 
.icache_bmu_r_ready (icache_bmu_r_ready ), 
.bmu_icache_r_data (bmu_icache_r_data ), 
.bmu_icache_r_id (bmu_icache_r_id ), 
.bmu_icache_r_last (bmu_icache_r_last ), 
.bmu_icache_r_resp (bmu_icache_r_resp ), 
.dcache_bmu_req_vld (dcache_bmu_req_vld ), 
.bmu_dcache_req_rdy (bmu_dcache_req_rdy ), 
.dcache_bmu_req_addr (dcache_bmu_req_addr ), 
.dcache_bmu_req_size (dcache_bmu_req_size ), 
.dcache_bmu_req_write (dcache_bmu_req_write ), 
.dcache_bmu_req_wdata (dcache_bmu_req_wdata ), 
.dcache_bmu_req_wstrb (dcache_bmu_req_wstrb ), 
.dcache_bmu_req_id (dcache_bmu_req_id ), 
.dcache_bmu_req_memattr (dcache_bmu_req_memattr ), 
.dcache_bmu_req_prot (dcache_bmu_req_prot ), 
.dcache_bmu_req_dm (dcache_bmu_req_dm ), 
.dcache_bmu_req_bitband (dcache_bmu_req_bitband ), 
.bmu_dcache_rsp_vld (bmu_dcache_rsp_vld ), 
.dcache_bmu_rsp_rdy (dcache_bmu_rsp_rdy ), 
.bmu_dcache_rsp_data (bmu_dcache_rsp_data ), 
.bmu_dcache_rsp_id (bmu_dcache_rsp_id ), 
.bmu_dcache_rsp_err (bmu_dcache_rsp_err ), 
.bmu_clic_req_vld (bmu_clic_req_vld ), 
.clic_bmu_req_rdy (clic_bmu_req_rdy ), 
.bmu_clic_req_addr (bmu_clic_req_addr ), 
.bmu_clic_req_size (bmu_clic_req_size ), 
.bmu_clic_req_write (bmu_clic_req_write ), 
.bmu_clic_req_wstrb (bmu_clic_req_wstrb ), 
.bmu_clic_req_wdata (bmu_clic_req_wdata ), 
.bmu_clic_req_dest (bmu_clic_req_dest ), 
.clic_bmu_rsp_vld (clic_bmu_rsp_vld ), 
.bmu_clic_rsp_rdy (bmu_clic_rsp_rdy ), 
.clic_bmu_rsp_rdata (clic_bmu_rsp_rdata ), 
.clic_bmu_rsp_err (clic_bmu_rsp_err ), 
.bmu_reri_req_vld (bmu_reri_req_vld ), 
.reri_bmu_req_rdy (reri_bmu_req_rdy ), 
.bmu_reri_req_addr (bmu_reri_req_addr ), 
.bmu_reri_req_write (bmu_reri_req_write ), 
.bmu_reri_req_wdata (bmu_reri_req_wdata ), 
.reri_bmu_rsp_vld (reri_bmu_rsp_vld ), 
.bmu_reri_rsp_rdy (bmu_reri_rsp_rdy ), 
.reri_bmu_rsp_rdata (reri_bmu_rsp_rdata ), 
.reri_bmu_rsp_err (reri_bmu_rsp_err ), 
.bmu_mss_req_vld (bmu_mss_req_vld ), 
.mss_bmu_req_rdy (mss_bmu_req_rdy ), 
.bmu_mss_req_addr (bmu_mss_req_addr ), 
.bmu_mss_req_write (bmu_mss_req_write ), 
.bmu_mss_req_wdata (bmu_mss_req_wdata ), 
.mss_bmu_rsp_vld (mss_bmu_rsp_vld ), 
.bmu_mss_rsp_rdy (bmu_mss_rsp_rdy ), 
.mss_bmu_rsp_rdata (mss_bmu_rsp_rdata ), 
.mss_bmu_rsp_err (mss_bmu_rsp_err ), 
.bmu_tcm_req_vld (bmu_tcm_req_vld ), 
.tcm_bmu_req_rdy (tcm_bmu_req_rdy ), 
.bmu_tcm_req_addr (bmu_tcm_req_addr ), 
.bmu_tcm_req_write (bmu_tcm_req_write ), 
.bmu_tcm_req_wstrb (bmu_tcm_req_wstrb ), 
.bmu_tcm_req_wdata (bmu_tcm_req_wdata ), 
.bmu_tcm_req_dest (bmu_tcm_req_dest ), 
.tcm_bmu_rsp_vld (tcm_bmu_rsp_vld ), 
.bmu_tcm_rsp_rdy (bmu_tcm_rsp_rdy ), 
.tcm_bmu_rsp_rdata (tcm_bmu_rsp_rdata ), 
.tcm_bmu_rsp_err (tcm_bmu_rsp_err ), 
.d_bus_ahb_haddr (d_haddr ), 
.d_bus_ahb_hburst (d_hburst ), 
.d_bus_ahb_hmastlock ( ), 
.d_bus_ahb_hprot (d_hprot ), 
.d_bus_ahb_hsize (d_hsize ), 
.d_bus_ahb_hmaster (d_hmaster ), 
.d_bus_ahb_htrans (d_htrans ), 
.d_bus_ahb_hwrite (d_hwrite ), 
.d_bus_ahb_hwdata (d_hwdata ), 
.d_bus_ahb_hrdata (d_hrdata ), 
.d_bus_ahb_hready (d_hready ), 
.d_bus_ahb_hresp (d_hresp ), 
.i_bus_ahb_haddr (i_haddr ), 
.i_bus_ahb_hburst (i_hburst ), 
.i_bus_ahb_hmastlock ( ), 
.i_bus_ahb_hprot (i_hprot ), 
.i_bus_ahb_hsize (i_hsize ), 
.i_bus_ahb_hmaster (i_hmaster ), 
.i_bus_ahb_htrans (i_htrans ), 
.i_bus_ahb_hwrite (i_hwrite ), 
.i_bus_ahb_hwdata (i_hwdata ), 
.i_bus_ahb_hrdata (i_hrdata ), 
.i_bus_ahb_hready (i_hready ), 
.i_bus_ahb_hresp (i_hresp ), 
.m_bus_ahb_haddr (m_haddr ), 
.m_bus_ahb_hburst (m_hburst ), 
.m_bus_ahb_hmastlock ( ), 
.m_bus_ahb_hprot (m_hprot ), 
.m_bus_ahb_hsize (m_hsize ), 
.m_bus_ahb_hmaster (m_hmaster ), 
.m_bus_ahb_htrans (m_htrans ), 
.m_bus_ahb_hwrite (m_hwrite ), 
.m_bus_ahb_hwdata (m_hwdata ), 
.m_bus_ahb_hrdata (m_hrdata ), 
.m_bus_ahb_hready (m_hready ), 
.m_bus_ahb_hresp (m_hresp ), 
.sleep_mode (core_sleep_mode ), 
.always_on_clk (always_on_clk ), 
.bmu_idle (bmu_idle ), 
.clk (gated_clk ), 
.rst_n (core_rst_n ) 
); 
m130_reri u_m130_reri ( 
.clk (gated_clk ), 
.rst_n (core_rst_n ), 
.inst_id (reri_bank_inst_id ), 
.err_valid (err_valid ), 
.err_ce (err_ce ), 
.err_ude (1'b0 ), 
.err_uue (err_uue ), 
.err_priority (err_priority ), 
.err_tt (err_tt ), 
.err_scrub (err_scrub ), 
.err_ec (err_ec ), 
.err_addr (err_addr ), 
.err_aec (err_aec ), 
.err_tag (err_tag ), 
.ras_int (ras_int ), 
.mmr_req_vld (bmu_reri_req_vld ), 
.mmr_req_rdy (reri_bmu_req_rdy ), 
.mmr_req_addr (bmu_reri_req_addr ), 
.mmr_req_write (bmu_reri_req_write ), 
.mmr_req_wdata (bmu_reri_req_wdata ), 
.mmr_rsp_vld (reri_bmu_rsp_vld ), 
.mmr_rsp_rdy (bmu_reri_rsp_rdy ), 
.mmr_rsp_rdata (reri_bmu_rsp_rdata ), 
.mmr_rsp_err (reri_bmu_rsp_err ) 
); 
assign halted = pcu_dm_halted ; 
endmodule
 
 
module m130_core_top ( 
input logic always_on_clk , 
input logic clk , 
input logic rst_n , 
input logic [32-1:0] top_ifu_boot_pc , 
input logic [32-1:0] itcm_base_addr , 
input logic [32-1:0] dtcm_base_addr , 
input logic core_wait , 
input logic mss_idle , 
input logic bmu_idle , 
output logic lockup , 
output logic [15:0] err_bus , 
input logic [32-1:0] top_hart_id , 
input logic [63:0] mem_map_mtime , 
input logic pmp_cfg_clk , 
output logic pmp_cfg_clk_en , 
input logic clic_pcu_int_vld , 
input logic [$clog2(64 + 32)-1:0] clic_pcu_int_id , 
input logic [8-1:0] clic_pcu_int_lvl , 
input logic clic_pcu_int_shv , 
input logic [1:0] clic_pcu_int_priv , 
output logic pcu_clic_rsp_clr_ip , 
output logic [$clog2(64 + 32)-1:0] pcu_clic_rsp_id , 
output logic pcu_clic_rsp_vld , 
output logic pcu_clic_stoptime , 
input logic dm_pcu_halt_req , 
input logic dm_pcu_halt_on_reset , 
input logic dm_pcu_resume_req , 
output logic pcu_dm_halted , 
output logic pcu_dm_havereset , 
output logic pcu_dm_unavail , 
input logic dm_pcu_ack_havereset , 
output logic pcu_dm_cmd_done , 
output logic pcu_dm_bus_err , 
output logic pcu_dm_cmd_excp , 
input logic dm_pcu_dsch0_write , 
input logic [32-1:0] dm_pcu_dsch0_wdata , 
output logic [32-1:0] pcu_dm_dsch0_rdata , 
input logic dm_ifu_inst_vld , 
output logic ifu_dm_inst_rdy , 
input logic [32-1:0] dm_ifu_inst_data , 
output logic [2-1:0] pcu_tm_pc_ex_vld , 
output logic [2-1:0][32-1:0] pcu_tm_pc_ex , 
output logic [2-1:0] pcu_tm_inst_size_ex , 
input logic [2-1:0] tm_pcu_trigger_pc_vld , 
input logic [2-1:0] tm_pcu_trigger_pc_debug , 
input logic [2-1:0][4-1:0] tm_pcu_trigger_hit_pc_idx , 
output logic pcu_tm_trap_vld , 
output logic [32-1:0] pcu_tm_trap_cause , 
output logic [2-1:0] pcu_tm_inst_retire , 
output logic pcu_tm_tcontrol_mte , 
output logic pcu_tm_trigger_hit , 
output logic [4-1:0] pcu_tm_trigger_hit_idx , 
input logic tm_pcu_csr_done , 
input logic [32-1:0] tm_pcu_csr_rdata , 
input logic tm_pcu_csr_excp , 
input logic tm_pcu_csr_match , 
output logic lsu_tm_a_vld , 
output logic[32-1:0] lsu_tm_a_addr , 
output logic[2:0] lsu_tm_a_ldst , 
output logic[2-1:0] lsu_tm_a_size , 
input logic tm_lsu_trigger_a_vld , 
input logic tm_lsu_trigger_debug_a , 
input logic[4-1:0] tm_lsu_trigger_hit_a_idx , 
output logic lsu_tm_d_vld , 
output logic[32-1:0] lsu_tm_d_data , 
output logic[2:0] lsu_tm_d_ldst , 
output logic[2-1:0] lsu_tm_d_size , 
input logic tm_lsu_trigger_d_vld , 
input logic tm_lsu_trigger_debug_d , 
output logic pcu_csr_vld , 
output logic [1:0] pcu_csr_opcode , 
output logic [11:0] pcu_csr_index , 
output logic [32-1:0] pcu_csr_wdata , 
output logic pcu_csr_mprv , 
output logic [1:0] pcu_csr_mpp , 
output logic [1:0] core_priv_mode , 
output logic core_dbg_mode , 
output logic core_sleep_mode , 
output logic core_deep_sleep , 
output logic core_sleep_wakeup , 
`ifdef M130_CORE_SUPPORT_ICACHE 
input logic ic_tagm_ecc_err , 
input logic ic_datm_ecc_err , 
`endif 
input logic mss_pcu_icache_init_busy , 
output logic ifu_mss_ar_valid , 
input logic mss_ifu_ar_ready , 
output logic [32-1:0] ifu_mss_ar_addr , 
output logic [2:0] ifu_mss_ar_size , 
output logic [7:0] ifu_mss_ar_len , 
output logic [1:0] ifu_mss_ar_burst , 
output logic [2-1:0] ifu_mss_ar_id , 
output logic [3:0] ifu_mss_ar_cache , 
output logic ifu_mss_ar_lock , 
output logic [2:0] ifu_mss_ar_prot , 
output logic [4-1:0] ifu_mss_ar_user , 
output logic ifu_mss_flush , 
input logic mss_ifu_r_valid , 
output logic ifu_mss_r_ready , 
input logic [64-1:0] mss_ifu_r_data , 
input logic [2-1:0] mss_ifu_r_id , 
input logic mss_ifu_r_last , 
input logic [1:0] mss_ifu_r_resp , 
input logic [33-1:0] mss_ifu_r_user , 
output logic lsu_mss_ar_valid , 
input logic mss_lsu_ar_ready , 
output logic [32-1:0] lsu_mss_ar_addr , 
output logic [2:0] lsu_mss_ar_size , 
output logic [7:0] lsu_mss_ar_len , 
output logic [1:0] lsu_mss_ar_burst , 
output logic [2-1:0] lsu_mss_ar_id , 
output logic [3:0] lsu_mss_ar_cache , 
output logic lsu_mss_ar_lock , 
output logic [2:0] lsu_mss_ar_prot , 
output logic [5-1:0] lsu_mss_ar_user , 
output logic lsu_mss_aw_valid , 
input logic mss_lsu_aw_ready , 
output logic [32-1:0] lsu_mss_aw_addr , 
output logic [2:0] lsu_mss_aw_size , 
output logic [7:0] lsu_mss_aw_len , 
output logic [1:0] lsu_mss_aw_burst , 
output logic [2-1:0] lsu_mss_aw_id , 
output logic [3:0] lsu_mss_aw_cache , 
output logic lsu_mss_aw_lock , 
output logic [2:0] lsu_mss_aw_prot , 
output logic [5:0] lsu_mss_aw_atop , 
output logic [5-1:0] lsu_mss_aw_user , 
output logic lsu_mss_w_valid , 
input logic mss_lsu_w_ready , 
output logic [32-1:0] lsu_mss_w_data , 
output logic [32/8-1:0] lsu_mss_w_strb , 
output logic lsu_mss_w_last , 
input logic mss_lsu_r_valid , 
output logic lsu_mss_r_ready , 
input logic [32-1:0] mss_lsu_r_data , 
input logic [2-1:0] mss_lsu_r_id , 
input logic mss_lsu_r_last , 
input logic [1:0] mss_lsu_r_resp , 
input logic [1-1:0] mss_lsu_r_user , 
input logic mss_lsu_b_valid , 
output logic lsu_mss_b_ready , 
input logic [2-1:0] mss_lsu_b_id , 
input logic [1:0] mss_lsu_b_resp , 
output logic lsu_mss_fencei_req , 
input logic mss_lsu_fencei_done , 
input logic endianess , 
input logic [32-1:0] core_mmr_base_addr , 
output logic lsu_mss_load_flush 
); 
logic [2-1:0] ifu_pcu_vld ; 
logic [2-1:0] pcu_ifu_rdy ; 
logic [2-1:0][32-1:0] ifu_pcu_instr_data ; 
logic [2-1:0][32-1:0] ifu_pcu_instr_pc ; 
logic [2-1:0][4:0] ifu_pcu_instr_rs1_idx ; 
logic [2-1:0][4:0] ifu_pcu_instr_rs2_idx ; 
logic [2-1:0] ifu_pcu_instr_rs1_vld ; 
logic [2-1:0] ifu_pcu_instr_rs2_vld ; 
logic [2-1:0] ifu_pcu_excp_vld ; 
logic [31:0] ifu_pcu_excp_cause ; 
logic pcu_ifu_flush_vld ; 
logic [32-1:0] pcu_ifu_flush_pc ; 
logic [1:0] pcu_ifu_shv_flush ; 
logic ifu_pcu_hv_done ; 
logic ifu_pcu_hv_excp_vld ; 
logic [32-1:0] ifu_pcu_hv_pc ; 
logic pcu_ifu_brq_rd ; 
logic [32-1:0] ifu_pcu_brq_pc ; 
logic pcu_ifu_call_rslv ; 
logic pcu_ifu_ret_rslv ; 
logic pcu_ifu_stall ; 
logic ifu_pmp_vld ; 
logic [32-1:0] ifu_pmp_addr ; 
logic pmp_ifu_err ; 
logic pma_ifu_err ; 
logic pma_ifu_cacheable ; 
logic core_init_busy ; 
logic ifu_idle ; 
logic lsu_idle ; 
logic [2-1:0] pcu_exu_inst_vld ; 
logic [2-1:0][32-1:0] pcu_exu_alu_op1_val ; 
logic [2-1:0][32-1:0] pcu_exu_alu_logic_op1_val ; 
logic [2-1:0][32-1:0] pcu_exu_bru_op1_val ; 
logic [2-1:0][32-1:0] pcu_exu_bru_eq_op1_val ; 
logic [2-1:0][32-1:0] pcu_exu_mdu_op1_val ; 
logic [2-1:0][32-1:0] pcu_exu_agu_op1_val ; 
logic [2-1:0][32-1:0] pcu_exu_alu_op2_val ; 
logic [2-1:0][4:0] pcu_exu_shift_shamt ; 
logic [2-1:0][32-1:0] pcu_exu_alu_logic_op2_val ; 
logic [2-1:0][32-1:0] pcu_exu_bru_op2_val ; 
logic [2-1:0][32-1:0] pcu_exu_bru_eq_op2_val ; 
logic [2-1:0][32-1:0] pcu_exu_mdu_op2_val ; 
logic [2-1:0][32-1:0] pcu_exu_agu_op2_val ; 
logic [2-1:0][32-1:0] pcu_exu_inst_op3_val ; 
logic [2-1:0] pcu_exu_inst_signed ; 
logic [2-1:0][8-1:0] pcu_exu_inst_funct ; 
logic [2-1:0][4-1:0] pcu_exu_inst_sub_funct ; 
logic [2-1:0][8-1:0] pcu_exu_inst_opcode ; 
logic [2-1:0] pcu_exu_inst_type ; 
logic [2-1:0] pcu_exu_inst_is_ret ; 
logic [32-1:0] pcu_exu_inst_pc ; 
logic [32-1:0] pcu_exu_inst_pred_pc ; 
logic pcu_exu_csr_uni_ari_dly ; 
logic pcu_exu_csr_uni_br_dly ; 
logic pcu_exu_random_exe_stall ; 
logic pcu_csr_sp_en ; 
logic [2-1:0][4-1:0] pcu_exu_sub_funct_wb ; 
logic [2-1:0][8-1:0] pcu_exu_opcode_wb ; 
logic exu_pcu_br_tkn ; 
logic [2-1:0] exu_pcu_inst_wb_stall ; 
logic [2-1:0][32-1:0] exu_pcu_inst_alu_data ; 
logic [2-1:0][32-1:0] exu_pcu_inst_alu_logic_data; 
logic [2-1:0][32-1:0] exu_pcu_inst_mdu_data ; 
logic [32-1:0] exu_pcu_inst_tgt_pc ; 
logic exu_pcu_bru_flush ; 
logic [32-1:0] exu_pcu_bru_flush_pc ; 
logic [1-1:0] pcu_lsu_inst_vld ; 
logic [1-1:0] lsu_pcu_inst_rdy ; 
logic [1-1:0][4-1:0] pcu_lsu_inst_cmd ; 
logic [1-1:0][3:0] pcu_lsu_inst_fence_op ; 
logic [1-1:0][$clog2(7)-1:0] pcu_lsu_inst_amo_op ; 
logic [1-1:0] pcu_lsu_inst_exclusive ; 
logic [1-1:0] pcu_lsu_inst_ord_aq ; 
logic [1-1:0] pcu_lsu_inst_ord_rl ; 
logic [1-1:0] pcu_lsu_inst_dst_vld ; 
logic [1-1:0][32-1:0] pcu_lsu_inst_data ; 
logic [1-1:0] pcu_lsu_inst_data_wb_vld ; 
logic [1-1:0][2-1:0] pcu_lsu_inst_size ; 
logic [1-1:0] pcu_lsu_inst_signed ; 
logic [1-1:0] pcu_lsu_inst_addr_fwd ; 
logic [1-1:0][32-1:0] pcu_lsu_inst_addr_fwd_val ; 
logic [1-1:0] lsu_pcu_cmt ; 
logic [1*32-1:0] lsu_pcu_cmt_data ; 
logic [1-1:0] lsu_pcu_cmt_excp ; 
logic [1-1:0][32-1:0] lsu_pcu_cmt_excp_cause ; 
logic [1-1:0][32-1:0] lsu_pcu_cmt_excp_addr ; 
logic [1-1:0] lsu_pcu_cmt_trig_dbg ; 
logic pcu_lsu_flush ; 
logic pcu_lsu_flush_st_inst_wb ; 
logic pcu_lsu_non_spec ; 
logic lsu_pcu_non_flush_infly ; 
logic pmp_pcu_csr_done ; 
logic [32-1:0] pmp_pcu_csr_rdata ; 
logic pmp_pcu_csr_excp ; 
logic pmp_pcu_csr_match ; 
logic [6:0] pcu_top_security_err_bus ; 
logic [32-1:0] exu_lsu_address ; 
logic lsu_pmp_vld ; 
logic [32-1:0] lsu_pmp_addr ; 
logic lsu_pmp_rw ; 
logic pmp_lsu_err ; 
logic pma_lsu_idempotency_n ; 
logic pma_lsu_cacheable ; 
m130_ifu u_m130_ifu ( 
.clk (clk ) , 
.rst_n (rst_n ) , 
.endianness (endianess ) , 
.top_ifu_boot_pc (top_ifu_boot_pc ) , 
.itcm_base_addr (itcm_base_addr ) , 
.dtcm_base_addr (dtcm_base_addr ) , 
.ifu_pcu_vld (ifu_pcu_vld ) , 
.pcu_ifu_rdy (pcu_ifu_rdy ) , 
.ifu_pcu_instr_data (ifu_pcu_instr_data ) , 
.ifu_pcu_instr_pc (ifu_pcu_instr_pc ) , 
.ifu_pcu_instr_rs1_idx (ifu_pcu_instr_rs1_idx ) , 
.ifu_pcu_instr_rs2_idx (ifu_pcu_instr_rs2_idx ) , 
.ifu_pcu_instr_rs1_vld (ifu_pcu_instr_rs1_vld ) , 
.ifu_pcu_instr_rs2_vld (ifu_pcu_instr_rs2_vld ) , 
.ifu_pcu_excp_vld (ifu_pcu_excp_vld ) , 
.ifu_pcu_excp_cause (ifu_pcu_excp_cause ) , 
.pcu_ifu_flush_vld (pcu_ifu_flush_vld ) , 
.pcu_ifu_flush_pc (pcu_ifu_flush_pc ) , 
.pcu_ifu_shv_flush (pcu_ifu_shv_flush ) , 
.ifu_pcu_hv_done (ifu_pcu_hv_done ) , 
.ifu_pcu_hv_excp_vld (ifu_pcu_hv_excp_vld ) , 
.ifu_pcu_hv_pc (ifu_pcu_hv_pc ) , 
.pcu_ifu_brq_rd (pcu_ifu_brq_rd ) , 
.ifu_pcu_brq_pc (ifu_pcu_brq_pc ) , 
.pcu_ifu_call_rslv (pcu_ifu_call_rslv ) , 
.pcu_ifu_ret_rslv (pcu_ifu_ret_rslv ) , 
.pcu_ifu_stall (pcu_ifu_stall|core_wait ) , 
.pcu_csr_sp_en (pcu_csr_sp_en ) , 
.core_priv_mode (core_priv_mode ) , 
.ifu_idle (ifu_idle ) , 
.ifu_mss_ar_valid (ifu_mss_ar_valid ) , 
.mss_ifu_ar_ready (mss_ifu_ar_ready ) , 
.ifu_mss_ar_addr (ifu_mss_ar_addr ) , 
.ifu_mss_ar_size (ifu_mss_ar_size ) , 
.ifu_mss_ar_len (ifu_mss_ar_len ) , 
.ifu_mss_ar_burst (ifu_mss_ar_burst ) , 
.ifu_mss_ar_id (ifu_mss_ar_id ) , 
.ifu_mss_ar_cache (ifu_mss_ar_cache ) , 
.ifu_mss_ar_lock (ifu_mss_ar_lock ) , 
.ifu_mss_ar_prot (ifu_mss_ar_prot ) , 
.ifu_mss_ar_user (ifu_mss_ar_user ) , 
.ifu_mss_flush (ifu_mss_flush ) , 
.mss_ifu_r_valid (mss_ifu_r_valid ) , 
.ifu_mss_r_ready (ifu_mss_r_ready ) , 
.mss_ifu_r_data (mss_ifu_r_data ) , 
.mss_ifu_r_id (mss_ifu_r_id ) , 
.mss_ifu_r_last (mss_ifu_r_last ) , 
.mss_ifu_r_resp (mss_ifu_r_resp ) , 
.mss_ifu_r_user (mss_ifu_r_user ) , 
.ifu_pmp_vld (ifu_pmp_vld ) , 
.ifu_pmp_addr (ifu_pmp_addr ) , 
.pmp_ifu_err (pmp_ifu_err ) , 
.pma_ifu_err (pma_ifu_err ) , 
.pma_ifu_cacheable (pma_ifu_cacheable ) , 
.dm_ifu_inst_vld (dm_ifu_inst_vld ) , 
.ifu_dm_inst_rdy (ifu_dm_inst_rdy ) , 
.dm_ifu_inst_data (dm_ifu_inst_data ) 
); 
m130_pcu_top u_m130_pcu_top ( 
.clk (clk ), 
.always_on_clk (always_on_clk ), 
.rst_n (rst_n ), 
.core_init_busy (core_init_busy ), 
.core_priv_mode (core_priv_mode ), 
.core_dbg_mode (core_dbg_mode ), 
.core_sleep_mode (core_sleep_mode ), 
.core_deep_sleep (core_deep_sleep ), 
.core_sleep_wakeup (core_sleep_wakeup ), 
.core_locked (lockup ), 
.ifu_idle (ifu_idle ), 
.lsu_idle (lsu_idle ), 
.mss_idle (mss_idle ), 
.bmu_idle (bmu_idle ), 
.mss_pcu_icache_init_busy (mss_pcu_icache_init_busy ), 
.top_hart_id (top_hart_id ), 
.top_boot_pc (top_ifu_boot_pc ), 
.top_mstatus_be (endianess ), 
.mem_map_mtime (mem_map_mtime ), 
.pcu_top_security_err_bus (pcu_top_security_err_bus ), 
.ifu_pcu_vld (ifu_pcu_vld ), 
.pcu_ifu_rdy (pcu_ifu_rdy ), 
.ifu_pcu_instr_data (ifu_pcu_instr_data ), 
.ifu_pcu_instr_pc (ifu_pcu_instr_pc ), 
.ifu_pcu_instr_rs1_idx (ifu_pcu_instr_rs1_idx ), 
.ifu_pcu_instr_rs2_idx (ifu_pcu_instr_rs2_idx ), 
.ifu_pcu_instr_rs1_vld (ifu_pcu_instr_rs1_vld ), 
.ifu_pcu_instr_rs2_vld (ifu_pcu_instr_rs2_vld ), 
.ifu_pcu_excp_vld (ifu_pcu_excp_vld ), 
.pcu_ifu_call_rslv (pcu_ifu_call_rslv ), 
.pcu_ifu_ret_rslv (pcu_ifu_ret_rslv ), 
.ifu_pcu_excp_cause (ifu_pcu_excp_cause ), 
.pcu_ifu_flush_vld (pcu_ifu_flush_vld ), 
.pcu_ifu_flush_pc (pcu_ifu_flush_pc ), 
.pcu_ifu_brq_rd (pcu_ifu_brq_rd ), 
.ifu_pcu_brq_pc (ifu_pcu_brq_pc ), 
.pcu_ifu_stall (pcu_ifu_stall ), 
.pcu_ifu_shv_flush (pcu_ifu_shv_flush ), 
.ifu_pcu_hv_done (ifu_pcu_hv_done ), 
.ifu_pcu_hv_excp_vld (ifu_pcu_hv_excp_vld ), 
.ifu_pcu_hv_pc (ifu_pcu_hv_pc ), 
.pcu_exu_inst_vld (pcu_exu_inst_vld ), 
.pcu_exu_alu_op1_val (pcu_exu_alu_op1_val ), 
.pcu_exu_alu_logic_op1_val (pcu_exu_alu_logic_op1_val ), 
.pcu_exu_bru_op1_val (pcu_exu_bru_op1_val ), 
.pcu_exu_bru_eq_op1_val (pcu_exu_bru_eq_op1_val ), 
.pcu_exu_mdu_op1_val (pcu_exu_mdu_op1_val ), 
.pcu_exu_agu_op1_val (pcu_exu_agu_op1_val ), 
.pcu_exu_alu_op2_val (pcu_exu_alu_op2_val ), 
.pcu_exu_shift_shamt (pcu_exu_shift_shamt ), 
.pcu_exu_alu_logic_op2_val (pcu_exu_alu_logic_op2_val ), 
.pcu_exu_bru_op2_val (pcu_exu_bru_op2_val ), 
.pcu_exu_bru_eq_op2_val (pcu_exu_bru_eq_op2_val ), 
.pcu_exu_mdu_op2_val (pcu_exu_mdu_op2_val ), 
.pcu_exu_agu_op2_val (pcu_exu_agu_op2_val ), 
.pcu_exu_inst_op3_val (pcu_exu_inst_op3_val ), 
.pcu_exu_inst_signed (pcu_exu_inst_signed ), 
.pcu_exu_inst_funct (pcu_exu_inst_funct ), 
.pcu_exu_inst_sub_funct (pcu_exu_inst_sub_funct ), 
.pcu_exu_inst_opcode (pcu_exu_inst_opcode ), 
.pcu_exu_inst_type (pcu_exu_inst_type ), 
.pcu_exu_inst_is_ret (pcu_exu_inst_is_ret ), 
.pcu_exu_inst_pc (pcu_exu_inst_pc ), 
.pcu_exu_inst_pred_pc (pcu_exu_inst_pred_pc ), 
.pcu_exu_sub_funct_wb (pcu_exu_sub_funct_wb ), 
.pcu_exu_opcode_wb (pcu_exu_opcode_wb ), 
.exu_pcu_br_tkn (exu_pcu_br_tkn ), 
.exu_pcu_inst_wb_stall (exu_pcu_inst_wb_stall ), 
.exu_pcu_inst_alu_data (exu_pcu_inst_alu_data ), 
.exu_pcu_inst_alu_logic_data (exu_pcu_inst_alu_logic_data), 
.exu_pcu_inst_mdu_data (exu_pcu_inst_mdu_data ), 
.exu_pcu_inst_tgt_pc (exu_pcu_inst_tgt_pc ), 
.exu_pcu_bru_flush (exu_pcu_bru_flush ), 
.exu_pcu_bru_flush_pc (exu_pcu_bru_flush_pc ), 
.pcu_lsu_inst_vld (pcu_lsu_inst_vld ), 
.lsu_pcu_inst_rdy (lsu_pcu_inst_rdy ), 
.pcu_lsu_inst_cmd (pcu_lsu_inst_cmd ), 
.pcu_lsu_inst_fence_op (pcu_lsu_inst_fence_op ), 
.pcu_lsu_inst_amo_op (pcu_lsu_inst_amo_op ), 
.pcu_lsu_inst_exclusive (pcu_lsu_inst_exclusive ), 
.pcu_lsu_inst_ord_aq (pcu_lsu_inst_ord_aq ), 
.pcu_lsu_inst_ord_rl (pcu_lsu_inst_ord_rl ), 
.pcu_lsu_inst_dst_vld (pcu_lsu_inst_dst_vld ), 
.pcu_lsu_inst_data (pcu_lsu_inst_data ), 
.pcu_lsu_inst_data_wb_vld (pcu_lsu_inst_data_wb_vld ), 
.pcu_lsu_inst_size (pcu_lsu_inst_size ), 
.pcu_lsu_inst_signed (pcu_lsu_inst_signed ), 
.pcu_lsu_inst_addr_fwd (pcu_lsu_inst_addr_fwd ), 
.pcu_lsu_inst_addr_fwd_val (pcu_lsu_inst_addr_fwd_val ), 
.lsu_pcu_cmt (lsu_pcu_cmt ), 
.lsu_pcu_cmt_data (lsu_pcu_cmt_data ), 
.lsu_pcu_cmt_excp (lsu_pcu_cmt_excp ), 
.lsu_pcu_cmt_excp_cause (lsu_pcu_cmt_excp_cause ), 
.lsu_pcu_cmt_excp_addr (lsu_pcu_cmt_excp_addr ), 
.lsu_pcu_cmt_trig_dbg (lsu_pcu_cmt_trig_dbg ), 
.pcu_lsu_flush_st_inst_wb (pcu_lsu_flush_st_inst_wb ) , 
.pcu_lsu_flush (pcu_lsu_flush ), 
.pcu_lsu_non_spec (pcu_lsu_non_spec ), 
.lsu_pcu_non_flush_infly (lsu_pcu_non_flush_infly ), 
.pmp_pcu_csr_done (pmp_pcu_csr_done ), 
.pmp_pcu_csr_rdata (pmp_pcu_csr_rdata ), 
.pmp_pcu_csr_excp (pmp_pcu_csr_excp ), 
.pmp_pcu_csr_match (pmp_pcu_csr_match ), 
.clic_pcu_int_vld (clic_pcu_int_vld ), 
.clic_pcu_int_id (clic_pcu_int_id ), 
.clic_pcu_int_lvl (clic_pcu_int_lvl ), 
.clic_pcu_int_shv (clic_pcu_int_shv ), 
.clic_pcu_int_priv (clic_pcu_int_priv ), 
.pcu_clic_rsp_clr_ip (pcu_clic_rsp_clr_ip ), 
.pcu_clic_rsp_id (pcu_clic_rsp_id ), 
.pcu_clic_rsp_vld (pcu_clic_rsp_vld ), 
.pcu_clic_stoptime (pcu_clic_stoptime ), 
.dm_pcu_halt_req (dm_pcu_halt_req ), 
.dm_pcu_halt_on_reset (dm_pcu_halt_on_reset ), 
.dm_pcu_resume_req (dm_pcu_resume_req ), 
.pcu_dm_halted (pcu_dm_halted ), 
.pcu_dm_havereset (pcu_dm_havereset ), 
.pcu_dm_unavail (pcu_dm_unavail ), 
.dm_pcu_ack_havereset (dm_pcu_ack_havereset ), 
.pcu_dm_cmd_done (pcu_dm_cmd_done ), 
.pcu_dm_bus_err (pcu_dm_bus_err ), 
.pcu_dm_cmd_excp (pcu_dm_cmd_excp ), 
.dm_pcu_dsch0_write (dm_pcu_dsch0_write ), 
.dm_pcu_dsch0_wdata (dm_pcu_dsch0_wdata ), 
.pcu_dm_dsch0_rdata (pcu_dm_dsch0_rdata ), 
.pcu_tm_pc_ex_vld (pcu_tm_pc_ex_vld ), 
.pcu_tm_pc_ex (pcu_tm_pc_ex ), 
.pcu_tm_inst_size_ex (pcu_tm_inst_size_ex ), 
.tm_pcu_trigger_pc_vld (tm_pcu_trigger_pc_vld ), 
.tm_pcu_trigger_pc_debug (tm_pcu_trigger_pc_debug ), 
.tm_pcu_trigger_hit_pc_idx (tm_pcu_trigger_hit_pc_idx ), 
.tm_pcu_trigger_addr_vld (tm_lsu_trigger_a_vld ), 
.tm_pcu_trigger_addr_debug (tm_lsu_trigger_debug_a ), 
.tm_pcu_trigger_hit_addr_idx (tm_lsu_trigger_hit_a_idx ), 
.pcu_tm_trap_vld (pcu_tm_trap_vld ), 
.pcu_tm_trap_cause (pcu_tm_trap_cause ), 
.pcu_tm_inst_retire (pcu_tm_inst_retire ), 
.pcu_tm_tcontrol_mte (pcu_tm_tcontrol_mte ), 
.pcu_tm_trigger_hit (pcu_tm_trigger_hit ), 
.pcu_tm_trigger_hit_idx (pcu_tm_trigger_hit_idx ), 
.tm_pcu_csr_done (tm_pcu_csr_done ), 
.tm_pcu_csr_rdata (tm_pcu_csr_rdata ), 
.tm_pcu_csr_excp (tm_pcu_csr_excp ), 
.tm_pcu_csr_match (tm_pcu_csr_match ), 
.pcu_csr_vld (pcu_csr_vld ), 
.pcu_csr_opcode (pcu_csr_opcode ), 
.pcu_csr_index (pcu_csr_index ), 
.pcu_csr_wdata (pcu_csr_wdata ), 
.pcu_csr_sp_en (pcu_csr_sp_en ), 
.pcu_csr_mprv (pcu_csr_mprv ), 
.pcu_csr_mpp (pcu_csr_mpp ), 
.pcu_csr_mstatus_be ( ) 
); 
m130_exu_top #( 
.SUPPORT_MUL_APPROACH (0) , 
.SUPPORT_DIV (1) , 
.SUPPORT_DIV_APPROACH (0) 
) u_m130_exu_top( 
.clk (clk ), 
.rst_n (rst_n ), 
.pcu_exu_inst_vld (pcu_exu_inst_vld ), 
.pcu_exu_flush (pcu_lsu_flush ), 
.pcu_exu_alu_op1_val (pcu_exu_alu_op1_val ), 
.pcu_exu_alu_logic_op1_val (pcu_exu_alu_logic_op1_val ), 
.pcu_exu_alu_op2_val (pcu_exu_alu_op2_val ), 
.pcu_exu_alu_logic_op2_val (pcu_exu_alu_logic_op2_val ), 
.pcu_exu_alu_shift_shamt (pcu_exu_shift_shamt ), 
.pcu_exu_bru_op1_val (pcu_exu_bru_op1_val ), 
.pcu_exu_bru_eq_op1_val (pcu_exu_bru_eq_op1_val ), 
.pcu_exu_bru_op2_val (pcu_exu_bru_op2_val ), 
.pcu_exu_bru_eq_op2_val (pcu_exu_bru_eq_op2_val ), 
.pcu_exu_mdu_op1_val (pcu_exu_mdu_op1_val ), 
.pcu_exu_mdu_op2_val (pcu_exu_mdu_op2_val ), 
.pcu_exu_agu_op1_val (pcu_exu_agu_op1_val ), 
.pcu_exu_agu_op2_val (pcu_exu_agu_op2_val ), 
.pcu_exu_inst_op3_val (pcu_exu_inst_op3_val ), 
.pcu_exu_inst_signed (pcu_exu_inst_signed ), 
.pcu_exu_inst_funct (pcu_exu_inst_funct ), 
.pcu_exu_inst_sub_funct (pcu_exu_inst_sub_funct ), 
.pcu_exu_inst_opcode (pcu_exu_inst_opcode ), 
.pcu_exu_sub_funct_wb (pcu_exu_sub_funct_wb ), 
.pcu_exu_opcode_wb (pcu_exu_opcode_wb ), 
.exu_pcu_br_tkn (exu_pcu_br_tkn ), 
.pcu_exu_inst_type (pcu_exu_inst_type ), 
.pcu_exu_inst_is_ret (pcu_exu_inst_is_ret ), 
.pcu_exu_inst_pc (pcu_exu_inst_pc ), 
.pcu_exu_inst_pred_pc (pcu_exu_inst_pred_pc ), 
.pcu_exu_csr_sp_en (pcu_csr_sp_en ), 
.exu_pcu_inst_wb_stall (exu_pcu_inst_wb_stall ), 
.exu_pcu_inst_alu_data (exu_pcu_inst_alu_data ), 
.exu_pcu_inst_alu_logic_data (exu_pcu_inst_alu_logic_data ), 
.exu_pcu_inst_mdu_data (exu_pcu_inst_mdu_data ), 
.exu_pcu_bru_flush (exu_pcu_bru_flush ), 
.exu_pcu_bru_flush_pc (exu_pcu_bru_flush_pc ), 
.exu_pcu_inst_tgt_pc (exu_pcu_inst_tgt_pc ), 
.exu_lsu_address (exu_lsu_address ) 
); 
m130_lsu_top u_m130_lsu_top( 
.clk (clk ) , 
.rst_n (rst_n ) , 
.pcu_lsu_inst_vld (pcu_lsu_inst_vld ) , 
.lsu_pcu_inst_rdy (lsu_pcu_inst_rdy ) , 
.pcu_lsu_inst_cmd (pcu_lsu_inst_cmd ) , 
.pcu_lsu_inst_fence_op (pcu_lsu_inst_fence_op ) , 
.pcu_lsu_inst_amo_op (pcu_lsu_inst_amo_op ) , 
.pcu_lsu_inst_exclusive (pcu_lsu_inst_exclusive ) , 
.pcu_lsu_inst_ord_aq (pcu_lsu_inst_ord_aq ) , 
.pcu_lsu_inst_ord_rl (pcu_lsu_inst_ord_rl ) , 
.pcu_lsu_inst_dst_vld (pcu_lsu_inst_dst_vld ) , 
.pcu_lsu_inst_data_wb_vld (pcu_lsu_inst_data_wb_vld ) , 
.pcu_lsu_inst_data (pcu_lsu_inst_data ) , 
.pcu_lsu_inst_size (pcu_lsu_inst_size ) , 
.pcu_lsu_inst_signed (pcu_lsu_inst_signed ) , 
.pcu_lsu_inst_addr_fwd (pcu_lsu_inst_addr_fwd ) , 
.pcu_lsu_inst_addr_fwd_val (pcu_lsu_inst_addr_fwd_val ) , 
.lsu_pcu_cmt (lsu_pcu_cmt ) , 
.lsu_pcu_cmt_data (lsu_pcu_cmt_data ) , 
.lsu_pcu_cmt_excp (lsu_pcu_cmt_excp ) , 
.lsu_pcu_cmt_excp_cause (lsu_pcu_cmt_excp_cause ) , 
.lsu_pcu_cmt_excp_addr (lsu_pcu_cmt_excp_addr ) , 
.lsu_pcu_cmt_trig_dbg (lsu_pcu_cmt_trig_dbg ) , 
.pcu_lsu_flush (pcu_lsu_flush ) , 
.pcu_lsu_flush_st_inst_wb (pcu_lsu_flush_st_inst_wb ) , 
.pcu_lsu_non_spec (pcu_lsu_non_spec ) , 
.lsu_pcu_non_flush_infly (lsu_pcu_non_flush_infly ) , 
.core_dbg_mode (core_dbg_mode ) , 
.core_priv_mode (core_priv_mode ) , 
.exu_lsu_address (exu_lsu_address ) , 
.lsu_pmp_vld (lsu_pmp_vld ) , 
.lsu_pmp_addr (lsu_pmp_addr ) , 
.lsu_pmp_rw (lsu_pmp_rw ) , 
.pmp_lsu_err (pmp_lsu_err ) , 
.pma_lsu_idempotency_n (pma_lsu_idempotency_n ) , 
.pma_lsu_cacheable (pma_lsu_cacheable ) , 
.lsu_tm_a_vld (lsu_tm_a_vld ) , 
.lsu_tm_a_addr (lsu_tm_a_addr ) , 
.lsu_tm_a_ldst (lsu_tm_a_ldst ) , 
.lsu_tm_a_size (lsu_tm_a_size ) , 
.tm_lsu_trigger_a_vld (tm_lsu_trigger_a_vld ) , 
.tm_lsu_trigger_debug_a (tm_lsu_trigger_debug_a ) , 
.lsu_tm_d_vld (lsu_tm_d_vld ) , 
.lsu_tm_d_data (lsu_tm_d_data ) , 
.lsu_tm_d_ldst (lsu_tm_d_ldst ) , 
.lsu_tm_d_size (lsu_tm_d_size ) , 
.tm_lsu_trigger_d_vld (tm_lsu_trigger_d_vld ) , 
.tm_lsu_trigger_debug_d (tm_lsu_trigger_debug_d ) , 
.lsu_mss_ar_valid (lsu_mss_ar_valid ) , 
.mss_lsu_ar_ready (mss_lsu_ar_ready ) , 
.lsu_mss_ar_addr (lsu_mss_ar_addr ) , 
.lsu_mss_ar_size (lsu_mss_ar_size ) , 
.lsu_mss_ar_len (lsu_mss_ar_len ) , 
.lsu_mss_ar_burst (lsu_mss_ar_burst ) , 
.lsu_mss_ar_id (lsu_mss_ar_id ) , 
.lsu_mss_ar_cache (lsu_mss_ar_cache ) , 
.lsu_mss_ar_lock (lsu_mss_ar_lock ) , 
.lsu_mss_ar_prot (lsu_mss_ar_prot ) , 
.lsu_mss_ar_user (lsu_mss_ar_user ) , 
.lsu_mss_aw_valid (lsu_mss_aw_valid ) , 
.mss_lsu_aw_ready (mss_lsu_aw_ready ) , 
.lsu_mss_aw_addr (lsu_mss_aw_addr ) , 
.lsu_mss_aw_size (lsu_mss_aw_size ) , 
.lsu_mss_aw_len (lsu_mss_aw_len ) , 
.lsu_mss_aw_burst (lsu_mss_aw_burst ) , 
.lsu_mss_aw_id (lsu_mss_aw_id ) , 
.lsu_mss_aw_cache (lsu_mss_aw_cache ) , 
.lsu_mss_aw_lock (lsu_mss_aw_lock ) , 
.lsu_mss_aw_prot (lsu_mss_aw_prot ) , 
.lsu_mss_aw_user (lsu_mss_aw_user ) , 
.lsu_mss_aw_atop (lsu_mss_aw_atop ) , 
.lsu_mss_w_valid (lsu_mss_w_valid ) , 
.mss_lsu_w_ready (mss_lsu_w_ready ) , 
.lsu_mss_w_data (lsu_mss_w_data ) , 
.lsu_mss_w_strb (lsu_mss_w_strb ) , 
.lsu_mss_w_last (lsu_mss_w_last ) , 
.mss_lsu_r_valid (mss_lsu_r_valid ) , 
.lsu_mss_r_ready (lsu_mss_r_ready ) , 
.mss_lsu_r_data (mss_lsu_r_data ) , 
.mss_lsu_r_id (mss_lsu_r_id ) , 
.mss_lsu_r_last (mss_lsu_r_last ) , 
.mss_lsu_r_resp (mss_lsu_r_resp ) , 
.mss_lsu_r_user (mss_lsu_r_user ) , 
.mss_lsu_b_valid (mss_lsu_b_valid ) , 
.lsu_mss_b_ready (lsu_mss_b_ready ) , 
.mss_lsu_b_id (mss_lsu_b_id ) , 
.mss_lsu_b_resp (mss_lsu_b_resp ) , 
.lsu_mss_fencei_req (lsu_mss_fencei_req ) , 
.mss_lsu_fencei_done (mss_lsu_fencei_done ) , 
.lsu_mss_load_flush (lsu_mss_load_flush ) , 
.itcm_base_addr (itcm_base_addr ) , 
.dtcm_base_addr (dtcm_base_addr ) , 
.endianess (endianess ) , 
.core_mmr_base_addr (core_mmr_base_addr ) , 
.lsu_idle (lsu_idle ) 
); 
m130_pmp u_m130_pmp( 
.clk (clk ), 
.rst_n (rst_n ), 
.pmp_cfg_clk (pmp_cfg_clk ), 
.pmp_cfg_clk_en (pmp_cfg_clk_en ), 
.core_priv_mode (core_priv_mode ), 
.pcu_csr_mprv (pcu_csr_mprv ), 
.pcu_csr_mpp (pcu_csr_mpp ), 
.pcu_csr_vld (pcu_csr_vld ), 
.pcu_csr_opcode (pcu_csr_opcode ), 
.pcu_csr_index (pcu_csr_index ), 
.pcu_csr_wdata (pcu_csr_wdata ), 
.pmp_pcu_csr_done (pmp_pcu_csr_done ), 
.pmp_pcu_csr_rdata (pmp_pcu_csr_rdata ), 
.pmp_pcu_csr_excp (pmp_pcu_csr_excp ), 
.pmp_pcu_csr_match (pmp_pcu_csr_match ), 
.lsu_pmp_vld (lsu_pmp_vld ), 
.lsu_pmp_addr (lsu_pmp_addr ), 
.lsu_pmp_rw (lsu_pmp_rw ), 
.pmp_lsu_err (pmp_lsu_err ), 
.pma_lsu_idempotency_n (pma_lsu_idempotency_n ), 
.pma_lsu_cacheable (pma_lsu_cacheable ), 
.ifu_pmp_vld (ifu_pmp_vld ), 
.ifu_pmp_addr (ifu_pmp_addr ), 
.pmp_ifu_err (pmp_ifu_err ), 
.pma_ifu_err (pma_ifu_err ), 
.pma_ifu_cacheable (pma_ifu_cacheable ) 
); 
assign err_bus = { 4'b0, 
pcu_top_security_err_bus[4], 
pcu_top_security_err_bus[2], 
pcu_top_security_err_bus[3], 
`ifdef M130_CORE_SUPPORT_ICACHE 
ic_datm_ecc_err, 
ic_tagm_ecc_err, 
`else 
1'b0, 
1'b0, 
`endif 
1'b0, 
1'b0, 
1'b0, 
1'b0, 
pcu_top_security_err_bus[1] 
, 
1'b0 
, 
pcu_top_security_err_bus[0] 
}; 
endmodule
 

 
 
 

 
 
module m130_ifu( 
input logic clk, 
input logic rst_n, 
input logic endianness, 
input logic [32-1:0] top_ifu_boot_pc, 
input logic [32-1:0] itcm_base_addr, 
input logic [32-1:0] dtcm_base_addr, 
output logic [2-1:0] ifu_pcu_vld, 
input logic [2-1:0] pcu_ifu_rdy, 
output logic [2-1:0][32-1:0] ifu_pcu_instr_data, 
output logic [2-1:0][32-1:0] ifu_pcu_instr_pc, 
output logic [2-1:0][4:0] ifu_pcu_instr_rs1_idx, 
output logic [2-1:0][4:0] ifu_pcu_instr_rs2_idx, 
output logic [2-1:0] ifu_pcu_instr_rs1_vld, 
output logic [2-1:0] ifu_pcu_instr_rs2_vld, 
output logic [2-1:0] ifu_pcu_excp_vld, 
output logic [31:0] ifu_pcu_excp_cause, 
input logic pcu_ifu_flush_vld, 
input logic [32-1:0] pcu_ifu_flush_pc, 
input logic [1:0] pcu_ifu_shv_flush, 
output logic ifu_pcu_hv_done, 
output logic ifu_pcu_hv_excp_vld, 
output logic [32-1:0] ifu_pcu_hv_pc, 
input logic pcu_ifu_brq_rd, 
output logic [32-1:0] ifu_pcu_brq_pc, 
input logic pcu_ifu_call_rslv, 
input logic pcu_ifu_ret_rslv, 
input logic pcu_ifu_stall, 
input logic pcu_csr_sp_en, 
input logic [1:0] core_priv_mode, 
output logic ifu_idle, 
output logic ifu_mss_ar_valid, 
input logic mss_ifu_ar_ready, 
output logic [32-1:0] ifu_mss_ar_addr, 
output logic [2:0] ifu_mss_ar_size, 
output logic [7:0] ifu_mss_ar_len, 
output logic [1:0] ifu_mss_ar_burst, 
output logic [2-1:0] ifu_mss_ar_id, 
output logic [3:0] ifu_mss_ar_cache, 
output logic ifu_mss_ar_lock, 
output logic [2:0] ifu_mss_ar_prot, 
output logic [4-1:0] ifu_mss_ar_user, 
output logic ifu_mss_flush, 
input logic mss_ifu_r_valid, 
output logic ifu_mss_r_ready, 
input logic [64-1:0] mss_ifu_r_data, 
input logic [2-1:0] mss_ifu_r_id, 
input logic mss_ifu_r_last, 
input logic [1:0] mss_ifu_r_resp, 
input logic [33-1:0] mss_ifu_r_user, 
output logic ifu_pmp_vld, 
output logic [32-1:0] ifu_pmp_addr, 
input logic pmp_ifu_err, 
input logic pma_ifu_err, 
input logic pma_ifu_cacheable, 
input logic dm_ifu_inst_vld, 
output logic ifu_dm_inst_rdy, 
input logic [32-1:0] dm_ifu_inst_data 
); 
localparam FETCHSET_INSTR_N = 64/16; 
localparam FETCHSET_OFFSET_W = $clog2(64/8); 
localparam BRANCH_FNTBT = 1; 
localparam RAS_DEPTH = 2; 
localparam RAS_PTR_W = ($clog2(2)); 
localparam RAS_OPT_TIMING = 0; 
localparam PCBUF_ADDR_W = $clog2(3); 
localparam BRQ_DEPTH = 3; 
localparam BRQ_ADDW = $clog2(BRQ_DEPTH); 
logic boot_done_en; 
logic boot_done_d; 
logic boot_done_q; 
logic fetch_pc_en; 
logic [30:0] fetch_pc_d; 
logic [30:0] fetch_pc_q; 
logic [31:0] fetch_pc_reg; 
logic [31:0] fetch_pc_cons; 
logic [31:0] fetch_pc; 
logic [31:0] pcgen_ifetch_pc; 
logic ar_hdsk_succeed_set; 
logic ar_hdsk_succeed_clr; 
logic ar_hdsk_succeed_en; 
logic ar_hdsk_succeed_d; 
logic ar_hdsk_succeed_q; 
logic ifetch_req_sent_en ; 
logic ifetch_req_sent_set ; 
logic ifetch_req_sent_clr ; 
logic ifetch_req_sent_d ; 
logic ifetch_req_sent_q ; 
logic[FETCHSET_OFFSET_W-1:0] ifetch_sent_offset ; 
logic hv_flag_set; 
logic hv_flag_clr; 
logic hv_flag_en; 
logic[1:0] hv_flag_d; 
logic[1:0] hv_flag_q; 
logic hv_done; 
logic hv_done_flag_q; 
logic hv_success; 
logic [31:0] hv_data_be_conv; 
logic [31:0] hv_data; 
logic [31:0] hv_redirect_pc; 
logic excp_wr_buf_no; 
logic ifetch_stall; 
logic hv_stall; 
logic ifetch_req; 
logic ifu_mss_ar_hdsk; 
logic ifetch_rsp_vld; 
logic route_ifu_dtcm ; 
logic tcm_reserve_err ; 
logic dtcm_reserve_err ; 
logic itcm_reserve_err ; 
logic prot_check_vld ; 
logic fetch_access_fault ; 
logic cancel_fetch_req ; 
logic flag_access_fault_set; 
logic flag_access_fault_clr; 
logic flag_access_fault_en ; 
logic flag_access_fault_d ; 
logic flag_access_fault_q ; 
logic ras_error ; 
logic rsp_excp_vld; 
logic excp_vld_set; 
logic excp_vld_clr; 
logic excp_vld_en ; 
logic excp_vld_d ; 
logic excp_vld_q ; 
logic excp_cause_en ; 
logic[1:0] excp_cause_d ; 
logic[1:0] excp_cause_q ; 
logic instr1_loc_slot1; 
logic res_buf_vld_set_pre; 
logic res_buf_vld_set; 
logic res_buf_vld_clr; 
logic res_buf_vld_clr_pre; 
logic res_buf_vld_en; 
logic res_buf_vld_d; 
logic res_buf_vld_q; 
logic res_buf_dat_en; 
logic [15:0] res_buf_dat_d; 
logic [15:0] res_buf_dat_q; 
logic [FETCHSET_INSTR_N-1:0] unaln_instr_is_rvi; 
logic [FETCHSET_INSTR_N-1:0] unaln_instr_is_rvc; 
logic [FETCHSET_INSTR_N-1:0][FETCHSET_OFFSET_W-1:0] unaln_instr_offset; 
logic [FETCHSET_INSTR_N-1:0] aln_instr_vld_pre; 
logic [FETCHSET_INSTR_N-1:0] aln_instr_vld; 
logic [FETCHSET_INSTR_N-1:0][31:0] aln_instr_dat; 
logic [FETCHSET_INSTR_N-1:0][31:0] aln_instr_addr; 
logic [FETCHSET_INSTR_N-1:0][31:FETCHSET_OFFSET_W] aln_instr_base_addr; 
logic [FETCHSET_INSTR_N-1:0][FETCHSET_OFFSET_W-1:0] aln_instr_offset; 
logic [FETCHSET_INSTR_N-1:0][6:0] pdec_rvi_op; 
logic [FETCHSET_INSTR_N-1:0][4:0] pdec_rvc_op; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvi_branch; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvi_jal; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvi_jalr; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvi_jalr_x0; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvi_call; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvi_ret; 
logic [FETCHSET_INSTR_N-1:0][4:0] pdec_rvi_rs1; 
logic [FETCHSET_INSTR_N-1:0][4:0] pdec_rvi_rd; 
logic [FETCHSET_INSTR_N-1:0][31:0] pdec_rvi_offs; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_branch; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jr_jalr; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jt_jalt; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_popret; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jalr; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jalr_x0; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jr; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jalt; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_j; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_jal; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_call; 
logic [FETCHSET_INSTR_N-1:0] pdec_rvc_ret; 
logic [FETCHSET_INSTR_N-1:0][4:0] pdec_rvc_rs1; 
logic [FETCHSET_INSTR_N-1:0][31:0] pdec_rvc_offs; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_rvc; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_branch; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_jal; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_jalr; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_jalr_x0; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_jvt; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_call; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_ret; 
logic [FETCHSET_INSTR_N-1:0] pdec_is_popret; 
logic [FETCHSET_INSTR_N-1:0][31:0] pdec_bj_offs; 
logic [FETCHSET_INSTR_N-1:0][31:0] pdec_next_addr; 
logic [FETCHSET_INSTR_N-1:0] sp_ctrl_tf; 
logic [FETCHSET_INSTR_N-1:0] sp_ctrl_tf_pre; 
logic [FETCHSET_INSTR_N-1:0] sp_ctrl_tf_mask; 
logic [FETCHSET_INSTR_N-1:0] sp_ctrl_tf_ff1; 
logic [FETCHSET_INSTR_N-1:0] sp_branch_taken; 
logic [FETCHSET_INSTR_N-1:0] sp_call_taken; 
logic [FETCHSET_INSTR_N-1:0] sp_ret_taken; 
logic [FETCHSET_INSTR_N-1:0] sp_bj_taken; 
logic [FETCHSET_INSTR_N-1:0] sp_predict_taken; 
logic sp_predict_vld; 
logic [31:0] sp_predict_pc; 
logic sp_pcgen_predict_vld; 
logic [31:0] sp_bj_addr; 
logic [31:0] sp_pcgen_predict_pc; 
logic sp_pcgen_predict_vld_q; 
logic sp_pcgen_predict_vld_d; 
logic [31:0] sp_pcgen_predict_pc_q; 
logic sp_jalr_stall_set; 
logic sp_jalr_stall_clr; 
logic sp_jalr_stall_en; 
logic sp_jalr_stall_d; 
logic sp_jalr_stall_q; 
logic [31:0] ras_addr_push; 
logic [31:0] ras_addr_pop; 
logic ras_push; 
logic ras_pop; 
logic [RAS_DEPTH-1:0] ras_buf_ent_en; 
logic [RAS_DEPTH-1:0][31:0] ras_buf_ent_d; 
logic [RAS_DEPTH-1:0][31:0] ras_buf_ent_q; 
logic [RAS_PTR_W-1:0] ras_ptr_update; 
logic [RAS_PTR_W-1:0] ras_ptr_next; 
logic ras_ptr_cur_en; 
logic [RAS_PTR_W-1:0] ras_ptr_cur_d; 
logic [RAS_PTR_W-1:0] ras_ptr_cur_q; 
logic ras_ptr_cmt_en; 
logic [RAS_PTR_W-1:0] ras_ptr_cmt_d; 
logic [RAS_PTR_W-1:0] ras_ptr_cmt_q; 
logic brq_wr_en; 
logic brq_rd_en; 
logic [31:1] brq_wr_data; 
logic [31:1] brq_rd_data; 
logic brq_afull; 
logic brq_full; 
logic brq_empty; 
logic brq_ifetch_stall; 
logic brq_wr_ptr_en; 
logic [BRQ_ADDW:0] brq_wr_ptr_d; 
logic [BRQ_ADDW:0] brq_wr_ptr_q; 
logic brq_rd_ptr_en; 
logic [BRQ_ADDW:0] brq_rd_ptr_d; 
logic [BRQ_ADDW:0] brq_rd_ptr_q; 
logic [BRQ_ADDW:0] brq_wr_ptr_stand; 
logic [BRQ_ADDW-1:0] brq_wr_addr; 
logic [BRQ_ADDW-1:0] brq_wr_addr_next; 
logic [BRQ_ADDW-1:0] brq_rd_addr; 
logic [BRQ_ADDW-1:0] brq_rd_addr_next; 
logic brq_wr_ptr_rv; 
logic brq_rd_ptr_rv; 
logic [BRQ_DEPTH-1:0] brq_ent_en; 
logic [BRQ_DEPTH-1:0][31:1] brq_ent_q; 
logic [2-1:0] pcbuf_rd_en_vec; 
logic [2-1:0][PCBUF_ADDR_W-1:0] pcbuf_rd_addr; 
logic [2-1:0][31:FETCHSET_OFFSET_W]pcbuf_rdata; 
logic [31:FETCHSET_OFFSET_W] prev_fetchset_addr; 
logic pcbuf_wr_no; 
logic pcbuf_wr_en; 
logic [31:FETCHSET_OFFSET_W] pcbuf_wdata; 
logic [PCBUF_ADDR_W-1:0] pcbuf_wr_addr; 
logic [PCBUF_ADDR_W-1:0] pcbuf_wr_addr_prev; 
logic pcbuf_afull; 
logic pcbuf_full; 
logic pcbuf_ifetch_stall; 
typedef struct packed { 
logic [31:0] instr; 
logic [PCBUF_ADDR_W-1:0] pcbuf_addr; 
logic [FETCHSET_OFFSET_W-1:1] pc_offset; 
logic pop_pcbuf; 
logic excp_vld; 
logic is_hv; 
} ibuf_data_t; 
logic buf_wr_vld_mask; 
logic pop_pcbuf0_pre; 
logic [FETCHSET_INSTR_N-1:0] ibuf_wr_vld; 
ibuf_data_t [FETCHSET_INSTR_N-1:0] ibuf_wdata; 
ibuf_data_t [2-1:0] ibuf_rdata; 
logic [2-1:0] ibuf_rd_vld; 
logic [2-1:0] ibuf_rd_rdy; 
logic ibuf_flush; 
logic ibuf_ifetch_stall; 
logic [2-1:0] ifu_pcu_hdsk; 
assign boot_done_en = ~boot_done_q; 
assign boot_done_d = 1'b1; 
`WDFFER(boot_done_q, boot_done_d, boot_done_en, clk, rst_n) 
assign fetch_pc_en = ifu_mss_ar_hdsk & ar_hdsk_succeed_q | 
~boot_done_q | 
pcu_ifu_flush_vld | 
sp_pcgen_predict_vld | 
hv_success | 
flag_access_fault_set; 
assign fetch_pc_d = boot_done_en ? top_ifu_boot_pc[32-1:1] : 
pcgen_ifetch_pc[32-1:1] ; 
`WDFFER(fetch_pc_q, fetch_pc_d, fetch_pc_en, clk, rst_n) 
assign fetch_pc_reg = {fetch_pc_q, 1'b0}; 
assign fetch_pc_cons = {fetch_pc_reg[31:FETCHSET_OFFSET_W], {FETCHSET_OFFSET_W{'0}}} + 64/8; 
assign ar_hdsk_succeed_set = ifu_mss_ar_hdsk; 
assign ar_hdsk_succeed_clr = pcu_ifu_flush_vld | sp_pcgen_predict_vld | hv_success; 
assign ar_hdsk_succeed_en = ar_hdsk_succeed_set | ar_hdsk_succeed_clr; 
assign ar_hdsk_succeed_d = ar_hdsk_succeed_set; 
`WDFFER(ar_hdsk_succeed_q, ar_hdsk_succeed_d, ar_hdsk_succeed_en, clk, rst_n) 
assign fetch_pc = ar_hdsk_succeed_q ? fetch_pc_cons : fetch_pc_reg; 
assign pcgen_ifetch_pc = pcu_ifu_flush_vld ? pcu_ifu_flush_pc : 
hv_success ? hv_redirect_pc : 
sp_pcgen_predict_vld ? sp_pcgen_predict_pc : 
fetch_pc; 
assign ifetch_req_sent_set = ifu_mss_ar_hdsk; 
assign ifetch_req_sent_clr = mss_ifu_r_valid | pcu_ifu_flush_vld; 
assign ifetch_req_sent_en = ifetch_req_sent_set | ifetch_req_sent_clr; 
assign ifetch_req_sent_d = ifetch_req_sent_set; 
`WDFFER(ifetch_req_sent_q, ifetch_req_sent_d, ifetch_req_sent_en, clk, rst_n) 
assign ifetch_stall = pcu_ifu_stall | 
~pcu_ifu_flush_vld & (ibuf_ifetch_stall | 
pcbuf_ifetch_stall | 
brq_ifetch_stall | 
sp_jalr_stall_q | 
hv_stall ); 
assign route_ifu_dtcm = 1'b0; 
assign ifu_mss_ar_user[2] = 1'b0; 
assign ifu_mss_ar_user[1] = 1'b0; 
assign ifu_mss_ar_user[0] = ~(ifu_mss_ar_user[1] || ifu_mss_ar_user[2]); 
assign ifetch_req = ~ifetch_stall & boot_done_q; 
assign ifu_mss_ar_hdsk = ifu_mss_ar_valid & mss_ifu_ar_ready; 
assign ifu_mss_ar_valid = ifetch_req & ~cancel_fetch_req; 
assign ifu_mss_ar_addr = pcgen_ifetch_pc; 
assign ifu_mss_ar_size = 3'b011; 
assign ifu_mss_ar_len = '0; 
assign ifu_mss_ar_burst = '0; 
assign ifu_mss_ar_id = '0; 
assign ifu_mss_ar_cache = pma_ifu_cacheable ? 4'b1111 : 4'b0011; 
assign ifu_mss_ar_lock = '0; 
assign ifu_mss_ar_prot[0] = &core_priv_mode; 
assign ifu_mss_ar_prot[2:1] = 2'b11; 
assign ifu_mss_r_ready = 1'b1; 
assign ifu_mss_flush = pcu_ifu_flush_vld | sp_pcgen_predict_vld | hv_success; 
assign hv_flag_set = pcu_ifu_flush_vld & pcu_ifu_shv_flush[0]; 
assign hv_flag_clr = pcu_ifu_flush_vld | hv_done; 
assign hv_flag_d = hv_flag_set ? {pcu_ifu_shv_flush} : 2'b00; 
assign hv_flag_en = hv_flag_set | hv_flag_clr; 
`WDFFER(hv_flag_q, hv_flag_d, hv_flag_en, clk, rst_n) 
assign hv_stall = hv_flag_q[0] & ifetch_req_sent_q & ~hv_success; 
assign hv_done = hv_flag_q[0] & ibuf_rdata[0].is_hv & ibuf_rd_vld[0]; 
assign hv_success = hv_done & ~ibuf_rdata[0].excp_vld; 
assign hv_data_be_conv = {aln_instr_dat[0][7:0] , 
aln_instr_dat[0][15:8] , 
aln_instr_dat[0][23:16], 
aln_instr_dat[0][31:24]}; 
assign hv_data = endianness ? hv_data_be_conv : aln_instr_dat[0]; 
assign hv_redirect_pc = {ibuf_rdata[0].instr[31:1], 1'b0}; 
assign ifu_pcu_hv_done = hv_done; 
assign ifu_pcu_hv_excp_vld = ibuf_rdata[0].excp_vld; 
assign ifu_pcu_hv_pc = ibuf_rdata[0].excp_vld ? fetch_pc_reg 
: hv_redirect_pc; 
assign ifu_mss_ar_user[3] = (pcu_ifu_flush_vld & pcu_ifu_shv_flush[0] & pcu_ifu_shv_flush[1]) | 
(hv_flag_q[1] & !hv_done & !(pcu_ifu_flush_vld )); 
assign ifu_dm_inst_rdy = {2{1'b1}}; 
assign ifu_idle = pcu_ifu_stall & ~ifetch_req_sent_q & ~ifu_pcu_vld[0]; 
assign ifetch_rsp_vld = mss_ifu_r_valid & ~sp_jalr_stall_q; 
assign prot_check_vld = ifetch_req; 
assign ifu_pmp_vld = prot_check_vld; 
assign ifu_pmp_addr = ifu_mss_ar_addr; 
if (32*1024 == 256*1024)begin:g_max_dtcm 
assign dtcm_reserve_err = 1'b0; 
end 
else begin:g_dtcm_reserve_err 
assign dtcm_reserve_err = route_ifu_dtcm && ( ifu_mss_ar_addr[17:0] >= 32*1024); 
end 
if (32*1024 == 256*1024)begin:g_mac_itcm 
assign itcm_reserve_err = 1'b0; 
end 
else begin: g_itcm_reserve_err 
assign itcm_reserve_err = ifu_mss_ar_user[1] && ( ifu_mss_ar_addr[17:0] >= 32*1024); 
end 
assign tcm_reserve_err = (route_ifu_dtcm) | itcm_reserve_err; 
assign fetch_access_fault = pmp_ifu_err | tcm_reserve_err | (pma_ifu_err & ifu_mss_ar_user[0]); 
assign cancel_fetch_req = (pma_ifu_err & ifu_mss_ar_user[0]) | tcm_reserve_err; 
assign flag_access_fault_set = prot_check_vld & fetch_access_fault & (~ifetch_req_sent_q | mss_ifu_ar_ready); 
assign flag_access_fault_clr = pcu_ifu_flush_vld; 
assign flag_access_fault_en = flag_access_fault_set | flag_access_fault_clr; 
assign flag_access_fault_d = flag_access_fault_set; 
`WDFFER(flag_access_fault_q, flag_access_fault_d, flag_access_fault_en, clk, rst_n) 
assign ras_error = mss_ifu_r_valid & (|mss_ifu_r_resp | mss_ifu_r_user[0]); 
assign rsp_excp_vld = flag_access_fault_q | ras_error; 
assign excp_vld_set = rsp_excp_vld; 
assign excp_vld_clr = pcu_ifu_flush_vld; 
assign excp_vld_en = excp_vld_set | excp_vld_clr; 
assign excp_vld_d = excp_vld_set & ~excp_vld_clr; 
`WDFFER(excp_vld_q, excp_vld_d, excp_vld_en, clk, rst_n) 
assign excp_wr_buf_no = excp_vld_q; 
assign excp_cause_en = (rsp_excp_vld & ~excp_vld_q) | pcu_ifu_flush_vld; 
assign excp_cause_d = pcu_ifu_flush_vld ? 2'b00 : {ras_error & ~flag_access_fault_q, 
flag_access_fault_q}; 
`WDFFER(excp_cause_q, excp_cause_d, excp_cause_en, clk, rst_n) 
assign ifu_pcu_excp_cause = ( {32{excp_cause_q[1]}} & 32'h1f ) | 
( {32{excp_cause_q[0]}} & 32'h1 ) ; 
for (genvar i=0; i<FETCHSET_INSTR_N; i++) begin: g_is_rvc 
assign unaln_instr_is_rvi[i] = &mss_ifu_r_data[16*i +: 2]; 
assign unaln_instr_is_rvc[i] = ~unaln_instr_is_rvi[i]; 
assign unaln_instr_offset[i] = i*2; 
end 
assign instr1_loc_slot1 = res_buf_vld_q | (~res_buf_vld_q & unaln_instr_is_rvc[0]); 
for (genvar i=0; i<FETCHSET_INSTR_N; i++) begin: g_instr_vld 
assign aln_instr_vld[i] = ifetch_rsp_vld & aln_instr_vld_pre[i]; 
assign aln_instr_addr[i]= {aln_instr_base_addr[i], aln_instr_offset[i]}; 
end 
assign aln_instr_base_addr[0] = (~|fetch_pc_reg[FETCHSET_OFFSET_W-1:1] & res_buf_vld_q) ? prev_fetchset_addr[31:FETCHSET_OFFSET_W] 
: fetch_pc_reg[31:FETCHSET_OFFSET_W]; 
for (genvar i=1; i<FETCHSET_INSTR_N; i++) begin: g_instr_addr 
assign aln_instr_base_addr[i][31:FETCHSET_OFFSET_W]= fetch_pc_reg[31:FETCHSET_OFFSET_W]; 
end 
if (64/8 == 8) begin: g_64bit_fetch_aln 
assign aln_instr_vld_pre[0] = ~((fetch_pc_reg[2:1] == 2'b11) & unaln_instr_is_rvi[3]); 
always_comb begin : ALN_INSTR_DAT0 
unique case (fetch_pc_reg[2:1]) 
2'b00: begin 
if (res_buf_vld_q) begin 
aln_instr_dat[0] = {mss_ifu_r_data[15:0], res_buf_dat_q[15:0]}; 
aln_instr_offset[0] = 3'b110; 
end else begin 
aln_instr_dat[0] = mss_ifu_r_data[31:0]; 
aln_instr_offset[0] = unaln_instr_offset[0]; 
end 
end 
2'b01: begin 
aln_instr_dat[0] = mss_ifu_r_data[47:16]; 
aln_instr_offset[0] = unaln_instr_offset[1]; 
end 
2'b10: begin 
aln_instr_dat[0] = mss_ifu_r_data[63:32]; 
aln_instr_offset[0] = unaln_instr_offset[2]; 
end 
2'b11: begin 
aln_instr_dat[0] = {16'd0, mss_ifu_r_data[63:48]}; 
aln_instr_offset[0] = unaln_instr_offset[3]; 
end 
default: begin 
aln_instr_dat[0] = 'x; 
aln_instr_offset[0] = 'x; 
end 
endcase 
end 
always_comb begin: aln_instr_vld1 
unique case (fetch_pc_reg[2:1]) 
2'b01: aln_instr_vld_pre[1] = ~(unaln_instr_is_rvi[1] & unaln_instr_is_rvi[3]); 
2'b10: aln_instr_vld_pre[1] = &unaln_instr_is_rvc[3:2]; 
2'b11: aln_instr_vld_pre[1] = 1'b0; 
default: aln_instr_vld_pre[1] = 1'b1; 
endcase 
end 
always_comb begin : aln_instr_dat1 
unique case (fetch_pc_reg[2:1]) 
2'b00: begin 
if (instr1_loc_slot1) begin 
aln_instr_dat[1] = mss_ifu_r_data[47:16]; 
aln_instr_offset[1] = unaln_instr_offset[1]; 
end else begin 
aln_instr_dat[1] = mss_ifu_r_data[63:32]; 
aln_instr_offset[1] = unaln_instr_offset[2]; 
end 
end 
2'b01: begin 
if (unaln_instr_is_rvc[1]) begin 
aln_instr_dat[1] = mss_ifu_r_data[63:32]; 
aln_instr_offset[1] = unaln_instr_offset[2]; 
end else begin 
aln_instr_dat[1] = {16'd0, mss_ifu_r_data[63:48]}; 
aln_instr_offset[1] = unaln_instr_offset[3]; 
end 
end 
2'b10: begin 
aln_instr_dat[1] = {16'd0, mss_ifu_r_data[63:48]}; 
aln_instr_offset[1] = unaln_instr_offset[3]; 
end 
default: begin 
aln_instr_dat[1] = mss_ifu_r_data[63:32]; 
aln_instr_offset[1] = unaln_instr_offset[2]; 
end 
endcase 
end 
always_comb begin: aln_instr_vld2 
unique case (fetch_pc_reg[2:1]) 
2'b00: aln_instr_vld_pre[2] = instr1_loc_slot1 ? 
~(unaln_instr_is_rvi[1] & unaln_instr_is_rvi[3]) : 
(unaln_instr_is_rvc[2] & unaln_instr_is_rvc[3]); 
2'b01: aln_instr_vld_pre[2] = &unaln_instr_is_rvc[3:1]; 
default: aln_instr_vld_pre[2] = 1'b0; 
endcase 
end 
always_comb begin : aln_instr_dat2 
if (~|fetch_pc_reg[2:1] & instr1_loc_slot1 & unaln_instr_is_rvc[1]) begin 
aln_instr_dat[2] = mss_ifu_r_data[63:32]; 
aln_instr_offset[2] = unaln_instr_offset[2] ; 
end else begin 
aln_instr_dat[2] = {16'd0, mss_ifu_r_data[63:48]}; 
aln_instr_offset[2] = unaln_instr_offset[3] ; 
end 
end 
assign aln_instr_vld_pre[3] = ~|fetch_pc_reg[2:1] & 
instr1_loc_slot1 & (&unaln_instr_is_rvc[3:1]); 
assign aln_instr_dat[3] = {16'd0, mss_ifu_r_data[63:48]}; 
assign aln_instr_offset[3] = unaln_instr_offset[3]; 
end else begin: g_32bit_fetch_aln 
assign aln_instr_vld_pre[0] = ~(fetch_pc_reg[1] & unaln_instr_is_rvi[1]); 
always_comb begin : ALN_INSTR_DAT0 
if (~fetch_pc_reg[1]) begin 
if (res_buf_vld_q) begin 
aln_instr_dat[0] = {mss_ifu_r_data[15:0], res_buf_dat_q}; 
aln_instr_offset[0] = 2'b10; 
end else begin 
aln_instr_dat[0] = mss_ifu_r_data[31:0]; 
aln_instr_offset[0] = unaln_instr_offset[0]; 
end 
end else begin 
aln_instr_dat[0] = {16'd0, mss_ifu_r_data[31:16]}; 
aln_instr_offset[0] = unaln_instr_offset[1]; 
end 
end 
assign aln_instr_vld_pre[1] = ~fetch_pc_reg[1] & instr1_loc_slot1 & unaln_instr_is_rvc[1]; 
assign aln_instr_dat[1] = {16'd0, mss_ifu_r_data[31:16]}; 
assign aln_instr_offset[1] = unaln_instr_offset[1]; 
end 
if (64 == 64) begin: g_64bit_fetch_residue 
always_comb begin: res_buf_set_clr 
unique case (fetch_pc_reg[2:1]) 
2'b00: begin 
if (instr1_loc_slot1) begin 
res_buf_vld_set_pre = unaln_instr_is_rvi[3] & (unaln_instr_is_rvi[1] | &unaln_instr_is_rvc[2:1]); 
end else begin 
res_buf_vld_set_pre = unaln_instr_is_rvi[3] & unaln_instr_is_rvc[2]; 
end 
end 
2'b01: res_buf_vld_set_pre = unaln_instr_is_rvi[3] & (unaln_instr_is_rvi[1] | &unaln_instr_is_rvc[2:1]); 
2'b10: res_buf_vld_set_pre = unaln_instr_is_rvi[3] & unaln_instr_is_rvc[2]; 
2'b11: res_buf_vld_set_pre = unaln_instr_is_rvi[3]; 
default: res_buf_vld_set_pre = 'x; 
endcase 
res_buf_vld_clr_pre = ~|fetch_pc_reg[2:1] & ~(unaln_instr_is_rvi[3] & (unaln_instr_is_rvi[1] | &unaln_instr_is_rvc[2:1])); 
end 
end 
else begin:g_32bit_fetch_residue 
assign res_buf_vld_set_pre = unaln_instr_is_rvi[1] & (instr1_loc_slot1 | fetch_pc_reg[1]); 
assign res_buf_vld_clr_pre = ~fetch_pc_reg[1] & ~unaln_instr_is_rvi[1]; 
end 
assign res_buf_vld_set = res_buf_vld_set_pre & ifetch_rsp_vld ; 
assign res_buf_vld_clr = pcu_ifu_flush_vld | 
sp_pcgen_predict_vld | 
hv_success | 
(res_buf_vld_q & ifetch_rsp_vld & res_buf_vld_clr_pre); 
assign res_buf_vld_en = ifetch_rsp_vld | pcu_ifu_flush_vld | sp_pcgen_predict_vld | hv_success; 
assign res_buf_vld_d = res_buf_vld_set & ~res_buf_vld_clr; 
assign res_buf_dat_en = res_buf_vld_set; 
assign res_buf_dat_d = mss_ifu_r_data[64-1:64-16]; 
`WDFFER (res_buf_vld_q, res_buf_vld_d, res_buf_vld_en, clk, rst_n) 
`WDFFENR(res_buf_dat_q, res_buf_dat_d, res_buf_dat_en, clk) 
for (genvar i=0; i<FETCHSET_INSTR_N; i++) begin: g_predecode 
assign pdec_rvi_op[i] = aln_instr_dat[i][6:0]; 
assign pdec_rvc_op[i] = {aln_instr_dat[i][15:13], aln_instr_dat[i][1:0]}; 
assign pdec_rvi_rs1[i] = aln_instr_dat[i][19:15]; 
assign pdec_rvi_rd[i] = aln_instr_dat[i][11:7]; 
assign pdec_rvc_rs1[i] = aln_instr_dat[i][11:7]; 
assign pdec_rvi_offs[i] = pdec_rvi_op[i][2] ? {{12{aln_instr_dat[i][31]}}, aln_instr_dat[i][19:12], aln_instr_dat[i][20], aln_instr_dat[i][30:21], 1'b0} : 
(pdec_rvi_op[i][3] ? {{23{aln_instr_dat[i][29]}}, aln_instr_dat[i][28:25], aln_instr_dat[i][11:8], 1'b0} 
: {{20{aln_instr_dat[i][31]}}, aln_instr_dat[i][7], aln_instr_dat[i][30:25], aln_instr_dat[i][11:8], 1'b0}); 
assign pdec_rvc_offs[i] = pdec_rvc_op[i][3] 
? {{24{aln_instr_dat[i][12]}}, aln_instr_dat[i][6:5], 
aln_instr_dat[i][2], aln_instr_dat[i][11:10], aln_instr_dat[i][4:3], 1'b0} 
: {{21{aln_instr_dat[i][12]}}, aln_instr_dat[i][8], aln_instr_dat[i][10:9], 
aln_instr_dat[i][6], aln_instr_dat[i][7], aln_instr_dat[i][2], 
aln_instr_dat[i][11], aln_instr_dat[i][5:3], 1'b0}; 
assign pdec_rvi_branch[i] = (pdec_rvi_op[i] == 7'b110_0011) | ((pdec_rvi_op[i] == 7'b0001011) && (~&aln_instr_dat[i][14:13])); 
assign pdec_rvi_jal[i] = (pdec_rvi_op[i] == 7'b110_1111); 
assign pdec_rvi_jalr[i] = (pdec_rvi_op[i] == 7'b110_0111); 
assign pdec_rvi_call[i] = (pdec_rvi_jal[i] | pdec_rvi_jalr[i]) & ((pdec_rvi_rd[i] == 5'd1) | 
(pdec_rvi_rd[i] == 5'd5)); 
assign pdec_rvi_ret[i] = pdec_rvi_jalr[i] & ((pdec_rvi_rs1[i] == 5'd1) | 
(pdec_rvi_rs1[i] == 5'd5)) 
& (pdec_rvi_rs1[i] != pdec_rvi_rd[i]); 
assign pdec_rvi_jalr_x0[i] = pdec_rvi_jalr[i] & (pdec_rvi_rs1[i] == 5'd0); 
assign pdec_rvc_branch[i] = {pdec_rvc_op[i][4:3],pdec_rvc_op[i][1:0]} == 4'b1101; 
assign pdec_rvc_jr_jalr[i] = ((pdec_rvc_op[i] == 5'b10010) && ~|aln_instr_dat[i][6:2] && |aln_instr_dat[i][11:7]); 
assign pdec_rvc_jt_jalt[i] = (pdec_rvc_op[i] == 5'b10110) && ~|aln_instr_dat[i][12:10]; 
assign pdec_rvc_jalr[i] = (pdec_rvc_jr_jalr[i] & aln_instr_dat[i][12]); 
assign pdec_rvc_jr[i] = (pdec_rvc_jr_jalr[i] & ~aln_instr_dat[i][12]); 
assign pdec_rvc_jalt[i] = (pdec_rvc_jt_jalt[i] & (|aln_instr_dat[i][9:7])); 
assign pdec_rvc_j[i] = (pdec_rvc_op[i] == 5'b10101); 
assign pdec_rvc_jal[i] = (pdec_rvc_op[i] == 5'b00101); 
assign pdec_rvc_call[i] = pdec_rvc_jal[i] | pdec_rvc_jalr[i] | pdec_rvc_jalt[i]; 
assign pdec_rvc_ret[i] = pdec_rvc_jr[i] & ((pdec_rvc_rs1[i] == 5'd1) | (pdec_rvc_rs1[i] == 5'd5)) | 
pdec_rvc_jalr[i] & ((pdec_rvc_rs1[i] == 5'd5)) | 
pdec_rvc_popret[i]; 
assign pdec_rvc_popret[i] = (pdec_rvc_op[i] == 5'b10110) && ((aln_instr_dat[i][12:8] == 5'b11100) 
|| (aln_instr_dat[i][12:8] == 5'b11110)); 
assign pdec_is_branch[i] = aln_instr_vld[i] & (pdec_rvi_branch[i] | pdec_rvc_branch[i]); 
assign pdec_is_jal[i] = aln_instr_vld[i] & (pdec_rvi_jal[i] | pdec_rvc_j[i] | pdec_rvc_jal[i]); 
assign pdec_is_jalr[i] = aln_instr_vld[i] & (pdec_rvi_jalr[i] | pdec_rvc_jr_jalr[i]); 
assign pdec_is_jvt[i] = aln_instr_vld[i] & (pdec_rvc_jt_jalt[i]); 
assign pdec_is_call[i] = aln_instr_vld[i] & (pdec_rvi_call[i] | pdec_rvc_call[i]); 
assign pdec_is_ret[i] = aln_instr_vld[i] & (pdec_rvi_ret[i] | pdec_rvc_ret[i]); 
assign pdec_is_popret[i] = aln_instr_vld[i] & (pdec_rvc_popret[i]); 
assign pdec_is_jalr_x0[i]= aln_instr_vld[i] & pdec_rvi_jalr_x0[i]; 
assign pdec_is_rvc[i] = ~&aln_instr_dat[i][1:0]; 
assign pdec_bj_offs[i] = pdec_is_rvc[i] ? pdec_rvc_offs[i] : pdec_rvi_offs[i]; 
assign pdec_next_addr[i] = aln_instr_addr[i] + (pdec_is_rvc[i] ? 3'b010 : 3'b100); 
end 
for (genvar i=0; i<FETCHSET_INSTR_N; i++) begin: g_branch_taken 
if (BRANCH_FNTBT == 1) begin : g_branch_fntbt 
assign sp_branch_taken[i] = pdec_is_branch[i] & pdec_bj_offs[i][31]; 
end else begin : g_branch_ant 
assign sp_branch_taken[i] = 1'b0; 
end 
end 
assign sp_ctrl_tf = (sp_branch_taken| 
pdec_is_jal | 
pdec_is_jalr | 
pdec_is_popret | 
pdec_is_jvt) & {FETCHSET_INSTR_N{pcu_csr_sp_en && ~hv_flag_q[0]}}; 
assign sp_ctrl_tf_pre[0] = 1'b0; 
for (genvar i=1; i<FETCHSET_INSTR_N; i++) begin: g_tf_pre 
assign sp_ctrl_tf_pre[i] = sp_ctrl_tf_pre[i-1] | 
sp_ctrl_tf [i-1] ; 
end 
assign sp_ctrl_tf_mask = ~sp_ctrl_tf_pre; 
assign sp_ctrl_tf_ff1 = sp_ctrl_tf & sp_ctrl_tf_mask; 
assign sp_bj_taken = sp_ctrl_tf_ff1 & (sp_branch_taken | pdec_is_jal); 
assign sp_call_taken = sp_ctrl_tf_ff1 & pdec_is_call; 
assign sp_ret_taken = sp_ctrl_tf_ff1 & pdec_is_ret; 
assign sp_predict_taken = sp_bj_taken | sp_ret_taken; 
assign sp_predict_vld = |sp_predict_taken ; 
logic [31:0] sp_bj_offset; 
logic [31:0] sp_bj_base_addr; 
`WCBB_AOMUX(pdec_bj_offs , sp_bj_taken, sp_bj_offset) 
`WCBB_AOMUX(aln_instr_addr, sp_bj_taken, sp_bj_base_addr) 
assign sp_bj_addr = sp_bj_base_addr + sp_bj_offset; 
assign sp_predict_pc = ras_pop ? ras_addr_pop : 
sp_bj_addr; 
assign sp_pcgen_predict_vld = sp_predict_vld; 
assign sp_pcgen_predict_pc = sp_predict_pc; 
assign sp_jalr_stall_set = |((pdec_is_jalr & ~pdec_is_ret & sp_ctrl_tf_ff1) | 
(pdec_is_jvt & sp_ctrl_tf_ff1) ); 
assign sp_jalr_stall_clr = pcu_ifu_flush_vld; 
assign sp_jalr_stall_en = sp_jalr_stall_set | sp_jalr_stall_clr; 
assign sp_jalr_stall_d = sp_jalr_stall_set & ~sp_jalr_stall_clr; 
`WDFFER(sp_jalr_stall_q, sp_jalr_stall_d, sp_jalr_stall_en, clk, rst_n) 
assign ras_ptr_cmt_en = pcu_ifu_call_rslv ^ pcu_ifu_ret_rslv; 
assign ras_ptr_cmt_d = pcu_ifu_call_rslv ? (ras_ptr_cmt_q + 1'b1) : 
pcu_ifu_ret_rslv ? (ras_ptr_cmt_q - 1'b1) : 
ras_ptr_cmt_q; 
`WDFFER(ras_ptr_cmt_q, ras_ptr_cmt_d, ras_ptr_cmt_en, clk, rst_n) 
assign ras_push = |sp_call_taken; 
assign ras_pop = |sp_ret_taken; 
assign ras_ptr_cur_en = pcu_ifu_flush_vld | (ras_push ^ ras_pop); 
assign ras_ptr_cur_d = pcu_ifu_flush_vld ? ras_ptr_cmt_d : 
ras_push ? (ras_ptr_cur_q + 1'b1) : 
(ras_ptr_cur_q - 1'b1) ; 
`WDFFER(ras_ptr_cur_q, ras_ptr_cur_d, ras_ptr_cur_en, clk, rst_n) 
if (RAS_OPT_TIMING==1) begin : ras_ptr_opt_timing 
logic ras_ptr_next_en; 
logic [RAS_PTR_W-1:0] ras_ptr_next_d; 
logic [RAS_PTR_W-1:0] ras_ptr_next_q; 
assign ras_ptr_next_en = pcu_ifu_flush_vld | (ras_push ^ ras_pop); 
assign ras_ptr_next_d = pcu_ifu_flush_vld ? (ras_ptr_cmt_q + 1'b1) : 
ras_push ? (ras_ptr_next_q + 1'b1) : 
(ras_ptr_next_q - 1'b1) ; 
`WDFFERVAL(ras_ptr_next_q, ras_ptr_next_d, ras_ptr_next_en, clk, rst_n, {{RAS_PTR_W-1{1'b0}},1'b1}) 
assign ras_ptr_next = ras_ptr_next_q; 
end else begin : ras_ptr_norm 
assign ras_ptr_next = ras_ptr_cur_q + 1'b1; 
end 
assign ras_ptr_update = ras_pop ? ras_ptr_cur_q : ras_ptr_next; 
`WCBB_AOMUX(pdec_next_addr, sp_call_taken, ras_addr_push) 
for (genvar i=0; i<RAS_DEPTH; i++) begin : g_ras 
assign ras_buf_ent_en[i] = ~pcu_ifu_flush_vld & ras_push & (i == ras_ptr_update); 
assign ras_buf_ent_d[i] = ras_addr_push; 
`WDFFER(ras_buf_ent_q[i], ras_buf_ent_d[i], ras_buf_ent_en[i], clk, rst_n) 
end 
assign ras_addr_pop = ras_buf_ent_q[ras_ptr_cur_q]; 
assign brq_wr_en = ~pcu_ifu_flush_vld & ras_pop; 
assign brq_wr_data = sp_predict_pc[31:1]; 
assign brq_rd_en = pcu_ifu_brq_rd; 
assign ifu_pcu_brq_pc = {brq_rd_data, 1'b0}; 
assign brq_ifetch_stall = brq_full | 
brq_afull & ifetch_req_sent_q; 
assign brq_full = (brq_rd_addr == brq_wr_addr) && (brq_rd_ptr_rv ^ brq_wr_ptr_rv); 
assign brq_empty = (brq_rd_addr == brq_wr_addr) && (brq_rd_ptr_rv == brq_wr_ptr_rv); 
assign brq_wr_ptr_stand = (brq_wr_addr == (BRQ_DEPTH-1)) ? {~brq_wr_ptr_rv, {BRQ_ADDW{1'b0}}} : 
{brq_wr_ptr_rv, brq_wr_addr_next}; 
assign brq_afull = (brq_rd_ptr_q[BRQ_ADDW-1:0] == brq_wr_ptr_stand[BRQ_ADDW-1:0]) && 
(brq_rd_ptr_rv ^ brq_wr_ptr_stand[BRQ_ADDW]); 
assign brq_wr_ptr_en = (brq_wr_en && !brq_full ) || pcu_ifu_flush_vld; 
assign brq_rd_ptr_en = (brq_rd_en && !brq_empty) || pcu_ifu_flush_vld; 
assign brq_wr_ptr_d = pcu_ifu_flush_vld ? '0 : 
(brq_wr_addr == (BRQ_DEPTH-1)) ? {~brq_wr_ptr_rv, {BRQ_ADDW{1'b0}}} : 
{brq_wr_ptr_rv, brq_wr_addr_next}; 
`WDFFER(brq_wr_ptr_q, brq_wr_ptr_d, brq_wr_ptr_en, clk, rst_n) 
assign brq_rd_ptr_d = pcu_ifu_flush_vld ? '0 : 
(brq_rd_addr == (BRQ_DEPTH-1)) ? {~brq_rd_ptr_rv, {BRQ_ADDW{1'b0}}} : 
{brq_rd_ptr_rv, brq_rd_addr_next}; 
`WDFFER(brq_rd_ptr_q, brq_rd_ptr_d, brq_rd_ptr_en, clk, rst_n) 
assign brq_wr_addr = brq_wr_ptr_q[BRQ_ADDW-1:0]; 
assign brq_rd_addr = brq_rd_ptr_q[BRQ_ADDW-1:0]; 
assign brq_wr_ptr_rv = brq_wr_ptr_q[BRQ_ADDW]; 
assign brq_rd_ptr_rv = brq_rd_ptr_q[BRQ_ADDW]; 
assign brq_wr_addr_next = brq_wr_addr + 1'b1; 
assign brq_rd_addr_next = brq_rd_addr + 1'b1; 
for (genvar i=0; i<BRQ_DEPTH; i++) begin : g_brq_ent 
assign brq_ent_en[i] = brq_wr_ptr_en & (i==brq_wr_addr); 
`WDFFER (brq_ent_q[i], brq_wr_data, brq_ent_en[i], clk, rst_n) 
end 
always_comb begin 
brq_rd_data = {31{1'b0}}; 
for (int i=0; i<BRQ_DEPTH; i++) begin : g_brq_rdata 
brq_rd_data = brq_rd_data | ( {31{brq_rd_addr == i}} & brq_ent_q[i]); 
end 
end 
assign buf_wr_vld_mask = ~(excp_wr_buf_no | hv_flag_q[0]); 
assign ibuf_wr_vld[0] = dm_ifu_inst_vld | ((aln_instr_vld[0] | rsp_excp_vld) & ~excp_wr_buf_no); 
assign ibuf_wdata[0].instr = dm_ifu_inst_vld ? dm_ifu_inst_data : 
hv_flag_q[0] ? hv_data : aln_instr_dat[0]; 
assign ibuf_flush = pcu_ifu_flush_vld | hv_success; 
for (genvar i=1; i<FETCHSET_INSTR_N; i++) begin : g_ibuf_wdata 
assign ibuf_wr_vld[i] = aln_instr_vld[i] & sp_ctrl_tf_mask[i] & ~rsp_excp_vld & buf_wr_vld_mask; 
assign ibuf_wdata[i].instr = aln_instr_dat[i]; 
end 
for (genvar i=0; i<FETCHSET_INSTR_N; i++) begin : g_ibuf_winfo 
assign ibuf_wdata[i].pc_offset = aln_instr_addr[i][FETCHSET_OFFSET_W-1:1]; 
assign ibuf_wdata[i].excp_vld = rsp_excp_vld; 
assign ibuf_wdata[i].is_hv = dm_ifu_inst_vld ? 1'b0 : hv_flag_q[0]; 
end 
assign ibuf_wdata[0].pcbuf_addr = res_buf_vld_q ? pcbuf_wr_addr_prev : pcbuf_wr_addr; 
assign pop_pcbuf0_pre = ((aln_instr_addr[0][FETCHSET_OFFSET_W-1:1]==FETCHSET_INSTR_N-1) & (~&aln_instr_dat[0][1:0] | res_buf_vld_q)) | 
((aln_instr_addr[0][FETCHSET_OFFSET_W-1:1]==FETCHSET_INSTR_N-2) & (&aln_instr_dat[0][1:0])) ; 
assign ibuf_wdata[0].pop_pcbuf = sp_predict_taken[0] | 
rsp_excp_vld | 
pop_pcbuf0_pre; 
for (genvar i=1; i<FETCHSET_INSTR_N; i++) begin : g_ibuf_wdata_addr 
assign ibuf_wdata[i].pcbuf_addr= pcbuf_wr_addr; 
assign ibuf_wdata[i].pop_pcbuf = sp_predict_taken[i] | 
(aln_instr_addr[i][FETCHSET_OFFSET_W-1:1]==FETCHSET_INSTR_N-1) & (~&aln_instr_dat[i][1:0]) | 
(aln_instr_addr[i][FETCHSET_OFFSET_W-1:1]==FETCHSET_INSTR_N-2) & (&aln_instr_dat[i][1:0]); 
end 
assign ifetch_sent_offset = fetch_pc_reg[FETCHSET_OFFSET_W-1:0]; 
m130_instr_buf #( 
.T ( ibuf_data_t ), 
.I_PORTS_N ( FETCHSET_INSTR_N ), 
.O_PORTS_N ( 2 ), 
.FIFO_DEPTH ( 10 ), 
.OFFSET_W ( FETCHSET_OFFSET_W ) 
) u_instr_buf ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( ibuf_flush ), 
.wr_vld ( ibuf_wr_vld ), 
.wdata ( ibuf_wdata ), 
.rd_vld ( ibuf_rd_vld ), 
.rd_rdy ( ibuf_rd_rdy ), 
.rdata ( ibuf_rdata ), 
.mrf_1 ( ifu_pcu_instr_rs1_idx), 
.mrf_2 ( ifu_pcu_instr_rs2_idx), 
.mrf_1_vld ( ifu_pcu_instr_rs1_vld), 
.mrf_2_vld ( ifu_pcu_instr_rs2_vld), 
.core_priv_mode( core_priv_mode ), 
.pending_wr ( ifetch_req_sent_q ), 
.fetch_offset ( ifetch_sent_offset ), 
.stall ( ibuf_ifetch_stall ) 
); 
assign pcbuf_wr_no = (( ~|sp_ctrl_tf_mask[FETCHSET_INSTR_N-1:1]) & sp_ctrl_tf_mask[0]) & res_buf_vld_q; 
assign pcbuf_wr_en = ifetch_rsp_vld & (~pcbuf_wr_no ) | 
rsp_excp_vld & (~excp_wr_buf_no) ; 
assign pcbuf_wdata = fetch_pc_reg[31:FETCHSET_OFFSET_W]; 
for (genvar i=0; i<2; i++) begin : g_pcbuf_rd 
if (i==0) begin:g_issue_0_pcbuf_pop 
assign pcbuf_rd_en_vec[i] = (ifu_pcu_hdsk[i] & ibuf_rdata[i].pop_pcbuf) | hv_done; 
end 
else begin:g_issue_1_pcbuf_pop 
assign pcbuf_rd_en_vec[i] = (ifu_pcu_hdsk[i] & ibuf_rdata[i].pop_pcbuf); 
end 
assign pcbuf_rd_addr[i] = ibuf_rdata[i].pcbuf_addr; 
end 
assign pcbuf_ifetch_stall = (pcbuf_full | (pcbuf_afull & ifetch_req_sent_q)); 
m130_pc_buf #( 
.DATA_WIDTH ( 32-FETCHSET_OFFSET_W ), 
.FIFO_DEPTH ( 3 ), 
.O_PORTS_N ( 2 ) 
) u_pc_buf ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( pcu_ifu_flush_vld ), 
.wr_en ( pcbuf_wr_en ), 
.wdata ( pcbuf_wdata ), 
.rd_en_vec ( pcbuf_rd_en_vec ), 
.rd_addr ( pcbuf_rd_addr ), 
.rdata ( pcbuf_rdata ), 
.afull ( pcbuf_afull ), 
.full ( pcbuf_full ), 
.wr_addr ( pcbuf_wr_addr ), 
.wr_data_prev ( prev_fetchset_addr ), 
.wr_addr_prev ( pcbuf_wr_addr_prev ) 
); 
assign ifu_pcu_hdsk = ifu_pcu_vld & pcu_ifu_rdy; 
for (genvar i=0; i<2; i++) begin : g_ifu_pcu_instr 
assign ifu_pcu_vld[i] = ibuf_rd_vld[i] & ~(ibuf_rdata[i].is_hv); 
assign ibuf_rd_rdy[i] = pcu_ifu_rdy[i]; 
assign ifu_pcu_instr_data[i] = ibuf_rdata[i].instr; 
assign ifu_pcu_instr_pc[i] = {pcbuf_rdata[i], ibuf_rdata[i].pc_offset, 1'b0}; 
assign ifu_pcu_excp_vld[i] = ibuf_rdata[i].excp_vld; 
end 
endmodule
 
 
module m130_ifu_dec( 
input logic [31:0] instr_data , 
input logic [1:0] core_priv_mode , 
output logic inst_type, 
output logic [127:0] instr_name, 
output logic [4:0] mrf_1, 
output logic [4:0] mrf_2, 
output logic mrf_1_vld, 
output logic mrf_2_vld 
); 
 
 
typedef enum logic[1:0] { 
PRIV_MODE_M = 2'b11, 
PRIV_MODE_S = 2'b01, 
PRIV_MODE_U = 2'b00 
} priv_mode_e; 
typedef enum logic[1:0] { 
CSR_OP_READ = 2'b00, 
CSR_OP_WRITE = 2'b01, 
CSR_OP_SET = 2'b10, 
CSR_OP_CLEAR = 2'b11 
} csr_opcode_e; 
typedef enum logic [11:0] { 
CSR_FFLAGS_ADDR = 12'h001, 
CSR_FRM_ADDR = 12'h002, 
CSR_FCSR_ADDR = 12'h003, 
CSR_FTRAN_ADDR = 12'h800, 
CSR_SSTATUS_ADDR = 12'h100, 
CSR_SIE_ADDR = 12'h104, 
CSR_STVEC_ADDR = 12'h105, 
CSR_SCOUNTEREN_ADDR = 12'h106, 
CSR_SSCRATCH_ADDR = 12'h140, 
CSR_SEPC_ADDR = 12'h141, 
CSR_SCAUSE_ADDR = 12'h142, 
CSR_STVAL_ADDR = 12'h143, 
CSR_SIP_ADDR = 12'h144, 
CSR_SATP_ADDR = 12'h180, 
CSR_MSTATUS_ADDR = 12'h300, 
CSR_MISA_ADDR = 12'h301, 
CSR_MEDELEG_ADDR = 12'h302, 
CSR_MIDELEG_ADDR = 12'h303, 
CSR_MIE_ADDR = 12'h304, 
CSR_MTVEC_ADDR = 12'h305, 
CSR_MCOUNTEREN_ADDR = 12'h306, 
CSR_MSTATUSH_ADDR = 12'h310, 
CSR_MSCRATCH_ADDR = 12'h340, 
CSR_MEPC_ADDR = 12'h341, 
CSR_MCAUSE_ADDR = 12'h342, 
CSR_MTVAL_ADDR = 12'h343, 
CSR_MIP_ADDR = 12'h344, 
CSR_MENVCFG_ADDR = 12'h30A, 
CSR_MENVCFGH_ADDR = 12'h31A, 
CSR_MSECCFG_ADDR = 12'h747, 
CSR_MSECCFGH_ADDR = 12'h757, 
CSR_PMPCFG0_ADDR = 12'h3A0, 
CSR_PMPCFG1_ADDR = 12'h3A1, 
CSR_PMPCFG2_ADDR = 12'h3A2, 
CSR_PMPCFG3_ADDR = 12'h3A3, 
CSR_PMPADDR0_ADDR = 12'h3B0, 
CSR_PMPADDR1_ADDR = 12'h3B1, 
CSR_PMPADDR2_ADDR = 12'h3B2, 
CSR_PMPADDR3_ADDR = 12'h3B3, 
CSR_PMPADDR4_ADDR = 12'h3B4, 
CSR_PMPADDR5_ADDR = 12'h3B5, 
CSR_PMPADDR6_ADDR = 12'h3B6, 
CSR_PMPADDR7_ADDR = 12'h3B7, 
CSR_PMPADDR8_ADDR = 12'h3B8, 
CSR_PMPADDR9_ADDR = 12'h3B9, 
CSR_PMPADDR10_ADDR = 12'h3BA, 
CSR_PMPADDR11_ADDR = 12'h3BB, 
CSR_PMPADDR12_ADDR = 12'h3BC, 
CSR_PMPADDR13_ADDR = 12'h3BD, 
CSR_PMPADDR14_ADDR = 12'h3BE, 
CSR_PMPADDR15_ADDR = 12'h3BF, 
CSR_MVENDORID_ADDR = 12'hF11, 
CSR_MARCHID_ADDR = 12'hF12, 
CSR_MIMPID_ADDR = 12'hF13, 
CSR_MHARTID_ADDR = 12'hF14, 
CSR_MCONFIGPTRID_ADDR = 12'hF15, 
CSR_MCYCLE_ADDR = 12'hB00, 
CSR_MCYCLEH_ADDR = 12'hB80, 
CSR_MINSTRET_ADDR = 12'hB02, 
CSR_MINSTRETH_ADDR = 12'hB82, 
CSR_MHPM_COUNTER_3_ADDR = 12'hB03, 
CSR_MHPM_COUNTER_4_ADDR = 12'hB04, 
CSR_MHPM_COUNTER_5_ADDR = 12'hB05, 
CSR_MHPM_COUNTER_6_ADDR = 12'hB06, 
CSR_MHPM_COUNTER_7_ADDR = 12'hB07, 
CSR_MHPM_COUNTER_8_ADDR = 12'hB08, 
CSR_MHPM_COUNTER_9_ADDR = 12'hB09, 
CSR_MHPM_COUNTER_10_ADDR = 12'hB0A, 
CSR_MHPM_COUNTER_11_ADDR = 12'hB0B, 
CSR_MHPM_COUNTER_12_ADDR = 12'hB0C, 
CSR_MHPM_COUNTER_13_ADDR = 12'hB0D, 
CSR_MHPM_COUNTER_14_ADDR = 12'hB0E, 
CSR_MHPM_COUNTER_15_ADDR = 12'hB0F, 
CSR_MHPM_COUNTER_16_ADDR = 12'hB10, 
CSR_MHPM_COUNTER_17_ADDR = 12'hB11, 
CSR_MHPM_COUNTER_18_ADDR = 12'hB12, 
CSR_MHPM_COUNTER_19_ADDR = 12'hB13, 
CSR_MHPM_COUNTER_20_ADDR = 12'hB14, 
CSR_MHPM_COUNTER_21_ADDR = 12'hB15, 
CSR_MHPM_COUNTER_22_ADDR = 12'hB16, 
CSR_MHPM_COUNTER_23_ADDR = 12'hB17, 
CSR_MHPM_COUNTER_24_ADDR = 12'hB18, 
CSR_MHPM_COUNTER_25_ADDR = 12'hB19, 
CSR_MHPM_COUNTER_26_ADDR = 12'hB1A, 
CSR_MHPM_COUNTER_27_ADDR = 12'hB1B, 
CSR_MHPM_COUNTER_28_ADDR = 12'hB1C, 
CSR_MHPM_COUNTER_29_ADDR = 12'hB1D, 
CSR_MHPM_COUNTER_30_ADDR = 12'hB1E, 
CSR_MHPM_COUNTER_31_ADDR = 12'hB1F, 
CSR_MHPM_COUNTER_3H_ADDR = 12'hB83, 
CSR_MHPM_COUNTER_4H_ADDR = 12'hB84, 
CSR_MHPM_COUNTER_5H_ADDR = 12'hB85, 
CSR_MHPM_COUNTER_6H_ADDR = 12'hB86, 
CSR_MHPM_COUNTER_7H_ADDR = 12'hB87, 
CSR_MHPM_COUNTER_8H_ADDR = 12'hB88, 
CSR_MHPM_COUNTER_9H_ADDR = 12'hB89, 
CSR_MHPM_COUNTER_10H_ADDR = 12'hB8A, 
CSR_MHPM_COUNTER_11H_ADDR = 12'hB8B, 
CSR_MHPM_COUNTER_12H_ADDR = 12'hB8C, 
CSR_MHPM_COUNTER_13H_ADDR = 12'hB8D, 
CSR_MHPM_COUNTER_14H_ADDR = 12'hB8E, 
CSR_MHPM_COUNTER_15H_ADDR = 12'hB8F, 
CSR_MHPM_COUNTER_16H_ADDR = 12'hB90, 
CSR_MHPM_COUNTER_17H_ADDR = 12'hB91, 
CSR_MHPM_COUNTER_18H_ADDR = 12'hB92, 
CSR_MHPM_COUNTER_19H_ADDR = 12'hB93, 
CSR_MHPM_COUNTER_20H_ADDR = 12'hB94, 
CSR_MHPM_COUNTER_21H_ADDR = 12'hB95, 
CSR_MHPM_COUNTER_22H_ADDR = 12'hB96, 
CSR_MHPM_COUNTER_23H_ADDR = 12'hB97, 
CSR_MHPM_COUNTER_24H_ADDR = 12'hB98, 
CSR_MHPM_COUNTER_25H_ADDR = 12'hB99, 
CSR_MHPM_COUNTER_26H_ADDR = 12'hB9A, 
CSR_MHPM_COUNTER_27H_ADDR = 12'hB9B, 
CSR_MHPM_COUNTER_28H_ADDR = 12'hB9C, 
CSR_MHPM_COUNTER_29H_ADDR = 12'hB9D, 
CSR_MHPM_COUNTER_30H_ADDR = 12'hB9E, 
CSR_MHPM_COUNTER_31H_ADDR = 12'hB9F, 
CSR_MCOUNTINHIBIT_ADDR = 12'h320, 
CSR_MHPM_EVENT_3_ADDR = 12'h323, 
CSR_MHPM_EVENT_4_ADDR = 12'h324, 
CSR_MHPM_EVENT_5_ADDR = 12'h325, 
CSR_MHPM_EVENT_6_ADDR = 12'h326, 
CSR_MHPM_EVENT_7_ADDR = 12'h327, 
CSR_MHPM_EVENT_8_ADDR = 12'h328, 
CSR_MHPM_EVENT_9_ADDR = 12'h329, 
CSR_MHPM_EVENT_10_ADDR = 12'h32a, 
CSR_MHPM_EVENT_11_ADDR = 12'h32b, 
CSR_MHPM_EVENT_12_ADDR = 12'h32c, 
CSR_MHPM_EVENT_13_ADDR = 12'h32d, 
CSR_MHPM_EVENT_14_ADDR = 12'h32e, 
CSR_MHPM_EVENT_15_ADDR = 12'h32f, 
CSR_MHPM_EVENT_16_ADDR = 12'h330, 
CSR_MHPM_EVENT_17_ADDR = 12'h331, 
CSR_MHPM_EVENT_18_ADDR = 12'h332, 
CSR_MHPM_EVENT_19_ADDR = 12'h333, 
CSR_MHPM_EVENT_20_ADDR = 12'h334, 
CSR_MHPM_EVENT_21_ADDR = 12'h335, 
CSR_MHPM_EVENT_22_ADDR = 12'h336, 
CSR_MHPM_EVENT_23_ADDR = 12'h337, 
CSR_MHPM_EVENT_24_ADDR = 12'h338, 
CSR_MHPM_EVENT_25_ADDR = 12'h339, 
CSR_MHPM_EVENT_26_ADDR = 12'h33a, 
CSR_MHPM_EVENT_27_ADDR = 12'h33b, 
CSR_MHPM_EVENT_28_ADDR = 12'h33c, 
CSR_MHPM_EVENT_29_ADDR = 12'h33d, 
CSR_MHPM_EVENT_30_ADDR = 12'h33e, 
CSR_MHPM_EVENT_31_ADDR = 12'h33f, 
CSR_MTVT_ADDR = 12'h307, 
CSR_MNXTI_ADDR = 12'h345, 
CSR_MINTSTATUS_ADDR = 12'hfb1, 
CSR_MINTTHRESH_ADDR = 12'h347, 
CSR_MSCRATCHCSW_ADDR = 12'h348, 
CSR_MSCRATCHCSWL_ADDR = 12'h349, 
CSR_MNSCRATCH_ADDR = 12'h740, 
CSR_MNEPC_ADDR = 12'h741, 
CSR_MNCAUSE_ADDR = 12'h742, 
CSR_MNSTATUS_ADDR = 12'h744, 
CSR_MNVEC_ADDR = 12'hFC0, 
CSR_TSELECT_ADDR = 12'h7A0, 
CSR_TDATA1_ADDR = 12'h7A1, 
CSR_TDATA2_ADDR = 12'h7A2, 
CSR_TDATA3_ADDR = 12'h7A3, 
CSR_TINFO_ADDR = 12'h7A4, 
CSR_TCONTROL_ADDR = 12'h7A5, 
CSR_DCSR_ADDR = 12'h7b0, 
CSR_DPC_ADDR = 12'h7b1, 
CSR_DSCRATCH0_ADDR = 12'h7b2, 
CSR_DSCRATCH1_ADDR = 12'h7b3, 
CSR_DSCRATCH2_ADDR = 12'h7c0, 
CSR_DSCRATCH3_ADDR = 12'h7c1, 
CSR_MCONTEXTSW_ADDR = 12'h7c2, 
CSR_MSECURFEAT_ADDR = 12'h7c3, 
CSR_MLPCFG_ADDR = 12'h7c4, 
CSR_CYCLE_ADDR = 12'hC00, 
CSR_CYCLEH_ADDR = 12'hC80, 
CSR_TIME_ADDR = 12'hC01, 
CSR_TIMEH_ADDR = 12'hC81, 
CSR_INSTRET_ADDR = 12'hC02, 
CSR_INSTRETH_ADDR = 12'hC82, 
CSR_HPM_COUNTER_3_ADDR = 12'hC03, 
CSR_HPM_COUNTER_4_ADDR = 12'hC04, 
CSR_HPM_COUNTER_5_ADDR = 12'hC05, 
CSR_HPM_COUNTER_6_ADDR = 12'hC06, 
CSR_HPM_COUNTER_7_ADDR = 12'hC07, 
CSR_HPM_COUNTER_8_ADDR = 12'hC08, 
CSR_HPM_COUNTER_9_ADDR = 12'hC09, 
CSR_HPM_COUNTER_10_ADDR = 12'hC0A, 
CSR_HPM_COUNTER_11_ADDR = 12'hC0B, 
CSR_HPM_COUNTER_12_ADDR = 12'hC0C, 
CSR_HPM_COUNTER_13_ADDR = 12'hC0D, 
CSR_HPM_COUNTER_14_ADDR = 12'hC0E, 
CSR_HPM_COUNTER_15_ADDR = 12'hC0F, 
CSR_HPM_COUNTER_16_ADDR = 12'hC10, 
CSR_HPM_COUNTER_17_ADDR = 12'hC11, 
CSR_HPM_COUNTER_18_ADDR = 12'hC12, 
CSR_HPM_COUNTER_19_ADDR = 12'hC13, 
CSR_HPM_COUNTER_20_ADDR = 12'hC14, 
CSR_HPM_COUNTER_21_ADDR = 12'hC15, 
CSR_HPM_COUNTER_22_ADDR = 12'hC16, 
CSR_HPM_COUNTER_23_ADDR = 12'hC17, 
CSR_HPM_COUNTER_24_ADDR = 12'hC18, 
CSR_HPM_COUNTER_25_ADDR = 12'hC19, 
CSR_HPM_COUNTER_26_ADDR = 12'hC1A, 
CSR_HPM_COUNTER_27_ADDR = 12'hC1B, 
CSR_HPM_COUNTER_28_ADDR = 12'hC1C, 
CSR_HPM_COUNTER_29_ADDR = 12'hC1D, 
CSR_HPM_COUNTER_30_ADDR = 12'hC1E, 
CSR_HPM_COUNTER_31_ADDR = 12'hC1F, 
CSR_HPM_COUNTER_3H_ADDR = 12'hC83, 
CSR_HPM_COUNTER_4H_ADDR = 12'hC84, 
CSR_HPM_COUNTER_5H_ADDR = 12'hC85, 
CSR_HPM_COUNTER_6H_ADDR = 12'hC86, 
CSR_HPM_COUNTER_7H_ADDR = 12'hC87, 
CSR_HPM_COUNTER_8H_ADDR = 12'hC88, 
CSR_HPM_COUNTER_9H_ADDR = 12'hC89, 
CSR_HPM_COUNTER_10H_ADDR = 12'hC8A, 
CSR_HPM_COUNTER_11H_ADDR = 12'hC8B, 
CSR_HPM_COUNTER_12H_ADDR = 12'hC8C, 
CSR_HPM_COUNTER_13H_ADDR = 12'hC8D, 
CSR_HPM_COUNTER_14H_ADDR = 12'hC8E, 
CSR_HPM_COUNTER_15H_ADDR = 12'hC8F, 
CSR_HPM_COUNTER_16H_ADDR = 12'hC90, 
CSR_HPM_COUNTER_17H_ADDR = 12'hC91, 
CSR_HPM_COUNTER_18H_ADDR = 12'hC92, 
CSR_HPM_COUNTER_19H_ADDR = 12'hC93, 
CSR_HPM_COUNTER_20H_ADDR = 12'hC94, 
CSR_HPM_COUNTER_21H_ADDR = 12'hC95, 
CSR_HPM_COUNTER_22H_ADDR = 12'hC96, 
CSR_HPM_COUNTER_23H_ADDR = 12'hC97, 
CSR_HPM_COUNTER_24H_ADDR = 12'hC98, 
CSR_HPM_COUNTER_25H_ADDR = 12'hC99, 
CSR_HPM_COUNTER_26H_ADDR = 12'hC9A, 
CSR_HPM_COUNTER_27H_ADDR = 12'hC9B, 
CSR_HPM_COUNTER_28H_ADDR = 12'hC9C, 
CSR_HPM_COUNTER_29H_ADDR = 12'hC9D, 
CSR_HPM_COUNTER_30H_ADDR = 12'hC9E, 
CSR_HPM_COUNTER_31H_ADDR = 12'hC9F, 
CSR_JVT_ADDR = 12'h017 
} csr_reg_t; 
 
logic [2:0] mrf_1_vld_pre; 
logic [1:0] mrf_2_vld_pre; 
always_comb begin 
instr_name = "ILL"; 
mrf_1 = instr_data[19:15]; 
mrf_2 = instr_data[24:20]; 
mrf_1_vld_pre = 3'b0; 
mrf_2_vld_pre = 2'b0; 
casez(instr_data[31:0]) 
32'b???????_?????_?????_???_?????_0110111 : begin 
instr_name = "LUI"; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_???_?????_0010111 : begin 
instr_name = "AUIPC"; 
mrf_1_vld_pre = 3'b100; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_???_?????_1101111 : begin 
instr_name = "JAL"; 
mrf_1_vld_pre = 3'b100; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_000_?????_1100111 : begin 
instr_name = "JALR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_000_?????_1100011 : begin 
instr_name = "BEQ"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_001_?????_1100011 : begin 
instr_name = "BNE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_100_?????_1100011 : begin 
instr_name = "BLT"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_101_?????_1100011 : begin 
instr_name = "BGE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_110_?????_1100011 : begin 
instr_name = "BLTU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_111_?????_1100011 : begin 
instr_name = "BGEU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_000_?????_0000011 : begin 
instr_name = "LB"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_001_?????_0000011 : begin 
instr_name = "LH"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_010_?????_0000011 : begin 
instr_name = "LW"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_100_?????_0000011 : begin 
instr_name = "LBU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_101_?????_0000011 : begin 
instr_name = "LHU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_000_?????_0100011 : begin 
instr_name = "SB"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_001_?????_0100011 : begin 
instr_name = "SH"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_010_?????_0100011 : begin 
instr_name = "SW"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_000_?????_0010011 : begin 
instr_name = "ADDI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_010_?????_0010011 : begin 
instr_name = "SLTI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_011_?????_0010011 : begin 
instr_name = "SLTIU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_100_?????_0010011 : begin 
instr_name = "XORI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_110_?????_0010011 : begin 
instr_name = "ORI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_111_?????_0010011 : begin 
instr_name = "ANDI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000000_?????_?????_001_?????_0010011 : begin 
instr_name = "SLLI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000000_?????_?????_101_?????_0010011 : begin 
instr_name = "SRLI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0100000_?????_?????_101_?????_0010011 : begin 
instr_name = "SRAI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000000_?????_?????_000_?????_0110011 : begin 
instr_name = "ADD"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100000_?????_?????_000_?????_0110011 : begin 
instr_name = "SUB"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_001_?????_0110011 : begin 
instr_name = "SLL"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_010_?????_0110011 : begin 
instr_name = "SLT"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_011_?????_0110011 : begin 
instr_name = "SLTU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_100_?????_0110011 : begin 
instr_name = "XOR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_101_?????_0110011 : begin 
instr_name = "SRL"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100000_?????_?????_101_?????_0110011 : begin 
instr_name = "SRA"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_110_?????_0110011 : begin 
instr_name = "OR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000000_?????_?????_111_?????_0110011 : begin 
instr_name = "AND"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_000_?????_0001111 : begin 
instr_name = "FENCE"; 
end 
32'b0000000_00000_?????_000_?????_1110011 : begin 
instr_name = "ECALL"; 
end 
32'b0000000_00001_?????_000_?????_1110011 : begin 
instr_name = "EBREAK"; 
end 
32'b0011000_00010_?????_000_?????_1110011 : begin 
instr_name = "MRET"; 
end 
32'b0001000_00101_?????_000_?????_1110011 : begin 
instr_name = "WFI"; 
end 
32'b???????_?????_?????_001_?????_0001111 : begin 
instr_name = "Fence.I"; 
end 
32'b???????_?????_?????_001_?????_1110011 : begin 
mrf_1 = (instr_data[31:20] == CSR_MCONTEXTSW_ADDR) && (&core_priv_mode) ? {instr_data[19:17],~instr_data[16],instr_data[15]} 
: instr_data[19:15] ; 
instr_name = "CSRRW"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_010_?????_1110011 : begin 
mrf_1 = (instr_data[31:20] == CSR_MCONTEXTSW_ADDR) && (&core_priv_mode) ? {instr_data[19:17],~instr_data[16],instr_data[15]} 
: instr_data[19:15] ; 
instr_name = "CSRRS"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_011_?????_1110011 : begin 
instr_name = "CSRRC"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b???????_?????_?????_101_?????_1110011 : begin 
instr_name = "CSRRWI"; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_110_?????_1110011 : begin 
instr_name = "CSRRSI"; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_111_?????_1110011 : begin 
instr_name = "CSRRCI"; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000001_?????_?????_000_?????_0110011 : begin 
instr_name = "MUL"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_001_?????_0110011 : begin 
instr_name = "MULH"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_010_?????_0110011 : begin 
instr_name = "MULHSU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_011_?????_0110011 : begin 
instr_name = "MULHU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_100_?????_0110011 : begin 
instr_name = "DIV"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_101_?????_0110011 : begin 
instr_name = "DIVU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_110_?????_0110011 : begin 
instr_name = "REM"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000001_?????_?????_111_?????_0110011 : begin 
instr_name = "REMU"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100000_?????_?????_111_?????_0110011 : begin 
instr_name = "andn"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100100_?????_?????_001_?????_0110011 : begin 
instr_name = "bclr"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100100_?????_?????_001_?????_0010011 : begin 
instr_name = "bclri"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0100100_?????_?????_101_?????_0110011 : begin 
instr_name = "bext"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100100_?????_?????_101_?????_0010011 : begin 
instr_name = "bexti"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0110100_?????_?????_001_?????_0110011 : begin 
instr_name = "binv"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0110100_?????_?????_001_?????_0010011 : begin 
instr_name = "binvi"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0010100_?????_?????_001_?????_0110011 : begin 
instr_name = "bset"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0010100_?????_?????_001_?????_0010011 : begin 
instr_name = "bseti"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000101_?????_?????_001_?????_0110011 : begin 
instr_name = "clmul"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000101_?????_?????_011_?????_0110011 : begin 
instr_name = "clmulh"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000101_?????_?????_010_?????_0110011 : begin 
instr_name = "clmulr"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0110000_00000_?????_001_?????_0010011 : begin 
instr_name = "clz"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0110000_00010_?????_001_?????_0010011 : begin 
instr_name = "cpop"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0110000_00001_?????_001_?????_0010011 : begin 
instr_name = "ctz"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0000101_?????_?????_110_?????_0110011 : begin 
instr_name = "max"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000101_?????_?????_111_?????_0110011 : begin 
instr_name = "maxu"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000101_?????_?????_100_?????_0110011 : begin 
instr_name = "min"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000101_?????_?????_101_?????_0110011 : begin 
instr_name = "minu"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0010100_00111_?????_101_?????_0010011 : begin 
instr_name = "orc.b"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0100000_?????_?????_110_?????_0110011 : begin 
instr_name = "orn"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0110100_11000_?????_101_?????_0010011 : begin 
instr_name = "rev8"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0110000_?????_?????_001_?????_0110011 : begin 
instr_name = "rol"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0110000_?????_?????_101_?????_0110011 : begin 
instr_name = "ror"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0110000_?????_?????_101_?????_0010011 : begin 
instr_name = "rori"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0110000_00100_?????_001_?????_0010011 : begin 
instr_name = "sext.b"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0110000_00101_?????_001_?????_0010011 : begin 
instr_name = "sext.h"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b0010000_?????_?????_010_?????_0110011 : begin 
instr_name = "sh1add"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0010000_?????_?????_100_?????_0110011 : begin 
instr_name = "sh2add"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0010000_?????_?????_110_?????_0110011 : begin 
instr_name = "sh3add"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100000_?????_?????_100_?????_0110011 : begin 
instr_name = "xnor"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000100_00000_?????_100_?????_0110011 : begin 
instr_name = "zext.h"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b????_????_????_????_000_???_???_??_???_00 : begin 
instr_name = "C.ADDI4SPN"; 
mrf_1 = 5'b10; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_010_???_???_??_???_00 : begin 
instr_name = "C.LW"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_110_???_???_??_???_00 : begin 
instr_name = "C.SW"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_000_???_???_??_???_01 : begin 
instr_name = "C.ADDI"; 
mrf_1 = instr_data[11:7]; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_001_???_???_??_???_01 : begin 
instr_name = "C.JAL"; 
mrf_1_vld_pre = 3'b100; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_010_???_???_??_???_01 : begin 
instr_name = "C.LI"; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_011_???_???_??_???_01 : begin 
if (instr_data[11:7] ==5'h2) begin 
instr_name = "C.ADDI16SP"; 
mrf_1 = 5'b10; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end else begin 
instr_name = "C.LUI"; 
mrf_2_vld_pre = 2'b10; 
end 
end 
32'b????_????_????_????_100_000_???_??_???_01 : begin 
instr_name = "C.SRLI"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_100_001_???_??_???_01 : begin 
instr_name = "C.SRAI"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_100_?10_???_??_???_01 : begin 
instr_name = "C.ANDI"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_100_011_???_00_???_01 : begin 
instr_name = "C.SUB"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????_????_????_????_100_011_???_01_???_01 : begin 
instr_name = "C.XOR"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????_????_????_????_100_011_???_10_???_01 : begin 
instr_name = "C.OR"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????_????_????_????_100_011_???_11_???_01 : begin 
instr_name = "C.AND"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????_????_????_????_101_???_???_??_???_01 : begin 
instr_name = "C.J"; 
mrf_1_vld_pre = 3'b100; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_110_???_???_??_???_01 : begin 
instr_name = "C.BEQZ"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
end 
32'b????_????_????_????_111_???_???_??_???_01 : begin 
instr_name = "C.BNEZ"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
end 
32'b????_????_????_????_000_0??_???_??_???_10 : begin 
instr_name = "C.SLLI"; 
mrf_1 = instr_data[11:7]; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_010_???_???_??_???_10 : begin 
instr_name = "C.LWSP"; 
mrf_1 = 5'b10; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????_????_????_????_100_0??_???_??_???_10 : begin 
if (instr_data[6:2] == 5'b0) begin 
instr_name = "C.JR"; 
mrf_1 = instr_data[11:7]; 
mrf_1_vld_pre = 3'b001; 
end else begin 
instr_name = "C.MV"; 
mrf_2 = instr_data[6:2]; 
mrf_2_vld_pre = 2'b01; 
end 
end 
32'b????_????_????_????_100_1??_???_??_???_10 : begin 
if (instr_data[11:2]==10'b0) begin 
instr_name = "C.EBREAK"; 
end else if (instr_data[6:2]==5'b0) begin 
instr_name = "C.JALR"; 
mrf_1 = instr_data[11:7]; 
mrf_1_vld_pre = 3'b001; 
end else begin 
instr_name = "C.ADD"; 
mrf_1 = instr_data[11:7]; 
mrf_2 = instr_data[6:2]; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
end 
32'b????_????_????_????_110_???_???_??_???_10 : begin 
instr_name = "C.SWSP"; 
mrf_1 = 5'b10; 
mrf_2 = instr_data[6:2]; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000111_?????_?????_101_?????_0110011 : begin 
instr_name = "czero.eqz"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000111_?????_?????_111_?????_0110011 : begin 
instr_name = "czero.nez"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????????????????_100_000_???_??_???_00 : begin 
instr_name = "c.lbu"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_001_???_1?_???_00 : begin 
instr_name = "c.lh"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_001_???_0?_???_00 : begin 
instr_name = "c.lhu"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_010_???_??_???_00 : begin 
instr_name = "c.sb"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_011_???_0?_???_00 : begin 
instr_name = "c.sh"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_111_???_11_000_01 : begin 
instr_name = "c.zext.b"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b00; 
end 
32'b????????????????_100_111_???_11_101_01 : begin 
instr_name = "c.not"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_111_???_10_???_01 : begin 
instr_name = "c.mul"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_2 = {2'b01,instr_data[4:2]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????????????????_101_110_00?_??_???_10 : begin 
instr_name = "cm.push"; 
mrf_1 = 5'h2; 
mrf_2 = 5'h1; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_101_110_10?_??_???_10 : begin 
instr_name = "cm.pop"; 
mrf_1 = 5'h2; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_101_111_10?_??_???_10 : begin 
instr_name = "cm.popret"; 
mrf_1 = 5'h2; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_101_111_00?_??_???_10 : begin 
instr_name = "cm.popretz"; 
mrf_1 = 5'h2; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_101_011_???_11_???_10 : begin 
instr_name = "cm.mva01s"; 
mrf_2 = {|instr_data[9:8],instr_data[9:8]==0,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b000; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????????????????_101_011_???_01_???_10 : begin 
instr_name = "cm.mvsa01"; 
mrf_2 = 5'ha; 
mrf_1_vld_pre = 3'b000; 
mrf_2_vld_pre = 2'b01; 
end 
32'b????????????????_101_000_???_??_???_10 : begin 
instr_name = "cm.jvt"; 
mrf_1_vld_pre = 3'b000; 
mrf_2_vld_pre = 2'b10; 
end 
32'b????????????????_100_111_???_11_001_01 : begin 
instr_name = "c.sext.b"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b00; 
end 
32'b????????????????_100_111_???_11_010_01 : begin 
instr_name = "c.zext.h"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b00; 
end 
32'b????????????????_100_111_???_11_011_01 : begin 
instr_name = "c.sext.h"; 
mrf_1 = {2'b01,instr_data[9:7]}; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b00; 
end 
32'b??_?????_?????_?????_000_?????_0101011 : begin 
instr_name = "ADDSHF"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b??_?????_?????_?????_001_?????_0101011 : begin 
instr_name = "SUBSHF"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b??_?????_?????_?????_100_?????_0101011 : begin 
instr_name = "ANDSHF"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b??_?????_?????_?????_011_?????_0101011 : begin 
instr_name = "XORSHF"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b??_?????_?????_?????_010_?????_0101011 : begin 
instr_name = "ORSHF"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b???????_?????_?????_000_?????_0001011 : begin 
instr_name = "BEQI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_011_?????_0001011 : begin 
instr_name = "BGEI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_101_?????_0001011 : begin 
instr_name = "BGEUI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_010_?????_0001011 : begin 
instr_name = "BLTI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_100_?????_0001011 : begin 
instr_name = "BLTUI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b???????_?????_?????_001_?????_0001011 : begin 
instr_name = "BNEI"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0000_????????_?????_111_?????_0001011 : begin 
instr_name = "LB.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1000_????????_?????_111_?????_0001011 : begin 
instr_name = "LB.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111000_?????_?????_111_?????_0001011 : begin 
instr_name = "LBR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011000_?????_?????_111_?????_0001011 : begin 
instr_name = "LBR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011000_?????_?????_111_?????_0001011 : begin 
instr_name = "LBR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0100_????????_?????_111_?????_0001011 : begin 
instr_name = "LBU.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1100_????????_?????_111_?????_0001011 : begin 
instr_name = "LBU.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111100_?????_?????_111_?????_0001011 : begin 
instr_name = "LBUR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011100_?????_?????_111_?????_0001011 : begin 
instr_name = "LBUR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011100_?????_?????_111_?????_0001011 : begin 
instr_name = "LBUR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0001_????????_?????_111_?????_0001011 : begin 
instr_name = "LH.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1001_????????_?????_111_?????_0001011 : begin 
instr_name = "LH.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111001_?????_?????_111_?????_0001011 : begin 
instr_name = "LHR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011001_?????_?????_111_?????_0001011 : begin 
instr_name = "LHR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011001_?????_?????_111_?????_0001011 : begin 
instr_name = "LHR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0101_????????_?????_111_?????_0001011 : begin 
instr_name = "LHU.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1101_????????_?????_111_?????_0001011 : begin 
instr_name = "LHU.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111101_?????_?????_111_?????_0001011 : begin 
instr_name = "LHUR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011101_?????_?????_111_?????_0001011 : begin 
instr_name = "LHUR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011101_?????_?????_111_?????_0001011 : begin 
instr_name = "LHUR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0010_????????_?????_111_?????_0001011 : begin 
instr_name = "LW.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1010_????????_?????_111_?????_0001011 : begin 
instr_name = "LW.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111010_?????_?????_111_?????_0001011 : begin 
instr_name = "LWR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011010_?????_?????_111_?????_0001011 : begin 
instr_name = "LWR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011010_?????_?????_111_?????_0001011 : begin 
instr_name = "LWR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0000_????????_?????_110_?????_0001011 : begin 
instr_name = "SB.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1000_????????_?????_110_?????_0001011 : begin 
instr_name = "SB.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111000_?????_?????_110_?????_0001011 : begin 
instr_name = "SBR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011000_?????_?????_110_?????_0001011 : begin 
instr_name = "SBR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011000_?????_?????_110_?????_0001011 : begin 
instr_name = "SBR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0001_????????_?????_110_?????_0001011 : begin 
instr_name = "SH.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1001_????????_?????_110_?????_0001011 : begin 
instr_name = "SH.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111001_?????_?????_110_?????_0001011 : begin 
instr_name = "SHR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011001_?????_?????_110_?????_0001011 : begin 
instr_name = "SHR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011001_?????_?????_110_?????_0001011 : begin 
instr_name = "SHR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0010_????????_?????_110_?????_0001011 : begin 
instr_name = "SW.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b1010_????????_?????_110_?????_0001011 : begin 
instr_name = "SW.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b10; 
end 
32'b0111010_?????_?????_110_?????_0001011 : begin 
instr_name = "SWR"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b0011010_?????_?????_110_?????_0001011 : begin 
instr_name = "SWR.POST"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b1011010_?????_?????_110_?????_0001011 : begin 
instr_name = "SWR.PRE"; 
mrf_1_vld_pre = 3'b001; 
mrf_2_vld_pre = 2'b01; 
end 
32'b111100000000_?????_110_?????_0001011 : begin 
instr_name = "REV8HW"; 
mrf_1_vld_pre = 3'b001; 
end 
32'b111100000001_?????_110_?????_0001011 : begin 
instr_name = "REV8SLHW"; 
mrf_1_vld_pre = 3'b001; 
end 
endcase 
end 
assign inst_type = (instr_data[1:0] == 2'b11) ? 1'b1 : 1'b0; 
assign mrf_1_vld = mrf_1_vld_pre[0]; 
assign mrf_2_vld = mrf_2_vld_pre[0]; 
endmodule
 
 
module m130_instr_buf #( 
parameter type T = logic [31:0], 
parameter I_PORTS_N = 4, 
parameter O_PORTS_N = 2, 
parameter FIFO_DEPTH = 8, 
parameter OFFSET_W = 3 
) ( 
input logic clk, 
input logic rst_n, 
input logic flush, 
input logic [I_PORTS_N-1:0] wr_vld, 
input T [I_PORTS_N-1:0] wdata, 
output logic [O_PORTS_N-1:0] rd_vld, 
input logic [O_PORTS_N-1:0] rd_rdy, 
output T [O_PORTS_N-1:0] rdata, 
output logic [O_PORTS_N-1:0][4:0] mrf_1, 
output logic [O_PORTS_N-1:0][4:0] mrf_2, 
output logic [O_PORTS_N-1:0] mrf_1_vld, 
output logic [O_PORTS_N-1:0] mrf_2_vld, 
input logic pending_wr, 
input logic[OFFSET_W-1:0] fetch_offset, 
input logic[1:0] core_priv_mode, 
output logic stall 
); 
localparam int unsigned ADDR_W = $clog2(FIFO_DEPTH); 
localparam RD_TIMING_OPT = 1; 
localparam int IS_POWER_OFTWO = (2 ** ADDR_W) == FIFO_DEPTH; 
localparam int DATA_WIDTH = $bits(T); 
logic [FIFO_DEPTH-1:0] wen_pre; 
logic [FIFO_DEPTH-1:0][FIFO_DEPTH-1:0] matrix_wen_shift; 
logic [FIFO_DEPTH-1:0] ent_data_wen; 
T [FIFO_DEPTH-1:0] ent_data_d; 
T [FIFO_DEPTH-1:0] ent_data_q; 
logic [FIFO_DEPTH-1:0][$clog2(I_PORTS_N)-1:0] wr_sel_fixed; 
logic [FIFO_DEPTH-1:0][FIFO_DEPTH-1:0][$clog2(I_PORTS_N)-1:0] matrix_wr_sel; 
logic [FIFO_DEPTH-1:0][$clog2(I_PORTS_N)-1:0] wr_sel_shifted; 
logic [O_PORTS_N-1:0][ADDR_W-1:0] rd_sel; 
logic [I_PORTS_N-1:0] wr_handshake; 
logic wr_ptr_en; 
logic [ADDR_W :0] wr_ptr_delta; 
logic [ADDR_W :0] wr_ptr_d; 
logic [ADDR_W :0] wr_ptr_q; 
logic [ADDR_W :0] wr_addr_acc; 
logic [ADDR_W-1:0] wr_addr; 
logic [O_PORTS_N-1:0] rd_handshake; 
logic rd_ptr_en; 
logic [ADDR_W :0] rd_ptr_delta; 
logic [ADDR_W :0] rd_ptr_d; 
logic [ADDR_W :0] rd_ptr_q; 
logic [ADDR_W :0] rd_addr_acc; 
logic [ADDR_W-1:0] rd_addr; 
logic [ADDR_W:0] cnt_occupy; 
logic [ADDR_W:0] effective_depth; 
logic lt_one_fetch; 
logic lt_two_fetch; 
logic hv_pop; 
assign wr_handshake = wr_vld; 
always_comb begin : cnt_in_hdsk 
wr_ptr_delta = '0; 
for (int i=0; i<I_PORTS_N; i++) begin 
wr_ptr_delta = wr_ptr_delta + wr_handshake[i]; 
end 
end 
if(IS_POWER_OFTWO == 1 ) begin : g_wr_ptr_POT 
assign wr_addr_acc = wr_ptr_q + wr_ptr_delta; 
assign wr_ptr_d = flush ? '0 : wr_addr_acc; 
end 
else begin : g_wr_ptr_nPOT 
logic wr_ovf_n; 
assign wr_addr_acc = {1'b0, wr_addr} + wr_ptr_delta; 
assign wr_ovf_n = wr_addr_acc < (FIFO_DEPTH); 
assign wr_ptr_d[ADDR_W] = flush ? '0 : 
wr_ovf_n ? wr_ptr_q[ADDR_W]: 
~wr_ptr_q[ADDR_W]; 
assign wr_ptr_d[ADDR_W-1:0] = flush ? '0 : 
wr_ovf_n ? wr_addr_acc[ADDR_W -1:0]: 
wr_addr_acc -(FIFO_DEPTH); 
end 
assign wr_ptr_en = flush | wr_handshake[0]; 
`WDFFER(wr_ptr_q, wr_ptr_d, wr_ptr_en, clk, rst_n) 
assign wr_addr = wr_ptr_q[ADDR_W-1:0]; 
assign hv_pop = rdata[0].is_hv & rd_vld[0]; 
assign rd_handshake = (rd_vld & rd_rdy) | {{(O_PORTS_N-1){1'b0}}, hv_pop}; 
always_comb begin : cnt_rd_en 
rd_ptr_delta = '0; 
for (int i=0; i<O_PORTS_N; i++) begin 
rd_ptr_delta = rd_ptr_delta + rd_handshake[i]; 
end 
end 
if(IS_POWER_OFTWO == 1 ) begin : g_rd_ptr_POT 
assign rd_addr_acc = rd_ptr_q + rd_ptr_delta; 
assign rd_ptr_d = flush ? '0 : rd_addr_acc; 
end 
else begin : g_rd_ptr_nPOT 
logic rd_ovf_n; 
assign rd_addr_acc = {1'b0, rd_addr} + rd_ptr_delta; 
assign rd_ovf_n = rd_addr_acc < FIFO_DEPTH; 
assign rd_ptr_d[ADDR_W] = flush ? '0 : 
rd_ovf_n ? rd_ptr_q[ADDR_W]: 
~rd_ptr_q[ADDR_W]; 
assign rd_ptr_d[ADDR_W-1:0] = flush ? '0 : 
rd_ovf_n ? rd_addr_acc[ADDR_W -1:0] : 
rd_addr_acc - FIFO_DEPTH; 
end 
assign rd_ptr_en = flush | rd_handshake[0]; 
`WDFFER(rd_ptr_q, rd_ptr_d, rd_ptr_en, clk, rst_n) 
assign rd_addr = rd_ptr_q[ADDR_W-1:0]; 
assign wen_pre = {{FIFO_DEPTH-I_PORTS_N{1'b0}}, wr_handshake[I_PORTS_N-1:0]}; 
for (genvar i=0; i<FIFO_DEPTH; i++) begin : g_matrix_wen 
if (i==0)begin:g_wen_matrix0 
assign matrix_wen_shift[i] = wen_pre[FIFO_DEPTH-1:0]; 
end 
else begin:g_wen_matrix 
assign matrix_wen_shift[i] = {wen_pre[0 +: (FIFO_DEPTH-i)], wen_pre[(FIFO_DEPTH-1) -: i]}; 
end 
end 
always@(*) begin 
ent_data_wen = {FIFO_DEPTH{1'b0}}; 
for (int i=0; i<FIFO_DEPTH; i++) begin : g_fifo_wen 
ent_data_wen = ent_data_wen | ( {FIFO_DEPTH{wr_addr == i}} & matrix_wen_shift[i]); 
end 
end 
always@(*) begin 
wr_sel_shifted = {FIFO_DEPTH*($clog2(I_PORTS_N)){1'b0}}; 
for (int i=0; i<FIFO_DEPTH; i++) begin : g_fifo_wen 
wr_sel_shifted = wr_sel_shifted | ( {FIFO_DEPTH*($clog2(I_PORTS_N)){wr_addr == i}} & matrix_wr_sel[i]); 
end 
end 
for (genvar i=0; i<FIFO_DEPTH; i++) begin : ent_data 
if (i<I_PORTS_N) begin: g_wr_sel_fixed 
assign wr_sel_fixed[i] = i; 
end 
else begin: g_wr_sel_dummy 
assign wr_sel_fixed[i] = '0; 
end 
if (i==0) begin:g_matrix_wr0_sel 
assign matrix_wr_sel[i] = wr_sel_fixed[0+:FIFO_DEPTH]; 
end 
else begin:g_matrix_wr_sel 
assign matrix_wr_sel[i] = {wr_sel_fixed[0+:(FIFO_DEPTH-i)], 
wr_sel_fixed[(FIFO_DEPTH-1)-:i]}; 
end 
assign ent_data_d[i] = wdata[wr_sel_shifted[i]]; 
`WDFFER(ent_data_q[i], ent_data_d[i], ent_data_wen[i], clk, rst_n) 
end 
logic [O_PORTS_N-1:0] head_reg_en; 
T [O_PORTS_N-1:0] head_reg_d; 
T [O_PORTS_N-1:0] head_reg_d_pre; 
T [O_PORTS_N-1:0] head_reg_q; 
logic [O_PORTS_N-1:0][1:0] head_mrf_vld_d; 
logic [O_PORTS_N-1:0][1:0] head_mrf_vld_q; 
T ent_data_q_rd_next; 
T ent_data_q_rd_plus_two; 
logic [O_PORTS_N-1:0][4:0] dec_mrf_1; 
logic [O_PORTS_N-1:0][4:0] dec_mrf_2; 
logic direct_write; 
logic [O_PORTS_N-1:0] instr_decd_inst_type; 
logic pop_one; 
logic pop_two; 
logic direct_write_head1; 
logic fifo_has_two; 
logic fifo_has_three; 
logic [ADDR_W-1:0] rd_ptr_d_next; 
assign pop_one = ^rd_handshake; 
assign pop_two = rd_handshake[0] & rd_handshake[1]; 
assign fifo_has_two = cnt_occupy == 'd2; 
assign fifo_has_three = cnt_occupy == 'd3; 
assign rd_ptr_d_next = rd_ptr_d[ADDR_W-1:0] == (FIFO_DEPTH-1) ? '0 : rd_ptr_d[ADDR_W-1:0] + 1; 
assign direct_write = ~(|cnt_occupy) | 
pop_two & fifo_has_two | 
pop_one & cnt_occupy == 'd1; 
assign direct_write_head1 = cnt_occupy == 'd1 | 
pop_one & fifo_has_two | 
(pop_two & fifo_has_three); 
always_comb begin 
ent_data_q_rd_next = 'b0; 
for (int i=0; i<FIFO_DEPTH; i++) begin 
ent_data_q_rd_next = ent_data_q_rd_next | ({DATA_WIDTH{rd_ptr_d[ADDR_W-1:0] == i}} & ent_data_q[i]); 
end 
end 
always_comb begin 
ent_data_q_rd_plus_two = 'b0; 
for (int i=0; i<FIFO_DEPTH; i++) begin 
ent_data_q_rd_plus_two = ent_data_q_rd_plus_two | ({DATA_WIDTH{rd_ptr_d_next[ADDR_W-1:0] == i}} & ent_data_q[i]); 
end 
end 
always_comb begin 
head_reg_d_pre = rdata; 
if(direct_write) begin 
head_reg_en = wr_handshake[1:0]; 
head_reg_d_pre = wdata[1:0]; 
end 
else begin 
head_reg_en[0] = rd_handshake[0]; 
head_reg_en[1] = direct_write_head1 ? wr_handshake[0] 
: rd_handshake[0]; 
head_reg_d_pre[0] = ent_data_q_rd_next; 
head_reg_d_pre[1] = direct_write_head1 ? wdata[0] 
: ent_data_q_rd_plus_two; 
end 
end 
for (genvar i=0; i<O_PORTS_N; i++) begin : g_head_reg_logic 
assign head_reg_d[i].pcbuf_addr = head_reg_d_pre[i].pcbuf_addr; 
assign head_reg_d[i].pc_offset = head_reg_d_pre[i].pc_offset; 
assign head_reg_d[i].pop_pcbuf = head_reg_d_pre[i].pop_pcbuf; 
assign head_reg_d[i].excp_vld = head_reg_d_pre[i].excp_vld; 
assign head_reg_d[i].is_hv = head_reg_d_pre[i].is_hv; 
m130_ifu_dec u_idx_dec( 
.instr_data (head_reg_d_pre[i].instr ), 
.inst_type (instr_decd_inst_type[i] ), 
.core_priv_mode (core_priv_mode ), 
.instr_name ( ), 
.mrf_1 (dec_mrf_1[i]), 
.mrf_2 (dec_mrf_2[i]), 
.mrf_1_vld (head_mrf_vld_d[i][0] ), 
.mrf_2_vld (head_mrf_vld_d[i][1] ) 
); 
assign head_reg_d[i].instr[31:26] = head_reg_d_pre[i].instr[31:26]; 
assign head_reg_d[i].instr[25] = instr_decd_inst_type[i] | head_reg_d_pre[i].is_hv ? head_reg_d_pre[i].instr[25] 
: head_reg_d_pre[i].instr[15]; 
assign head_reg_d[i].instr[24:15] = head_reg_d_pre[i].is_hv ? head_reg_d_pre[i].instr[24:15] 
: {dec_mrf_2[i], dec_mrf_1[i]}; 
assign head_reg_d[i].instr[14:0] = head_reg_d_pre[i].instr[14:0]; 
`WDFFER(head_reg_q[i], head_reg_d[i], head_reg_en[i], clk, rst_n) 
`WDFFER(head_mrf_vld_q[i], head_mrf_vld_d[i], head_reg_en[i], clk, rst_n) 
assign rdata[i].pcbuf_addr = head_reg_q[i].pcbuf_addr; 
assign rdata[i].pc_offset = head_reg_q[i].pc_offset; 
assign rdata[i].pop_pcbuf = head_reg_q[i].pop_pcbuf; 
assign rdata[i].excp_vld = head_reg_q[i].excp_vld; 
assign rdata[i].is_hv = head_reg_q[i].is_hv; 
assign rdata[i].instr[31:16] = head_reg_q[i].instr[31:16]; 
assign rdata[i].instr[15] = &(head_reg_q[i].instr[1:0]) | head_reg_q[i].is_hv ? head_reg_q[i].instr[15] 
: head_reg_q[i].instr[25]; 
assign rdata[i].instr[14:0] = head_reg_q[i].instr[14:0]; 
assign mrf_2[i] = head_reg_q[i].instr[24:20]; 
assign mrf_1[i] = head_reg_q[i].instr[19:15]; 
assign mrf_1_vld[i] = head_mrf_vld_q[i][0]; 
assign mrf_2_vld[i] = head_mrf_vld_q[i][1]; 
end 
assign cnt_occupy = (wr_ptr_q[ADDR_W] ^ rd_ptr_q[ADDR_W]) ? (FIFO_DEPTH + {1'b0,wr_addr} - {1'b0,rd_addr}) : 
(wr_ptr_q - rd_ptr_q); 
for (genvar i=0; i<O_PORTS_N; i++) begin : g_rd_vld 
assign rd_vld[i] = cnt_occupy > i; 
end 
assign effective_depth = FIFO_DEPTH + fetch_offset[OFFSET_W-1:1]; 
assign lt_two_fetch = $signed(cnt_occupy) > $signed(effective_depth - 2*I_PORTS_N); 
assign lt_one_fetch = cnt_occupy > (FIFO_DEPTH - I_PORTS_N); 
assign stall = lt_one_fetch | 
lt_two_fetch & pending_wr; 
endmodule
 
 
 
module m130_pc_buf #( 
parameter DATA_WIDTH = 32, 
parameter FIFO_DEPTH = 2, 
parameter O_PORTS_N = 2, 
parameter ADDR_W = $clog2(FIFO_DEPTH) 
) ( 
input logic clk, 
input logic rst_n, 
input logic flush, 
input logic wr_en, 
input logic [DATA_WIDTH-1:0] wdata, 
input logic [O_PORTS_N-1:0] rd_en_vec, 
input logic [O_PORTS_N-1:0][ADDR_W-1:0] rd_addr, 
output logic [O_PORTS_N-1:0][DATA_WIDTH-1:0] rdata, 
output logic afull, 
output logic full, 
output logic [DATA_WIDTH-1:0] wr_data_prev, 
output logic [ADDR_W-1:0] wr_addr_prev, 
output logic [ADDR_W-1:0] wr_addr 
); 
localparam int IS_POWER_OFTWO = (2 ** ADDR_W)== FIFO_DEPTH; 
localparam int unsigned AFULL_WL = 1; 
logic wr_handshake; 
logic wr_ptr_en; 
logic [ADDR_W:0] wr_ptr_d; 
logic [ADDR_W:0] wr_ptr_q; 
logic [ADDR_W:0] wr_ptr_next; 
logic empty; 
logic [ADDR_W:0] wr_ptr_stand; 
logic [O_PORTS_N-1:0] rd_handshake; 
logic rd_ptr_en; 
logic [ADDR_W :0] rd_ptr_d; 
logic [ADDR_W :0] rd_ptr_q; 
logic [ADDR_W :0] rd_ptr_acc; 
logic [FIFO_DEPTH-1:0][DATA_WIDTH-1:0] fifo_ent; 
logic [FIFO_DEPTH-1:0] fifo_ent_en; 
if(IS_POWER_OFTWO == 1 ) begin : g_flags_POT 
assign empty = (rd_ptr_q[ADDR_W-1:0] == wr_ptr_q[ADDR_W-1:0]) && 
(rd_ptr_q[ADDR_W] == wr_ptr_q[ADDR_W]); 
assign full = (rd_ptr_q[ADDR_W-1:0] == wr_ptr_q[ADDR_W-1:0]) && 
(rd_ptr_q[ADDR_W] ^ wr_ptr_q[ADDR_W]); 
assign wr_ptr_stand = wr_ptr_q + AFULL_WL; 
assign afull = (rd_ptr_q[ADDR_W-1:0] == wr_ptr_stand[ADDR_W-1:0]) && 
(rd_ptr_q[ADDR_W] ^ wr_ptr_stand[ADDR_W]); 
end else begin : g_flags_non_POT 
logic cnt_en; 
logic [ADDR_W-1:0] cnt_d; 
logic [ADDR_W-1:0] cnt_q; 
logic [ADDR_W-1:0] wr_handshake_ext; 
logic [ADDR_W-1:0] rd_handshake_ext; 
assign wr_handshake_ext = {{ADDR_W-1{1'b0}}, wr_handshake}; 
assign rd_handshake_ext = {{ADDR_W-O_PORTS_N{1'b0}}, rd_handshake}; 
assign cnt_en = |rd_handshake | wr_handshake | flush; 
assign cnt_d = flush ? '0 : 
&rd_handshake ? cnt_q + wr_handshake_ext - 2 : 
cnt_q + wr_handshake_ext - |rd_handshake_ext; 
`WDFFER(cnt_q, cnt_d, cnt_en, clk, rst_n) 
assign empty = cnt_q == '0; 
assign full = cnt_q == FIFO_DEPTH; 
assign afull = cnt_q == (FIFO_DEPTH-AFULL_WL); 
end 
assign wr_handshake = wr_en && !full; 
assign wr_ptr_en = flush | wr_handshake; 
assign wr_ptr_next = wr_ptr_q[ADDR_W:0] + 1'b1; 
if(IS_POWER_OFTWO == 1 ) begin:g_pot_wr_ptr 
assign wr_ptr_d = flush ? '0 : wr_ptr_next; 
assign wr_addr_prev = wr_ptr_q[ADDR_W-1:0] - 1'b1; 
end 
else begin:g_non_pot_wr_ptr 
assign wr_ptr_d = flush | (wr_ptr_next == FIFO_DEPTH) ? '0 : wr_ptr_next; 
assign wr_addr_prev = |wr_ptr_q[ADDR_W-1:0] ? wr_ptr_q[ADDR_W-1:0] - 1'b1 : FIFO_DEPTH-1; 
end 
`WDFFER(wr_ptr_q, wr_ptr_d, wr_ptr_en, clk, rst_n) 
assign wr_addr = wr_ptr_q[ADDR_W-1:0]; 
always_comb begin 
wr_data_prev = {DATA_WIDTH{1'b0}}; 
for (int i=0; i<FIFO_DEPTH; i++) begin : g_fifo_pre_wdata 
wr_data_prev = wr_data_prev | ({DATA_WIDTH{wr_addr_prev == i}} & fifo_ent[i]); 
end 
end 
assign rd_handshake = rd_en_vec & {O_PORTS_N{!empty}}; 
always_comb begin : cnt_rd_en 
rd_ptr_acc = rd_ptr_q; 
for (int i=0; i<O_PORTS_N; i++) begin 
rd_ptr_acc = rd_ptr_acc + rd_handshake[i]; 
end 
end 
if(IS_POWER_OFTWO == 1 ) begin:g_pot_rd_ptr 
assign rd_ptr_d = flush ? '0 : rd_ptr_acc; 
end else begin:g_non_pot_rd_ptr 
assign rd_ptr_d = flush | rd_ptr_acc == FIFO_DEPTH ? '0 : 
rd_ptr_acc == (FIFO_DEPTH+1) ? 'd1 : 
rd_ptr_acc; 
end 
assign rd_ptr_en = flush | (|rd_handshake); 
`WDFFER(rd_ptr_q, rd_ptr_d, rd_ptr_en, clk, rst_n) 
for (genvar i=0; i<FIFO_DEPTH; i++) begin: gen_fifo_ent 
assign fifo_ent_en[i] = wr_ptr_en & (i==wr_addr); 
`WDFFENR (fifo_ent[i], wdata, fifo_ent_en[i], clk) 
end 
always_comb begin 
for (int i=0; i<O_PORTS_N; i++) begin 
rdata[i] = {DATA_WIDTH{1'b0}}; 
for (int j=0; j<FIFO_DEPTH; j++) begin : g_fifo_pre_wdata 
rdata[i] = rdata[i] | ({DATA_WIDTH{rd_addr[i] == j}} & fifo_ent[j]); 
end 
end 
end 
endmodule
 
 
 
module m130_pcu_top ( 
input logic clk , 
input logic always_on_clk , 
input logic rst_n , 
output logic core_init_busy , 
output logic [1:0] core_priv_mode , 
output logic core_dbg_mode , 
output logic core_sleep_mode , 
output logic core_deep_sleep , 
output logic core_sleep_wakeup , 
output logic core_locked , 
input logic ifu_idle , 
input logic lsu_idle , 
input logic mss_idle , 
input logic bmu_idle , 
input logic mss_pcu_icache_init_busy , 
input logic [32-1:0] top_hart_id , 
input logic [32-1:0] top_boot_pc , 
input logic top_mstatus_be , 
input logic [63:0] mem_map_mtime , 
output logic [6:0] pcu_top_security_err_bus , 
input logic [2-1:0] ifu_pcu_vld , 
output logic [2-1:0] pcu_ifu_rdy , 
input logic [2-1:0][31:0] ifu_pcu_instr_data , 
input logic [2-1:0][4:0] ifu_pcu_instr_rs1_idx , 
input logic [2-1:0] ifu_pcu_instr_rs1_vld , 
input logic [2-1:0][4:0] ifu_pcu_instr_rs2_idx , 
input logic [2-1:0] ifu_pcu_instr_rs2_vld , 
input logic [2-1:0][32-1:0] ifu_pcu_instr_pc , 
input logic [2-1:0] ifu_pcu_excp_vld , 
output logic pcu_ifu_call_rslv , 
output logic pcu_ifu_ret_rslv , 
input logic [32-1:0] ifu_pcu_excp_cause , 
output logic pcu_ifu_flush_vld , 
output logic [32-1:0] pcu_ifu_flush_pc , 
output logic pcu_ifu_brq_rd , 
input logic [32-1:0] ifu_pcu_brq_pc , 
output logic pcu_ifu_stall , 
output logic [1:0] pcu_ifu_shv_flush , 
input logic ifu_pcu_hv_done , 
input logic ifu_pcu_hv_excp_vld , 
input logic [32-1:0] ifu_pcu_hv_pc , 
output logic [2-1:0] pcu_exu_inst_vld , 
output logic [2-1:0][32-1:0] pcu_exu_alu_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_alu_logic_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_eq_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_mdu_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_agu_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_alu_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_alu_logic_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_eq_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_mdu_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_agu_op2_val , 
output logic [2-1:0][4:0] pcu_exu_shift_shamt , 
output logic [2-1:0][32-1:0] pcu_exu_inst_op3_val , 
output logic [2-1:0] pcu_exu_inst_signed , 
output logic [2-1:0][8-1:0] pcu_exu_inst_funct , 
output logic [2-1:0][4-1:0] pcu_exu_inst_sub_funct , 
output logic [2-1:0][8-1:0] pcu_exu_inst_opcode , 
output logic [2-1:0] pcu_exu_inst_type , 
output logic [2-1:0] pcu_exu_inst_is_ret , 
output logic [32-1:0] pcu_exu_inst_pc , 
output logic [32-1:0] pcu_exu_inst_pred_pc , 
output logic [2-1:0][4-1:0] pcu_exu_sub_funct_wb , 
output logic [2-1:0][8-1:0] pcu_exu_opcode_wb , 
input logic exu_pcu_br_tkn , 
input logic [2-1:0] exu_pcu_inst_wb_stall , 
input logic [2-1:0][32-1:0] exu_pcu_inst_alu_data , 
input logic [2-1:0][32-1:0] exu_pcu_inst_alu_logic_data, 
input logic [2-1:0][32-1:0] exu_pcu_inst_mdu_data , 
input logic [32-1:0] exu_pcu_inst_tgt_pc , 
input logic exu_pcu_bru_flush , 
input logic [32-1:0] exu_pcu_bru_flush_pc , 
output logic [1-1:0] pcu_lsu_inst_vld , 
input logic [1-1:0] lsu_pcu_inst_rdy , 
output logic [1-1:0][4-1:0] pcu_lsu_inst_cmd , 
output logic [1-1:0][3:0] pcu_lsu_inst_fence_op , 
output logic [1-1:0][$clog2(7)-1:0] pcu_lsu_inst_amo_op , 
output logic [1-1:0] pcu_lsu_inst_exclusive , 
output logic [1-1:0] pcu_lsu_inst_ord_aq , 
output logic [1-1:0] pcu_lsu_inst_ord_rl , 
output logic [1-1:0] pcu_lsu_inst_dst_vld , 
output logic [1-1:0][32-1:0] pcu_lsu_inst_data , 
output logic [1-1:0] pcu_lsu_inst_data_wb_vld , 
output logic [1-1:0][2-1:0] pcu_lsu_inst_size , 
output logic [1-1:0] pcu_lsu_inst_signed , 
output logic [1-1:0] pcu_lsu_inst_addr_fwd , 
output logic [1-1:0][32-1:0] pcu_lsu_inst_addr_fwd_val , 
input logic [1-1:0] lsu_pcu_cmt , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_data , 
input logic [1-1:0] lsu_pcu_cmt_excp , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_excp_cause , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_excp_addr , 
input logic [1-1:0] lsu_pcu_cmt_trig_dbg , 
output logic pcu_lsu_flush_st_inst_wb , 
output logic pcu_lsu_flush , 
output logic pcu_lsu_non_spec , 
input logic lsu_pcu_non_flush_infly , 
input logic pmp_pcu_csr_done , 
input logic [32-1:0] pmp_pcu_csr_rdata , 
input logic pmp_pcu_csr_excp , 
input logic pmp_pcu_csr_match , 
input logic clic_pcu_int_vld , 
input logic [$clog2(64 + 32)-1:0] clic_pcu_int_id , 
input logic [8-1:0] clic_pcu_int_lvl , 
input logic clic_pcu_int_shv , 
input logic [1:0] clic_pcu_int_priv , 
output logic pcu_clic_rsp_clr_ip , 
output logic [$clog2(64 + 32)-1:0] pcu_clic_rsp_id , 
output logic pcu_clic_rsp_vld , 
output logic pcu_clic_stoptime , 
input logic dm_pcu_halt_req , 
input logic dm_pcu_halt_on_reset , 
input logic dm_pcu_resume_req , 
output logic pcu_dm_halted , 
output logic pcu_dm_havereset , 
output logic pcu_dm_unavail , 
input logic dm_pcu_ack_havereset , 
output logic pcu_dm_cmd_done , 
output logic pcu_dm_bus_err , 
output logic pcu_dm_cmd_excp , 
input logic dm_pcu_dsch0_write , 
input logic [32-1:0] dm_pcu_dsch0_wdata , 
output logic [32-1:0] pcu_dm_dsch0_rdata , 
output logic [2-1:0] pcu_tm_pc_ex_vld , 
output logic [2-1:0][32-1:0] pcu_tm_pc_ex , 
output logic [2-1:0] pcu_tm_inst_size_ex , 
input logic [2-1:0] tm_pcu_trigger_pc_vld , 
input logic [2-1:0] tm_pcu_trigger_pc_debug , 
input logic [2-1:0][4-1:0] tm_pcu_trigger_hit_pc_idx , 
input logic tm_pcu_trigger_addr_vld , 
input logic tm_pcu_trigger_addr_debug , 
input logic [4-1:0] tm_pcu_trigger_hit_addr_idx, 
output logic pcu_tm_trap_vld , 
output logic [32-1:0] pcu_tm_trap_cause , 
output logic [2-1:0] pcu_tm_inst_retire , 
output logic pcu_tm_tcontrol_mte , 
output logic pcu_tm_trigger_hit , 
output logic [4-1:0] pcu_tm_trigger_hit_idx , 
input logic tm_pcu_csr_done , 
input logic [32-1:0] tm_pcu_csr_rdata , 
input logic tm_pcu_csr_excp , 
input logic tm_pcu_csr_match , 
output logic pcu_csr_vld , 
output logic [1:0] pcu_csr_opcode , 
output logic [11:0] pcu_csr_index , 
output logic [32-1:0] pcu_csr_wdata , 
output logic pcu_csr_sp_en , 
output logic pcu_csr_mstatus_be , 
output logic pcu_csr_mprv , 
output logic [1:0] pcu_csr_mpp 
); 
 
localparam X0 = 5'b0 ; 
localparam X2 = 5'h2 ; 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
localparam FENCE_INST = 0; 
localparam AMO_INST = 1; 
localparam LD_INST = 2; 
localparam ST_INST = 3; 
localparam LS_POST = 5; 
localparam EXCLUSIVE_INST = 0; 
localparam SYS = 0 ; 
localparam SYS_E = 1 ; 
localparam CSR_INST = 2 ; 
localparam BRKPT = 12'h3 ; 
localparam ILL_INST = 12'h2 ; 
localparam LP_ERR = 12'h12 ; 
localparam ECALL_U = 12'h8 ; 
localparam ECALL_M = 12'hb ; 
localparam LD_RAS_ERR = 12'h1F ; 
localparam INST_MISALIGN = 12'h0 ; 
localparam IFU_PMP_ERR = 12'h1 ; 
localparam LD_ACC_FAULT = 12'h5 ; 
localparam LD_MISALIGN = 12'h4 ; 
localparam ST_ACC_FAULT = 12'h7 ; 
localparam ST_MISALIGN = 12'h6 ; 
localparam RAS_ERR = 12'h1f ; 
localparam CSRRW = 0 ; 
localparam CSRRS = 1 ; 
localparam CSRRC = 2 ; 
localparam CSRIMM = 3 ; 
localparam UNAVAIL = 2'h0; 
localparam HALTED = 2'h1; 
localparam RUNNING = 2'h2; 
localparam NMI_ID = 12'hFFF ; 
localparam MRET = 0 ; 
localparam WFI = 1 ; 
localparam MNRET = 2 ; 
localparam ECALL = 0 ; 
localparam EBREAK = 1 ; 
localparam MRASIE = 31 ; 
localparam MEIE = 11 ; 
localparam MTIE = 7 ; 
localparam MSIE = 3 ; 
localparam MRASIP = 31 ; 
localparam MEIP = 11 ; 
localparam MTIP = 7 ; 
localparam MSIP = 3 ; 
typedef enum logic [4:0]{ 
LOAD = 5'b00001 , 
STORE = 5'b00010 , 
FENCE = 5'b00100 
}sub_func_lsu_e ; 
typedef enum logic [4:0] { 
SUB_FU_NONE = 5'b00000 , 
ARITHMATIC = 5'b00001 , 
SHIFT = 5'b00010 , 
LOGICAL = 5'b00100 , 
MISC = 5'b01000 
} sub_func_alu_e; 
typedef enum logic [4:0] { 
UNCOND_DIRE = 5'b00001 , 
COND_DIRE = 5'b00010 , 
UNCOND_INDIRE = 5'b00100 
}sub_func_bru_e ; 
typedef enum logic [3:0] { 
OTHER_TYPES = 4'd0 , 
EXCP_TYPE = 4'd1 , 
INT_TYPE = 4'd2 , 
MRET_TYPE = 4'd3 , 
NT_BR_TYPE = 4'd4 , 
T_BR_TYPE = 4'd5 , 
UNINF_TYPE = 4'd6 
}itype; 
typedef struct packed{ 
logic interrupt; 
logic minhv; 
logic [1:0] mpp; 
logic mpie; 
logic [7:0] mpil; 
logic [11:0] exccode; 
} mcause_csr; 
typedef struct packed{ 
logic mprv; 
logic [1:0] mpp; 
logic mpie; 
logic mie; 
} mstatus_csr; 
typedef struct packed{ 
logic mnpelp; 
logic [1:0] mnpp; 
logic mnpv; 
logic nmie; 
}mnstatus_csr; 
typedef struct packed{ 
logic [3:0] debuger ; 
logic pelp ; 
logic ebreakvs ; 
logic ebreakvu ; 
logic ebreakm ; 
logic ebreaks ; 
logic ebreaku ; 
logic stepie ; 
logic stopcount; 
logic stoptime ; 
logic [2:0] cause ; 
logic v ; 
logic mprven ; 
logic nmip ; 
logic step ; 
logic [1:0] prv ; 
}dcsr_csr; 
typedef enum logic [3:0] { 
SPLIT_IDLE = 4'b0000 , 
SPLIT_UOP = 4'b0001 , 
SPLIT_LI = 4'b0010 , 
SPLIT_ADD = 4'b0110 , 
SPLIT_RET = 4'b1000 , 
SPLIT_SHF_ADD = 4'b1100 
} state_e; 
 
logic [2-1:0] instr_decd_inst_type ; 
logic [2-1:0][2:0] instr_decd_op1_src ; 
logic [2-1:0][1:0] instr_decd_op2_src ; 
logic [2-1:0][1:0] instr_decd_op3_src ; 
logic [2-1:0][8-1:0] instr_decd_funct ; 
logic [2-1:0][4-1:0] instr_decd_sub_funct ; 
logic [2-1:0][8-1:0] instr_decd_opcode ; 
logic [2-1:0] instr_decd_signed ; 
logic [2-1:0][4:0] instr_decd_dest_idx ; 
logic [2-1:0][4:0] instr_decd_src1_idx ; 
logic [2-1:0][4:0] iss_rs1_idx_ex ; 
logic [2-1:0][4:0] instr_decd_src2_idx ; 
logic [2-1:0][4:0] iss_rs2_idx_ex ; 
logic [2-1:0][4:0] instr_decd_src3_idx ; 
logic [2-1:0][$clog2(32)-1:0] instr_rs1_idx_iss_ex ; 
logic [2-1:0][$clog2(32)-1:0] instr_rs2_idx_iss_ex ; 
logic [2-1:0][32-1:0] instr_decd_imm ; 
logic [2-1:0] instr_decd_illegal_pre ; 
logic [2-1:0] instr_decd_illegal ; 
logic [2-1:0][127:0] instr_name ; 
logic [2-1:0][2:0] op1_src_vld ; 
logic [2-1:0][1:0] op2_src_vld ; 
logic [2-1:0][1:0] op3_src_vld ; 
logic [4-1:0][$clog2(32)-1:0] mrf_gpr_rd_idx ; 
logic [4-1:0][32-1:0] mrf_gpr_rd_data ; 
logic [32-1:0] mrf_gpr_x7 ; 
logic [2-1:0] mrf_gpr_wr_vld ; 
logic [2-1:0][$clog2(32)-1:0] mrf_gpr_wr_idx ; 
logic [2-1:0][32-1:0] mrf_gpr_wr_data ; 
logic [2-1:0] mrf_gpr_dst_vld ; 
logic mrf_gpr_need_cleard ; 
logic cmt_flush ; 
logic cmt_flush_wo_dbg ; 
logic [32-1:0] cmt_flush_pc ; 
logic flush_wb ; 
logic flush_wb_wo_dbg ; 
logic cmt_stall ; 
logic [2-1:0] cmt_inst_retire ; 
logic [2-1:0] cmt_inst_split_retire ; 
logic ls_exe_cmt_data_vld ; 
logic [2-1:0] cmt_inst_vld ; 
logic cmt_excp_vld ; 
logic [2-1:0] cmt_inst_type ; 
logic [2-1:0] cmt_exp_lp_inst ; 
logic [2-1:0] cmt_lp_inst ; 
logic [2-1:0] cmt_dbg_disable ; 
logic [2-1:0] cmt_ls_addr_write_back ; 
logic cmt_ls_exe_2_cycs_stall_ex; 
logic [2-1:0][8-1:0] cmt_inst_funct ; 
logic [2-1:0][4-1:0] cmt_inst_sub_funct ; 
logic [2-1:0][8-1:0] cmt_inst_opcode ; 
logic [2-1:0] cmt_dest_vld ; 
logic [2-1:0][$clog2(32)-1:0] cmt_dest ; 
logic [1:0][32-1:0] cmt_pc ; 
logic cmt_inst_en ; 
logic cmt_uop_int_mask ; 
logic cmt_trig_exp ; 
logic cmt_trig_dbg ; 
logic cmt_uop_inst ; 
logic cmt_br_tkn ; 
logic cmt_excp ; 
logic [32-1:0] cmt_excp_cause ; 
logic [11:0] cmt_csr_idx_wb ; 
logic [$clog2(32)-1:0] ls_post_pre_addr_dst_idx_wb; 
logic [11:0] csr_idx_ex ; 
logic csr_vld_ex ; 
logic csr_excpt_all_ex ; 
logic pcu_csr_excpt ; 
logic csr_access_excp ; 
logic [32-1:0] jvt_csr ; 
logic [32-1:0] mlpcfg ; 
logic pcu_csr_match_local ; 
logic pcu_csr_match ; 
logic ebreak_dbg_vld ; 
logic debug_mode_enter ; 
logic dcsr_stopcount ; 
logic csr_oth_match ; 
logic [32-1:0] csr_oth_rdata ; 
logic csr_par_match ; 
logic [32-1:0] csr_par_rdata ; 
logic jvt_inst_ex ; 
logic core_step_vld ; 
logic pcu_csr_has_write_op_ex ; 
logic [2:0] fcsr_frm ; 
logic stall_ex ; 
logic core_pseudo_run_state ; 
logic [2-1:0] auipc_inst_ex ; 
logic [2-1:0] mv_li_inst ; 
logic [31:0] br_offset ; 
logic alu_shift_op ; 
logic pcu_csr_split_integrity ; 
logic core_elp_state_vld ; 
logic priv_mode_parity_err ; 
logic cmt_acc_bus_err ; 
logic cmt_acc_pmp_err ; 
logic cmt_inst_dec_ill_err ; 
logic cmt_inst_lpd_flow_err ; 
logic [1:0] lsu_size_ex ; 
logic cm_pop_ret_vld ; 
logic cm_popretz_vld ; 
logic cm_pop_vld ; 
logic [3:0] wb_branch_status ; 
logic wb_inst_vld ; 
logic [31:0] wb_pc_addr ; 
logic [3:0] ex_branch_status ; 
logic ex_inst_vld ; 
logic [31:0] ex_pc_addr ; 
logic [4-1:0] tm_pcu_trigger_hit_idx_ex ; 
logic pcu_ifu_flush_vld_d ; 
logic pcu_ifu_shv_flush_d ; 
logic pcu_lsu_flush_d ; 
logic [31:0] pcu_ifu_flush_pc_d ; 
logic [32-1:0] pcu_csr_rdata_all ; 
logic ifu_pcu_hv_done_fnl ; 
genvar i; 
logic areg_init; 
`WDFFERVAL(areg_init, 1'b0, areg_init, clk, rst_n,1'b1) 
assign pcu_top_security_err_bus = {1'b0,cmt_inst_lpd_flow_err,cmt_inst_dec_ill_err,cmt_acc_bus_err, cmt_acc_pmp_err, 2'b0}; 
assign pcu_csr_match = pcu_csr_match_local | csr_oth_match ; 
assign csr_oth_match = csr_par_match | tm_pcu_csr_match | pmp_pcu_csr_match; 
assign csr_excpt_all_ex = (pmp_pcu_csr_excp & pmp_pcu_csr_match | 
tm_pcu_csr_excp & tm_pcu_csr_match | 
csr_access_excp | 
pcu_csr_excpt | 
!pcu_csr_match ) & pcu_csr_vld ; 
assign csr_oth_rdata = {32{pmp_pcu_csr_match }} & pmp_pcu_csr_rdata | 
{32{tm_pcu_csr_match }} & tm_pcu_csr_rdata | 
{32{csr_par_match }} & csr_par_rdata ; 
assign pcu_tm_pc_ex_vld = ifu_pcu_vld; 
generate for(i = 0; i < 2; i = i + 1) begin: GEN_TM_INTERFACE 
assign pcu_tm_pc_ex[i] = pcu_tm_pc_ex_vld[i] ? ifu_pcu_instr_pc[i] : 32'b0; 
assign pcu_tm_inst_size_ex[i]= pcu_tm_pc_ex_vld[i] ? instr_decd_inst_type[i] : 1'b0; 
end 
endgenerate 
assign instr_decd_illegal[0] = instr_decd_illegal_pre[0] | csr_excpt_all_ex; 
assign mv_li_inst[0] = 1'b0; 
m130_pcu_dec0 u_m130_pcu_dec_0 ( 
.instr_data (ifu_pcu_instr_data[0] ), 
.core_dbg_mode (core_dbg_mode ), 
.core_priv_mode (core_priv_mode ), 
.inst_type (instr_decd_inst_type[0] ), 
.op1_src_vld (op1_src_vld[0] ), 
.op2_src_vld (op2_src_vld[0] ), 
.op3_src_vld (op3_src_vld[0] ), 
.funct_unit (instr_decd_funct[0] ), 
.sub_funct (instr_decd_sub_funct[0] ), 
.opcode (instr_decd_opcode[0] ), 
.op_signed (instr_decd_signed[0] ), 
.dest_idx (instr_decd_dest_idx[0] ), 
.op1_idx (instr_decd_src1_idx[0] ), 
.op2_idx (instr_decd_src2_idx[0] ), 
.op3_idx (instr_decd_src3_idx[0] ), 
.imm (instr_decd_imm[0] ), 
.illegal (instr_decd_illegal_pre[0] ), 
.instr_name (instr_name[0] ), 
.auipc_inst (auipc_inst_ex[0] ), 
.br_offset (br_offset ), 
.alu_shift_op (alu_shift_op ), 
.lsu_size (lsu_size_ex ), 
.cm_pop_ret_vld (cm_pop_ret_vld ), 
.cm_popretz_vld (cm_popretz_vld ), 
.cm_pop_vld (cm_pop_vld ) 
); 
assign instr_decd_op1_src[0] = {op1_src_vld[0][2:1],ifu_pcu_instr_rs1_vld[0]}; 
assign instr_decd_op2_src[0] = {op2_src_vld[0][1] ,ifu_pcu_instr_rs2_vld[0]}; 
assign instr_decd_op3_src[0] = {op3_src_vld[0]}; 
assign instr_decd_illegal[1] = instr_decd_illegal_pre[1]; 
assign instr_decd_op1_src[1] = {op1_src_vld[1][2:1],ifu_pcu_instr_rs1_vld[1]}; 
assign instr_decd_op2_src[1] = {op2_src_vld[1][1] ,ifu_pcu_instr_rs2_vld[1]}; 
assign instr_decd_op3_src[1] = {op3_src_vld[1]}; 
m130_pcu_dec1 u_m130_pcu_dec_1 ( 
.instr_data (ifu_pcu_instr_data[1] ), 
.inst_type (instr_decd_inst_type[1] ), 
.op1_src_vld (op1_src_vld[1] ), 
.op2_src_vld (op2_src_vld[1] ), 
.op3_src_vld (op3_src_vld[1] ), 
.funct_unit (instr_decd_funct[1] ), 
.sub_funct (instr_decd_sub_funct[1] ), 
.opcode (instr_decd_opcode[1] ), 
.op_signed (instr_decd_signed[1] ), 
.dest_idx (instr_decd_dest_idx[1] ), 
.op1_idx (instr_decd_src1_idx[1] ), 
.op2_idx (instr_decd_src2_idx[1] ), 
.op3_idx (instr_decd_src3_idx[1] ), 
.imm (instr_decd_imm[1] ), 
.illegal (instr_decd_illegal_pre[1] ), 
.instr_name (instr_name[1] ), 
.auipc_inst (auipc_inst_ex[1] ), 
.mv_li_inst (mv_li_inst[1] ), 
.br_offset () 
); 
assign iss_rs1_idx_ex = ifu_pcu_instr_rs1_idx; 
assign iss_rs2_idx_ex = ifu_pcu_instr_rs2_idx; 
generate 
for(i = 0; i < 2; i = i + 1) begin: ISS_IDX 
assign instr_rs1_idx_iss_ex[i] = iss_rs1_idx_ex[i][$clog2(32)-1:0]; 
assign instr_rs2_idx_iss_ex[i] = iss_rs2_idx_ex[i][$clog2(32)-1:0]; 
end 
endgenerate 
assign tm_pcu_trigger_hit_idx_ex = tm_pcu_trigger_pc_vld[0]? tm_pcu_trigger_hit_pc_idx[0] : tm_pcu_trigger_hit_addr_idx; 
assign ifu_pcu_hv_done_fnl = ifu_pcu_hv_done; 
m130_pcu_isu u_m130_pcu_isu ( 
.clk (clk ), 
.always_on_clk (always_on_clk ), 
.rst_n (rst_n ), 
.core_step_vld (core_step_vld ), 
.core_dbg_mode (core_dbg_mode ), 
.core_elp_state_vld (core_elp_state_vld ), 
.areg_init (areg_init ), 
.top_boot_pc (top_boot_pc ), 
.dbg_disable (1'b0 ), 
.core_pseudo_run_state (core_pseudo_run_state ), 
.ifu_pcu_vld (ifu_pcu_vld ), 
.pcu_ifu_rdy (pcu_ifu_rdy ), 
.ifu_pcu_instr_pc (ifu_pcu_instr_pc ), 
.ifu_pcu_instr_data (ifu_pcu_instr_data ), 
.ifu_pcu_excp_vld (ifu_pcu_excp_vld ), 
.ifu_pcu_excp_cause (ifu_pcu_excp_cause ), 
.ifu_pcu_hv_done (ifu_pcu_hv_done_fnl ), 
.ifu_pcu_hv_pc (ifu_pcu_hv_pc ), 
.pcu_ifu_call_rslv (pcu_ifu_call_rslv ), 
.pcu_ifu_ret_rslv (pcu_ifu_ret_rslv ), 
.jvt_inst_ex (jvt_inst_ex ), 
.pcu_ifu_brq_rd (pcu_ifu_brq_rd ), 
.ifu_pcu_brq_pc (ifu_pcu_brq_pc ), 
.instr_decd_inst_type (instr_decd_inst_type ), 
.instr_decd_op1_src (instr_decd_op1_src ), 
.instr_decd_op2_src (instr_decd_op2_src ), 
.instr_decd_op3_src (instr_decd_op3_src ), 
.instr_decd_funct (instr_decd_funct ), 
.instr_decd_sub_funct (instr_decd_sub_funct ), 
.instr_decd_opcode (instr_decd_opcode ), 
.instr_decd_signed (instr_decd_signed ), 
.instr_decd_dest_idx (instr_decd_dest_idx ), 
.instr_decd_src1_idx (instr_rs1_idx_iss_ex ), 
.instr_decd_src2_idx (instr_rs2_idx_iss_ex ), 
.instr_decd_src3_idx (instr_decd_src3_idx ), 
.pcu_csr_rdata_all (pcu_csr_rdata_all ), 
.csr_idx_ex (csr_idx_ex ), 
.csr_vld_ex (csr_vld_ex ), 
.ex_inst_vld (ex_inst_vld ), 
.ex_branch_status (ex_branch_status ), 
.ex_pc_addr (ex_pc_addr ), 
.instr_decd_imm (instr_decd_imm ), 
.instr_decd_illegal (instr_decd_illegal ), 
.auipc_inst_ex (auipc_inst_ex ), 
.mv_li_inst_ex (mv_li_inst ), 
.br_offset (br_offset ), 
.alu_shift_op (alu_shift_op ), 
.lsu_size_ex (lsu_size_ex ), 
.cm_pop_ret_vld (cm_pop_ret_vld ), 
.cm_popretz_vld (cm_popretz_vld ), 
.cm_pop_vld (cm_pop_vld ), 
.mrf_gpr_rd_idx (mrf_gpr_rd_idx ), 
.mrf_gpr_rd_data (mrf_gpr_rd_data ), 
.mrf_gpr_x7 (mrf_gpr_x7 ), 
.mrf_gpr_wr_idx (mrf_gpr_wr_idx ), 
.mrf_gpr_wr_data (mrf_gpr_wr_data ), 
.mrf_gpr_dst_vld (mrf_gpr_dst_vld ), 
.pcu_exu_inst_vld (pcu_exu_inst_vld ), 
.pcu_exu_alu_op1_val (pcu_exu_alu_op1_val ), 
.pcu_exu_alu_logic_op1_val (pcu_exu_alu_logic_op1_val ), 
.pcu_exu_bru_op1_val (pcu_exu_bru_op1_val ), 
.pcu_exu_bru_eq_op1_val (pcu_exu_bru_eq_op1_val ), 
.pcu_exu_mdu_op1_val (pcu_exu_mdu_op1_val ), 
.pcu_exu_agu_op1_val (pcu_exu_agu_op1_val ), 
.pcu_exu_alu_op2_val (pcu_exu_alu_op2_val ), 
.pcu_exu_alu_logic_op2_val (pcu_exu_alu_logic_op2_val ), 
.pcu_exu_bru_op2_val (pcu_exu_bru_op2_val ), 
.pcu_exu_bru_eq_op2_val (pcu_exu_bru_eq_op2_val ), 
.pcu_exu_mdu_op2_val (pcu_exu_mdu_op2_val ), 
.pcu_exu_agu_op2_val (pcu_exu_agu_op2_val ), 
.pcu_exu_shift_shamt (pcu_exu_shift_shamt ), 
.pcu_exu_inst_op3_val (pcu_exu_inst_op3_val ), 
.pcu_exu_inst_signed (pcu_exu_inst_signed ), 
.pcu_exu_inst_funct (pcu_exu_inst_funct ), 
.pcu_exu_inst_sub_funct (pcu_exu_inst_sub_funct ), 
.pcu_exu_inst_opcode (pcu_exu_inst_opcode ), 
.pcu_exu_inst_type (pcu_exu_inst_type ), 
.pcu_exu_inst_is_ret (pcu_exu_inst_is_ret ), 
.pcu_exu_inst_pc (pcu_exu_inst_pc ), 
.pcu_exu_inst_pred_pc (pcu_exu_inst_pred_pc ), 
.exu_pcu_br_tkn (exu_pcu_br_tkn ), 
.exu_pcu_bru_flush (exu_pcu_bru_flush ), 
.exu_pcu_inst_alu_data (exu_pcu_inst_alu_data ), 
.exu_pcu_inst_alu_logic_data (exu_pcu_inst_alu_logic_data ), 
.exu_pcu_inst_mdu_data (exu_pcu_inst_mdu_data ), 
.exu_pcu_inst_tgt_pc (exu_pcu_inst_tgt_pc ), 
.pcu_csr_split_integrity (pcu_csr_split_integrity ), 
.pcu_lsu_inst_vld (pcu_lsu_inst_vld ), 
.lsu_pcu_inst_rdy (lsu_pcu_inst_rdy ), 
.pcu_lsu_inst_cmd (pcu_lsu_inst_cmd ), 
.pcu_lsu_inst_fence_op (pcu_lsu_inst_fence_op ), 
.pcu_lsu_inst_dst_vld (pcu_lsu_inst_dst_vld ), 
.pcu_lsu_inst_data (pcu_lsu_inst_data ), 
.pcu_lsu_inst_data_wb_vld (pcu_lsu_inst_data_wb_vld ), 
.pcu_lsu_inst_size (pcu_lsu_inst_size ), 
.pcu_lsu_inst_signed (pcu_lsu_inst_signed ), 
.pcu_lsu_inst_addr_fwd (pcu_lsu_inst_addr_fwd ), 
.pcu_lsu_inst_addr_fwd_val (pcu_lsu_inst_addr_fwd_val ), 
.lsu_pcu_cmt_data (lsu_pcu_cmt_data ), 
.stall_ex (stall_ex ), 
.cmt_flush (cmt_flush ), 
.cmt_flush_wo_dbg (cmt_flush_wo_dbg ), 
.cmt_flush_pc (cmt_flush_pc ), 
.flush_wb (flush_wb ), 
.flush_wb_wo_dbg (flush_wb_wo_dbg ), 
.cmt_inst_split_retire (cmt_inst_split_retire ), 
.cmt_stall (cmt_stall ), 
.ls_exe_cmt_data_vld (ls_exe_cmt_data_vld ), 
.cmt_inst_vld (cmt_inst_vld ), 
.cmt_inst_funct (cmt_inst_funct ), 
.cmt_inst_sub_funct (cmt_inst_sub_funct ), 
.cmt_inst_opcode (cmt_inst_opcode ), 
.cmt_dest_vld (cmt_dest_vld ), 
.cmt_dest (cmt_dest ), 
.cmt_pc (cmt_pc ), 
.cmt_inst_en (cmt_inst_en ), 
.cmt_uop_int_mask (cmt_uop_int_mask ), 
.cmt_excp_vld (cmt_excp_vld ), 
.cmt_excp_cause (cmt_excp_cause ), 
.cmt_trig_exp (cmt_trig_exp ), 
.cmt_trig_dbg (cmt_trig_dbg ), 
.cmt_uop_inst (cmt_uop_inst ), 
.cmt_br_tkn (cmt_br_tkn ), 
.cmt_inst_type (cmt_inst_type ), 
.cmt_csr_idx_wb (cmt_csr_idx_wb ), 
.cmt_exp_lp_inst (cmt_exp_lp_inst ), 
.cmt_lp_inst (cmt_lp_inst ), 
.cmt_dbg_disable (cmt_dbg_disable ), 
.cmt_ls_addr_write_back (cmt_ls_addr_write_back ), 
.cmt_ls_exe_2_cycs_stall_ex (cmt_ls_exe_2_cycs_stall_ex ), 
.ls_post_pre_addr_dst_idx_wb (ls_post_pre_addr_dst_idx_wb ), 
.cmt_trigger_hit_idx (pcu_tm_trigger_hit_idx ), 
.tm_pcu_trigger_ex_vld (tm_pcu_trigger_pc_vld ), 
.tm_pcu_trigger_debug_ex (tm_pcu_trigger_pc_debug ), 
.tm_pcu_trigger_hit_idx_ex (tm_pcu_trigger_hit_idx_ex ), 
.dm_pcu_resume_req (dm_pcu_resume_req ), 
.jvt_csr (jvt_csr ), 
.instr_name (instr_name ) 
); 
m130_pcu_mrf u_m130_pcu_mrf( 
.clk (clk ), 
.rst_n (rst_n ), 
.top_polarity (1'b0 ), 
.areg_init (areg_init ), 
.mrf_gpr_rd_idx (mrf_gpr_rd_idx ), 
.mrf_gpr_rd_data (mrf_gpr_rd_data ), 
.mrf_gpr_x7 (mrf_gpr_x7 ), 
.mrf_gpr_wr_vld (mrf_gpr_wr_vld ), 
.mrf_gpr_need_cleard (mrf_gpr_need_cleard ), 
.mrf_gpr_wr_idx (mrf_gpr_wr_idx ), 
.mrf_gpr_wr_data (mrf_gpr_wr_data ) 
); 
logic cmt_dbg_disbale_csr; 
assign cmt_dbg_disbale_csr = cmt_dbg_disable[0] & cmt_inst_vld[0]; 
m130_pcu_csr u_m130_pcu_csr( 
.clk (clk ) , 
.rst_n (rst_n ) , 
.top_hart_id (top_hart_id ) , 
.top_sr_lock (8'b0 ) , 
.dbg_disable (1'b0 ) , 
.core_dbg_mode (core_dbg_mode ) , 
.ebreak_dbg_vld (ebreak_dbg_vld ) , 
.debug_mode_enter (debug_mode_enter ) , 
.mem_map_mtime (mem_map_mtime ) , 
.core_priv_mode (core_priv_mode ) , 
.cmt_inst_retire (cmt_inst_retire ) , 
.dcsr_stopcount (dcsr_stopcount ) , 
.dcsr_stoptime (pcu_clic_stoptime ) , 
.pcu_csr_vld (pcu_csr_vld ) , 
.pcu_csr_opcode (pcu_csr_opcode ) , 
.pcu_csr_index (pcu_csr_index ) , 
.pcu_csr_wdata (pcu_csr_wdata ) , 
.pcu_csr_has_write_op_ex (pcu_csr_has_write_op_ex ) , 
.csr_par_rdata (csr_par_rdata ) , 
.csr_par_match (csr_par_match ) , 
.csr_access_excp (csr_access_excp ) , 
.jvt_csr (jvt_csr ) , 
.mlpcfg (mlpcfg ) 
); 
assign core_deep_sleep = mlpcfg[0]; 
m130_pcu_cmt u_m130_pcu_cmt( 
.clk (clk ), 
.always_on_clk (always_on_clk ), 
.rst_n (rst_n ), 
.top_boot_pc (top_boot_pc ), 
.core_init_busy (core_init_busy ), 
.core_priv_mode (core_priv_mode ), 
.core_dbg_mode (core_dbg_mode ), 
.core_sleep_mode (core_sleep_mode ), 
.core_sleep_wakeup (core_sleep_wakeup ), 
.core_locked (core_locked ), 
.mss_pcu_icache_init_busy (mss_pcu_icache_init_busy ), 
.top_mstatus_be (top_mstatus_be ), 
.core_step_vld (core_step_vld ), 
.ifu_idle (ifu_idle ), 
.lsu_idle (lsu_idle ), 
.mss_idle (mss_idle ), 
.bmu_idle (bmu_idle ), 
.areg_init (areg_init ), 
.top_sr_lock (8'b0 ), 
.dbg_disable (1'b0 ), 
.gpr_cleared_in_trap (1'b0 ), 
.random_exe_stall ( ), 
.priv_mode_parity_err (priv_mode_parity_err ), 
.cmt_acc_bus_err (cmt_acc_bus_err ), 
.cmt_acc_pmp_err (cmt_acc_pmp_err ), 
.cmt_inst_dec_ill_err (cmt_inst_dec_ill_err ), 
.cmt_inst_lpd_flow_err (cmt_inst_lpd_flow_err ), 
.pcu_csr_sp_en (pcu_csr_sp_en ), 
.pcu_csr_split_integrity (pcu_csr_split_integrity ), 
.core_pseudo_run_state (core_pseudo_run_state ), 
.core_elp_state_vld (core_elp_state_vld ), 
.pcu_ifu_flush_vld (pcu_ifu_flush_vld ), 
.pcu_ifu_flush_pc (pcu_ifu_flush_pc ), 
.pcu_ifu_stall (pcu_ifu_stall ), 
.pcu_ifu_shv_flush (pcu_ifu_shv_flush ), 
.ifu_pcu_hv_done (ifu_pcu_hv_done_fnl ), 
.ifu_pcu_hv_excp_vld (ifu_pcu_hv_excp_vld ), 
.ifu_pcu_hv_pc (ifu_pcu_hv_pc ), 
.ifu_pcu_excp_cause (ifu_pcu_excp_cause ), 
.ifu_pcu_vld_ex (ifu_pcu_vld ), 
.ifu_pcu_excp_vld_ex (ifu_pcu_excp_vld ), 
.ifu_pcu_instr_pc (ifu_pcu_instr_pc ), 
.pcu_lsu_flush_st_inst_wb (pcu_lsu_flush_st_inst_wb ), 
.pcu_lsu_flush (pcu_lsu_flush ), 
.pcu_lsu_non_spec (pcu_lsu_non_spec ), 
.lsu_pcu_non_flush_infly (lsu_pcu_non_flush_infly ), 
.wb_branch_status (wb_branch_status ), 
.wb_inst_vld (wb_inst_vld ), 
.wb_pc_addr (wb_pc_addr ), 
.cmt_flush (cmt_flush ), 
.cmt_flush_wo_dbg (cmt_flush_wo_dbg ), 
.cmt_flush_pc (cmt_flush_pc ), 
.cmt_stall (cmt_stall ), 
.flush_wb (flush_wb ), 
.flush_wb_wo_dbg (flush_wb_wo_dbg ), 
.instr_decd_funct (instr_decd_funct ), 
.stall_ex (stall_ex ), 
.jvt_inst_ex (jvt_inst_ex ), 
.cmt_inst_split_retire (cmt_inst_split_retire ), 
.cmt_inst_retire (cmt_inst_retire ), 
.ls_exe_cmt_data_vld (ls_exe_cmt_data_vld ), 
.cmt_inst_vld (cmt_inst_vld ), 
.cmt_inst_funct (cmt_inst_funct ), 
.cmt_inst_sub_funct (cmt_inst_sub_funct ), 
.cmt_inst_opcode (cmt_inst_opcode ), 
.cmt_dest_vld (cmt_dest_vld ), 
.cmt_dest (cmt_dest ), 
.cmt_pc (cmt_pc ), 
.cmt_inst_en (cmt_inst_en ), 
.cmt_uop_int_mask (cmt_uop_int_mask ), 
.cmt_trig_exp (cmt_trig_exp ), 
.cmt_trig_dbg (cmt_trig_dbg ), 
.cmt_uop_inst (cmt_uop_inst ), 
.cmt_br_tkn (cmt_br_tkn ), 
.cmt_inst_type (cmt_inst_type ), 
.cmt_exp_lp_inst (cmt_exp_lp_inst ), 
.cmt_lp_inst (cmt_lp_inst ), 
.cmt_ls_addr_write_back (cmt_ls_addr_write_back ), 
.cmt_ls_exe_2_cycs_stall_ex(cmt_ls_exe_2_cycs_stall_ex ), 
.ls_post_pre_addr_dst_idx_wb (ls_post_pre_addr_dst_idx_wb ), 
.cmt_excp_vld (cmt_excp_vld ), 
.cmt_excp_cause (cmt_excp_cause[11:0] ), 
.exu_pcu_inst_wb_stall (exu_pcu_inst_wb_stall ), 
.exu_pcu_inst_alu_data (exu_pcu_inst_alu_data ), 
.exu_pcu_inst_mdu_data (exu_pcu_inst_mdu_data ), 
.exu_pcu_bru_flush (exu_pcu_bru_flush ), 
.exu_pcu_bru_flush_pc (exu_pcu_bru_flush_pc ), 
.lsu_pcu_cmt (lsu_pcu_cmt ), 
.lsu_pcu_cmt_data (lsu_pcu_cmt_data ), 
.lsu_pcu_cmt_excp (lsu_pcu_cmt_excp ), 
.lsu_pcu_cmt_excp_cause (lsu_pcu_cmt_excp_cause ), 
.lsu_pcu_cmt_excp_addr (lsu_pcu_cmt_excp_addr ), 
.lsu_pcu_cmt_trig_dbg (lsu_pcu_cmt_trig_dbg ), 
.mrf_gpr_wr_vld (mrf_gpr_wr_vld ), 
.mrf_gpr_wr_idx (mrf_gpr_wr_idx ), 
.mrf_gpr_wr_data (mrf_gpr_wr_data ), 
.mrf_gpr_x7 (mrf_gpr_x7 ), 
.mrf_gpr_dst_vld (mrf_gpr_dst_vld ), 
.mrf_gpr_need_cleard (mrf_gpr_need_cleard ), 
.csr_idx_ex (csr_idx_ex ), 
.csr_vld_ex (csr_vld_ex ), 
.pcu_csr_has_write_op_ex (pcu_csr_has_write_op_ex ), 
.csr_oth_match (csr_oth_match ), 
.csr_oth_rdata (csr_oth_rdata ), 
.cmt_csr_idx_wb (cmt_csr_idx_wb ), 
.pcu_csr_vld (pcu_csr_vld ), 
.pcu_csr_opcode (pcu_csr_opcode ), 
.pcu_csr_index (pcu_csr_index ), 
.pcu_csr_rdata_all (pcu_csr_rdata_all ) , 
.pcu_csr_wdata (pcu_csr_wdata ), 
.pcu_csr_excpt (pcu_csr_excpt ), 
.pcu_csr_match_local (pcu_csr_match_local ), 
.ebreak_dbg_vld (ebreak_dbg_vld ), 
.debug_mode_enter (debug_mode_enter ), 
.dcsr_stopcount (dcsr_stopcount ), 
.dcsr_stoptime (pcu_clic_stoptime ), 
.pcu_csr_mprv (pcu_csr_mprv ), 
.pcu_csr_mpp (pcu_csr_mpp ), 
.pcu_csr_mstatus_be (pcu_csr_mstatus_be ), 
.dm_pcu_halt_req (dm_pcu_halt_req ), 
.dm_pcu_halt_on_reset (dm_pcu_halt_on_reset ), 
.dm_pcu_resume_req (dm_pcu_resume_req ), 
.pcu_dm_halted (pcu_dm_halted ), 
.pcu_dm_havereset (pcu_dm_havereset ), 
.pcu_dm_unavail (pcu_dm_unavail ), 
.dm_pcu_ack_havereset (dm_pcu_ack_havereset ), 
.pcu_dm_cmd_done (pcu_dm_cmd_done ), 
.pcu_dm_bus_err (pcu_dm_bus_err ), 
.pcu_dm_cmd_excp (pcu_dm_cmd_excp ), 
.dm_pcu_dsch0_write (dm_pcu_dsch0_write ), 
.dm_pcu_dsch0_wdata (dm_pcu_dsch0_wdata ), 
.pcu_dm_dsch0_rdata (pcu_dm_dsch0_rdata ), 
.pcu_tm_trap_vld (pcu_tm_trap_vld ), 
.pcu_tm_trigger_hit (pcu_tm_trigger_hit ), 
.pcu_tm_trap_cause (pcu_tm_trap_cause ), 
.pcu_tm_inst_retire (pcu_tm_inst_retire ), 
.pcu_tm_tcontrol_mte (pcu_tm_tcontrol_mte ), 
.clic_pcu_nmi_vld (1'b0 ), 
.clic_pcu_int_vld (clic_pcu_int_vld ), 
.clic_pcu_int_id (clic_pcu_int_id ), 
.clic_pcu_int_lvl (clic_pcu_int_lvl ), 
.clic_pcu_int_shv (clic_pcu_int_shv ), 
.clic_pcu_int_priv (clic_pcu_int_priv ), 
.pcu_clic_rsp_clr_ip (pcu_clic_rsp_clr_ip ), 
.pcu_clic_rsp_id (pcu_clic_rsp_id ), 
.pcu_clic_rsp_vld (pcu_clic_rsp_vld ) 
); 
assign pcu_lsu_inst_amo_op = instr_decd_opcode[0][2:0]; 
assign pcu_lsu_inst_exclusive = instr_decd_opcode[0][0] & (|instr_decd_sub_funct[0][3:2]) & !instr_decd_funct[0][FU_ALU_BIT]; 
assign pcu_lsu_inst_ord_aq = ifu_pcu_instr_data[0][26]; 
assign pcu_lsu_inst_ord_rl = ifu_pcu_instr_data[0][25]; 
assign pcu_exu_sub_funct_wb = cmt_inst_sub_funct; 
assign pcu_exu_opcode_wb = cmt_inst_opcode; 
assign pcu_csr_has_write_op_ex = (ifu_pcu_instr_data[0][13:12] == 2'b01) | 
(ifu_pcu_instr_data[0][13] == 1'b1) & (|ifu_pcu_instr_data[0][19:15]) ; 
endmodule
 
 
module m130_pcu_dec0( 
input logic [31:0] instr_data , 
input logic core_dbg_mode , 
input logic [1:0] core_priv_mode , 
output logic inst_type, 
output logic [127:0] instr_name, 
output logic [31:0] imm, 
output logic [4:0] dest_idx, 
output logic [4:0] op1_idx, 
output logic [4:0] op2_idx, 
output logic [4:0] op3_idx, 
output logic [2:0] op1_src_vld, 
output logic [1:0] op2_src_vld, 
output logic [1:0] op3_src_vld, 
output logic [7:0] funct_unit, 
output logic op_signed, 
output logic [3:0] sub_funct, 
output logic [7:0] opcode, 
output logic illegal, 
output logic auipc_inst, 
output logic [31:0] br_offset, 
output logic alu_shift_op, 
output logic [1:0] lsu_size, 
output logic cm_pop_ret_vld, 
output logic cm_popretz_vld, 
output logic cm_pop_vld 
); 
always_comb begin 
instr_name = "ILL"; 
imm = 32'hx; 
dest_idx = 5'b0; 
op1_idx = instr_data[19:15]; 
op2_idx = instr_data[24:20]; 
op3_idx = instr_data[31:27]; 
op1_src_vld = 3'b0; 
op2_src_vld = 2'b0; 
op3_src_vld = 2'b0; 
funct_unit = 8'h1; 
op_signed = 1'b1; 
sub_funct = 4'hx; 
opcode = 8'h0; 
illegal = 1'b0; 
auipc_inst = 1'b0; 
br_offset = 32'bx; 
alu_shift_op = 1'b0; 
lsu_size = 2'h0; 
cm_pop_ret_vld = 1'h0; 
cm_popretz_vld = 1'h0; 
cm_pop_vld = 1'h0; 
casez(instr_data[31:0]) 
32'b???????_?????_?????_???_?????_0110111 : begin 
instr_name = "LUI"; 
imm = {instr_data[31:12],12'b0}; 
dest_idx = instr_data[11:7]; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b???????_?????_?????_???_?????_0010111 : begin 
instr_name = "AUIPC"; 
imm = {instr_data[31:12],12'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b100; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
illegal = core_dbg_mode; 
auipc_inst = 1'b1; 
end 
32'b???????_?????_?????_???_?????_1101111 : begin 
instr_name = "JAL"; 
imm = {{12{instr_data[31]}},instr_data[19:12],instr_data[20],instr_data[30:21],1'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b100; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0001; 
illegal = core_dbg_mode; 
end 
32'b???????_?????_?????_000_?????_1100111 : begin 
instr_name = "JALR"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0100; 
illegal = core_dbg_mode; 
end 
32'b???????_?????_?????_000_?????_1100011 : begin 
instr_name = "BEQ"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000001; 
illegal = core_dbg_mode; 
br_offset = {{20{instr_data[31]}},instr_data[7],instr_data[30:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_001_?????_1100011 : begin 
instr_name = "BNE"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
illegal = core_dbg_mode; 
br_offset = {{20{instr_data[31]}},instr_data[7],instr_data[30:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_100_?????_1100011 : begin 
instr_name = "BLT"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00001000; 
illegal = core_dbg_mode; 
br_offset = {{20{instr_data[31]}},instr_data[7],instr_data[30:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_101_?????_1100011 : begin 
instr_name = "BGE"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000100; 
illegal = core_dbg_mode; 
br_offset = {{20{instr_data[31]}},instr_data[7],instr_data[30:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_110_?????_1100011 : begin 
instr_name = "BLTU"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
op_signed = 1'b0; 
sub_funct = 4'b0010; 
opcode = 8'b00001000; 
illegal = core_dbg_mode; 
br_offset = {{20{instr_data[31]}},instr_data[7],instr_data[30:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_111_?????_1100011 : begin 
instr_name = "BGEU"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
op_signed = 1'b0; 
sub_funct = 4'b0010; 
opcode = 8'b00000100; 
illegal = core_dbg_mode; 
br_offset = {{20{instr_data[31]}},instr_data[7],instr_data[30:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_000_?????_0000011 : begin 
instr_name = "LB"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
end 
32'b???????_?????_?????_001_?????_0000011 : begin 
instr_name = "LH"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b???????_?????_?????_010_?????_0000011 : begin 
instr_name = "LW"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b???????_?????_?????_100_?????_0000011 : begin 
instr_name = "LBU"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
end 
32'b???????_?????_?????_101_?????_0000011 : begin 
instr_name = "LHU"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b???????_?????_?????_000_?????_0100011 : begin 
instr_name = "SB"; 
imm = {{20{instr_data[31]}},instr_data[31:25],instr_data[11:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
end 
32'b???????_?????_?????_001_?????_0100011 : begin 
instr_name = "SH"; 
imm = {{20{instr_data[31]}},instr_data[31:25],instr_data[11:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b???????_?????_?????_010_?????_0100011 : begin 
instr_name = "SW"; 
imm = {{20{instr_data[31]}},instr_data[31:25],instr_data[11:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b???????_?????_?????_000_?????_0010011 : begin 
instr_name = "ADDI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b???????_?????_?????_010_?????_0010011 : begin 
instr_name = "SLTI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b???????_?????_?????_011_?????_0010011 : begin 
instr_name = "SLTIU"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b???????_?????_?????_100_?????_0010011 : begin 
instr_name = "XORI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b???????_?????_?????_110_?????_0010011 : begin 
instr_name = "ORI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b???????_?????_?????_111_?????_0010011 : begin 
instr_name = "ANDI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b0000000_?????_?????_001_?????_0010011 : begin 
instr_name = "SLLI"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00011000; 
end 
32'b0000000_?????_?????_101_?????_0010011 : begin 
instr_name = "SRLI"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00010000; 
end 
32'b0100000_?????_?????_101_?????_0010011 : begin 
instr_name = "SRAI"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00100000; 
end 
32'b0000000_?????_?????_000_?????_0110011 : begin 
instr_name = "ADD"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b0100000_?????_?????_000_?????_0110011 : begin 
instr_name = "SUB"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b0000000_?????_?????_001_?????_0110011 : begin 
instr_name = "SLL"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00011000; 
end 
32'b0000000_?????_?????_010_?????_0110011 : begin 
instr_name = "SLT"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0000000_?????_?????_011_?????_0110011 : begin 
instr_name = "SLTU"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0000000_?????_?????_100_?????_0110011 : begin 
instr_name = "XOR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b0000000_?????_?????_101_?????_0110011 : begin 
instr_name = "SRL"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00010000; 
end 
32'b0100000_?????_?????_101_?????_0110011 : begin 
instr_name = "SRA"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00100000; 
end 
32'b0000000_?????_?????_110_?????_0110011 : begin 
instr_name = "OR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b0000000_?????_?????_111_?????_0110011 : begin 
instr_name = "AND"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b???????_?????_?????_000_?????_0001111 : begin 
instr_name = "FENCE"; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0001; 
opcode = {5'b00000,|instr_data[27:24] & |instr_data[23:20],|instr_data[27:24] & |instr_data[23:20],|instr_data[27:24] & |instr_data[23:20]}; 
illegal = (instr_data[31:28]!= 4'b1000) & (instr_data[31:28] != 4'b0000); 
end 
32'b0000000_00000_00000_000_00000_1110011 : begin 
instr_name = "ECALL"; 
funct_unit = 8'b00010000; 
op_signed = 1'bx; 
sub_funct = 4'b0010; 
opcode = 8'b00000001; 
illegal = core_dbg_mode; 
end 
32'b0000000_00001_00000_000_00000_1110011 : begin 
instr_name = "EBREAK"; 
funct_unit = 8'b00010000; 
op_signed = 1'bx; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
illegal = core_dbg_mode; 
end 
32'b0011000_00010_00000_000_00000_1110011 : begin 
instr_name = "MRET"; 
funct_unit = 8'b00010000; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
illegal = core_dbg_mode | !(&core_priv_mode); 
end 
32'b0001000_00101_00000_000_00000_1110011 : begin 
instr_name = "WFI"; 
funct_unit = 8'b00010000; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
illegal = !(&core_priv_mode); 
end 
32'b???????_?????_?????_001_?????_0001111 : begin 
instr_name = "Fence.I"; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b???????_?????_?????_001_?????_1110011 : begin 
instr_name = "CSRRW"; 
imm = 32'h0; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00010001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
lsu_size = 2'h2; 
end 
32'b???????_?????_?????_010_?????_1110011 : begin 
instr_name = "CSRRS"; 
imm = 32'h0; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00010001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
lsu_size = 2'h2; 
end 
32'b???????_?????_?????_011_?????_1110011 : begin 
instr_name = "CSRRC"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00010001; 
op_signed = 1'bx; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b???????_?????_?????_101_?????_1110011 : begin 
instr_name = "CSRRWI"; 
imm = {27'b0,instr_data[19:15]}; 
dest_idx = instr_data[11:7]; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00010001; 
op_signed = 1'bx; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b???????_?????_?????_110_?????_1110011 : begin 
instr_name = "CSRRSI"; 
imm = {27'b0,instr_data[19:15]}; 
dest_idx = instr_data[11:7]; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00010001; 
op_signed = 1'bx; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b???????_?????_?????_111_?????_1110011 : begin 
instr_name = "CSRRCI"; 
imm = {27'b0,instr_data[19:15]}; 
dest_idx = instr_data[11:7]; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00010001; 
op_signed = 1'bx; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b0000001_?????_?????_000_?????_0110011 : begin 
instr_name = "MUL"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b0000001_?????_?????_001_?????_0110011 : begin 
instr_name = "MULH"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
sub_funct = 4'b0001; 
opcode = 8'b00000000; 
end 
32'b0000001_?????_?????_010_?????_0110011 : begin 
instr_name = "MULHSU"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b0000001_?????_?????_011_?????_0110011 : begin 
instr_name = "MULHU"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00000000; 
end 
32'b0000001_?????_?????_100_?????_0110011 : begin 
instr_name = "DIV"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
sub_funct = 4'b0010; 
end 
32'b0000001_?????_?????_101_?????_0110011 : begin 
instr_name = "DIVU"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
op_signed = 1'b0; 
sub_funct = 4'b0010; 
end 
32'b0000001_?????_?????_110_?????_0110011 : begin 
instr_name = "REM"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
sub_funct = 4'b0100; 
end 
32'b0000001_?????_?????_111_?????_0110011 : begin 
instr_name = "REMU"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
end 
32'b0100000_?????_?????_111_?????_0110011 : begin 
instr_name = "andn"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00010001; 
end 
32'b0100100_?????_?????_001_?????_0110011 : begin 
instr_name = "bclr"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00100000; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b0100100_?????_?????_001_?????_0010011 : begin 
instr_name = "bclri"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00100000; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b0100100_?????_?????_101_?????_0110011 : begin 
instr_name = "bext"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00100000; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0100100_?????_?????_101_?????_0010011 : begin 
instr_name = "bexti"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00100000; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0110100_?????_?????_001_?????_0110011 : begin 
instr_name = "binv"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0110100_?????_?????_001_?????_0010011 : begin 
instr_name = "binvi"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0010100_?????_?????_001_?????_0110011 : begin 
instr_name = "bset"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b0010100_?????_?????_001_?????_0010011 : begin 
instr_name = "bseti"; 
imm = {27'bx,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b0000101_?????_?????_001_?????_0110011 : begin 
instr_name = "clmul"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0000101_?????_?????_011_?????_0110011 : begin 
instr_name = "clmulh"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00000101; 
end 
32'b0000101_?????_?????_010_?????_0110011 : begin 
instr_name = "clmulr"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000100; 
op_signed = 1'bx; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0110000_00000_?????_001_?????_0010011 : begin 
instr_name = "clz"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0010; 
opcode = 8'b00000001; 
end 
32'b0110000_00010_?????_001_?????_0010011 : begin 
instr_name = "cpop"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
end 
32'b0110000_00001_?????_001_?????_0010011 : begin 
instr_name = "ctz"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00100000; 
op_signed = 1'bx; 
sub_funct = 4'b0010; 
opcode = 8'b00000000; 
end 
32'b0000101_?????_?????_110_?????_0110011 : begin 
instr_name = "max"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0000101_?????_?????_111_?????_0110011 : begin 
instr_name = "maxu"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0000101_?????_?????_100_?????_0110011 : begin 
instr_name = "min"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00010000; 
end 
32'b0000101_?????_?????_101_?????_0110011 : begin 
instr_name = "minu"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00010000; 
end 
32'b0010100_00111_?????_101_?????_0010011 : begin 
instr_name = "orc.b"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00000001; 
end 
32'b0100000_?????_?????_110_?????_0110011 : begin 
instr_name = "orn"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00010010; 
end 
32'b0110100_11000_?????_101_?????_0010011 : begin 
instr_name = "rev8"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00000010; 
end 
32'b0110000_?????_?????_001_?????_0110011 : begin 
instr_name = "rol"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b01001000; 
end 
32'b0110000_?????_?????_101_?????_0110011 : begin 
instr_name = "ror"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b01000000; 
end 
32'b0110000_?????_?????_101_?????_0010011 : begin 
instr_name = "rori"; 
imm = {27'b0,instr_data[24:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b01000000; 
end 
32'b0110000_00100_?????_001_?????_0010011 : begin 
instr_name = "sext.b"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00010000; 
end 
32'b0110000_00101_?????_001_?????_0010011 : begin 
instr_name = "sext.h"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b0010000_?????_?????_010_?????_0110011 : begin 
instr_name = "sh1add"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00100001; 
end 
32'b0010000_?????_?????_100_?????_0110011 : begin 
instr_name = "sh2add"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b01000001; 
end 
32'b0010000_?????_?????_110_?????_0110011 : begin 
instr_name = "sh3add"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b01100001; 
end 
32'b0100000_?????_?????_100_?????_0110011 : begin 
instr_name = "xnor"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00001000; 
end 
32'b0000100_00000_?????_100_?????_0110011 : begin 
instr_name = "zext.h"; 
imm = 'bx; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b????_????_????_????_000_???_???_??_???_00 : begin 
instr_name = "C.ADDI4SPN"; 
imm = {22'b0,instr_data[10:7],instr_data[12:11],instr_data[5],instr_data[6],2'b0}; 
dest_idx = {2'b01,instr_data[4:2]}; 
op1_idx = 5'b10; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
illegal = instr_data[12:5] == 8'b0; 
end 
32'b????_????_????_????_010_???_???_??_???_00 : begin 
instr_name = "C.LW"; 
imm = {25'b0,instr_data[5],instr_data[12:10],instr_data[6],2'b0}; 
dest_idx = {2'b01,instr_data[4:2]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b????_????_????_????_110_???_???_??_???_00 : begin 
instr_name = "C.SW"; 
imm = {25'b0,instr_data[5],instr_data[12:10],instr_data[6],2'b0}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b????_????_????_????_000_???_???_??_???_01 : begin 
instr_name = "C.ADDI"; 
imm = {{27{instr_data[12]}},instr_data[6:2]}; 
dest_idx = ({instr_data[12],instr_data[6:2]} ==6'b0) ? 5'b0 :instr_data[11:7]; 
op1_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b????_????_????_????_001_???_???_??_???_01 : begin 
instr_name = "C.JAL"; 
imm = {{21{instr_data[12]}}, instr_data[8], instr_data[10:9], instr_data[6], instr_data[7], instr_data[2], instr_data[11], instr_data[5:3],1'b0}; 
dest_idx = 5'b1; 
op1_src_vld = 3'b100; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0001; 
illegal = core_dbg_mode; 
end 
32'b????_????_????_????_010_???_???_??_???_01 : begin 
instr_name = "C.LI"; 
imm = {{27{instr_data[12]}},instr_data[6:2]}; 
dest_idx = instr_data[11:7]; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b????_????_????_????_011_???_???_??_???_01 : begin 
if (instr_data[11:7] ==5'h2) begin 
instr_name = "C.ADDI16SP"; 
imm = {{23{instr_data[12]}},instr_data[4:3],instr_data[5],instr_data[2],instr_data[6],4'b0}; 
dest_idx = 5'b10; 
op1_idx = 5'b10; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
illegal = ({instr_data[12],instr_data[6:2]}== 6'b0); 
end else begin 
instr_name = "C.LUI"; 
imm = {{15{instr_data[12]}},instr_data[6:2],12'b0}; 
dest_idx = instr_data[11:7]; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
illegal = ({instr_data[12],instr_data[6:2]}== 6'b0); 
end 
end 
32'b????_????_????_????_100_000_???_??_???_01 : begin 
instr_name = "C.SRLI"; 
imm = {27'bx,instr_data[6:2]}; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00010000; 
end 
32'b????_????_????_????_100_001_???_??_???_01 : begin 
instr_name = "C.SRAI"; 
imm = {27'bx,instr_data[6:2]}; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00100000; 
end 
32'b????_????_????_????_100_?10_???_??_???_01 : begin 
instr_name = "C.ANDI"; 
imm = {{27{instr_data[12]}},instr_data[6:2]}; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b????_????_????_????_100_011_???_00_???_01 : begin 
instr_name = "C.SUB"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b????_????_????_????_100_011_???_01_???_01 : begin 
instr_name = "C.XOR"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b????_????_????_????_100_011_???_10_???_01 : begin 
instr_name = "C.OR"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b????_????_????_????_100_011_???_11_???_01 : begin 
instr_name = "C.AND"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b????_????_????_????_101_???_???_??_???_01 : begin 
instr_name = "C.J"; 
imm = {{21{instr_data[12]}},instr_data[8],instr_data[10:9],instr_data[6],instr_data[7],instr_data[2],instr_data[11],instr_data[5:3],1'b0}; 
op1_idx = 'b0; 
op1_src_vld = 3'b100; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0001; 
illegal = core_dbg_mode; 
end 
32'b????_????_????_????_110_???_???_??_???_01 : begin 
instr_name = "C.BEQZ"; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000001; 
illegal = core_dbg_mode; 
br_offset = {{24{instr_data[12]}},instr_data[6:5],instr_data[2],instr_data[11:10],instr_data[4:3],1'b0}; 
end 
32'b????_????_????_????_111_???_???_??_???_01 : begin 
instr_name = "C.BNEZ"; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
illegal = core_dbg_mode; 
br_offset = {{24{instr_data[12]}},instr_data[6:5],instr_data[2],instr_data[11:10],instr_data[4:3],1'b0}; 
end 
32'b????_????_????_????_000_0??_???_??_???_10 : begin 
instr_name = "C.SLLI"; 
imm = {26'bx,instr_data[12],instr_data[6:2]}; 
dest_idx = ({instr_data[12],instr_data[6:2]} ==6'b0) ? 5'b0 :instr_data[11:7]; 
op1_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00011000; 
end 
32'b????_????_????_????_010_???_???_??_???_10 : begin 
instr_name = "C.LWSP"; 
imm = {24'b0,instr_data[3:2],instr_data[12],instr_data[6:4],2'b0}; 
dest_idx = instr_data[11:7]; 
op1_idx = 5'b10; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
illegal = (instr_data[11:7]==5'b0); 
lsu_size = 2'h2; 
end 
32'b????_????_????_????_100_0??_???_??_???_10 : begin 
if (instr_data[6:2] == 5'b0) begin 
instr_name = "C.JR"; 
op1_idx = instr_data[11:7]; 
op2_idx = 'b0; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0100; 
illegal = core_dbg_mode|(~|instr_data[11:7]); 
end else begin 
instr_name = "C.MV"; 
dest_idx = instr_data[11:7]; 
op2_idx = instr_data[6:2]; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
end 
32'b????_????_????_????_100_1??_???_??_???_10 : begin 
if (instr_data[11:2]==10'b0) begin 
instr_name = "C.EBREAK"; 
funct_unit = 8'b00010000; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
illegal = core_dbg_mode; 
end else if (instr_data[6:2]==5'b0) begin 
instr_name = "C.JALR"; 
dest_idx = 5'b1; 
op1_idx = instr_data[11:7]; 
op2_idx = 'b0; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0100; 
illegal = core_dbg_mode; 
end else begin 
instr_name = "C.ADD"; 
dest_idx = instr_data[11:7]; 
op1_idx = instr_data[11:7]; 
op2_idx = instr_data[6:2]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
end 
32'b????_????_????_????_110_???_???_??_???_10 : begin 
instr_name = "C.SWSP"; 
imm = {24'b0,instr_data[8:7],instr_data[12:9],2'b0}; 
op1_idx = 5'b10; 
op2_idx = instr_data[6:2]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b0000111_?????_?????_101_?????_0110011 : begin 
instr_name = "czero.eqz"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00000100; 
end 
32'b0000111_?????_?????_111_?????_0110011 : begin 
instr_name = "czero.nez"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00001000; 
end 
32'b????????????????_100_000_???_??_???_00 : begin 
instr_name = "c.lbu"; 
imm = {30'b0,instr_data[5],instr_data[6]}; 
dest_idx = {2'b01,instr_data[4:2]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00001000; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
end 
32'b????????????????_100_001_???_1?_???_00 : begin 
instr_name = "c.lh"; 
imm = {30'b0,instr_data[5],1'b0}; 
dest_idx = {2'b01,instr_data[4:2]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b????????????????_100_001_???_0?_???_00 : begin 
instr_name = "c.lhu"; 
imm = {30'b0,instr_data[5],1'b0}; 
dest_idx = {2'b01,instr_data[4:2]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00001000; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b????????????????_100_010_???_??_???_00 : begin 
instr_name = "c.sb"; 
imm = {30'b0,instr_data[5],instr_data[6]}; 
dest_idx = 'b0; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
end 
32'b????????????????_100_011_???_0?_???_00 : begin 
instr_name = "c.sh"; 
imm = {30'b0,instr_data[5],1'b0}; 
dest_idx = 'b0; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b????????????????_100_111_???_11_000_01 : begin 
instr_name = "c.zext.b"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00010000; 
end 
32'b????????????????_100_111_???_11_101_01 : begin 
instr_name = "c.not"; 
imm = 32'hffff_ffff; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b????????????????_100_111_???_10_???_01 : begin 
instr_name = "c.mul"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000100; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b????????????????_101_110_00?_??_???_10 : begin 
instr_name = "cm.push"; 
imm = 'bx; 
dest_idx = 5'h0; 
op1_idx = 5'h2; 
op2_idx = 5'h1; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b10001000; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b????????????????_101_110_10?_??_???_10 : begin 
instr_name = "cm.pop"; 
imm = 'bx; 
dest_idx = 5'h1; 
op1_idx = 5'h2; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b10001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
cm_pop_vld = 1'h1; 
end 
32'b????????????????_101_111_10?_??_???_10 : begin 
instr_name = "cm.popret"; 
imm = 'bx; 
dest_idx = 5'h1; 
op1_idx = 5'h2; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b10001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
cm_pop_ret_vld = 1'h1; 
end 
32'b????????????????_101_111_00?_??_???_10 : begin 
instr_name = "cm.popretz"; 
imm = 'bx; 
dest_idx = 5'h1; 
op1_idx = 5'h2; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b10001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
cm_pop_ret_vld = 1'h1; 
cm_popretz_vld = 1'h1; 
end 
32'b????????????????_101_011_???_11_???_10 : begin 
instr_name = "cm.mva01s"; 
imm = 'bx; 
dest_idx = 5'ha; 
op2_idx = {|instr_data[9:8],instr_data[9:8]==0,instr_data[9:7]}; 
op1_src_vld = 3'b000; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b00; 
funct_unit = 8'b10000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b????????????????_101_011_???_01_???_10 : begin 
instr_name = "cm.mvsa01"; 
imm = 'bx; 
dest_idx = {|instr_data[9:8],instr_data[9:8]==0,instr_data[9:7]}; 
op2_idx = 5'ha; 
op1_src_vld = 3'b000; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b00; 
funct_unit = 8'b10000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b????????????????_101_000_???_??_???_10 : begin 
instr_name = "cm.jvt"; 
imm = {22'b0,instr_data[9:2],2'b0}; 
dest_idx = {5{|instr_data[9:7]}} & 5'h1; 
op1_idx = 'b0; 
op2_idx = 'b0; 
op1_src_vld = 3'b000; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000010; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
illegal = core_dbg_mode; 
end 
32'b????????????????_100_111_???_11_001_01 : begin 
instr_name = "c.sext.b"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00010000; 
end 
32'b????????????????_100_111_???_11_010_01 : begin 
instr_name = "c.zext.h"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b????????????????_100_111_???_11_011_01 : begin 
instr_name = "c.sext.h"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b??_?????_?????_?????_000_?????_0101011 : begin 
instr_name = "ADDSHF"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b10000001; 
sub_funct = 4'b0010; 
opcode = {1'b0,instr_data[31:30] == 2'b11,instr_data[31:30] == 2'b10,~instr_data[31],instr_data[31:30] == 2'b00,3'b000}; 
illegal = instr_data[29:25] == 5'b0; 
alu_shift_op = 1'b1; 
end 
32'b??_?????_?????_?????_001_?????_0101011 : begin 
instr_name = "SUBSHF"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b10000001; 
sub_funct = 4'b0010; 
opcode = {1'b0,instr_data[31:30] == 2'b11,instr_data[31:30] == 2'b10,~instr_data[31],instr_data[31:30] == 2'b00,3'b000}; 
illegal = instr_data[29:25] == 5'b0; 
alu_shift_op = 1'b1; 
end 
32'b??_?????_?????_?????_100_?????_0101011 : begin 
instr_name = "ANDSHF"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = {1'b0,instr_data[31:30] == 2'b11,instr_data[31:30] == 2'b10,~instr_data[31],instr_data[31:30] == 2'b00,3'b001}; 
illegal = instr_data[29:25] == 5'b0; 
alu_shift_op = 1'b1; 
end 
32'b??_?????_?????_?????_011_?????_0101011 : begin 
instr_name = "XORSHF"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = {1'b0,instr_data[31:30] == 2'b11,instr_data[31:30] == 2'b10,~instr_data[31],instr_data[31:30] == 2'b00,3'b100}; 
illegal = instr_data[29:25] == 5'b0; 
alu_shift_op = 1'b1; 
end 
32'b??_?????_?????_?????_010_?????_0101011 : begin 
instr_name = "ORSHF"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = {1'b0,instr_data[31:30] == 2'b11,instr_data[31:30] == 2'b10,~instr_data[31],instr_data[31:30] == 2'b00,3'b010}; 
illegal = instr_data[29:25] == 5'b0; 
alu_shift_op = 1'b1; 
end 
32'b???????_?????_?????_000_?????_0001011 : begin 
instr_name = "BEQI"; 
imm = {{24{instr_data[31]}},instr_data[31:30],instr_data[24:20],instr_data[7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000001; 
illegal = core_dbg_mode; 
br_offset = {{22{instr_data[29]}},instr_data[29:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_011_?????_0001011 : begin 
instr_name = "BGEI"; 
imm = {{24{instr_data[31]}},instr_data[31:30],instr_data[24:20],instr_data[7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000100; 
illegal = core_dbg_mode; 
br_offset = {{22{instr_data[29]}},instr_data[29:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_101_?????_0001011 : begin 
instr_name = "BGEUI"; 
imm = {{24{1'b0}},instr_data[31:30],instr_data[24:20],instr_data[7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
op_signed = 1'b0; 
sub_funct = 4'b0010; 
opcode = 8'b00000100; 
illegal = core_dbg_mode; 
br_offset = {{22{instr_data[29]}},instr_data[29:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_010_?????_0001011 : begin 
instr_name = "BLTI"; 
imm = {{24{instr_data[31]}},instr_data[31:30],instr_data[24:20],instr_data[7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00001000; 
illegal = core_dbg_mode; 
br_offset = {{22{instr_data[29]}},instr_data[29:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_100_?????_0001011 : begin 
instr_name = "BLTUI"; 
imm = {{24{1'b0}},instr_data[31:30],instr_data[24:20],instr_data[7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
op_signed = 1'b0; 
sub_funct = 4'b0010; 
opcode = 8'b00001000; 
illegal = core_dbg_mode; 
br_offset = {{22{instr_data[29]}},instr_data[29:25],instr_data[11:8],1'b0}; 
end 
32'b???????_?????_?????_001_?????_0001011 : begin 
instr_name = "BNEI"; 
imm = {{24{instr_data[31]}},instr_data[31:30],instr_data[24:20],instr_data[7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b10; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
illegal = core_dbg_mode; 
br_offset = {{22{instr_data[29]}},instr_data[29:25],instr_data[11:8],1'b0}; 
end 
32'b0000_????????_?????_111_?????_0001011 : begin 
instr_name = "LB.POST"; 
imm = {{24{instr_data[27]}},instr_data[27:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
end 
32'b1000_????????_?????_111_?????_0001011 : begin 
instr_name = "LB.PRE"; 
imm = {{24{instr_data[27]}},instr_data[27:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
end 
32'b0111000_?????_?????_111_?????_0001011 : begin 
instr_name = "LBR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
end 
32'b0011000_?????_?????_111_?????_0001011 : begin 
instr_name = "LBR.POST"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
end 
32'b1011000_?????_?????_111_?????_0001011 : begin 
instr_name = "LBR.PRE"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
end 
32'b0100_????????_?????_111_?????_0001011 : begin 
instr_name = "LBU.POST"; 
imm = {{24{instr_data[27]}},instr_data[27:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
end 
32'b1100_????????_?????_111_?????_0001011 : begin 
instr_name = "LBU.PRE"; 
imm = {{24{instr_data[27]}},instr_data[27:20]}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
end 
32'b0111100_?????_?????_111_?????_0001011 : begin 
instr_name = "LBUR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
end 
32'b0011100_?????_?????_111_?????_0001011 : begin 
instr_name = "LBUR.POST"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
end 
32'b1011100_?????_?????_111_?????_0001011 : begin 
instr_name = "LBUR.PRE"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
end 
32'b0001_????????_?????_111_?????_0001011 : begin 
instr_name = "LH.POST"; 
imm = {{23{instr_data[27]}},instr_data[27:20],1'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
lsu_size = 2'h1; 
end 
32'b1001_????????_?????_111_?????_0001011 : begin 
instr_name = "LH.PRE"; 
imm = {{23{instr_data[27]}},instr_data[27:20],1'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
lsu_size = 2'h1; 
end 
32'b0111001_?????_?????_111_?????_0001011 : begin 
instr_name = "LHR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b0011001_?????_?????_111_?????_0001011 : begin 
instr_name = "LHR.POST"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
lsu_size = 2'h1; 
end 
32'b1011001_?????_?????_111_?????_0001011 : begin 
instr_name = "LHR.PRE"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
lsu_size = 2'h1; 
end 
32'b0101_????????_?????_111_?????_0001011 : begin 
instr_name = "LHU.POST"; 
imm = {{23{instr_data[27]}},instr_data[27:20],1'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
lsu_size = 2'h1; 
end 
32'b1101_????????_?????_111_?????_0001011 : begin 
instr_name = "LHU.PRE"; 
imm = {{23{instr_data[27]}},instr_data[27:20],1'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
lsu_size = 2'h1; 
end 
32'b0111101_?????_?????_111_?????_0001011 : begin 
instr_name = "LHUR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b0011101_?????_?????_111_?????_0001011 : begin 
instr_name = "LHUR.POST"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
lsu_size = 2'h1; 
end 
32'b1011101_?????_?????_111_?????_0001011 : begin 
instr_name = "LHUR.PRE"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
op_signed = 1'b0; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
lsu_size = 2'h1; 
end 
32'b0010_????????_?????_111_?????_0001011 : begin 
instr_name = "LW.POST"; 
imm = {{22{instr_data[27]}},instr_data[27:20],2'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
lsu_size = 2'h2; 
end 
32'b1010_????????_?????_111_?????_0001011 : begin 
instr_name = "LW.PRE"; 
imm = {{22{instr_data[27]}},instr_data[27:20],2'b0}; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
lsu_size = 2'h2; 
end 
32'b0111010_?????_?????_111_?????_0001011 : begin 
instr_name = "LWR"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001000; 
sub_funct = 4'b0100; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b0011010_?????_?????_111_?????_0001011 : begin 
instr_name = "LWR.POST"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b00100001; 
lsu_size = 2'h2; 
end 
32'b1011010_?????_?????_111_?????_0001011 : begin 
instr_name = "LWR.PRE"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b0100; 
opcode = 8'b01000001; 
lsu_size = 2'h2; 
end 
32'b0000_????????_?????_110_?????_0001011 : begin 
instr_name = "SB.POST"; 
imm = {{24{instr_data[27]}},instr_data[27:25],instr_data[11:7]}; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00100001; 
end 
32'b1000_????????_?????_110_?????_0001011 : begin 
instr_name = "SB.PRE"; 
imm = {{24{instr_data[27]}},instr_data[27:25],instr_data[11:7]}; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b01000001; 
end 
32'b0111000_?????_?????_110_?????_0001011 : begin 
instr_name = "SBR"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
end 
32'b0011000_?????_?????_110_?????_0001011 : begin 
instr_name = "SBR.POST"; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00100001; 
end 
32'b1011000_?????_?????_110_?????_0001011 : begin 
instr_name = "SBR.PRE"; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b01000001; 
end 
32'b0001_????????_?????_110_?????_0001011 : begin 
instr_name = "SH.POST"; 
imm = {{23{instr_data[27]}},instr_data[27:25],instr_data[11:7],1'b0}; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00100001; 
lsu_size = 2'h1; 
end 
32'b1001_????????_?????_110_?????_0001011 : begin 
instr_name = "SH.PRE"; 
imm = {{23{instr_data[27]}},instr_data[27:25],instr_data[11:7],1'b0}; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b01000001; 
lsu_size = 2'h1; 
end 
32'b0111001_?????_?????_110_?????_0001011 : begin 
instr_name = "SHR"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h1; 
end 
32'b0011001_?????_?????_110_?????_0001011 : begin 
instr_name = "SHR.POST"; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00100001; 
lsu_size = 2'h1; 
end 
32'b1011001_?????_?????_110_?????_0001011 : begin 
instr_name = "SHR.PRE"; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b01000001; 
lsu_size = 2'h1; 
end 
32'b0010_????????_?????_110_?????_0001011 : begin 
instr_name = "SW.POST"; 
imm = {{22{instr_data[27]}},instr_data[27:25],instr_data[11:7],2'b0}; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00100001; 
lsu_size = 2'h2; 
end 
32'b1010_????????_?????_110_?????_0001011 : begin 
instr_name = "SW.PRE"; 
imm = {{22{instr_data[27]}},instr_data[27:25],instr_data[11:7],2'b0}; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b01000001; 
lsu_size = 2'h2; 
end 
32'b0111010_?????_?????_110_?????_0001011 : begin 
instr_name = "SWR"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00000000; 
lsu_size = 2'h2; 
end 
32'b0011010_?????_?????_110_?????_0001011 : begin 
instr_name = "SWR.POST"; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b00100001; 
lsu_size = 2'h2; 
end 
32'b1011010_?????_?????_110_?????_0001011 : begin 
instr_name = "SWR.PRE"; 
dest_idx = instr_data[19:15]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
op3_src_vld = 2'b01; 
funct_unit = 8'b00001001; 
sub_funct = 4'b1000; 
opcode = 8'b01000001; 
lsu_size = 2'h2; 
end 
32'b111100000000_?????_110_?????_0001011 : begin 
instr_name = "REV8HW"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b01000000; 
end 
32'b111100000001_?????_110_?????_0001011 : begin 
instr_name = "REV8SLHW"; 
dest_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b10000000; 
end 
default:illegal = 1'b1; 
endcase 
end 
assign inst_type = (instr_data[1:0] == 2'b11) ? 1'b1 : 1'b0; 
endmodule
 
 
module m130_pcu_dec1( 
input logic [31:0] instr_data , 
output logic inst_type, 
output logic [127:0] instr_name, 
output logic [31:0] imm, 
output logic [4:0] dest_idx, 
output logic [4:0] op1_idx, 
output logic [4:0] op2_idx, 
output logic [4:0] op3_idx, 
output logic [2:0] op1_src_vld, 
output logic [1:0] op2_src_vld, 
output logic [1:0] op3_src_vld, 
output logic [7:0] funct_unit, 
output logic op_signed, 
output logic [3:0] sub_funct, 
output logic [7:0] opcode, 
output logic illegal, 
output logic auipc_inst, 
output logic mv_li_inst, 
output logic [31:0] br_offset 
); 
always_comb begin 
instr_name = "ILL"; 
imm = 32'hx; 
dest_idx = instr_data[11:7]; 
op1_idx = instr_data[19:15]; 
op2_idx = instr_data[24:20]; 
op3_idx = instr_data[31:27]; 
op1_src_vld = 3'b0; 
op2_src_vld = 2'b0; 
op3_src_vld = 2'b0; 
funct_unit = 8'h1; 
op_signed = 1'b1; 
sub_funct = 4'h0; 
opcode = 8'h0; 
illegal = 1'b0; 
auipc_inst = 1'b0; 
br_offset = 32'bx; 
mv_li_inst = 1'b0; 
casez(instr_data[31:0]) 
32'b???????_?????_?????_???_?????_0110111 : begin 
instr_name = "LUI"; 
imm = {instr_data[31:12],12'b0}; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00100010; 
mv_li_inst = 1'b1; 
end 
32'b???????_?????_?????_???_?????_0010111 : begin 
instr_name = "AUIPC"; 
imm = {instr_data[31:12],12'b0}; 
op1_src_vld = 3'b100; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
auipc_inst = 1'b1; 
end 
32'b???????_?????_?????_000_?????_0010011 : begin 
instr_name = "ADDI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b???????_?????_?????_010_?????_0010011 : begin 
instr_name = "SLTI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b???????_?????_?????_011_?????_0010011 : begin 
instr_name = "SLTIU"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b???????_?????_?????_100_?????_0010011 : begin 
instr_name = "XORI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b???????_?????_?????_110_?????_0010011 : begin 
instr_name = "ORI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b???????_?????_?????_111_?????_0010011 : begin 
instr_name = "ANDI"; 
imm = {{20{instr_data[31]}},instr_data[31:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b0000000_?????_?????_001_?????_0010011 : begin 
instr_name = "SLLI"; 
imm = {27'bx,instr_data[24:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00011000; 
end 
32'b0000000_?????_?????_101_?????_0010011 : begin 
instr_name = "SRLI"; 
imm = {27'bx,instr_data[24:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00010000; 
end 
32'b0100000_?????_?????_101_?????_0010011 : begin 
instr_name = "SRAI"; 
imm = {27'bx,instr_data[24:20]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00100000; 
end 
32'b0000000_?????_?????_000_?????_0110011 : begin 
instr_name = "ADD"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b0100000_?????_?????_000_?????_0110011 : begin 
instr_name = "SUB"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b0000000_?????_?????_001_?????_0110011 : begin 
instr_name = "SLL"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00011000; 
end 
32'b0000000_?????_?????_010_?????_0110011 : begin 
instr_name = "SLT"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0000000_?????_?????_011_?????_0110011 : begin 
instr_name = "SLTU"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00000100; 
end 
32'b0000000_?????_?????_100_?????_0110011 : begin 
instr_name = "XOR"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b0000000_?????_?????_101_?????_0110011 : begin 
instr_name = "SRL"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00010000; 
end 
32'b0100000_?????_?????_101_?????_0110011 : begin 
instr_name = "SRA"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00100000; 
end 
32'b0000000_?????_?????_110_?????_0110011 : begin 
instr_name = "OR"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b0000000_?????_?????_111_?????_0110011 : begin 
instr_name = "AND"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b0100000_?????_?????_111_?????_0110011 : begin 
instr_name = "andn"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00010001; 
end 
32'b0000101_?????_?????_110_?????_0110011 : begin 
instr_name = "max"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0000101_?????_?????_111_?????_0110011 : begin 
instr_name = "maxu"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00001000; 
end 
32'b0000101_?????_?????_100_?????_0110011 : begin 
instr_name = "min"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00010000; 
end 
32'b0000101_?????_?????_101_?????_0110011 : begin 
instr_name = "minu"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b0001; 
opcode = 8'b00010000; 
end 
32'b0010100_00111_?????_101_?????_0010011 : begin 
instr_name = "orc.b"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00000001; 
end 
32'b0100000_?????_?????_110_?????_0110011 : begin 
instr_name = "orn"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00010010; 
end 
32'b0110100_11000_?????_101_?????_0010011 : begin 
instr_name = "rev8"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00000010; 
end 
32'b0110000_?????_?????_001_?????_0110011 : begin 
instr_name = "rol"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b01001000; 
end 
32'b0110000_?????_?????_101_?????_0110011 : begin 
instr_name = "ror"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b01000000; 
end 
32'b0110000_?????_?????_101_?????_0010011 : begin 
instr_name = "rori"; 
imm = {27'b0,instr_data[24:20]}; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b01000000; 
end 
32'b0110000_00100_?????_001_?????_0010011 : begin 
instr_name = "sext.b"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00010000; 
end 
32'b0110000_00101_?????_001_?????_0010011 : begin 
instr_name = "sext.h"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b0010000_?????_?????_010_?????_0110011 : begin 
instr_name = "sh1add"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00100001; 
end 
32'b0010000_?????_?????_100_?????_0110011 : begin 
instr_name = "sh2add"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b01000001; 
end 
32'b0010000_?????_?????_110_?????_0110011 : begin 
instr_name = "sh3add"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b01100001; 
end 
32'b0100000_?????_?????_100_?????_0110011 : begin 
instr_name = "xnor"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00001000; 
end 
32'b0000100_00000_?????_100_?????_0110011 : begin 
instr_name = "zext.h"; 
imm = 'bx; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b????_????_????_????_000_???_???_??_???_00 : begin 
instr_name = "C.ADDI4SPN"; 
imm = {22'b0,instr_data[10:7],instr_data[12:11],instr_data[5],instr_data[6],2'b0}; 
dest_idx = {2'b01,instr_data[4:2]}; 
op1_idx = 5'b10; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
illegal = instr_data[12:5] == 8'b0; 
end 
32'b????_????_????_????_000_???_???_??_???_01 : begin 
instr_name = "C.ADDI"; 
imm = {{27{instr_data[12]}},instr_data[6:2]}; 
dest_idx = ({instr_data[12],instr_data[6:2]} ==6'b0) ? 5'b0 :instr_data[11:7]; 
op1_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
32'b????_????_????_????_010_???_???_??_???_01 : begin 
instr_name = "C.LI"; 
imm = {{27{instr_data[12]}},instr_data[6:2]}; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
mv_li_inst = 1'b1; 
opcode = 8'b00100010; 
end 
32'b????_????_????_????_011_???_???_??_???_01 : begin 
if (instr_data[11:7] ==5'h2) begin 
instr_name = "C.ADDI16SP"; 
imm = {{23{instr_data[12]}},instr_data[4:3],instr_data[5],instr_data[2],instr_data[6],4'b0}; 
dest_idx = 5'b10; 
op1_idx = 5'b10; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
illegal = ({instr_data[12],instr_data[6:2]}== 6'b0); 
end else begin 
instr_name = "C.LUI"; 
imm = {{15{instr_data[12]}},instr_data[6:2],12'b0}; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00100010; 
mv_li_inst = 1'b1; 
illegal = ({instr_data[12],instr_data[6:2]}== 6'b0); 
end 
end 
32'b????_????_????_????_100_000_???_??_???_01 : begin 
instr_name = "C.SRLI"; 
imm = {27'bx,instr_data[6:2]}; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00010000; 
end 
32'b????_????_????_????_100_001_???_??_???_01 : begin 
instr_name = "C.SRAI"; 
imm = {27'bx,instr_data[6:2]}; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00100000; 
end 
32'b????_????_????_????_100_?10_???_??_???_01 : begin 
instr_name = "C.ANDI"; 
imm = {{27{instr_data[12]}},instr_data[6:2]}; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b????_????_????_????_100_011_???_00_???_01 : begin 
instr_name = "C.SUB"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000010; 
end 
32'b????_????_????_????_100_011_???_01_???_01 : begin 
instr_name = "C.XOR"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b????_????_????_????_100_011_???_10_???_01 : begin 
instr_name = "C.OR"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000010; 
end 
32'b????_????_????_????_100_011_???_11_???_01 : begin 
instr_name = "C.AND"; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op2_idx = {2'b01,instr_data[4:2]}; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000001; 
end 
32'b????_????_????_????_000_0??_???_??_???_10 : begin 
instr_name = "C.SLLI"; 
imm = {26'bx,instr_data[12],instr_data[6:2]}; 
dest_idx = ({instr_data[12],instr_data[6:2]} ==6'b0) ? 5'b0 :instr_data[11:7]; 
op1_idx = instr_data[11:7]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0010; 
opcode = 8'b00011000; 
end 
32'b????_????_????_????_100_0??_???_??_???_10 : begin 
if (instr_data[6:2] == 5'b0) begin 
instr_name = "C.JR"; 
op1_idx = instr_data[11:7]; 
op2_idx = 'b0; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0100; 
illegal = (~|instr_data[11:7]); 
end else begin 
instr_name = "C.MV"; 
op2_idx = instr_data[6:2]; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00100010; 
mv_li_inst = 1'b1; 
end 
end 
32'b????_????_????_????_100_1??_???_??_???_10 : begin 
if (instr_data[11:2]==10'b0) begin 
instr_name = "C.EBREAK"; 
funct_unit = 8'b00010000; 
sub_funct = 4'b0010; 
opcode = 8'b00000010; 
end else if (instr_data[6:2]==5'b0) begin 
instr_name = "C.JALR"; 
dest_idx = 5'b1; 
op1_idx = instr_data[11:7]; 
op2_idx = 'b0; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000010; 
sub_funct = 4'b0100; 
end else begin 
instr_name = "C.ADD"; 
op1_idx = instr_data[11:7]; 
op2_idx = instr_data[6:2]; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0001; 
opcode = 8'b00000001; 
end 
end 
32'b0000111_?????_?????_101_?????_0110011 : begin 
instr_name = "czero.eqz"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00000100; 
end 
32'b0000111_?????_?????_111_?????_0110011 : begin 
instr_name = "czero.nez"; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b01; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00001000; 
end 
32'b????????????????_100_111_???_11_000_01 : begin 
instr_name = "c.zext.b"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00010000; 
end 
32'b????????????????_100_111_???_11_101_01 : begin 
instr_name = "c.not"; 
imm = 32'hffff_ffff; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b10; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
sub_funct = 4'b0100; 
opcode = 8'b00000100; 
end 
32'b????????????????_100_111_???_11_001_01 : begin 
instr_name = "c.sext.b"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00010000; 
end 
32'b????????????????_100_111_???_11_010_01 : begin 
instr_name = "c.zext.h"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
op_signed = 1'b0; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b????????????????_100_111_???_11_011_01 : begin 
instr_name = "c.sext.h"; 
imm = 'bx; 
dest_idx = {2'b01,instr_data[9:7]}; 
op1_idx = {2'b01,instr_data[9:7]}; 
op3_idx = 'bx; 
op1_src_vld = 3'b001; 
op2_src_vld = 2'b00; 
op3_src_vld = 2'b00; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b00100000; 
end 
32'b111100000000_?????_110_?????_0001011 : begin 
instr_name = "REV8HW"; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b01000000; 
end 
32'b111100000001_?????_110_?????_0001011 : begin 
instr_name = "REV8SLHW"; 
op1_src_vld = 3'b001; 
funct_unit = 8'b00000001; 
sub_funct = 4'b1000; 
opcode = 8'b10000000; 
end 
default:illegal = 1'b1; 
endcase 
end 
assign inst_type = (instr_data[1:0] == 2'b11) ? 1'b1 : 1'b0; 
endmodule
 
 
 
module m130_pcu_isu ( 
input logic clk , 
input logic always_on_clk , 
input logic rst_n , 
input logic core_step_vld , 
input logic core_dbg_mode , 
input logic core_elp_state_vld , 
input logic core_pseudo_run_state , 
input logic dbg_disable , 
input logic areg_init , 
input logic [32-1:0] top_boot_pc , 
input logic [2-1:0] ifu_pcu_vld , 
output logic [2-1:0] pcu_ifu_rdy , 
input logic [2-1:0][32-1:0] ifu_pcu_instr_pc , 
input logic [2-1:0][31:0] ifu_pcu_instr_data , 
input logic [2-1:0] ifu_pcu_excp_vld , 
input logic [32-1:0] ifu_pcu_excp_cause , 
input logic ifu_pcu_hv_done , 
input logic [31:0] ifu_pcu_hv_pc , 
output logic pcu_ifu_call_rslv , 
output logic pcu_ifu_ret_rslv , 
output logic jvt_inst_ex , 
output logic pcu_ifu_brq_rd , 
input logic [32-1:0] ifu_pcu_brq_pc , 
input logic [2-1:0] instr_decd_inst_type , 
input logic [2-1:0][2:0] instr_decd_op1_src , 
input logic [2-1:0][1:0] instr_decd_op2_src , 
input logic [2-1:0][1:0] instr_decd_op3_src , 
input logic [2-1:0][8-1:0] instr_decd_funct , 
input logic [2-1:0][4-1:0] instr_decd_sub_funct , 
input logic [2-1:0][8-1:0] instr_decd_opcode , 
input logic [2-1:0] instr_decd_signed , 
input logic [2-1:0][4:0] instr_decd_dest_idx , 
input logic [2-1:0][$clog2(32)-1:0] instr_decd_src1_idx , 
input logic [2-1:0][$clog2(32)-1:0] instr_decd_src2_idx , 
input logic [2-1:0][4:0] instr_decd_src3_idx , 
input logic [2-1:0][32-1:0] instr_decd_imm , 
input logic [2-1:0] instr_decd_illegal , 
input logic [2-1:0] auipc_inst_ex , 
input logic [2-1:0] mv_li_inst_ex , 
input logic [31:0] br_offset , 
input logic alu_shift_op , 
input logic [1:0] lsu_size_ex , 
input logic cm_pop_ret_vld , 
input logic cm_popretz_vld , 
input logic cm_pop_vld , 
input logic [32-1:0] pcu_csr_rdata_all , 
output logic csr_vld_ex , 
output logic [11:0] csr_idx_ex , 
output logic ex_inst_vld , 
output logic [3:0] ex_branch_status , 
output logic [31:0] ex_pc_addr , 
output logic [4-1:0][$clog2(32)-1:0] mrf_gpr_rd_idx , 
input logic [4-1:0][32-1:0] mrf_gpr_rd_data , 
input logic [32-1:0] mrf_gpr_x7 , 
input logic [2-1:0][32-1:0] mrf_gpr_wr_data , 
input logic [2-1:0][$clog2(32)-1:0] mrf_gpr_wr_idx , 
input logic [2-1:0] mrf_gpr_dst_vld , 
output logic [2-1:0] pcu_exu_inst_vld , 
output logic [2-1:0][32-1:0] pcu_exu_alu_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_alu_logic_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_eq_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_mdu_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_agu_op1_val , 
output logic [2-1:0][32-1:0] pcu_exu_alu_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_alu_logic_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_bru_eq_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_mdu_op2_val , 
output logic [2-1:0][32-1:0] pcu_exu_agu_op2_val , 
output logic [2-1:0][4:0] pcu_exu_shift_shamt , 
output logic [2-1:0][32-1:0] pcu_exu_inst_op3_val , 
output logic [2-1:0] pcu_exu_inst_signed , 
output logic [2-1:0][8-1:0] pcu_exu_inst_funct , 
output logic [2-1:0][4-1:0] pcu_exu_inst_sub_funct , 
output logic [2-1:0][8-1:0] pcu_exu_inst_opcode , 
output logic [2-1:0] pcu_exu_inst_type , 
output logic [2-1:0] pcu_exu_inst_is_ret , 
output logic [32-1:0] pcu_exu_inst_pc , 
output logic [32-1:0] pcu_exu_inst_pred_pc , 
input logic exu_pcu_br_tkn , 
input logic exu_pcu_bru_flush , 
input logic [2-1:0][32-1:0] exu_pcu_inst_alu_data , 
input logic [2-1:0][32-1:0] exu_pcu_inst_alu_logic_data, 
input logic [2-1:0][32-1:0] exu_pcu_inst_mdu_data , 
input logic [32-1:0] exu_pcu_inst_tgt_pc , 
input logic pcu_csr_split_integrity , 
output logic [1-1:0] pcu_lsu_inst_vld , 
input logic [1-1:0] lsu_pcu_inst_rdy , 
output logic [1-1:0][4-1:0] pcu_lsu_inst_cmd , 
output logic [1-1:0][3:0] pcu_lsu_inst_fence_op , 
output logic [1-1:0] pcu_lsu_inst_dst_vld , 
output logic [1-1:0][32-1:0] pcu_lsu_inst_data , 
output logic [1-1:0] pcu_lsu_inst_data_wb_vld , 
output logic [1-1:0][1:0] pcu_lsu_inst_size , 
output logic [1-1:0] pcu_lsu_inst_signed , 
output logic [1-1:0] pcu_lsu_inst_addr_fwd , 
output logic [1-1:0][32-1:0] pcu_lsu_inst_addr_fwd_val , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_data , 
output logic stall_ex , 
input logic cmt_flush , 
input logic cmt_flush_wo_dbg , 
input logic [32-1:0] cmt_flush_pc , 
input logic [2-1:0] cmt_inst_split_retire , 
input logic cmt_stall , 
input logic flush_wb , 
input logic flush_wb_wo_dbg , 
input logic ls_exe_cmt_data_vld , 
output logic [2-1:0] cmt_inst_vld , 
output logic [2-1:0][8-1:0] cmt_inst_funct , 
output logic [2-1:0][4-1:0] cmt_inst_sub_funct , 
output logic [2-1:0][8-1:0] cmt_inst_opcode , 
output logic [2-1:0] cmt_dest_vld , 
output logic [2-1:0][$clog2(32)-1:0] cmt_dest , 
output logic [1:0][32-1:0] cmt_pc , 
output logic cmt_inst_en , 
output logic cmt_uop_int_mask , 
output logic cmt_excp_vld , 
output logic [32-1:0] cmt_excp_cause , 
output logic cmt_trig_exp , 
output logic cmt_trig_dbg , 
output logic cmt_uop_inst , 
output logic cmt_br_tkn , 
output logic [2-1:0] cmt_inst_type , 
output logic [11:0] cmt_csr_idx_wb , 
output logic [2-1:0] cmt_exp_lp_inst , 
output logic [2-1:0] cmt_lp_inst , 
output logic [2-1:0] cmt_dbg_disable , 
output logic [2-1:0] cmt_ls_addr_write_back , 
output logic cmt_ls_exe_2_cycs_stall_ex, 
output logic [4-1:0] cmt_trigger_hit_idx , 
output logic [$clog2(32)-1:0] ls_post_pre_addr_dst_idx_wb, 
input logic [2-1:0] tm_pcu_trigger_ex_vld , 
input logic [2-1:0] tm_pcu_trigger_debug_ex , 
input logic [4-1:0] tm_pcu_trigger_hit_idx_ex , 
input logic dm_pcu_resume_req , 
input logic [32-1:0] jvt_csr , 
input logic [2-1:0][127:0] instr_name 
); 
 
localparam X0 = 5'b0 ; 
localparam X2 = 5'h2 ; 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
localparam FENCE_INST = 0; 
localparam AMO_INST = 1; 
localparam LD_INST = 2; 
localparam ST_INST = 3; 
localparam LS_POST = 5; 
localparam EXCLUSIVE_INST = 0; 
localparam SYS = 0 ; 
localparam SYS_E = 1 ; 
localparam CSR_INST = 2 ; 
localparam BRKPT = 12'h3 ; 
localparam ILL_INST = 12'h2 ; 
localparam LP_ERR = 12'h12 ; 
localparam ECALL_U = 12'h8 ; 
localparam ECALL_M = 12'hb ; 
localparam LD_RAS_ERR = 12'h1F ; 
localparam INST_MISALIGN = 12'h0 ; 
localparam IFU_PMP_ERR = 12'h1 ; 
localparam LD_ACC_FAULT = 12'h5 ; 
localparam LD_MISALIGN = 12'h4 ; 
localparam ST_ACC_FAULT = 12'h7 ; 
localparam ST_MISALIGN = 12'h6 ; 
localparam RAS_ERR = 12'h1f ; 
localparam CSRRW = 0 ; 
localparam CSRRS = 1 ; 
localparam CSRRC = 2 ; 
localparam CSRIMM = 3 ; 
localparam UNAVAIL = 2'h0; 
localparam HALTED = 2'h1; 
localparam RUNNING = 2'h2; 
localparam NMI_ID = 12'hFFF ; 
localparam MRET = 0 ; 
localparam WFI = 1 ; 
localparam MNRET = 2 ; 
localparam ECALL = 0 ; 
localparam EBREAK = 1 ; 
localparam MRASIE = 31 ; 
localparam MEIE = 11 ; 
localparam MTIE = 7 ; 
localparam MSIE = 3 ; 
localparam MRASIP = 31 ; 
localparam MEIP = 11 ; 
localparam MTIP = 7 ; 
localparam MSIP = 3 ; 
typedef enum logic [4:0]{ 
LOAD = 5'b00001 , 
STORE = 5'b00010 , 
FENCE = 5'b00100 
}sub_func_lsu_e ; 
typedef enum logic [4:0] { 
SUB_FU_NONE = 5'b00000 , 
ARITHMATIC = 5'b00001 , 
SHIFT = 5'b00010 , 
LOGICAL = 5'b00100 , 
MISC = 5'b01000 
} sub_func_alu_e; 
typedef enum logic [4:0] { 
UNCOND_DIRE = 5'b00001 , 
COND_DIRE = 5'b00010 , 
UNCOND_INDIRE = 5'b00100 
}sub_func_bru_e ; 
typedef enum logic [3:0] { 
OTHER_TYPES = 4'd0 , 
EXCP_TYPE = 4'd1 , 
INT_TYPE = 4'd2 , 
MRET_TYPE = 4'd3 , 
NT_BR_TYPE = 4'd4 , 
T_BR_TYPE = 4'd5 , 
UNINF_TYPE = 4'd6 
}itype; 
typedef struct packed{ 
logic interrupt; 
logic minhv; 
logic [1:0] mpp; 
logic mpie; 
logic [7:0] mpil; 
logic [11:0] exccode; 
} mcause_csr; 
typedef struct packed{ 
logic mprv; 
logic [1:0] mpp; 
logic mpie; 
logic mie; 
} mstatus_csr; 
typedef struct packed{ 
logic mnpelp; 
logic [1:0] mnpp; 
logic mnpv; 
logic nmie; 
}mnstatus_csr; 
typedef struct packed{ 
logic [3:0] debuger ; 
logic pelp ; 
logic ebreakvs ; 
logic ebreakvu ; 
logic ebreakm ; 
logic ebreaks ; 
logic ebreaku ; 
logic stepie ; 
logic stopcount; 
logic stoptime ; 
logic [2:0] cause ; 
logic v ; 
logic mprven ; 
logic nmip ; 
logic step ; 
logic [1:0] prv ; 
}dcsr_csr; 
typedef enum logic [3:0] { 
SPLIT_IDLE = 4'b0000 , 
SPLIT_UOP = 4'b0001 , 
SPLIT_LI = 4'b0010 , 
SPLIT_ADD = 4'b0110 , 
SPLIT_RET = 4'b1000 , 
SPLIT_SHF_ADD = 4'b1100 
} state_e; 
 
 
 
typedef enum logic[1:0] { 
PRIV_MODE_M = 2'b11, 
PRIV_MODE_S = 2'b01, 
PRIV_MODE_U = 2'b00 
} priv_mode_e; 
typedef enum logic[1:0] { 
CSR_OP_READ = 2'b00, 
CSR_OP_WRITE = 2'b01, 
CSR_OP_SET = 2'b10, 
CSR_OP_CLEAR = 2'b11 
} csr_opcode_e; 
typedef enum logic [11:0] { 
CSR_FFLAGS_ADDR = 12'h001, 
CSR_FRM_ADDR = 12'h002, 
CSR_FCSR_ADDR = 12'h003, 
CSR_FTRAN_ADDR = 12'h800, 
CSR_SSTATUS_ADDR = 12'h100, 
CSR_SIE_ADDR = 12'h104, 
CSR_STVEC_ADDR = 12'h105, 
CSR_SCOUNTEREN_ADDR = 12'h106, 
CSR_SSCRATCH_ADDR = 12'h140, 
CSR_SEPC_ADDR = 12'h141, 
CSR_SCAUSE_ADDR = 12'h142, 
CSR_STVAL_ADDR = 12'h143, 
CSR_SIP_ADDR = 12'h144, 
CSR_SATP_ADDR = 12'h180, 
CSR_MSTATUS_ADDR = 12'h300, 
CSR_MISA_ADDR = 12'h301, 
CSR_MEDELEG_ADDR = 12'h302, 
CSR_MIDELEG_ADDR = 12'h303, 
CSR_MIE_ADDR = 12'h304, 
CSR_MTVEC_ADDR = 12'h305, 
CSR_MCOUNTEREN_ADDR = 12'h306, 
CSR_MSTATUSH_ADDR = 12'h310, 
CSR_MSCRATCH_ADDR = 12'h340, 
CSR_MEPC_ADDR = 12'h341, 
CSR_MCAUSE_ADDR = 12'h342, 
CSR_MTVAL_ADDR = 12'h343, 
CSR_MIP_ADDR = 12'h344, 
CSR_MENVCFG_ADDR = 12'h30A, 
CSR_MENVCFGH_ADDR = 12'h31A, 
CSR_MSECCFG_ADDR = 12'h747, 
CSR_MSECCFGH_ADDR = 12'h757, 
CSR_PMPCFG0_ADDR = 12'h3A0, 
CSR_PMPCFG1_ADDR = 12'h3A1, 
CSR_PMPCFG2_ADDR = 12'h3A2, 
CSR_PMPCFG3_ADDR = 12'h3A3, 
CSR_PMPADDR0_ADDR = 12'h3B0, 
CSR_PMPADDR1_ADDR = 12'h3B1, 
CSR_PMPADDR2_ADDR = 12'h3B2, 
CSR_PMPADDR3_ADDR = 12'h3B3, 
CSR_PMPADDR4_ADDR = 12'h3B4, 
CSR_PMPADDR5_ADDR = 12'h3B5, 
CSR_PMPADDR6_ADDR = 12'h3B6, 
CSR_PMPADDR7_ADDR = 12'h3B7, 
CSR_PMPADDR8_ADDR = 12'h3B8, 
CSR_PMPADDR9_ADDR = 12'h3B9, 
CSR_PMPADDR10_ADDR = 12'h3BA, 
CSR_PMPADDR11_ADDR = 12'h3BB, 
CSR_PMPADDR12_ADDR = 12'h3BC, 
CSR_PMPADDR13_ADDR = 12'h3BD, 
CSR_PMPADDR14_ADDR = 12'h3BE, 
CSR_PMPADDR15_ADDR = 12'h3BF, 
CSR_MVENDORID_ADDR = 12'hF11, 
CSR_MARCHID_ADDR = 12'hF12, 
CSR_MIMPID_ADDR = 12'hF13, 
CSR_MHARTID_ADDR = 12'hF14, 
CSR_MCONFIGPTRID_ADDR = 12'hF15, 
CSR_MCYCLE_ADDR = 12'hB00, 
CSR_MCYCLEH_ADDR = 12'hB80, 
CSR_MINSTRET_ADDR = 12'hB02, 
CSR_MINSTRETH_ADDR = 12'hB82, 
CSR_MHPM_COUNTER_3_ADDR = 12'hB03, 
CSR_MHPM_COUNTER_4_ADDR = 12'hB04, 
CSR_MHPM_COUNTER_5_ADDR = 12'hB05, 
CSR_MHPM_COUNTER_6_ADDR = 12'hB06, 
CSR_MHPM_COUNTER_7_ADDR = 12'hB07, 
CSR_MHPM_COUNTER_8_ADDR = 12'hB08, 
CSR_MHPM_COUNTER_9_ADDR = 12'hB09, 
CSR_MHPM_COUNTER_10_ADDR = 12'hB0A, 
CSR_MHPM_COUNTER_11_ADDR = 12'hB0B, 
CSR_MHPM_COUNTER_12_ADDR = 12'hB0C, 
CSR_MHPM_COUNTER_13_ADDR = 12'hB0D, 
CSR_MHPM_COUNTER_14_ADDR = 12'hB0E, 
CSR_MHPM_COUNTER_15_ADDR = 12'hB0F, 
CSR_MHPM_COUNTER_16_ADDR = 12'hB10, 
CSR_MHPM_COUNTER_17_ADDR = 12'hB11, 
CSR_MHPM_COUNTER_18_ADDR = 12'hB12, 
CSR_MHPM_COUNTER_19_ADDR = 12'hB13, 
CSR_MHPM_COUNTER_20_ADDR = 12'hB14, 
CSR_MHPM_COUNTER_21_ADDR = 12'hB15, 
CSR_MHPM_COUNTER_22_ADDR = 12'hB16, 
CSR_MHPM_COUNTER_23_ADDR = 12'hB17, 
CSR_MHPM_COUNTER_24_ADDR = 12'hB18, 
CSR_MHPM_COUNTER_25_ADDR = 12'hB19, 
CSR_MHPM_COUNTER_26_ADDR = 12'hB1A, 
CSR_MHPM_COUNTER_27_ADDR = 12'hB1B, 
CSR_MHPM_COUNTER_28_ADDR = 12'hB1C, 
CSR_MHPM_COUNTER_29_ADDR = 12'hB1D, 
CSR_MHPM_COUNTER_30_ADDR = 12'hB1E, 
CSR_MHPM_COUNTER_31_ADDR = 12'hB1F, 
CSR_MHPM_COUNTER_3H_ADDR = 12'hB83, 
CSR_MHPM_COUNTER_4H_ADDR = 12'hB84, 
CSR_MHPM_COUNTER_5H_ADDR = 12'hB85, 
CSR_MHPM_COUNTER_6H_ADDR = 12'hB86, 
CSR_MHPM_COUNTER_7H_ADDR = 12'hB87, 
CSR_MHPM_COUNTER_8H_ADDR = 12'hB88, 
CSR_MHPM_COUNTER_9H_ADDR = 12'hB89, 
CSR_MHPM_COUNTER_10H_ADDR = 12'hB8A, 
CSR_MHPM_COUNTER_11H_ADDR = 12'hB8B, 
CSR_MHPM_COUNTER_12H_ADDR = 12'hB8C, 
CSR_MHPM_COUNTER_13H_ADDR = 12'hB8D, 
CSR_MHPM_COUNTER_14H_ADDR = 12'hB8E, 
CSR_MHPM_COUNTER_15H_ADDR = 12'hB8F, 
CSR_MHPM_COUNTER_16H_ADDR = 12'hB90, 
CSR_MHPM_COUNTER_17H_ADDR = 12'hB91, 
CSR_MHPM_COUNTER_18H_ADDR = 12'hB92, 
CSR_MHPM_COUNTER_19H_ADDR = 12'hB93, 
CSR_MHPM_COUNTER_20H_ADDR = 12'hB94, 
CSR_MHPM_COUNTER_21H_ADDR = 12'hB95, 
CSR_MHPM_COUNTER_22H_ADDR = 12'hB96, 
CSR_MHPM_COUNTER_23H_ADDR = 12'hB97, 
CSR_MHPM_COUNTER_24H_ADDR = 12'hB98, 
CSR_MHPM_COUNTER_25H_ADDR = 12'hB99, 
CSR_MHPM_COUNTER_26H_ADDR = 12'hB9A, 
CSR_MHPM_COUNTER_27H_ADDR = 12'hB9B, 
CSR_MHPM_COUNTER_28H_ADDR = 12'hB9C, 
CSR_MHPM_COUNTER_29H_ADDR = 12'hB9D, 
CSR_MHPM_COUNTER_30H_ADDR = 12'hB9E, 
CSR_MHPM_COUNTER_31H_ADDR = 12'hB9F, 
CSR_MCOUNTINHIBIT_ADDR = 12'h320, 
CSR_MHPM_EVENT_3_ADDR = 12'h323, 
CSR_MHPM_EVENT_4_ADDR = 12'h324, 
CSR_MHPM_EVENT_5_ADDR = 12'h325, 
CSR_MHPM_EVENT_6_ADDR = 12'h326, 
CSR_MHPM_EVENT_7_ADDR = 12'h327, 
CSR_MHPM_EVENT_8_ADDR = 12'h328, 
CSR_MHPM_EVENT_9_ADDR = 12'h329, 
CSR_MHPM_EVENT_10_ADDR = 12'h32a, 
CSR_MHPM_EVENT_11_ADDR = 12'h32b, 
CSR_MHPM_EVENT_12_ADDR = 12'h32c, 
CSR_MHPM_EVENT_13_ADDR = 12'h32d, 
CSR_MHPM_EVENT_14_ADDR = 12'h32e, 
CSR_MHPM_EVENT_15_ADDR = 12'h32f, 
CSR_MHPM_EVENT_16_ADDR = 12'h330, 
CSR_MHPM_EVENT_17_ADDR = 12'h331, 
CSR_MHPM_EVENT_18_ADDR = 12'h332, 
CSR_MHPM_EVENT_19_ADDR = 12'h333, 
CSR_MHPM_EVENT_20_ADDR = 12'h334, 
CSR_MHPM_EVENT_21_ADDR = 12'h335, 
CSR_MHPM_EVENT_22_ADDR = 12'h336, 
CSR_MHPM_EVENT_23_ADDR = 12'h337, 
CSR_MHPM_EVENT_24_ADDR = 12'h338, 
CSR_MHPM_EVENT_25_ADDR = 12'h339, 
CSR_MHPM_EVENT_26_ADDR = 12'h33a, 
CSR_MHPM_EVENT_27_ADDR = 12'h33b, 
CSR_MHPM_EVENT_28_ADDR = 12'h33c, 
CSR_MHPM_EVENT_29_ADDR = 12'h33d, 
CSR_MHPM_EVENT_30_ADDR = 12'h33e, 
CSR_MHPM_EVENT_31_ADDR = 12'h33f, 
CSR_MTVT_ADDR = 12'h307, 
CSR_MNXTI_ADDR = 12'h345, 
CSR_MINTSTATUS_ADDR = 12'hfb1, 
CSR_MINTTHRESH_ADDR = 12'h347, 
CSR_MSCRATCHCSW_ADDR = 12'h348, 
CSR_MSCRATCHCSWL_ADDR = 12'h349, 
CSR_MNSCRATCH_ADDR = 12'h740, 
CSR_MNEPC_ADDR = 12'h741, 
CSR_MNCAUSE_ADDR = 12'h742, 
CSR_MNSTATUS_ADDR = 12'h744, 
CSR_MNVEC_ADDR = 12'hFC0, 
CSR_TSELECT_ADDR = 12'h7A0, 
CSR_TDATA1_ADDR = 12'h7A1, 
CSR_TDATA2_ADDR = 12'h7A2, 
CSR_TDATA3_ADDR = 12'h7A3, 
CSR_TINFO_ADDR = 12'h7A4, 
CSR_TCONTROL_ADDR = 12'h7A5, 
CSR_DCSR_ADDR = 12'h7b0, 
CSR_DPC_ADDR = 12'h7b1, 
CSR_DSCRATCH0_ADDR = 12'h7b2, 
CSR_DSCRATCH1_ADDR = 12'h7b3, 
CSR_DSCRATCH2_ADDR = 12'h7c0, 
CSR_DSCRATCH3_ADDR = 12'h7c1, 
CSR_MCONTEXTSW_ADDR = 12'h7c2, 
CSR_MSECURFEAT_ADDR = 12'h7c3, 
CSR_MLPCFG_ADDR = 12'h7c4, 
CSR_CYCLE_ADDR = 12'hC00, 
CSR_CYCLEH_ADDR = 12'hC80, 
CSR_TIME_ADDR = 12'hC01, 
CSR_TIMEH_ADDR = 12'hC81, 
CSR_INSTRET_ADDR = 12'hC02, 
CSR_INSTRETH_ADDR = 12'hC82, 
CSR_HPM_COUNTER_3_ADDR = 12'hC03, 
CSR_HPM_COUNTER_4_ADDR = 12'hC04, 
CSR_HPM_COUNTER_5_ADDR = 12'hC05, 
CSR_HPM_COUNTER_6_ADDR = 12'hC06, 
CSR_HPM_COUNTER_7_ADDR = 12'hC07, 
CSR_HPM_COUNTER_8_ADDR = 12'hC08, 
CSR_HPM_COUNTER_9_ADDR = 12'hC09, 
CSR_HPM_COUNTER_10_ADDR = 12'hC0A, 
CSR_HPM_COUNTER_11_ADDR = 12'hC0B, 
CSR_HPM_COUNTER_12_ADDR = 12'hC0C, 
CSR_HPM_COUNTER_13_ADDR = 12'hC0D, 
CSR_HPM_COUNTER_14_ADDR = 12'hC0E, 
CSR_HPM_COUNTER_15_ADDR = 12'hC0F, 
CSR_HPM_COUNTER_16_ADDR = 12'hC10, 
CSR_HPM_COUNTER_17_ADDR = 12'hC11, 
CSR_HPM_COUNTER_18_ADDR = 12'hC12, 
CSR_HPM_COUNTER_19_ADDR = 12'hC13, 
CSR_HPM_COUNTER_20_ADDR = 12'hC14, 
CSR_HPM_COUNTER_21_ADDR = 12'hC15, 
CSR_HPM_COUNTER_22_ADDR = 12'hC16, 
CSR_HPM_COUNTER_23_ADDR = 12'hC17, 
CSR_HPM_COUNTER_24_ADDR = 12'hC18, 
CSR_HPM_COUNTER_25_ADDR = 12'hC19, 
CSR_HPM_COUNTER_26_ADDR = 12'hC1A, 
CSR_HPM_COUNTER_27_ADDR = 12'hC1B, 
CSR_HPM_COUNTER_28_ADDR = 12'hC1C, 
CSR_HPM_COUNTER_29_ADDR = 12'hC1D, 
CSR_HPM_COUNTER_30_ADDR = 12'hC1E, 
CSR_HPM_COUNTER_31_ADDR = 12'hC1F, 
CSR_HPM_COUNTER_3H_ADDR = 12'hC83, 
CSR_HPM_COUNTER_4H_ADDR = 12'hC84, 
CSR_HPM_COUNTER_5H_ADDR = 12'hC85, 
CSR_HPM_COUNTER_6H_ADDR = 12'hC86, 
CSR_HPM_COUNTER_7H_ADDR = 12'hC87, 
CSR_HPM_COUNTER_8H_ADDR = 12'hC88, 
CSR_HPM_COUNTER_9H_ADDR = 12'hC89, 
CSR_HPM_COUNTER_10H_ADDR = 12'hC8A, 
CSR_HPM_COUNTER_11H_ADDR = 12'hC8B, 
CSR_HPM_COUNTER_12H_ADDR = 12'hC8C, 
CSR_HPM_COUNTER_13H_ADDR = 12'hC8D, 
CSR_HPM_COUNTER_14H_ADDR = 12'hC8E, 
CSR_HPM_COUNTER_15H_ADDR = 12'hC8F, 
CSR_HPM_COUNTER_16H_ADDR = 12'hC90, 
CSR_HPM_COUNTER_17H_ADDR = 12'hC91, 
CSR_HPM_COUNTER_18H_ADDR = 12'hC92, 
CSR_HPM_COUNTER_19H_ADDR = 12'hC93, 
CSR_HPM_COUNTER_20H_ADDR = 12'hC94, 
CSR_HPM_COUNTER_21H_ADDR = 12'hC95, 
CSR_HPM_COUNTER_22H_ADDR = 12'hC96, 
CSR_HPM_COUNTER_23H_ADDR = 12'hC97, 
CSR_HPM_COUNTER_24H_ADDR = 12'hC98, 
CSR_HPM_COUNTER_25H_ADDR = 12'hC99, 
CSR_HPM_COUNTER_26H_ADDR = 12'hC9A, 
CSR_HPM_COUNTER_27H_ADDR = 12'hC9B, 
CSR_HPM_COUNTER_28H_ADDR = 12'hC9C, 
CSR_HPM_COUNTER_29H_ADDR = 12'hC9D, 
CSR_HPM_COUNTER_30H_ADDR = 12'hC9E, 
CSR_HPM_COUNTER_31H_ADDR = 12'hC9F, 
CSR_JVT_ADDR = 12'h017 
} csr_reg_t; 
 
localparam OP1_SRC_NONE = 3'b000 ; 
localparam OP1_SRC_MRF = 3'b001 ; 
localparam OP1_SRC_PC = 3'b010 ; 
localparam OP1_SRC_IMM = 3'b100 ; 
localparam OP1_SRC_MRF_BIT = 0 ; 
localparam OP1_SRC_IMM_BIT = 1 ; 
localparam OP1_SRC_PC_BIT = 2 ; 
localparam OP2_SRC_NONE = 3'b000 ; 
localparam OP2_SRC_MRF = 3'b001 ; 
localparam OP2_SRC_IMM = 3'b010 ; 
localparam OP2_SRC_MRF_BIT = 0 ; 
localparam OP2_SRC_IMM_BIT = 1 ; 
localparam OP3_SRC_NONE = 2'b00 ; 
localparam OP3_SRC_MRF = 2'b01 ; 
localparam OP3_SRC_IMM = 2'b10 ; 
localparam OP3_SRC_MRF_BIT = 0 ; 
localparam OP3_SRC_IMM_BIT = 1 ; 
localparam EX_WB_PIPELINE_INFO_W = 1 + $clog2(32) + $clog2(32) + 12 + 32 + 8 + 4 + 
8 + 1 + 3 + 2 + 2 + 1 + 32 + 4 + 1 +1 + 1 + 1 + 1 + 1 +1+ 1+1 + 1 + 1; 
logic iss_cm_jvt_inst ; 
logic [1:0][32-1:0] inst_ex_pc ; 
logic [2-1:0] iss_gpr_src1_vld_ex ; 
logic [2-1:0] iss_gpr_src2_vld_ex ; 
logic [2-1:0] iss_gpr_src3_vld_ex ; 
logic [2-1:0][8-1:0] iss_funct_ex ; 
logic [2-1:0][4-1:0] iss_sub_funct_ex ; 
logic [8-1:0] iss0_opcode_pre_ex ; 
logic [2-1:0][8-1:0] iss_opcode_ex ; 
logic [2-1:0] iss_excp_vld_ex ; 
logic [2-1:0] iss_trig_exp_ex ; 
logic [2-1:0] iss_trig_dbg_ex ; 
logic [32-1:0] iss0_excp_cause_ex ; 
logic [4-1:0] iss_trigger_hit_idx_ex ; 
logic iss0_gpr_src1_raw ; 
logic iss0_gpr_src2_raw ; 
logic iss0_gpr_src_raw ; 
logic iss0_stall_by_lsu ; 
logic iss0_stall_by_spu ; 
logic iss0_stall_by_uop ; 
logic iss0_stall_by_sbr ; 
logic iss1_gpr_src1_raw ; 
logic iss1_gpr_src2_raw ; 
logic iss1_gpr_src_raw ; 
logic iss1_stall_by_issue0 ; 
logic iss1_stall_by_type ; 
logic iss1_stall_by_uop ; 
logic iss1_stall_by_step ; 
logic iss1_stall_by_excp ; 
logic [2-1:0] iss_vld_pre_ex ; 
logic [2-1:0] iss_vld_ex ; 
logic [2-1:0] pipeline_vld_ex ; 
logic [2-1:0][EX_WB_PIPELINE_INFO_W-1:0] pipeline_info_ex ; 
logic [2-1:0][EX_WB_PIPELINE_INFO_W-1:0] pipeline_info_wb ; 
logic pipeline_stall_ex ; 
logic pipeline_stall_wb ; 
logic [2-1:0] pipeline_vld_wb ; 
logic [2-1:0] pipeline_info_en_ex ; 
logic iss1_rs1_index_pipe_en ; 
logic [2-1:0] iss_vld_wb ; 
logic [2-1:0][$clog2(32)-1:0] iss_gpr_dst_idx_wb ; 
logic [2-1:0] iss_gpr_dst_vld_wb ; 
logic [2-1:0][$clog2(32)-1:0] iss_gpr_src1_idx_wb ; 
logic [2-1:0][$clog2(32)-1:0] iss_gpr_src2_idx_wb ; 
logic [2-1:0][11:0] iss_csr_idx_wb ; 
logic [2-1:0][32-1:0] iss_imm_wb ; 
logic [1:0][32-1:0] iss_pc_wb ; 
logic [1:0] pc_pipe_en ; 
logic [2-1:0][8-1:0] iss_funct_wb ; 
logic [2-1:0][4-1:0] iss_sub_funct_wb ; 
logic [2-1:0][8-1:0] iss_opcode_wb ; 
logic [2-1:0] iss_signed_wb ; 
logic [2-1:0][2:0] iss_op1_src_wb ; 
logic [2-1:0][1:0] iss_op2_src_wb ; 
logic [2-1:0] iss_excpt_wb ; 
logic [2-1:0][32-1:0] iss_excpt_code_wb ; 
logic [2-1:0][4-1:0] iss_trigger_hit_idx ; 
logic [2-1:0] iss_inst_cmt_en_wb ; 
logic iss_uop_int_mask_wb ; 
logic iss0_inst_cmt_en_ex ; 
logic iss0_uop_int_mask_ex ; 
logic [2-1:0][1:0] iss_rs_fwd_wb ; 
logic [1:0][32-1:0] mrf_gpr_rd_data_ex ; 
logic [1:0][32-1:0] mrf_gpr_rd_data_wb ; 
logic [1:0] iss0_rs_ealu_fwd_any_ex ; 
logic [1:0] iss0_rs_lalu1_fwd_1cyc_ex; 
logic [1:0] iss0_rs_lalu2_fwd_1cyc_ex; 
logic [1:0] iss0_rs_lalu_fwd_lsu_ex ; 
logic [1:0] iss1_rs_1cyc_fwd_lalu_ex ; 
logic [1:0] iss1_rs_lsu_fwd_lalu_ex ; 
logic [1:0] iss0_rs_lsu_fwd_lsu_ex ; 
logic [1:0] iss0_rs_lsu_fwd_bru_ex ; 
logic [1:0][1:0] iss0_rs_fwd_ex ; 
logic [1:0] iss1_rs_fwd_ex ; 
logic [32-1:0] iss0_op1_val_ex ; 
logic [32-1:0] iss0_op1_fwd_val_ex ; 
logic iss0_op1_fwd_sel_ex ; 
logic [32-1:0] iss0_op1_fwd_val_4_agu ; 
logic iss0_op2_vld_ex ; 
logic [32-1:0] iss0_op2_val_ex ; 
logic [32-1:0] iss0_op2_fwd_val_ex ; 
logic iss0_op2_fwd_sel_ex ; 
logic [32-1:0] iss0_op2_fwd_val_4_agu ; 
logic [32-1:0] iss0_logic_op1_fwd_val_ex; 
logic [32-1:0] iss0_logic_op2_fwd_val_ex; 
logic iss0_op3_vld_ex ; 
logic [32-1:0] iss0_op3_val_ex ; 
logic [32-1:0] iss1_op1_val_wb ; 
logic [32-1:0] iss1_op1_fwd_val_wb ; 
logic iss1_op1_fwd_sel_wb ; 
logic [32-1:0] iss1_op2_val_wb ; 
logic [32-1:0] iss1_op2_fwd_val_wb ; 
logic iss1_op2_fwd_sel_wb ; 
logic [32-1:0] iss1_op2_mv_fwd_val_wb ; 
logic [32-1:0] iss1_op3_val_wb ; 
logic shf_uop_fwd ; 
logic uop_ill ; 
logic uop_inst ; 
logic uop_push ; 
logic uop_pop_all ; 
logic uop_pop ; 
logic uop_pop_ret ; 
logic uop_popretz ; 
logic uop_mv ; 
logic [2-1:0] uop_inst_wb ; 
logic [2-1:0] iss_trig_exp_wb ; 
logic [2-1:0] iss_trig_dbg_wb ; 
logic partial_spu_vld_wb ; 
logic [1:0] iss0_rs1_raw_iss_vld ; 
logic [1:0] iss0_rs2_raw_iss_vld ; 
logic iss1_uop_vld_ex ; 
logic [$clog2(32)-1:0] iss1_inst_src1_idx ; 
logic [2-1:0] iss_inst_code_ill ; 
logic iss0_inst_br_tkn ; 
logic [2-1:0] iss_inst_br_tkn_wb ; 
logic [2-1:0] iss_inst_type_wb ; 
logic [2-1:0] iss_mv_li_inst_wb ; 
logic [2-1:0] iss_inst_vld_ex ; 
logic [2-1:0] uop_dst_vld ; 
logic [2-1:0][$clog2(32)-1:0] uop_dst_idx ; 
logic [2-1:0] uop_rs1_vld ; 
logic [2-1:0][$clog2(32)-1:0] uop_rs1_idx ; 
logic [2-1:0] uop_rs2_vld ; 
logic [2-1:0][$clog2(32)-1:0] uop_rs2_idx ; 
logic [2-1:0] uop_rs2_fr_imm_vld ; 
logic [2-1:0][31:0] uop_rs2_fr_imm ; 
logic [2-1:0] uop_rs1_src_vld ; 
logic [2-1:0][2:0] uop_rs1_src ; 
logic [2-1:0] uop_rs2_src_vld ; 
logic [2-1:0][1:0] uop_rs2_src ; 
logic [2-1:0] uop_funct_vld ; 
logic [2-1:0][8-1:0] uop_funct ; 
logic [2-1:0] uop_sub_funct_vld ; 
logic [2-1:0][4-1:0] uop_sub_funct ; 
logic [2-1:0] uop_opcode_vld ; 
logic [2-1:0][8-1:0] uop_opcode ; 
logic uop_stall ; 
logic uop_int_mask ; 
logic uop_inst_cmt_en ; 
logic split_ret_state ; 
logic split_idle_state ; 
logic split_uop_state ; 
logic iss1_uop_vld ; 
logic [2-1:0][2:0] iss_src1_vld_fnl_ex ; 
logic [2-1:0][1:0] iss_src2_vld_fnl_ex ; 
logic [2-1:0] iss_inst_dst_vld_ex ; 
logic [2-1:0][$clog2(32)-1:0] iss_inst_dst_idx_ex ; 
logic [2-1:0][$clog2(32)-1:0] iss_inst_rs2_idx_ex ; 
logic [2-1:0][$clog2(32)-1:0] iss_inst_rs1_idx_ex ; 
logic [2-1:0][31:0] inst_imm_ex ; 
logic instr_decd_is_call ; 
logic instr_decd_is_ret ; 
logic instr_jalr ; 
logic instr_jal ; 
logic instr_cjalr ; 
logic instr_cjal ; 
logic instr_cm_jalt ; 
logic [2-1:0] ldp_inst_ex ; 
logic exp_lp_inst_ex ; 
logic [1:0] uop_shift ; 
logic uop_ld_dest ; 
logic uop_st_src ; 
logic ls_exe_2_cycs_cmt ; 
logic ls_addr_write_back ; 
logic [2-1:0] iss_op_sign_ex ; 
logic uop_csr_cs ; 
logic uop_csr_cs_st ; 
logic uop_csr_cs_ld ; 
logic uop_csr_cs_nop ; 
logic uop_csr_cs_mask_dst ; 
logic [2-1:0] uop_shf_alu_fwd ; 
logic ls_post_addr ; 
logic lp_mark_excp ; 
genvar i; 
assign instr_jalr = instr_decd_funct[0][FU_BRU_BIT]& (instr_decd_sub_funct[0] == 4'b0100) & instr_decd_inst_type[0]; 
assign instr_jal = instr_decd_funct[0][FU_BRU_BIT]& (instr_decd_sub_funct[0] == 4'b0001) & instr_decd_inst_type[0]; 
assign instr_cjalr = instr_decd_funct[0][FU_BRU_BIT]& (instr_decd_sub_funct[0] == 4'b0100) & !instr_decd_inst_type[0]; 
assign instr_cjal = instr_decd_funct[0][FU_BRU_BIT]& (instr_decd_sub_funct[0] == 4'b0001) & !instr_decd_inst_type[0]; 
assign instr_decd_is_call = (instr_jalr | instr_cjalr | instr_jal | instr_cjal) & (instr_decd_dest_idx[0] == 5'h1 | instr_decd_dest_idx[0] == 5'h5) | instr_cm_jalt; 
assign instr_decd_is_ret = (instr_jalr | instr_cjalr) & (instr_decd_src1_idx[0] == {{($clog2(32)-4){1'b0}},4'h1} | instr_decd_src1_idx[0] == {{($clog2(32)-4){1'b0}},4'h5}) & (instr_decd_src1_idx[0] != instr_decd_dest_idx[0][$clog2(32)-1:0]); 
assign instr_cm_jalt = instr_decd_funct[0][1]& (instr_decd_sub_funct[0] == 4'b1000) & (|ifu_pcu_instr_data[0][ 9: 7]) ; 
assign uop_pop = cm_pop_vld; 
assign uop_pop_ret = cm_pop_ret_vld; 
assign uop_popretz = cm_popretz_vld; 
assign uop_push = instr_decd_funct[0][FU_LSU_BIT] & instr_decd_funct[0][FU_UOP_BIT] & instr_decd_sub_funct[0][ST_INST]; 
assign uop_mv = instr_decd_funct[0][FU_ALU_BIT] & instr_decd_funct[0][FU_UOP_BIT] & instr_decd_sub_funct[0][2]; 
assign iss_cm_jvt_inst = instr_decd_funct[0][FU_BRU_BIT] & instr_decd_sub_funct[0][3]; 
assign ldp_inst_ex[0] = 1'b0; 
assign exp_lp_inst_ex = 1'b0; 
assign lp_mark_excp = 1'b0; 
assign uop_shift = {ifu_pcu_instr_data[0][12],~ifu_pcu_instr_data[0][12]} & {2{instr_decd_funct[0][FU_ALU_BIT] & instr_decd_funct[0][FU_UOP_BIT] & instr_decd_sub_funct[0][1]}}; 
assign uop_ld_dest = instr_decd_funct[0][FU_ALU_BIT] & instr_decd_funct[0][FU_LSU_BIT] & instr_decd_sub_funct[0][LD_INST] & (|instr_decd_opcode[0][6:5]); 
assign uop_st_src = instr_decd_funct[0][FU_ALU_BIT] & instr_decd_funct[0][FU_LSU_BIT] & instr_decd_sub_funct[0][ST_INST] & instr_decd_op2_src[0][OP2_SRC_MRF_BIT]; 
assign ls_addr_write_back = instr_decd_funct[0][FU_ALU_BIT] & instr_decd_funct[0][FU_LSU_BIT] & (|instr_decd_opcode[0][6:5]); 
`WDFFER(cmt_ls_exe_2_cycs_stall_ex,(iss_vld_ex[0] & (uop_st_src | uop_ld_dest) | cmt_ls_exe_2_cycs_stall_ex & !ls_exe_2_cycs_cmt) & !cmt_flush, !pipeline_stall_wb | ls_exe_2_cycs_cmt, clk, rst_n) 
assign ls_exe_2_cycs_cmt = cmt_ls_exe_2_cycs_stall_ex & ls_exe_cmt_data_vld; 
assign uop_csr_cs_st = (ifu_pcu_instr_data[0][31:20] == CSR_MCONTEXTSW_ADDR) & instr_decd_funct[0][FU_SPU_BIT] & instr_decd_sub_funct[0][CSR_INST] & 
(ifu_pcu_instr_data[0][14:12] == 3'b010) & ((instr_decd_src1_idx[0] == X2) & (ifu_pcu_instr_data[0][11:7] == X0)); 
assign uop_csr_cs_ld = (ifu_pcu_instr_data[0][31:20] == CSR_MCONTEXTSW_ADDR) & instr_decd_funct[0][FU_SPU_BIT] & instr_decd_sub_funct[0][CSR_INST] & 
(ifu_pcu_instr_data[0][14:12] == 3'b001) & ((instr_decd_src1_idx[0] == X2) & (ifu_pcu_instr_data[0][11:7] == X0)); 
assign uop_csr_cs_nop = (ifu_pcu_instr_data[0][31:20] == CSR_MCONTEXTSW_ADDR) & instr_decd_funct[0][FU_SPU_BIT] & instr_decd_sub_funct[0][CSR_INST] & !(uop_csr_cs_st | uop_csr_cs_ld); 
assign uop_csr_cs_mask_dst = (ifu_pcu_instr_data[0][31:20] == CSR_MCONTEXTSW_ADDR) & instr_decd_funct[0][FU_SPU_BIT] & instr_decd_sub_funct[0][CSR_INST]; 
assign uop_csr_cs = uop_csr_cs_st | uop_csr_cs_ld; 
assign uop_inst = instr_decd_funct[0][FU_UOP_BIT] | uop_csr_cs; 
assign uop_pop_all = uop_pop | uop_pop_ret; 
assign iss_inst_code_ill[0] = instr_decd_illegal[0] | uop_ill ; 
assign iss_inst_vld_ex[0] = ifu_pcu_vld[0]; 
assign iss_op_sign_ex[0] = instr_decd_signed[0]; 
assign iss_inst_code_ill[1] = instr_decd_illegal[1] | ldp_inst_ex[1]; 
assign iss_inst_vld_ex[1] = ifu_pcu_vld[1]; 
assign iss_op_sign_ex[1] = (!split_idle_state | uop_mv | (|uop_shift)) ? 1'b1 : instr_decd_signed[1]; 
assign ldp_inst_ex[1] = 1'b0; 
assign uop_shf_alu_fwd[1] = shf_uop_fwd; 
assign uop_shf_alu_fwd[0] = 1'b0; 
generate 
for(i = 0; i < 2; i = i + 1) begin: DECODER_INFO_PROCESS 
assign iss_gpr_src1_vld_ex[i] = (iss_src1_vld_fnl_ex[i] == OP1_SRC_MRF) && (iss_inst_rs1_idx_ex[i] != X0); 
assign iss_gpr_src2_vld_ex[i] = ((iss_src2_vld_fnl_ex[i] == OP2_SRC_MRF) || (instr_decd_op3_src[i] == OP3_SRC_MRF)) && 
(iss_inst_rs2_idx_ex[i] != X0); 
assign iss_gpr_src3_vld_ex[i] = 1'b0; 
assign iss_trig_exp_ex[i] = tm_pcu_trigger_ex_vld[i] & !tm_pcu_trigger_debug_ex[i]; 
assign iss_trig_dbg_ex[i] = tm_pcu_trigger_ex_vld[i] & tm_pcu_trigger_debug_ex[i]; 
assign iss_excp_vld_ex[i] = (ifu_pcu_excp_vld[i] & ifu_pcu_vld[i] | iss_inst_code_ill[i]); 
assign iss_inst_dst_vld_ex[i] = (iss_inst_dst_idx_ex[i] != X0); 
assign iss_inst_dst_idx_ex[i] = uop_dst_vld[i] ? uop_dst_idx[i] : instr_decd_dest_idx[i][$clog2(32)-1:0]; 
assign iss_inst_rs1_idx_ex[i] = uop_rs1_vld[i] ? uop_rs1_idx[i] : instr_decd_src1_idx[i][$clog2(32)-1:0]; 
assign iss_inst_rs2_idx_ex[i] = uop_rs2_vld[i] ? uop_rs2_idx[i] : instr_decd_src2_idx[i][$clog2(32)-1:0]; 
assign inst_imm_ex[i] = uop_rs2_fr_imm_vld[i] ? uop_rs2_fr_imm[i] : instr_decd_imm[i]; 
assign iss_src1_vld_fnl_ex[i] = uop_rs1_src_vld[i] ? uop_rs1_src[i] : instr_decd_op1_src[i]; 
assign iss_src2_vld_fnl_ex[i] = uop_rs2_src_vld[i] ? uop_rs2_src[i] : instr_decd_op2_src[i][1:0]; 
assign iss_funct_ex[i] = uop_funct_vld[i] ? uop_funct[i] : instr_decd_funct[i] | {{(8-4){1'b0}},uop_csr_cs_st | uop_csr_cs_ld,3'b0}; 
assign iss_sub_funct_ex[i] = uop_sub_funct_vld[i] ? uop_sub_funct[i] : {instr_decd_sub_funct[i][3]|uop_csr_cs_st,instr_decd_sub_funct[i][2]&!(uop_csr_cs_st|uop_csr_cs_nop),instr_decd_sub_funct[i][1:0]}; 
assign iss_opcode_ex[i] = uop_opcode_vld[i] ? uop_opcode[i] : instr_decd_opcode[i]; 
end 
endgenerate 
assign iss_trigger_hit_idx_ex = tm_pcu_trigger_hit_idx_ex; 
assign iss0_excp_cause_ex = 
ifu_pcu_excp_vld[0] ? ifu_pcu_excp_cause : 
lp_mark_excp ? LP_ERR : 
ILL_INST; 
generate for(i = 0; i < 2; i = i + 1) begin:ISS0_RAW_VLD 
assign iss0_rs1_raw_iss_vld[i] = iss_vld_wb[i] & mrf_gpr_dst_vld[i] & (iss_inst_rs1_idx_ex[0] == mrf_gpr_wr_idx[i]); 
assign iss0_rs2_raw_iss_vld[i] = iss_vld_wb[i] & mrf_gpr_dst_vld[i] & (iss_inst_rs2_idx_ex[0] == mrf_gpr_wr_idx[i]); 
end 
endgenerate 
assign iss0_gpr_src1_raw = iss_gpr_src1_vld_ex[0] & (iss0_rs1_raw_iss_vld[0] & !iss0_rs_fwd_ex[0][0]|iss0_rs1_raw_iss_vld[1] & !iss0_rs_fwd_ex[1][0]); 
assign iss0_gpr_src2_raw = iss_gpr_src2_vld_ex[0] & (iss0_rs2_raw_iss_vld[0] & !iss0_rs_fwd_ex[0][1]|iss0_rs2_raw_iss_vld[1] & !iss0_rs_fwd_ex[1][1]); 
assign iss0_gpr_src_raw = iss_inst_vld_ex[0] & (iss0_gpr_src1_raw | iss0_gpr_src2_raw); 
assign iss0_stall_by_lsu = iss_funct_ex[0][FU_LSU_BIT] & !lsu_pcu_inst_rdy; 
assign iss0_stall_by_spu = partial_spu_vld_wb; 
assign iss0_stall_by_sbr = iss_funct_ex[0][FU_LSU_BIT] & iss_sub_funct_ex[0][ST_INST] & 
(cmt_inst_funct[0][FU_LSU_BIT] & cmt_inst_sub_funct[0][ST_INST]) & cmt_ls_exe_2_cycs_stall_ex; 
assign iss0_stall_by_uop = uop_stall; 
assign iss0_inst_cmt_en_ex = uop_inst_cmt_en; 
assign iss0_uop_int_mask_ex = uop_int_mask; 
assign iss1_gpr_src1_raw = iss_gpr_src1_vld_ex[1] & iss_inst_dst_vld_ex[0] & 
(iss_inst_rs1_idx_ex[1] == iss_inst_dst_idx_ex[0][$clog2(32)-1:0]) & ( 
iss_funct_ex[0][FU_MDU_BIT] ); 
assign iss1_gpr_src2_raw = iss_gpr_src2_vld_ex[1] & iss_inst_dst_vld_ex[0] & 
(iss_inst_rs2_idx_ex[1] == iss_inst_dst_idx_ex[0][$clog2(32)-1:0]) & ( 
iss_funct_ex[0][FU_MDU_BIT] ); 
assign iss1_gpr_src_raw = iss_inst_vld_ex[1] & (iss1_gpr_src1_raw | iss1_gpr_src2_raw); 
assign iss1_stall_by_issue0 = !pcu_ifu_rdy[0] | 
iss_funct_ex[0][FU_SPU_BIT] | 
(iss_funct_ex[0][FU_LSU_BIT] && iss_sub_funct_ex[0][0]) | 
uop_ld_dest | uop_st_src | uop_csr_cs; 
assign iss1_stall_by_type = !iss_funct_ex[1][FU_ALU_BIT]; 
assign iss1_stall_by_uop = uop_inst | uop_st_src|uop_ld_dest; 
assign iss1_stall_by_step = core_step_vld; 
assign iss1_stall_by_excp = (|iss_excp_vld_ex) | (|tm_pcu_trigger_ex_vld); 
assign pcu_ifu_rdy[0] = !(iss0_gpr_src_raw | 
iss0_stall_by_lsu | 
iss0_stall_by_spu | 
iss0_stall_by_uop | 
iss0_stall_by_sbr | 
pipeline_stall_wb | 
core_pseudo_run_state); 
assign csr_vld_ex = ifu_pcu_vld[0] & iss_funct_ex[0][FU_SPU_BIT] & instr_decd_sub_funct[0][CSR_INST]; 
assign iss_vld_pre_ex[0] = iss_inst_vld_ex[0] & !(iss0_gpr_src_raw | 
iss0_stall_by_lsu | 
iss0_stall_by_spu | 
iss0_stall_by_sbr | 
pipeline_stall_wb ); 
assign iss_vld_ex[0] = iss_vld_pre_ex[0] & !core_pseudo_run_state; 
assign pipeline_vld_ex[0] = iss_vld_ex[0] & !(cmt_flush); 
assign inst_ex_pc[0] = areg_init ? top_boot_pc : 
cmt_flush ? cmt_flush_pc : ifu_pcu_instr_pc[0]; 
assign inst_ex_pc[1] = ifu_pcu_hv_done? ifu_pcu_hv_pc : (iss_inst_code_ill[0])? {{16{instr_decd_inst_type[0]}} & ifu_pcu_instr_data[0][31:16],ifu_pcu_instr_data[0][15:0]} : exu_pcu_inst_tgt_pc; 
assign pc_pipe_en[0] = (ifu_pcu_vld[0] & !cmt_flush & !pipeline_stall_ex & !pipeline_stall_wb | cmt_flush_wo_dbg & !cmt_stall | areg_init | flush_wb_wo_dbg) & !(core_dbg_mode & !dm_pcu_resume_req); 
assign pc_pipe_en[1] = (ifu_pcu_vld[0] & !cmt_flush & !pipeline_stall_ex & !pipeline_stall_wb | ifu_pcu_hv_done) & !(core_dbg_mode & !dm_pcu_resume_req); 
assign pipeline_info_en_ex[0] = iss_inst_vld_ex[0] & !(cmt_flush) & !pipeline_stall_ex & !pipeline_stall_wb; 
assign pcu_ifu_rdy[1] = !iss1_gpr_src_raw & !iss1_stall_by_issue0 & !iss1_stall_by_uop & !iss1_stall_by_type & !iss1_stall_by_step & !pipeline_stall_wb & !iss1_stall_by_excp; 
assign iss1_uop_vld_ex = iss1_uop_vld & lsu_pcu_inst_rdy & !iss0_gpr_src_raw | iss_vld_ex[0] & (uop_mv | (|uop_shift) | uop_st_src | uop_ld_dest); 
assign iss_vld_pre_ex[1] = iss_inst_vld_ex[1] & pcu_ifu_rdy[1] | iss1_uop_vld_ex & iss_inst_vld_ex[0]; 
assign iss_vld_ex[1] = iss_vld_pre_ex[1] & !core_pseudo_run_state; 
assign pipeline_vld_ex[1] = iss_vld_ex[1] & !(cmt_flush) & !exu_pcu_bru_flush & !pipeline_stall_ex; 
assign pipeline_info_en_ex[1] = (iss_inst_vld_ex[1] & pcu_ifu_rdy[1] | iss1_uop_vld_ex)& !(cmt_flush) & !pipeline_stall_ex && !pipeline_stall_wb; 
assign iss0_inst_br_tkn = 1'b0; 
assign pipeline_stall_ex = !(iss_vld_ex[0]) ; 
assign stall_ex = !(iss_vld_pre_ex[0]); 
assign pipeline_stall_wb = cmt_stall; 
assign iss0_opcode_pre_ex = iss_funct_ex[0][FU_SPU_BIT] & iss_sub_funct_ex[0][CSR_INST] ? {{(8-5){1'b0}},ifu_pcu_instr_data[0][19:15] == 5'b0,ifu_pcu_instr_data[0][14],ifu_pcu_instr_data[0][13:12] == 2'b11,ifu_pcu_instr_data[0][13:12] == 2'b10,ifu_pcu_instr_data[0][13:12] == 2'b01} : {iss_opcode_ex[0][8-1:1],iss_opcode_ex[0][0] & !iss_funct_ex[0][FU_ALU_BIT]}; 
assign csr_idx_ex = (ifu_pcu_instr_data[0][31:20] == CSR_MCONTEXTSW_ADDR)? (split_idle_state ? CSR_MCAUSE_ADDR : CSR_MEPC_ADDR) :ifu_pcu_instr_data[0][31:20]; 
assign pipeline_info_ex[0] = {iss_inst_dst_vld_ex[0] & !(split_idle_state & (|uop_shift) | uop_csr_cs_mask_dst), 
iss_inst_dst_idx_ex[0], 
{$clog2(32){1'b0}}, 
csr_idx_ex, 
{32{1'b0}}, 
iss_funct_ex[0], 
iss_sub_funct_ex[0], 
iss0_opcode_pre_ex, 
1'b0, 
3'b0, 
2'b0, 
2'b0, 
iss_excp_vld_ex[0]|lp_mark_excp, 
iss0_excp_cause_ex, 
iss_trigger_hit_idx_ex, 
iss0_inst_cmt_en_ex, 
iss_trig_exp_ex[0], 
iss_trig_dbg_ex[0], 
uop_inst|uop_st_src|uop_ld_dest, 
iss0_inst_br_tkn, 
instr_decd_inst_type[0], 
ls_addr_write_back, 
exp_lp_inst_ex, 
ldp_inst_ex[0], 
dbg_disable, 
1'b0 
}; 
assign iss1_inst_src1_idx = iss_inst_rs1_idx_ex[1]; 
assign iss1_rs1_index_pipe_en = (iss_inst_vld_ex[1] | iss1_uop_vld_ex | instr_decd_op3_src[0][0] & iss_inst_vld_ex[0])& !(cmt_flush) & !pipeline_stall_ex && !pipeline_stall_wb; 
`WDFFENR(iss_gpr_src1_idx_wb[1], iss1_inst_src1_idx, iss1_rs1_index_pipe_en,clk) 
assign pipeline_info_ex[1] = {iss_inst_dst_vld_ex[1] | uop_mv, 
iss_inst_dst_idx_ex[1], 
iss_inst_rs2_idx_ex[1], 
12'b0, 
inst_imm_ex[1][31:0], 
iss_funct_ex[1], 
iss_sub_funct_ex[1], 
iss_opcode_ex[1], 
iss_op_sign_ex[1], 
iss_src1_vld_fnl_ex[1], 
iss_src2_vld_fnl_ex[1], 
iss1_rs_fwd_ex, 
1'b0, 
32'b0, 
{4{1'b0}}, 
1'b0, 
1'b0, 
1'b0, 
1'b0, 
1'b0, 
instr_decd_inst_type[1], 
1'b0, 
1'b0, 
1'b0, 
1'b0, 
mv_li_inst_ex[1] & pcu_ifu_rdy[1] 
}; 
generate for(i = 0; i < 2; i = i + 1) begin:PC_PIPE 
`WDFFER(iss_pc_wb[i], inst_ex_pc[i],pc_pipe_en[i] , clk,rst_n) 
end 
endgenerate 
assign iss_gpr_src1_idx_wb[0] = 5'b0; 
`WDFFER(iss_uop_int_mask_wb, iss0_uop_int_mask_ex & !cmt_flush, pipeline_info_en_ex[0] | cmt_inst_split_retire[0] | flush_wb, clk, rst_n) 
generate 
for(i = 0; i < 2; i = i + 1) begin: EX_WB_PIPE_INFO_GEN 
`WDFFER(pipeline_vld_wb[i], pipeline_vld_ex[i], !pipeline_stall_wb | flush_wb, clk, rst_n) 
`WDFFENR(pipeline_info_wb[i], pipeline_info_ex[i], pipeline_info_en_ex[i],clk) 
assign {iss_gpr_dst_vld_wb[i] , 
iss_gpr_dst_idx_wb[i] , 
iss_gpr_src2_idx_wb[i], 
iss_csr_idx_wb[i] , 
iss_imm_wb[i] , 
iss_funct_wb[i] , 
iss_sub_funct_wb[i] , 
iss_opcode_wb[i] , 
iss_signed_wb[i] , 
iss_op1_src_wb[i] , 
iss_op2_src_wb[i] , 
iss_rs_fwd_wb[i] , 
iss_excpt_wb[i] , 
iss_excpt_code_wb[i] , 
iss_trigger_hit_idx[i], 
iss_inst_cmt_en_wb[i] , 
iss_trig_exp_wb[i] , 
iss_trig_dbg_wb[i] , 
uop_inst_wb[i] , 
iss_inst_br_tkn_wb[i] , 
iss_inst_type_wb[i] , 
cmt_ls_addr_write_back[i], 
cmt_exp_lp_inst[i] , 
cmt_lp_inst[i] , 
cmt_dbg_disable[i] , 
iss_mv_li_inst_wb[i] 
} = pipeline_info_wb[i]; 
end 
endgenerate 
assign partial_spu_vld_wb = iss_vld_wb[0] & iss_funct_wb[0][FU_SPU_BIT] & iss_sub_funct_wb[0][CSR_INST]; 
assign iss_vld_wb = pipeline_vld_wb ; 
assign mrf_gpr_rd_idx[0] = iss_inst_rs1_idx_ex[0]; 
assign mrf_gpr_rd_idx[1] = iss_inst_rs2_idx_ex[0]; 
assign mrf_gpr_rd_data_ex[0] = mrf_gpr_rd_data[0]; 
assign mrf_gpr_rd_data_ex[1] = mrf_gpr_rd_data[1]; 
assign mrf_gpr_rd_idx[2] = iss_gpr_src1_idx_wb[1]; 
assign mrf_gpr_rd_idx[3] = iss_gpr_src2_idx_wb[1]; 
assign mrf_gpr_rd_data_wb[0] = mrf_gpr_rd_data[2]; 
assign mrf_gpr_rd_data_wb[1] = mrf_gpr_rd_data[3]; 
assign iss0_rs_ealu_fwd_any_ex[0] = (iss_funct_wb[0][FU_ALU_BIT]&!iss_funct_wb[0][FU_LSU_BIT]|iss_funct_wb[0][FU_BMU_BIT]|iss_funct_wb[0][FU_BRU_BIT]) & iss0_rs1_raw_iss_vld[0]; 
assign iss0_rs_ealu_fwd_any_ex[1] = (iss_funct_wb[0][FU_ALU_BIT]&!iss_funct_wb[0][FU_LSU_BIT]|iss_funct_wb[0][FU_BMU_BIT]|iss_funct_wb[0][FU_BRU_BIT]) & iss0_rs2_raw_iss_vld[0]; 
assign iss1_rs_1cyc_fwd_lalu_ex[0] = (iss_funct_ex[0][FU_ALU_BIT] | iss_funct_ex[0][FU_BMU_BIT] | iss_funct_ex[0][FU_BRU_BIT]) & 
iss_gpr_src1_vld_ex[1] & 
ifu_pcu_vld[0] & iss_inst_dst_vld_ex[0] & (iss_inst_rs1_idx_ex[1] == iss_inst_dst_idx_ex[0][$clog2(32)-1:0]) & !uop_shf_alu_fwd[1]; 
assign iss1_rs_1cyc_fwd_lalu_ex[1] = (iss_funct_ex[0][FU_ALU_BIT] | iss_funct_ex[0][FU_BMU_BIT] | iss_funct_ex[0][FU_BRU_BIT]) & 
iss_gpr_src2_vld_ex[1] & 
ifu_pcu_vld[0] & iss_inst_dst_vld_ex[0] & (iss_inst_rs2_idx_ex[1] == iss_inst_dst_idx_ex[0][$clog2(32)-1:0]) | uop_shf_alu_fwd[1]; 
assign iss0_rs_lsu_fwd_lsu_ex[0] = iss_funct_wb[0][FU_LSU_BIT] & !iss_funct_wb[0][FU_ALU_BIT]& iss_funct_ex[0][FU_LSU_BIT] & iss0_rs1_raw_iss_vld[0] & ( 
(inst_imm_ex[0][11:0] == 12'b0) & instr_decd_op2_src[0][OP2_SRC_IMM_BIT] | 
instr_decd_opcode[0][LS_POST] 
); 
assign iss0_rs_lsu_fwd_lsu_ex[1] = (iss_funct_wb[0][FU_LSU_BIT] & !iss_funct_wb[0][FU_ALU_BIT]) & 
(iss_funct_ex[0][FU_LSU_BIT] & iss_sub_funct_ex[0][ST_INST] & instr_decd_op2_src[0][OP2_SRC_IMM_BIT]) & iss0_rs2_raw_iss_vld[0]; 
assign iss0_rs_lsu_fwd_bru_ex[0] = (iss_funct_wb[0][FU_LSU_BIT] & !iss_funct_wb[0][FU_ALU_BIT] ) & 
(iss_funct_ex[0][FU_BRU_BIT] & iss_sub_funct_ex[0][1] & (|iss_opcode_ex[0][1:0]) ) & iss0_rs1_raw_iss_vld[0]; 
assign iss0_rs_lsu_fwd_bru_ex[1] = (iss_funct_wb[0][FU_LSU_BIT] & !iss_funct_wb[0][FU_ALU_BIT] ) & 
(iss_funct_ex[0][FU_BRU_BIT] & iss_sub_funct_ex[0][1] & (|iss_opcode_ex[0][1:0]) ) & iss0_rs2_raw_iss_vld[0]; 
assign iss0_rs_lalu1_fwd_1cyc_ex[0]= 1'b0; 
assign iss0_rs_lalu1_fwd_1cyc_ex[1]= 1'b0; 
assign iss0_rs_lalu2_fwd_1cyc_ex[0]= (iss_funct_wb[1][FU_ALU_BIT] ) & 
(iss_funct_ex[0][FU_ALU_BIT] & !iss_funct_ex[0][FU_LSU_BIT] & iss_sub_funct_ex[0][2]) & iss0_rs1_raw_iss_vld[1]; 
assign iss0_rs_lalu2_fwd_1cyc_ex[1]= (iss_funct_wb[1][FU_ALU_BIT] ) & 
(iss_funct_ex[0][FU_ALU_BIT] & !iss_funct_ex[0][FU_LSU_BIT] & iss_sub_funct_ex[0][2]) & iss0_rs2_raw_iss_vld[1]; 
assign iss0_rs_lalu_fwd_lsu_ex[0] = (iss_funct_wb[1][FU_ALU_BIT] & iss_mv_li_inst_wb[1] ) & 
!iss_rs_fwd_wb[1][0] & 
(iss_funct_ex[0][FU_LSU_BIT] & !(|instr_decd_opcode[0][6:5]) ) & iss0_rs1_raw_iss_vld[1]; 
assign iss0_rs_lalu_fwd_lsu_ex[1] = (iss_funct_wb[1][FU_ALU_BIT] ) & 
(iss_funct_ex[0][FU_LSU_BIT] & iss_sub_funct_ex[0][ST_INST] & !iss_src2_vld_fnl_ex[0][OP2_SRC_MRF_BIT] ) & iss0_rs2_raw_iss_vld[1]; 
assign iss0_rs_fwd_ex[1][0] = (iss0_rs_lalu_fwd_lsu_ex[0] | iss0_rs_lalu1_fwd_1cyc_ex[0] | iss0_rs_lalu2_fwd_1cyc_ex[0]); 
assign iss0_rs_fwd_ex[1][1] = (iss0_rs_lalu_fwd_lsu_ex[1] | iss0_rs_lalu1_fwd_1cyc_ex[1] | iss0_rs_lalu2_fwd_1cyc_ex[1]); 
assign iss1_rs_lsu_fwd_lalu_ex[0] = (iss_funct_ex[0][FU_LSU_BIT] ) & 
iss_gpr_src1_vld_ex[1] & 
ifu_pcu_vld[0] & iss_inst_dst_vld_ex[0] & (iss_inst_rs1_idx_ex[1] == iss_inst_dst_idx_ex[0][$clog2(32)-1:0]); 
assign iss1_rs_lsu_fwd_lalu_ex[1] = (iss_funct_ex[0][FU_LSU_BIT] ) & 
iss_gpr_src2_vld_ex[1] & 
ifu_pcu_vld[0] & iss_inst_dst_vld_ex[0] & (iss_inst_rs2_idx_ex[1] == iss_inst_dst_idx_ex[0][$clog2(32)-1:0]); 
assign iss0_rs_fwd_ex[0][0] = iss0_rs_ealu_fwd_any_ex[0] | iss0_rs_lsu_fwd_lsu_ex[0] | iss0_rs_lsu_fwd_bru_ex[0]; 
assign iss0_rs_fwd_ex[0][1] = iss0_rs_ealu_fwd_any_ex[1] | iss0_rs_lsu_fwd_lsu_ex[1] | iss0_rs_lsu_fwd_bru_ex[1] | uop_shf_alu_fwd[0] ; 
assign iss0_op1_fwd_sel_ex = (iss0_rs1_raw_iss_vld[0] | iss0_rs1_raw_iss_vld[1]) & iss_src1_vld_fnl_ex[0][OP1_SRC_MRF_BIT]; 
assign iss0_op2_fwd_sel_ex = (iss0_rs2_raw_iss_vld[0] | iss0_rs2_raw_iss_vld[1]) & iss_src2_vld_fnl_ex[0][OP2_SRC_MRF_BIT] | uop_shf_alu_fwd[0]; 
assign iss1_rs_fwd_ex[0] = (iss1_rs_1cyc_fwd_lalu_ex[0] | iss1_rs_lsu_fwd_lalu_ex[0]) & !uop_mv; 
assign iss1_rs_fwd_ex[1] = (iss1_rs_1cyc_fwd_lalu_ex[1] | iss1_rs_lsu_fwd_lalu_ex[1]) & !uop_mv; 
assign iss0_op1_val_ex = ({32{iss_src1_vld_fnl_ex[0][OP1_SRC_MRF_BIT]}} & mrf_gpr_rd_data_ex[0]) | 
({32{iss_src1_vld_fnl_ex[0][OP1_SRC_PC_BIT]}} & ifu_pcu_instr_pc[0]) | 
({32{iss_cm_jvt_inst }} & jvt_csr) ; 
logic [32-1:0] fwd_wb_data; 
assign fwd_wb_data = ({32{~(cmt_inst_funct[0][FU_LSU_BIT] & (!cmt_inst_sub_funct[0][ST_INST]|cmt_inst_opcode[0][EXCLUSIVE_INST]))}} & exu_pcu_inst_alu_data[0]) | 
({32{ (cmt_inst_funct[0][FU_LSU_BIT] & (!cmt_inst_sub_funct[0][ST_INST]|cmt_inst_opcode[0][EXCLUSIVE_INST]))}} & lsu_pcu_cmt_data[0] ) ; 
assign iss0_op1_fwd_val_ex = 
fwd_wb_data; 
assign iss0_logic_op1_fwd_val_ex = 
iss0_rs1_raw_iss_vld[1] ? mrf_gpr_wr_data[1] : 
fwd_wb_data; 
assign iss0_op2_vld_ex = (iss_src2_vld_fnl_ex[0] != OP2_SRC_NONE); 
assign iss0_op2_val_ex = ({32{iss_src2_vld_fnl_ex[0][OP2_SRC_MRF_BIT]}} & mrf_gpr_rd_data_ex[1]) | 
({32{iss_src2_vld_fnl_ex[0][OP2_SRC_IMM_BIT]}} & inst_imm_ex[0]) ; 
assign iss0_op2_fwd_val_ex = 
fwd_wb_data; 
assign iss0_logic_op2_fwd_val_ex = 
iss0_rs2_raw_iss_vld[1] ? mrf_gpr_wr_data[1] : 
fwd_wb_data; 
assign iss0_op3_vld_ex = (instr_decd_op3_src[0] != OP3_SRC_NONE); 
assign iss0_op3_val_ex = br_offset; 
assign iss1_op1_val_wb = ({32{iss_op1_src_wb[1][OP1_SRC_MRF_BIT]}} & mrf_gpr_rd_data_wb[0]) | 
({32{iss_op1_src_wb[1][OP1_SRC_PC_BIT] }} & iss_pc_wb[1]) ; 
assign iss1_op1_fwd_val_wb = ({32{~(cmt_inst_funct[0][FU_LSU_BIT] & (!cmt_inst_sub_funct[0][ST_INST]|cmt_inst_opcode[0][EXCLUSIVE_INST]))}} & exu_pcu_inst_alu_data[0]) | 
({32{ (cmt_inst_funct[0][FU_LSU_BIT] & (!cmt_inst_sub_funct[0][ST_INST]|cmt_inst_opcode[0][EXCLUSIVE_INST]))}} & lsu_pcu_cmt_data[0]) ; 
assign iss1_op1_fwd_sel_wb = iss_rs_fwd_wb[1][0]; 
assign iss1_op2_val_wb = ({32{iss_op2_src_wb[1][OP2_SRC_MRF_BIT]}} & mrf_gpr_rd_data_wb[1]) | 
({32{iss_op2_src_wb[1][OP2_SRC_IMM_BIT]}} & iss_imm_wb[1]) ; 
assign iss1_op2_fwd_val_wb = ({32{~(cmt_inst_funct[0][FU_LSU_BIT] & (!cmt_inst_sub_funct[0][ST_INST]|cmt_inst_opcode[0][EXCLUSIVE_INST]))}} & exu_pcu_inst_alu_data[0]) | 
({32{ (cmt_inst_funct[0][FU_LSU_BIT] & (!cmt_inst_sub_funct[0][ST_INST]|cmt_inst_opcode[0][EXCLUSIVE_INST]))}} & lsu_pcu_cmt_data[0]) ; 
assign iss1_op2_fwd_sel_wb = iss_rs_fwd_wb[1][1]; 
assign iss1_op2_mv_fwd_val_wb = ({32{iss_op2_src_wb[1][OP1_SRC_MRF_BIT]}} & mrf_gpr_rd_data[3]) | 
({32{iss_op2_src_wb[1][OP2_SRC_IMM_BIT]}} & iss_imm_wb[1] ) ; 
assign iss1_op3_val_wb = {32{1'b0}}; 
assign iss0_op1_fwd_val_4_agu = ({32{iss_op2_src_wb[1][OP1_SRC_MRF_BIT] & !iss_rs_fwd_wb[1][1] & iss0_rs1_raw_iss_vld[1]}} & mrf_gpr_rd_data[3]) | 
({32{iss_op2_src_wb[1][OP2_SRC_IMM_BIT] & !iss_rs_fwd_wb[1][1] & iss0_rs1_raw_iss_vld[1]}} & iss_imm_wb[1] ) | 
({32{iss_op2_src_wb[1][OP1_SRC_MRF_BIT] & iss_rs_fwd_wb[1][1] | !iss0_rs1_raw_iss_vld[1]}} & exu_pcu_inst_alu_data[0]) ; 
assign iss0_op2_fwd_val_4_agu = ({32{iss_op2_src_wb[1][OP1_SRC_MRF_BIT] & !iss_rs_fwd_wb[1][1] & iss0_rs2_raw_iss_vld[1]}} & mrf_gpr_rd_data[3]) | 
({32{iss_op2_src_wb[1][OP2_SRC_IMM_BIT] & !iss_rs_fwd_wb[1][1] & iss0_rs2_raw_iss_vld[1]}} & iss_imm_wb[1] ) | 
({32{iss_op2_src_wb[1][OP1_SRC_MRF_BIT] & iss_rs_fwd_wb[1][1] | !iss0_rs2_raw_iss_vld[1]}} & exu_pcu_inst_alu_data[0]) ; 
assign pcu_exu_inst_vld[0] = iss_vld_ex[0] & !core_elp_state_vld; 
assign pcu_exu_alu_op1_val[0] = iss0_op1_fwd_sel_ex ? iss0_op1_fwd_val_ex : iss0_op1_val_ex ; 
assign pcu_exu_alu_logic_op1_val[0] = iss0_op1_fwd_sel_ex ? iss0_logic_op1_fwd_val_ex : iss0_op1_val_ex ; 
assign pcu_exu_bru_op1_val[0] = iss0_op1_fwd_sel_ex ? exu_pcu_inst_alu_data[0] : iss0_op1_val_ex ; 
assign pcu_exu_bru_eq_op1_val[0] = iss0_op1_fwd_sel_ex ? fwd_wb_data : iss0_op1_val_ex ; 
assign pcu_exu_mdu_op1_val[0] = iss0_op1_fwd_sel_ex ? exu_pcu_inst_alu_data[0] : iss0_op1_val_ex ; 
assign pcu_exu_agu_op1_val[0] = iss0_op1_fwd_sel_ex ? iss0_op1_fwd_val_4_agu : iss0_op1_val_ex ; 
assign pcu_exu_alu_op2_val[0] = iss0_op2_fwd_sel_ex ? iss0_op2_fwd_val_ex : iss0_op2_val_ex ; 
assign pcu_exu_shift_shamt[0] = 
alu_shift_op ? ifu_pcu_instr_data[0][29:25] : 
iss0_op2_fwd_sel_ex ? iss0_op2_fwd_val_ex[4:0] : 
iss0_op2_val_ex[4:0]; 
assign pcu_exu_alu_logic_op2_val[0] = iss0_op2_fwd_sel_ex ? iss0_logic_op2_fwd_val_ex : iss0_op2_val_ex ; 
assign pcu_exu_bru_op2_val[0] = iss0_op2_fwd_sel_ex ? exu_pcu_inst_alu_data[0] : iss0_op2_val_ex ; 
assign pcu_exu_bru_eq_op2_val[0] = iss0_op2_fwd_sel_ex ? fwd_wb_data : iss0_op2_val_ex ; 
assign pcu_exu_mdu_op2_val[0] = iss0_op2_fwd_sel_ex ? exu_pcu_inst_alu_data[0] : iss0_op2_val_ex ; 
assign pcu_exu_agu_op2_val[0] = 
iss0_op2_fwd_sel_ex ? exu_pcu_inst_alu_data[0] : iss0_op2_val_ex; 
assign pcu_exu_inst_op3_val[0] = iss0_op3_val_ex ; 
assign pcu_exu_inst_signed[0] = instr_decd_signed[0]; 
assign pcu_exu_inst_funct[0] = iss_funct_ex[0] ; 
assign pcu_exu_inst_sub_funct[0] = {iss_sub_funct_ex[0][4-1:1] & {(4-1){~ls_addr_write_back}},iss_sub_funct_ex[0][0] | ls_addr_write_back} ; 
assign pcu_exu_inst_opcode[0] = {iss_opcode_ex[0][8-1:7],iss_opcode_ex[0][6:5] & {2{~ls_addr_write_back}},iss_opcode_ex[0][4:2],iss_opcode_ex[0][1]&!uop_csr_cs,iss_opcode_ex[0][0]|uop_csr_cs}; 
assign pcu_exu_inst_type[0] = instr_decd_inst_type[0]; 
assign pcu_exu_inst_is_ret[0] = (instr_decd_is_ret | split_ret_state)& iss_vld_ex[0] & !core_dbg_mode; 
assign pcu_exu_inst_pc = ifu_pcu_instr_pc[0] ; 
assign pcu_exu_inst_pred_pc = ifu_pcu_brq_pc ; 
assign pcu_exu_inst_vld[1] = iss_vld_wb[1] ; 
assign pcu_exu_alu_op1_val[1] = iss1_op1_fwd_sel_wb ? iss1_op1_fwd_val_wb : iss1_op1_val_wb ; 
assign pcu_exu_alu_logic_op1_val[1] = iss1_op1_fwd_sel_wb ? iss1_op1_fwd_val_wb : iss1_op1_val_wb; 
assign pcu_exu_bru_op1_val[1] = 'b0; 
assign pcu_exu_bru_eq_op1_val[1] = 'b0; 
assign pcu_exu_mdu_op1_val[1] = 'b0; 
assign pcu_exu_agu_op1_val[1] = 'b0; 
assign pcu_exu_alu_op2_val[1] = iss1_op2_fwd_sel_wb ? iss1_op2_fwd_val_wb : iss1_op2_val_wb ; 
assign pcu_exu_shift_shamt[1] = iss1_op2_fwd_sel_wb ? iss1_op2_fwd_val_wb[4:0] : iss1_op2_val_wb[4:0] ; 
assign pcu_exu_alu_logic_op2_val[1] = 'b0; 
assign pcu_exu_bru_op2_val[1] = 'b0; 
assign pcu_exu_bru_eq_op2_val[1] = 'b0; 
assign pcu_exu_mdu_op2_val[1] = 'b0; 
assign pcu_exu_agu_op2_val[1] = 'b0; 
assign pcu_exu_inst_op3_val[1] = iss1_op3_val_wb ; 
assign pcu_exu_inst_signed[1] = iss_signed_wb[1] ; 
assign pcu_exu_inst_funct[1] = iss_funct_wb[1] ; 
assign pcu_exu_inst_sub_funct[1] = iss_sub_funct_wb[1] ; 
assign pcu_exu_inst_opcode[1] = iss_opcode_wb[1] ; 
assign pcu_exu_inst_type[1] = 1'b0 ; 
assign pcu_exu_inst_is_ret[1] = 1'b0 ; 
assign pcu_lsu_inst_vld = iss_vld_ex[0] & iss_funct_ex[0][FU_LSU_BIT] & !(iss_excp_vld_ex[0]|core_pseudo_run_state|core_elp_state_vld); 
assign pcu_lsu_inst_cmd[0] = {iss_sub_funct_ex[0][1:0],iss_sub_funct_ex[0][3:2]} & {4{pcu_lsu_inst_vld}}; 
assign pcu_lsu_inst_fence_op[0] = iss_opcode_ex[0][3:0]; 
assign pcu_lsu_inst_dst_vld[0] = iss_inst_dst_vld_ex[0]; 
assign pcu_lsu_inst_size[0] = lsu_size_ex; 
assign pcu_lsu_inst_signed = instr_decd_signed[0]; 
assign ls_post_addr = instr_decd_opcode[0][LS_POST]; 
assign pcu_lsu_inst_addr_fwd = iss0_rs_lsu_fwd_lsu_ex[0] | ls_post_addr; 
assign pcu_lsu_inst_addr_fwd_val[0] = ls_post_addr? (iss0_op1_fwd_sel_ex ? iss0_op1_fwd_val_ex : mrf_gpr_rd_data[0]) : iss0_rs1_raw_iss_vld[1]? iss1_op2_mv_fwd_val_wb : lsu_pcu_cmt_data[0]; 
assign pcu_lsu_inst_data[0] = 
uop_csr_cs_st & !(cmt_ls_exe_2_cycs_stall_ex & cmt_inst_sub_funct[0][ST_INST])? pcu_csr_rdata_all : 
cmt_ls_exe_2_cycs_stall_ex & cmt_inst_sub_funct[0][ST_INST]? mrf_gpr_rd_data_wb[1] : 
iss0_rs_fwd_ex[1][1] ? mrf_gpr_wr_data[1] : 
iss0_rs_fwd_ex[0][1] & !(cmt_ls_exe_2_cycs_stall_ex & cmt_inst_sub_funct[0][ST_INST]) ? fwd_wb_data : 
mrf_gpr_rd_data[1]; 
assign pcu_lsu_inst_data_wb_vld[0] = cmt_ls_exe_2_cycs_stall_ex & cmt_inst_sub_funct[0][ST_INST]; 
assign cmt_inst_vld = iss_vld_wb; 
assign cmt_inst_funct = iss_funct_wb; 
assign cmt_inst_sub_funct = iss_sub_funct_wb; 
assign cmt_inst_opcode = iss_opcode_wb; 
assign cmt_dest_vld = iss_gpr_dst_vld_wb ; 
assign cmt_dest = iss_gpr_dst_idx_wb ; 
assign cmt_pc = iss_pc_wb; 
assign cmt_excp_vld = iss_excpt_wb[0]; 
assign cmt_excp_cause = iss_excpt_code_wb[0]; 
assign cmt_inst_en = iss_inst_cmt_en_wb[0]; 
assign cmt_trigger_hit_idx = iss_vld_wb[0] ? iss_trigger_hit_idx[0] : {4{1'b0}}; 
assign cmt_uop_int_mask = iss_uop_int_mask_wb; 
assign cmt_trig_exp = iss_trig_exp_wb[0]; 
assign cmt_trig_dbg = iss_trig_dbg_wb[0]; 
assign cmt_uop_inst = uop_inst_wb[0]; 
assign cmt_br_tkn = iss_inst_br_tkn_wb[0]; 
assign cmt_inst_type = iss_inst_type_wb; 
assign cmt_csr_idx_wb = iss_csr_idx_wb[0]; 
assign pcu_ifu_brq_rd = pcu_ifu_ret_rslv; 
assign pcu_ifu_call_rslv = instr_decd_is_call & iss_vld_ex[0] & !core_dbg_mode; 
assign pcu_ifu_ret_rslv = (instr_decd_is_ret | split_ret_state)& iss_vld_ex[0] & !core_dbg_mode; 
assign jvt_inst_ex = iss_cm_jvt_inst & !(cmt_flush); 
logic uncond_j; 
logic cond_br; 
assign uncond_j = iss_vld_ex[0] & iss_funct_ex[0][FU_BRU_BIT]& (~iss_sub_funct_ex[0][1]); 
assign cond_br = iss_vld_ex[0] & iss_funct_ex[0][FU_BRU_BIT]& (iss_sub_funct_ex[0][1]); 
assign ex_inst_vld = ifu_pcu_vld[0]; 
assign ex_branch_status = {1'b0,uncond_j,cond_br & ~br_offset[31],cond_br & br_offset[31]}; 
assign ex_pc_addr = ifu_pcu_instr_pc[0]; 
logic iss_src_raw; 
assign iss_src_raw = iss0_gpr_src_raw; 
m130_pcu_uop u_uop( 
.clk (clk ), 
.rst_n (rst_n ), 
.ifu_pcu_instr_data (ifu_pcu_instr_data[0] ), 
.instr_decd_src1_idx (instr_decd_src1_idx ), 
.instr_decd_src2_idx (instr_decd_src2_idx ), 
.instr_decd_dest_idx (instr_decd_dest_idx ), 
.instr_decd_op2_src (instr_decd_op2_src ), 
.core_dbg_mode (core_dbg_mode ), 
.cmt_flush (cmt_flush ), 
.iss_vld (iss_vld_ex[0] ), 
.iss_src_raw (iss_src_raw ), 
.uop_shift (uop_shift ), 
.uop_pop (uop_pop ), 
.uop_pop_ret (uop_pop_ret ), 
.uop_popretz (uop_popretz ), 
.uop_pop_all (uop_pop_all ), 
.uop_push (uop_push ), 
.uop_mv (uop_mv ), 
.uop_inst (uop_inst ), 
.uop_csr_cs (uop_csr_cs ), 
.uop_ld_dest (uop_ld_dest ), 
.uop_st_src (uop_st_src ), 
.ls_exe_2_cycs_cmt (ls_exe_2_cycs_cmt ), 
.pcu_csr_split_integrity (pcu_csr_split_integrity ), 
.uop_dst_vld (uop_dst_vld ), 
.uop_dst_idx (uop_dst_idx ), 
.uop_rs1_vld (uop_rs1_vld ), 
.uop_rs1_idx (uop_rs1_idx ), 
.uop_rs2_vld (uop_rs2_vld ), 
.uop_rs2_idx (uop_rs2_idx ), 
.uop_rs2_fr_imm_vld (uop_rs2_fr_imm_vld ), 
.uop_rs2_fr_imm (uop_rs2_fr_imm ), 
.uop_rs1_src_vld (uop_rs1_src_vld ), 
.uop_rs1_src (uop_rs1_src ), 
.uop_rs2_src_vld (uop_rs2_src_vld ), 
.uop_rs2_src (uop_rs2_src ), 
.uop_funct_vld (uop_funct_vld ), 
.uop_funct (uop_funct ), 
.uop_sub_funct_vld (uop_sub_funct_vld ), 
.uop_sub_funct (uop_sub_funct ), 
.uop_opcode_vld (uop_opcode_vld ), 
.uop_opcode (uop_opcode ), 
.uop_stall (uop_stall ), 
.uop_int_mask (uop_int_mask ), 
.uop_inst_cmt_en (uop_inst_cmt_en ), 
.split_ret_state (split_ret_state ), 
.uop_ill (uop_ill ), 
.shf_uop_fwd (shf_uop_fwd ), 
.split_idle_state (split_idle_state ), 
.split_uop_state (split_uop_state ), 
.iss1_uop_vld (iss1_uop_vld ) 
); 
assign ls_post_pre_addr_dst_idx_wb = uop_rs2_idx[0]; 
endmodule
 
 
 
module m130_pcu_mrf( 
input logic clk , 
input logic rst_n , 
input logic top_polarity , 
input logic areg_init , 
input logic [4-1:0][$clog2(32)-1:0] mrf_gpr_rd_idx , 
output logic [4-1:0][32-1:0] mrf_gpr_rd_data , 
output logic [32-1:0] mrf_gpr_x7 , 
input logic [2-1:0] mrf_gpr_wr_vld , 
input logic mrf_gpr_need_cleard , 
input logic [2-1:0][$clog2(32)-1:0] mrf_gpr_wr_idx , 
input logic [2-1:0][32-1:0] mrf_gpr_wr_data 
); 
 
localparam X0 = 5'b0 ; 
localparam X2 = 5'h2 ; 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
localparam FENCE_INST = 0; 
localparam AMO_INST = 1; 
localparam LD_INST = 2; 
localparam ST_INST = 3; 
localparam LS_POST = 5; 
localparam EXCLUSIVE_INST = 0; 
localparam SYS = 0 ; 
localparam SYS_E = 1 ; 
localparam CSR_INST = 2 ; 
localparam BRKPT = 12'h3 ; 
localparam ILL_INST = 12'h2 ; 
localparam LP_ERR = 12'h12 ; 
localparam ECALL_U = 12'h8 ; 
localparam ECALL_M = 12'hb ; 
localparam LD_RAS_ERR = 12'h1F ; 
localparam INST_MISALIGN = 12'h0 ; 
localparam IFU_PMP_ERR = 12'h1 ; 
localparam LD_ACC_FAULT = 12'h5 ; 
localparam LD_MISALIGN = 12'h4 ; 
localparam ST_ACC_FAULT = 12'h7 ; 
localparam ST_MISALIGN = 12'h6 ; 
localparam RAS_ERR = 12'h1f ; 
localparam CSRRW = 0 ; 
localparam CSRRS = 1 ; 
localparam CSRRC = 2 ; 
localparam CSRIMM = 3 ; 
localparam UNAVAIL = 2'h0; 
localparam HALTED = 2'h1; 
localparam RUNNING = 2'h2; 
localparam NMI_ID = 12'hFFF ; 
localparam MRET = 0 ; 
localparam WFI = 1 ; 
localparam MNRET = 2 ; 
localparam ECALL = 0 ; 
localparam EBREAK = 1 ; 
localparam MRASIE = 31 ; 
localparam MEIE = 11 ; 
localparam MTIE = 7 ; 
localparam MSIE = 3 ; 
localparam MRASIP = 31 ; 
localparam MEIP = 11 ; 
localparam MTIP = 7 ; 
localparam MSIP = 3 ; 
typedef enum logic [4:0]{ 
LOAD = 5'b00001 , 
STORE = 5'b00010 , 
FENCE = 5'b00100 
}sub_func_lsu_e ; 
typedef enum logic [4:0] { 
SUB_FU_NONE = 5'b00000 , 
ARITHMATIC = 5'b00001 , 
SHIFT = 5'b00010 , 
LOGICAL = 5'b00100 , 
MISC = 5'b01000 
} sub_func_alu_e; 
typedef enum logic [4:0] { 
UNCOND_DIRE = 5'b00001 , 
COND_DIRE = 5'b00010 , 
UNCOND_INDIRE = 5'b00100 
}sub_func_bru_e ; 
typedef enum logic [3:0] { 
OTHER_TYPES = 4'd0 , 
EXCP_TYPE = 4'd1 , 
INT_TYPE = 4'd2 , 
MRET_TYPE = 4'd3 , 
NT_BR_TYPE = 4'd4 , 
T_BR_TYPE = 4'd5 , 
UNINF_TYPE = 4'd6 
}itype; 
typedef struct packed{ 
logic interrupt; 
logic minhv; 
logic [1:0] mpp; 
logic mpie; 
logic [7:0] mpil; 
logic [11:0] exccode; 
} mcause_csr; 
typedef struct packed{ 
logic mprv; 
logic [1:0] mpp; 
logic mpie; 
logic mie; 
} mstatus_csr; 
typedef struct packed{ 
logic mnpelp; 
logic [1:0] mnpp; 
logic mnpv; 
logic nmie; 
}mnstatus_csr; 
typedef struct packed{ 
logic [3:0] debuger ; 
logic pelp ; 
logic ebreakvs ; 
logic ebreakvu ; 
logic ebreakm ; 
logic ebreaks ; 
logic ebreaku ; 
logic stepie ; 
logic stopcount; 
logic stoptime ; 
logic [2:0] cause ; 
logic v ; 
logic mprven ; 
logic nmip ; 
logic step ; 
logic [1:0] prv ; 
}dcsr_csr; 
typedef enum logic [3:0] { 
SPLIT_IDLE = 4'b0000 , 
SPLIT_UOP = 4'b0001 , 
SPLIT_LI = 4'b0010 , 
SPLIT_ADD = 4'b0110 , 
SPLIT_RET = 4'b1000 , 
SPLIT_SHF_ADD = 4'b1100 
} state_e; 
 
localparam M130_CORE_GPR_W = 32; 
logic [4-1:0][32-1:0] mrf_gpr_rd_data_tmp ; 
logic [32-1:0][32-1:0] gpr_dout ; 
logic [32-1:0][32-1:0] gpr_din ; 
logic [32-1:0][1:0] gpr_din_parity ; 
logic [32-1:0] gpr_en ; 
logic [4-1:0][32-1:0] gpr_rd_idx_oht ; 
logic [2-1:0][32-1:0] gpr_wr_idx_oht ; 
logic [32-1:0] mrf_gpr_cleared ; 
logic [32-1:0] mrf_gpr_wr_polarity ; 
genvar i,j; 
integer x; 
logic areg_init_after ; 
assign areg_init_after = areg_init; 
assign gpr_din[0] = {32{1'b0}}; 
assign gpr_en[0] = 1'b0; 
assign gpr_wr_idx_oht[0][0] = 1'b0; 
assign gpr_rd_idx_oht[2][0] = 1'b0; 
assign gpr_rd_idx_oht[3][0] = 1'b0; 
assign gpr_wr_idx_oht[1][0] = 1'b0; 
assign gpr_rd_idx_oht[0][0] = 1'b0; 
assign gpr_rd_idx_oht[1][0] = 1'b0; 
assign gpr_dout[0] = {{32{1'b0}}}; 
logic [2-1:0][32-1 :0] mrf_gpr_wr_data_fnl; 
logic [2-1:0][1:0][32/2-1 :0] mrf_gpr_wr_data_bef; 
logic [2-1:0][1:0] mrf_gpr_wr_parity ; 
generate 
for(j = 0; j < 2; j = j + 1) begin: GPR_WR_PORT_GEN 
assign mrf_gpr_wr_data_fnl[j] = mrf_gpr_wr_data[j]; 
assign mrf_gpr_wr_parity[j][0] = 1'b0; 
assign mrf_gpr_wr_parity[j][1] = 1'b0; 
end 
for(i = 0; i < 4; i = i + 1) begin: GPR_CLR_EN_0_3 
assign mrf_gpr_cleared[i] = 1'b0; 
assign mrf_gpr_wr_polarity[i] = areg_init_after ? 1'b0 : top_polarity; 
end 
for(i = 4; i < 32; i = i + 1) begin: GPR_CLR_EN_4_X 
assign mrf_gpr_cleared[i] = mrf_gpr_need_cleard; 
assign mrf_gpr_wr_polarity[i] = (areg_init_after | mrf_gpr_need_cleard)? 1'b0 : top_polarity; 
end 
for(i = 1; i < 32; i = i + 1) begin: GPR_GEN 
for(j = 0; j < 2; j = j + 1) begin: GPR_WR_PORT_GEN 
assign gpr_wr_idx_oht[j][i] = mrf_gpr_wr_vld[j] & (mrf_gpr_wr_idx[j] == i); 
end 
assign gpr_en[i] = gpr_wr_idx_oht[0][i] | gpr_wr_idx_oht[1][i] | areg_init_after | mrf_gpr_cleared[i]; 
assign gpr_din[i] = (areg_init_after | mrf_gpr_cleared[i])? {32{1'b0}} : gpr_wr_idx_oht[1][i] ? mrf_gpr_wr_data_fnl[1] : mrf_gpr_wr_data_fnl[0] ; 
assign gpr_din_parity[i][0] = (areg_init_after | mrf_gpr_cleared[i])? 1'b1 : gpr_wr_idx_oht[1][i] ? mrf_gpr_wr_parity[1][0] : mrf_gpr_wr_parity[0][0] ; 
assign gpr_din_parity[i][1] = (areg_init_after | mrf_gpr_cleared[i])? 1'b0 : gpr_wr_idx_oht[1][i] ? mrf_gpr_wr_parity[1][1] : mrf_gpr_wr_parity[0][1] ; 
`WDFFENR(gpr_dout[i], gpr_din[i], gpr_en[i], clk) 
for(j = 0; j < 4; j = j + 1) begin: GPR_RD_IDX_GEN 
assign gpr_rd_idx_oht[j][i] = (mrf_gpr_rd_idx[j] == i); 
end 
end 
for(i = 0; i < 4; i = i + 1) begin: GPR_RD_PORT_GEN 
always@(*) begin 
mrf_gpr_rd_data_tmp[i] = {M130_CORE_GPR_W{1'b0}}; 
for(x = 0; x < 32 ; x =x + 1) begin 
mrf_gpr_rd_data_tmp[i] = ({M130_CORE_GPR_W{gpr_rd_idx_oht[i][x]}} & gpr_dout[x]) | mrf_gpr_rd_data_tmp[i]; 
end 
mrf_gpr_rd_data[i] = mrf_gpr_rd_data_tmp[i]; 
end 
end 
endgenerate 
assign mrf_gpr_x7 = (mrf_gpr_wr_vld[0] & (mrf_gpr_wr_idx[0] == 5'h7)) ? mrf_gpr_wr_data[0] : gpr_dout[7]; 
endmodule
 
 
 
module m130_pcu_csr ( 
input logic clk , 
input logic rst_n , 
input logic [32-1:0] top_hart_id , 
input logic [7:0] top_sr_lock , 
input logic core_dbg_mode , 
input logic ebreak_dbg_vld , 
input logic debug_mode_enter , 
input logic [63:0] mem_map_mtime , 
input logic [1:0] core_priv_mode , 
input logic dbg_disable , 
input logic [2-1:0] cmt_inst_retire , 
input logic dcsr_stopcount , 
input logic dcsr_stoptime , 
input logic pcu_csr_vld , 
input logic [1:0] pcu_csr_opcode , 
input logic [11:0] pcu_csr_index , 
input logic [32-1:0] pcu_csr_wdata , 
input logic pcu_csr_has_write_op_ex , 
output logic [32-1:0] csr_par_rdata , 
output logic csr_par_match , 
output logic csr_access_excp , 
output logic [32-1:0] jvt_csr , 
output logic [32-1:0] mlpcfg 
); 
 
localparam X0 = 5'b0 ; 
localparam X2 = 5'h2 ; 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
localparam FENCE_INST = 0; 
localparam AMO_INST = 1; 
localparam LD_INST = 2; 
localparam ST_INST = 3; 
localparam LS_POST = 5; 
localparam EXCLUSIVE_INST = 0; 
localparam SYS = 0 ; 
localparam SYS_E = 1 ; 
localparam CSR_INST = 2 ; 
localparam BRKPT = 12'h3 ; 
localparam ILL_INST = 12'h2 ; 
localparam LP_ERR = 12'h12 ; 
localparam ECALL_U = 12'h8 ; 
localparam ECALL_M = 12'hb ; 
localparam LD_RAS_ERR = 12'h1F ; 
localparam INST_MISALIGN = 12'h0 ; 
localparam IFU_PMP_ERR = 12'h1 ; 
localparam LD_ACC_FAULT = 12'h5 ; 
localparam LD_MISALIGN = 12'h4 ; 
localparam ST_ACC_FAULT = 12'h7 ; 
localparam ST_MISALIGN = 12'h6 ; 
localparam RAS_ERR = 12'h1f ; 
localparam CSRRW = 0 ; 
localparam CSRRS = 1 ; 
localparam CSRRC = 2 ; 
localparam CSRIMM = 3 ; 
localparam UNAVAIL = 2'h0; 
localparam HALTED = 2'h1; 
localparam RUNNING = 2'h2; 
localparam NMI_ID = 12'hFFF ; 
localparam MRET = 0 ; 
localparam WFI = 1 ; 
localparam MNRET = 2 ; 
localparam ECALL = 0 ; 
localparam EBREAK = 1 ; 
localparam MRASIE = 31 ; 
localparam MEIE = 11 ; 
localparam MTIE = 7 ; 
localparam MSIE = 3 ; 
localparam MRASIP = 31 ; 
localparam MEIP = 11 ; 
localparam MTIP = 7 ; 
localparam MSIP = 3 ; 
typedef enum logic [4:0]{ 
LOAD = 5'b00001 , 
STORE = 5'b00010 , 
FENCE = 5'b00100 
}sub_func_lsu_e ; 
typedef enum logic [4:0] { 
SUB_FU_NONE = 5'b00000 , 
ARITHMATIC = 5'b00001 , 
SHIFT = 5'b00010 , 
LOGICAL = 5'b00100 , 
MISC = 5'b01000 
} sub_func_alu_e; 
typedef enum logic [4:0] { 
UNCOND_DIRE = 5'b00001 , 
COND_DIRE = 5'b00010 , 
UNCOND_INDIRE = 5'b00100 
}sub_func_bru_e ; 
typedef enum logic [3:0] { 
OTHER_TYPES = 4'd0 , 
EXCP_TYPE = 4'd1 , 
INT_TYPE = 4'd2 , 
MRET_TYPE = 4'd3 , 
NT_BR_TYPE = 4'd4 , 
T_BR_TYPE = 4'd5 , 
UNINF_TYPE = 4'd6 
}itype; 
typedef struct packed{ 
logic interrupt; 
logic minhv; 
logic [1:0] mpp; 
logic mpie; 
logic [7:0] mpil; 
logic [11:0] exccode; 
} mcause_csr; 
typedef struct packed{ 
logic mprv; 
logic [1:0] mpp; 
logic mpie; 
logic mie; 
} mstatus_csr; 
typedef struct packed{ 
logic mnpelp; 
logic [1:0] mnpp; 
logic mnpv; 
logic nmie; 
}mnstatus_csr; 
typedef struct packed{ 
logic [3:0] debuger ; 
logic pelp ; 
logic ebreakvs ; 
logic ebreakvu ; 
logic ebreakm ; 
logic ebreaks ; 
logic ebreaku ; 
logic stepie ; 
logic stopcount; 
logic stoptime ; 
logic [2:0] cause ; 
logic v ; 
logic mprven ; 
logic nmip ; 
logic step ; 
logic [1:0] prv ; 
}dcsr_csr; 
typedef enum logic [3:0] { 
SPLIT_IDLE = 4'b0000 , 
SPLIT_UOP = 4'b0001 , 
SPLIT_LI = 4'b0010 , 
SPLIT_ADD = 4'b0110 , 
SPLIT_RET = 4'b1000 , 
SPLIT_SHF_ADD = 4'b1100 
} state_e; 
 
 
 
typedef enum logic[1:0] { 
PRIV_MODE_M = 2'b11, 
PRIV_MODE_S = 2'b01, 
PRIV_MODE_U = 2'b00 
} priv_mode_e; 
typedef enum logic[1:0] { 
CSR_OP_READ = 2'b00, 
CSR_OP_WRITE = 2'b01, 
CSR_OP_SET = 2'b10, 
CSR_OP_CLEAR = 2'b11 
} csr_opcode_e; 
typedef enum logic [11:0] { 
CSR_FFLAGS_ADDR = 12'h001, 
CSR_FRM_ADDR = 12'h002, 
CSR_FCSR_ADDR = 12'h003, 
CSR_FTRAN_ADDR = 12'h800, 
CSR_SSTATUS_ADDR = 12'h100, 
CSR_SIE_ADDR = 12'h104, 
CSR_STVEC_ADDR = 12'h105, 
CSR_SCOUNTEREN_ADDR = 12'h106, 
CSR_SSCRATCH_ADDR = 12'h140, 
CSR_SEPC_ADDR = 12'h141, 
CSR_SCAUSE_ADDR = 12'h142, 
CSR_STVAL_ADDR = 12'h143, 
CSR_SIP_ADDR = 12'h144, 
CSR_SATP_ADDR = 12'h180, 
CSR_MSTATUS_ADDR = 12'h300, 
CSR_MISA_ADDR = 12'h301, 
CSR_MEDELEG_ADDR = 12'h302, 
CSR_MIDELEG_ADDR = 12'h303, 
CSR_MIE_ADDR = 12'h304, 
CSR_MTVEC_ADDR = 12'h305, 
CSR_MCOUNTEREN_ADDR = 12'h306, 
CSR_MSTATUSH_ADDR = 12'h310, 
CSR_MSCRATCH_ADDR = 12'h340, 
CSR_MEPC_ADDR = 12'h341, 
CSR_MCAUSE_ADDR = 12'h342, 
CSR_MTVAL_ADDR = 12'h343, 
CSR_MIP_ADDR = 12'h344, 
CSR_MENVCFG_ADDR = 12'h30A, 
CSR_MENVCFGH_ADDR = 12'h31A, 
CSR_MSECCFG_ADDR = 12'h747, 
CSR_MSECCFGH_ADDR = 12'h757, 
CSR_PMPCFG0_ADDR = 12'h3A0, 
CSR_PMPCFG1_ADDR = 12'h3A1, 
CSR_PMPCFG2_ADDR = 12'h3A2, 
CSR_PMPCFG3_ADDR = 12'h3A3, 
CSR_PMPADDR0_ADDR = 12'h3B0, 
CSR_PMPADDR1_ADDR = 12'h3B1, 
CSR_PMPADDR2_ADDR = 12'h3B2, 
CSR_PMPADDR3_ADDR = 12'h3B3, 
CSR_PMPADDR4_ADDR = 12'h3B4, 
CSR_PMPADDR5_ADDR = 12'h3B5, 
CSR_PMPADDR6_ADDR = 12'h3B6, 
CSR_PMPADDR7_ADDR = 12'h3B7, 
CSR_PMPADDR8_ADDR = 12'h3B8, 
CSR_PMPADDR9_ADDR = 12'h3B9, 
CSR_PMPADDR10_ADDR = 12'h3BA, 
CSR_PMPADDR11_ADDR = 12'h3BB, 
CSR_PMPADDR12_ADDR = 12'h3BC, 
CSR_PMPADDR13_ADDR = 12'h3BD, 
CSR_PMPADDR14_ADDR = 12'h3BE, 
CSR_PMPADDR15_ADDR = 12'h3BF, 
CSR_MVENDORID_ADDR = 12'hF11, 
CSR_MARCHID_ADDR = 12'hF12, 
CSR_MIMPID_ADDR = 12'hF13, 
CSR_MHARTID_ADDR = 12'hF14, 
CSR_MCONFIGPTRID_ADDR = 12'hF15, 
CSR_MCYCLE_ADDR = 12'hB00, 
CSR_MCYCLEH_ADDR = 12'hB80, 
CSR_MINSTRET_ADDR = 12'hB02, 
CSR_MINSTRETH_ADDR = 12'hB82, 
CSR_MHPM_COUNTER_3_ADDR = 12'hB03, 
CSR_MHPM_COUNTER_4_ADDR = 12'hB04, 
CSR_MHPM_COUNTER_5_ADDR = 12'hB05, 
CSR_MHPM_COUNTER_6_ADDR = 12'hB06, 
CSR_MHPM_COUNTER_7_ADDR = 12'hB07, 
CSR_MHPM_COUNTER_8_ADDR = 12'hB08, 
CSR_MHPM_COUNTER_9_ADDR = 12'hB09, 
CSR_MHPM_COUNTER_10_ADDR = 12'hB0A, 
CSR_MHPM_COUNTER_11_ADDR = 12'hB0B, 
CSR_MHPM_COUNTER_12_ADDR = 12'hB0C, 
CSR_MHPM_COUNTER_13_ADDR = 12'hB0D, 
CSR_MHPM_COUNTER_14_ADDR = 12'hB0E, 
CSR_MHPM_COUNTER_15_ADDR = 12'hB0F, 
CSR_MHPM_COUNTER_16_ADDR = 12'hB10, 
CSR_MHPM_COUNTER_17_ADDR = 12'hB11, 
CSR_MHPM_COUNTER_18_ADDR = 12'hB12, 
CSR_MHPM_COUNTER_19_ADDR = 12'hB13, 
CSR_MHPM_COUNTER_20_ADDR = 12'hB14, 
CSR_MHPM_COUNTER_21_ADDR = 12'hB15, 
CSR_MHPM_COUNTER_22_ADDR = 12'hB16, 
CSR_MHPM_COUNTER_23_ADDR = 12'hB17, 
CSR_MHPM_COUNTER_24_ADDR = 12'hB18, 
CSR_MHPM_COUNTER_25_ADDR = 12'hB19, 
CSR_MHPM_COUNTER_26_ADDR = 12'hB1A, 
CSR_MHPM_COUNTER_27_ADDR = 12'hB1B, 
CSR_MHPM_COUNTER_28_ADDR = 12'hB1C, 
CSR_MHPM_COUNTER_29_ADDR = 12'hB1D, 
CSR_MHPM_COUNTER_30_ADDR = 12'hB1E, 
CSR_MHPM_COUNTER_31_ADDR = 12'hB1F, 
CSR_MHPM_COUNTER_3H_ADDR = 12'hB83, 
CSR_MHPM_COUNTER_4H_ADDR = 12'hB84, 
CSR_MHPM_COUNTER_5H_ADDR = 12'hB85, 
CSR_MHPM_COUNTER_6H_ADDR = 12'hB86, 
CSR_MHPM_COUNTER_7H_ADDR = 12'hB87, 
CSR_MHPM_COUNTER_8H_ADDR = 12'hB88, 
CSR_MHPM_COUNTER_9H_ADDR = 12'hB89, 
CSR_MHPM_COUNTER_10H_ADDR = 12'hB8A, 
CSR_MHPM_COUNTER_11H_ADDR = 12'hB8B, 
CSR_MHPM_COUNTER_12H_ADDR = 12'hB8C, 
CSR_MHPM_COUNTER_13H_ADDR = 12'hB8D, 
CSR_MHPM_COUNTER_14H_ADDR = 12'hB8E, 
CSR_MHPM_COUNTER_15H_ADDR = 12'hB8F, 
CSR_MHPM_COUNTER_16H_ADDR = 12'hB90, 
CSR_MHPM_COUNTER_17H_ADDR = 12'hB91, 
CSR_MHPM_COUNTER_18H_ADDR = 12'hB92, 
CSR_MHPM_COUNTER_19H_ADDR = 12'hB93, 
CSR_MHPM_COUNTER_20H_ADDR = 12'hB94, 
CSR_MHPM_COUNTER_21H_ADDR = 12'hB95, 
CSR_MHPM_COUNTER_22H_ADDR = 12'hB96, 
CSR_MHPM_COUNTER_23H_ADDR = 12'hB97, 
CSR_MHPM_COUNTER_24H_ADDR = 12'hB98, 
CSR_MHPM_COUNTER_25H_ADDR = 12'hB99, 
CSR_MHPM_COUNTER_26H_ADDR = 12'hB9A, 
CSR_MHPM_COUNTER_27H_ADDR = 12'hB9B, 
CSR_MHPM_COUNTER_28H_ADDR = 12'hB9C, 
CSR_MHPM_COUNTER_29H_ADDR = 12'hB9D, 
CSR_MHPM_COUNTER_30H_ADDR = 12'hB9E, 
CSR_MHPM_COUNTER_31H_ADDR = 12'hB9F, 
CSR_MCOUNTINHIBIT_ADDR = 12'h320, 
CSR_MHPM_EVENT_3_ADDR = 12'h323, 
CSR_MHPM_EVENT_4_ADDR = 12'h324, 
CSR_MHPM_EVENT_5_ADDR = 12'h325, 
CSR_MHPM_EVENT_6_ADDR = 12'h326, 
CSR_MHPM_EVENT_7_ADDR = 12'h327, 
CSR_MHPM_EVENT_8_ADDR = 12'h328, 
CSR_MHPM_EVENT_9_ADDR = 12'h329, 
CSR_MHPM_EVENT_10_ADDR = 12'h32a, 
CSR_MHPM_EVENT_11_ADDR = 12'h32b, 
CSR_MHPM_EVENT_12_ADDR = 12'h32c, 
CSR_MHPM_EVENT_13_ADDR = 12'h32d, 
CSR_MHPM_EVENT_14_ADDR = 12'h32e, 
CSR_MHPM_EVENT_15_ADDR = 12'h32f, 
CSR_MHPM_EVENT_16_ADDR = 12'h330, 
CSR_MHPM_EVENT_17_ADDR = 12'h331, 
CSR_MHPM_EVENT_18_ADDR = 12'h332, 
CSR_MHPM_EVENT_19_ADDR = 12'h333, 
CSR_MHPM_EVENT_20_ADDR = 12'h334, 
CSR_MHPM_EVENT_21_ADDR = 12'h335, 
CSR_MHPM_EVENT_22_ADDR = 12'h336, 
CSR_MHPM_EVENT_23_ADDR = 12'h337, 
CSR_MHPM_EVENT_24_ADDR = 12'h338, 
CSR_MHPM_EVENT_25_ADDR = 12'h339, 
CSR_MHPM_EVENT_26_ADDR = 12'h33a, 
CSR_MHPM_EVENT_27_ADDR = 12'h33b, 
CSR_MHPM_EVENT_28_ADDR = 12'h33c, 
CSR_MHPM_EVENT_29_ADDR = 12'h33d, 
CSR_MHPM_EVENT_30_ADDR = 12'h33e, 
CSR_MHPM_EVENT_31_ADDR = 12'h33f, 
CSR_MTVT_ADDR = 12'h307, 
CSR_MNXTI_ADDR = 12'h345, 
CSR_MINTSTATUS_ADDR = 12'hfb1, 
CSR_MINTTHRESH_ADDR = 12'h347, 
CSR_MSCRATCHCSW_ADDR = 12'h348, 
CSR_MSCRATCHCSWL_ADDR = 12'h349, 
CSR_MNSCRATCH_ADDR = 12'h740, 
CSR_MNEPC_ADDR = 12'h741, 
CSR_MNCAUSE_ADDR = 12'h742, 
CSR_MNSTATUS_ADDR = 12'h744, 
CSR_MNVEC_ADDR = 12'hFC0, 
CSR_TSELECT_ADDR = 12'h7A0, 
CSR_TDATA1_ADDR = 12'h7A1, 
CSR_TDATA2_ADDR = 12'h7A2, 
CSR_TDATA3_ADDR = 12'h7A3, 
CSR_TINFO_ADDR = 12'h7A4, 
CSR_TCONTROL_ADDR = 12'h7A5, 
CSR_DCSR_ADDR = 12'h7b0, 
CSR_DPC_ADDR = 12'h7b1, 
CSR_DSCRATCH0_ADDR = 12'h7b2, 
CSR_DSCRATCH1_ADDR = 12'h7b3, 
CSR_DSCRATCH2_ADDR = 12'h7c0, 
CSR_DSCRATCH3_ADDR = 12'h7c1, 
CSR_MCONTEXTSW_ADDR = 12'h7c2, 
CSR_MSECURFEAT_ADDR = 12'h7c3, 
CSR_MLPCFG_ADDR = 12'h7c4, 
CSR_CYCLE_ADDR = 12'hC00, 
CSR_CYCLEH_ADDR = 12'hC80, 
CSR_TIME_ADDR = 12'hC01, 
CSR_TIMEH_ADDR = 12'hC81, 
CSR_INSTRET_ADDR = 12'hC02, 
CSR_INSTRETH_ADDR = 12'hC82, 
CSR_HPM_COUNTER_3_ADDR = 12'hC03, 
CSR_HPM_COUNTER_4_ADDR = 12'hC04, 
CSR_HPM_COUNTER_5_ADDR = 12'hC05, 
CSR_HPM_COUNTER_6_ADDR = 12'hC06, 
CSR_HPM_COUNTER_7_ADDR = 12'hC07, 
CSR_HPM_COUNTER_8_ADDR = 12'hC08, 
CSR_HPM_COUNTER_9_ADDR = 12'hC09, 
CSR_HPM_COUNTER_10_ADDR = 12'hC0A, 
CSR_HPM_COUNTER_11_ADDR = 12'hC0B, 
CSR_HPM_COUNTER_12_ADDR = 12'hC0C, 
CSR_HPM_COUNTER_13_ADDR = 12'hC0D, 
CSR_HPM_COUNTER_14_ADDR = 12'hC0E, 
CSR_HPM_COUNTER_15_ADDR = 12'hC0F, 
CSR_HPM_COUNTER_16_ADDR = 12'hC10, 
CSR_HPM_COUNTER_17_ADDR = 12'hC11, 
CSR_HPM_COUNTER_18_ADDR = 12'hC12, 
CSR_HPM_COUNTER_19_ADDR = 12'hC13, 
CSR_HPM_COUNTER_20_ADDR = 12'hC14, 
CSR_HPM_COUNTER_21_ADDR = 12'hC15, 
CSR_HPM_COUNTER_22_ADDR = 12'hC16, 
CSR_HPM_COUNTER_23_ADDR = 12'hC17, 
CSR_HPM_COUNTER_24_ADDR = 12'hC18, 
CSR_HPM_COUNTER_25_ADDR = 12'hC19, 
CSR_HPM_COUNTER_26_ADDR = 12'hC1A, 
CSR_HPM_COUNTER_27_ADDR = 12'hC1B, 
CSR_HPM_COUNTER_28_ADDR = 12'hC1C, 
CSR_HPM_COUNTER_29_ADDR = 12'hC1D, 
CSR_HPM_COUNTER_30_ADDR = 12'hC1E, 
CSR_HPM_COUNTER_31_ADDR = 12'hC1F, 
CSR_HPM_COUNTER_3H_ADDR = 12'hC83, 
CSR_HPM_COUNTER_4H_ADDR = 12'hC84, 
CSR_HPM_COUNTER_5H_ADDR = 12'hC85, 
CSR_HPM_COUNTER_6H_ADDR = 12'hC86, 
CSR_HPM_COUNTER_7H_ADDR = 12'hC87, 
CSR_HPM_COUNTER_8H_ADDR = 12'hC88, 
CSR_HPM_COUNTER_9H_ADDR = 12'hC89, 
CSR_HPM_COUNTER_10H_ADDR = 12'hC8A, 
CSR_HPM_COUNTER_11H_ADDR = 12'hC8B, 
CSR_HPM_COUNTER_12H_ADDR = 12'hC8C, 
CSR_HPM_COUNTER_13H_ADDR = 12'hC8D, 
CSR_HPM_COUNTER_14H_ADDR = 12'hC8E, 
CSR_HPM_COUNTER_15H_ADDR = 12'hC8F, 
CSR_HPM_COUNTER_16H_ADDR = 12'hC90, 
CSR_HPM_COUNTER_17H_ADDR = 12'hC91, 
CSR_HPM_COUNTER_18H_ADDR = 12'hC92, 
CSR_HPM_COUNTER_19H_ADDR = 12'hC93, 
CSR_HPM_COUNTER_20H_ADDR = 12'hC94, 
CSR_HPM_COUNTER_21H_ADDR = 12'hC95, 
CSR_HPM_COUNTER_22H_ADDR = 12'hC96, 
CSR_HPM_COUNTER_23H_ADDR = 12'hC97, 
CSR_HPM_COUNTER_24H_ADDR = 12'hC98, 
CSR_HPM_COUNTER_25H_ADDR = 12'hC99, 
CSR_HPM_COUNTER_26H_ADDR = 12'hC9A, 
CSR_HPM_COUNTER_27H_ADDR = 12'hC9B, 
CSR_HPM_COUNTER_28H_ADDR = 12'hC9C, 
CSR_HPM_COUNTER_29H_ADDR = 12'hC9D, 
CSR_HPM_COUNTER_30H_ADDR = 12'hC9E, 
CSR_HPM_COUNTER_31H_ADDR = 12'hC9F, 
CSR_JVT_ADDR = 12'h017 
} csr_reg_t; 
 
localparam E_EXT = 
1'b0 ; 
localparam A_EXT = 
1'b0 ; 
localparam B_EXT = 
1'b1 ; 
localparam C_EXT = 
1'b1 ; 
localparam M_EXT = 
1'b1 ; 
localparam U_EXT = 
1'b1 ; 
localparam S_EXT = 1'b0; 
localparam H_EXT = 1'b0; 
localparam F_EXT = 1'b0; 
localparam D_EXT = 1'b0; 
localparam I_EXT = 
1'b1 ; 
localparam V_EXT = 1'b0; 
localparam P_EXT = 1'b0; 
localparam Q_EXT = 1'b0; 
localparam X_EXT = 
1'b1 ; 
localparam MXL = 2'b1; 
localparam MCYCLE_H_EXIST = 32 > 32 ; 
localparam MINSTRET_H_EXIST = 32 > 32; 
logic csr_only_r ; 
logic csr_only_m_mode ; 
logic [32-1:0] csr_wdata ; 
logic [32-1:0] op_val ; 
logic [32-1:0] mvendorid ; 
logic [32-1:0] marchid ; 
logic [32-1:0] mimpid ; 
logic [32-1:0] mhartid ; 
logic [32-1:0] mconfigptr ; 
logic [32-1:0] misa ; 
logic [32-1:0] mcounteren ; 
logic mcounteren_updt_en ; 
logic [32-1:0] medeleg ; 
logic [32-1:0] mideleg ; 
logic [32-1:0] mcycle ; 
logic [32-1:0] mcycle_pre ; 
logic [32-1:0] mcycleh ; 
logic [32-1:0] mcycleh_pre ; 
logic [32*2-1:0] mcycle_plus1 ; 
logic [32-1:0] u_cycle ; 
logic [32-1:0] u_cycleh ; 
logic [32*2-1:0] u_time_all ; 
logic [32-1:0] u_time ; 
logic [32-1:0] u_timeh ; 
logic [32-1:0] mcycle_d ; 
logic mcycle_updt_en ; 
logic [32*2-1:0] minstret_plus ; 
logic [32-1:0] minstret_pre ; 
logic [32-1:0] minstret ; 
logic [32-1:0] u_instret ; 
logic [32-1:0] minstret_d ; 
logic minstret_updt_en ; 
logic [32-1:0] mcycleh_d ; 
logic mcycleh_updt_en ; 
logic [32-1:0] minstreth_pre ; 
logic [32-1:0] minstreth ; 
logic [32-1:0] u_instreth ; 
logic [32-1:0] minstreth_d ; 
logic minstreth_updt_en ; 
logic [32-1:0] mhpmcounter3 ; 
logic [32-1:0] mhpmcounter4 ; 
logic [32-1:0] mhpmcounter5 ; 
logic [32-1:0] mhpmcounter6 ; 
logic [32-1:0] mhpmcounter7 ; 
logic [32-1:0] mhpmcounter8 ; 
logic [32-1:0] mhpmcounter9 ; 
logic [32-1:0] mhpmcounter10 ; 
logic [32-1:0] mhpmcounter11 ; 
logic [32-1:0] mhpmcounter12 ; 
logic [32-1:0] mhpmcounter13 ; 
logic [32-1:0] mhpmcounter14 ; 
logic [32-1:0] mhpmcounter15 ; 
logic [32-1:0] mhpmcounter16 ; 
logic [32-1:0] mhpmcounter17 ; 
logic [32-1:0] mhpmcounter18 ; 
logic [32-1:0] mhpmcounter19 ; 
logic [32-1:0] mhpmcounter20 ; 
logic [32-1:0] mhpmcounter21 ; 
logic [32-1:0] mhpmcounter22 ; 
logic [32-1:0] mhpmcounter23 ; 
logic [32-1:0] mhpmcounter24 ; 
logic [32-1:0] mhpmcounter25 ; 
logic [32-1:0] mhpmcounter26 ; 
logic [32-1:0] mhpmcounter27 ; 
logic [32-1:0] mhpmcounter28 ; 
logic [32-1:0] mhpmcounter29 ; 
logic [32-1:0] mhpmcounter30 ; 
logic [32-1:0] mhpmcounter31 ; 
logic [32-1:0] mhpmcounter3h ; 
logic [32-1:0] mhpmcounter4h ; 
logic [32-1:0] mhpmcounter5h ; 
logic [32-1:0] mhpmcounter6h ; 
logic [32-1:0] mhpmcounter7h ; 
logic [32-1:0] mhpmcounter8h ; 
logic [32-1:0] mhpmcounter9h ; 
logic [32-1:0] mhpmcounter10h ; 
logic [32-1:0] mhpmcounter11h ; 
logic [32-1:0] mhpmcounter12h ; 
logic [32-1:0] mhpmcounter13h ; 
logic [32-1:0] mhpmcounter14h ; 
logic [32-1:0] mhpmcounter15h ; 
logic [32-1:0] mhpmcounter16h ; 
logic [32-1:0] mhpmcounter17h ; 
logic [32-1:0] mhpmcounter18h ; 
logic [32-1:0] mhpmcounter19h ; 
logic [32-1:0] mhpmcounter20h ; 
logic [32-1:0] mhpmcounter21h ; 
logic [32-1:0] mhpmcounter22h ; 
logic [32-1:0] mhpmcounter23h ; 
logic [32-1:0] mhpmcounter24h ; 
logic [32-1:0] mhpmcounter25h ; 
logic [32-1:0] mhpmcounter26h ; 
logic [32-1:0] mhpmcounter27h ; 
logic [32-1:0] mhpmcounter28h ; 
logic [32-1:0] mhpmcounter29h ; 
logic [32-1:0] mhpmcounter30h ; 
logic [32-1:0] mhpmcounter31h ; 
logic [32-1:0] mcountinhibit ; 
logic mcountinhibit_updt_en ; 
logic [63:0] mtime_bk_dbg ; 
logic jvt_csr_updt_en ; 
logic csr_wr_vld ; 
assign csr_wr_vld = pcu_csr_vld & pcu_csr_opcode[0] & !top_sr_lock[0]; 
assign mvendorid = 32'h77B; 
assign marchid = 32'hE1545130; 
assign mimpid = 32'h13402404; 
assign mhartid = top_hart_id; 
assign mconfigptr= {32{1'b0}}; 
assign misa = {MXL,4'b0,2'b0,X_EXT,1'b0,V_EXT,U_EXT,1'b0,S_EXT,1'b0,Q_EXT,P_EXT,2'b0,M_EXT,3'b0,I_EXT,H_EXT,1'b0,F_EXT,E_EXT,D_EXT,C_EXT,B_EXT,A_EXT}; 
assign mcounteren_updt_en = csr_wr_vld & (pcu_csr_index == CSR_MCOUNTEREN_ADDR); 
`WDFFER(mcounteren,{{(32-3){1'b0}},pcu_csr_wdata[2:0]},mcounteren_updt_en,clk,rst_n) 
assign medeleg = {32{1'b0}}; 
assign mideleg = {32{1'b0}}; 
assign mcountinhibit_updt_en = csr_wr_vld & (pcu_csr_index == CSR_MCOUNTINHIBIT_ADDR); 
`WDFFER(mcountinhibit,{{(32-3){1'b0}},pcu_csr_wdata[2],1'b0,pcu_csr_wdata[0]},mcountinhibit_updt_en,clk,rst_n) 
assign mcycle_plus1 = {mcycleh,mcycle} + 1'b1; 
assign mcycle_updt_en = ((!core_dbg_mode & !ebreak_dbg_vld | (core_dbg_mode | ebreak_dbg_vld)& !dcsr_stopcount) & !mcountinhibit[0] & !dbg_disable| 
csr_wr_vld & (pcu_csr_index == CSR_MCYCLE_ADDR)); 
assign mcycle_d = (csr_wr_vld & (pcu_csr_index == CSR_MCYCLE_ADDR)) ? pcu_csr_wdata : mcycle_plus1[31:0]; 
`WDFFER(mcycle_pre,mcycle_d,mcycle_updt_en,clk,rst_n) 
assign minstret_updt_en = ((!core_dbg_mode & !ebreak_dbg_vld | (core_dbg_mode | ebreak_dbg_vld)& !dcsr_stopcount) & cmt_inst_retire[0] & !mcountinhibit[2] & !dbg_disable| 
csr_wr_vld & (pcu_csr_index == CSR_MINSTRET_ADDR)); 
assign minstret_plus = {minstreth,minstret} + (cmt_inst_retire[1] ? 64'h2 : 64'h1); 
assign minstret_d = (csr_wr_vld & (pcu_csr_index == CSR_MINSTRET_ADDR)) ? pcu_csr_wdata : minstret_plus[31:0]; 
`WDFFER(minstret_pre,minstret_d,minstret_updt_en,clk,rst_n) 
generate 
if(MCYCLE_H_EXIST) begin : GEN_MCYCLE 
assign mcycleh_updt_en = ((!core_dbg_mode & !ebreak_dbg_vld | (core_dbg_mode | ebreak_dbg_vld)& !dcsr_stopcount) & !mcountinhibit[0] & (&mcycle) & !dbg_disable | 
csr_wr_vld & (pcu_csr_index == CSR_MCYCLEH_ADDR)); 
assign mcycleh_d = (csr_wr_vld & (pcu_csr_index == CSR_MCYCLEH_ADDR))? pcu_csr_wdata : mcycle_plus1[63:32]; 
`WDFFER(mcycleh_pre, mcycleh_d, mcycleh_updt_en, clk, rst_n) 
assign mcycle = mcycle_pre; 
assign mcycleh = {{(64 - 32){1'b0}},mcycleh_pre[32-33 : 0]}; 
end 
else begin : GEN_ELSE_MCYCLE 
assign mcycle = {{(32 - 32){1'b0}},mcycle_pre[32-1 : 0]}; 
assign mcycleh = 32'b0; 
end 
endgenerate 
generate 
if(MINSTRET_H_EXIST) begin : GEN_MINSTRET 
assign minstreth_updt_en = ((!core_dbg_mode & !ebreak_dbg_vld | (core_dbg_mode | ebreak_dbg_vld)& !dcsr_stopcount) & cmt_inst_retire[0] & !mcountinhibit[2] & (&minstret[31:1]) & !dbg_disable| 
csr_wr_vld & (pcu_csr_index == CSR_MINSTRETH_ADDR)); 
assign minstreth_d = (csr_wr_vld & (pcu_csr_index == CSR_MINSTRETH_ADDR)) ? pcu_csr_wdata : minstret_plus[63:32]; 
`WDFFER(minstreth_pre, minstreth_d, minstreth_updt_en, clk, rst_n) 
assign minstret = minstret_pre; 
assign minstreth = {{(64 - 32){1'b0}},minstreth_pre[32-33 : 0]}; 
end 
else begin : GEN_ELSE_MINSTRET 
assign minstret = {{(32 - 32){1'b0}},minstret_pre[32-1 : 0]}; 
assign minstreth = 32'b0; 
end 
endgenerate 
assign mhpmcounter3 = 'b0; 
assign mhpmcounter4 = 'b0; 
assign mhpmcounter5 = 'b0; 
assign mhpmcounter6 = 'b0; 
assign mhpmcounter7 = 'b0; 
assign mhpmcounter8 = 'b0; 
assign mhpmcounter9 = 'b0; 
assign mhpmcounter10 = 'b0; 
assign mhpmcounter11 = 'b0; 
assign mhpmcounter12 = 'b0; 
assign mhpmcounter13 = 'b0; 
assign mhpmcounter14 = 'b0; 
assign mhpmcounter15 = 'b0; 
assign mhpmcounter16 = 'b0; 
assign mhpmcounter17 = 'b0; 
assign mhpmcounter18 = 'b0; 
assign mhpmcounter19 = 'b0; 
assign mhpmcounter20 = 'b0; 
assign mhpmcounter21 = 'b0; 
assign mhpmcounter22 = 'b0; 
assign mhpmcounter23 = 'b0; 
assign mhpmcounter24 = 'b0; 
assign mhpmcounter25 = 'b0; 
assign mhpmcounter26 = 'b0; 
assign mhpmcounter27 = 'b0; 
assign mhpmcounter28 = 'b0; 
assign mhpmcounter29 = 'b0; 
assign mhpmcounter30 = 'b0; 
assign mhpmcounter31 = 'b0; 
assign mhpmcounter3h = 'b0; 
assign mhpmcounter4h = 'b0; 
assign mhpmcounter5h = 'b0; 
assign mhpmcounter6h = 'b0; 
assign mhpmcounter7h = 'b0; 
assign mhpmcounter8h = 'b0; 
assign mhpmcounter9h = 'b0; 
assign mhpmcounter10h = 'b0; 
assign mhpmcounter11h = 'b0; 
assign mhpmcounter12h = 'b0; 
assign mhpmcounter13h = 'b0; 
assign mhpmcounter14h = 'b0; 
assign mhpmcounter15h = 'b0; 
assign mhpmcounter16h = 'b0; 
assign mhpmcounter17h = 'b0; 
assign mhpmcounter18h = 'b0; 
assign mhpmcounter19h = 'b0; 
assign mhpmcounter20h = 'b0; 
assign mhpmcounter21h = 'b0; 
assign mhpmcounter22h = 'b0; 
assign mhpmcounter23h = 'b0; 
assign mhpmcounter24h = 'b0; 
assign mhpmcounter25h = 'b0; 
assign mhpmcounter26h = 'b0; 
assign mhpmcounter27h = 'b0; 
assign mhpmcounter28h = 'b0; 
assign mhpmcounter29h = 'b0; 
assign mhpmcounter30h = 'b0; 
assign mhpmcounter31h = 'b0; 
assign u_cycle = mcycle; 
assign u_cycleh = mcycleh; 
assign {u_timeh,u_time} = {{(64-32){1'b0}},mem_map_mtime[32-1:0]}; 
assign u_instret = minstret; 
assign u_instreth = minstreth; 
assign jvt_csr_updt_en = csr_wr_vld & (pcu_csr_index == CSR_JVT_ADDR); 
`WDFFER(jvt_csr,{pcu_csr_wdata[32-1:6],6'b0}, jvt_csr_updt_en, clk, rst_n) 
logic mlpcfg_updt_en; 
logic mlpcfg_ds; 
assign mlpcfg_updt_en = csr_wr_vld & (pcu_csr_index == CSR_MLPCFG_ADDR); 
`WDFFER(mlpcfg_ds,pcu_csr_wdata[0],mlpcfg_updt_en, clk, rst_n) 
assign mlpcfg = {31'b0,mlpcfg_ds}; 
always@* begin 
csr_par_match = 1'b1 ; 
csr_only_r = 1'b0 ; 
csr_only_m_mode = 1'b1 ; 
casez(pcu_csr_index) 
CSR_MVENDORID_ADDR : begin 
csr_par_rdata = mvendorid; 
csr_only_r = 1'b1; 
end 
CSR_MARCHID_ADDR : begin 
csr_par_rdata = marchid; 
csr_only_r = 1'b1; 
end 
CSR_MIMPID_ADDR : begin 
csr_par_rdata = mimpid; 
csr_only_r = 1'b1; 
end 
CSR_MHARTID_ADDR : begin 
csr_par_rdata = mhartid; 
csr_only_r = 1'b1; 
end 
CSR_MCONFIGPTRID_ADDR : begin 
csr_par_rdata = mconfigptr; 
csr_only_r = 1'b1; 
end 
CSR_MISA_ADDR : csr_par_rdata = misa; 
CSR_MCOUNTEREN_ADDR : csr_par_rdata = {{(32-3){1'b0}},mcounteren[2:0]}; 
CSR_MCYCLE_ADDR : begin 
csr_par_rdata = mcycle; 
end 
CSR_MINSTRET_ADDR : csr_par_rdata = minstret; 
CSR_MCYCLEH_ADDR : csr_par_rdata = mcycleh; 
CSR_MINSTRETH_ADDR : csr_par_rdata = minstreth; 
CSR_MCOUNTINHIBIT_ADDR : csr_par_rdata = mcountinhibit; 
CSR_CYCLE_ADDR : begin 
csr_par_rdata = u_cycle; 
csr_only_r = 1'b1; 
csr_only_m_mode = ~mcounteren[0]; 
end 
CSR_CYCLEH_ADDR : begin 
csr_par_rdata = u_cycleh; 
csr_only_r = 1'b1; 
csr_only_m_mode = ~mcounteren[0]; 
end 
CSR_TIME_ADDR : begin 
csr_par_rdata = u_time; 
csr_only_r = 1'b1; 
csr_only_m_mode = ~mcounteren[1]; 
end 
CSR_TIMEH_ADDR : begin 
csr_par_rdata = u_timeh; 
csr_only_r = 1'b1; 
csr_only_m_mode = ~mcounteren[1]; 
end 
CSR_INSTRET_ADDR : begin 
csr_par_rdata = u_instret; 
csr_only_r = 1'b1; 
csr_only_m_mode = ~mcounteren[2]; 
end 
CSR_INSTRETH_ADDR : begin 
csr_par_rdata = u_instreth; 
csr_only_r = 1'b1; 
csr_only_m_mode = ~mcounteren[2]; 
end 
CSR_JVT_ADDR : begin 
csr_par_rdata = jvt_csr; 
csr_only_m_mode = 1'b0 ; 
end 
CSR_MLPCFG_ADDR : begin 
csr_par_rdata = mlpcfg; 
end 
default : begin 
csr_par_match = 1'b0; 
csr_par_rdata = 32'hx; 
end 
endcase 
end 
assign csr_access_excp = pcu_csr_vld & csr_par_match & csr_only_r & pcu_csr_has_write_op_ex | 
pcu_csr_vld & csr_par_match & csr_only_m_mode & (core_priv_mode == PRIV_MODE_U); 
endmodule
 
 
 
module m130_pcu_cmt ( 
input logic clk , 
input logic always_on_clk , 
input logic rst_n , 
input logic [32-1:0] top_boot_pc , 
output logic core_init_busy , 
output logic [1:0] core_priv_mode , 
output logic core_dbg_mode , 
output logic core_sleep_mode , 
output logic core_sleep_wakeup , 
output logic core_locked , 
input logic mss_pcu_icache_init_busy , 
input logic top_mstatus_be , 
output logic core_step_vld , 
input logic ifu_idle , 
input logic lsu_idle , 
input logic mss_idle , 
input logic bmu_idle , 
input logic areg_init , 
output logic priv_mode_parity_err , 
output logic cmt_acc_bus_err , 
output logic cmt_acc_pmp_err , 
output logic cmt_inst_dec_ill_err , 
output logic cmt_inst_lpd_flow_err , 
output logic random_exe_stall , 
input logic gpr_cleared_in_trap , 
input logic dbg_disable , 
input logic [7:0] top_sr_lock , 
output logic pcu_csr_sp_en , 
output logic pcu_csr_split_integrity , 
output logic core_elp_state_vld , 
output logic core_pseudo_run_state , 
output logic pcu_ifu_flush_vld , 
output logic [32-1:0] pcu_ifu_flush_pc , 
output logic pcu_ifu_stall , 
output logic [1:0] pcu_ifu_shv_flush , 
input logic ifu_pcu_hv_done , 
input logic ifu_pcu_hv_excp_vld , 
input logic [32-1:0] ifu_pcu_hv_pc , 
input logic [32-1:0] ifu_pcu_excp_cause , 
input logic [2-1:0] ifu_pcu_vld_ex , 
input logic [2-1:0] ifu_pcu_excp_vld_ex , 
input logic [2-1:0][32-1:0] ifu_pcu_instr_pc , 
output logic pcu_lsu_flush_st_inst_wb , 
output logic pcu_lsu_flush , 
output logic pcu_lsu_non_spec , 
input logic lsu_pcu_non_flush_infly , 
output logic wb_inst_vld , 
output logic [3:0] wb_branch_status , 
output logic [31:0] wb_pc_addr , 
output logic cmt_flush , 
output logic cmt_flush_wo_dbg , 
output logic [32-1:0] cmt_flush_pc , 
output logic flush_wb , 
output logic flush_wb_wo_dbg , 
output logic cmt_stall , 
input logic [2-1:0][8-1:0] instr_decd_funct , 
input logic stall_ex , 
input logic jvt_inst_ex , 
output logic [2-1:0] cmt_inst_retire , 
output logic [2-1:0] cmt_inst_split_retire , 
output logic ls_exe_cmt_data_vld , 
input logic [2-1:0] cmt_inst_vld , 
input logic [2-1:0][8-1:0] cmt_inst_funct , 
input logic [2-1:0][4-1:0] cmt_inst_sub_funct , 
input logic [2-1:0][8-1:0] cmt_inst_opcode , 
input logic [2-1:0] cmt_dest_vld , 
input logic [2-1:0][$clog2(32)-1:0] cmt_dest , 
input logic [1:0][32-1:0] cmt_pc , 
input logic cmt_inst_en , 
input logic cmt_uop_int_mask , 
input logic cmt_excp_vld , 
input logic [11:0] cmt_excp_cause , 
input logic cmt_trig_exp , 
input logic cmt_trig_dbg , 
input logic cmt_uop_inst , 
input logic cmt_br_tkn , 
input logic [2-1:0] cmt_inst_type , 
input logic [2-1:0] cmt_exp_lp_inst , 
input logic [2-1:0] cmt_lp_inst , 
input logic [2-1:0] cmt_ls_addr_write_back , 
input logic cmt_ls_exe_2_cycs_stall_ex, 
input logic [$clog2(32)-1:0] ls_post_pre_addr_dst_idx_wb, 
input logic [2-1:0] exu_pcu_inst_wb_stall , 
input logic [2-1:0][32-1:0] exu_pcu_inst_alu_data , 
input logic [2-1:0][32-1:0] exu_pcu_inst_mdu_data , 
input logic exu_pcu_bru_flush , 
input logic [32-1:0] exu_pcu_bru_flush_pc , 
input logic [1-1:0] lsu_pcu_cmt , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_data , 
input logic [1-1:0] lsu_pcu_cmt_excp , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_excp_cause , 
input logic [1-1:0][32-1:0] lsu_pcu_cmt_excp_addr , 
input logic [1-1:0] lsu_pcu_cmt_trig_dbg , 
output logic [2-1:0] mrf_gpr_wr_vld , 
output logic [2-1:0][$clog2(32)-1:0] mrf_gpr_wr_idx , 
output logic [2-1:0][32-1:0] mrf_gpr_wr_data , 
output logic mrf_gpr_need_cleard , 
input logic [32-1:0] mrf_gpr_x7 , 
output logic [2-1:0] mrf_gpr_dst_vld , 
input logic [11:0] csr_idx_ex , 
input logic csr_vld_ex , 
input logic pcu_csr_has_write_op_ex , 
input logic csr_oth_match , 
input logic [32-1:0] csr_oth_rdata , 
input logic [11:0] cmt_csr_idx_wb , 
output logic pcu_csr_vld , 
output logic [1:0] pcu_csr_opcode , 
output logic [11:0] pcu_csr_index , 
output logic [32-1:0] pcu_csr_rdata_all , 
output logic [32-1:0] pcu_csr_wdata , 
output logic pcu_csr_excpt , 
output logic pcu_csr_match_local , 
output logic ebreak_dbg_vld , 
output logic debug_mode_enter , 
output logic dcsr_stopcount , 
output logic dcsr_stoptime , 
output logic pcu_csr_mprv , 
output logic [1:0] pcu_csr_mpp , 
output logic pcu_csr_mstatus_be , 
input logic clic_pcu_nmi_vld , 
input logic clic_pcu_int_vld , 
input logic [$clog2(64 + 32)-1:0] clic_pcu_int_id , 
input logic [8-1:0] clic_pcu_int_lvl , 
input logic clic_pcu_int_shv , 
input logic [1:0] clic_pcu_int_priv , 
output logic pcu_clic_rsp_clr_ip , 
output logic [$clog2(64 + 32)-1:0] pcu_clic_rsp_id , 
output logic pcu_clic_rsp_vld , 
input logic dm_pcu_halt_req , 
input logic dm_pcu_halt_on_reset , 
input logic dm_pcu_resume_req , 
input logic dm_pcu_ack_havereset , 
output logic pcu_dm_halted , 
output logic pcu_dm_havereset , 
output logic pcu_dm_unavail , 
output logic pcu_dm_cmd_done , 
output logic pcu_dm_bus_err , 
output logic pcu_dm_cmd_excp , 
input logic dm_pcu_dsch0_write , 
input logic [32-1:0] dm_pcu_dsch0_wdata , 
output logic [32-1:0] pcu_dm_dsch0_rdata , 
output logic pcu_tm_trap_vld , 
output logic pcu_tm_trigger_hit , 
output logic [32-1:0] pcu_tm_trap_cause , 
output logic [2-1:0] pcu_tm_inst_retire , 
output logic pcu_tm_tcontrol_mte 
); 
 
localparam X0 = 5'b0 ; 
localparam X2 = 5'h2 ; 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
localparam FENCE_INST = 0; 
localparam AMO_INST = 1; 
localparam LD_INST = 2; 
localparam ST_INST = 3; 
localparam LS_POST = 5; 
localparam EXCLUSIVE_INST = 0; 
localparam SYS = 0 ; 
localparam SYS_E = 1 ; 
localparam CSR_INST = 2 ; 
localparam BRKPT = 12'h3 ; 
localparam ILL_INST = 12'h2 ; 
localparam LP_ERR = 12'h12 ; 
localparam ECALL_U = 12'h8 ; 
localparam ECALL_M = 12'hb ; 
localparam LD_RAS_ERR = 12'h1F ; 
localparam INST_MISALIGN = 12'h0 ; 
localparam IFU_PMP_ERR = 12'h1 ; 
localparam LD_ACC_FAULT = 12'h5 ; 
localparam LD_MISALIGN = 12'h4 ; 
localparam ST_ACC_FAULT = 12'h7 ; 
localparam ST_MISALIGN = 12'h6 ; 
localparam RAS_ERR = 12'h1f ; 
localparam CSRRW = 0 ; 
localparam CSRRS = 1 ; 
localparam CSRRC = 2 ; 
localparam CSRIMM = 3 ; 
localparam UNAVAIL = 2'h0; 
localparam HALTED = 2'h1; 
localparam RUNNING = 2'h2; 
localparam NMI_ID = 12'hFFF ; 
localparam MRET = 0 ; 
localparam WFI = 1 ; 
localparam MNRET = 2 ; 
localparam ECALL = 0 ; 
localparam EBREAK = 1 ; 
localparam MRASIE = 31 ; 
localparam MEIE = 11 ; 
localparam MTIE = 7 ; 
localparam MSIE = 3 ; 
localparam MRASIP = 31 ; 
localparam MEIP = 11 ; 
localparam MTIP = 7 ; 
localparam MSIP = 3 ; 
typedef enum logic [4:0]{ 
LOAD = 5'b00001 , 
STORE = 5'b00010 , 
FENCE = 5'b00100 
}sub_func_lsu_e ; 
typedef enum logic [4:0] { 
SUB_FU_NONE = 5'b00000 , 
ARITHMATIC = 5'b00001 , 
SHIFT = 5'b00010 , 
LOGICAL = 5'b00100 , 
MISC = 5'b01000 
} sub_func_alu_e; 
typedef enum logic [4:0] { 
UNCOND_DIRE = 5'b00001 , 
COND_DIRE = 5'b00010 , 
UNCOND_INDIRE = 5'b00100 
}sub_func_bru_e ; 
typedef enum logic [3:0] { 
OTHER_TYPES = 4'd0 , 
EXCP_TYPE = 4'd1 , 
INT_TYPE = 4'd2 , 
MRET_TYPE = 4'd3 , 
NT_BR_TYPE = 4'd4 , 
T_BR_TYPE = 4'd5 , 
UNINF_TYPE = 4'd6 
}itype; 
typedef struct packed{ 
logic interrupt; 
logic minhv; 
logic [1:0] mpp; 
logic mpie; 
logic [7:0] mpil; 
logic [11:0] exccode; 
} mcause_csr; 
typedef struct packed{ 
logic mprv; 
logic [1:0] mpp; 
logic mpie; 
logic mie; 
} mstatus_csr; 
typedef struct packed{ 
logic mnpelp; 
logic [1:0] mnpp; 
logic mnpv; 
logic nmie; 
}mnstatus_csr; 
typedef struct packed{ 
logic [3:0] debuger ; 
logic pelp ; 
logic ebreakvs ; 
logic ebreakvu ; 
logic ebreakm ; 
logic ebreaks ; 
logic ebreaku ; 
logic stepie ; 
logic stopcount; 
logic stoptime ; 
logic [2:0] cause ; 
logic v ; 
logic mprven ; 
logic nmip ; 
logic step ; 
logic [1:0] prv ; 
}dcsr_csr; 
typedef enum logic [3:0] { 
SPLIT_IDLE = 4'b0000 , 
SPLIT_UOP = 4'b0001 , 
SPLIT_LI = 4'b0010 , 
SPLIT_ADD = 4'b0110 , 
SPLIT_RET = 4'b1000 , 
SPLIT_SHF_ADD = 4'b1100 
} state_e; 
 
 
 
typedef enum logic[1:0] { 
PRIV_MODE_M = 2'b11, 
PRIV_MODE_S = 2'b01, 
PRIV_MODE_U = 2'b00 
} priv_mode_e; 
typedef enum logic[1:0] { 
CSR_OP_READ = 2'b00, 
CSR_OP_WRITE = 2'b01, 
CSR_OP_SET = 2'b10, 
CSR_OP_CLEAR = 2'b11 
} csr_opcode_e; 
typedef enum logic [11:0] { 
CSR_FFLAGS_ADDR = 12'h001, 
CSR_FRM_ADDR = 12'h002, 
CSR_FCSR_ADDR = 12'h003, 
CSR_FTRAN_ADDR = 12'h800, 
CSR_SSTATUS_ADDR = 12'h100, 
CSR_SIE_ADDR = 12'h104, 
CSR_STVEC_ADDR = 12'h105, 
CSR_SCOUNTEREN_ADDR = 12'h106, 
CSR_SSCRATCH_ADDR = 12'h140, 
CSR_SEPC_ADDR = 12'h141, 
CSR_SCAUSE_ADDR = 12'h142, 
CSR_STVAL_ADDR = 12'h143, 
CSR_SIP_ADDR = 12'h144, 
CSR_SATP_ADDR = 12'h180, 
CSR_MSTATUS_ADDR = 12'h300, 
CSR_MISA_ADDR = 12'h301, 
CSR_MEDELEG_ADDR = 12'h302, 
CSR_MIDELEG_ADDR = 12'h303, 
CSR_MIE_ADDR = 12'h304, 
CSR_MTVEC_ADDR = 12'h305, 
CSR_MCOUNTEREN_ADDR = 12'h306, 
CSR_MSTATUSH_ADDR = 12'h310, 
CSR_MSCRATCH_ADDR = 12'h340, 
CSR_MEPC_ADDR = 12'h341, 
CSR_MCAUSE_ADDR = 12'h342, 
CSR_MTVAL_ADDR = 12'h343, 
CSR_MIP_ADDR = 12'h344, 
CSR_MENVCFG_ADDR = 12'h30A, 
CSR_MENVCFGH_ADDR = 12'h31A, 
CSR_MSECCFG_ADDR = 12'h747, 
CSR_MSECCFGH_ADDR = 12'h757, 
CSR_PMPCFG0_ADDR = 12'h3A0, 
CSR_PMPCFG1_ADDR = 12'h3A1, 
CSR_PMPCFG2_ADDR = 12'h3A2, 
CSR_PMPCFG3_ADDR = 12'h3A3, 
CSR_PMPADDR0_ADDR = 12'h3B0, 
CSR_PMPADDR1_ADDR = 12'h3B1, 
CSR_PMPADDR2_ADDR = 12'h3B2, 
CSR_PMPADDR3_ADDR = 12'h3B3, 
CSR_PMPADDR4_ADDR = 12'h3B4, 
CSR_PMPADDR5_ADDR = 12'h3B5, 
CSR_PMPADDR6_ADDR = 12'h3B6, 
CSR_PMPADDR7_ADDR = 12'h3B7, 
CSR_PMPADDR8_ADDR = 12'h3B8, 
CSR_PMPADDR9_ADDR = 12'h3B9, 
CSR_PMPADDR10_ADDR = 12'h3BA, 
CSR_PMPADDR11_ADDR = 12'h3BB, 
CSR_PMPADDR12_ADDR = 12'h3BC, 
CSR_PMPADDR13_ADDR = 12'h3BD, 
CSR_PMPADDR14_ADDR = 12'h3BE, 
CSR_PMPADDR15_ADDR = 12'h3BF, 
CSR_MVENDORID_ADDR = 12'hF11, 
CSR_MARCHID_ADDR = 12'hF12, 
CSR_MIMPID_ADDR = 12'hF13, 
CSR_MHARTID_ADDR = 12'hF14, 
CSR_MCONFIGPTRID_ADDR = 12'hF15, 
CSR_MCYCLE_ADDR = 12'hB00, 
CSR_MCYCLEH_ADDR = 12'hB80, 
CSR_MINSTRET_ADDR = 12'hB02, 
CSR_MINSTRETH_ADDR = 12'hB82, 
CSR_MHPM_COUNTER_3_ADDR = 12'hB03, 
CSR_MHPM_COUNTER_4_ADDR = 12'hB04, 
CSR_MHPM_COUNTER_5_ADDR = 12'hB05, 
CSR_MHPM_COUNTER_6_ADDR = 12'hB06, 
CSR_MHPM_COUNTER_7_ADDR = 12'hB07, 
CSR_MHPM_COUNTER_8_ADDR = 12'hB08, 
CSR_MHPM_COUNTER_9_ADDR = 12'hB09, 
CSR_MHPM_COUNTER_10_ADDR = 12'hB0A, 
CSR_MHPM_COUNTER_11_ADDR = 12'hB0B, 
CSR_MHPM_COUNTER_12_ADDR = 12'hB0C, 
CSR_MHPM_COUNTER_13_ADDR = 12'hB0D, 
CSR_MHPM_COUNTER_14_ADDR = 12'hB0E, 
CSR_MHPM_COUNTER_15_ADDR = 12'hB0F, 
CSR_MHPM_COUNTER_16_ADDR = 12'hB10, 
CSR_MHPM_COUNTER_17_ADDR = 12'hB11, 
CSR_MHPM_COUNTER_18_ADDR = 12'hB12, 
CSR_MHPM_COUNTER_19_ADDR = 12'hB13, 
CSR_MHPM_COUNTER_20_ADDR = 12'hB14, 
CSR_MHPM_COUNTER_21_ADDR = 12'hB15, 
CSR_MHPM_COUNTER_22_ADDR = 12'hB16, 
CSR_MHPM_COUNTER_23_ADDR = 12'hB17, 
CSR_MHPM_COUNTER_24_ADDR = 12'hB18, 
CSR_MHPM_COUNTER_25_ADDR = 12'hB19, 
CSR_MHPM_COUNTER_26_ADDR = 12'hB1A, 
CSR_MHPM_COUNTER_27_ADDR = 12'hB1B, 
CSR_MHPM_COUNTER_28_ADDR = 12'hB1C, 
CSR_MHPM_COUNTER_29_ADDR = 12'hB1D, 
CSR_MHPM_COUNTER_30_ADDR = 12'hB1E, 
CSR_MHPM_COUNTER_31_ADDR = 12'hB1F, 
CSR_MHPM_COUNTER_3H_ADDR = 12'hB83, 
CSR_MHPM_COUNTER_4H_ADDR = 12'hB84, 
CSR_MHPM_COUNTER_5H_ADDR = 12'hB85, 
CSR_MHPM_COUNTER_6H_ADDR = 12'hB86, 
CSR_MHPM_COUNTER_7H_ADDR = 12'hB87, 
CSR_MHPM_COUNTER_8H_ADDR = 12'hB88, 
CSR_MHPM_COUNTER_9H_ADDR = 12'hB89, 
CSR_MHPM_COUNTER_10H_ADDR = 12'hB8A, 
CSR_MHPM_COUNTER_11H_ADDR = 12'hB8B, 
CSR_MHPM_COUNTER_12H_ADDR = 12'hB8C, 
CSR_MHPM_COUNTER_13H_ADDR = 12'hB8D, 
CSR_MHPM_COUNTER_14H_ADDR = 12'hB8E, 
CSR_MHPM_COUNTER_15H_ADDR = 12'hB8F, 
CSR_MHPM_COUNTER_16H_ADDR = 12'hB90, 
CSR_MHPM_COUNTER_17H_ADDR = 12'hB91, 
CSR_MHPM_COUNTER_18H_ADDR = 12'hB92, 
CSR_MHPM_COUNTER_19H_ADDR = 12'hB93, 
CSR_MHPM_COUNTER_20H_ADDR = 12'hB94, 
CSR_MHPM_COUNTER_21H_ADDR = 12'hB95, 
CSR_MHPM_COUNTER_22H_ADDR = 12'hB96, 
CSR_MHPM_COUNTER_23H_ADDR = 12'hB97, 
CSR_MHPM_COUNTER_24H_ADDR = 12'hB98, 
CSR_MHPM_COUNTER_25H_ADDR = 12'hB99, 
CSR_MHPM_COUNTER_26H_ADDR = 12'hB9A, 
CSR_MHPM_COUNTER_27H_ADDR = 12'hB9B, 
CSR_MHPM_COUNTER_28H_ADDR = 12'hB9C, 
CSR_MHPM_COUNTER_29H_ADDR = 12'hB9D, 
CSR_MHPM_COUNTER_30H_ADDR = 12'hB9E, 
CSR_MHPM_COUNTER_31H_ADDR = 12'hB9F, 
CSR_MCOUNTINHIBIT_ADDR = 12'h320, 
CSR_MHPM_EVENT_3_ADDR = 12'h323, 
CSR_MHPM_EVENT_4_ADDR = 12'h324, 
CSR_MHPM_EVENT_5_ADDR = 12'h325, 
CSR_MHPM_EVENT_6_ADDR = 12'h326, 
CSR_MHPM_EVENT_7_ADDR = 12'h327, 
CSR_MHPM_EVENT_8_ADDR = 12'h328, 
CSR_MHPM_EVENT_9_ADDR = 12'h329, 
CSR_MHPM_EVENT_10_ADDR = 12'h32a, 
CSR_MHPM_EVENT_11_ADDR = 12'h32b, 
CSR_MHPM_EVENT_12_ADDR = 12'h32c, 
CSR_MHPM_EVENT_13_ADDR = 12'h32d, 
CSR_MHPM_EVENT_14_ADDR = 12'h32e, 
CSR_MHPM_EVENT_15_ADDR = 12'h32f, 
CSR_MHPM_EVENT_16_ADDR = 12'h330, 
CSR_MHPM_EVENT_17_ADDR = 12'h331, 
CSR_MHPM_EVENT_18_ADDR = 12'h332, 
CSR_MHPM_EVENT_19_ADDR = 12'h333, 
CSR_MHPM_EVENT_20_ADDR = 12'h334, 
CSR_MHPM_EVENT_21_ADDR = 12'h335, 
CSR_MHPM_EVENT_22_ADDR = 12'h336, 
CSR_MHPM_EVENT_23_ADDR = 12'h337, 
CSR_MHPM_EVENT_24_ADDR = 12'h338, 
CSR_MHPM_EVENT_25_ADDR = 12'h339, 
CSR_MHPM_EVENT_26_ADDR = 12'h33a, 
CSR_MHPM_EVENT_27_ADDR = 12'h33b, 
CSR_MHPM_EVENT_28_ADDR = 12'h33c, 
CSR_MHPM_EVENT_29_ADDR = 12'h33d, 
CSR_MHPM_EVENT_30_ADDR = 12'h33e, 
CSR_MHPM_EVENT_31_ADDR = 12'h33f, 
CSR_MTVT_ADDR = 12'h307, 
CSR_MNXTI_ADDR = 12'h345, 
CSR_MINTSTATUS_ADDR = 12'hfb1, 
CSR_MINTTHRESH_ADDR = 12'h347, 
CSR_MSCRATCHCSW_ADDR = 12'h348, 
CSR_MSCRATCHCSWL_ADDR = 12'h349, 
CSR_MNSCRATCH_ADDR = 12'h740, 
CSR_MNEPC_ADDR = 12'h741, 
CSR_MNCAUSE_ADDR = 12'h742, 
CSR_MNSTATUS_ADDR = 12'h744, 
CSR_MNVEC_ADDR = 12'hFC0, 
CSR_TSELECT_ADDR = 12'h7A0, 
CSR_TDATA1_ADDR = 12'h7A1, 
CSR_TDATA2_ADDR = 12'h7A2, 
CSR_TDATA3_ADDR = 12'h7A3, 
CSR_TINFO_ADDR = 12'h7A4, 
CSR_TCONTROL_ADDR = 12'h7A5, 
CSR_DCSR_ADDR = 12'h7b0, 
CSR_DPC_ADDR = 12'h7b1, 
CSR_DSCRATCH0_ADDR = 12'h7b2, 
CSR_DSCRATCH1_ADDR = 12'h7b3, 
CSR_DSCRATCH2_ADDR = 12'h7c0, 
CSR_DSCRATCH3_ADDR = 12'h7c1, 
CSR_MCONTEXTSW_ADDR = 12'h7c2, 
CSR_MSECURFEAT_ADDR = 12'h7c3, 
CSR_MLPCFG_ADDR = 12'h7c4, 
CSR_CYCLE_ADDR = 12'hC00, 
CSR_CYCLEH_ADDR = 12'hC80, 
CSR_TIME_ADDR = 12'hC01, 
CSR_TIMEH_ADDR = 12'hC81, 
CSR_INSTRET_ADDR = 12'hC02, 
CSR_INSTRETH_ADDR = 12'hC82, 
CSR_HPM_COUNTER_3_ADDR = 12'hC03, 
CSR_HPM_COUNTER_4_ADDR = 12'hC04, 
CSR_HPM_COUNTER_5_ADDR = 12'hC05, 
CSR_HPM_COUNTER_6_ADDR = 12'hC06, 
CSR_HPM_COUNTER_7_ADDR = 12'hC07, 
CSR_HPM_COUNTER_8_ADDR = 12'hC08, 
CSR_HPM_COUNTER_9_ADDR = 12'hC09, 
CSR_HPM_COUNTER_10_ADDR = 12'hC0A, 
CSR_HPM_COUNTER_11_ADDR = 12'hC0B, 
CSR_HPM_COUNTER_12_ADDR = 12'hC0C, 
CSR_HPM_COUNTER_13_ADDR = 12'hC0D, 
CSR_HPM_COUNTER_14_ADDR = 12'hC0E, 
CSR_HPM_COUNTER_15_ADDR = 12'hC0F, 
CSR_HPM_COUNTER_16_ADDR = 12'hC10, 
CSR_HPM_COUNTER_17_ADDR = 12'hC11, 
CSR_HPM_COUNTER_18_ADDR = 12'hC12, 
CSR_HPM_COUNTER_19_ADDR = 12'hC13, 
CSR_HPM_COUNTER_20_ADDR = 12'hC14, 
CSR_HPM_COUNTER_21_ADDR = 12'hC15, 
CSR_HPM_COUNTER_22_ADDR = 12'hC16, 
CSR_HPM_COUNTER_23_ADDR = 12'hC17, 
CSR_HPM_COUNTER_24_ADDR = 12'hC18, 
CSR_HPM_COUNTER_25_ADDR = 12'hC19, 
CSR_HPM_COUNTER_26_ADDR = 12'hC1A, 
CSR_HPM_COUNTER_27_ADDR = 12'hC1B, 
CSR_HPM_COUNTER_28_ADDR = 12'hC1C, 
CSR_HPM_COUNTER_29_ADDR = 12'hC1D, 
CSR_HPM_COUNTER_30_ADDR = 12'hC1E, 
CSR_HPM_COUNTER_31_ADDR = 12'hC1F, 
CSR_HPM_COUNTER_3H_ADDR = 12'hC83, 
CSR_HPM_COUNTER_4H_ADDR = 12'hC84, 
CSR_HPM_COUNTER_5H_ADDR = 12'hC85, 
CSR_HPM_COUNTER_6H_ADDR = 12'hC86, 
CSR_HPM_COUNTER_7H_ADDR = 12'hC87, 
CSR_HPM_COUNTER_8H_ADDR = 12'hC88, 
CSR_HPM_COUNTER_9H_ADDR = 12'hC89, 
CSR_HPM_COUNTER_10H_ADDR = 12'hC8A, 
CSR_HPM_COUNTER_11H_ADDR = 12'hC8B, 
CSR_HPM_COUNTER_12H_ADDR = 12'hC8C, 
CSR_HPM_COUNTER_13H_ADDR = 12'hC8D, 
CSR_HPM_COUNTER_14H_ADDR = 12'hC8E, 
CSR_HPM_COUNTER_15H_ADDR = 12'hC8F, 
CSR_HPM_COUNTER_16H_ADDR = 12'hC90, 
CSR_HPM_COUNTER_17H_ADDR = 12'hC91, 
CSR_HPM_COUNTER_18H_ADDR = 12'hC92, 
CSR_HPM_COUNTER_19H_ADDR = 12'hC93, 
CSR_HPM_COUNTER_20H_ADDR = 12'hC94, 
CSR_HPM_COUNTER_21H_ADDR = 12'hC95, 
CSR_HPM_COUNTER_22H_ADDR = 12'hC96, 
CSR_HPM_COUNTER_23H_ADDR = 12'hC97, 
CSR_HPM_COUNTER_24H_ADDR = 12'hC98, 
CSR_HPM_COUNTER_25H_ADDR = 12'hC99, 
CSR_HPM_COUNTER_26H_ADDR = 12'hC9A, 
CSR_HPM_COUNTER_27H_ADDR = 12'hC9B, 
CSR_HPM_COUNTER_28H_ADDR = 12'hC9C, 
CSR_HPM_COUNTER_29H_ADDR = 12'hC9D, 
CSR_HPM_COUNTER_30H_ADDR = 12'hC9E, 
CSR_HPM_COUNTER_31H_ADDR = 12'hC9F, 
CSR_JVT_ADDR = 12'h017 
} csr_reg_t; 
 
logic iss0_cmt_vld ; 
logic iss1_cmt_vld ; 
logic [32-1:0] pc_reg_din ; 
logic [32-1:0] pc_reg_dout ; 
logic pc_reg_en ; 
logic spu_done ; 
logic [1:0] priv_mode_d ; 
logic priv_mode_updt_en ; 
mcause_csr mcause_d, mcause ; 
logic [32-1:0] mcause_val ; 
logic mcause_updt_en ; 
logic mcause_int_updt_en ; 
logic mcause_mpil_updt_en ; 
logic mcause_exccode_updt_en ; 
logic mcause_csrinst_updt_en ; 
logic mcause_minhv_updt_en ; 
mstatus_csr mstatus_d,mstatus ; 
logic [32-1:0] mstatus_val ; 
logic mstatus_updt_en ; 
logic mstatus_mprv_updt_en ; 
logic mstatus_mpp_updt_en ; 
logic mstatus_mpie_updt_en ; 
logic mstatus_mie_updt_en ; 
logic mnxti_updt_mie ; 
logic [32-1:0] mip ; 
logic [32-1:0] mie ; 
logic [32-1:0] mie_d ; 
logic mie_updt_en ; 
logic [32-1:0] mtvec_d ; 
logic mtvec_updt_en ; 
logic [32-1:0] mtvec_pre ; 
logic [32-1:0] mtvec ; 
logic [32-1:0] mstatush ; 
logic [32-1:0] mscratch_d ; 
logic [32-1:0] mscratch ; 
logic [32-1:0] mepc_d ; 
logic [32-1:0] mepc ; 
logic mepc_updt_en ; 
logic mtval_updt_en ; 
logic [32-1:0] mtval_d ; 
logic [32-1:0] mtval ; 
logic [32-1:0] mtinst ; 
logic [32-1:0] mtval2 ; 
logic [32-1:0] menvcfg ; 
logic [32-1:0] menvcfgh ; 
logic [32-1:0] mseccfg ; 
logic [32-1:0] mseccfgh ; 
logic [32-1:0] mnscratch_d ; 
logic mnscratch_updt_en ; 
logic [32-1:0] mnscratch ; 
logic [32-1:0] mnepc_d ; 
logic mnepc_updt_en ; 
logic [32-1:0] mnepc ; 
logic [32-1:0] mncause_val ; 
mnstatus_csr mnstatus_d,mnstatus ; 
logic mnstatus_updt_en ; 
logic [32-1:0] mnstatus_val ; 
logic [32-1:0] dcsr_val ; 
logic [32-1:0] dpc_d ; 
logic dpc_updt_en ; 
logic [32-1:0] dpc_pre ; 
logic [32-1:0] dpc ; 
logic [32-1:0] dscratch0 ; 
logic dscratch0_updt_en ; 
logic [32-1:0] dscratch0_d ; 
logic dscratch2_updt_en ; 
logic dscratch3_updt_en ; 
logic [32-1:0] dscratch2 ; 
logic [32-1:0] dscratch3 ; 
dcsr_csr dcsr,dcsr_d,dcsr_rst_val ; 
logic [32-1:0] tcontrol ; 
logic [32-1:0] tcontrol_d ; 
logic tcontrol_updt_en ; 
logic [32-1:0] mtvt ; 
logic [32-1:0] mtvt_d ; 
logic mtvt_updt_en ; 
logic [32-1:0] mnxti ; 
logic [32-1:0] mintstatus_d ; 
logic [32-1:0] mintstatus ; 
logic mintstatus_updt_en ; 
logic [32-1:0] mintthresh_d ; 
logic [32-1:0] mintthresh_pre ; 
logic [32-1:0] mintthresh ; 
logic mintthresh_updt_en ; 
logic [32-1:0] wr_gpr_par_data ; 
logic excp_vld_wi_dbg ; 
logic excp_vld ; 
logic excp ; 
logic dbg_flush ; 
logic step_flush_pre ; 
logic int_tkn_vld ; 
logic int_tkn_wi_nmi ; 
logic int_tkn_cur_vld ; 
logic [11:0] excp_cause ; 
logic [11:0] int_cause ; 
logic ecall_inst ; 
logic ebreak_inst ; 
logic mret_inst ; 
logic mnret_inst ; 
logic wfi_inst_wb ; 
logic wfi_inst ; 
logic ebreak_2_excp ; 
logic ebreak_2_dbg ; 
logic int_vld ; 
logic nmi_vld ; 
logic nmi_pending_vld_in_dm ; 
logic debug_mode ; 
logic [1:0] core_status_d ; 
logic [1:0] core_status ; 
logic core_init_busy_updt_en ; 
logic core_init_busy_d ; 
logic dbg_flush_wo_step ; 
logic debug_mode_updt_en ; 
logic already_reset ; 
logic already_reset_en ; 
logic already_reset_d ; 
logic step_flush ; 
logic trigger_dbg ; 
logic excp_int_dbg_flush ; 
logic sleep_wakeup ; 
logic sleep_mode ; 
logic sleep_mode_d ; 
logic sleep_mode_updt_en ; 
logic [32-1:0] csr_wdata ; 
logic csr_only_m_mode ; 
logic csr_only_d_mode ; 
logic csr_only_r ; 
logic jvt_wb ; 
logic jvt_table_fetch_done ; 
logic fencei_inst ; 
logic mnxti_csr_nop_inst ; 
logic mnxti_csr_rw_inst ; 
logic mnxti_csr_clic_nest_en ; 
logic mscratchcsw_csr_nop_inst ; 
logic [32-1:0] int_flush_pc ; 
logic core_status_en ; 
logic mscratch_updt_en ; 
logic dcsr_updt_en ; 
logic inst0_cmt_vld_pre ; 
logic csr_flush_q ; 
logic [2-1:0] nxt_pc_fr_cmt_nxt ; 
logic [32-1:0] csr_rdata_all ; 
logic cmt_csr_vld ; 
logic cmt_csr_wr_vld ; 
logic cmt_clint_csr_wr_vld ; 
logic cmt_clic_csr_wr_vld ; 
logic cmt_tm_csr_wr_vld ; 
logic cmt_oths_csr_wr_vld ; 
logic [1:0] core_priv_mode_reg ; 
logic jvt_table_excp_vld ; 
logic jvt_excp_vld ; 
logic jvt_cmt_done ; 
logic int_shv_excp_vld ; 
logic nmi_resped ; 
logic nmi_mask ; 
logic int_mask ; 
logic excp_vld_wi_nmi ; 
logic step_2_dbg_en ; 
logic step_2_dbg_d ; 
logic step_2_dbg ; 
logic [30:0] nmicause ; 
logic mncause_updt_en ; 
logic [31:0] csr_rdata_local ; 
logic csr_match_local ; 
logic br_flush_at_wb ; 
logic cmt_csr_no_write ; 
logic [2:0] random_exe_cycle ; 
logic pcu_csr_ulpe_en ; 
logic pcu_csr_mlpe_en ; 
logic ls_addr_write_back_vld_iss0; 
logic ls_addr_write_back_vld_iss1; 
logic ls_exe_2_cycs_q ; 
logic dcsr_step ; 
logic shv_done_bef_mask_int_debug; 
logic wb_inst_excp_vld ; 
logic [1:0] pcu_ifu_shv_flush_d ; 
assign wb_inst_excp_vld = cmt_excp_vld | cmt_trig_exp & !dbg_disable & !debug_mode; 
assign ecall_inst = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][SYS_E] & cmt_inst_opcode[0][ECALL] & !debug_mode & !dbg_flush_wo_step; 
assign ebreak_inst = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][SYS_E] & cmt_inst_opcode[0][EBREAK] & !debug_mode & !dbg_disable & !wb_inst_excp_vld; 
assign mret_inst = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][SYS] & cmt_inst_opcode[0][MRET] & !debug_mode & !dbg_flush_wo_step& !wb_inst_excp_vld; 
assign wfi_inst_wb = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][SYS] & cmt_inst_opcode[0][WFI]; 
assign wfi_inst = wfi_inst_wb & !debug_mode & !dcsr_step & !(int_tkn_vld | dbg_flush_wo_step | wb_inst_excp_vld); 
assign mnret_inst = 1'b0; 
assign jvt_wb = cmt_inst_vld[0] & cmt_inst_funct[0][FU_BRU_BIT] & cmt_inst_sub_funct[0][3] & !debug_mode; 
assign fencei_inst = cmt_inst_vld[0] & cmt_inst_funct[0][FU_LSU_BIT] & cmt_inst_sub_funct[0][FENCE_INST] & cmt_inst_opcode[0][3] & !debug_mode & !excp_int_dbg_flush; 
assign mnxti_csr_nop_inst = cmt_csr_vld & (cmt_inst_opcode[0][CSRRW] | 
cmt_inst_opcode[0][CSRRS] & !cmt_inst_opcode[0][CSRIMM] & !cmt_inst_opcode[0][4] | 
cmt_inst_opcode[0][CSRRC] & !cmt_inst_opcode[0][CSRIMM] | 
debug_mode) & 
(pcu_csr_index == CSR_MNXTI_ADDR); 
assign mnxti_csr_rw_inst = cmt_csr_vld & (cmt_inst_opcode[0][CSRRS] & cmt_inst_opcode[0][CSRIMM] & !cmt_inst_opcode[0][4] | 
cmt_inst_opcode[0][CSRRC] & cmt_inst_opcode[0][CSRIMM] & !cmt_inst_opcode[0][4] ) & 
(pcu_csr_index == CSR_MNXTI_ADDR) & !debug_mode; 
assign mscratchcsw_csr_nop_inst = cmt_csr_vld & !(cmt_inst_opcode[0][CSRRW] & !cmt_inst_opcode[0][CSRIMM] & !cmt_inst_opcode[0][4] & |cmt_dest[0]) & 
((pcu_csr_index == CSR_MSCRATCHCSW_ADDR) | (pcu_csr_index == CSR_MSCRATCHCSWL_ADDR)); 
assign mnxti_csr_clic_nest_en = clic_pcu_int_vld & (clic_pcu_int_priv == PRIV_MODE_M) & (clic_pcu_int_lvl > mcause.mpil) & (clic_pcu_int_lvl > mintthresh[7:0]) & !clic_pcu_int_shv & !nmi_resped; 
assign ebreak_2_excp = ebreak_inst & (!dcsr.ebreakm & (core_priv_mode_reg == PRIV_MODE_M) | !dcsr.ebreaku & (core_priv_mode_reg == PRIV_MODE_U)); 
assign ebreak_2_dbg = ebreak_inst & ( dcsr.ebreakm & (core_priv_mode_reg == PRIV_MODE_M) | dcsr.ebreaku & (core_priv_mode_reg == PRIV_MODE_U)); 
assign ebreak_dbg_vld = ebreak_2_dbg; 
assign jvt_cmt_done = jvt_wb & (jvt_table_fetch_done | ifu_pcu_hv_done) & !random_exe_stall; 
assign jvt_table_excp_vld = jvt_wb & ifu_pcu_hv_done & ifu_pcu_hv_excp_vld; 
assign jvt_excp_vld = jvt_table_excp_vld; 
assign excp = wb_inst_excp_vld | 
lsu_pcu_cmt_excp[0] & lsu_pcu_cmt | 
ecall_inst | 
ebreak_2_excp | 
jvt_excp_vld ; 
assign excp_vld = (excp & cmt_inst_vld[0] & !debug_mode | int_shv_excp_vld) & !dbg_flush_wo_step; 
assign excp_vld_wi_nmi = excp_vld & !nmi_vld; 
assign excp_cause = cmt_inst_vld[0] & cmt_trig_exp & !dbg_disable ? BRKPT : 
cmt_inst_vld[0] & cmt_excp_vld ? cmt_excp_cause : 
(jvt_wb | int_shv_excp_vld) ? ifu_pcu_excp_cause[11:0] : 
ecall_inst ? ((core_priv_mode_reg == PRIV_MODE_M)? ECALL_M : ECALL_U) : 
ebreak_2_excp ? BRKPT : lsu_pcu_cmt_excp_cause[0][11:0]; 
assign pcu_dm_cmd_excp = excp_vld_wi_dbg; 
assign pcu_dm_cmd_done = debug_mode & (iss0_cmt_vld & cmt_inst_en | cmt_inst_vld[0] & pcu_dm_cmd_excp); 
assign pcu_dm_bus_err = (lsu_pcu_cmt_excp_cause[0][11:0] == LD_RAS_ERR) & lsu_pcu_cmt_excp[0] & lsu_pcu_cmt; 
assign priv_mode_d = (excp_vld | int_tkn_vld) ? PRIV_MODE_M : 
mret_inst ? mstatus.mpp : dcsr.prv; 
assign priv_mode_updt_en = excp_vld | int_tkn_vld | mret_inst | mnret_inst | dm_pcu_resume_req & debug_mode; 
`WDFFERVAL(core_priv_mode_reg, dbg_flush? PRIV_MODE_M : priv_mode_d, priv_mode_updt_en|dbg_flush, clk, rst_n,PRIV_MODE_M) 
assign priv_mode_parity_err = 1'b0; 
assign core_priv_mode = priv_mode_updt_en ? priv_mode_d : core_priv_mode_reg; 
assign core_dbg_mode = debug_mode; 
assign core_init_busy_d = mss_pcu_icache_init_busy; 
assign core_init_busy_updt_en = 1'b1; 
`WDFFERVAL(core_init_busy, core_init_busy_d, core_init_busy_updt_en, clk, rst_n,1'b1) 
logic inst0_cmt_vld; 
assign inst0_cmt_vld_pre = cmt_inst_funct[0][FU_ALU_BIT] & !cmt_inst_funct[0][FU_SPU_BIT] & !cmt_inst_funct[0][FU_LSU_BIT]| 
cmt_inst_funct[0][FU_BMU_BIT] | 
cmt_inst_funct[0][FU_BRU_BIT] & !jvt_wb | jvt_cmt_done | 
cmt_inst_funct[0][FU_MDU_BIT] & !exu_pcu_inst_wb_stall[0] | 
cmt_inst_funct[0][FU_LSU_BIT] & lsu_pcu_cmt | 
cmt_inst_funct[0][FU_SPU_BIT] & !cmt_inst_funct[0][FU_LSU_BIT] & spu_done ; 
assign inst0_cmt_vld = inst0_cmt_vld_pre & (random_exe_cycle == 3'b0); 
assign ls_exe_cmt_data_vld = inst0_cmt_vld; 
assign iss0_cmt_vld = cmt_inst_vld[0] & !(excp | dbg_flush_wo_step | int_tkn_cur_vld) & inst0_cmt_vld; 
assign random_exe_cycle = 3'b0; 
assign core_pseudo_run_state = 1'b0; 
assign core_elp_state_vld = 1'b0; 
assign random_exe_stall = 1'b0; 
assign iss1_cmt_vld = cmt_inst_vld[1] & iss0_cmt_vld & !cmt_uop_inst; 
assign cmt_inst_retire = {iss1_cmt_vld,iss0_cmt_vld} & {2{cmt_inst_en}}; 
assign cmt_inst_split_retire = {cmt_inst_vld[1] & cmt_inst_vld[0] & (inst0_cmt_vld | excp_int_dbg_flush),cmt_inst_vld[0] & (inst0_cmt_vld | excp_int_dbg_flush)}; 
logic ls_addr_write_data_vld; 
assign ls_addr_write_data_vld = ls_addr_write_back_vld_iss0 | cmt_ls_addr_write_back[0] & cmt_inst_sub_funct[0][ST_INST]; 
assign wr_gpr_par_data = ({32{( 
cmt_inst_funct[0][FU_ALU_BIT] & (!cmt_inst_funct[0][FU_LSU_BIT] | cmt_inst_funct[0][FU_LSU_BIT] & ls_addr_write_data_vld)| 
cmt_inst_funct[0][FU_BMU_BIT] | 
cmt_inst_funct[0][FU_BRU_BIT] )}} & exu_pcu_inst_alu_data[0]) | 
({32 {cmt_inst_funct[0][FU_LSU_BIT] & cmt_inst_sub_funct[0][LD_INST] & !ls_exe_2_cycs_q}} & lsu_pcu_cmt_data[0]) ; 
assign mrf_gpr_wr_vld[0] = cmt_inst_vld[0] & (inst0_cmt_vld & cmt_dest_vld[0] & 
!(excp | dbg_flush_wo_step | int_tkn_cur_vld | mnxti_csr_nop_inst | mscratchcsw_csr_nop_inst) | ls_addr_write_back_vld_iss0); 
assign mrf_gpr_wr_idx[0] = ls_addr_write_back_vld_iss0 ? ls_post_pre_addr_dst_idx_wb : cmt_dest[0]; 
assign mrf_gpr_wr_data[0] = cmt_inst_funct[0][FU_SPU_BIT] ? csr_rdata_all : 
cmt_inst_funct[0][FU_MDU_BIT] ? exu_pcu_inst_mdu_data[0] : 
wr_gpr_par_data; 
assign mrf_gpr_dst_vld[0] = (cmt_dest_vld[0] | ls_addr_write_back_vld_iss0) & (mrf_gpr_wr_idx[0] != X0); 
assign mrf_gpr_wr_vld[1] = cmt_inst_vld[1] & inst0_cmt_vld & (cmt_dest_vld[1] | cmt_inst_vld[0] & ls_addr_write_back_vld_iss1)& !(excp | dbg_flush_wo_step | int_tkn_cur_vld); 
assign mrf_gpr_wr_idx[1] = cmt_dest[1]; 
assign mrf_gpr_wr_data[1] = ls_addr_write_back_vld_iss1 ? exu_pcu_inst_alu_data[0] : exu_pcu_inst_alu_data[1]; 
assign mrf_gpr_dst_vld[1] = cmt_dest_vld[1] & (mrf_gpr_wr_idx[1] != X0); 
assign core_locked = 1'b0; 
assign mrf_gpr_need_cleard = 1'b0; 
logic shv_done_bef_d; 
assign shv_done_bef_d = (int_vld & clic_pcu_int_shv | mcause.minhv & mret_inst) & cmt_flush & (!ifu_pcu_hv_done & !jvt_wb | jvt_wb); 
`WDFFER(shv_done_bef_mask_int_debug ,shv_done_bef_d ,cmt_flush|ifu_pcu_hv_done , clk, rst_n) 
assign pc_reg_din = cmt_inst_retire[0] ? cmt_inst_retire[1] ? cmt_pc[1] : cmt_pc[0] : pc_reg_dout; 
assign pc_reg_en = |cmt_inst_retire & !core_dbg_mode; 
`WDFFER(pc_reg_dout, pc_reg_din, pc_reg_en, clk, rst_n) 
logic [31:0] retire_pc; 
assign retire_pc = pc_reg_dout; 
assign excp_vld_wi_dbg = excp & cmt_inst_vld[0] | int_shv_excp_vld; 
assign flush_wb = excp_vld_wi_dbg | int_tkn_vld | csr_flush_q; 
assign flush_wb_wo_dbg = (excp_vld | int_tkn_vld | csr_flush_q) & !pcu_ifu_shv_flush_d[1]; 
assign cmt_flush = flush_wb | 
dbg_flush | 
step_flush_pre | 
debug_mode & dm_pcu_resume_req | 
mret_inst | 
mnret_inst | 
wfi_inst | 
fencei_inst & lsu_pcu_cmt | 
br_flush_at_wb ; 
assign cmt_flush_wo_dbg = debug_mode & dm_pcu_resume_req | 
mret_inst | 
mnret_inst | 
wfi_inst | 
fencei_inst & lsu_pcu_cmt | 
br_flush_at_wb | 
step_flush_pre; 
assign cmt_flush_pc = 
debug_mode & dm_pcu_resume_req ? {dpc[32-1:1],1'b0}: 
excp_vld ? {mtvec[32-1:2],2'b0} : 
int_vld ? int_flush_pc : 
mret_inst ? {mepc[32-1:2],!mcause.minhv & mepc[1],1'b0} : 
cmt_pc[1] ; 
assign excp_int_dbg_flush = (wb_inst_excp_vld) & cmt_inst_vld[0] | int_tkn_vld | dbg_flush_wo_step; 
assign ls_addr_write_back_vld_iss0 = 1'b0; 
assign ls_addr_write_back_vld_iss1 = cmt_ls_addr_write_back[0] & cmt_inst_sub_funct[0][LD_INST]; 
assign ls_exe_2_cycs_q = 1'b0; 
assign cmt_stall = cmt_inst_vld[0] & ~inst0_cmt_vld & ~excp_int_dbg_flush; 
logic pcu_ifu_stall_d; 
assign pcu_ifu_stall_d = dbg_flush | 
debug_mode & ~dm_pcu_resume_req | 
wfi_inst | 
(core_status == UNAVAIL); 
assign pcu_ifu_shv_flush_d = {int_vld & clic_pcu_int_shv | mcause.minhv & mret_inst,int_vld & clic_pcu_int_shv | mcause.minhv & mret_inst | jvt_inst_ex}; 
assign pcu_ifu_flush_vld = cmt_flush | exu_pcu_bru_flush & !core_pseudo_run_state; 
assign pcu_ifu_stall = pcu_ifu_stall_d; 
assign pcu_ifu_flush_pc = cmt_flush ? cmt_flush_pc : exu_pcu_bru_flush_pc; 
assign pcu_ifu_shv_flush = pcu_ifu_shv_flush_d; 
assign br_flush_at_wb = 1'b0; 
assign jvt_table_fetch_done = 1'b0; 
assign int_shv_excp_vld = !jvt_wb & ifu_pcu_hv_done & ifu_pcu_hv_excp_vld; 
assign pcu_lsu_non_spec = 1'b1; 
assign pcu_lsu_flush_st_inst_wb = excp_vld | int_tkn_vld & int_tkn_cur_vld | dbg_flush ; 
assign pcu_lsu_flush = cmt_flush ; 
assign cmt_csr_no_write = (cmt_inst_opcode[0][CSRRS] | cmt_inst_opcode[0][CSRRC]) & cmt_inst_opcode[0][4]; 
assign cmt_csr_vld = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][CSR_INST] & !(excp_int_dbg_flush|lsu_pcu_cmt_excp[0] & lsu_pcu_cmt) & (!cmt_inst_funct[0][FU_LSU_BIT] | cmt_inst_funct[0][FU_LSU_BIT] & cmt_inst_sub_funct[0][LD_INST] & lsu_pcu_cmt); 
assign cmt_oths_csr_wr_vld = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][CSR_INST] & !(excp_int_dbg_flush|lsu_pcu_cmt_excp[0] & lsu_pcu_cmt) & (!cmt_inst_funct[0][FU_LSU_BIT] & !top_sr_lock[0] | cmt_inst_funct[0][FU_LSU_BIT] & cmt_inst_sub_funct[0][LD_INST] & lsu_pcu_cmt) & !cmt_csr_no_write; 
assign cmt_csr_wr_vld = cmt_csr_vld & !cmt_csr_no_write; 
assign cmt_clint_csr_wr_vld = cmt_csr_wr_vld & !top_sr_lock[5]; 
assign cmt_clic_csr_wr_vld = cmt_csr_wr_vld & !top_sr_lock[6]; 
assign cmt_tm_csr_wr_vld = cmt_csr_wr_vld & !top_sr_lock[3]; 
assign spu_done = !wfi_inst | wfi_inst & sleep_wakeup; 
assign sleep_mode_d = wfi_inst & !sleep_wakeup; 
assign sleep_mode_updt_en = wfi_inst & (ifu_idle & lsu_idle & mss_idle & bmu_idle) & !sleep_mode | sleep_wakeup; 
`WDFFER(sleep_mode, sleep_mode_d,sleep_mode_updt_en, always_on_clk, rst_n) 
assign sleep_wakeup = (((clic_pcu_int_priv == PRIV_MODE_M) & (core_priv_mode_reg == PRIV_MODE_U) | 
(clic_pcu_int_priv == PRIV_MODE_M) & (core_priv_mode_reg == PRIV_MODE_M) & 
(clic_pcu_int_lvl > mintstatus[31:24]) & (clic_pcu_int_lvl > mintthresh[7:0])) & clic_pcu_int_vld | 
dm_pcu_halt_req | 
clic_pcu_nmi_vld) ; 
assign core_sleep_mode = sleep_mode; 
assign core_sleep_wakeup = sleep_wakeup; 
assign nmi_pending_vld_in_dm = 1'b0; 
assign nmi_vld = 1'b0; 
assign nmi_resped = 1'b0; 
assign int_mask = excp_vld | debug_mode | cmt_uop_int_mask | shv_done_bef_mask_int_debug | mret_inst | mnret_inst | lsu_pcu_non_flush_infly | nmi_resped | dbg_flush_wo_step | ls_exe_2_cycs_q | 
pcu_csr_split_integrity & exu_pcu_inst_wb_stall[0]; 
assign int_vld = ((clic_pcu_int_priv == PRIV_MODE_M) & (core_priv_mode_reg == PRIV_MODE_U) | 
(clic_pcu_int_priv == PRIV_MODE_M) & (core_priv_mode_reg == PRIV_MODE_M) & mstatus.mie & 
(clic_pcu_int_lvl > mintstatus[31:24]) & (clic_pcu_int_lvl > mintthresh[7:0]) 
) & 
clic_pcu_int_vld & 
!clic_pcu_nmi_vld & 
(dcsr.stepie & dcsr_step | !dcsr_step) & 
!int_mask ; 
assign int_cause = {{(12-$clog2(64 + 32)){1'b0}},clic_pcu_int_vld ? clic_pcu_int_id : {$clog2(64 + 32){1'b0}}}; 
assign int_flush_pc = clic_pcu_int_shv ? ({mtvt[32-1:6],6'b0} + {int_cause,2'b0}) : {mtvec[32-1:6],6'b0} ; 
assign int_tkn_vld = int_vld | nmi_vld; 
assign int_tkn_cur_vld = int_tkn_vld & (cmt_inst_vld[0] & (cmt_inst_funct[0][FU_LSU_BIT] & !lsu_pcu_cmt | 
cmt_inst_funct[0][FU_MDU_BIT] & exu_pcu_inst_wb_stall[0] | 
cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][CSR_INST] | 
excp_vld | 
cmt_exp_lp_inst[0] | 
cmt_lp_inst[0] | 
!cmt_inst_en | 
random_exe_stall | 
jvt_wb ) | 
!(cmt_inst_vld[0] | nxt_pc_fr_cmt_nxt[0]) ); 
logic dbg_halt_req_wo_reset; 
logic dbg_mask; 
assign trigger_dbg = (cmt_trig_dbg | lsu_pcu_cmt_trig_dbg & !wb_inst_excp_vld); 
assign dbg_mask = (wfi_inst_wb) & cmt_inst_vld[0] | lsu_pcu_non_flush_infly | ls_exe_2_cycs_q | cmt_uop_int_mask | shv_done_bef_mask_int_debug; 
assign dbg_halt_req_wo_reset = ((dm_pcu_halt_req & (core_status != UNAVAIL))& !dbg_mask | (ebreak_2_dbg | trigger_dbg) & cmt_inst_vld[0]) & !debug_mode; 
assign dbg_flush_wo_step = (dm_pcu_halt_on_reset & (core_status == UNAVAIL) & !core_init_busy & !areg_init | dbg_halt_req_wo_reset) & !dbg_disable; 
assign dbg_flush = (dbg_flush_wo_step | step_flush); 
assign core_status_d = dbg_flush ? HALTED : RUNNING; 
assign core_status_en = dbg_flush | (core_status == UNAVAIL) & !core_init_busy | debug_mode & dm_pcu_resume_req; 
`WDFFERVAL(core_status, core_status_d, core_status_en, clk, rst_n, UNAVAIL) 
assign already_reset_d = (core_status == UNAVAIL) & !core_init_busy | already_reset & !dm_pcu_ack_havereset; 
assign already_reset_en = (core_status == UNAVAIL) & !core_init_busy | already_reset & dm_pcu_ack_havereset; 
`WDFFER(already_reset,already_reset_d,already_reset_en,always_on_clk,rst_n) 
assign pcu_dm_havereset = already_reset; 
assign pcu_dm_unavail = core_status == UNAVAIL; 
assign pcu_dm_halted = core_status == HALTED; 
assign core_step_vld = dcsr.step; 
assign step_2_dbg_en = dcsr_step & (excp_vld | int_tkn_vld | step_flush_pre | jvt_wb) | dbg_flush; 
assign step_2_dbg_d = dbg_flush ? 1'b0 : 1'b1; 
assign step_flush_pre = dcsr_step & cmt_inst_retire[0] & !debug_mode & !jvt_wb; 
assign step_flush = step_2_dbg & ifu_pcu_vld_ex[0] & !int_tkn_vld; 
assign debug_mode = core_status == HALTED; 
`WDFFER(step_2_dbg,step_2_dbg_d,step_2_dbg_en,clk,rst_n) 
assign debug_mode_enter = dbg_flush; 
assign csr_rdata_all = {32{csr_match_local }} & csr_rdata_local | 
{32{csr_oth_match }} & csr_oth_rdata ; 
assign csr_wdata = {32{cmt_inst_opcode[0][CSRRW]}} & wr_gpr_par_data | 
{32{cmt_inst_opcode[0][CSRRS]}} & (csr_rdata_all | wr_gpr_par_data) | 
{32{cmt_inst_opcode[0][CSRRC]}} & (csr_rdata_all & ~wr_gpr_par_data) ; 
always@* begin 
csr_match_local = 1'b1; 
csr_only_m_mode = 1'b1; 
csr_only_d_mode = 1'b0; 
csr_only_r = 1'b0; 
casez(pcu_csr_index) 
CSR_MSTATUS_ADDR : csr_rdata_local = mstatus_val; 
CSR_MIE_ADDR : csr_rdata_local = mie; 
CSR_MTVEC_ADDR : csr_rdata_local = mtvec; 
CSR_MSTATUSH_ADDR : csr_rdata_local = mstatush; 
CSR_MEPC_ADDR : csr_rdata_local = mepc; 
CSR_MCAUSE_ADDR : csr_rdata_local = mcause_val; 
CSR_MTVAL_ADDR : csr_rdata_local = mtval; 
CSR_MIP_ADDR : csr_rdata_local = mip; 
CSR_MSCRATCH_ADDR : csr_rdata_local = mscratch; 
CSR_MTVT_ADDR : csr_rdata_local = mtvt; 
CSR_MNXTI_ADDR : csr_rdata_local = mnxti; 
CSR_MINTSTATUS_ADDR : begin 
csr_rdata_local = mintstatus; 
csr_only_r = 1'b1; 
end 
CSR_MINTTHRESH_ADDR : csr_rdata_local = mintthresh; 
CSR_MSCRATCHCSW_ADDR : csr_rdata_local = (mstatus.mpp == PRIV_MODE_M)? exu_pcu_inst_alu_data[0] : mscratch; 
CSR_MSCRATCHCSWL_ADDR : csr_rdata_local = ((mcause.mpil == 0) != (mintstatus[31:24] == 0)) ? mscratch : exu_pcu_inst_alu_data[0]; 
CSR_DCSR_ADDR : begin 
csr_rdata_local = dcsr_val; 
csr_only_d_mode = 1'b1; 
end 
CSR_DPC_ADDR : begin 
csr_rdata_local = dpc; 
csr_only_d_mode = 1'b1; 
end 
CSR_DSCRATCH0_ADDR : begin 
csr_rdata_local = dscratch0; 
csr_only_d_mode = 1'b1; 
end 
CSR_DSCRATCH2_ADDR : begin 
csr_rdata_local = dscratch2; 
csr_only_d_mode = 1'b1; 
end 
CSR_DSCRATCH3_ADDR : begin 
csr_rdata_local = dscratch3; 
csr_only_d_mode = 1'b1; 
end 
CSR_TCONTROL_ADDR : begin 
csr_rdata_local = tcontrol; 
end 
default : begin 
csr_match_local = 1'b0; 
csr_rdata_local = 'hx; 
end 
endcase 
end 
assign pcu_csr_excpt = pcu_csr_match_local & ( csr_only_m_mode & (core_priv_mode_reg == PRIV_MODE_U) | 
csr_only_d_mode & !debug_mode | 
csr_only_r & pcu_csr_has_write_op_ex ); 
assign pcu_csr_match_local = csr_match_local & pcu_csr_vld; 
`WDFFER(nxt_pc_fr_cmt_nxt,(cmt_inst_retire | {{(2-1){1'b0}},ifu_pcu_hv_done}) & {2{~cmt_flush}},cmt_inst_vld[0] & !debug_mode | cmt_flush | ifu_pcu_hv_done,clk,rst_n) 
`WDFFER(csr_flush_q, cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & !cmt_inst_funct[0][FU_LSU_BIT] & cmt_inst_sub_funct[0][CSR_INST] & !cmt_flush,cmt_inst_vld[0] & !debug_mode | cmt_flush,clk,rst_n) 
assign int_tkn_wi_nmi = int_vld | nmi_vld; 
assign mstatus_d.mprv = cmt_csr_vld ? csr_wdata[17] : 1'b0; 
assign mstatus_d.mpp = (excp_vld | int_tkn_wi_nmi) ? core_priv_mode_reg : 
cmt_csr_vld ? ((pcu_csr_index == CSR_MSTATUS_ADDR) ? ((|csr_wdata[12:11])? PRIV_MODE_M : PRIV_MODE_U) : 
((|csr_wdata[29:28])? PRIV_MODE_M : PRIV_MODE_U) ) : PRIV_MODE_U; 
assign mstatus_d.mpie = (excp_vld | int_tkn_wi_nmi) ? mstatus.mie : 
cmt_csr_vld ?((pcu_csr_index == CSR_MSTATUS_ADDR) ? csr_wdata[7] : csr_wdata[27]) : 1'b1; 
assign mnxti_updt_mie = cmt_inst_opcode[0][CSRRW] & ( exu_pcu_inst_alu_data[0][3]) | 
cmt_inst_opcode[0][CSRRS] & (mstatus.mie | exu_pcu_inst_alu_data[0][3]) | 
cmt_inst_opcode[0][CSRRC] & (mstatus.mie & ~exu_pcu_inst_alu_data[0][3]) ; 
assign mstatus_d.mie = (excp_vld | int_tkn_wi_nmi) ? 1'b0 : 
cmt_csr_vld ? mnxti_updt_mie : 
mstatus.mpie ; 
assign mstatus_updt_en = excp_vld_wi_nmi | mret_inst | int_tkn_wi_nmi | mnret_inst & ~mnstatus.mnpp[0]| 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSTATUS_ADDR) | 
mnxti_csr_rw_inst & (mstatus_d.mie != mstatus.mie)| dm_pcu_resume_req & debug_mode & ~dcsr.prv[0]; 
assign mstatus_mprv_updt_en = mnret_inst & ~mnstatus.mnpp[0] | 
mret_inst & ~mstatus.mpp[0] | 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSTATUS_ADDR) | 
dm_pcu_resume_req & debug_mode & ~dcsr.prv[0] ; 
assign mstatus_mpp_updt_en = 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MCAUSE_ADDR) | 
excp_vld_wi_nmi | mret_inst | int_tkn_wi_nmi | 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSTATUS_ADDR); 
assign mstatus_mpie_updt_en = 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MCAUSE_ADDR) | 
excp_vld_wi_nmi | mret_inst | int_tkn_wi_nmi | 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSTATUS_ADDR); 
assign mstatus_mie_updt_en = excp_vld_wi_nmi | mret_inst | int_tkn_wi_nmi | 
cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSTATUS_ADDR) | 
mnxti_csr_rw_inst ; 
`WDFFER (mstatus.mprv,mstatus_d.mprv,mstatus_mprv_updt_en,clk,rst_n) 
`WDFFERVAL(mstatus.mpp ,mstatus_d.mpp ,mstatus_mpp_updt_en ,clk,rst_n,2'b11) 
`WDFFERVAL(mstatus.mpie,mstatus_d.mpie,mstatus_mpie_updt_en,clk,rst_n,1'b1) 
`WDFFER (mstatus.mie ,mstatus_d.mie ,mstatus_mie_updt_en ,clk,rst_n) 
assign mstatus_val = {10'b0,1'b1,3'b0,mstatus.mprv,4'b0,mstatus.mpp,3'b0,mstatus.mpie,top_mstatus_be,2'b0,mstatus.mie,3'b0}; 
assign mie_d = 32'b0; 
assign mie_updt_en = 1'b0; 
`WDFFER(mie,mie_d,mie_updt_en,clk,rst_n) 
assign mtvec_updt_en = cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MTVEC_ADDR); 
assign mtvec_d = csr_wdata; 
`WDFFER(mtvec_pre,mtvec_d,mtvec_updt_en,clk,rst_n) 
assign mtvec = {mtvec_pre[32-1:6],6'b000011}; 
logic [32-1:0] mstatush_bk; 
assign mstatush = {32{1'b0}}; 
assign mstatush_bk = {32{1'b0}}; 
assign mscratch_updt_en = cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSCRATCH_ADDR) 
| cmt_csr_vld & (pcu_csr_index == CSR_MSCRATCHCSW_ADDR) & (mstatus.mpp != PRIV_MODE_M) & !mscratchcsw_csr_nop_inst 
| cmt_csr_vld & (pcu_csr_index == CSR_MSCRATCHCSWL_ADDR) & ((mcause.mpil == 0) != (mintstatus[31:24] == 0)) & !mscratchcsw_csr_nop_inst 
; 
assign mscratch_d = (cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MSCRATCH_ADDR))? csr_wdata : exu_pcu_inst_alu_data[0]; 
`WDFFER(mscratch,mscratch_d,mscratch_updt_en,clk,rst_n) 
assign mepc_d[32-1:0] = int_shv_excp_vld ? ifu_pcu_hv_pc : 
(excp_vld | int_tkn_cur_vld)? cmt_pc[0] : 
cmt_csr_vld ? csr_wdata : 
cmt_pc[1] + ({3{(nxt_pc_fr_cmt_nxt[1] & !(|cmt_inst_vld) | cmt_inst_vld[1] & !cmt_uop_inst)}} & (cmt_inst_type[1]? 3'h4 : 3'h2)); 
assign mepc_updt_en = excp_vld_wi_nmi | int_tkn_wi_nmi | cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MEPC_ADDR); 
`WDFFER(mepc,{mepc_d[32-1:1],1'b0},mepc_updt_en,clk,rst_n) 
assign mcause_csrinst_updt_en = cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MCAUSE_ADDR); 
assign mcause_updt_en = mcause_csrinst_updt_en | 
mnxti_csr_rw_inst & mnxti_csr_clic_nest_en & (mcause_d.interrupt != mcause.interrupt | mcause.exccode != mcause_d.exccode)| 
excp_vld_wi_nmi | 
int_tkn_wi_nmi | 
mcause_minhv_updt_en ; 
assign mcause_d.interrupt = 
nmi_vld ? 1'b1 : 
excp_vld ? 1'b0 : 
(int_vld | mnxti_csr_rw_inst) ? 1'b1 : csr_wdata[31]; 
assign mcause_minhv_updt_en = ifu_pcu_hv_done & ~ifu_pcu_hv_excp_vld & !jvt_wb | int_vld | mcause_csrinst_updt_en; 
assign mcause_d.minhv = int_vld ? clic_pcu_int_shv : 
ifu_pcu_hv_done & !jvt_wb ? 1'b0 : csr_wdata[30]; 
assign mcause_d.mpil = int_tkn_wi_nmi ? mintstatus[31:24] : csr_wdata[23:16]; 
assign mcause_d.exccode = 
nmi_vld ? NMI_ID : 
excp_vld ? excp_cause : 
(int_vld | mnxti_csr_rw_inst) ? int_cause : csr_wdata[11:0]; 
assign mcause_int_updt_en = mcause_csrinst_updt_en | mnxti_csr_rw_inst & mnxti_csr_clic_nest_en | excp_vld_wi_nmi | int_tkn_wi_nmi; 
assign mcause_mpil_updt_en = mcause_csrinst_updt_en | int_tkn_wi_nmi; 
assign mcause_exccode_updt_en = mcause_csrinst_updt_en | excp_vld_wi_nmi | int_tkn_wi_nmi | mnxti_csr_rw_inst & mnxti_csr_clic_nest_en; 
`WDFFER(mcause.interrupt,mcause_d.interrupt,mcause_int_updt_en ,clk,rst_n) 
`WDFFER(mcause.minhv, mcause_d.minhv, mcause_minhv_updt_en ,clk,rst_n) 
`WDFFER(mcause.mpil, mcause_d.mpil, mcause_mpil_updt_en ,clk,rst_n) 
`WDFFER(mcause.exccode, mcause_d.exccode, mcause_exccode_updt_en,clk,rst_n) 
assign mcause_val = {mcause.interrupt,mcause.minhv,mstatus.mpp,mstatus.mpie,3'b0,mcause.mpil,4'b0,mcause.exccode}; 
assign mtval_d = 
cmt_inst_vld[0] & cmt_excp_vld & !(cmt_trig_exp & !dbg_disable) & (cmt_excp_cause[1:0] == 2'h2)? cmt_pc[1] : 
cmt_inst_vld[0] & (wb_inst_excp_vld|ebreak_2_excp) ? cmt_pc[0] : 
ifu_pcu_hv_done & ifu_pcu_hv_excp_vld ? ifu_pcu_hv_pc : 
ecall_inst ? 32'b0 : 
lsu_pcu_cmt_excp[0] & lsu_pcu_cmt ? lsu_pcu_cmt_excp_addr[0] : 
csr_wdata; 
assign mtval_updt_en = excp_vld_wi_nmi | cmt_oths_csr_wr_vld & (pcu_csr_index == CSR_MTVAL_ADDR); 
`WDFFER(mtval,mtval_d,mtval_updt_en,clk,rst_n) 
assign mip = {32{1'b0}}; 
assign mtinst ={32{1'b0}}; 
assign mtval2 = {32{1'b0}}; 
assign menvcfg = {32{1'b0}}; 
assign menvcfgh = {32{1'b0}}; 
assign mseccfg = {32{1'b0}}; 
assign mseccfgh = {32{1'b0}}; 
assign mnscratch = 'b0; 
assign mnepc = 'b0; 
assign mncause_val = 'b0; 
assign mnstatus = 'b0; 
assign dcsr_d.debuger = 4'h4; 
assign dcsr_d.ebreakvs = 1'b0; 
assign dcsr_d.ebreakvu = 1'b0; 
assign dcsr_d.pelp = 1'b0; 
assign dcsr_d.ebreakm = csr_wdata[15]; 
assign dcsr_d.ebreaks = 1'b0; 
assign dcsr_d.ebreaku = csr_wdata[12]; 
assign dcsr_d.stepie = csr_wdata[11]; 
assign dcsr_d.stopcount = csr_wdata[10]; 
assign dcsr_d.stoptime = csr_wdata[9]; 
assign dcsr_d.cause = dm_pcu_halt_on_reset & (core_status == UNAVAIL) ? 3'h5 : 
dm_pcu_halt_req & !dbg_disable ? 3'h3 : 
trigger_dbg ? 3'h2 : 
ebreak_2_dbg ? 3'h1 : 3'h4 ; 
assign dcsr_d.v = 1'b0; 
assign dcsr_d.mprven = csr_wdata[4] ; 
assign dcsr_d.nmip = nmi_pending_vld_in_dm ; 
assign dcsr_d.step = csr_wdata[2] ; 
assign dcsr_d.prv = debug_mode ? ((|csr_wdata[1:0]) ? PRIV_MODE_M : PRIV_MODE_U): core_priv_mode_reg; 
assign dcsr_updt_en = dbg_flush | cmt_csr_wr_vld & (pcu_csr_index == CSR_DCSR_ADDR) & debug_mode; 
logic dcsr_csr_update_en; 
assign dcsr_csr_update_en = cmt_csr_wr_vld & (pcu_csr_index == CSR_DCSR_ADDR) & debug_mode; 
`WDFFERVAL(dcsr.prv ,dcsr_d.prv ,dcsr_updt_en ,clk,rst_n,2'h3) 
`WDFFER (dcsr.step ,dcsr_d.step ,dcsr_csr_update_en ,clk,rst_n) 
`WDFFER (dcsr.mprven ,dcsr_d.mprven ,dcsr_csr_update_en ,clk,rst_n) 
`WDFFER (dcsr.cause ,dcsr_d.cause ,dbg_flush ,clk,rst_n) 
`WDFFER (dcsr.stoptime ,dcsr_d.stoptime ,dcsr_csr_update_en ,clk,rst_n) 
`WDFFER (dcsr.stopcount ,dcsr_d.stopcount ,dcsr_csr_update_en ,clk,rst_n) 
`WDFFER (dcsr.stepie ,dcsr_d.stepie ,dcsr_csr_update_en ,clk,rst_n) 
`WDFFER (dcsr.ebreaku ,dcsr_d.ebreaku ,dcsr_csr_update_en ,clk,rst_n) 
`WDFFER (dcsr.ebreakm ,dcsr_d.ebreakm ,dcsr_csr_update_en ,clk,rst_n) 
assign dcsr.pelp = 1'b0; 
assign dcsr.nmip = nmi_pending_vld_in_dm ; 
assign dcsr_step = dcsr.step & !dbg_disable; 
assign dcsr_val = {4'h4,9'b0,dcsr.pelp,2'b0,dcsr.ebreakm,1'b0,1'b0,dcsr.ebreaku,dcsr.stepie,dcsr.stopcount,dcsr.stoptime,dcsr.cause,1'b0,dcsr.mprven,dcsr.nmip,dcsr.step,dcsr.prv}; 
assign dcsr_stoptime = dcsr.stoptime & core_dbg_mode; 
assign dcsr_stopcount = dcsr.stopcount; 
assign dpc_d = (dbg_flush_wo_step & (!nxt_pc_fr_cmt_nxt[0] | cmt_inst_vld[0]))? cmt_pc[0] : 
step_flush ? ifu_pcu_instr_pc[0] : 
cmt_csr_vld ? csr_wdata : 
cmt_pc[1] + ({3{(nxt_pc_fr_cmt_nxt[1] & !(|cmt_inst_vld) | cmt_inst_vld[1] & !cmt_uop_inst)}} & (cmt_inst_type[1]? 3'h4 : 3'h2)); 
assign dpc_updt_en = dbg_flush | cmt_csr_wr_vld & (pcu_csr_index == CSR_DPC_ADDR) & debug_mode; 
`WDFFER(dpc_pre,dpc_d,dpc_updt_en,clk,rst_n) 
assign dpc = {dpc_pre[31:1],1'b0}; 
assign dscratch0_d = dm_pcu_dsch0_write ? dm_pcu_dsch0_wdata : csr_wdata; 
assign dscratch0_updt_en = cmt_csr_wr_vld & (pcu_csr_index == CSR_DSCRATCH0_ADDR) & debug_mode| dm_pcu_dsch0_write & debug_mode; 
`WDFFER(dscratch0,dscratch0_d,dscratch0_updt_en,clk,rst_n) 
assign dscratch2_updt_en = cmt_csr_wr_vld & (pcu_csr_index == CSR_DSCRATCH2_ADDR) & debug_mode; 
`WDFFER(dscratch2,csr_wdata,dscratch2_updt_en,clk,rst_n) 
assign dscratch3_updt_en = cmt_csr_wr_vld & (pcu_csr_index == CSR_DSCRATCH3_ADDR) & debug_mode; 
`WDFFER(dscratch3,csr_wdata,dscratch3_updt_en,clk,rst_n) 
assign tcontrol_d[32-1:8] = {(32-8){1'b0}}; 
assign tcontrol_d[7] = (excp_vld | nmi_vld | int_vld) ? tcontrol[3] : 
cmt_csr_vld ? csr_wdata[7]: 1'b0; 
assign tcontrol_d[6:4] = 3'b0; 
assign tcontrol_d[3] = (mret_inst|mnret_inst) ? tcontrol[7] : 
cmt_csr_vld ? csr_wdata[3]: 1'b0; 
assign tcontrol_d[2:0] = 3'b0; 
assign tcontrol_updt_en = cmt_tm_csr_wr_vld & (pcu_csr_index == CSR_TCONTROL_ADDR) | 
excp_vld | nmi_vld | 
int_vld | 
mret_inst | mnret_inst; 
`WDFFER(tcontrol,tcontrol_d,tcontrol_updt_en,clk,rst_n) 
assign mtvt_d = {csr_wdata[32-1:6],6'b0}; 
assign mtvt_updt_en = cmt_clic_csr_wr_vld & (pcu_csr_index == CSR_MTVT_ADDR); 
`WDFFER(mtvt,mtvt_d,mtvt_updt_en,clk,rst_n) 
assign mnxti = mnxti_csr_clic_nest_en ? ({mtvt[32-1:6],6'b0} + {clic_pcu_int_id,2'b0}) : {32{1'b0}}; 
assign mintstatus_d[23:0] = {24'b0}; 
assign mintstatus_d[31:24] = nmi_vld ? 8'hff : (int_vld | mnxti_csr_rw_inst) ? clic_pcu_int_lvl : mcause.mpil; 
assign mintstatus_updt_en = nmi_vld | int_vld | mret_inst & mcause.interrupt | mnxti_csr_rw_inst & mnxti_csr_clic_nest_en ; 
`WDFFER(mintstatus,mintstatus_d,mintstatus_updt_en,clk,rst_n) 
assign mintthresh_d = {24'h0,mret_inst ? {8{1'b0}} : csr_wdata[7-:8],{(8-8){1'b1}}}; 
assign mintthresh_updt_en = cmt_clic_csr_wr_vld & (pcu_csr_index == CSR_MINTTHRESH_ADDR) | mret_inst & (mstatus.mpp == PRIV_MODE_U); 
`WDFFER(mintthresh_pre,mintthresh_d,mintthresh_updt_en,clk,rst_n) 
assign mintthresh = {24'h0,mintthresh_pre[7-:8],{(8-8){1'b1}}}; 
assign pcu_csr_wdata = csr_wdata; 
assign pcu_csr_vld = csr_vld_ex | cmt_csr_vld; 
assign pcu_csr_index = cmt_inst_vld[0] & cmt_inst_funct[0][FU_SPU_BIT] & cmt_inst_sub_funct[0][CSR_INST] ? cmt_csr_idx_wb : csr_idx_ex; 
assign pcu_csr_opcode = cmt_csr_wr_vld ? 2'b01 : 2'b00 ; 
assign pcu_csr_rdata_all = csr_rdata_all; 
logic [11:0] access_csr_idx; 
logic [31:0] access_csr_wdata; 
assign access_csr_idx = pcu_csr_index; 
assign access_csr_wdata = pcu_csr_wdata; 
assign pcu_csr_mprv = (debug_mode & !dcsr.mprven)? 1'b0 : mstatus.mprv; 
assign pcu_csr_mpp = mstatus.mpp; 
assign pcu_csr_ulpe_en = menvcfg[2]; 
assign pcu_csr_mlpe_en = mseccfg[10]; 
assign pcu_csr_mstatus_be = top_mstatus_be; 
assign pcu_dm_dsch0_rdata = dscratch0; 
assign pcu_tm_trap_vld = excp_vld | int_vld | nmi_vld; 
assign pcu_tm_trigger_hit = (excp_vld_wi_nmi & (mcause_d.exccode == BRKPT) & !ebreak_2_excp | (dcsr_d.cause == 3'h2) & cmt_inst_vld[0] & !debug_mode) & !dbg_disable; 
assign pcu_tm_trap_cause = {mcause_d.interrupt,19'b0,mcause_d.exccode}; 
assign pcu_tm_inst_retire = cmt_inst_retire & {2{~debug_mode}}; 
assign pcu_tm_tcontrol_mte = tcontrol[3]; 
logic pcu_clic_rsp_vld_d; 
logic pcu_clic_rsp_vld_en; 
assign pcu_clic_rsp_vld_en = pcu_clic_rsp_vld | int_vld | mnxti_csr_rw_inst & mnxti_csr_clic_nest_en; 
assign pcu_clic_rsp_vld_d = int_vld | mnxti_csr_rw_inst & mnxti_csr_clic_nest_en; 
`WDFFER(pcu_clic_rsp_vld, pcu_clic_rsp_vld_d, pcu_clic_rsp_vld_en, clk, rst_n) 
logic pcu_clic_rsp_clr_ip_d; 
assign pcu_clic_rsp_clr_ip_d = int_vld & clic_pcu_int_shv | mnxti_csr_rw_inst & mnxti_csr_clic_nest_en; 
`WDFFENR(pcu_clic_rsp_id, clic_pcu_int_id, pcu_clic_rsp_vld_d, clk) 
`WDFFENR(pcu_clic_rsp_clr_ip, pcu_clic_rsp_clr_ip_d, pcu_clic_rsp_vld_d, clk) 
assign pcu_csr_sp_en = 1'b1; 
assign pcu_csr_split_integrity = 1'b0; 
assign cmt_acc_bus_err = excp_vld_wi_nmi & ((excp_cause == RAS_ERR )); 
assign cmt_acc_pmp_err = excp_vld_wi_nmi & ((excp_cause == IFU_PMP_ERR ) | 
(excp_cause == LD_MISALIGN ) | 
(excp_cause == LD_ACC_FAULT) | 
(excp_cause == ST_MISALIGN ) | 
(excp_cause == ST_ACC_FAULT) ); 
assign cmt_inst_dec_ill_err = excp_vld_wi_nmi & (excp_cause == ILL_INST); 
assign cmt_inst_lpd_flow_err = excp_vld_wi_nmi & (excp_cause == LP_ERR); 
logic br_inst_wb ; 
logic uninf_inst_wb; 
logic uncond_j_wb; 
assign uncond_j_wb = cmt_inst_vld[0] & cmt_inst_funct[0][FU_BRU_BIT] & ~cmt_inst_sub_funct[0][1]; 
assign br_inst_wb = cmt_inst_vld[0] & cmt_inst_funct[0][FU_BRU_BIT] & cmt_inst_sub_funct[0][1]; 
assign uninf_inst_wb = cmt_inst_vld[0] & cmt_inst_funct[0][FU_BRU_BIT] & cmt_inst_sub_funct[0][2]; 
assign wb_inst_vld = cmt_inst_retire[0]; 
assign wb_branch_status = {1'b0,uncond_j_wb,br_inst_wb,uncond_j_wb|br_inst_wb}; 
assign wb_pc_addr = cmt_pc[0]; 
endmodule
 
 
 
module m130_pcu_uop ( 
input logic clk , 
input logic rst_n , 
input logic [31:0] ifu_pcu_instr_data, 
input logic [2-1:0][$clog2(32)-1:0] instr_decd_src1_idx, 
input logic [2-1:0][$clog2(32)-1:0] instr_decd_src2_idx, 
input logic [2-1:0][4:0] instr_decd_dest_idx, 
input logic [2-1:0][1:0] instr_decd_op2_src, 
input logic core_dbg_mode , 
input logic cmt_flush , 
input logic iss_vld , 
input logic iss_src_raw , 
input logic [1:0] uop_shift , 
input logic uop_pop , 
input logic uop_pop_ret , 
input logic uop_popretz , 
input logic uop_pop_all , 
input logic uop_push , 
input logic uop_inst , 
input logic uop_mv , 
input logic uop_csr_cs , 
input logic uop_ld_dest , 
input logic uop_st_src , 
input logic ls_exe_2_cycs_cmt , 
input logic pcu_csr_split_integrity, 
output logic [2-1:0] uop_dst_vld , 
output logic [2-1:0][$clog2(32)-1:0] uop_dst_idx , 
output logic [2-1:0] uop_rs1_vld , 
output logic [2-1:0][$clog2(32)-1:0] uop_rs1_idx , 
output logic [2-1:0] uop_rs2_vld , 
output logic [2-1:0][$clog2(32)-1:0] uop_rs2_idx , 
output logic [2-1:0] uop_rs2_fr_imm_vld, 
output logic [2-1:0][31:0] uop_rs2_fr_imm , 
output logic [2-1:0] uop_rs1_src_vld , 
output logic [2-1:0][2:0] uop_rs1_src , 
output logic [2-1:0] uop_rs2_src_vld , 
output logic [2-1:0][1:0] uop_rs2_src , 
output logic [2-1:0] uop_funct_vld , 
output logic [2-1:0][8-1:0] uop_funct , 
output logic [2-1:0] uop_sub_funct_vld , 
output logic [2-1:0][4-1:0] uop_sub_funct , 
output logic [2-1:0] uop_opcode_vld , 
output logic [2-1:0][8-1:0] uop_opcode , 
output logic uop_stall , 
output logic uop_int_mask , 
output logic uop_inst_cmt_en , 
output logic split_ret_state , 
output logic uop_ill , 
output logic shf_uop_fwd , 
output logic split_idle_state , 
output logic split_uop_state , 
output logic iss1_uop_vld 
); 
 
localparam X0 = 5'b0 ; 
localparam X2 = 5'h2 ; 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
localparam FENCE_INST = 0; 
localparam AMO_INST = 1; 
localparam LD_INST = 2; 
localparam ST_INST = 3; 
localparam LS_POST = 5; 
localparam EXCLUSIVE_INST = 0; 
localparam SYS = 0 ; 
localparam SYS_E = 1 ; 
localparam CSR_INST = 2 ; 
localparam BRKPT = 12'h3 ; 
localparam ILL_INST = 12'h2 ; 
localparam LP_ERR = 12'h12 ; 
localparam ECALL_U = 12'h8 ; 
localparam ECALL_M = 12'hb ; 
localparam LD_RAS_ERR = 12'h1F ; 
localparam INST_MISALIGN = 12'h0 ; 
localparam IFU_PMP_ERR = 12'h1 ; 
localparam LD_ACC_FAULT = 12'h5 ; 
localparam LD_MISALIGN = 12'h4 ; 
localparam ST_ACC_FAULT = 12'h7 ; 
localparam ST_MISALIGN = 12'h6 ; 
localparam RAS_ERR = 12'h1f ; 
localparam CSRRW = 0 ; 
localparam CSRRS = 1 ; 
localparam CSRRC = 2 ; 
localparam CSRIMM = 3 ; 
localparam UNAVAIL = 2'h0; 
localparam HALTED = 2'h1; 
localparam RUNNING = 2'h2; 
localparam NMI_ID = 12'hFFF ; 
localparam MRET = 0 ; 
localparam WFI = 1 ; 
localparam MNRET = 2 ; 
localparam ECALL = 0 ; 
localparam EBREAK = 1 ; 
localparam MRASIE = 31 ; 
localparam MEIE = 11 ; 
localparam MTIE = 7 ; 
localparam MSIE = 3 ; 
localparam MRASIP = 31 ; 
localparam MEIP = 11 ; 
localparam MTIP = 7 ; 
localparam MSIP = 3 ; 
typedef enum logic [4:0]{ 
LOAD = 5'b00001 , 
STORE = 5'b00010 , 
FENCE = 5'b00100 
}sub_func_lsu_e ; 
typedef enum logic [4:0] { 
SUB_FU_NONE = 5'b00000 , 
ARITHMATIC = 5'b00001 , 
SHIFT = 5'b00010 , 
LOGICAL = 5'b00100 , 
MISC = 5'b01000 
} sub_func_alu_e; 
typedef enum logic [4:0] { 
UNCOND_DIRE = 5'b00001 , 
COND_DIRE = 5'b00010 , 
UNCOND_INDIRE = 5'b00100 
}sub_func_bru_e ; 
typedef enum logic [3:0] { 
OTHER_TYPES = 4'd0 , 
EXCP_TYPE = 4'd1 , 
INT_TYPE = 4'd2 , 
MRET_TYPE = 4'd3 , 
NT_BR_TYPE = 4'd4 , 
T_BR_TYPE = 4'd5 , 
UNINF_TYPE = 4'd6 
}itype; 
typedef struct packed{ 
logic interrupt; 
logic minhv; 
logic [1:0] mpp; 
logic mpie; 
logic [7:0] mpil; 
logic [11:0] exccode; 
} mcause_csr; 
typedef struct packed{ 
logic mprv; 
logic [1:0] mpp; 
logic mpie; 
logic mie; 
} mstatus_csr; 
typedef struct packed{ 
logic mnpelp; 
logic [1:0] mnpp; 
logic mnpv; 
logic nmie; 
}mnstatus_csr; 
typedef struct packed{ 
logic [3:0] debuger ; 
logic pelp ; 
logic ebreakvs ; 
logic ebreakvu ; 
logic ebreakm ; 
logic ebreaks ; 
logic ebreaku ; 
logic stepie ; 
logic stopcount; 
logic stoptime ; 
logic [2:0] cause ; 
logic v ; 
logic mprven ; 
logic nmip ; 
logic step ; 
logic [1:0] prv ; 
}dcsr_csr; 
typedef enum logic [3:0] { 
SPLIT_IDLE = 4'b0000 , 
SPLIT_UOP = 4'b0001 , 
SPLIT_LI = 4'b0010 , 
SPLIT_ADD = 4'b0110 , 
SPLIT_RET = 4'b1000 , 
SPLIT_SHF_ADD = 4'b1100 
} state_e; 
 
genvar i; 
logic [$clog2(32)-1:0] uop_rw_idx_ex ; 
logic [11:0] iss_split_addi_imm_ex ; 
logic [3:0] uop_split_cnt_pre ; 
logic [3:0] uop_split_cnt ; 
logic [3:0] uop_split_cnt_d ; 
logic [3:0] uop_split_cnt_max ; 
state_e split_state ; 
state_e split_state_nxt ; 
logic split_state_updt_en ; 
logic [3:0] instr_decd_rlist ; 
logic [1:0] instr_decd_spimm ; 
logic [$clog2(32)-1:0] split_iss_rs2_idx_d_ex ; 
logic [$clog2(32)-1:0] split_iss_rs2_idx_ex ; 
logic split_code_ex_updt_vld ; 
logic [$clog2(32)-1:0] mv_snd_uop_inst_dst_idx ; 
logic [$clog2(32)-1:0] mv_snd_uop_inst_src2_idx ; 
logic uop_split_start ; 
logic [11:0] iss_split_imm_ex ; 
logic [2:0] sp_offset ; 
logic uop_split_cnt_updt_en ; 
logic split_add_state ; 
assign instr_decd_rlist = ifu_pcu_instr_data[7:4]; 
assign instr_decd_spimm = ifu_pcu_instr_data[3:2]; 
assign sp_offset = instr_decd_spimm + {&instr_decd_rlist,instr_decd_rlist[3:2] & {2{!(&instr_decd_rlist)}}}; 
always@* 
begin 
casez(uop_split_cnt_max - uop_split_cnt_pre) 
4'h0 : uop_rw_idx_ex = 5'h8; 
4'h1 : uop_rw_idx_ex = 5'h9; 
4'h2 : uop_rw_idx_ex = 5'h12; 
4'h3 : uop_rw_idx_ex = 5'h13; 
4'h4 : uop_rw_idx_ex = 5'h14; 
4'h5 : uop_rw_idx_ex = 5'h15; 
4'h6 : uop_rw_idx_ex = 5'h16; 
4'h7 : uop_rw_idx_ex = 5'h17; 
4'h8 : uop_rw_idx_ex = 5'h18; 
4'h9 : uop_rw_idx_ex = 5'h19; 
4'ha : uop_rw_idx_ex = 5'h1a; 
4'hb : uop_rw_idx_ex = 5'h1b; 
default : uop_rw_idx_ex = 5'hx; 
endcase 
end 
`WDFFERVAL(split_state, split_state_nxt, split_state_updt_en, clk, rst_n,SPLIT_IDLE) 
assign split_idle_state = split_state == SPLIT_IDLE; 
assign split_uop_state = split_state[0]; 
assign split_ret_state = split_state[3] & !split_state[2]; 
assign split_add_state = split_state[2]; 
always@* 
begin 
casez({cmt_flush,split_state}) 
{1'b1,4'b??? }: begin 
split_state_updt_en = 1'b1; 
split_state_nxt = SPLIT_IDLE; 
end 
{1'b0,SPLIT_IDLE}: begin 
split_state_updt_en = iss_vld & (uop_split_start & !iss_src_raw & ((uop_push|uop_pop) & (instr_decd_rlist != 4'h4) | uop_pop_ret | uop_csr_cs)) ; 
split_state_nxt = (instr_decd_rlist == 4'h4) & !(uop_csr_cs)? (uop_popretz ? SPLIT_LI : SPLIT_RET) : SPLIT_UOP; 
end 
{1'b0,SPLIT_UOP }: begin 
split_state_updt_en = ((uop_split_cnt == 4'h1)|uop_csr_cs) & iss_vld & !iss_src_raw; 
split_state_nxt = (uop_push|uop_pop|uop_csr_cs) ? SPLIT_IDLE : 
uop_popretz ? SPLIT_LI : SPLIT_RET; 
end 
{1'b0,SPLIT_LI }: begin 
split_state_updt_en = iss_vld; 
split_state_nxt = SPLIT_RET; 
end 
{1'b0,SPLIT_RET} : begin 
split_state_updt_en = !iss_src_raw & iss_vld; 
split_state_nxt = SPLIT_IDLE; 
end 
default : begin 
split_state_updt_en = 1'b0; 
split_state_nxt = SPLIT_IDLE; 
end 
endcase 
end 
assign uop_split_cnt_max = instr_decd_rlist[3:0] - (&instr_decd_rlist[3:0]? 4'h2 : 4'h3); 
assign iss_split_imm_ex = ({12{uop_pop_all}} & {5'b0,sp_offset,4'b0}) - {6'b0,uop_split_cnt_pre,2'b0}; 
assign iss1_uop_vld = (split_uop_state) & (uop_split_cnt == 4'h1) | (split_state == SPLIT_IDLE) & (instr_decd_rlist == 4'h4) & (uop_push | uop_pop_all); 
assign uop_split_start = (split_state == SPLIT_IDLE) & uop_inst & iss_vld; 
assign iss_split_addi_imm_ex = uop_pop_all ? {5'b0,sp_offset,4'b0} : ~{5'b0,sp_offset,4'b0} + 1'b1; 
assign shf_uop_fwd = (|uop_shift); 
assign split_iss_rs2_idx_d_ex = uop_rw_idx_ex; 
`WDFFENR(split_iss_rs2_idx_ex, split_iss_rs2_idx_d_ex, split_code_ex_updt_vld,clk) 
assign uop_split_cnt_pre = (split_state == SPLIT_IDLE) ? uop_split_cnt_max : uop_split_cnt; 
assign uop_split_cnt_d = cmt_flush ? 'b0 : uop_split_cnt_pre - 4'h1; 
assign uop_split_cnt_updt_en = uop_split_start | (split_uop_state) & iss_vld | cmt_flush ; 
`WDFFERVAL(uop_split_cnt, uop_split_cnt_d, uop_split_cnt_updt_en, clk, rst_n,'b0) 
assign split_code_ex_updt_vld = iss_vld; 
assign mv_snd_uop_inst_dst_idx = !ifu_pcu_instr_data[6]? {|ifu_pcu_instr_data[4:3],ifu_pcu_instr_data[4:3]==0,ifu_pcu_instr_data[4:2]} : 5'hb ; 
assign mv_snd_uop_inst_src2_idx = ifu_pcu_instr_data[6]? {|ifu_pcu_instr_data[4:3],ifu_pcu_instr_data[4:3]==0,ifu_pcu_instr_data[4:2]} : 5'hb ; 
assign uop_dst_vld[0] = (split_ret_state) | (split_state != SPLIT_IDLE) & !uop_push; 
assign uop_dst_idx[0] = {$clog2(32){(split_state == SPLIT_LI )}} & {{($clog2(32)-4){1'b0}},4'ha} | 
{$clog2(32){(split_uop_state)}} & split_iss_rs2_idx_ex[$clog2(32)-1:0] ; 
assign uop_dst_vld[1] = uop_inst|uop_ld_dest|uop_st_src; 
assign uop_dst_idx[1] = {$clog2(32){(uop_push|uop_pop_all)}} & {{($clog2(32)-4){1'b0}},4'h2} | 
{$clog2(32){ uop_mv}} & mv_snd_uop_inst_dst_idx | 
{$clog2(32){ (|uop_shift)}} & instr_decd_dest_idx[0][$clog2(32)-1:0] | 
{$clog2(32){ uop_ld_dest }} & ifu_pcu_instr_data[15+:$clog2(32)] ; 
assign uop_rs1_vld[0] = (split_ret_state); 
assign uop_rs1_idx[0] = {{($clog2(32)-4){1'b0}},4'h1}; 
assign uop_rs1_vld[1] = uop_inst; 
assign uop_rs1_idx[1] = {$clog2(32){(uop_push|uop_pop_all)}} & {{($clog2(32)-4){1'b0}},4'h2} | 
{$clog2(32){ (|uop_shift) }} & instr_decd_src2_idx[0]; 
assign uop_rs2_vld[0] = (split_uop_state ); 
assign uop_rs2_idx[0] = split_iss_rs2_idx_ex[$clog2(32)-1:0]; 
assign uop_rs2_vld[1] = uop_mv | (|uop_shift) | uop_st_src; 
assign uop_rs2_idx[1] = {$clog2(32){ uop_mv}} & mv_snd_uop_inst_src2_idx | 
{$clog2(32){(|uop_shift)|uop_st_src}} & ifu_pcu_instr_data[7+:$clog2(32)]; 
assign uop_rs1_src_vld[0] = (split_state == SPLIT_LI) | (split_ret_state); 
assign uop_rs1_src[0] = {2'b0,split_ret_state}; 
assign uop_rs1_src_vld[1] = uop_inst; 
assign uop_rs1_src[1] = uop_mv ? 3'b0 : 3'b1; 
assign uop_rs2_src_vld[0] = (split_state == SPLIT_LI) | (split_ret_state); 
assign uop_rs2_src[0] = 2'b0; 
assign uop_rs2_src_vld[1] = uop_inst; 
assign uop_rs2_src[1] = instr_decd_op2_src[0]; 
assign uop_rs2_fr_imm_vld[0] = uop_inst; 
assign uop_rs2_fr_imm[0] = (uop_csr_cs ) ? (split_uop_state? 32'h4 : 32'h0) : {{(32-12){iss_split_imm_ex[11]}},iss_split_imm_ex} ; 
assign uop_rs2_fr_imm_vld[1] = uop_inst; 
assign uop_rs2_fr_imm[1] = {{(32-12){iss_split_addi_imm_ex[11]}},iss_split_addi_imm_ex}; 
assign uop_funct_vld[0] = (split_state == SPLIT_LI) | (split_ret_state); 
assign uop_funct[0] = ((split_state == SPLIT_LI)) ? {{(8-2){1'b0}},2'b01} : {{(8-2){1'b0}},2'b10}; 
assign uop_funct_vld[1] = uop_inst; 
assign uop_funct[1] = {{(8-2){1'b0}},2'b01} ; 
assign uop_sub_funct_vld[0] = (split_state == SPLIT_LI) | (split_ret_state); 
assign uop_sub_funct[0] = ((split_state == SPLIT_LI)) ? {{(4-3){1'b0}},3'b001} : {{(4-3){1'b0}},3'b100}; 
assign uop_sub_funct_vld[1] = uop_inst; 
assign uop_sub_funct[1] = {{(4-3){1'b0}},3'b001}; 
assign uop_opcode_vld[0] = 1'b0; 
assign uop_opcode[0] = {(8){1'b0}} ; 
assign uop_opcode_vld[1] = uop_inst; 
assign uop_opcode[1] = uop_shift[1] ? {{(8-3){1'b0}},3'b010} : {{(8-3){1'b0}},3'b001}; 
assign uop_stall = iss_vld & ((split_state == SPLIT_IDLE) & ((uop_pop | uop_push) & ((instr_decd_rlist != 4'h4)) | uop_csr_cs)| 
(split_uop_state ) & (uop_split_cnt != 4'h1) & (uop_pop | uop_push) | 
(split_state != SPLIT_RET ) & uop_pop_ret); 
assign uop_int_mask = (split_state == SPLIT_LI ) | 
(split_ret_state ) | 
(split_state != SPLIT_IDLE) & uop_inst & (pcu_csr_split_integrity | uop_csr_cs); 
assign uop_inst_cmt_en = (split_ret_state ) & uop_pop_ret | 
(split_uop_state ) & ((uop_split_cnt == 4'h1) & (uop_push | uop_pop) | uop_csr_cs)| 
(split_state == SPLIT_IDLE) & (uop_pop | uop_push) & (instr_decd_rlist == 4'h4) | 
!(uop_inst & !(uop_mv | (|uop_shift) | uop_st_src | uop_ld_dest)); 
assign uop_ill = 
((instr_decd_rlist[3:0] == 4'h0) | (instr_decd_rlist[3:0] == 4'h1) | (instr_decd_rlist[3:0] == 4'h2) | (instr_decd_rlist[3:0] == 4'h3)) & (uop_pop_all | uop_push) | 
uop_mv & !ifu_pcu_instr_data[6] & (ifu_pcu_instr_data[9:7] == ifu_pcu_instr_data[4:2]) | 
uop_pop_ret & core_dbg_mode ; 
endmodule
 
 
module m130_exu_top #( 
parameter SUPPORT_MUL_APPROACH = 1 , 
parameter SUPPORT_DIV = 1 , 
parameter SUPPORT_DIV_APPROACH = 0 
)( 
input logic clk , 
input logic rst_n , 
input logic [2-1:0] pcu_exu_inst_vld , 
input logic pcu_exu_flush , 
input logic [2-1:0][32-1:0] pcu_exu_alu_op1_val , 
input logic [2-1:0][32-1:0] pcu_exu_alu_op2_val , 
input logic [2-1:0][$clog2(32)-1:0] pcu_exu_alu_shift_shamt , 
input logic [2-1:0][32-1:0] pcu_exu_alu_logic_op1_val , 
input logic [2-1:0][32-1:0] pcu_exu_alu_logic_op2_val , 
input logic [2-1:0][32-1:0] pcu_exu_bru_op1_val , 
input logic [2-1:0][32-1:0] pcu_exu_bru_eq_op1_val , 
input logic [2-1:0][32-1:0] pcu_exu_bru_op2_val , 
input logic [2-1:0][32-1:0] pcu_exu_bru_eq_op2_val , 
input logic [2-1:0][32-1:0] pcu_exu_mdu_op1_val , 
input logic [2-1:0][32-1:0] pcu_exu_mdu_op2_val , 
input logic [2-1:0][32-1:0] pcu_exu_agu_op1_val , 
input logic [2-1:0][32-1:0] pcu_exu_agu_op2_val , 
input logic [2-1:0][32-1:0] pcu_exu_inst_op3_val , 
input logic [2-1:0] pcu_exu_inst_signed , 
input logic [2-1:0][8-1:0] pcu_exu_inst_funct , 
input logic [2-1:0][4-1:0] pcu_exu_inst_sub_funct , 
input logic [2-1:0][8-1:0] pcu_exu_inst_opcode , 
input logic [2-1:0][4-1:0] pcu_exu_sub_funct_wb , 
input logic [2-1:0][8-1:0] pcu_exu_opcode_wb , 
input logic [2-1:0] pcu_exu_inst_type , 
input logic [2-1:0] pcu_exu_inst_is_ret , 
input logic [32-1:0] pcu_exu_inst_pc , 
input logic [32-1:0] pcu_exu_inst_pred_pc , 
input logic pcu_exu_csr_sp_en , 
output logic [2-1:0] exu_pcu_inst_wb_stall , 
output logic [2-1:0][32-1:0] exu_pcu_inst_alu_data , 
output logic [2-1:0][32-1:0] exu_pcu_inst_alu_logic_data , 
output logic [2-1:0][32-1:0] exu_pcu_inst_mdu_data , 
output logic exu_pcu_bru_flush , 
output logic [32-1:0] exu_pcu_bru_flush_pc , 
output logic [32-1:0] exu_pcu_inst_tgt_pc , 
output logic exu_pcu_br_tkn, 
output logic [32-1:0] exu_lsu_address 
); 
localparam FU_ALU_BIT = 0 ; 
localparam FU_BRU_BIT = 1 ; 
localparam FU_MDU_BIT = 2 ; 
localparam FU_LSU_BIT = 3 ; 
localparam FU_SPU_BIT = 4 ; 
localparam FU_BMU_BIT = 5 ; 
localparam FU_FPU_BIT = 6 ; 
localparam FU_UOP_BIT = 7 ; 
logic e_alu_inst_vld ; 
logic [32-1:0] e_alu_inst_op1_val ; 
logic [32-1:0] e_alu_inst_op2_val ; 
logic [32-1:0] e_alu_logic_inst_op1_val ; 
logic [32-1:0] e_alu_logic_inst_op2_val ; 
logic [$clog2(32)-1:0] e_alu_inst_shift_shamt ; 
logic [32-1:0] e_bru_inst_op1_val ; 
logic [32-1:0] e_bru_inst_op2_val ; 
logic [32-1:0] mdu_inst_op1_val ; 
logic [32-1:0] mdu_inst_op2_val ; 
logic [32-1:0] e_agu_inst_op1_val ; 
logic [32-1:0] e_agu_inst_op2_val ; 
logic e_alu_inst_signed ; 
logic [4-1:0] e_alu_inst_sub_funct ; 
logic [8-1:0] e_alu_inst_opcode ; 
logic e_agu_inst_vld ; 
logic e_bru_inst_vld ; 
logic e_spu_inst_vld ; 
logic e_bru_inst_type ; 
logic e_bru_inst_is_ret ; 
logic [32-1:0] e_bru_inst_op3_val ; 
logic [4-1:0] e_bru_inst_sub_funct ; 
logic [8-1:0] e_bru_inst_opcode ; 
logic e_bru_inst_signed ; 
logic [32-1:0] e_bru_inst_pc ; 
logic [32-1:0] e_bru_pred_tgt_pc ; 
logic [32-1:0] e_alu_inst_res_val ; 
logic [32-1:0] e_alu_logic_inst_res_val ; 
logic [32-1:0] e_agu_inst_res_val ; 
logic [32-1:0] e_bru_inst_res_val ; 
logic e_bru_flush ; 
logic [32-1:0] e_bru_flush_pc ; 
logic e_bru_tkn ; 
logic [32-1:0] e_bru_tgt_pc ; 
logic [32-1:0] e_alu_res ; 
logic e_bru_csr_sp_en ; 
logic mdu_inst_vld ; 
logic mdu_inst_vld_q ; 
logic [32-1:0] mdu_inst_op1_val_q ; 
logic mdu_inst_op1_signed ; 
logic mdu_inst_op2_signed ; 
logic [4-1:0] mdu_inst_sub_funct ; 
logic [4-1:0] mdu_sub_funct_wb ; 
logic [8-1:0] mdu_opcode_wb ; 
logic mdu_inst_res_stall ; 
logic mdu_csr_uni_dly ; 
logic mdu_random_exe_stall ; 
logic [32-1:0] mdu_inst_res_val ; 
logic mdu_csr_uni_ari_dly ; 
logic [32-1:0] exu_pcu_inst_ex_res_val ; 
logic [32-1:0] exu_pcu_inst_ex_res_val_q ; 
logic bmu_inst_vld ; 
logic [32-1:0] bmu_inst_op1_val ; 
logic [32-1:0] bmu_inst_op2_val ; 
logic [4-1:0] bmu_inst_sub_funct ; 
logic [8-1:0] bmu_inst_opcode ; 
logic bmu_inst_signed ; 
logic [32-1:0] bmu_inst_res_val ; 
logic [32-1:0] mdu_div_ite_ex_op1_val ; 
logic [32-1:0] mdu_div_ite_ex_op1_val_q ; 
logic mdu_div_ite_op1_neg ; 
logic mdu_div_ite_op1_neg_q ; 
logic [$clog2(32)-1:0] mdu_div_ite_op1_index ; 
logic [$clog2(32)-1:0] mdu_div_ite_op1_index_q ; 
logic [$clog2(32)-1:0] mdu_div_ite_op1_index_sel ; 
m130_exu_alu #( 
.FINE_GRAINED_DATA_GATING (5'b0 ), 
.SUPPORT_AGU (1 ), 
.SUPPORT_BRU (1 ) 
) u_early_alu( 
.clk (clk ), 
.rst_n (rst_n ), 
.alu_inst_vld (e_alu_inst_vld ), 
.alu_inst_op1_val (e_alu_inst_op1_val ), 
.alu_inst_op2_val (e_alu_inst_op2_val ), 
.alu_inst_shift_shamt (e_alu_inst_shift_shamt ), 
.alu_logic_inst_op1_val (e_alu_logic_inst_op1_val ), 
.alu_logic_inst_op2_val (e_alu_logic_inst_op2_val ), 
.agu_inst_op1_val (e_agu_inst_op1_val ), 
.agu_inst_op2_val (e_agu_inst_op2_val ), 
.bru_inst_op1_val (e_bru_inst_op1_val ), 
.bru_inst_op2_val (e_bru_inst_op2_val ), 
.alu_inst_signed (e_alu_inst_signed ), 
.alu_inst_sub_funct (e_alu_inst_sub_funct ), 
.alu_inst_opcode (e_alu_inst_opcode ), 
.agu_inst_vld (e_agu_inst_vld ), 
.bru_inst_vld (e_bru_inst_vld ), 
.bru_inst_type (e_bru_inst_type ), 
.bru_inst_is_ret (e_bru_inst_is_ret ), 
.bru_inst_eq_op1_val (pcu_exu_bru_eq_op1_val[0] ), 
.bru_inst_eq_op2_val (pcu_exu_bru_eq_op2_val[0] ), 
.bru_inst_op3_val (e_bru_inst_op3_val ), 
.bru_inst_sub_funct (e_bru_inst_sub_funct ), 
.bru_inst_opcode (e_bru_inst_opcode ), 
.bru_inst_signed (e_bru_inst_signed ), 
.bru_inst_pc (e_bru_inst_pc ), 
.bru_pred_tgt_pc (e_bru_pred_tgt_pc ), 
.alu_inst_res_val (e_alu_inst_res_val ), 
.alu_logic_inst_res_val ( ), 
.agu_inst_res_val (e_agu_inst_res_val ), 
.bru_inst_res_val (e_bru_inst_res_val ), 
.bru_flush (e_bru_flush ), 
.bru_flush_pc (e_bru_flush_pc ), 
.bru_tkn (e_bru_tkn ), 
.bru_csr_sp_en (e_bru_csr_sp_en ), 
.bru_tgt_pc (e_bru_tgt_pc ) 
); 
assign e_alu_inst_vld = pcu_exu_inst_funct[0][FU_ALU_BIT]; 
assign e_alu_inst_op1_val = pcu_exu_alu_op1_val[0]; 
assign e_alu_inst_op2_val = pcu_exu_alu_op2_val[0]; 
assign e_alu_inst_shift_shamt = pcu_exu_alu_shift_shamt[0]; 
assign e_alu_logic_inst_op1_val = pcu_exu_alu_logic_op1_val[0]; 
assign e_alu_logic_inst_op2_val = pcu_exu_alu_logic_op2_val[0]; 
assign e_agu_inst_op1_val = pcu_exu_agu_op1_val[0]; 
assign e_agu_inst_op2_val = pcu_exu_agu_op2_val[0]; 
assign e_bru_inst_op1_val = pcu_exu_bru_op1_val[0]; 
assign e_bru_inst_op2_val = pcu_exu_bru_op2_val[0]; 
assign e_alu_inst_signed = pcu_exu_inst_signed[0]; 
assign e_alu_inst_sub_funct = pcu_exu_inst_sub_funct[0]; 
assign e_alu_inst_opcode = pcu_exu_inst_opcode[0]; 
assign e_agu_inst_vld = pcu_exu_inst_funct[0][FU_LSU_BIT]; 
assign e_bru_inst_vld = pcu_exu_inst_funct[0][FU_BRU_BIT]; 
assign e_bru_inst_type = pcu_exu_inst_type[0]; 
assign e_bru_inst_is_ret = pcu_exu_inst_is_ret[0]; 
assign e_bru_inst_op3_val = pcu_exu_inst_op3_val[0]; 
assign e_bru_inst_sub_funct = pcu_exu_inst_sub_funct[0]; 
assign e_bru_inst_opcode = pcu_exu_inst_opcode[0]; 
assign e_bru_inst_signed = pcu_exu_inst_signed[0]; 
assign e_bru_inst_pc = pcu_exu_inst_pc; 
assign e_bru_pred_tgt_pc = pcu_exu_inst_pred_pc; 
assign e_bru_csr_sp_en = pcu_exu_csr_sp_en; 
assign e_alu_res = ({32{e_alu_inst_vld}} & e_alu_inst_res_val) | 
({32{e_bru_inst_vld}} & e_bru_inst_res_val) ; 
assign exu_lsu_address = e_agu_inst_res_val; 
assign exu_pcu_bru_flush = pcu_exu_inst_vld[0] & e_bru_flush; 
assign exu_pcu_bru_flush_pc = e_bru_flush_pc; 
assign exu_pcu_inst_tgt_pc = e_bru_tgt_pc; 
assign exu_pcu_br_tkn = e_bru_tkn; 
m130_exu_mdu #( 
.SUPPORT_MUL_APPROACH (SUPPORT_MUL_APPROACH ), 
.SUPPORT_DIV (SUPPORT_DIV ), 
.SUPPORT_DIV_APPROACH (SUPPORT_DIV_APPROACH ) 
) u_m130_exu_mdu( 
.clk (clk ), 
.rst_n (rst_n ), 
.mdu_inst_vld (mdu_inst_vld ), 
.mdu_inst_vld_q (mdu_inst_vld_q ), 
.mdu_inst_flush (pcu_exu_flush ), 
.mdu_inst_op1_val (mdu_inst_op1_val ), 
.mdu_inst_op2_val (mdu_inst_op2_val ), 
.mdu_inst_op1_val_q (mdu_inst_op1_val_q ), 
.mdu_inst_op1_signed (mdu_inst_op1_signed ), 
.mdu_inst_op2_signed (mdu_inst_op2_signed ), 
.div_ite_op1_neg_q (mdu_div_ite_op1_neg_q ), 
.div_ite_op1_index (mdu_div_ite_op1_index ), 
.div_ite_op1_index_q (mdu_div_ite_op1_index_q), 
.div_ite_op1_val_q (mdu_div_ite_ex_op1_val_q), 
.mdu_inst_sub_funct (mdu_inst_sub_funct ), 
.mdu_sub_funct_wb (mdu_sub_funct_wb ), 
.mdu_opcode_wb (mdu_opcode_wb ), 
.mdu_inst_res_stall (mdu_inst_res_stall ), 
.mdu_inst_res_val (mdu_inst_res_val ) 
); 
assign mdu_inst_vld = pcu_exu_inst_vld[0] & pcu_exu_inst_funct[0][FU_MDU_BIT]; 
assign mdu_inst_op1_val = pcu_exu_mdu_op1_val[0]; 
assign mdu_inst_op2_val = pcu_exu_mdu_op2_val[0]; 
assign mdu_inst_op1_signed = pcu_exu_inst_signed[0]; 
assign mdu_inst_op2_signed = pcu_exu_inst_sub_funct[0][0] ? (pcu_exu_inst_opcode[0][1] ? 1'b0 : pcu_exu_inst_signed[0]) 
: pcu_exu_inst_signed[0]; 
assign mdu_inst_sub_funct = pcu_exu_inst_sub_funct[0]; 
assign mdu_sub_funct_wb = pcu_exu_sub_funct_wb[0]; 
assign mdu_opcode_wb = pcu_exu_opcode_wb[0]; 
m130_exu_bmu #( 
.FINE_GRAINED_DATA_GATING (3'b0 ) 
) u_m130_exu_bmu( 
.bmu_inst_vld (bmu_inst_vld ), 
.bmu_inst_op1_val (bmu_inst_op1_val ), 
.bmu_inst_op2_val (bmu_inst_op2_val ), 
.bmu_inst_sub_funct (bmu_inst_sub_funct ), 
.bmu_inst_opcode (bmu_inst_opcode ), 
.bmu_inst_signed (bmu_inst_signed ), 
.bmu_inst_res_val (bmu_inst_res_val ) 
); 
assign bmu_inst_vld = pcu_exu_inst_vld[0] & pcu_exu_inst_funct[0][FU_BMU_BIT]; 
assign bmu_inst_op1_val = pcu_exu_alu_op1_val[0]; 
assign bmu_inst_op2_val = pcu_exu_alu_op2_val[0]; 
assign bmu_inst_sub_funct = pcu_exu_inst_sub_funct[0]; 
assign bmu_inst_opcode = pcu_exu_inst_opcode[0]; 
assign bmu_inst_signed = pcu_exu_inst_signed[0]; 
logic [32-1:0] exu_pcu_inst_ex2wb_val ; 
logic [32-1:0] exu_pcu_inst_ex2wb_val_q ; 
logic exu_pcu_inst_ex2wb_en ; 
logic exu_pcu_inst_ex_res_vld ; 
logic exu_pcu_inst_ex_res_vld_q ; 
logic mdu_vld_enable ; 
`WDFFR(mdu_inst_vld_q,mdu_inst_vld & (!pcu_exu_flush),clk,rst_n) 
assign mdu_vld_enable = 1'b0; 
`WDFFER(exu_pcu_inst_ex2wb_val_q, exu_pcu_inst_ex2wb_val, exu_pcu_inst_ex2wb_en, clk, rst_n) 
`WDFFR(exu_pcu_inst_ex_res_vld_q, exu_pcu_inst_ex_res_vld, clk, rst_n) 
`WDFFER(mdu_div_ite_op1_neg_q,mdu_div_ite_op1_neg,mdu_inst_vld,clk,rst_n) 
`WDFFER(mdu_div_ite_op1_index_q,mdu_div_ite_op1_index,mdu_inst_vld,clk,rst_n) 
if((SUPPORT_DIV == 1) & (SUPPORT_DIV_APPROACH == 1'b0)) begin:ITERATION_DIV 
assign mdu_div_ite_op1_neg = mdu_inst_op1_signed & mdu_inst_op1_val[32-1]; 
assign mdu_div_ite_ex_op1_val = mdu_div_ite_op1_neg ? (~mdu_inst_op1_val + 1'b1) : mdu_inst_op1_val; 
wing_cbb_lzc #( 
.DIRECTION (1 ), 
.WIDTH (32 ), 
.IDX_W ($clog2(32) ) 
) u_lzc_high2low( 
.vec_i (mdu_div_ite_ex_op1_val), 
.cnt_o (mdu_div_ite_op1_index ), 
.empty_o ( ) 
); 
assign mdu_div_ite_op1_index_sel = 
mdu_div_ite_op1_index; 
assign exu_pcu_inst_ex_res_val = pcu_exu_inst_funct[0][FU_BMU_BIT] ? bmu_inst_res_val : e_alu_res; 
assign exu_pcu_inst_ex2wb_val = ({32{mdu_inst_vld & (mdu_inst_sub_funct[0])}} & mdu_inst_op1_val) | 
({32{mdu_inst_vld & (mdu_inst_sub_funct[1] | mdu_inst_sub_funct[2])}} & mdu_div_ite_ex_op1_val) | 
({32{exu_pcu_inst_ex_res_vld}} & exu_pcu_inst_ex_res_val) ; 
assign exu_pcu_inst_ex_res_vld = pcu_exu_inst_vld[0] & (pcu_exu_inst_funct[0][FU_ALU_BIT] | pcu_exu_inst_funct[0][FU_BMU_BIT] | pcu_exu_inst_funct[0][FU_BRU_BIT]); 
assign exu_pcu_inst_ex2wb_en = exu_pcu_inst_ex_res_vld | mdu_inst_vld; 
assign exu_pcu_inst_ex_res_val_q = exu_pcu_inst_ex2wb_val_q; 
assign mdu_div_ite_ex_op1_val_q = exu_pcu_inst_ex2wb_val_q; 
assign mdu_inst_op1_val_q = exu_pcu_inst_ex2wb_val_q; 
end else begin: OTHERS 
assign mdu_div_ite_op1_neg = mdu_inst_op1_signed & mdu_inst_op1_val[32-1]; 
assign exu_pcu_inst_ex_res_val = pcu_exu_inst_funct[0][FU_BMU_BIT] ? bmu_inst_res_val : e_alu_res ; 
assign exu_pcu_inst_ex2wb_val = ({32{mdu_inst_vld}} & mdu_inst_op1_val) | 
({32{exu_pcu_inst_ex_res_vld}} & exu_pcu_inst_ex_res_val) ; 
assign exu_pcu_inst_ex_res_vld = pcu_exu_inst_vld[0] & (pcu_exu_inst_funct[0][FU_ALU_BIT] | pcu_exu_inst_funct[0][FU_BMU_BIT] | pcu_exu_inst_funct[0][FU_BRU_BIT]); 
assign exu_pcu_inst_ex2wb_en = exu_pcu_inst_ex_res_vld | mdu_inst_vld; 
assign exu_pcu_inst_ex_res_val_q = exu_pcu_inst_ex2wb_val_q; 
assign mdu_div_ite_ex_op1_val_q = exu_pcu_inst_ex2wb_val_q; 
assign mdu_inst_op1_val_q = exu_pcu_inst_ex2wb_val_q; 
assign mdu_div_ite_op1_index = {$clog2(32){1'b0}}; 
end 
if (2 == 2) begin:LATE_ALU 
logic l_alu_inst_vld ; 
logic [32-1:0] l_alu_inst_op1_val ; 
logic [32-1:0] l_alu_inst_op2_val ; 
logic [$clog2(32)-1:0] l_alu_inst_shift_shamt ; 
logic l_alu_inst_signed ; 
logic [4-1:0] l_alu_inst_sub_funct ; 
logic [8-1:0] l_alu_inst_opcode ; 
logic [32-1:0] l_alu_inst_res_val ; 
logic [32-1:0] l_alu_logic_inst_res_val ; 
m130_exu_alu #( 
.FINE_GRAINED_DATA_GATING (5'b0 ), 
.SUPPORT_AGU (0 ), 
.SUPPORT_BRU (0 ) 
) u_late_alu( 
.clk (clk ), 
.rst_n (rst_n ), 
.alu_inst_vld (l_alu_inst_vld ), 
.alu_inst_op1_val (l_alu_inst_op1_val ), 
.alu_inst_op2_val (l_alu_inst_op2_val ), 
.alu_inst_shift_shamt (l_alu_inst_shift_shamt ), 
.alu_logic_inst_op1_val (l_alu_inst_op1_val ), 
.alu_logic_inst_op2_val (l_alu_inst_op2_val ), 
.agu_inst_op1_val ({32{1'b0}} ), 
.agu_inst_op2_val ({32{1'b0}} ), 
.bru_inst_op1_val ({32{1'b0}} ), 
.bru_inst_op2_val ({32{1'b0}} ), 
.alu_inst_signed (l_alu_inst_signed ), 
.alu_inst_sub_funct (l_alu_inst_sub_funct ), 
.alu_inst_opcode (l_alu_inst_opcode ), 
.agu_inst_vld (1'b0 ), 
.bru_inst_vld (1'b0 ), 
.bru_inst_type (1'b0 ), 
.bru_inst_is_ret (1'b0 ), 
.bru_inst_eq_op1_val ({32{1'b0}} ), 
.bru_inst_eq_op2_val ({32{1'b0}} ), 
.bru_inst_op3_val ({32{1'b0}} ), 
.bru_inst_sub_funct ({4{1'b0}} ), 
.bru_inst_opcode ({8{1'b0}} ), 
.bru_inst_signed (1'b0 ), 
.bru_inst_pc ({32{1'b0}} ), 
.bru_pred_tgt_pc ({32{1'b0}} ), 
.alu_inst_res_val (l_alu_inst_res_val ), 
.alu_logic_inst_res_val (l_alu_logic_inst_res_val ), 
.agu_inst_res_val ( ), 
.bru_inst_res_val ( ), 
.bru_flush ( ), 
.bru_flush_pc ( ), 
.bru_tkn ( ), 
.bru_csr_sp_en ('0 ), 
.bru_tgt_pc ( ) 
); 
assign l_alu_inst_vld = pcu_exu_inst_vld[1]; 
assign l_alu_inst_op1_val = pcu_exu_alu_op1_val[1]; 
assign l_alu_inst_op2_val = pcu_exu_alu_op2_val[1]; 
assign l_alu_inst_shift_shamt = pcu_exu_alu_shift_shamt[1]; 
assign l_alu_inst_signed = pcu_exu_inst_signed[1]; 
assign l_alu_inst_sub_funct = pcu_exu_inst_sub_funct[1]; 
assign l_alu_inst_opcode = pcu_exu_inst_opcode[1]; 
assign exu_pcu_inst_wb_stall[1] = 1'b0; 
assign exu_pcu_inst_alu_data[1] = l_alu_inst_res_val; 
assign exu_pcu_inst_alu_logic_data[1] = l_alu_logic_inst_res_val; 
assign exu_pcu_inst_mdu_data[1] = '0; 
end 
assign exu_pcu_inst_wb_stall[0] = mdu_inst_res_stall; 
assign exu_pcu_inst_alu_data[0] = exu_pcu_inst_ex_res_val_q; 
assign exu_pcu_inst_alu_logic_data[0] = '0; 
assign exu_pcu_inst_mdu_data[0] = mdu_inst_res_val; 
endmodule
 
 
module m130_exu_alu #( 
parameter FINE_GRAINED_DATA_GATING = 5'b0 , 
parameter SUPPORT_AGU = 1 , 
parameter SUPPORT_BRU = 1 
)( 
input logic clk , 
input logic rst_n , 
input logic alu_inst_vld , 
input logic [32-1:0] alu_inst_op1_val , 
input logic [32-1:0] alu_inst_op2_val , 
input logic [$clog2(32)-1:0] alu_inst_shift_shamt , 
input logic [32-1:0] alu_logic_inst_op1_val , 
input logic [32-1:0] alu_logic_inst_op2_val , 
input logic [32-1:0] agu_inst_op1_val , 
input logic [32-1:0] agu_inst_op2_val , 
input logic [32-1:0] bru_inst_op1_val , 
input logic [32-1:0] bru_inst_op2_val , 
input logic alu_inst_signed , 
input logic [4-1:0] alu_inst_sub_funct , 
input logic [8-1:0] alu_inst_opcode , 
input logic agu_inst_vld , 
input logic bru_inst_vld , 
input logic bru_inst_type , 
input logic bru_inst_is_ret , 
input logic [32-1:0] bru_inst_eq_op1_val , 
input logic [32-1:0] bru_inst_eq_op2_val , 
input logic [32-1:0] bru_inst_op3_val , 
input logic [4-1:0] bru_inst_sub_funct , 
input logic [8-1:0] bru_inst_opcode , 
input logic bru_inst_signed , 
input logic [32-1:0] bru_inst_pc , 
input logic [32-1:0] bru_pred_tgt_pc , 
input logic bru_csr_sp_en , 
output logic [32-1:0] alu_inst_res_val , 
output logic [32-1:0] alu_logic_inst_res_val , 
output logic [32-1:0] agu_inst_res_val , 
output logic [32-1:0] bru_inst_res_val , 
output logic bru_flush , 
output logic [32-1:0] bru_flush_pc , 
output logic bru_tkn , 
output logic [32-1:0] bru_tgt_pc 
); 
localparam ARITHMATIC_BIT = 0 ; 
localparam SHIFT_BIT = 1 ; 
localparam LOGICAL_BIT = 2 ; 
localparam MISC_BIT = 3 ; 
localparam BRU_BIT = 4 ; 
logic [32-1:0] arithmatic_op1_val ; 
logic [32-1:0] arithmatic_op2_val ; 
logic [32-1:0] adder_a_val ; 
logic adder_a_signed ; 
logic [32-1:0] adder_b_val ; 
logic adder_b_signed ; 
logic adder_ci ; 
logic [32:0] adder_res ; 
logic [32-1:0] adder_res_val ; 
logic adder_res_signed ; 
logic [32-1:0] adder_max_val ; 
logic [32-1:0] adder_min_val ; 
logic [32-1:0] adder_min_max_val ; 
logic [32-1:0] arithmatic_inst_res ; 
logic [32-1:0] shift_op1_val ; 
logic [$clog2(32)-1:0] shift_op2_val ; 
logic [32-1:0] shift_src_high ; 
logic [32-1:0] shift_src_low ; 
logic [$clog2(32):0] shift_amount ; 
logic [32*2-1:0] shift_res ; 
logic [32-1:0] shift_inst_res ; 
logic [32-1:0] logical_op1_val ; 
logic [32-1:0] logical_op2_val ; 
logic [32-1:0] logical_inst_res ; 
logic logical_p0 ; 
logic logical_p1 ; 
logic logical_p2 ; 
logic logical_p3 ; 
logic [32-1:0] logical_temp1_val ; 
logic [32-1:0] logical_temp2_val ; 
logic [32-1:0] misc_op1_val ; 
logic [32-1:0] misc_op2_val ; 
logic [32-1:0] misc_orcb_res ; 
logic [32-1:0] misc_rev8_res ; 
logic [32-1:0] misc_inst_res ; 
logic [32-1:0] misc_rev8hw_res ; 
logic [32-1:0] misc_rev8slhw_res ; 
logic [32-1:0] bru_op1_val ; 
logic [32-1:0] bru_op2_val ; 
logic [32-1:0] bru_op3_val ; 
logic [32-1:0] bru_cond_adder_a_val ; 
logic bru_cond_adder_a_signed ; 
logic [32-1:0] bru_cond_adder_b_val ; 
logic bru_cond_adder_b_signed ; 
logic bru_cond_adder_ci ; 
logic [32:0] bru_cond_adder_res ; 
logic bru_cond_adder_res_signed; 
logic bru_cond_eq ; 
logic bru_cond_dir ; 
logic [32-1:0] bru_tgt_adder_a_val ; 
logic [32-1:0] bru_tgt_adder_b_val ; 
logic [32-1:0] bru_tgt_adder_res ; 
logic [32-1:0] bru_seq_pc ; 
logic bru_dir_mispred ; 
logic bru_tgt_mispred ; 
logic [32-1:0] bru_tgt_adder_res_tmp ; 
logic [32-1:0] re_adder_a_val ; 
logic re_adder_a_signed ; 
logic [32-1:0] re_adder_b_val ; 
logic re_adder_b_signed ; 
logic re_adder_ci ; 
logic [32:0] re_adder_res ; 
logic [32-1:0] re_adder_res_val ; 
logic re_adder_res_signed ; 
localparam ARITHMATIC_ADD = 0 ; 
localparam ARITHMATIC_SUB = 1 ; 
localparam ARITHMATIC_SLT = 2 ; 
localparam ARITHMATIC_MAX = 3 ; 
localparam ARITHMATIC_MIN = 4 ; 
localparam ARITHMATIC_SHADD_1 = 5 ; 
localparam ARITHMATIC_SHADD_2 = 6 ; 
assign arithmatic_op1_val = alu_inst_op1_val; 
assign arithmatic_op2_val = alu_inst_op2_val; 
assign adder_a_signed = 
(alu_inst_signed ? arithmatic_op1_val[32-1] : 1'b0); 
assign adder_b_signed = 
((|alu_inst_opcode[ARITHMATIC_MIN:ARITHMATIC_SUB]) ^ (alu_inst_signed ? arithmatic_op2_val[32-1] : 1'b0)); 
assign adder_b_val = 
((|alu_inst_opcode[ARITHMATIC_MIN:ARITHMATIC_SUB]) ? ~arithmatic_op2_val : arithmatic_op2_val); 
assign adder_res = {adder_a_signed,adder_a_val} + {adder_b_signed,adder_b_val} + adder_ci; 
assign adder_ci = 
(|alu_inst_opcode[ARITHMATIC_MIN:ARITHMATIC_SUB]); 
assign adder_res_val = adder_res[32-1:0]; 
assign adder_res_signed = adder_res[32]; 
assign adder_a_val = 
(arithmatic_op1_val << alu_inst_opcode[ARITHMATIC_SHADD_2 : ARITHMATIC_SHADD_1]); 
assign adder_max_val = adder_res_signed ? arithmatic_op2_val : arithmatic_op1_val; 
assign adder_min_val = adder_res_signed ? arithmatic_op1_val : arithmatic_op2_val; 
assign arithmatic_inst_res = ({(32){alu_inst_opcode[ARITHMATIC_SLT]}} & {{(32-1){1'b0}},adder_res_signed}) | 
({(32){alu_inst_opcode[ARITHMATIC_MAX]}} & adder_max_val) | 
({(32){alu_inst_opcode[ARITHMATIC_MIN]}} & adder_min_val) | 
({(32){alu_inst_opcode[ARITHMATIC_SUB] | alu_inst_opcode[ARITHMATIC_ADD]}} & adder_res_val); 
localparam SHIFT_AND = 0 ; 
localparam SHIFT_OR = 1 ; 
localparam SHIFT_XOR = 2 ; 
localparam SHIFT_LEFT = 3 ; 
localparam SHIFT_LOGICAL = 4 ; 
localparam SHIFT_ARITHMATIC = 5 ; 
localparam SHIFT_CYCLE = 6 ; 
assign shift_op1_val = alu_inst_op1_val ; 
assign shift_op2_val = alu_inst_shift_shamt ; 
assign shift_src_high = ({32{alu_inst_opcode[SHIFT_LEFT] | alu_inst_opcode[SHIFT_CYCLE]}} & shift_op1_val) | 
{32{(!alu_inst_opcode[SHIFT_LEFT] & alu_inst_opcode[SHIFT_ARITHMATIC]) & shift_op1_val[32-1]}} ; 
assign shift_src_low = {32{(!alu_inst_opcode[SHIFT_LEFT] | alu_inst_opcode[SHIFT_CYCLE])}} & shift_op1_val; 
assign shift_amount = alu_inst_opcode[SHIFT_LEFT] ? ({1'b0, ~shift_op2_val} + 1'b1) : {1'b0, shift_op2_val}; 
assign shift_res = {shift_src_high,shift_src_low} >> shift_amount; 
assign shift_inst_res = shift_res[32-1:0]; 
localparam LOGICAL_AND = 0 ; 
localparam LOGICAL_OR = 1 ; 
localparam LOGICAL_XOR = 2 ; 
localparam LOGICAL_XNOR = 3 ; 
localparam LOGICAL_OP2_NOT = 4 ; 
assign logical_op1_val = 
alu_inst_sub_funct[SHIFT_BIT] ? shift_inst_res : 
alu_logic_inst_op1_val ; 
assign logical_op2_val = alu_logic_inst_op2_val ; 
assign logical_p0 = (alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OR] & alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OP2_NOT] & !alu_inst_opcode[LOGICAL_OR] & !alu_inst_opcode[LOGICAL_AND]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_XNOR]) ; 
assign logical_p1 = (alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OR] & !alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[SHIFT_BIT] & alu_inst_opcode[LOGICAL_OR]) | 
( alu_inst_opcode[LOGICAL_XOR]) ; 
assign logical_p2 = (alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OR] & !alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[SHIFT_BIT ] & alu_inst_opcode[LOGICAL_OR]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OR] & alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OP2_NOT] & !alu_inst_opcode[LOGICAL_OR] & !alu_inst_opcode[LOGICAL_AND]) | 
( alu_inst_opcode[LOGICAL_XOR]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_AND] & alu_inst_opcode[LOGICAL_OP2_NOT]) ; 
assign logical_p3 = (alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_AND] & !alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OR] & alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_OR] & !alu_inst_opcode[LOGICAL_OP2_NOT]) | 
(alu_inst_sub_funct[SHIFT_BIT ] & alu_inst_opcode[LOGICAL_OR]) | 
(alu_inst_sub_funct[SHIFT_BIT ] & alu_inst_opcode[LOGICAL_AND]) | 
(alu_inst_sub_funct[LOGICAL_BIT] & alu_inst_opcode[LOGICAL_XNOR]) ; 
for (genvar i=0; i<32; i++)begin: BOOLEAN_LOGICAL_OPERATION 
assign logical_temp1_val[i] = logical_op2_val[i] ? logical_p1 : logical_p0; 
assign logical_temp2_val[i] = logical_op2_val[i] ? logical_p3 : logical_p2; 
assign logical_inst_res[i] = logical_op1_val[i] ? logical_temp2_val[i] : logical_temp1_val[i]; 
end 
localparam MISC_ORC_B = 0 ; 
localparam MISC_REV8 = 1 ; 
localparam MISC_ZERO_EQZ = 2 ; 
localparam MISC_ZERO_NEZ = 3 ; 
localparam MISC_MVSRC1_B = 4 ; 
localparam MISC_MVSRC1_H = 5 ; 
localparam MISC_REV8HW = 6 ; 
localparam MISC_REV8SLHW = 7 ; 
assign misc_op1_val = alu_inst_op1_val; 
assign misc_op2_val = alu_inst_op2_val; 
logic [3:0][32/4-1:0] misc_rev8_op_val; 
assign misc_rev8_op_val = misc_op1_val; 
assign misc_rev8hw_res = {misc_rev8_op_val[2], 
misc_rev8_op_val[3], 
misc_rev8_op_val[0], 
misc_rev8_op_val[1] }; 
assign misc_rev8slhw_res = {{(32/2){misc_op1_val[(32/4)-1]}} , 
misc_op1_val[(32/4)-1:0] , 
misc_op1_val[(32/2)-1:(32/4)]} ; 
assign misc_orcb_res = {{8{|misc_op1_val[31:24]}},{8{|misc_op1_val[23:16]}},{8{|misc_op1_val[15:8]}},{8{|misc_op1_val[7:0]}}}; 
assign misc_rev8_res = {misc_op1_val[7], misc_op1_val[6], misc_op1_val[5], misc_op1_val[4], misc_op1_val[3], misc_op1_val[2], misc_op1_val[1], misc_op1_val[0], 
misc_op1_val[15], misc_op1_val[14], misc_op1_val[13],misc_op1_val[12],misc_op1_val[11],misc_op1_val[10],misc_op1_val[9],misc_op1_val[8], 
misc_op1_val[23],misc_op1_val[22],misc_op1_val[21],misc_op1_val[20],misc_op1_val[19],misc_op1_val[18],misc_op1_val[17],misc_op1_val[16], 
misc_op1_val[31],misc_op1_val[30],misc_op1_val[29],misc_op1_val[28],misc_op1_val[27],misc_op1_val[26],misc_op1_val[25],misc_op1_val[24]}; 
assign misc_inst_res = ({32{alu_inst_opcode[MISC_MVSRC1_B]}} & {{(32-8){(alu_inst_signed & misc_op1_val[7])}}, misc_op1_val[7:0]}) | 
({32{alu_inst_opcode[MISC_MVSRC1_H]}} & {{(32-16){(alu_inst_signed & misc_op1_val[15])}},misc_op1_val[15:0]}) | 
({32{alu_inst_opcode[MISC_ORC_B]}} & misc_orcb_res) | 
({32{alu_inst_opcode[MISC_REV8]}} & misc_rev8_res) | 
({32{alu_inst_opcode[MISC_ZERO_EQZ]}} & (misc_op2_val == 'b0 ? {32{1'b0}} : misc_op1_val)) | 
({32{alu_inst_opcode[MISC_REV8HW]}} & (misc_rev8hw_res)) | 
({32{alu_inst_opcode[MISC_REV8SLHW]}} & (misc_rev8slhw_res)) | 
({32{alu_inst_opcode[MISC_ZERO_NEZ]}} & (misc_op2_val != 'b0 ? {32{1'b0}} : misc_op1_val)) ; 
assign alu_inst_res_val = ({32{alu_inst_sub_funct[ARITHMATIC_BIT]}} & arithmatic_inst_res) | 
({32{alu_inst_sub_funct[SHIFT_BIT] & !alu_inst_opcode[SHIFT_AND] & !alu_inst_opcode[SHIFT_OR] & !alu_inst_opcode[SHIFT_XOR]}} & shift_inst_res ) | 
({32{alu_inst_sub_funct[SHIFT_BIT] & (alu_inst_opcode[SHIFT_AND] | alu_inst_opcode[SHIFT_OR] | alu_inst_opcode[SHIFT_XOR])}} & logical_inst_res ) | 
({32{alu_inst_sub_funct[LOGICAL_BIT]}} & logical_inst_res ) | 
({32{alu_inst_sub_funct[MISC_BIT]}} & misc_inst_res ) ; 
assign alu_logic_inst_res_val = {32{alu_inst_sub_funct[LOGICAL_BIT]}} & logical_inst_res; 
generate 
if(SUPPORT_AGU == 1) begin:INDEPENDENT_AGU_GEN 
logic [32-1:0] agu_adder_a ; 
logic [32-1:0] agu_adder_b ; 
assign agu_adder_a = agu_inst_op1_val; 
assign agu_adder_b = agu_inst_op2_val; 
assign agu_inst_res_val = agu_adder_a + agu_adder_b; 
end 
else begin:SHARED_AGU_GEN 
assign agu_inst_res_val = {32{1'b0}}; 
end 
endgenerate 
generate 
if(SUPPORT_BRU == 1) begin:BRU_GEN 
assign bru_op1_val = bru_inst_op1_val; 
assign bru_op2_val = bru_inst_op2_val; 
assign bru_op3_val = {32{bru_inst_vld}} & bru_inst_op3_val; 
assign bru_cond_adder_a_val = bru_op1_val; 
assign bru_cond_adder_a_signed = bru_inst_signed ? bru_op1_val[32-1] : 1'b0; 
assign bru_cond_adder_b_val = bru_inst_sub_funct[1] ? ~bru_op2_val : bru_op2_val; 
assign bru_cond_adder_b_signed = bru_inst_sub_funct[1] ^ (bru_inst_signed ? bru_op2_val[32-1] : 1'b0); 
assign bru_cond_adder_ci = bru_inst_sub_funct[1]; 
assign bru_cond_adder_res = {bru_cond_adder_a_signed,bru_cond_adder_a_val} + {bru_cond_adder_b_signed,bru_cond_adder_b_val} + bru_cond_adder_ci; 
assign bru_cond_adder_res_signed = bru_cond_adder_res[32]; 
assign bru_cond_eq = (bru_inst_eq_op1_val == bru_inst_eq_op2_val) ; 
assign bru_cond_dir = (bru_inst_opcode[0] & bru_cond_eq) | 
(bru_inst_opcode[1] & !bru_cond_eq) | 
(bru_inst_opcode[2] & !bru_cond_adder_res_signed) | 
(bru_inst_opcode[3] & bru_cond_adder_res_signed) ; 
assign bru_tgt_adder_a_val = bru_inst_sub_funct[1] ? bru_inst_pc : bru_op1_val; 
assign bru_tgt_adder_b_val = bru_inst_sub_funct[1] ? bru_op3_val : bru_op2_val; 
assign bru_tgt_adder_res_tmp = bru_tgt_adder_a_val + bru_tgt_adder_b_val; 
assign bru_tgt_adder_res = {bru_tgt_adder_res_tmp[32-1:1],1'b0}; 
logic [32-1:0] a_comp; 
logic [32-1:0] b_comp; 
logic [32-1:0] k_comp; 
logic [32-1:0] unequal_que; 
wing_cbb_sum_compare #(.WIDTH(32)) wing_cbb_sum_compare ( 
.a_comp(a_comp), .b_comp(b_comp), .k_comp(k_comp), .unequal_que(unequal_que),.equal_re()); 
assign a_comp = bru_tgt_adder_a_val[32-1:0]; 
assign b_comp = bru_tgt_adder_b_val[32-1:0]; 
assign k_comp = bru_pred_tgt_pc; 
assign bru_tgt_mispred = (bru_inst_sub_funct[2:0] == 3'b100) ? 
(!bru_inst_is_ret ? 1'b1 : 
(bru_csr_sp_en ? (|unequal_que) : {1'b1}) 
): 1'b0; 
assign bru_seq_pc = bru_inst_pc + (bru_inst_type ? 3'h4 : 3'h2); 
assign bru_dir_mispred = (bru_inst_sub_funct[1] & 
(bru_csr_sp_en ? (bru_cond_dir != bru_inst_op3_val[32-1]) : (bru_tkn ? ({1'b1}) : ({1'b0}))) )| 
(bru_inst_sub_funct[0] & 
(bru_csr_sp_en ? {1'b0} : {1'b1}) ); 
assign bru_flush = bru_inst_vld & (bru_dir_mispred | bru_tgt_mispred | bru_inst_sub_funct[3] ) ; 
assign bru_flush_pc = 
((bru_inst_sub_funct[3:0] == 4'b0001) | 
(bru_inst_sub_funct[3:0] == 4'b0100) | 
((bru_inst_sub_funct[3:0] == 4'b0010) & (bru_csr_sp_en ? (!bru_inst_op3_val[32 - 1]) : (bru_cond_dir)))| 
(bru_inst_sub_funct[3:0] == 4'b1000) ) 
? bru_tgt_adder_res : bru_seq_pc ; 
assign bru_tgt_pc = bru_tkn ? bru_tgt_adder_res : bru_seq_pc; 
assign bru_tkn = bru_inst_vld & (bru_inst_sub_funct[0] | 
bru_inst_sub_funct[2] | 
bru_inst_sub_funct[3] | 
(bru_inst_sub_funct[1] & bru_cond_dir)); 
assign bru_inst_res_val = bru_seq_pc; 
end 
else begin:NO_BRU 
assign bru_inst_res_val = {32{1'b0}}; 
assign bru_flush = 1'b0; 
assign bru_flush_pc = {32{1'b0}}; 
assign bru_tgt_pc = {32{1'b0}}; 
end 
endgenerate 
assign re_adder_a_val = {32{1'b0}}; 
assign re_adder_a_signed = 1'b0; 
assign re_adder_b_val = {32{1'b0}}; 
assign re_adder_b_signed = 1'b0; 
assign re_adder_ci = 1'b0; 
assign re_adder_res = {(32 + 1){1'b0}}; 
assign re_adder_res_val = {32{1'b0}}; 
assign re_adder_res_signed = 1'b0; 
endmodule
 
 
module m130_exu_mdu #( 
parameter SUPPORT_MUL_APPROACH = 1 , 
parameter SUPPORT_DIV = 1 , 
parameter SUPPORT_DIV_APPROACH = 1 
)( 
input logic clk , 
input logic rst_n , 
input logic mdu_inst_vld , 
input logic mdu_inst_vld_q , 
input logic mdu_inst_flush , 
input logic [32-1:0] mdu_inst_op1_val , 
input logic [32-1:0] mdu_inst_op2_val , 
input logic [32-1:0] mdu_inst_op1_val_q , 
input logic mdu_inst_op1_signed , 
input logic mdu_inst_op2_signed , 
input logic div_ite_op1_neg_q , 
input logic [$clog2(32)-1:0] div_ite_op1_index , 
input logic [$clog2(32)-1:0] div_ite_op1_index_q , 
input logic [32-1:0] div_ite_op1_val_q , 
input logic [4-1:0] mdu_inst_sub_funct , 
input logic [4-1:0] mdu_sub_funct_wb , 
input logic [8-1:0] mdu_opcode_wb , 
output logic mdu_inst_res_stall , 
output logic [32-1:0] mdu_inst_res_val 
); 
localparam M130_CORE_XLEN_DW = 32*2 ; 
logic [32-1:0] mdu_inst_op2_val_q ; 
logic mdu_inst_op1_signed_q ; 
logic mdu_inst_op2_signed_q ; 
logic [32-1:0] mul_res ; 
logic mul_stall ; 
logic [$clog2(32)-1:0] mul_ite_cnt ; 
logic [$clog2(32)-1:0] mul_ite_cnt_q ; 
logic mul_ite_stall ; 
logic [32:0] mdu_sum ; 
logic [32:0] mdu_sum_q ; 
logic [32:0] mdu_pp2 ; 
logic [$clog2(32)-1:0] mul_ite_op2_index ; 
logic [$clog2(32)-1:0] mul_ite_op2_index_q ; 
logic [32-1:0] div_res ; 
logic div_stall ; 
logic [$clog2(32)-1:0] div_ite_cnt ; 
logic [$clog2(32)-1:0] div_ite_cnt_q ; 
logic div_ite_stall ; 
logic [32-1:0] div_ite_op2_val ; 
logic [32-1:0] div_ite_op2_sub_val ; 
logic [32-1:0] div_ite_op2_sub_val_q ; 
logic div_ite_op2_neg ; 
logic div_ite_op2_neg_q ; 
logic div_ite_d_signed ; 
logic [32-1:0] div_ite_sft_h ; 
logic [32-1:0] div_ite_sft_l ; 
logic [32-1:0] div_ite_sft_h_q ; 
logic [32-1:0] div_ite_sft_l_q ; 
logic [32:0] div_ite_sft_reg_h ; 
logic [32:0] div_ite_sft_reg_l ; 
logic [32:0] div_ite_sft_reg_h_q ; 
logic [32:0] div_ite_sft_reg_l_q ; 
logic [M130_CORE_XLEN_DW+1:0] div_ite_sft_reg_q ; 
logic [32-1:0] mdu_inst_src2_val ; 
logic [32-1:0] mdu_inst_src2_val_q ; 
logic [32-1:0] div_bypass_res_val ; 
logic [32-1:0] div_bypass_res_val_q ; 
logic div_bypass ; 
logic div_bypass_q ; 
logic [$clog2(32)-1:0] div_ite_op2_temp_index ; 
logic [32-1:0] re_adder_a_val ; 
logic re_adder_a_signed ; 
logic [32-1:0] re_adder_b_val ; 
logic re_adder_b_signed ; 
logic re_adder_ci ; 
logic [32 :0] re_adder_res ; 
logic [32-1:0] re_adder_res_val ; 
logic re_adder_res_signed ; 
logic [$clog2(32)-1:0] div_ite_op2_index ; 
logic [$clog2(32)-1:0] div_ite_op2_index_q ; 
logic [$clog2(32)-1:0] div_ite_op1_index_q_sel ; 
logic [$clog2(32)-1:0] div_ite_op2_index_q_sel ; 
if(SUPPORT_MUL_APPROACH == 1'b0) begin:INDEPENDENT_MUL_ITE_GEN 
logic [$clog2(32)-1:0] mdu_cnt_q ; 
logic [32-1:0] mdu_pp1 ; 
logic mul_ite_flag_set ; 
logic mul_ite_flag_clr ; 
logic mul_ite_flag_en ; 
logic mul_ite_flag ; 
logic mul_ite_flag_q ; 
`WDFFER(mul_ite_flag_q, mul_ite_flag, mul_ite_flag_en, clk, rst_n) 
assign mul_ite_cnt = (mdu_inst_vld_q & mdu_sub_funct_wb[0] & (!mdu_opcode_wb[2] & !mdu_opcode_wb[3])) ? {$clog2(32){1'b0}} : 
((mdu_sub_funct_wb[0] & (!mdu_opcode_wb[2] & !mdu_opcode_wb[3])) ? (mul_ite_cnt_q + 1'b1) : (mul_ite_cnt_q)); 
assign mul_ite_flag_set = mdu_inst_vld_q & mdu_sub_funct_wb[0] & (!mdu_opcode_wb[2] & !mdu_opcode_wb[3]); 
assign mul_ite_flag_clr = (mdu_inst_vld_q & (!mdu_opcode_wb[2] & !mdu_opcode_wb[3])) ? 1'b0 : ((mdu_opcode_wb[0] == 1'b0) ? 
(mul_ite_cnt == {$clog2(32){1'b1}}) : 
((mul_ite_op2_index_q == {$clog2(32){1'b1}}) ? (mul_ite_cnt == ({{($clog2(32)-1){1'b0}}, {1'b1}})) : 
(mul_ite_cnt == (~mul_ite_op2_index_q)))); 
assign mul_ite_flag_en = mul_ite_flag_set | mul_ite_flag_clr | mdu_inst_flush; 
assign mul_ite_flag = (mul_ite_flag_clr | mdu_inst_flush) ? 1'b0 : mul_ite_flag_set; 
assign mul_ite_stall = (mul_ite_flag_set | mul_ite_flag_q) & (~mul_ite_flag_clr); 
assign mul_stall = mul_ite_stall; 
assign mdu_pp1 = {32{mdu_inst_op2_val_q[mul_ite_cnt]}} & mdu_inst_op1_val_q; 
assign mdu_pp2 = (mdu_sub_funct_wb[0] & mdu_opcode_wb[0]) ? 
({{1'b0}, ({32{mdu_inst_op2_val_q[mul_ite_cnt]}} & (mdu_inst_op1_val_q << mul_ite_cnt))}) : 
(((mul_ite_cnt == {$clog2(32){1'b1}}) & (mdu_inst_op2_val_q[mul_ite_cnt] == 1'b1) & (mdu_inst_op2_signed_q == 1'b1)) ? 
(~{mdu_pp1[32-1], mdu_pp1} + 1'b1): 
((!mdu_inst_op1_signed_q) ? {{1'b0}, mdu_pp1} : 
({mdu_pp1[32-1], mdu_pp1}))); 
assign mdu_sum = re_adder_res; 
assign mul_res = (mdu_opcode_wb[0] ? mdu_sum[32-1:0] : 
((mdu_sum[32:1]))); 
assign re_adder_a_val = (mdu_inst_vld_q & (!mdu_opcode_wb[2] & !mdu_opcode_wb[3])) ? ({32{1'b0}}) : (mdu_opcode_wb[0] ? mdu_sum_q[32-1 : 0] : 
(mdu_sum_q[32:1])); 
assign re_adder_a_signed = (mdu_inst_vld_q & (!mdu_opcode_wb[2] & !mdu_opcode_wb[3])) ? (1'b0) : 
(mdu_inst_op1_signed_q ? ((mdu_opcode_wb[0] == 1'b0) ? mdu_sum_q[32] : 1'b0) : 
(1'b0)); 
assign re_adder_b_val = (mdu_pp2[32-1 : 0]); 
assign re_adder_b_signed = (mdu_inst_op1_signed_q & (mdu_opcode_wb[0] == 1'b0)) ? mdu_pp2[32] : 1'b0; 
assign re_adder_ci = 1'b0; 
assign re_adder_res = {re_adder_a_signed, re_adder_a_val} + {re_adder_b_signed,re_adder_b_val} + re_adder_ci; 
assign re_adder_res_val = re_adder_res[32-1 : 0]; 
assign re_adder_res_signed = re_adder_res[32]; 
end 
else begin:independent_mul_booth_gen 
logic [M130_CORE_XLEN_DW-1:0] mul_booth_res ; 
logic signed [32:0] mul_booth_op1_val ; 
logic signed [32:0] mul_booth_op2_val ; 
assign mul_booth_op1_val = mdu_inst_op1_signed_q ? {mdu_inst_op1_val_q[31],mdu_inst_op1_val_q} : {1'b0,mdu_inst_op1_val_q}; 
assign mul_booth_op2_val = mdu_inst_op2_signed_q ? {mdu_inst_op2_val_q[31],mdu_inst_op2_val_q} : {1'b0,mdu_inst_op2_val_q}; 
assign mul_booth_res = mul_booth_op1_val * mul_booth_op2_val; 
assign mul_res = mdu_opcode_wb[0] ? mul_booth_res[31:0] : mul_booth_res[63:32]; 
assign mul_stall = 1'b0; 
assign mdu_sum = {(32+1){1'b0}}; 
assign mul_ite_stall = 1'b0; 
assign mul_ite_cnt = {$clog2(32){1'b0}}; 
end 
logic clmul_ite_flag_set ; 
logic clmul_ite_flag_clr ; 
logic clmul_ite_flag_en ; 
logic clmul_ite_flag ; 
logic clmul_ite_flag_q ; 
logic clmul_ite_stall ; 
logic [32-1:0] clmul_ite_sum ; 
logic [32-1:0] clmul_ite_sum_q ; 
logic [32:0] clmul_ite_sum_reg ; 
logic [32:0] clmul_ite_sum_reg_q ; 
logic [32-1:0] clmul_ite_res ; 
logic [32-1:0] clmul_ite_sft1 ; 
logic [32-1:0] clmul_ite_sft1_q ; 
logic [32:0] clmul_ite_sft1_reg ; 
logic [32:0] clmul_ite_sft1_reg_q ; 
logic [$clog2(32)-1:0] clmul_ite_cnt ; 
logic [$clog2(32)-1:0] clmul_ite_cnt_q ; 
logic [32-1:0] clmul_ite_pp1 ; 
`WDFFER(clmul_ite_flag_q, clmul_ite_flag, clmul_ite_flag_en, clk, rst_n) 
assign clmul_ite_flag_set = mdu_inst_vld_q & mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3])); 
assign clmul_ite_flag_clr = ((mdu_inst_vld_q & mdu_sub_funct_wb[0]) ? 1'b0 : (clmul_ite_cnt == {$clog2(32){1'b1}})); 
assign clmul_ite_flag_en = clmul_ite_flag_set | clmul_ite_flag_clr | mdu_inst_flush; 
assign clmul_ite_flag = (clmul_ite_flag_clr | mdu_inst_flush) ? 1'b0 : clmul_ite_flag_set; 
assign clmul_ite_stall = (clmul_ite_flag_set | clmul_ite_flag_q) & (~clmul_ite_flag_clr); 
assign clmul_ite_sft1 = (mdu_inst_vld_q & mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3]))) ? mdu_inst_op1_val_q : 
(((!mdu_opcode_wb[0]) & mdu_opcode_wb[2]) ? (clmul_ite_sft1_q << 1) : 
(clmul_ite_sft1_q)); 
assign clmul_ite_cnt = (mdu_inst_vld_q & mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3]))) ? {$clog2(32){1'b0}} : 
((mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3]))) ? (clmul_ite_cnt_q + 1'b1) : (clmul_ite_cnt_q)); 
assign clmul_ite_pp1 = (mdu_inst_op2_val_q[clmul_ite_cnt] == 1'b1) ? clmul_ite_sft1 : {32{1'b0}}; 
assign clmul_ite_sum = (mdu_inst_vld_q & mdu_sub_funct_wb[0]) ? clmul_ite_pp1 : 
(((mdu_opcode_wb[2] & mdu_opcode_wb[0]) | (mdu_opcode_wb[3])) ? ({1'b0, clmul_ite_sum_q[32-1 : 1]} ^ clmul_ite_pp1) : 
(clmul_ite_sum_q ^ clmul_ite_pp1)); 
assign clmul_ite_res = (mdu_opcode_wb[2] & mdu_opcode_wb[0]) ? (clmul_ite_sum >> 1) : (clmul_ite_sum); 
assign clmul_ite_sum_reg = {{1'b0}, clmul_ite_sum}; 
assign clmul_ite_sum_q = clmul_ite_sum_reg_q[32-1:0]; 
assign clmul_ite_sft1_reg = {{1'b0}, clmul_ite_sft1}; 
assign clmul_ite_sft1_q = clmul_ite_sft1_reg_q[32-1:0]; 
`WDFFER(mdu_inst_src2_val_q, mdu_inst_src2_val, mdu_inst_vld, clk, rst_n) 
`WDFFER(mdu_inst_op1_signed_q, mdu_inst_op1_signed, mdu_inst_vld, clk, rst_n) 
`WDFFER(mdu_inst_op2_signed_q, mdu_inst_op2_signed, mdu_inst_vld, clk, rst_n) 
`WDFFER(div_ite_op2_neg_q, div_ite_op2_neg, mdu_inst_vld, clk, rst_n) 
`WDFFER(mul_ite_op2_index_q, mul_ite_op2_index, mdu_inst_vld, clk, rst_n) 
`WDFFER(div_ite_op2_index_q, div_ite_op2_index, mdu_inst_vld, clk, rst_n) 
`WDFFER(div_bypass_q, div_bypass, mdu_inst_vld, clk, rst_n) 
if(SUPPORT_MUL_APPROACH == 1'b0) begin:INDEPENDENT_MUL_ITE_LZC_GEN 
wing_cbb_lzc #( 
.DIRECTION (1), 
.WIDTH (32 ), 
.IDX_W ($clog2(32)) 
) mdu_lzc( 
.vec_i (mdu_inst_op2_val ), 
.cnt_o (mul_ite_op2_index ), 
.empty_o ( ) 
); 
end 
else begin:independent_mul_booth_lzc_gen 
assign mul_ite_op2_index = {$clog2(32){1'b0}}; 
end 
if((SUPPORT_DIV == 1) & (SUPPORT_DIV_APPROACH == 1'b0)) begin:ITERATION_DIV 
wing_cbb_lzc #( 
.DIRECTION (1 ), 
.WIDTH (32 ), 
.IDX_W ($clog2(32) ) 
) mdu_lzc( 
.vec_i (div_ite_op2_val ), 
.cnt_o (div_ite_op2_temp_index), 
.empty_o ( ) 
); 
assign div_ite_op2_neg = mdu_inst_op2_signed & mdu_inst_op2_val[32-1]; 
assign div_ite_op2_sub_val = div_ite_op2_neg ? (mdu_inst_op2_val) : (~mdu_inst_op2_val + 1'b1); 
assign div_ite_op2_val = div_ite_op2_neg ? (~mdu_inst_op2_val + 1'b1) : (mdu_inst_op2_val); 
assign div_ite_op2_index = div_ite_op2_temp_index; 
assign mdu_inst_src2_val = ({32{(mdu_inst_vld & mdu_inst_sub_funct[0])}} & mdu_inst_op2_val) | 
({32{(mdu_inst_vld & div_bypass)}} & div_bypass_res_val) | 
({32{(mdu_inst_vld & (~div_bypass) & (mdu_inst_sub_funct[1] | mdu_inst_sub_funct[2]))}} & div_ite_op2_sub_val); 
assign mdu_inst_op2_val_q = mdu_inst_src2_val_q; 
assign div_bypass_res_val_q = mdu_inst_src2_val_q; 
assign div_ite_op2_sub_val_q= mdu_inst_src2_val_q; 
end else if (SUPPORT_DIV == 1)begin: SRT_DIV 
assign div_ite_op2_neg = mdu_inst_op2_signed & mdu_inst_op2_val[32-1]; 
assign div_ite_op2_index = {32{1'b0}}; 
assign mdu_inst_src2_val = ({32{(mdu_inst_vld & mdu_inst_sub_funct[0])}} & mdu_inst_op2_val) | 
({32{div_bypass}} & div_bypass_res_val) ; 
assign mdu_inst_op2_val_q = mdu_inst_src2_val_q; 
assign div_bypass_res_val_q = mdu_inst_src2_val_q; 
end else begin: NO_DIV 
assign div_ite_op2_neg = {1'b0}; 
assign div_ite_op2_index = {32{1'b0}}; 
assign mdu_inst_src2_val = ({32{(mdu_inst_vld & (~div_bypass))}} & mdu_inst_op2_val) ; 
assign mdu_inst_op2_val_q = mdu_inst_src2_val_q; 
end 
if(SUPPORT_DIV == 1) begin: independent_div_gen 
logic [32-1:0] div_bypass_div_s ; 
logic [32-1:0] div_bypass_div_u ; 
logic [32-1:0] div_bypass_rem_s ; 
logic [32-1:0] div_bypass_rem_u ; 
logic div_op2_z ; 
logic div_op1_z ; 
logic div_op_equal ; 
logic div_overflow ; 
logic [32-1:0] div_inst_div_s ; 
logic [32-1:0] div_inst_div_u ; 
logic [32-1:0] div_inst_rem_s ; 
logic [32-1:0] div_inst_rem_u ; 
logic [32-1:0] div_srt_div_s ; 
logic [32-1:0] div_srt_div_u ; 
logic [32-1:0] div_srt_rem_s ; 
logic [32-1:0] div_srt_rem_u ; 
logic [32-1:0] div_s ; 
logic [32-1:0] div_u ; 
logic [32-1:0] rem_s ; 
logic [32-1:0] rem_u ; 
logic div_by_big ; 
logic div_by_big_wb ; 
logic [32-1:0] div_by_big_res_val_q ; 
assign div_op2_z = mdu_inst_op2_val == '0; 
assign div_op1_z = mdu_inst_op1_val == {32{1'b0}}; 
assign div_op_equal = mdu_inst_op1_val == mdu_inst_op2_val; 
assign div_overflow = mdu_inst_op1_signed & ({~mdu_inst_op1_val[32-1],mdu_inst_op1_val[32-2:0]} == {32{1'b0}}) 
& (mdu_inst_op2_val == {32{1'b1}}); 
assign div_bypass = (mdu_inst_vld & (mdu_inst_sub_funct[1] | mdu_inst_sub_funct[2]) & (div_op2_z | div_op1_z | div_op_equal | div_overflow)); 
assign div_bypass_div_s = div_op2_z ? {32{1'b1}} : 
div_op1_z ? {32{1'b0}} : 
div_op_equal ? 'd1 : 
div_overflow ? {1'b1,{(32-1){1'b0}}} : 
'0 ; 
assign div_bypass_div_u = div_op2_z ? {32{1'b1}} : 
div_op1_z ? {32{1'b0}} : 
div_op_equal ? 'd1 : 
'0 ; 
assign div_bypass_rem_s = div_op2_z ? mdu_inst_op1_val : 
(div_overflow) ? {32{1'b0}} : 
div_op1_z ? {32{1'b0}} : 
div_op_equal ? {32{1'b0}} : 
mdu_inst_op1_val ; 
assign div_bypass_rem_u = div_op2_z ? mdu_inst_op1_val : 
div_op1_z ? {32{1'b0}} : 
div_op_equal ? {32{1'b0}} : 
mdu_inst_op1_val ; 
assign div_bypass_res_val = ({32{(mdu_inst_sub_funct[1] & mdu_inst_op1_signed)}} & div_bypass_div_s) | 
({32{(mdu_inst_sub_funct[1] & (~mdu_inst_op1_signed))}} & div_bypass_div_u) | 
({32{(mdu_inst_sub_funct[2] & mdu_inst_op1_signed)}} & div_bypass_rem_s) | 
({32{(mdu_inst_sub_funct[2] & (~mdu_inst_op1_signed))}} & div_bypass_rem_u) ; 
if(SUPPORT_DIV_APPROACH == 1'b0) begin: independent_div_ite_gen 
logic [32-1:0] div_ite_s_q ; 
logic div_ite_flag_set ; 
logic div_ite_flag_clr ; 
logic div_ite_flag_en ; 
logic div_ite_flag ; 
logic div_ite_flag_q ; 
logic [32-1:0] div_ite_quotient ; 
logic [32-1:0] div_s_result ; 
logic [M130_CORE_XLEN_DW-1:0] div_ite_sft ; 
logic [M130_CORE_XLEN_DW-1:0] div_ite_sft_q ; 
logic [M130_CORE_XLEN_DW-1:0] div_ite_ini ; 
logic [32-1:0] div_ite_restore ; 
logic [32:0] div_ite_sub ; 
logic div_ite_q_digit ; 
logic [32-1:0] div_s_neg_result ; 
logic [$clog2(32)+1:0] div_ini_presft ; 
assign div_by_big_wb = (div_ite_op2_index_q < div_ite_op1_index_q); 
assign div_ite_op1_index_q_sel = 
div_ite_op1_index_q; 
assign div_ite_op2_index_q_sel = 
div_ite_op2_index_q; 
assign div_ini_presft = ({$clog2(32){1'b1}} + {1'b1} - div_ite_op2_index_q_sel + div_ite_op1_index_q_sel); 
assign div_ite_ini = {{32{1'b0}}, div_ite_op1_val_q} << div_ini_presft; 
assign div_ite_sub = ((mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? ({{1'b0}, div_ite_ini[M130_CORE_XLEN_DW-1:32]}) : 
{{1'b0}, div_ite_sft_q[M130_CORE_XLEN_DW-1:32]}) + {{1'b1}, div_ite_op2_sub_val_q}; 
assign div_ite_restore = div_ite_sub[32] ? 
((mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? (div_ite_ini[M130_CORE_XLEN_DW-1:32]) : 
div_ite_sft_q[M130_CORE_XLEN_DW-1:32]) : 
div_ite_sub[32-1:0]; 
assign div_ite_q_digit = div_ite_sub[32] ? 1'b0 : 1'b1; 
assign div_ite_sft = (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? 
({div_ite_restore[32-2:0], div_ite_ini[32-1:0], div_ite_q_digit}) : 
({div_ite_restore[32-2:0], div_ite_sft_q[32-1:0], div_ite_q_digit}); 
assign div_ite_flag_set = (div_bypass_q) ? 1'b0 : (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])); 
assign div_ite_flag_clr = (div_ite_cnt == {$clog2(32){1'b1}}); 
assign div_ite_flag_en = div_ite_flag_set | div_ite_flag_clr | mdu_inst_flush; 
assign div_ite_flag = (div_ite_flag_clr | mdu_inst_flush)? 1'b0 : div_ite_flag_set; 
assign div_ite_stall = (div_ite_flag_set | div_ite_flag_q) & (~div_ite_flag_clr); 
assign div_ite_cnt = (div_bypass_q) ? ('0) : (((mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2]))) ? 
({$clog2(32){1'b1}} + div_ite_op1_index_q_sel - div_ite_op2_index_q_sel) : 
(((mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? (div_ite_cnt_q + 1'b1) : (div_ite_cnt_q))); 
`WDFFER(div_ite_flag_q,div_ite_flag,div_ite_flag_en,clk,rst_n) 
assign div_ite_sft_h = div_ite_sft[M130_CORE_XLEN_DW-1:32]; 
assign div_ite_sft_l = div_ite_sft[32-1:0]; 
assign div_ite_sft_reg_h = {{2'd0}, div_ite_sft_h[32-1:1]}; 
assign div_ite_sft_reg_l = {div_ite_sft_h[0], div_ite_sft_l}; 
assign div_ite_sft_reg_q = {div_ite_sft_reg_h_q, div_ite_sft_reg_l_q}; 
assign div_ite_sft_q = div_ite_sft_reg_q[M130_CORE_XLEN_DW-1:0]; 
assign div_s_result = div_ite_restore; 
assign div_s_neg_result = (~div_s_result + 1'b1); 
assign div_inst_div_s = (div_ite_op1_neg_q ^ div_ite_op2_neg_q) ? (~div_ite_sft[32-1:0] + 1'b1) : div_ite_sft[32-1:0]; 
assign div_inst_div_u = div_ite_sft[32-1:0]; 
assign div_inst_rem_s = (div_ite_op1_neg_q) ? div_s_neg_result : div_s_result; 
assign div_inst_rem_u = div_s_result; 
end else begin: independent_div_srt_gen 
logic [32-1:0] divisor; 
logic [32-1:0] dividend; 
logic [32-1:0] quotient; 
logic [32-1:0] remainder; 
logic div_ite_flag_set; 
logic div_ite_flag_clr; 
logic div_ite_flag_en ; 
logic div_ite_flag; 
logic div_ite_flag_q; 
logic [2:0] srt_q_digit; 
logic [32-1:0] srt_otf_a_q; 
logic [32-1:0] srt_otf_b_q; 
logic [32-1:0] srt_otf_a; 
logic [32-1:0] srt_otf_b; 
logic [32-1:0] srt_quotient_result; 
logic [32-1:0] srt_otf_result_q; 
logic [32-1:0] srt_otf_result; 
logic [32-1:0] shifted_dividend; 
logic [32-1:0] shifted_divisor; 
logic [32-1:0] size_dividend; 
logic [32-1:0] size_divisor; 
logic [32-1:0] number_shift_dividend; 
logic [32-1:0] number_shift_temp_dividend; 
logic [32-1:0] number_shift_temp_divisor; 
logic [32-1:0] number_shift_divisor; 
logic number_shift_dividend_odd; 
logic number_shift_divisor_odd; 
logic shift_equal; 
logic [32-1:0] normalized_dividend; 
logic [32-1:0] normalized_divisor; 
logic [6:0] r_approximate; 
logic [4:0] d_approximate; 
logic [32-1:0] quotient_temp; 
logic [2:0] q_digit; 
logic [32-1:0] iteration_num; 
logic [32-1:0] neg_qD; 
logic [32-1:0] sum; 
logic [32-1:0] carry; 
logic [32-1:0] sum_q; 
logic [32-1:0] carry_q; 
logic quotient_temp_vld; 
logic [32-1:0] quotient_final; 
logic [32-1:0] quotient_res_val; 
logic [32-1:0] real_remainder_2; 
logic [32-1:0] real_remainder_1_q; 
logic [32-1:0] real_remainder_2_q; 
logic [32-1:0] real_remainder; 
logic [32-1:0] sum_shift; 
logic [32-1:0] sum_shift_q; 
logic [32-1:0] carry_shift; 
logic [32-1:0] carry_shift_q; 
assign div_by_big_wb = 1'b0; 
assign div_ite_flag_set = div_bypass_q ? 1'b0 : 
(mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2]) & (div_ite_op1_index_q != {$clog2(32){1'b1}})); 
assign div_ite_flag_clr = (div_ite_cnt == iteration_num + 1'b1); 
assign div_ite_flag_en = div_ite_flag_set | div_ite_flag_clr | mdu_inst_flush; 
assign div_ite_flag = (div_ite_flag_clr | mdu_inst_flush)? 1'b0 : div_ite_flag_set; 
assign div_ite_stall = (div_ite_flag_set | div_ite_flag_q) & (~div_ite_flag_clr); 
assign div_ite_cnt = (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? div_ite_op1_index_q : (div_ite_cnt_q + 1'b1); 
`WDFFER(div_ite_flag_q, div_ite_flag, div_ite_flag_en, clk, rst_n) 
assign srt_otf_a = (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? {32{1'b0}} : 
(~q_digit[2] ? {srt_otf_a_q[32-3:0], q_digit[1:0]} : {srt_otf_b_q[32-3:0], q_digit[1:0]}); 
assign srt_otf_b = (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? {32{1'b0}} : 
(~q_digit[2] & (|q_digit[1:0]) ? {srt_otf_a_q[32-3:0], q_digit[2:1]} : {srt_otf_b_q[32-3:0], {(q_digit[1]^~q_digit[0]), ~q_digit[0]}}); 
`WDFFER(srt_otf_result_q, srt_otf_result, div_ite_stall, clk, rst_n) 
`WDFFER(srt_otf_a_q, srt_otf_a, div_ite_stall, clk, rst_n) 
`WDFFER(srt_otf_b_q, srt_otf_b, div_ite_stall, clk, rst_n) 
assign srt_otf_result = (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? ({32{1'b0}}) : 
((div_ite_cnt == iteration_num - 1'b1) ? ((real_remainder[32-1]) ? (srt_otf_a-1'b1) : srt_otf_a) : 
((div_ite_cnt == iteration_num) ? (srt_otf_result_q) : (srt_otf_a))); 
shift_size shift_size_dividend 
(.in_number(mdu_inst_op1_val_q), .out_number(shifted_dividend), .out_number_size(size_dividend), .out_number_shift(number_shift_temp_dividend)); 
shift_size shift_size_divisor 
(.in_number(mdu_inst_op2_val_q), .out_number(shifted_divisor), .out_number_size(size_divisor), .out_number_shift(number_shift_temp_divisor)); 
assign number_shift_dividend_odd = number_shift_temp_dividend[0]; 
assign number_shift_divisor_odd = number_shift_divisor[0]; 
assign shift_equal = ~((number_shift_dividend_odd) ^ (number_shift_divisor_odd)); 
assign number_shift_dividend = number_shift_temp_dividend - (shift_equal ? 5'd6 : 5'd5); 
assign number_shift_divisor = number_shift_temp_divisor - 3'd3; 
assign normalized_dividend = shift_equal ? {6'b00_0000, shifted_dividend[32-1 : 6]} : 
{5'b0_0000, shifted_dividend[32-1 : 5]}; 
assign normalized_divisor = shifted_divisor[32-1 : 3]; 
assign iteration_num = (number_shift_divisor - number_shift_dividend) / 2 + 1'b1; 
qds qds_inst (.r_approximate(r_approximate), .d_approximate(d_approximate), .q_digit(q_digit)); 
assign real_remainder = (carry_shift + sum_shift); 
assign real_remainder_2 = (carry_shift + sum_shift + normalized_divisor) ; 
assign r_approximate = (div_ite_cnt == {$clog2(32){1'b0}}) ? (normalized_dividend[32-1 -: 7]) : 
(sum_shift_q[32-1 -:7] + carry_shift_q[32-1 -:7]); 
assign d_approximate = shifted_divisor[32-1 -: 5]; 
assign quotient_temp = (mdu_inst_vld_q & (mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])) ? {32{1'b0}} : {quotient_temp[0 +: 32-4], q_digit}; 
assign neg_qD = (quotient_temp[2:0] == 3'b010) ? ((~normalized_divisor) << 1) : ( 
(quotient_temp[2:0] == 3'b001) ? (~normalized_divisor) : ( 
(quotient_temp[2:0] == 3'b000) ? ({32{1'b0}}) : ( 
(quotient_temp[2:0] == 3'b111) ? (normalized_divisor) : (normalized_divisor << 1)))); 
assign carry = 
(div_ite_cnt == {($clog2(32)-1){1'b0}}) ? 
(((quotient_temp[2:0] == 3'b001) | (quotient_temp[2:0] == 3'b010)) ? 
((((normalized_dividend & {32{1'b0}}) | ({32{1'b0}} & neg_qD) | (neg_qD & normalized_dividend)) << 1) + 1'b1) : 
(((normalized_dividend & {32{1'b0}}) | ({32{1'b0}} & neg_qD) | (neg_qD & normalized_dividend)) << 1)) : 
(((quotient_temp[2:0] == 3'b001) | (quotient_temp[2:0] == 3'b010)) ? 
((((sum_shift_q & carry_shift_q) | (carry_shift_q & neg_qD) | (neg_qD & sum_shift_q)) << 1) + 1'b1) : 
(((sum_shift_q & carry_shift_q) | (carry_shift_q & neg_qD) | (neg_qD & sum_shift_q)) << 1)); 
assign sum = (div_ite_cnt == {$clog2(32){1'b0}}) ? (normalized_dividend ^ {32{1'b0}} ^ neg_qD) : (sum_shift_q ^ carry_shift_q ^ neg_qD); 
assign sum_shift = (mdu_inst_vld ) ? {32{1'b0}} : (sum << 2); 
assign carry_shift = (mdu_inst_vld) ? ({32{1'b0}}) : (carry << 2); 
`WDFFER(sum_q, sum, div_ite_stall, clk, rst_n) 
`WDFFER(carry_q, carry, div_ite_stall, clk, rst_n) 
`WDFFER(sum_shift_q, sum_shift, div_ite_stall, clk, rst_n) 
`WDFFER(carry_shift_q, carry_shift, div_ite_stall, clk, rst_n) 
assign div_inst_div_s = div_bypass_q ? 1'b0 : srt_otf_result_q; 
assign div_inst_div_u = div_bypass_q ? 1'b0 : srt_otf_result_q; 
assign div_inst_rem_s = div_bypass_q ? 1'b0 : r_approximate[6] ? real_remainder_2 : real_remainder; 
assign div_inst_rem_u = div_bypass_q ? 1'b0 : r_approximate[6] ? real_remainder_2 : real_remainder; 
assign div_ite_sft_reg_h = '0; 
assign div_ite_sft_reg_l = '0; 
end 
assign div_by_big_res_val_q = ({32{(mdu_sub_funct_wb[1] & mdu_inst_op1_signed_q)}} & '0) | 
({32{(mdu_sub_funct_wb[1] & (~mdu_inst_op1_signed_q))}} & '0) | 
({32{(mdu_sub_funct_wb[2] & mdu_inst_op1_signed_q)}} & ((div_ite_op1_neg_q) ? ~div_ite_op1_val_q+ 1'b1 : div_ite_op1_val_q)) | 
({32{(mdu_sub_funct_wb[2] & (~mdu_inst_op1_signed_q))}} & div_ite_op1_val_q) ; 
assign div_s = {32{mdu_sub_funct_wb[1]}} & div_inst_div_s; 
assign div_u = {32{mdu_sub_funct_wb[1]}} & div_inst_div_u; 
assign rem_s = {32{mdu_sub_funct_wb[2]}} & div_inst_rem_s; 
assign rem_u = {32{mdu_sub_funct_wb[2]}} & div_inst_rem_u; 
assign div_res = (div_bypass_q) ? div_bypass_res_val_q : 
div_by_big_wb ? div_by_big_res_val_q : 
mdu_inst_op1_signed_q ? (div_s | rem_s) : 
(div_u | rem_u) ; 
assign div_stall = div_ite_stall; 
end else begin:shared_div_gen 
assign div_res = {32{1'b0}}; 
assign div_stall = {1'b0}; 
assign div_ite_sft_reg_h = {(32+1){1'b0}}; 
assign div_ite_sft_reg_l = {(32+1){1'b0}}; 
assign div_ite_stall = {1'b0}; 
assign div_ite_cnt = {$clog2(32){1'b0}}; 
assign div_bypass = {1'b0}; 
end 
logic [32:0] mdu_wb_buf0 ; 
logic [32:0] mdu_wb_buf1 ; 
logic [$clog2(32)-1:0] mdu_wb_cnt_buf ; 
logic [32:0] mdu_wb_buf0_q ; 
logic [32:0] mdu_wb_buf1_q ; 
logic [$clog2(32)-1:0] mdu_wb_cnt_buf_q ; 
logic mdu_wb_buf0_en ; 
logic mdu_wb_buf1_en ; 
logic mdu_wb_cnt_buf_en ; 
assign mdu_wb_cnt_buf = mdu_sub_funct_wb[0] ? ((mdu_opcode_wb[2] | mdu_opcode_wb[3]) ? clmul_ite_cnt : mul_ite_cnt) : div_ite_cnt; 
assign mdu_wb_buf0 = mdu_sub_funct_wb[0] ? ((mdu_opcode_wb[2] | mdu_opcode_wb[3]) ? clmul_ite_sft1_reg : mdu_sum) : div_ite_sft_reg_h; 
assign mdu_wb_buf1 = (mdu_sub_funct_wb[0] & (mdu_opcode_wb[2] | mdu_opcode_wb[3])) ? clmul_ite_sum_reg : div_ite_sft_reg_l; 
assign mdu_wb_buf0_en = mdu_sub_funct_wb[0] ? ((mdu_opcode_wb[2] | mdu_opcode_wb[3]) ? clmul_ite_stall : mul_ite_stall) : div_ite_stall; 
assign mdu_wb_buf1_en = mdu_sub_funct_wb[0] ? ((mdu_opcode_wb[2] | mdu_opcode_wb[3]) ? clmul_ite_stall : mul_ite_stall) : div_ite_stall; 
assign mdu_wb_cnt_buf_en = mdu_sub_funct_wb[0] ? ((mdu_opcode_wb[2] | mdu_opcode_wb[3]) ? clmul_ite_stall : mul_ite_stall) : div_ite_stall; 
assign mdu_sum_q = {(32+1){mdu_sub_funct_wb[0]}} & mdu_wb_buf0_q; 
assign mul_ite_cnt_q = {$clog2(32) {mdu_sub_funct_wb[0] & (!mdu_opcode_wb[2]) & (!mdu_opcode_wb[3])}} & mdu_wb_cnt_buf_q; 
assign div_ite_sft_reg_h_q = {(32+1){mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2]}} & mdu_wb_buf0_q; 
assign div_ite_sft_reg_l_q = {(32+1){mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2]}} & mdu_wb_buf1_q; 
assign div_ite_cnt_q = {$clog2(32) {mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2]}} & mdu_wb_cnt_buf_q; 
assign clmul_ite_cnt_q = {$clog2(32) {mdu_sub_funct_wb[0] & (mdu_opcode_wb[2] | mdu_opcode_wb[3])}} & mdu_wb_cnt_buf_q; 
assign clmul_ite_sft1_reg_q = {(32+1){mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3]))}} & mdu_wb_buf0_q; 
assign clmul_ite_sum_reg_q = {(32+1){mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3]))}} & mdu_wb_buf1_q; 
`WDFFER(mdu_wb_buf0_q,mdu_wb_buf0,mdu_wb_buf0_en,clk,rst_n) 
`WDFFER(mdu_wb_buf1_q,mdu_wb_buf1,mdu_wb_buf1_en,clk,rst_n) 
`WDFFER(mdu_wb_cnt_buf_q,mdu_wb_cnt_buf,mdu_wb_cnt_buf_en,clk,rst_n) 
assign mdu_inst_res_val = ({32{mdu_sub_funct_wb[0] & (!(mdu_opcode_wb[2] | mdu_opcode_wb[3]))}} & mul_res) | 
({32{(mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2])}} & div_res) | 
({32{mdu_sub_funct_wb[0] & (mdu_opcode_wb[2] | mdu_opcode_wb[3])}} & clmul_ite_res) ; 
assign mdu_inst_res_stall = 
(mdu_sub_funct_wb[0] & (!(mdu_opcode_wb[2] | mdu_opcode_wb[3])) & mul_stall) | 
((mdu_sub_funct_wb[1] | mdu_sub_funct_wb[2]) & div_stall) | 
(mdu_sub_funct_wb[0] & ((mdu_opcode_wb[2] | mdu_opcode_wb[3])) & clmul_ite_stall); 
endmodule
 
 
module m130_exu_bmu #( 
parameter FINE_GRAINED_DATA_GATING = 3'b0 
)( 
input logic bmu_inst_vld , 
input logic [32-1:0] bmu_inst_op1_val , 
input logic [32-1:0] bmu_inst_op2_val , 
input logic [4-1:0] bmu_inst_sub_funct , 
input logic [8-1:0] bmu_inst_opcode , 
input logic bmu_inst_signed , 
output logic [32-1:0] bmu_inst_res_val 
); 
localparam SINGLEB_BIT = 0 ; 
localparam COUNTB_BIT = 1 ; 
logic [32-1:0] singleb_op1_val ; 
logic [$clog2(32)-1:0] singleb_op2_val ; 
logic [32-1:0] singleb_op2_onehot_val ; 
logic [32-1:0] singleb_rev8hw_res ; 
logic [32-1:0] singleb_rev8slhw_res ; 
logic [32-1:0] singleb_inst_res ; 
logic [32-1:0] countb_op1_val ; 
logic [32-1:0] countb_lzd_val ; 
logic [$clog2(32)-1:0] countb_h2l_binary_val ; 
logic [$clog2(32):0] countb_cpop_res ; 
logic [32-1:0] countb_inst_res ; 
logic countb_h2l_binary_val_empty ; 
localparam CLEAR = 0 ; 
localparam SET = 1 ; 
localparam INVERT = 2 ; 
localparam EXTRACT = 3 ; 
assign singleb_op1_val = bmu_inst_op1_val; 
assign singleb_op2_val = bmu_inst_op2_val[$clog2(32)-1:0]; 
`WCBB_BIN2ONEHOT(singleb_op2_val, singleb_op2_onehot_val) 
assign singleb_inst_res = bmu_inst_opcode[CLEAR] ? (singleb_op1_val & (~singleb_op2_onehot_val)) : 
bmu_inst_opcode[SET] ? (singleb_op1_val | singleb_op2_onehot_val) : 
bmu_inst_opcode[INVERT] ? (singleb_op1_val ^ singleb_op2_onehot_val) : 
bmu_inst_opcode[EXTRACT] ? ({{{32-1}{1'b0}},{|(singleb_op1_val & singleb_op2_onehot_val)}}) : 
{32{1'b0}} ; 
localparam HIGH_LOW = 0 ; 
localparam Z_COUNT1 = 1 ; 
assign countb_op1_val = bmu_inst_op1_val; 
assign countb_lzd_val = bmu_inst_opcode[HIGH_LOW] ? countb_op1_val : 
{countb_op1_val[0], countb_op1_val[1], countb_op1_val[2], countb_op1_val[3], 
countb_op1_val[4], countb_op1_val[5], countb_op1_val[6], countb_op1_val[7], 
countb_op1_val[8], countb_op1_val[9], countb_op1_val[10], countb_op1_val[11], 
countb_op1_val[12], countb_op1_val[13], countb_op1_val[14], countb_op1_val[15], 
countb_op1_val[16], countb_op1_val[17], countb_op1_val[18], countb_op1_val[19], 
countb_op1_val[20], countb_op1_val[21], countb_op1_val[22], countb_op1_val[23], 
countb_op1_val[24], countb_op1_val[25], countb_op1_val[26], countb_op1_val[27], 
countb_op1_val[28], countb_op1_val[29], countb_op1_val[30], countb_op1_val[31]}; 
wing_cbb_lzc #( 
.DIRECTION (1), 
.WIDTH (32), 
.IDX_W ($clog2(32)) 
) u_lzc_high2low( 
.vec_i (countb_lzd_val), 
.cnt_o (countb_h2l_binary_val), 
.empty_o (countb_h2l_binary_val_empty) 
); 
integer i; 
always @(*) begin 
countb_cpop_res = 'b0; 
for (i = 0; i<32; i= i+1) begin 
countb_cpop_res = countb_cpop_res + countb_op1_val[i]; 
end 
end 
assign countb_inst_res = bmu_inst_opcode[Z_COUNT1] ? ({{(32-$clog2(32)-1){1'b0}}, countb_cpop_res}) : 
(countb_h2l_binary_val_empty ? {{(32-$clog2(32)-1){1'b0}},{1'b1},{($clog2(32)){1'b0}}}: 
{{(32-$clog2(32)){1'b0}}, countb_h2l_binary_val}); 
assign bmu_inst_res_val = ({32{bmu_inst_sub_funct[SINGLEB_BIT]}} & singleb_inst_res) | 
({32{bmu_inst_sub_funct[COUNTB_BIT]}} & countb_inst_res ) ; 
endmodule
 
 
 
module m130_lsu_top ( 
input logic clk, 
input logic rst_n, 
input logic[1-1:0] pcu_lsu_inst_vld, 
output logic[1-1:0] lsu_pcu_inst_rdy, 
input logic[1*4-1:0] pcu_lsu_inst_cmd, 
input logic[1*4-1:0] pcu_lsu_inst_fence_op, 
input logic[1*$clog2(7)-1:0] pcu_lsu_inst_amo_op, 
input logic[1-1:0] pcu_lsu_inst_exclusive, 
input logic[1-1:0] pcu_lsu_inst_ord_aq, 
input logic[1-1:0] pcu_lsu_inst_ord_rl, 
input logic[1-1:0] pcu_lsu_inst_dst_vld, 
input logic [1-1:0] pcu_lsu_inst_data_wb_vld, 
input logic [1*32-1:0] pcu_lsu_inst_data, 
input logic [1*2-1:0] pcu_lsu_inst_size, 
input logic [1-1:0] pcu_lsu_inst_signed, 
input logic [1-1:0] pcu_lsu_inst_addr_fwd, 
input logic [1-1:0][32-1:0] pcu_lsu_inst_addr_fwd_val , 
output logic [1-1:0] lsu_pcu_cmt, 
output logic [1*32-1:0] lsu_pcu_cmt_data, 
output logic [1-1:0] lsu_pcu_cmt_excp, 
output logic[1*32-1:0] lsu_pcu_cmt_excp_cause, 
output logic[32-1:0] lsu_pcu_cmt_excp_addr, 
output logic[1-1:0] lsu_pcu_cmt_trig_dbg, 
input logic pcu_lsu_flush, 
input logic pcu_lsu_flush_st_inst_wb, 
input logic pcu_lsu_non_spec, 
output logic lsu_pcu_non_flush_infly, 
input logic core_dbg_mode, 
input logic [1:0] core_priv_mode, 
input logic [32-1:0] exu_lsu_address, 
output logic lsu_pmp_vld, 
output logic [32-1:0] lsu_pmp_addr, 
output logic lsu_pmp_rw, 
input logic pmp_lsu_err, 
input logic pma_lsu_idempotency_n, 
input logic pma_lsu_cacheable, 
output logic lsu_tm_a_vld, 
output logic[32-1:0] lsu_tm_a_addr, 
output logic[2:0] lsu_tm_a_ldst, 
output logic[2-1:0] lsu_tm_a_size, 
input logic tm_lsu_trigger_a_vld, 
input logic tm_lsu_trigger_debug_a, 
output logic lsu_tm_d_vld, 
output logic[32-1:0] lsu_tm_d_data, 
output logic[2:0] lsu_tm_d_ldst, 
output logic[2-1:0] lsu_tm_d_size, 
input logic tm_lsu_trigger_d_vld, 
input logic tm_lsu_trigger_debug_d, 
output logic lsu_mss_ar_valid, 
input logic mss_lsu_ar_ready, 
output logic [32-1:0] lsu_mss_ar_addr, 
output logic [2:0] lsu_mss_ar_size, 
output logic [7:0] lsu_mss_ar_len, 
output logic [1:0] lsu_mss_ar_burst, 
output logic [2-1:0] lsu_mss_ar_id, 
output logic [3:0] lsu_mss_ar_cache, 
output logic lsu_mss_ar_lock, 
output logic [2:0] lsu_mss_ar_prot, 
output logic [5-1:0]lsu_mss_ar_user, 
output logic lsu_mss_aw_valid, 
input logic mss_lsu_aw_ready, 
output logic [32-1:0] lsu_mss_aw_addr, 
output logic [2:0] lsu_mss_aw_size, 
output logic [7:0] lsu_mss_aw_len, 
output logic [1:0] lsu_mss_aw_burst, 
output logic [2-1:0] lsu_mss_aw_id, 
output logic [3:0] lsu_mss_aw_cache, 
output logic lsu_mss_aw_lock, 
output logic [2:0] lsu_mss_aw_prot, 
output logic [5-1:0]lsu_mss_aw_user, 
output logic [5:0] lsu_mss_aw_atop, 
output logic lsu_mss_w_valid, 
input logic mss_lsu_w_ready, 
output logic [32-1:0] lsu_mss_w_data, 
output logic [32/8-1:0] lsu_mss_w_strb, 
output logic lsu_mss_w_last, 
input logic mss_lsu_r_valid, 
output logic lsu_mss_r_ready, 
input logic [32-1:0] mss_lsu_r_data, 
input logic [2-1:0] mss_lsu_r_id, 
input logic mss_lsu_r_last, 
input logic [1:0] mss_lsu_r_resp, 
input logic [1-1:0] mss_lsu_r_user, 
input logic mss_lsu_b_valid, 
output logic lsu_mss_b_ready, 
input logic [2-1:0] mss_lsu_b_id, 
input logic [1:0] mss_lsu_b_resp, 
output logic lsu_mss_fencei_req, 
input logic mss_lsu_fencei_done, 
output logic lsu_mss_load_flush, 
input logic[31:0] itcm_base_addr, 
input logic[31:0] dtcm_base_addr, 
input logic endianess, 
input logic[32-1:0] core_mmr_base_addr, 
output logic lsu_idle 
); 
localparam logic[4-1:0] PCU_OP_LOAD = 'b0001; 
localparam logic[4-1:0] PCU_OP_STORE = 'b0010; 
localparam logic[4-1:0] PCU_OP_FENCE = 'b0100; 
localparam logic[4-1:0] PCU_OP_AMO = 'b1000; 
localparam logic[$clog2(4)-1:0] LSU_OP_LOAD = 'd0; 
localparam logic[$clog2(4)-1:0] LSU_OP_STORE = 'd1; 
localparam logic[$clog2(4)-1:0] LSU_OP_FENCE = 'd2; 
localparam logic[$clog2(4)-1:0] LSU_OP_AMO = 'd3; 
localparam logic[$clog2(7)-1:0] AMO_OP_SWAP = 'd0; 
localparam logic[$clog2(7)-1:0] AMO_OP_ADD = 'd1; 
localparam logic[$clog2(7)-1:0] AMO_OP_AND = 'd2; 
localparam logic[$clog2(7)-1:0] AMO_OP_OR = 'd3; 
localparam logic[$clog2(7)-1:0] AMO_OP_XOR = 'd4; 
localparam logic[$clog2(7)-1:0] AMO_OP_MAX = 'd5; 
localparam logic[$clog2(7)-1:0] AMO_OP_MIN = 'd6; 
localparam ATOMIC_OP_SWAP = 4'b0000; 
localparam ATOMIC_OP_ADD = 4'b0001; 
localparam ATOMIC_OP_AND = 4'b0010; 
localparam ATOMIC_OP_OR = 4'b0011; 
localparam ATOMIC_OP_XOR = 4'b0100; 
localparam ATOMIC_OP_MAX = 4'b0101; 
localparam ATOMIC_OP_MIN = 4'b0110; 
localparam ATOMIC_OP_MAXU = 4'b0111; 
localparam ATOMIC_OP_MINU = 4'b1000; 
localparam ATOMIC_OP_SC = 4'b1001; 
localparam ATOMIC_OP_NONE = 4'b1111; 
localparam MSS_RESP_OKAY = 2'b00; 
localparam MSS_RESP_EXOKAY = 2'b01; 
localparam MSS_RESP_SLVERR = 2'b10; 
localparam MSS_RESP_DECERR = 2'b11; 
localparam logic[2-1:0] AXI_ID_COMMON = 'd0; 
localparam logic[2-1:0] AXI_ID_EXCLU = 'd1; 
localparam logic[2-1:0] AXI_ID_AMO = 'd2; 
localparam LDST_LD = 1'b0; 
localparam LDST_ST = 1'b1; 
localparam FENCE_OP_PRED = 4'b0001; 
localparam FENCE_OP_SUCC = 4'b0010; 
localparam FENCE_OP_TSO = 4'b0100; 
localparam FENCE_OP_FENCEI = 4'b1000; 
localparam FENCE_CMD_NONE = 2'b00; 
localparam FENCE_CMD_PRED = 2'b01; 
localparam FENCE_CMD_FENCEI = 2'b10; 
localparam EXCP_BRKP = 32'h3; 
localparam EXCP_LD_ACCESS_FAUTL = 32'h5; 
localparam EXCP_ST_ACCESS_FAUTL = 32'h7; 
localparam EXCP_LD_MISALIGN = 32'h4; 
localparam EXCP_ST_MISALIGN = 32'h6; 
localparam EXCP_RAS_ERR = 32'h1f; 
localparam EX_EXCP_W = 2; 
localparam logic[EX_EXCP_W-1:0] EX_NO_EXCP = 'b00; 
localparam logic[EX_EXCP_W-1:0] EX_EXCP_ACCESS_FAULT = 'b01; 
localparam logic[EX_EXCP_W-1:0] EX_EXCP_MISALIGN = 'b10; 
localparam logic[EX_EXCP_W-1:0] EX_EXCP_BRKP = 'b11; 
localparam SIZE_BYTE = 2'b00; 
localparam SIZE_HALF = 2'b01; 
localparam SIZE_WORD = 2'b10; 
localparam ATTRI_DEVICE = 4'b0000; 
localparam ATTRI_NC = 4'b0011; 
localparam ATTRI_WBC = 4'b1111; 
localparam MSS_ROUT_SOC = 4'b0001; 
localparam MSS_ROUT_ITCM = 4'b0010; 
localparam MSS_ROUT_DTCM = 4'b0100; 
localparam MSS_ROUT_BTBD = 4'b1001; 
localparam REG_ROUT_SOC = 2'b00; 
localparam REG_ROUT_ITCM = 2'b01; 
localparam REG_ROUT_DTCM = 2'b10; 
localparam REG_ROUT_BTBD = 2'b11; 
typedef struct packed { 
logic [32-1:0] addr; 
logic [$clog2(4)-1:0] cmd; 
logic [1:0] fence_op; 
logic [2-1:0] width; 
logic is_signed; 
logic unalign; 
logic req_in_mmr; 
logic tm_hit; 
logic tm_req_dbg; 
} lpcr_t; 
typedef struct packed { 
logic [EX_EXCP_W-1:0] excp_code; 
logic [1:0] mem_attri; 
logic number; 
logic [1:0] addr_route; 
} lpcr2_t; 
typedef enum logic [1:0] { 
IDLE = 2'b00, 
EX = 2'b01, 
WB = 2'b10 
} state_t; 
typedef struct packed { 
logic [32-3:0] addr; 
logic [32-1:0] data; 
logic [3:0] strb; 
logic [1:0] mem_attri; 
logic [1:0] addr_route; 
logic req_from_dbg; 
logic priv_mode; 
} sbu_entry_t; 
localparam SBU_ENTRY_WIDTH = $bits(sbu_entry_t); 
localparam SBU_PTR_W = $clog2(2); 
lpcr_t lpcr_q, lpcr_d; 
logic lpcr_upd; 
lpcr2_t lpcr2_q, lpcr2_d; 
logic lpcr2_upd; 
state_t ld_state1_q, ld_state2_q, st_state_q, 
ld_state1_d, ld_state2_d, st_state_d; 
logic ld_s1_upd, ld_s2_upd, st_s_upd; 
logic ld1_idle; 
logic ld1_ex; 
logic ld1_wb; 
logic ld2_idle; 
logic ld2_ex; 
logic ld2_wb; 
logic st_idle; 
logic st_ex; 
logic st_wb; 
logic mmr_berr2acfault; 
logic cmt_excp_bus_err; 
logic st_fst_berr_q, st_fst_berr_d, st_fst_berr_upd; 
logic b_bus_err; 
logic ld_req; 
logic ld_hs; 
logic st_req_common; 
logic st_hs_common; 
logic st_req_nofence; 
logic st_hs_nofence; 
logic fence_req; 
logic fence_hs; 
logic st_req; 
logic st_hs; 
logic ld1_stall_in_ex; 
logic ld_spec_device; 
logic ld1_spec_device; 
logic ld_sbu_match; 
logic ld1_sbu_match; 
logic ld1_ex_excp; 
logic ld2_ex_excp; 
logic ld2_stall_in_ex; 
logic ld2_sbu_match; 
logic device_st; 
logic dbg_st_pop; 
logic dbg_st_num_q; 
logic dbg_st_num_d; 
logic dbg_st_num_en; 
logic dbg_st_cmt; 
logic common_st; 
logic fence_st; 
logic device_st_done; 
logic common_st_done; 
logic fence_st_done; 
logic st_done; 
logic misalign; 
logic misalign_err; 
logic device_acc_fault; 
logic ld1_wb_excp; 
logic st_wb_excp; 
logic ld_unalign; 
logic st_unalign; 
logic check_pmp; 
logic pcu_req_hs; 
logic sec_req_go; 
logic pmp_ld; 
logic pmp_st; 
logic attri_device; 
logic attri_cacheable; 
logic pmp_err; 
logic device_st_pop; 
logic device_st_handling; 
logic predesessor_handling; 
logic fencei_done; 
logic fencei_hs; 
logic fencei_handling; 
logic mss_rd_req; 
logic ld1_mss_req; 
logic ld2_mss_req; 
logic mss_rd_ready; 
logic mss_ld_req_hs; 
logic mss_data_resp; 
logic mss_dbus_err; 
logic mss_resp_excp; 
logic ld_cmt; 
logic ld1_cmt_nexcp; 
logic ld1_cmt; 
logic ld2_cmt_nexcp; 
logic ld2_cmt; 
logic ld1_cmt_excp; 
logic ld2_cmt_excp; 
logic st_cmt_excp; 
logic st_cmt_nexcp; 
logic st_cmt; 
logic st_align; 
logic st_fst; 
logic st_sec; 
logic sbu_wr_mss_addr; 
logic sbu_wr_mss_data; 
logic mem_wr_req_addr_hs; 
logic mem_wr_req_data_hs; 
logic mem_wr_done; 
logic cmt_excp_brkp; 
logic cmt_excp_misalign; 
logic cmt_excp_acfault; 
logic lsu_flush; 
logic ex_tm_brkp; 
logic tm_data_brkp; 
logic ld_device_infly; 
logic req_unalign; 
logic unalign; 
logic lpcr_has_excp; 
logic lsu_fsm_idle; 
logic lsu_all_idle; 
logic fencei_infly; 
logic sbu_wr_need_check; 
logic ld2_wb_excp; 
logic sbu_wr_hs; 
logic sbu_flush; 
logic sync_store_at_sbu_head; 
logic st_sync_bus_err; 
logic ld_dbus_err; 
logic route_addr_in_mmr; 
logic route_addr_in_pbus; 
logic addr_is_bged; 
logic big_endian; 
logic ld1_ex_excp_no_pmp_tm; 
logic ld2_ex_excp_no_pmp_tm; 
logic mss_rd_req_no_pmp_tm; 
logic ld_hs_mss_req_no_pmp_tm; 
logic ld1_mss_req_no_pmp_tm; 
logic ld2_mss_req_no_pmp_tm; 
logic ex1_ld_req_misalign; 
logic ld1_req_size; 
logic wb_tm_hit; 
logic [3:0] ar_route; 
logic [3:0] aw_route; 
logic [2-1:0] ld1_width; 
logic [32-1:0] ed_route_addr; 
logic [32-1:0] bitband_base0_addr; 
logic [32-1:0] bitband_base1_addr; 
logic [3:0] wstrb_lted; 
logic [1:0] size_v; 
logic [32-1:0] pcu_req_addr_aligned; 
logic [32-1:0] pcu_req_addr; 
logic [32-1:0] ld_pmp_addr; 
logic [32-1:0] pmp_addr; 
logic [2-1:0] pcu_req_size; 
logic [32-1:0] fst_req_addr; 
logic [32-1:0] sec_req_addr; 
logic [2-1:0] ld_sbu_match_vec; 
logic [2-1:0] ld2_sbu_match_vec; 
logic [2-1:0] ld1_sbu_match_vec; 
logic [2-1:0] lpcr_sbu_match_vec; 
logic [32-1:0] ld2_req_addr; 
logic [2:0] lsu_mss_prot; 
logic [1:0] mem_attri; 
logic [32-1:0] mss_rdata; 
logic [32-1:0] lsu_cmt_data; 
logic [32-1:0] st_data_unshift; 
logic [1:0] st_addr_low; 
logic [32-1:0] st_data; 
logic [32-1:0] req_addr; 
logic [2-1:0] req_width; 
logic [$clog2(4)-1:0] addr_add_low; 
logic [$clog2(4)-1:0] addr_low; 
sbu_entry_t sbu_hdata; 
sbu_entry_t sbu_sdata; 
logic sbu_hdata_vld; 
logic sbu_sdata_vld; 
logic sbu_stop_send; 
logic fencei_d, fencei_q, fencei_upd; 
logic [32-1:0] ld_fst_data; 
logic [32-1:0] ldst_data_q; 
logic [32-1:0] ldst_data_d; 
logic ldst_data_upd; 
logic ld_data_upd; 
logic st_data_upd; 
logic [3:0] fst_st_strb; 
logic [3:0] sec_st_strb; 
logic [4:0] data_shift; 
logic [32-1:0] ld1_req_addr; 
logic [16-1:0] lpcr_match_addr; 
logic [32-1:0] ld_cmt_data; 
logic lpcr_match_sbu_write; 
logic lpcr_sbu_match; 
logic [32-1:0] cmt_excp_code; 
logic [EX_EXCP_W-1:0] cmt_ex_excp_code; 
logic [EX_EXCP_W-1:0] ex_excp_code; 
logic [1:0] pcu_fence_cmd; 
logic sbu_wr_en; 
sbu_entry_t sbu_wdata; 
logic sbu_head_pop; 
logic sbu_addr_sent; 
logic sbu_data_sent; 
logic sbu_wr_rdy; 
sbu_entry_t sbu_rdata [2-1:0]; 
logic sbu_rdata_valid [2-1:0]; 
logic [SBU_PTR_W-1:0] sbu_send_ptr; 
logic [SBU_PTR_W-1:0] sbu_head_ptr; 
logic sbu_full; 
logic sbu_empty; 
logic ld_hs_mss_req; 
logic ld_match_sbu_write; 
logic [32-1:0] pcu_inst_addr; 
logic check_addr_brkp; 
logic [32-1:0] tm_addr; 
logic [2:0] tm_a_ldst; 
logic [2-1:0]tm_a_size; 
logic ex_tm_addr_hit; 
logic ex_tm_dbg; 
logic [2:0] tm_d_ldst; 
logic [2-1:0]tm_d_size; 
logic wb_tm_data_hit; 
logic wb_tm_req_dbg; 
logic [1:0] ar_addr_route; 
logic unalign_st_fst; 
logic unalign_st_sec; 
logic cmt_excp_misalign_ld; 
logic cmt_excp_misalign_st; 
logic cmt_excp_acfault_ld; 
logic cmt_excp_acfault_st; 
logic [32-1:0] st_route_addr; 
logic [32-1:0] route_addr; 
logic [32-1:0] ld_route_addr; 
logic addr_match_dtcm; 
logic addr_match_itcm; 
logic [1:0] addr_route_res; 
logic [$clog2(4)-1:0] lsu_inst_cmd; 
logic sec_req_need_check_q, sec_req_need_check_d, sec_req_need_check_upd; 
logic sel_pcu, sel_ld1, sel_fst, sel_sec; 
logic dst_is_bitband; 
logic dst_is_soc; 
logic dst_is_dtcm; 
logic dst_is_itcm; 
logic dst_is_mmr; 
logic [18:0] tcm_route_addr; 
logic dtcm_acc_fault; 
logic itcm_acc_fault; 
logic tcm_err; 
logic [32-1:0] mss_rdata_rotated; 
logic st_nonflush_infly; 
logic ld1_is_done; 
logic st_sbu_hs; 
logic sbu_wr_en_without_flush; 
logic ld_hs_succ; 
logic ld_stall_to_ex; 
logic ld1_is_ending; 
logic unalign_ld_hs; 
logic ld2_is_ending; 
logic unalign_ld2_hs; 
logic st_hs_succ; 
logic st_committing; 
logic pcu_inst_is_load; 
logic pcu_inst_is_store; 
logic pcu_inst_is_fence; 
logic pcu_inst_is_amo; 
logic ld_stall; 
logic ld1_stall; 
logic dst_is_btbd_alias; 
logic btbd_alias_err; 
logic ex_access_fault; 
logic ex_misalign_err; 
logic ex_access_fault_no_pmp; 
assign pcu_inst_addr = pcu_lsu_inst_addr_fwd ? pcu_lsu_inst_addr_fwd_val : exu_lsu_address; 
assign lsu_inst_cmd = ({$clog2(4){pcu_lsu_inst_cmd[0]}} & LSU_OP_LOAD) 
| ({$clog2(4){pcu_lsu_inst_cmd[1]}} & LSU_OP_STORE) 
| ({$clog2(4){pcu_lsu_inst_cmd[2]}} & LSU_OP_FENCE) 
| ({$clog2(4){pcu_lsu_inst_cmd[3]}} & LSU_OP_AMO); 
assign pcu_inst_is_load = pcu_lsu_inst_cmd[0]; 
assign pcu_inst_is_store = pcu_lsu_inst_cmd[1]; 
assign pcu_inst_is_fence = pcu_lsu_inst_cmd[2]; 
assign pcu_inst_is_amo = pcu_lsu_inst_cmd[3]; 
assign ld1_idle = ld_state1_q == IDLE; 
assign ld1_ex = ld_state1_q == EX; 
assign ld1_wb = ld_state1_q == WB; 
assign ld2_idle = ld_state2_q == IDLE; 
assign ld2_ex = ld_state2_q == EX; 
assign ld2_wb = ld_state2_q == WB; 
assign st_idle = st_state_q == IDLE; 
assign st_ex = st_state_q == EX; 
assign st_wb = st_state_q == WB; 
assign ld_req = pcu_lsu_inst_vld && pcu_inst_is_load; 
assign ld_hs = ld_req && lsu_pcu_inst_rdy; 
assign st_req_common = pcu_lsu_inst_vld && pcu_inst_is_store; 
assign st_hs_common = st_req_common && lsu_pcu_inst_rdy; 
assign fence_req = pcu_lsu_inst_vld && pcu_inst_is_fence; 
assign fence_hs = fence_req && lsu_pcu_inst_rdy; 
assign st_req = st_req_common || fence_req; 
assign st_hs = st_req && lsu_pcu_inst_rdy; 
assign st_req_nofence = st_req && !fence_req; 
assign st_hs_nofence = st_req_nofence && lsu_pcu_inst_rdy; 
assign lsu_flush = pcu_lsu_flush; 
assign ld_spec_device = ((ld_hs && !unalign) && attri_device) && !pcu_lsu_non_spec; 
assign ld1_spec_device = (ld1_ex && attri_device) && !pcu_lsu_non_spec; 
assign ld1_ex_excp = ex_misalign_err || ex_access_fault || ex_tm_brkp; 
assign ld1_ex_excp_no_pmp_tm = ex_misalign_err || ex_access_fault_no_pmp; 
assign ld2_ex_excp = ld1_ex_excp; 
assign ld2_ex_excp_no_pmp_tm = ld1_ex_excp_no_pmp_tm; 
assign lsu_fsm_idle = ld1_idle && ld2_idle && st_idle; 
assign lsu_all_idle = !pcu_lsu_inst_vld && lsu_fsm_idle; 
assign ld_s1_upd = !lsu_all_idle; 
assign ld_s2_upd = !lsu_all_idle; 
assign st_s_upd = !lsu_all_idle; 
`WDFFERVAL(ld_state1_q, ld_state1_d, ld_s1_upd, clk, rst_n, IDLE) 
`WDFFERVAL(ld_state2_q, ld_state2_d, ld_s2_upd, clk, rst_n, IDLE) 
`WDFFERVAL(st_state_q, st_state_d, st_s_upd, clk, rst_n, IDLE) 
assign ld_stall = ld_sbu_match || ld_spec_device 
; 
assign ld1_stall = ld1_sbu_match || ld1_spec_device 
; 
assign ld_stall_to_ex = (!mss_lsu_ar_ready || attri_device || ld_stall) && !ld1_ex_excp; 
assign ld1_stall_in_ex = (!mss_lsu_ar_ready || ld1_stall) && !wb_tm_hit; 
assign ld_hs_succ = ld_hs && !lsu_flush; 
assign ld1_is_ending = mss_data_resp || ld1_wb_excp; 
always_comb begin 
ld_state1_d = IDLE; 
case(ld_state1_q) 
IDLE: begin 
ld_state1_d = (ld_hs_succ && ld_stall_to_ex) ? EX : 
(ld_hs_succ && !ld_stall_to_ex) ? WB : IDLE; 
end 
EX: ld_state1_d = lsu_flush ? IDLE : (!ld1_stall_in_ex ? WB : EX); 
WB: ld_state1_d = ((ld1_is_ending && !ld_hs) || lsu_flush) ? IDLE : 
(ld1_is_ending && ld_hs && ld_stall_to_ex) ? EX : WB; 
default: ld_state1_d = IDLE; 
endcase 
end 
assign ld1_is_done = (ld1_wb && mss_data_resp) || ld1_idle; 
assign ld2_stall_in_ex = !ld1_is_done || ((!mss_rd_ready || ld2_sbu_match || ld_hs) && !ld2_ex_excp); 
assign ld2_wb_excp = lpcr_has_excp; 
assign unalign_ld_hs = ld_hs && ld_unalign; 
assign ld2_is_ending = (mss_data_resp && !ld1_wb) || ld2_wb_excp; 
assign unalign_ld2_hs = unalign_ld_hs && !ld1_ex_excp; 
always_comb begin 
ld_state2_d = IDLE; 
case(ld_state2_q) 
IDLE: begin 
ld_state2_d = (unalign_ld_hs && !ld1_ex_excp && !lsu_flush) ? EX : IDLE; 
end 
EX: ld_state2_d = (lsu_flush || ld1_cmt_excp) ? IDLE : (!ld2_stall_in_ex ? WB : EX); 
WB: ld_state2_d = ((ld2_is_ending && !unalign_ld2_hs) || lsu_flush) ? IDLE : 
(ld2_is_ending && unalign_ld2_hs && ld2_stall_in_ex) ? EX : WB; 
default: ld_state2_d = IDLE; 
endcase 
end 
assign device_st = lpcr_q.cmd == LSU_OP_STORE && lpcr2_q.mem_attri[0]; 
assign fence_st = lpcr_q.cmd == LSU_OP_FENCE; 
assign device_st_done = st_wb && device_st && !device_st_handling; 
assign fence_st_done = st_wb && fence_st && (!fencei_handling && !predesessor_handling); 
assign common_st = lpcr_q.cmd == LSU_OP_STORE && !lpcr2_q.mem_attri[0]; 
assign common_st_done = st_wb && common_st && st_sbu_hs; 
assign st_done = common_st_done || device_st_done || fence_st_done; 
assign sync_store_at_sbu_head = (device_st && sbu_hdata_vld && sbu_hdata.mem_attri[0]) || (sbu_hdata_vld && sbu_hdata.req_from_dbg && ~dbg_st_num_q 
); 
assign st_hs_succ = st_hs && !lsu_flush; 
assign st_committing = core_dbg_mode ? (dbg_st_cmt || st_wb_excp) : 
((st_done && lpcr2_q.number == 1'b0) || st_wb_excp); 
always_comb begin 
st_state_d = IDLE; 
case(st_state_q) 
IDLE : st_state_d = st_hs_succ ? WB : IDLE; 
WB : st_state_d = ((st_committing && !st_hs_succ) || lsu_flush) ? IDLE : WB; 
default: st_state_d = IDLE; 
endcase 
end 
assign device_acc_fault = (misalign || sec_req_go) && attri_device; 
assign misalign_err = 1'b0; 
assign btbd_alias_err = (unalign || sec_req_go || (req_width != SIZE_WORD)) && dst_is_bitband; 
assign lpcr_has_excp = (lpcr2_q.excp_code != EX_NO_EXCP) || wb_tm_hit; 
assign ld1_wb_excp = lpcr_has_excp; 
assign st_wb_excp = lpcr_has_excp; 
assign addr_low = req_addr[1:0]; 
always_comb begin 
case (req_width) 
SIZE_BYTE: req_unalign = 1'b0; 
SIZE_HALF: req_unalign = (addr_low == 2'b11) ? 1'b1 : 1'b0; 
SIZE_WORD: req_unalign = (addr_low == 2'b00) ? 1'b0 : 1'b1; 
default: req_unalign = 1'b0; 
endcase 
end 
assign unalign = pcu_inst_is_fence ? 1'b0 : req_unalign; 
assign misalign = !pcu_inst_is_fence && 
((req_addr[0] != 1'b0 && req_width == SIZE_HALF) || 
(req_addr[1:0] != 2'b00 && req_width == SIZE_WORD)); 
assign pcu_req_addr_aligned = {req_addr[32-1:$clog2(4)], {$clog2(4){1'b0}}}; 
assign pcu_req_addr = misalign ? pcu_req_addr_aligned : req_addr; 
assign pcu_req_size = misalign ? SIZE_WORD : req_width; 
assign ld_unalign = unalign; 
assign st_unalign = unalign; 
assign sel_pcu = !ld1_ex && !ld2_ex; 
assign sel_ld1 = ld1_ex && !ld2_ex && !lpcr_q.unalign; 
assign sel_fst = ld1_ex && lpcr_q.unalign; 
assign sel_sec = !ld1_ex && ld2_ex; 
assign check_pmp = 1'b1; 
assign pcu_req_hs = pcu_lsu_inst_vld && lsu_pcu_inst_rdy; 
assign sec_req_go = (ld2_ex && ld1_is_done) || 
(st_done && lpcr2_q.number == 1'b1); 
assign ld_pmp_addr = ({32{sel_pcu}} & pcu_req_addr) 
| ({32{sel_ld1}} & ld1_req_addr) 
| ({32{sel_fst}} & fst_req_addr) 
| ({32{sel_sec}} & sec_req_addr); 
assign pmp_addr = (st_wb && sec_req_go) ? sec_req_addr : ld_pmp_addr; 
assign pmp_ld = ld_hs || (sec_req_go && lpcr_q.cmd == LSU_OP_LOAD); 
assign pmp_st = st_hs_nofence || (sec_req_go && lpcr_q.cmd == LSU_OP_STORE); 
assign attri_device = (dst_is_dtcm || dst_is_itcm) ? 1'b0 : 
dst_is_mmr ? 1'b1 : pma_lsu_idempotency_n; 
assign attri_cacheable = (dst_is_dtcm || dst_is_itcm) ? 1'b1 : 
dst_is_mmr ? 1'b0 : pma_lsu_cacheable; 
assign pmp_err = pmp_lsu_err; 
assign lsu_pmp_vld = check_pmp; 
assign lsu_pmp_addr = pmp_addr; 
assign lsu_pmp_rw = pmp_st; 
assign check_addr_brkp = pcu_req_hs && !fence_hs; 
assign tm_addr = pcu_inst_addr; 
assign tm_a_ldst = {pcu_inst_is_amo, pcu_inst_is_store, pcu_inst_is_load}; 
assign tm_a_size = pcu_lsu_inst_size; 
assign ex_tm_addr_hit = tm_lsu_trigger_a_vld; 
assign ex_tm_dbg = tm_lsu_trigger_debug_a && tm_lsu_trigger_a_vld; 
assign ex_tm_brkp = 1'b0; 
assign lsu_tm_a_vld = check_addr_brkp; 
assign lsu_tm_a_addr = tm_addr & {32{lsu_tm_a_vld}}; 
assign lsu_tm_a_ldst = tm_a_ldst; 
assign lsu_tm_a_size = tm_a_size; 
assign wb_tm_data_hit = '0; 
assign wb_tm_req_dbg = '0; 
assign lsu_tm_d_vld = '0; 
assign lsu_tm_d_data = '0; 
assign lsu_tm_d_ldst = '0; 
assign lsu_tm_d_size = '0; 
assign tm_data_brkp = 1'b0; 
`WDFFER(lpcr_q, lpcr_d, lpcr_upd, clk, rst_n) 
`WDFFER(lpcr2_q, lpcr2_d, lpcr2_upd, clk, rst_n) 
assign ex_access_fault = device_acc_fault || pmp_err || tcm_err 
; 
assign ex_access_fault_no_pmp = device_acc_fault || tcm_err 
; 
assign ex_misalign_err = misalign_err 
; 
always_comb begin 
casez({fence_hs, ex_tm_brkp, ex_access_fault, ex_misalign_err}) 
4'b1??? : ex_excp_code = EX_NO_EXCP; 
4'b01?? : ex_excp_code = EX_EXCP_BRKP; 
4'b001? : ex_excp_code = EX_EXCP_ACCESS_FAULT; 
4'b0001 : ex_excp_code = EX_EXCP_MISALIGN; 
4'b0000 : ex_excp_code = EX_NO_EXCP; 
default : ex_excp_code = 'x; 
endcase 
end 
always_comb begin 
casez(pcu_lsu_inst_fence_op) 
4'b1??? : pcu_fence_cmd = FENCE_CMD_FENCEI; 
4'b01?? : pcu_fence_cmd = FENCE_CMD_PRED; 
4'b0011 : pcu_fence_cmd = FENCE_CMD_PRED; 
default : pcu_fence_cmd = FENCE_CMD_NONE; 
endcase 
end 
assign lpcr_upd = pcu_req_hs; 
always_comb begin 
lpcr_d.addr = pcu_inst_addr; 
lpcr_d.cmd = lsu_inst_cmd; 
lpcr_d.fence_op = pcu_fence_cmd; 
lpcr_d.width = pcu_lsu_inst_size; 
lpcr_d.is_signed = pcu_lsu_inst_signed; 
lpcr_d.unalign = unalign; 
lpcr_d.req_in_mmr = dst_is_mmr; 
lpcr_d.tm_hit = ex_tm_addr_hit; 
lpcr_d.tm_req_dbg = ex_tm_dbg; 
end 
assign lpcr2_upd = pcu_req_hs || sec_req_go; 
always_comb begin 
lpcr2_d.mem_attri = '0; 
lpcr2_d.excp_code = '0; 
lpcr2_d.addr_route = '0; 
case({pcu_req_hs, sec_req_go}) 
2'b10: begin 
lpcr2_d.mem_attri[0] = attri_device; 
lpcr2_d.mem_attri[1] = attri_cacheable; 
lpcr2_d.excp_code = ex_excp_code; 
lpcr2_d.addr_route = addr_route_res; 
lpcr2_d.number = unalign; 
end 
2'b01: begin 
lpcr2_d.mem_attri[0] = attri_device; 
lpcr2_d.mem_attri[1] = attri_cacheable; 
lpcr2_d.excp_code = ex_excp_code; 
lpcr2_d.addr_route = addr_route_res; 
lpcr2_d.number = 1'b0; 
end 
default: lpcr2_d = lpcr2_q; 
endcase 
end 
assign ld1_req_addr = ex1_ld_req_misalign ? {lpcr_q.addr[32-1:2], 2'b00} : lpcr_q.addr; 
assign fst_req_addr = {lpcr_q.addr[32-1:$clog2(4)], {$clog2(4){1'b0}}}; 
assign sec_req_addr = {lpcr_q.addr[32-1:$clog2(4)] + 1'b1 
, {$clog2(4){1'b0}}}; 
m130_lsu_sbu #( 
.T (sbu_entry_t ), 
.DATA_WIDTH (SBU_ENTRY_WIDTH ), 
.BUFF_DEPTH (2 ), 
.SBU_PTR_W (SBU_PTR_W ) 
) u_m130_sbu ( 
.clk (clk ), 
.rst_n (rst_n ), 
.wr_en (sbu_wr_en ), 
.wdata (sbu_wdata ), 
.head_pop (sbu_head_pop ), 
.sbu_addr_sent (sbu_addr_sent ), 
.sbu_data_sent (sbu_data_sent ), 
.unalign_st_fst (unalign_st_fst ), 
.unalign_st_sec (unalign_st_sec ), 
.flush (sbu_flush ), 
.rdata (sbu_rdata ), 
.rdata_valid (sbu_rdata_valid ), 
.send_ptr (sbu_send_ptr ), 
.head_ptr (sbu_head_ptr ), 
.is_full (sbu_full ), 
.is_empty (sbu_empty ), 
.stop_send (sbu_stop_send ) 
); 
assign lpcr_match_addr = ld1_ex ? ld1_req_addr[16+1:2] : ld2_req_addr[16+1:2]; 
for (genvar i=0; i<2; i++) begin : lsu_gen_sbu_match 
assign ld_sbu_match_vec[i] = (sbu_rdata[i].addr[16-1:0] == pcu_req_addr[16+1:2]) && sbu_rdata_valid[i]; 
assign lpcr_sbu_match_vec[i] = (sbu_rdata[i].addr[16-1:0] == lpcr_match_addr[16-1:0]) && sbu_rdata_valid[i]; 
end 
assign ld2_req_addr = sec_req_addr; 
assign ld_match_sbu_write = (sbu_wdata.addr[16-1:0] == pcu_req_addr[16+1:2]) && sbu_wr_need_check; 
assign lpcr_match_sbu_write = (sbu_wdata.addr[16-1:0] == lpcr_match_addr[16-1:0]) && sbu_wr_need_check; 
assign ld_sbu_match = |ld_sbu_match_vec || ld_match_sbu_write; 
assign lpcr_sbu_match = |lpcr_sbu_match_vec || lpcr_match_sbu_write; 
assign ld1_sbu_match = lpcr_sbu_match; 
assign ld2_sbu_match = lpcr_sbu_match; 
assign dbg_st_pop = (sbu_hdata.req_from_dbg && sbu_hdata_vld) && sbu_head_pop; 
assign dbg_st_num_d = pcu_req_hs ? unalign : 1'b0; 
assign dbg_st_num_en = pcu_req_hs || dbg_st_pop || lsu_flush; 
`WDFFER(dbg_st_num_q, dbg_st_num_d, dbg_st_num_en, clk, rst_n) 
assign device_st_pop = (sbu_hdata.mem_attri[0] && sbu_hdata_vld) && mem_wr_done; 
assign device_st_handling = (device_st && st_wb) && !device_st_pop; 
assign predesessor_handling = fence_st && (lpcr_q.fence_op == FENCE_CMD_PRED || lpcr_q.fence_op == FENCE_CMD_FENCEI) && !sbu_empty; 
logic st_wait4sbu_q; 
logic st_wait4sbu_d; 
logic st_wait4sbu_upd; 
logic fencei_sending; 
`WDFFER(st_wait4sbu_q, st_wait4sbu_d, st_wait4sbu_upd, clk, rst_n) 
assign fencei_hs = fence_hs && (pcu_fence_cmd == FENCE_CMD_FENCEI); 
assign st_wait4sbu_d = fencei_hs && !lsu_flush; 
assign st_wait4sbu_upd = (fencei_hs || fencei_sending) || lsu_flush; 
assign fencei_sending = sbu_empty && st_wait4sbu_q && (lpcr_q.cmd == LSU_OP_FENCE && lpcr_q.fence_op == FENCE_CMD_FENCEI); 
`WDFFER(fencei_q, fencei_d, fencei_upd, clk, rst_n) 
assign fencei_done = mss_lsu_fencei_done; 
assign fencei_d = fencei_sending && !fencei_done; 
assign fencei_upd = fencei_sending || fencei_done; 
assign fencei_handling = (fencei_q || fencei_sending) && !fencei_done; 
assign lsu_mss_fencei_req = fencei_sending; 
assign lsu_mss_prot = {1'b0, 1'b1, core_priv_mode == 2'b11}; 
assign mss_rd_req = (ld_hs_mss_req || ld1_mss_req || ld2_mss_req) && !lsu_flush; 
assign ld_hs_mss_req = ld_hs && !ld1_ex_excp && !ld_stall; 
assign ld1_mss_req = ld1_ex && !lpcr_has_excp && !ld1_stall; 
assign ld2_mss_req = (ld1_is_done && ld2_ex) && !ld2_sbu_match && !ld2_ex_excp; 
assign mss_rd_req_no_pmp_tm = (ld_hs_mss_req_no_pmp_tm || ld1_mss_req_no_pmp_tm || ld2_mss_req_no_pmp_tm) && !lsu_flush; 
assign ld_hs_mss_req_no_pmp_tm = ld_hs && !ld1_ex_excp_no_pmp_tm && !ld_stall && !attri_device; 
assign ld1_mss_req_no_pmp_tm = ld1_mss_req; 
assign ld2_mss_req_no_pmp_tm = (ld1_is_done && ld2_ex) && !ld2_sbu_match && !ld2_ex_excp_no_pmp_tm; 
assign mem_attri = {attri_cacheable, attri_device}; 
assign ex1_ld_req_misalign = (lpcr_q.cmd == LSU_OP_LOAD) && 
((lpcr_q.addr[0] != 1'b0 && lpcr_q.width == SIZE_HALF) || 
(lpcr_q.addr[1:0] != 2'b00 && lpcr_q.width == SIZE_WORD)); 
assign ld1_width = ex1_ld_req_misalign ? SIZE_WORD : lpcr_q.width; 
assign mss_rd_ready = mss_lsu_ar_ready; 
assign lsu_mss_ar_valid = mss_rd_req_no_pmp_tm; 
assign lsu_mss_ar_size = ({2{sel_pcu}} & pcu_req_size) 
| ({2{sel_ld1}} & ld1_width) 
| ({2{sel_fst}} & SIZE_WORD) 
| ({2{sel_sec}} & SIZE_WORD); 
assign lsu_mss_ar_addr = ({32{sel_pcu}} & pcu_req_addr) 
| ({32{sel_ld1}} & ld1_req_addr) 
| ({32{sel_fst}} & fst_req_addr) 
| ({32{sel_sec}} & sec_req_addr); 
assign lsu_mss_ar_len = 8'b0; 
assign lsu_mss_ar_burst = 2'b00; 
assign lsu_mss_ar_id = 2'b00; 
assign lsu_mss_ar_lock = 1'b0; 
assign lsu_mss_ar_cache = ({4{mem_attri == 2'b00}} & ATTRI_NC) 
| ({4{mem_attri == 2'b01}} & ATTRI_DEVICE) 
| ({4{mem_attri == 2'b10}} & ATTRI_WBC); 
assign lsu_mss_ar_prot = lsu_mss_prot; 
assign ar_addr_route = addr_route_res; 
assign ar_route = ({4{ar_addr_route == REG_ROUT_BTBD}} & MSS_ROUT_BTBD) 
| ({4{ar_addr_route == REG_ROUT_DTCM}} & MSS_ROUT_DTCM) 
| ({4{ar_addr_route == REG_ROUT_ITCM}} & MSS_ROUT_ITCM) 
| ({4{ar_addr_route == REG_ROUT_SOC }} & MSS_ROUT_SOC ); 
assign lsu_mss_ar_user = {core_dbg_mode, ar_route}; 
assign mss_ld_req_hs = lsu_mss_ar_valid && mss_lsu_ar_ready; 
assign lsu_mss_r_ready = 1'b1; 
assign mss_data_resp = mss_lsu_r_valid; 
assign ed_route_addr = {32{ld1_wb && !lpcr_q.unalign}} & ld1_req_addr 
| {32{ld1_wb && lpcr_q.unalign }} & fst_req_addr 
| {32{ld2_wb}} & sec_req_addr 
| {32{st_wb}} & {sbu_wdata.addr, 2'b00}; 
assign route_addr_in_mmr = ed_route_addr[31:18] == core_mmr_base_addr[31:18]; 
assign route_addr_in_pbus = 
1'b0 
; 
assign addr_is_bged = !(route_addr_in_mmr || route_addr_in_pbus); 
assign big_endian = endianess && addr_is_bged; 
assign mss_rdata = mss_lsu_r_data; 
assign ld_dbus_err = mss_lsu_r_valid && (mss_lsu_r_resp != MSS_RESP_OKAY); 
assign mss_dbus_err = ld_dbus_err || st_sync_bus_err 
; 
assign mss_resp_excp = mss_dbus_err; 
assign ld_cmt = ld1_cmt || ld2_cmt; 
assign ld1_cmt_nexcp = (mss_data_resp && !mss_resp_excp && ld1_wb) && !lpcr_q.unalign; 
assign ld1_cmt = ld1_cmt_nexcp || ld1_cmt_excp; 
assign ld2_cmt_nexcp = mss_data_resp && !mss_resp_excp && (ld2_wb && !ld1_wb); 
assign ld2_cmt = ld2_cmt_nexcp || ld2_cmt_excp; 
assign lsu_cmt_data = ld_cmt_data; 
always_comb begin 
casez({big_endian, lpcr_q.addr[1:0], lpcr_q.unalign && ~lpcr2_q.number, lpcr_q.width}) 
{1'b0, 2'b00, 1'b?, 2'b??}: mss_rdata_rotated = mss_rdata; 
{1'b0, 2'b01, 1'b0, 2'b??}: mss_rdata_rotated = {8'bx, mss_rdata[31:8]}; 
{1'b0, 2'b10, 1'b0, 2'b??}: mss_rdata_rotated = {16'bx, mss_rdata[31:16]}; 
{1'b0, 2'b11, 1'b0, 2'b??}: mss_rdata_rotated = {24'bx, mss_rdata[31:24]}; 
{1'b1, 2'b00, 1'b?, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[7:0], mss_rdata[15:8], mss_rdata[23:16], mss_rdata[31:24]}; 
{1'b1, 2'b01, 1'b0, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[15:8], mss_rdata[23:16], mss_rdata[31:24], 8'bx}; 
{1'b1, 2'b10, 1'b0, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[23:16], mss_rdata[31:24], 16'bx}; 
{1'b1, 2'b11, 1'b0, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[31:24], 24'bx}; 
{1'b1, 2'b00, 1'b?, SIZE_HALF}: mss_rdata_rotated = {16'bx, mss_rdata[7:0], mss_rdata[15:8]}; 
{1'b1, 2'b01, 1'b?, SIZE_HALF}: mss_rdata_rotated = {16'bx, mss_rdata[15:8], mss_rdata[23:16]}; 
{1'b1, 2'b10, 1'b?, SIZE_HALF}: mss_rdata_rotated = {16'bx, mss_rdata[23:16], mss_rdata[31:24]}; 
{1'b1, 2'b11, 1'b0, SIZE_HALF}: mss_rdata_rotated = {16'bx, mss_rdata[31:24], 8'bx}; 
{1'b1, 2'b00, 1'b?, SIZE_BYTE}: mss_rdata_rotated = {24'bx, mss_rdata[7:0]}; 
{1'b1, 2'b01, 1'b?, SIZE_BYTE}: mss_rdata_rotated = {24'bx, mss_rdata[15:8]}; 
{1'b1, 2'b10, 1'b?, SIZE_BYTE}: mss_rdata_rotated = {24'bx, mss_rdata[23:16]}; 
{1'b1, 2'b11, 1'b?, SIZE_BYTE}: mss_rdata_rotated = {24'bx, mss_rdata[31:24]}; 
{1'b0, 2'b01, 1'b1, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[7:0], 24'bx}; 
{1'b0, 2'b10, 1'b1, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[15:0], 16'bx}; 
{1'b0, 2'b11, 1'b1, SIZE_HALF}: mss_rdata_rotated = {16'bx, mss_rdata[7:0], 8'bx}; 
{1'b0, 2'b11, 1'b1, SIZE_WORD}: mss_rdata_rotated = {mss_rdata[23:0], 8'bx}; 
{1'b1, 2'b11, 1'b1, SIZE_WORD}: mss_rdata_rotated = {8'bx, mss_rdata[7:0], mss_rdata[15:8], mss_rdata[23:16]}; 
{1'b1, 2'b10, 1'b1, SIZE_WORD}: mss_rdata_rotated = {16'bx, mss_rdata[7:0], mss_rdata[15:8]}; 
{1'b1, 2'b01, 1'b1, SIZE_WORD}: mss_rdata_rotated = {24'bx, mss_rdata[7:0]}; 
{1'b1, 2'b11, 1'b1, SIZE_HALF}: mss_rdata_rotated = {24'bx, mss_rdata[7:0]}; 
default: mss_rdata_rotated = 32'bx; 
endcase 
end 
assign ld_data_upd = mss_data_resp && ld1_wb; 
assign st_data_upd = st_hs || pcu_lsu_inst_data_wb_vld; 
`WDFFENR(ldst_data_q, ldst_data_d, ldst_data_upd, clk) 
assign ldst_data_d = st_data_upd ? pcu_lsu_inst_data : mss_rdata_rotated; 
assign ldst_data_upd = ld_data_upd || st_data_upd; 
assign ld_fst_data = ldst_data_q; 
assign st_data_unshift = pcu_lsu_inst_data_wb_vld ? pcu_lsu_inst_data : ldst_data_q; 
always_comb begin 
casez({lpcr_q.addr[1:0], lpcr_q.width, lpcr_q.unalign, big_endian}) 
{2'b??, SIZE_BYTE, 1'b0, 1'b?} : ld_cmt_data[31:0] = {{24{lpcr_q.is_signed & mss_rdata_rotated[7]}}, mss_rdata_rotated[7:0]}; 
{2'b??, SIZE_HALF, 1'b0, 1'b?} : ld_cmt_data[31:0] = {{16{lpcr_q.is_signed & mss_rdata_rotated[15]}}, mss_rdata_rotated[15:0]}; 
{2'b??, SIZE_WORD, 1'b0, 1'b?} : ld_cmt_data[31:0] = mss_rdata_rotated[31:0]; 
{2'b01, SIZE_WORD, 1'b1, 1'b0} : ld_cmt_data[31:0] = {mss_rdata_rotated[31:24], ld_fst_data[23:0]}; 
{2'b10, SIZE_WORD, 1'b1, 1'b0} : ld_cmt_data[31:0] = {mss_rdata_rotated[31:16], ld_fst_data[15:0]}; 
{2'b11, SIZE_HALF, 1'b1, 1'b0} : ld_cmt_data[31:0] = {{16{lpcr_q.is_signed & mss_rdata_rotated[15]}}, mss_rdata_rotated[15:8], ld_fst_data[7:0]}; 
{2'b11, SIZE_WORD, 1'b1, 1'b0} : ld_cmt_data[31:0] = {mss_rdata_rotated[31:8], ld_fst_data[7:0]}; 
{2'b11, SIZE_WORD, 1'b1, 1'b1} : ld_cmt_data[31:0] = {ld_fst_data[31:24], mss_rdata_rotated[23:0]}; 
{2'b10, SIZE_WORD, 1'b1, 1'b1} : ld_cmt_data[31:0] = {ld_fst_data[31:16], mss_rdata_rotated[15:0]}; 
{2'b01, SIZE_WORD, 1'b1, 1'b1} : ld_cmt_data[31:0] = {ld_fst_data[31:8] , mss_rdata_rotated[7:0] }; 
{2'b11, SIZE_HALF, 1'b1, 1'b1} : ld_cmt_data[31:0] = {{16{lpcr_q.is_signed & ld_fst_data[15]}}, ld_fst_data[15:8] , mss_rdata_rotated[7:0]}; 
default : ld_cmt_data = {32{1'bx}}; 
endcase 
end 
assign ld1_cmt_excp = (ld1_wb_excp || mss_resp_excp || wb_tm_hit) && (ld1_wb && !ld2_wb); 
assign ld2_cmt_excp = (ld2_wb_excp || mss_resp_excp) && ld2_wb; 
assign sbu_wr_rdy = !sbu_full || sbu_head_pop; 
assign sbu_wr_hs = sbu_wr_en && sbu_wr_rdy; 
assign st_sbu_hs = sbu_wr_en_without_flush && sbu_wr_rdy; 
assign st_cmt_excp = st_wb && (st_wb_excp || wb_tm_hit || st_sync_bus_err 
); 
assign dbg_st_cmt = (core_dbg_mode && st_wb) && ((~dbg_st_num_q && dbg_st_pop) || (fence_st && fence_st_done)); 
assign st_cmt_nexcp = !st_cmt_excp && (st_done && lpcr2_q.number == 1'b0); 
assign st_cmt = core_dbg_mode ? (st_cmt_excp || dbg_st_cmt) : 
(st_cmt_excp || st_cmt_nexcp); 
assign st_addr_low = lpcr_q.addr[1:0]; 
always_comb begin 
casez({big_endian, st_addr_low, lpcr_q.width}) 
{1'b0, 2'b00, 2'b??}: st_data = st_data_unshift; 
{1'b0, 2'b01, 2'b??}: st_data = {st_data_unshift[23:16], st_data_unshift[15: 8], st_data_unshift[ 7: 0], st_data_unshift[31:24]}; 
{1'b0, 2'b10, 2'b??}: st_data = {st_data_unshift[15: 8], st_data_unshift[ 7: 0], st_data_unshift[31:24], st_data_unshift[23:16]}; 
{1'b0, 2'b11, 2'b??}: st_data = {st_data_unshift[7 : 0], st_data_unshift[31:24], st_data_unshift[23:16], st_data_unshift[15: 8]}; 
{1'b1, 2'b00, SIZE_WORD}: st_data = {st_data_unshift[ 7: 0], st_data_unshift[15: 8], st_data_unshift[23:16], st_data_unshift[31:24]}; 
{1'b1, 2'b01, SIZE_WORD}: st_data = {st_data_unshift[15: 8], st_data_unshift[23:16], st_data_unshift[31:24], st_data_unshift[ 7: 0]}; 
{1'b1, 2'b10, SIZE_WORD}: st_data = {st_data_unshift[23:16], st_data_unshift[31:24], st_data_unshift[ 7: 0], st_data_unshift[15: 8]}; 
{1'b1, 2'b11, SIZE_WORD}: st_data = {st_data_unshift[31:24], st_data_unshift[ 7: 0], st_data_unshift[15: 8], st_data_unshift[23:16]}; 
{1'b1, 2'b00, SIZE_HALF}: st_data = { 8'b0, 8'b0, st_data_unshift[ 7: 0], st_data_unshift[15: 8]}; 
{1'b1, 2'b01, SIZE_HALF}: st_data = { 8'b0, st_data_unshift[ 7: 0], st_data_unshift[15: 8], 8'b0}; 
{1'b1, 2'b10, SIZE_HALF}: st_data = {st_data_unshift[ 7: 0], st_data_unshift[15: 8], 8'b0, 8'b0}; 
{1'b1, 2'b11, SIZE_HALF}: st_data = {st_data_unshift[15: 8], 8'b0, 8'b0, st_data_unshift[ 7: 0]}; 
{1'b1, 2'b00, SIZE_BYTE}: st_data = { 8'b0, 8'b0, 8'b0, st_data_unshift[ 7: 0]}; 
{1'b1, 2'b01, SIZE_BYTE}: st_data = { 8'b0, 8'b0, st_data_unshift[ 7: 0], 8'b0}; 
{1'b1, 2'b10, SIZE_BYTE}: st_data = { 8'b0, st_data_unshift[ 7: 0], 8'b0, 8'b0}; 
{1'b1, 2'b11, SIZE_BYTE}: st_data = {st_data_unshift[ 7: 0], 8'b0, 8'b0, 8'b0}; 
default: st_data = 32'bx; 
endcase 
end 
always_comb begin 
fst_st_strb = 4'b0000; 
sec_st_strb = 4'b0000; 
casez({lpcr_q.unalign, lpcr_q.width, st_addr_low}) 
{1'b0, SIZE_BYTE, 2'b??} : fst_st_strb = 4'b0001 << st_addr_low; 
{1'b0, SIZE_HALF, 2'b??} : fst_st_strb = 4'b0011 << st_addr_low; 
{1'b0, SIZE_WORD, 2'b??} : fst_st_strb = 4'b1111; 
{1'b1, SIZE_HALF, 2'b11} : begin 
fst_st_strb = 4'b1000; 
sec_st_strb = 4'b0001; 
end 
{1'b1, SIZE_WORD, 2'b01} : begin 
fst_st_strb = 4'b1110; 
sec_st_strb = 4'b0001; 
end 
{1'b1, SIZE_WORD, 2'b10} : begin 
fst_st_strb = 4'b1100; 
sec_st_strb = 4'b0011; 
end 
{1'b1, SIZE_WORD, 2'b11} : begin 
fst_st_strb = 4'b1000; 
sec_st_strb = 4'b0111; 
end 
default : begin 
fst_st_strb = {4{1'bx}}; 
sec_st_strb = {4{1'bx}}; 
end 
endcase 
end 
assign st_align = !lpcr_q.unalign; 
assign st_fst = lpcr_q.unalign && lpcr2_q.number == 1'b1; 
assign st_sec = lpcr_q.unalign && lpcr2_q.number == 1'b0; 
logic store_wait4write_q, store_wait4write_d, store_wait4write_upd; 
`WDFFER(store_wait4write_q, store_wait4write_d, store_wait4write_upd, clk, rst_n) 
assign store_wait4write_upd = (st_hs_nofence || (sec_req_go && st_wb)) || (store_wait4write_q && st_sbu_hs) || lsu_flush; 
assign store_wait4write_d = ((st_hs_nofence || sec_req_go) && !lsu_flush) ? 1'b1 : 1'b0; 
assign sbu_wr_en_without_flush = (st_wb && store_wait4write_q) && !st_wb_excp 
; 
assign sbu_wr_en = sbu_wr_en_without_flush && !pcu_lsu_flush_st_inst_wb && (!lsu_flush || (((common_st && !(lpcr_q.unalign && lpcr2_q.number)) 
) && sbu_wr_rdy)); 
assign sbu_wr_need_check = (st_wb && !fence_st) && !st_wb_excp && !(device_st 
); 
assign sbu_wdata.addr = st_sec ? sec_req_addr[32-1:2] : fst_req_addr[32-1:2]; 
assign wstrb_lted = st_sec ? sec_st_strb : fst_st_strb; 
assign sbu_wdata.data = st_data; 
assign sbu_wdata.strb = wstrb_lted; 
assign sbu_wdata.mem_attri = lpcr2_q.mem_attri; 
assign sbu_wdata.addr_route = lpcr2_q.addr_route; 
assign sbu_wdata.req_from_dbg = core_dbg_mode; 
assign sbu_wdata.priv_mode = core_priv_mode == 2'b11; 
assign sbu_head_pop = mem_wr_done; 
assign sbu_addr_sent = mem_wr_req_addr_hs; 
assign sbu_data_sent = mem_wr_req_data_hs; 
assign unalign_st_fst = st_wb && lpcr_q.unalign && (lpcr2_q.number == 1'b1); 
assign unalign_st_sec = st_wb && lpcr_q.unalign && (lpcr2_q.number == 1'b0); 
assign sbu_flush = lsu_flush && st_wb && (!st_cmt_nexcp || pcu_lsu_flush_st_inst_wb); 
assign sbu_wr_mss_addr = !sbu_stop_send && !sbu_empty && sbu_sdata_vld; 
assign sbu_wr_mss_data = !sbu_stop_send && !sbu_empty && sbu_sdata_vld; 
assign sbu_sdata = sbu_rdata[sbu_send_ptr]; 
assign sbu_sdata_vld = sbu_rdata_valid[sbu_send_ptr]; 
assign sbu_hdata = sbu_rdata[sbu_head_ptr]; 
assign sbu_hdata_vld = sbu_rdata_valid[sbu_head_ptr]; 
assign mem_wr_req_addr_hs = lsu_mss_aw_valid && mss_lsu_aw_ready; 
assign lsu_mss_aw_valid = sbu_wr_mss_addr; 
assign lsu_mss_aw_addr = {sbu_sdata.addr, 2'b00}; 
assign lsu_mss_aw_size = 3'b10; 
assign lsu_mss_aw_len = 8'b0; 
assign lsu_mss_aw_burst = 2'b00; 
assign lsu_mss_aw_id = 1'b0; 
assign lsu_mss_aw_lock = 1'b0; 
assign lsu_mss_aw_cache = ({4{sbu_sdata.mem_attri == 2'b01}} & ATTRI_DEVICE) 
| ({4{sbu_sdata.mem_attri == 2'b00}} & ATTRI_NC) 
| ({4{sbu_sdata.mem_attri == 2'b10}} & ATTRI_WBC); 
assign lsu_mss_aw_prot = {1'b0, 1'b1, sbu_sdata.priv_mode}; 
assign aw_route = ({4{sbu_sdata.addr_route == REG_ROUT_BTBD}} & MSS_ROUT_BTBD) 
| ({4{sbu_sdata.addr_route == REG_ROUT_DTCM}} & MSS_ROUT_DTCM) 
| ({4{sbu_sdata.addr_route == REG_ROUT_ITCM}} & MSS_ROUT_ITCM) 
| ({4{sbu_sdata.addr_route == REG_ROUT_SOC }} & MSS_ROUT_SOC ); 
assign lsu_mss_aw_user = {sbu_sdata.req_from_dbg, aw_route}; 
assign lsu_mss_aw_atop = 6'b0; 
assign mem_wr_req_data_hs = lsu_mss_w_valid && mss_lsu_w_ready; 
assign lsu_mss_w_valid = sbu_wr_mss_data; 
assign lsu_mss_w_data = sbu_sdata.data; 
assign lsu_mss_w_strb = sbu_sdata.strb; 
assign lsu_mss_w_last = 1'b1; 
assign mem_wr_done = lsu_mss_b_ready && mss_lsu_b_valid; 
assign lsu_mss_b_ready = 1'b1; 
assign lsu_pcu_cmt = ld_cmt || st_cmt; 
assign lsu_pcu_cmt_data = lsu_cmt_data; 
assign lsu_pcu_cmt_excp = (ld1_cmt_excp || ld2_cmt_excp || st_cmt_excp) && !lsu_pcu_cmt_trig_dbg; 
assign lsu_pcu_cmt_excp_cause = cmt_excp_code; 
assign lsu_pcu_cmt_excp_addr = (ld1_cmt_excp || (st_cmt_excp && (!lpcr_q.unalign || (lpcr_q.unalign && lpcr2_q.number)))) ? lpcr_q.addr : 
(ld2_cmt_excp || (st_cmt_excp && lpcr_q.unalign && !lpcr2_q.number)) ? sec_req_addr : 32'b0; 
assign lsu_pcu_cmt_trig_dbg = (ld1_wb || st_wb) && (lpcr_q.tm_req_dbg && wb_tm_hit); 
assign lsu_pcu_inst_rdy = (ld1_idle && ld2_idle && st_idle) || lsu_pcu_cmt; 
assign cmt_ex_excp_code = lpcr2_q.excp_code; 
assign wb_tm_hit = (lpcr_q.tm_hit || wb_tm_req_dbg) 
; 
assign cmt_excp_brkp = wb_tm_hit; 
assign mmr_berr2acfault = mss_dbus_err && lpcr_q.req_in_mmr; 
assign cmt_excp_bus_err = mss_dbus_err && !lpcr_q.req_in_mmr; 
assign cmt_excp_misalign = cmt_ex_excp_code == EX_EXCP_MISALIGN; 
assign cmt_excp_acfault = cmt_ex_excp_code == EX_EXCP_ACCESS_FAULT || mmr_berr2acfault; 
assign cmt_excp_misalign_ld = cmt_excp_misalign && (lpcr_q.cmd == LSU_OP_LOAD); 
assign cmt_excp_misalign_st = cmt_excp_misalign && (lpcr_q.cmd == LSU_OP_STORE || lpcr_q.cmd == LSU_OP_AMO); 
assign cmt_excp_acfault_ld = cmt_excp_acfault && (lpcr_q.cmd == LSU_OP_LOAD); 
assign cmt_excp_acfault_st = cmt_excp_acfault && (lpcr_q.cmd == LSU_OP_STORE || lpcr_q.cmd == LSU_OP_AMO); 
always_comb begin 
casez({cmt_excp_brkp, cmt_excp_acfault_ld, cmt_excp_acfault_st, cmt_excp_misalign_ld, cmt_excp_misalign_st, cmt_excp_bus_err}) 
6'b1????? : cmt_excp_code = EXCP_BRKP; 
6'b01???? : cmt_excp_code = EXCP_LD_ACCESS_FAUTL; 
6'b001??? : cmt_excp_code = EXCP_ST_ACCESS_FAUTL; 
6'b0001?? : cmt_excp_code = EXCP_LD_MISALIGN; 
6'b00001? : cmt_excp_code = EXCP_ST_MISALIGN; 
6'b000001 : cmt_excp_code = EXCP_RAS_ERR; 
default : cmt_excp_code = {32{1'b0}}; 
endcase 
end 
assign req_addr = pcu_inst_addr; 
assign req_width = pcu_lsu_inst_size; 
assign st_nonflush_infly = ((device_st 
) && !lpcr_has_excp) && !store_wait4write_q && st_wb; 
assign ld_device_infly = (ld1_wb && !lpcr_has_excp) && lpcr2_q.mem_attri[0] == 1'b1; 
assign fencei_infly = fencei_handling; 
assign lsu_pcu_non_flush_infly = ld_device_infly || fencei_infly || st_nonflush_infly; 
assign ld_route_addr = ({32{sel_pcu}} & pcu_req_addr) 
| ({32{sel_ld1}} & ld1_req_addr) 
| ({32{sel_fst}} & fst_req_addr) 
| ({32{sel_sec}} & sec_req_addr); 
assign route_addr = (st_wb && sec_req_go) ? sec_req_addr : ld_route_addr; 
assign bitband_base0_addr = 32'h2000_0000; 
assign bitband_base1_addr = 32'h4000_0000; 
assign dst_is_soc = !dst_is_itcm && !dst_is_dtcm; 
assign tcm_route_addr = {1'b0, route_addr[17:0]}; 
assign dst_is_mmr = route_addr[31:18] == core_mmr_base_addr[31:18]; 
assign dst_is_bitband = 1'b0; 
assign dst_is_dtcm = 1'b0; 
assign addr_match_dtcm = 1'b0; 
assign dtcm_acc_fault = 1'b0; 
assign dst_is_itcm = 1'b0; 
assign addr_match_itcm = 1'b0; 
assign itcm_acc_fault = 1'b0; 
assign addr_route_res = addr_match_itcm ? REG_ROUT_ITCM : 
addr_match_dtcm ? REG_ROUT_DTCM : 
dst_is_bitband ? REG_ROUT_BTBD : REG_ROUT_SOC; 
assign tcm_err = itcm_acc_fault || dtcm_acc_fault; 
`WDFFER(st_fst_berr_q, st_fst_berr_d, st_fst_berr_upd, clk, rst_n) 
assign st_fst_berr_upd = dbg_st_pop || lsu_flush; 
assign st_fst_berr_d = (dbg_st_pop && dbg_st_num_q && b_bus_err) ? 1'b1 : 1'b0; 
assign st_sync_bus_err = sync_store_at_sbu_head && mss_lsu_b_valid && (st_fst_berr_q || b_bus_err); 
assign b_bus_err = 
(mss_lsu_b_resp != MSS_RESP_OKAY) 
; 
assign lsu_idle = lsu_fsm_idle && sbu_empty; 
assign lsu_mss_load_flush = lsu_flush; 
endmodule
 
 
module m130_lsu_sbu #( 
parameter type T = logic [63:0], 
parameter DATA_WIDTH = 64, 
parameter BUFF_DEPTH = 2, 
parameter SBU_PTR_W = 1 
) ( 
input logic clk, 
input logic rst_n, 
input logic wr_en, 
input T wdata, 
input logic head_pop, 
input logic sbu_addr_sent, 
input logic sbu_data_sent, 
input logic unalign_st_fst, 
input logic unalign_st_sec, 
input logic flush, 
output T rdata [BUFF_DEPTH-1:0], 
output logic rdata_valid [BUFF_DEPTH-1:0], 
output logic[SBU_PTR_W-1:0] send_ptr, 
output logic[SBU_PTR_W-1:0] head_ptr, 
output logic is_full, 
output logic is_empty, 
output logic stop_send 
); 
localparam PTR_W = $clog2(BUFF_DEPTH); 
localparam logic [PTR_W-1:0] FIFO_MAX = BUFF_DEPTH - 1; 
logic wr_sign; 
logic wr_sign_upd; 
logic wr_sign_d; 
logic head_sign; 
logic [PTR_W-1:0] wr_ptr_d; 
logic [PTR_W-1:0] wr_ptr_q; 
logic wr_ptr_upd; 
logic [PTR_W-1:0] head_ptr_d; 
logic [PTR_W-1:0] head_ptr_q; 
logic head_ptr_upd; 
logic write_to_non_flush; 
logic send_sign; 
logic [PTR_W-1:0] send_ptr_d; 
logic [PTR_W-1:0] send_ptr_q; 
logic send_ptr_upd; 
logic all_sent; 
logic non_flush_sign; 
logic non_flush_sign_d; 
logic non_flush_sign_upd; 
logic [PTR_W-1:0] non_flush_ptr_d; 
logic [PTR_W-1:0] non_flush_ptr_q; 
logic non_flush_ptr_upd; 
logic non_flush_to_write; 
T sbu [BUFF_DEPTH-1:0]; 
T sbu_upd_data [BUFF_DEPTH-1:0]; 
logic sbu_upd [BUFF_DEPTH-1:0]; 
logic sbu_valid_q [BUFF_DEPTH-1:0]; 
logic sbu_valid_d [BUFF_DEPTH-1:0]; 
logic sbu_valid_upd [BUFF_DEPTH-1:0]; 
logic sbu_parity_q [BUFF_DEPTH-1:0]; 
logic sbu_parity_d [BUFF_DEPTH-1:0]; 
logic sbu_parity_upd [BUFF_DEPTH-1:0]; 
logic check_parity_q; 
T sbu_wdata; 
logic sbu_wr; 
logic sbu_pop; 
logic sbu_wr_hs; 
logic wr_ptr_step; 
logic [BUFF_DEPTH-1:0] parity_err_vec; 
assign is_full = (head_ptr_q == wr_ptr_q) && (wr_sign != head_sign); 
assign is_empty = (head_ptr_q == wr_ptr_q) && (wr_sign == head_sign); 
assign all_sent = (send_ptr_q == wr_ptr_q) && (wr_sign == send_sign); 
assign stop_send = (send_ptr_q == non_flush_ptr_q) && (non_flush_sign == send_sign); 
assign wr_ptr_d = write_to_non_flush ? non_flush_ptr_q : ((wr_ptr_q == FIFO_MAX) ? {PTR_W{1'b0}} : (wr_ptr_q + 1'b1)); 
assign head_ptr_d = (head_ptr_q == FIFO_MAX) ? {PTR_W{1'b0}} : (head_ptr_q + 1'b1); 
assign write_to_non_flush = flush && !(wr_ptr_q == non_flush_ptr_q && wr_sign == non_flush_sign); 
assign sbu_wr_hs = wr_en && (!is_full 
|| head_ptr_upd 
); 
assign wr_ptr_step = sbu_wr_hs; 
assign wr_ptr_upd = wr_ptr_step || write_to_non_flush; 
assign wr_sign_upd = (wr_ptr_step && (wr_ptr_q == FIFO_MAX)) || write_to_non_flush; 
assign head_ptr_upd = head_pop && !is_empty; 
assign wr_sign_d = write_to_non_flush ? non_flush_sign : ~wr_sign; 
`WDFFER(wr_ptr_q, wr_ptr_d, wr_ptr_upd, clk, rst_n) 
`WDFFER(head_ptr_q, head_ptr_d, head_ptr_upd, clk, rst_n) 
`WDFFER(wr_sign, wr_sign_d, wr_sign_upd, clk, rst_n) 
`WDFFER(head_sign, ~head_sign, head_ptr_upd && (head_ptr_q == FIFO_MAX), clk, rst_n) 
assign send_ptr_d = (send_ptr_q == FIFO_MAX) ? {PTR_W{1'b0}} : (send_ptr_q + 1'b1); 
assign send_ptr_upd = (sbu_valid_q[send_ptr_q] && (sbu_addr_sent && sbu_data_sent)) && !all_sent; 
`WDFFER(send_ptr_q, send_ptr_d, send_ptr_upd, clk, rst_n) 
`WDFFER(send_sign, ~send_sign, send_ptr_upd && (send_ptr_q == FIFO_MAX), clk, rst_n) 
assign non_flush_ptr_d = non_flush_to_write ? wr_ptr_d : 
(non_flush_ptr_q == FIFO_MAX) ? {PTR_W{1'b0}} : (non_flush_ptr_q + 1'b1); 
assign non_flush_ptr_upd = sbu_wr_hs && !unalign_st_fst; 
assign non_flush_to_write = sbu_wr_hs && unalign_st_sec; 
assign non_flush_sign_d = non_flush_to_write ? (wr_sign_upd ? ~wr_sign : wr_sign) : ~non_flush_sign; 
assign non_flush_sign_upd = non_flush_to_write ? 1'b1 : non_flush_ptr_upd && (non_flush_ptr_q == FIFO_MAX); 
`WDFFER(non_flush_ptr_q, non_flush_ptr_d, non_flush_ptr_upd, clk, rst_n) 
`WDFFER(non_flush_sign, non_flush_sign_d, non_flush_sign_upd, clk, rst_n) 
assign send_ptr = send_ptr_q; 
assign head_ptr = head_ptr_q; 
assign sbu_wr = wr_ptr_upd && !write_to_non_flush; 
assign sbu_pop = head_ptr_upd; 
for(genvar i=0; i<2; i++) begin : sbu_gen_entries 
assign sbu_valid_upd[i] = (sbu_wr && wr_ptr_q == i) 
|| (sbu_pop && head_ptr_q == i) 
|| (write_to_non_flush && non_flush_ptr_q == i); 
assign sbu_valid_d[i] = ((sbu_wr && wr_ptr_q == i) & 1'b1) 
| ((write_to_non_flush && non_flush_ptr_q == i) & 1'b0); 
`WDFFER(sbu_valid_q[i], sbu_valid_d[i], sbu_valid_upd[i], clk, rst_n) 
assign sbu_upd[i] = (sbu_wr && wr_ptr_q == i) 
; 
assign sbu_upd_data[i] = wdata 
; 
`WDFFENR(sbu[i], sbu_upd_data[i], sbu_upd[i], clk) 
end 
assign rdata = sbu; 
assign rdata_valid = sbu_valid_q; 
endmodule
 
 
module m130_pmp ( 
input logic clk, 
input logic rst_n, 
input logic pmp_cfg_clk, 
output logic pmp_cfg_clk_en, 
input logic [1 : 0] core_priv_mode, 
input logic pcu_csr_mprv, 
input logic [1 : 0] pcu_csr_mpp, 
input logic pcu_csr_vld, 
input logic [1 : 0] pcu_csr_opcode, 
input logic [11 : 0] pcu_csr_index, 
input logic [32 - 1 : 0] pcu_csr_wdata, 
output logic pmp_pcu_csr_done, 
output logic [32 - 1 : 0] pmp_pcu_csr_rdata, 
output logic pmp_pcu_csr_excp, 
output logic pmp_pcu_csr_match, 
input logic lsu_pmp_vld, 
input logic [32 - 1 : 0] lsu_pmp_addr, 
input logic lsu_pmp_rw, 
output logic pmp_lsu_err, 
output logic pma_lsu_idempotency_n, 
output logic pma_lsu_cacheable, 
input logic ifu_pmp_vld, 
input logic [32 - 1 : 0] ifu_pmp_addr, 
output logic pmp_ifu_err, 
output logic pma_ifu_cacheable, 
output logic pma_ifu_err 
); 
 
 
typedef enum logic[1:0] { 
PRIV_MODE_M = 2'b11, 
PRIV_MODE_S = 2'b01, 
PRIV_MODE_U = 2'b00 
} priv_mode_e; 
typedef enum logic[1:0] { 
CSR_OP_READ = 2'b00, 
CSR_OP_WRITE = 2'b01, 
CSR_OP_SET = 2'b10, 
CSR_OP_CLEAR = 2'b11 
} csr_opcode_e; 
typedef enum logic [11:0] { 
CSR_FFLAGS_ADDR = 12'h001, 
CSR_FRM_ADDR = 12'h002, 
CSR_FCSR_ADDR = 12'h003, 
CSR_FTRAN_ADDR = 12'h800, 
CSR_SSTATUS_ADDR = 12'h100, 
CSR_SIE_ADDR = 12'h104, 
CSR_STVEC_ADDR = 12'h105, 
CSR_SCOUNTEREN_ADDR = 12'h106, 
CSR_SSCRATCH_ADDR = 12'h140, 
CSR_SEPC_ADDR = 12'h141, 
CSR_SCAUSE_ADDR = 12'h142, 
CSR_STVAL_ADDR = 12'h143, 
CSR_SIP_ADDR = 12'h144, 
CSR_SATP_ADDR = 12'h180, 
CSR_MSTATUS_ADDR = 12'h300, 
CSR_MISA_ADDR = 12'h301, 
CSR_MEDELEG_ADDR = 12'h302, 
CSR_MIDELEG_ADDR = 12'h303, 
CSR_MIE_ADDR = 12'h304, 
CSR_MTVEC_ADDR = 12'h305, 
CSR_MCOUNTEREN_ADDR = 12'h306, 
CSR_MSTATUSH_ADDR = 12'h310, 
CSR_MSCRATCH_ADDR = 12'h340, 
CSR_MEPC_ADDR = 12'h341, 
CSR_MCAUSE_ADDR = 12'h342, 
CSR_MTVAL_ADDR = 12'h343, 
CSR_MIP_ADDR = 12'h344, 
CSR_MENVCFG_ADDR = 12'h30A, 
CSR_MENVCFGH_ADDR = 12'h31A, 
CSR_MSECCFG_ADDR = 12'h747, 
CSR_MSECCFGH_ADDR = 12'h757, 
CSR_PMPCFG0_ADDR = 12'h3A0, 
CSR_PMPCFG1_ADDR = 12'h3A1, 
CSR_PMPCFG2_ADDR = 12'h3A2, 
CSR_PMPCFG3_ADDR = 12'h3A3, 
CSR_PMPADDR0_ADDR = 12'h3B0, 
CSR_PMPADDR1_ADDR = 12'h3B1, 
CSR_PMPADDR2_ADDR = 12'h3B2, 
CSR_PMPADDR3_ADDR = 12'h3B3, 
CSR_PMPADDR4_ADDR = 12'h3B4, 
CSR_PMPADDR5_ADDR = 12'h3B5, 
CSR_PMPADDR6_ADDR = 12'h3B6, 
CSR_PMPADDR7_ADDR = 12'h3B7, 
CSR_PMPADDR8_ADDR = 12'h3B8, 
CSR_PMPADDR9_ADDR = 12'h3B9, 
CSR_PMPADDR10_ADDR = 12'h3BA, 
CSR_PMPADDR11_ADDR = 12'h3BB, 
CSR_PMPADDR12_ADDR = 12'h3BC, 
CSR_PMPADDR13_ADDR = 12'h3BD, 
CSR_PMPADDR14_ADDR = 12'h3BE, 
CSR_PMPADDR15_ADDR = 12'h3BF, 
CSR_MVENDORID_ADDR = 12'hF11, 
CSR_MARCHID_ADDR = 12'hF12, 
CSR_MIMPID_ADDR = 12'hF13, 
CSR_MHARTID_ADDR = 12'hF14, 
CSR_MCONFIGPTRID_ADDR = 12'hF15, 
CSR_MCYCLE_ADDR = 12'hB00, 
CSR_MCYCLEH_ADDR = 12'hB80, 
CSR_MINSTRET_ADDR = 12'hB02, 
CSR_MINSTRETH_ADDR = 12'hB82, 
CSR_MHPM_COUNTER_3_ADDR = 12'hB03, 
CSR_MHPM_COUNTER_4_ADDR = 12'hB04, 
CSR_MHPM_COUNTER_5_ADDR = 12'hB05, 
CSR_MHPM_COUNTER_6_ADDR = 12'hB06, 
CSR_MHPM_COUNTER_7_ADDR = 12'hB07, 
CSR_MHPM_COUNTER_8_ADDR = 12'hB08, 
CSR_MHPM_COUNTER_9_ADDR = 12'hB09, 
CSR_MHPM_COUNTER_10_ADDR = 12'hB0A, 
CSR_MHPM_COUNTER_11_ADDR = 12'hB0B, 
CSR_MHPM_COUNTER_12_ADDR = 12'hB0C, 
CSR_MHPM_COUNTER_13_ADDR = 12'hB0D, 
CSR_MHPM_COUNTER_14_ADDR = 12'hB0E, 
CSR_MHPM_COUNTER_15_ADDR = 12'hB0F, 
CSR_MHPM_COUNTER_16_ADDR = 12'hB10, 
CSR_MHPM_COUNTER_17_ADDR = 12'hB11, 
CSR_MHPM_COUNTER_18_ADDR = 12'hB12, 
CSR_MHPM_COUNTER_19_ADDR = 12'hB13, 
CSR_MHPM_COUNTER_20_ADDR = 12'hB14, 
CSR_MHPM_COUNTER_21_ADDR = 12'hB15, 
CSR_MHPM_COUNTER_22_ADDR = 12'hB16, 
CSR_MHPM_COUNTER_23_ADDR = 12'hB17, 
CSR_MHPM_COUNTER_24_ADDR = 12'hB18, 
CSR_MHPM_COUNTER_25_ADDR = 12'hB19, 
CSR_MHPM_COUNTER_26_ADDR = 12'hB1A, 
CSR_MHPM_COUNTER_27_ADDR = 12'hB1B, 
CSR_MHPM_COUNTER_28_ADDR = 12'hB1C, 
CSR_MHPM_COUNTER_29_ADDR = 12'hB1D, 
CSR_MHPM_COUNTER_30_ADDR = 12'hB1E, 
CSR_MHPM_COUNTER_31_ADDR = 12'hB1F, 
CSR_MHPM_COUNTER_3H_ADDR = 12'hB83, 
CSR_MHPM_COUNTER_4H_ADDR = 12'hB84, 
CSR_MHPM_COUNTER_5H_ADDR = 12'hB85, 
CSR_MHPM_COUNTER_6H_ADDR = 12'hB86, 
CSR_MHPM_COUNTER_7H_ADDR = 12'hB87, 
CSR_MHPM_COUNTER_8H_ADDR = 12'hB88, 
CSR_MHPM_COUNTER_9H_ADDR = 12'hB89, 
CSR_MHPM_COUNTER_10H_ADDR = 12'hB8A, 
CSR_MHPM_COUNTER_11H_ADDR = 12'hB8B, 
CSR_MHPM_COUNTER_12H_ADDR = 12'hB8C, 
CSR_MHPM_COUNTER_13H_ADDR = 12'hB8D, 
CSR_MHPM_COUNTER_14H_ADDR = 12'hB8E, 
CSR_MHPM_COUNTER_15H_ADDR = 12'hB8F, 
CSR_MHPM_COUNTER_16H_ADDR = 12'hB90, 
CSR_MHPM_COUNTER_17H_ADDR = 12'hB91, 
CSR_MHPM_COUNTER_18H_ADDR = 12'hB92, 
CSR_MHPM_COUNTER_19H_ADDR = 12'hB93, 
CSR_MHPM_COUNTER_20H_ADDR = 12'hB94, 
CSR_MHPM_COUNTER_21H_ADDR = 12'hB95, 
CSR_MHPM_COUNTER_22H_ADDR = 12'hB96, 
CSR_MHPM_COUNTER_23H_ADDR = 12'hB97, 
CSR_MHPM_COUNTER_24H_ADDR = 12'hB98, 
CSR_MHPM_COUNTER_25H_ADDR = 12'hB99, 
CSR_MHPM_COUNTER_26H_ADDR = 12'hB9A, 
CSR_MHPM_COUNTER_27H_ADDR = 12'hB9B, 
CSR_MHPM_COUNTER_28H_ADDR = 12'hB9C, 
CSR_MHPM_COUNTER_29H_ADDR = 12'hB9D, 
CSR_MHPM_COUNTER_30H_ADDR = 12'hB9E, 
CSR_MHPM_COUNTER_31H_ADDR = 12'hB9F, 
CSR_MCOUNTINHIBIT_ADDR = 12'h320, 
CSR_MHPM_EVENT_3_ADDR = 12'h323, 
CSR_MHPM_EVENT_4_ADDR = 12'h324, 
CSR_MHPM_EVENT_5_ADDR = 12'h325, 
CSR_MHPM_EVENT_6_ADDR = 12'h326, 
CSR_MHPM_EVENT_7_ADDR = 12'h327, 
CSR_MHPM_EVENT_8_ADDR = 12'h328, 
CSR_MHPM_EVENT_9_ADDR = 12'h329, 
CSR_MHPM_EVENT_10_ADDR = 12'h32a, 
CSR_MHPM_EVENT_11_ADDR = 12'h32b, 
CSR_MHPM_EVENT_12_ADDR = 12'h32c, 
CSR_MHPM_EVENT_13_ADDR = 12'h32d, 
CSR_MHPM_EVENT_14_ADDR = 12'h32e, 
CSR_MHPM_EVENT_15_ADDR = 12'h32f, 
CSR_MHPM_EVENT_16_ADDR = 12'h330, 
CSR_MHPM_EVENT_17_ADDR = 12'h331, 
CSR_MHPM_EVENT_18_ADDR = 12'h332, 
CSR_MHPM_EVENT_19_ADDR = 12'h333, 
CSR_MHPM_EVENT_20_ADDR = 12'h334, 
CSR_MHPM_EVENT_21_ADDR = 12'h335, 
CSR_MHPM_EVENT_22_ADDR = 12'h336, 
CSR_MHPM_EVENT_23_ADDR = 12'h337, 
CSR_MHPM_EVENT_24_ADDR = 12'h338, 
CSR_MHPM_EVENT_25_ADDR = 12'h339, 
CSR_MHPM_EVENT_26_ADDR = 12'h33a, 
CSR_MHPM_EVENT_27_ADDR = 12'h33b, 
CSR_MHPM_EVENT_28_ADDR = 12'h33c, 
CSR_MHPM_EVENT_29_ADDR = 12'h33d, 
CSR_MHPM_EVENT_30_ADDR = 12'h33e, 
CSR_MHPM_EVENT_31_ADDR = 12'h33f, 
CSR_MTVT_ADDR = 12'h307, 
CSR_MNXTI_ADDR = 12'h345, 
CSR_MINTSTATUS_ADDR = 12'hfb1, 
CSR_MINTTHRESH_ADDR = 12'h347, 
CSR_MSCRATCHCSW_ADDR = 12'h348, 
CSR_MSCRATCHCSWL_ADDR = 12'h349, 
CSR_MNSCRATCH_ADDR = 12'h740, 
CSR_MNEPC_ADDR = 12'h741, 
CSR_MNCAUSE_ADDR = 12'h742, 
CSR_MNSTATUS_ADDR = 12'h744, 
CSR_MNVEC_ADDR = 12'hFC0, 
CSR_TSELECT_ADDR = 12'h7A0, 
CSR_TDATA1_ADDR = 12'h7A1, 
CSR_TDATA2_ADDR = 12'h7A2, 
CSR_TDATA3_ADDR = 12'h7A3, 
CSR_TINFO_ADDR = 12'h7A4, 
CSR_TCONTROL_ADDR = 12'h7A5, 
CSR_DCSR_ADDR = 12'h7b0, 
CSR_DPC_ADDR = 12'h7b1, 
CSR_DSCRATCH0_ADDR = 12'h7b2, 
CSR_DSCRATCH1_ADDR = 12'h7b3, 
CSR_DSCRATCH2_ADDR = 12'h7c0, 
CSR_DSCRATCH3_ADDR = 12'h7c1, 
CSR_MCONTEXTSW_ADDR = 12'h7c2, 
CSR_MSECURFEAT_ADDR = 12'h7c3, 
CSR_MLPCFG_ADDR = 12'h7c4, 
CSR_CYCLE_ADDR = 12'hC00, 
CSR_CYCLEH_ADDR = 12'hC80, 
CSR_TIME_ADDR = 12'hC01, 
CSR_TIMEH_ADDR = 12'hC81, 
CSR_INSTRET_ADDR = 12'hC02, 
CSR_INSTRETH_ADDR = 12'hC82, 
CSR_HPM_COUNTER_3_ADDR = 12'hC03, 
CSR_HPM_COUNTER_4_ADDR = 12'hC04, 
CSR_HPM_COUNTER_5_ADDR = 12'hC05, 
CSR_HPM_COUNTER_6_ADDR = 12'hC06, 
CSR_HPM_COUNTER_7_ADDR = 12'hC07, 
CSR_HPM_COUNTER_8_ADDR = 12'hC08, 
CSR_HPM_COUNTER_9_ADDR = 12'hC09, 
CSR_HPM_COUNTER_10_ADDR = 12'hC0A, 
CSR_HPM_COUNTER_11_ADDR = 12'hC0B, 
CSR_HPM_COUNTER_12_ADDR = 12'hC0C, 
CSR_HPM_COUNTER_13_ADDR = 12'hC0D, 
CSR_HPM_COUNTER_14_ADDR = 12'hC0E, 
CSR_HPM_COUNTER_15_ADDR = 12'hC0F, 
CSR_HPM_COUNTER_16_ADDR = 12'hC10, 
CSR_HPM_COUNTER_17_ADDR = 12'hC11, 
CSR_HPM_COUNTER_18_ADDR = 12'hC12, 
CSR_HPM_COUNTER_19_ADDR = 12'hC13, 
CSR_HPM_COUNTER_20_ADDR = 12'hC14, 
CSR_HPM_COUNTER_21_ADDR = 12'hC15, 
CSR_HPM_COUNTER_22_ADDR = 12'hC16, 
CSR_HPM_COUNTER_23_ADDR = 12'hC17, 
CSR_HPM_COUNTER_24_ADDR = 12'hC18, 
CSR_HPM_COUNTER_25_ADDR = 12'hC19, 
CSR_HPM_COUNTER_26_ADDR = 12'hC1A, 
CSR_HPM_COUNTER_27_ADDR = 12'hC1B, 
CSR_HPM_COUNTER_28_ADDR = 12'hC1C, 
CSR_HPM_COUNTER_29_ADDR = 12'hC1D, 
CSR_HPM_COUNTER_30_ADDR = 12'hC1E, 
CSR_HPM_COUNTER_31_ADDR = 12'hC1F, 
CSR_HPM_COUNTER_3H_ADDR = 12'hC83, 
CSR_HPM_COUNTER_4H_ADDR = 12'hC84, 
CSR_HPM_COUNTER_5H_ADDR = 12'hC85, 
CSR_HPM_COUNTER_6H_ADDR = 12'hC86, 
CSR_HPM_COUNTER_7H_ADDR = 12'hC87, 
CSR_HPM_COUNTER_8H_ADDR = 12'hC88, 
CSR_HPM_COUNTER_9H_ADDR = 12'hC89, 
CSR_HPM_COUNTER_10H_ADDR = 12'hC8A, 
CSR_HPM_COUNTER_11H_ADDR = 12'hC8B, 
CSR_HPM_COUNTER_12H_ADDR = 12'hC8C, 
CSR_HPM_COUNTER_13H_ADDR = 12'hC8D, 
CSR_HPM_COUNTER_14H_ADDR = 12'hC8E, 
CSR_HPM_COUNTER_15H_ADDR = 12'hC8F, 
CSR_HPM_COUNTER_16H_ADDR = 12'hC90, 
CSR_HPM_COUNTER_17H_ADDR = 12'hC91, 
CSR_HPM_COUNTER_18H_ADDR = 12'hC92, 
CSR_HPM_COUNTER_19H_ADDR = 12'hC93, 
CSR_HPM_COUNTER_20H_ADDR = 12'hC94, 
CSR_HPM_COUNTER_21H_ADDR = 12'hC95, 
CSR_HPM_COUNTER_22H_ADDR = 12'hC96, 
CSR_HPM_COUNTER_23H_ADDR = 12'hC97, 
CSR_HPM_COUNTER_24H_ADDR = 12'hC98, 
CSR_HPM_COUNTER_25H_ADDR = 12'hC99, 
CSR_HPM_COUNTER_26H_ADDR = 12'hC9A, 
CSR_HPM_COUNTER_27H_ADDR = 12'hC9B, 
CSR_HPM_COUNTER_28H_ADDR = 12'hC9C, 
CSR_HPM_COUNTER_29H_ADDR = 12'hC9D, 
CSR_HPM_COUNTER_30H_ADDR = 12'hC9E, 
CSR_HPM_COUNTER_31H_ADDR = 12'hC9F, 
CSR_JVT_ADDR = 12'h017 
} csr_reg_t; 
 
logic pma_lsu_err; 
localparam int unsigned PMP_ENTRY_N = 8; 
localparam int unsigned PMP_GRAIN_W = 3; 
localparam int unsigned PMPADDR_CUT_W = 32-2; 
localparam int unsigned PMP_MAX_NUM = 64; 
localparam int unsigned PMP_CFG_MAX_NUM = PMP_MAX_NUM; 
localparam int unsigned PMP_CFG_CSR_W = 8; 
localparam int unsigned PMP_ADDR_CSR_W = PMPADDR_CUT_W - PMP_GRAIN_W + 1; 
typedef enum logic [1:0] { 
PMP_A_OFF = 2'b00, 
PMP_A_TOR = 2'b01, 
PMP_A_NA4 = 2'b10, 
PMP_A_NAPOT = 2'b11 
} pmp_addr_match_e; 
typedef struct packed { 
logic x; 
logic w; 
logic r; 
} pmp_xwr_s; 
typedef struct packed { 
logic l; 
logic [1:0] reserved; 
pmp_addr_match_e a; 
pmp_xwr_s xwr; 
} pmp_cfg_s; 
logic [11:0] csr_addr; 
logic csr_wr_vld; 
logic [32 - 1 : 0] csr_wr_data; 
logic [32 - 1 : 0] csr_wr_data_refined; 
logic [PMP_MAX_NUM - 1 : 0] csr_sel_pmpcfg; 
logic [PMP_MAX_NUM - 1 : 0] csr_sel_pmpaddr; 
logic [32 - 1 : 0] cfg_or_result; 
logic [32 - 1 : 0] addr_or_result; 
logic [32 - 1 : 0][PMP_CFG_MAX_NUM / 4 - 1 : 0] csr_cfg_rotate; 
logic [32 - 1 : 0][PMP_CFG_MAX_NUM - 1 : 0] csr_addr_rotate; 
logic addr_base_match_addr; 
logic addr_base_match_cfg; 
logic [PMP_ENTRY_N - 1 : 0] ff_pmpcfg_en; 
pmp_cfg_s [PMP_CFG_MAX_NUM - 1 : 0] ff_pmpcfg_d ; 
pmp_cfg_s [PMP_CFG_MAX_NUM - 1 : 0] ff_pmpcfg_q ; 
logic [PMP_ENTRY_N - 1 : 0] ff_pmpaddr_en; 
logic [PMP_ENTRY_N - 1 : 0][PMPADDR_CUT_W - 1 : PMP_GRAIN_W - 1] ff_pmpaddr_d; 
logic [PMP_MAX_NUM - 1 : 0][PMPADDR_CUT_W - 1 : PMP_GRAIN_W - 1] ff_pmpaddr_q; 
logic [PMP_CFG_MAX_NUM / 4 - 1 : 0][32 - 1 : 0] pmpcfg_readout; 
logic [PMP_MAX_NUM - 1 : 0][32 - 1 : 0] pmpaddr_readout; 
logic [PMP_ENTRY_N - 1 : 0][PMPADDR_CUT_W - 1 : 0] pmpaddr_napot; 
logic [PMP_ENTRY_N - 1 : 0][PMPADDR_CUT_W - 1 : PMP_GRAIN_W - 1] pmpaddr_napot_mask; 
logic [PMP_ENTRY_N-1:0][PMPADDR_CUT_W-1:0] pmpaddr_tor; 
logic [PMP_ENTRY_N-1:0][PMPADDR_CUT_W-1:0] pmpaddr_off; 
logic [PMPADDR_CUT_W - 1 : 0] lsu2pmp_addr_cut; 
logic [PMPADDR_CUT_W - 1 : 0] ifu2pmp_addr_cut; 
logic [PMP_ENTRY_N - 1 : 0] lsuaddr_lt_pmpaddr; 
logic [PMP_ENTRY_N - 1 : 0] lsuaddr_ge_pmpaddr_prcd; 
logic [PMP_ENTRY_N - 1 : 0] ifuaddr_lt_pmpaddr; 
logic [PMP_ENTRY_N - 1 : 0] ifuaddr_ge_pmpaddr_prcd; 
logic [PMP_ENTRY_N - 1 : 0] lsu_addr_match_tor; 
logic [PMP_ENTRY_N - 1 : 0] ifu_addr_match_tor; 
logic [PMP_ENTRY_N - 1 : 0] lsu_addr_match_napot; 
logic [PMP_ENTRY_N - 1 : 0] ifu_addr_match_napot; 
logic [PMP_ENTRY_N - 1 : 0] lsu_addr_match; 
logic [PMP_ENTRY_N - 1 : 0] ifu_addr_match; 
logic [PMP_ENTRY_N - 1 : 0] lsu_xrw_check; 
logic [PMP_ENTRY_N - 1 : 0] ifu_xrw_check; 
logic [PMP_ENTRY_N - 1 : 0] lsu_addr_match_onehot_val; 
logic [PMP_ENTRY_N - 1 : 0] ifu_addr_match_onehot_val; 
logic csr_access_fault; 
logic [1 : 0] ldst_priv_mode; 
logic lsu_grt_err; 
logic ifu_grt_err; 
assign lsu2pmp_addr_cut = lsu_pmp_addr[32-1:2]; 
assign ifu2pmp_addr_cut = ifu_pmp_addr[32-1:2]; 
assign csr_access_fault = ~(core_priv_mode == PRIV_MODE_M); 
assign pmp_pcu_csr_excp = pcu_csr_vld & csr_access_fault; 
for (genvar i = 0; i < PMP_ENTRY_N; i++) begin : G_PMPADDR 
for (genvar j = 0; j < PMPADDR_CUT_W; j++) begin: G_PMPADDR_2 
if (j > PMP_GRAIN_W-2) begin: G_PMPADDR_3 
assign pmpaddr_napot[i][j] = ff_pmpaddr_q[i][j]; 
end else begin: G_PMPADDR_5 
assign pmpaddr_napot[i][j] = 1'b1; 
end 
if (j > PMP_GRAIN_W-1) begin: G_PMPADDR_4 
assign pmpaddr_tor[i][j] = ff_pmpaddr_q[i][j]; 
end else begin: G_PMPADDR_6 
assign pmpaddr_tor[i][j] = 1'b0; 
end 
end 
assign pmpaddr_readout[i] = (ff_pmpcfg_q[i].a[1] == 1'b1) ? {2'b00, pmpaddr_napot[i]} : 
{2'b00, pmpaddr_tor[i]}; 
end 
for (genvar i = 0; i < PMP_CFG_MAX_NUM / 4; i++) begin: G_ARR_MASKED_CFG 
assign pmpcfg_readout[i] = {ff_pmpcfg_q[i*4+3], ff_pmpcfg_q[i*4+2], ff_pmpcfg_q[i*4+1], ff_pmpcfg_q[i*4]}; 
for (genvar j = 0; j < 32; j++) begin : G_CSR_CFG_ROTATE 
assign csr_cfg_rotate[j][i] = pmpcfg_readout[i][j] & csr_sel_pmpcfg[i * 4]; 
end 
end 
for (genvar i = 0; i < PMP_CFG_MAX_NUM; i++) begin: G_ARR_MASKED_ADDR 
for (genvar j = 0; j < 32; j++) begin: G_ARR_MASKED_ADDR_2 
assign csr_addr_rotate[j][i] = pmpaddr_readout[i][j] & csr_sel_pmpaddr[i]; 
end 
end 
for (genvar j = 0; j < 32; j++) begin: G_ADDR_SEL 
assign addr_or_result[j] = | csr_addr_rotate[j][PMP_CFG_MAX_NUM - 1 : 0]; 
assign cfg_or_result[j] = | csr_cfg_rotate [j][PMP_CFG_MAX_NUM / 4 - 1 : 0]; 
end 
assign pmp_pcu_csr_rdata = addr_or_result | cfg_or_result; 
assign csr_wr_vld = (pcu_csr_opcode == CSR_OP_WRITE) 
& pcu_csr_vld 
& (!csr_access_fault); 
assign pmp_cfg_clk_en = csr_wr_vld; 
assign csr_wr_data = pcu_csr_wdata; 
for (genvar j = 0; j < 4; j++) begin: G_CSR_WR_DATA_REFINED 
assign csr_wr_data_refined[8*j+7] = csr_wr_data[8*j+7]; 
assign csr_wr_data_refined[8*j+6: 8*j+5] = 2'b00; 
assign csr_wr_data_refined[8*j+3 +: 2] = (csr_wr_data[8*j+3 +: 2] == PMP_A_NAPOT) ? 
pmp_addr_match_e'(csr_wr_data[8*j+3 +: 2]) : 
PMP_A_OFF; 
assign csr_wr_data_refined[8*j +: 3] = (csr_wr_data[8*j +: 2] == 2'b10) ? 
3'b000 : 
pmp_xwr_s'(csr_wr_data[8*j +: 3]); 
end 
for (genvar i = 0; i < PMP_ENTRY_N/4; i++) begin : G_PMPCFG_D 
for (genvar j = 0; j < 4; j++) begin : G_PMPCFG_D_2 
assign ff_pmpcfg_d[4*i+j].l = csr_wr_data_refined[8*j+7]; 
assign ff_pmpcfg_d[4*i+j].reserved = 2'b00; 
assign ff_pmpcfg_d[4*i+j].a = pmp_addr_match_e'(csr_wr_data_refined[8*j+3 +: 2]); 
assign ff_pmpcfg_d[4*i+j].xwr = pmp_xwr_s'(csr_wr_data_refined[8*j +: 3]); 
end 
end 
for (genvar i = 0; i < PMP_ENTRY_N; i++) begin : G_PMPADDR_D 
assign ff_pmpaddr_d[i] = csr_wr_data[32 - 3 : PMP_GRAIN_W - 1]; 
end 
assign csr_addr = pcu_csr_index; 
assign addr_base_match_cfg = (csr_addr[11 : 4] == 8'h3A); 
assign addr_base_match_addr = (csr_addr[11 : 7] == 5'h7); 
assign pmp_pcu_csr_match = (| csr_sel_pmpcfg) | (| csr_sel_pmpaddr); 
for (genvar i = 0; i < PMP_CFG_MAX_NUM; i++) begin: G_CSR_SEL 
if (i <= PMP_ENTRY_N-1) begin: G_CSR_SEL_PMPCFG 
assign csr_sel_pmpcfg[i] = addr_base_match_cfg & (csr_addr[3 : 0] == i/4); 
assign csr_sel_pmpaddr[i] = addr_base_match_addr & (csr_addr[6 : 0] == i + 6'h30); 
end else begin: G_CSR_SEL_PMPCFG_TIE_0 
assign csr_sel_pmpcfg[i] = 1'b0; 
assign csr_sel_pmpaddr[i] = 1'b0; 
end 
end 
logic [PMP_ENTRY_N - 1 : 0] ff_pmpaddr_back_lock; 
for (genvar i = 0; i < PMP_ENTRY_N; i++) begin : G_FF_PMP 
assign ff_pmpcfg_en[i] = csr_wr_vld & csr_sel_pmpcfg[i] & (~ff_pmpcfg_q[i].l); 
assign ff_pmpaddr_en[i] = csr_wr_vld & csr_sel_pmpaddr[i] & (~ff_pmpcfg_q[i].l); 
`WDFFER(ff_pmpcfg_q[i], ff_pmpcfg_d[i], ff_pmpcfg_en[i], pmp_cfg_clk, rst_n) 
`WDFFER(ff_pmpaddr_q[i],ff_pmpaddr_d[i], ff_pmpaddr_en[i], pmp_cfg_clk, rst_n) 
end 
for (genvar i = PMP_ENTRY_N; i < PMP_MAX_NUM; i++) begin : G_TIE0 
assign ff_pmpcfg_d[i] = '0; 
assign ff_pmpcfg_q[i] = '0; 
assign ff_pmpaddr_q[i] = '0; 
assign pmpaddr_readout[i] = '0; 
end 
for (genvar i = 0; i < PMP_ENTRY_N; i++) begin : G_ITR_ENT_ADDR 
for (genvar b = PMP_GRAIN_W-1; b < PMPADDR_CUT_W; b++) begin: G_PMPADDR_NAPOT_MASK 
assign pmpaddr_napot_mask[i][b] = ~&pmpaddr_napot[i][(b-1):0]; 
end 
assign lsu_addr_match_napot[i] = (lsu2pmp_addr_cut[PMPADDR_CUT_W-1:PMP_GRAIN_W-1] & pmpaddr_napot_mask[i]) == 
(pmpaddr_napot[i][PMPADDR_CUT_W-1:PMP_GRAIN_W-1] & pmpaddr_napot_mask[i]); 
assign ifu_addr_match_napot[i] = (ifu2pmp_addr_cut[PMPADDR_CUT_W-1:PMP_GRAIN_W-1] & pmpaddr_napot_mask[i]) == 
(pmpaddr_napot[i][PMPADDR_CUT_W-1:PMP_GRAIN_W-1] & pmpaddr_napot_mask[i]); 
assign lsu_addr_match[i] = (ff_pmpcfg_q[i].a == PMP_A_NAPOT) & lsu_addr_match_napot[i]; 
assign ifu_addr_match[i] = (ff_pmpcfg_q[i].a == PMP_A_NAPOT) & ifu_addr_match_napot[i]; 
assign lsu_xrw_check[i] = lsu_pmp_rw & ff_pmpcfg_q[i].xwr.w | 
~lsu_pmp_rw & ff_pmpcfg_q[i].xwr.r ; 
assign ifu_xrw_check[i] = ff_pmpcfg_q[i].xwr.x ; 
end 
assign ldst_priv_mode = ((core_priv_mode == PRIV_MODE_M) && pcu_csr_mprv) ? pcu_csr_mpp : 
core_priv_mode; 
logic [PMP_ENTRY_N - 1 : 0] lsu_m_err; 
logic [PMP_ENTRY_N - 1 : 0] ifu_m_err; 
logic [PMP_ENTRY_N - 1 : 0] lsu_u_err; 
logic [PMP_ENTRY_N - 1 : 0] ifu_u_err; 
for (genvar i = PMP_ENTRY_N-1; i >= 0; i--) begin: G_LSU_ERR 
assign lsu_m_err[i] = (ldst_priv_mode == PRIV_MODE_M) & ff_pmpcfg_q[i].l & ~lsu_xrw_check[i]; 
assign lsu_u_err[i] = (ldst_priv_mode != PRIV_MODE_M) & ~lsu_xrw_check[i]; 
end 
`WCBB_PRIORITY_QUEUE_LSB(lsu_addr_match, lsu_addr_match_onehot_val) 
always_comb begin 
lsu_grt_err = (| lsu_addr_match) ? (| (lsu_addr_match_onehot_val & (lsu_u_err | lsu_m_err))) 
: (ldst_priv_mode != PRIV_MODE_M); 
end 
for (genvar i = PMP_ENTRY_N-1; i >= 0; i--) begin: G_IFU_ERR 
assign ifu_m_err[i] = (core_priv_mode == PRIV_MODE_M) & ff_pmpcfg_q[i].l & ~ifu_xrw_check[i]; 
assign ifu_u_err[i] = (core_priv_mode != PRIV_MODE_M) & ~ifu_xrw_check[i]; 
end 
`WCBB_PRIORITY_QUEUE_LSB(ifu_addr_match, ifu_addr_match_onehot_val) 
always_comb begin 
ifu_grt_err = (| ifu_addr_match) ? (| (ifu_addr_match_onehot_val & (ifu_m_err | ifu_u_err))) 
: core_priv_mode != PRIV_MODE_M; 
end 
assign pmp_lsu_err = lsu_pmp_vld & (lsu_grt_err | pma_lsu_err); 
assign pmp_ifu_err = ifu_pmp_vld & ifu_grt_err; 
assign pmp_pcu_csr_done = 1'b1; 
//`include "wing_pma_define.svh" 
typedef struct packed { 
logic mode; 
logic [32 - 1 : 0] addr_base; 
logic [32 - 1 : 0] addr_end; 
logic [4 : 0] addr_size; 
logic cacheable; 
logic idempotency_n; 
logic vacant; 
logic r; 
logic w; 
logic x; 
} pma_entry_t; 
localparam pma_entry_t [12 - 1 : 0] PMA_ENTRY = '{ 
'{mode:1'b1, addr_base:32'h8000_0000, addr_end:32'hFFFF_FFFF, addr_size:5'd31, cacheable: 1'b0, idempotency_n: 1'b1, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b0}, 
'{mode:1'b1, addr_base:32'h4000_0000, addr_end:32'hFFFF_FFFF, addr_size:5'd30, cacheable: 1'b0, idempotency_n: 1'b1, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b0}, 
'{mode:1'b1, addr_base:32'h3000_0000, addr_end:32'hFFFF_FFFF, addr_size:5'd28, cacheable: 1'b0, idempotency_n: 1'b1, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b0}, 
'{mode:1'b1, addr_base:32'h2000_0000, addr_end:32'h2FFF_FFFF, addr_size:5'd28, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1800_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd27, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1400_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd26, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1200_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd25, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1100_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd24, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1080_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd23, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1040_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd22, cacheable: 1'b1, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h1000_0000, addr_end:32'h1FFF_FFFF, addr_size:5'd22, cacheable: 1'b0, idempotency_n: 1'b0, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b1}, 
'{mode:1'b1, addr_base:32'h0000_0000, addr_end:32'h0FFF_FFFF, addr_size:5'd28, cacheable: 1'b0, idempotency_n: 1'b1, vacant: 1'b0,r:1'b1,w:1'b1,x:1'b0} 
}; 


localparam int unsigned PMA_ENTRY_N = 12; 
logic [PMA_ENTRY_N - 1 : 0] pma_ifu_hit; 
logic [PMA_ENTRY_N - 1 : 0] pma_lsu_hit; 
logic [PMA_ENTRY_N - 1 : 0] pma_ifu_x_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_ifu_cacheable_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_lsu_cacheable_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_lsu_r_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_lsu_w_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_ifu_idenpotency_n_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_lsu_idempotency_n_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_ifu_vacant_temp; 
logic [PMA_ENTRY_N - 1 : 0] pma_lsu_vacant_temp; 
logic [PMA_ENTRY_N - 1 : 0][32 - 1 : 0] pma_ent_addr_base; 
logic [PMA_ENTRY_N - 1 : 0][32 - 1 : 0] pma_ent_addr_end; 
for (genvar k = 0; k < PMA_ENTRY_N; k++) begin : gen_pma_addr_hit 
assign pma_ent_addr_base[k] = PMA_ENTRY[k].addr_base; 
assign pma_ent_addr_end [k] = PMA_ENTRY[k].addr_end; 
assign pma_ifu_hit[k] = PMA_ENTRY[k].mode ? 
(ifu_pmp_addr[32 - 1 : PMA_ENTRY[k].addr_size] == pma_ent_addr_base[k][32 - 1 : PMA_ENTRY[k].addr_size]) : 
(ifu_pmp_addr >= pma_ent_addr_base[k]) && (ifu_pmp_addr <= pma_ent_addr_end[k]); 
assign pma_lsu_hit[k] = PMA_ENTRY[k].mode ? 
(lsu_pmp_addr[32 - 1 : PMA_ENTRY[k].addr_size] == pma_ent_addr_base[k][32 - 1 : PMA_ENTRY[k].addr_size]) : 
(lsu_pmp_addr >= pma_ent_addr_base[k]) && (lsu_pmp_addr <= pma_ent_addr_end[k]); 
end 
for (genvar j = 0; j < PMA_ENTRY_N; j++) begin : gen_pma_signal_modify 
assign pma_ifu_x_temp[j] = pma_ifu_hit[j] & PMA_ENTRY[j].x; 
assign pma_lsu_r_temp[j] = pma_lsu_hit[j] & PMA_ENTRY[j].r; 
assign pma_lsu_w_temp[j] = pma_lsu_hit[j] & PMA_ENTRY[j].w; 
assign pma_ifu_cacheable_temp[j] = pma_ifu_hit[j] & PMA_ENTRY[j].cacheable; 
assign pma_lsu_cacheable_temp[j] = pma_lsu_hit[j] & PMA_ENTRY[j].cacheable; 
assign pma_ifu_idenpotency_n_temp[j] = pma_ifu_hit[j] & PMA_ENTRY[j].idempotency_n; 
assign pma_lsu_idempotency_n_temp[j] = pma_lsu_hit[j] & PMA_ENTRY[j].idempotency_n; 
assign pma_ifu_vacant_temp[j] = pma_ifu_hit[j] & PMA_ENTRY[j].vacant; 
assign pma_lsu_vacant_temp[j] = pma_lsu_hit[j] & PMA_ENTRY[j].vacant; 
end 
assign pma_ifu_cacheable = | pma_ifu_cacheable_temp; 
assign pma_lsu_cacheable = | pma_lsu_cacheable_temp; 
assign pma_lsu_idempotency_n = | pma_lsu_idempotency_n_temp; 
assign pma_ifu_err = (| pma_ifu_idenpotency_n_temp) | (| pma_ifu_vacant_temp) | !(|pma_ifu_x_temp); 
assign pma_lsu_err = (| pma_lsu_vacant_temp) | (lsu_pmp_rw? !(| pma_lsu_w_temp) : !(| pma_lsu_r_temp)); 
endmodule
 
 
module m130_mss_top( 
output logic mss_pcu_icache_init_busy, 
output logic mss_idle , 
input logic pcu_dm_halted , 
input logic endianess , 
input logic mmr_req_vld , 
output logic mmr_req_rdy , 
input logic [31:0] mmr_req_addr , 
input logic mmr_req_write, 
input logic [32-1:0] mmr_req_wdata, 
output logic mmr_rsp_vld , 
input logic mmr_rsp_rdy , 
output logic [32-1:0] mmr_rsp_rdata, 
output logic mmr_rsp_err , 
input logic ifu_mss_ar_valid, 
output logic mss_ifu_ar_ready, 
input logic [32-1:0] ifu_mss_ar_addr, 
input logic [2:0] ifu_mss_ar_size, 
input logic [7:0] ifu_mss_ar_len, 
input logic [1:0] ifu_mss_ar_burst, 
input logic [2-1:0] ifu_mss_ar_id, 
input logic [3:0] ifu_mss_ar_cache, 
input logic ifu_mss_ar_lock, 
input logic [2:0] ifu_mss_ar_prot, 
input logic [4-1:0] ifu_mss_ar_user, 
output logic mss_ifu_r_valid, 
input logic ifu_mss_r_ready, 
output logic [64-1:0] mss_ifu_r_data, 
output logic [2-1:0] mss_ifu_r_id, 
output logic mss_ifu_r_last, 
output logic [1:0] mss_ifu_r_resp, 
output logic [33-1:0] mss_ifu_r_user, 
input logic ifu_mss_flush, 
input logic lsu_mss_ar_valid, 
output logic mss_lsu_ar_ready, 
input logic [32-1:0] lsu_mss_ar_addr, 
input logic [2:0] lsu_mss_ar_size, 
input logic [7:0] lsu_mss_ar_len, 
input logic [1:0] lsu_mss_ar_burst, 
input logic [2-1:0] lsu_mss_ar_id, 
input logic [3:0] lsu_mss_ar_cache, 
input logic lsu_mss_ar_lock, 
input logic [2:0] lsu_mss_ar_prot, 
input logic [5-1:0] lsu_mss_ar_user, 
input logic lsu_mss_aw_valid, 
output logic mss_lsu_aw_ready, 
input logic [32-1:0] lsu_mss_aw_addr, 
input logic [2:0] lsu_mss_aw_size, 
input logic [7:0] lsu_mss_aw_len, 
input logic [1:0] lsu_mss_aw_burst, 
input logic [2-1:0] lsu_mss_aw_id, 
input logic [3:0] lsu_mss_aw_cache, 
input logic lsu_mss_aw_lock, 
input logic [2:0] lsu_mss_aw_prot, 
input logic [5-1:0] lsu_mss_aw_user, 
input logic [5:0] lsu_mss_aw_atop, 
input logic lsu_mss_w_valid, 
output logic mss_lsu_w_ready, 
input logic [32-1:0] lsu_mss_w_data, 
input logic [32/8-1:0] lsu_mss_w_strb, 
input logic lsu_mss_w_last, 
output logic mss_lsu_r_valid, 
input logic lsu_mss_r_ready, 
output logic [32-1:0] mss_lsu_r_data, 
output logic [2-1:0] mss_lsu_r_id, 
output logic mss_lsu_r_last, 
output logic [1:0] mss_lsu_r_resp, 
output logic [1-1:0] mss_lsu_r_user, 
output logic mss_lsu_b_valid, 
input logic lsu_mss_b_ready, 
output logic [2-1:0] mss_lsu_b_id, 
output logic [1:0] mss_lsu_b_resp, 
input logic lsu_mss_fencei_req, 
output logic mss_lsu_fencei_done, 
input logic lsu_mss_load_flush, 
output logic icache_bmu_ar_valid, 
input logic bmu_icache_ar_ready, 
output logic [32-1:0] icache_bmu_ar_addr, 
output logic [2:0] icache_bmu_ar_size, 
output logic [7:0] icache_bmu_ar_len, 
output logic [1:0] icache_bmu_ar_burst, 
output logic [1-1:0] icache_bmu_ar_id, 
output logic [3:0] icache_bmu_ar_cache, 
output logic icache_bmu_ar_lock, 
output logic [2:0] icache_bmu_ar_prot, 
output logic icache_bmu_ar_user, 
input logic bmu_icache_r_valid, 
output logic icache_bmu_r_ready, 
input logic [32-1:0] bmu_icache_r_data, 
input logic [1-1:0] bmu_icache_r_id, 
input logic bmu_icache_r_last, 
input logic [1:0] bmu_icache_r_resp, 
output logic dcache_bmu_req_vld, 
input logic bmu_dcache_req_rdy, 
output logic [32-1:0] dcache_bmu_req_addr, 
output logic [1:0] dcache_bmu_req_size, 
output logic dcache_bmu_req_write, 
output logic [32-1:0] dcache_bmu_req_wdata, 
output logic [32/8-1:0] dcache_bmu_req_wstrb, 
output logic [1-1:0] dcache_bmu_req_id, 
output logic [1:0] dcache_bmu_req_memattr, 
output logic [1:0] dcache_bmu_req_prot, 
output logic dcache_bmu_req_dm, 
output logic dcache_bmu_req_bitband, 
input logic bmu_dcache_rsp_vld, 
output logic dcache_bmu_rsp_rdy, 
input logic [32-1:0] bmu_dcache_rsp_data, 
input logic [1-1:0] bmu_dcache_rsp_id, 
input logic bmu_dcache_rsp_err, 
input logic bmu_tcm_req_vld, 
output logic tcm_bmu_req_rdy, 
input logic [32-1:0] bmu_tcm_req_addr, 
input logic bmu_tcm_req_write, 
input logic [32-1:0] bmu_tcm_req_wdata, 
input logic [32/8-1:0] bmu_tcm_req_wstrb, 
input logic [1:0] bmu_tcm_req_dest, 
output logic tcm_bmu_rsp_vld, 
input logic bmu_tcm_rsp_rdy, 
output logic [32-1:0] tcm_bmu_rsp_rdata, 
output logic tcm_bmu_rsp_err, 
`ifdef M130_CORE_SUPPORT_ICACHE 
output logic ic_tagm_ecc_err, 
output logic ic_datm_ecc_err, 
output logic [2-1:0] ic_sram_tag_cs, 
output logic [2-1:0] ic_sram_tag_wr, 
output logic [2-1:0][$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] ic_sram_tag_addr, 
output logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_wdata, 
input logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_rdata, 
output logic [2-1:0] ic_sram_data_cs, 
output logic [2-1:0] ic_sram_data_wr, 
output logic [2-1:0][($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64)))-1:0] ic_sram_data_addr, 
output logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_wdata, 
input logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_rdata, 
`endif 
output logic err_valid, 
output logic err_ce, 
output logic err_uue, 
output logic [1:0] err_priority, 
output logic [2:0] err_tt, 
output logic err_scrub, 
output logic [7:0] err_ec, 
output logic [31:0] err_addr, 
output logic [3:0] err_aec, 
output logic err_tag, 
input logic clk, 
input logic rst_n 
); 
typedef enum logic [1:0] { 
DMX_CORE_SOC = 2'd0, 
DMX_CORE_ITCM = 2'd1, 
DMX_CORE_DTCM = 2'd2 
} mss_demux_id_e; 
typedef struct packed { 
logic [ 3:0] cache; 
logic [ 2:0] prot ; 
logic fetch_shv; 
} ft_misc_info_t; 
typedef struct packed { 
logic [31:0] addr ; 
logic [ 3:0] cache; 
logic [ 2:0] prot ; 
logic fetch_shv; 
} ft_req_info_t; 
typedef struct packed { 
logic [64-1:0] data; 
logic [1:0] resp; 
logic ecc_err; 
} ft_rsp_info_t; 
typedef struct packed { 
logic [31:0] addr ; 
logic [ 1:0] size ; 
logic [2-1:0] id ; 
logic [3:0] cache; 
logic lock ; 
logic priv ; 
logic [2:0] dest ; 
logic dm ; 
} ld_req_info_t; 
typedef enum logic { 
MUX_LSU_LD = 1'b0, 
MUX_LSU_ST = 1'b1 
} mss_mux_id_ldst_e; 
typedef struct packed { 
logic write; 
logic [31:0] addr ; 
logic [ 1:0] size ; 
logic [2-1:0] id ; 
logic [3:0] cache; 
logic lock ; 
logic priv ; 
logic [2:0] dest ; 
logic [32-1:0] wdata; 
logic [32/8-1:0] wstrb; 
logic [5:0] atop ; 
logic dm ; 
} ldst_mux_req_info_t; 
parameter MSS_ID_W = 2 + 1; 
typedef struct packed { 
logic [32-1:0] data ; 
logic [MSS_ID_W-1:0] id ; 
logic [1:0] resp ; 
logic ecc_err; 
} ldst_dmx_rsp_info_t; 
typedef enum logic [1:0] { 
REGSLICE_MODE_FEEDTHROUGH = 2'b00, 
REGSLICE_MODE_FORWARD = 2'b01, 
REGSLICE_MODE_BACKWARD = 2'b10, 
REGSLICE_MODE_BIDIRECTION = 2'b11 
} regslice_mode_e; 
function automatic ldst_mux_req_info_t req_info_ld2ldst (input ld_req_info_t req_ld_info); 
ldst_mux_req_info_t req_ldst_info; 
req_ldst_info.write = 1'b0; 
req_ldst_info.addr = req_ld_info.addr; 
req_ldst_info.size = req_ld_info.size; 
req_ldst_info.id = req_ld_info.id ; 
req_ldst_info.cache = req_ld_info.cache; 
req_ldst_info.lock = req_ld_info.lock; 
req_ldst_info.priv = req_ld_info.priv; 
req_ldst_info.wdata = 32'b0; 
req_ldst_info.wstrb = 4'b0; 
req_ldst_info.atop = 6'b0; 
req_ldst_info.dest = req_ld_info.dest; 
req_ldst_info.dm = req_ld_info.dm; 
return req_ldst_info; 
endfunction 
logic [32-1:0] mmr_rdata; 
logic ft_dtcm_req_vld; 
logic dtcm_ft_req_rdy; 
logic [$clog2(32*1024)-1:0] ft_dtcm_req_addr; 
logic dtcm_ft_rsp_vld; 
logic [32-1:0] dtcm_ft_rsp_data; 
logic dtcm_ft_rsp_ded; 
logic dtcm_ft_rsp_sec; 
logic [31:0] ras_dtcm_fetch_addr; 
logic dtcm_dc_rsp_exokay ; 
logic dtcm_dc_rsp_resp ; 
logic dtcm_dc_rsp_atop_ld ; 
logic dff_fetch_addr_en; 
logic [31:0] dff_fetch_addr_d; 
logic [31:0] dff_fetch_addr_q; 
logic [32-1:0] ras_bus_ft_addr; 
logic [32-1:0] ras_bus_refil_addr; 
logic [32-1:0] ras_ic_ce_addr; 
logic ras_ic_tag_ce; 
logic ras_ic_dat_ce; 
logic ras_bus_fetch_uue ; 
logic ras_bus_refil_uue ; 
logic ras_itcm_fetch_ce ; 
logic ras_itcm_fetch_uue; 
logic itcm_dc_rsp_sec ; 
logic itcm_dc_rsp_ded ; 
logic ras_itcm_ld_ce ; 
logic ras_itcm_ld_uue; 
logic ras_itcm_st_ce ; 
logic ras_itcm_st_uue; 
logic ras_dtcm_fetch_ce ; 
logic ras_dtcm_fetch_uue; 
logic dtcm_dc_rsp_sec ; 
logic dtcm_dc_rsp_ded ; 
logic ras_dtcm_ld_ce ; 
logic ras_dtcm_ld_uue; 
logic ras_dtcm_st_ce ; 
logic ras_dtcm_st_uue; 
logic ras_itcm_sbus_ce ; 
logic ras_itcm_sbus_uue; 
logic ras_dtcm_sbus_ce ; 
logic ras_dtcm_sbus_uue; 
logic dff_ft_bmu_infly_set; 
logic dff_ft_bmu_infly_clr; 
logic dff_ft_bmu_infly_en ; 
logic dff_ft_bmu_infly_d ; 
logic dff_ft_bmu_infly_q ; 
ld_req_info_t lsu_mss_ar_info; 
ld_req_info_t ld_mst_req_info; 
logic mss_lsu_r_valid_pre; 
logic dff_dm_en ; 
logic dff_dm_d ; 
logic dff_dm_q ; 
logic lsu_mss_dm; 
logic dff_ld_float_vld_set; 
logic dff_ld_float_vld_clr; 
logic dff_ld_float_vld_en ; 
logic dff_ld_float_vld_d ; 
logic dff_ld_float_vld_q ; 
logic dff_ld_float_flush_set; 
logic dff_ld_float_flush_clr; 
logic dff_ld_float_flush_en ; 
logic dff_ld_float_flush_d ; 
logic dff_ld_float_flush_q ; 
logic [1:0] ldst_mux_slv_req_vld ; 
logic [1:0] ldst_mux_slv_req_rdy ; 
ldst_mux_req_info_t [1:0] ldst_mux_slv_req_info; 
logic [1:0] ldst_mux_slv_rsp_vld ; 
logic ldst_mux_mst_req_vld ; 
logic ldst_mux_mst_req_rdy ; 
ldst_mux_req_info_t ldst_mux_mst_req_info; 
logic ldst_mux_mst_rsp_vld ; 
logic ldst_mux_mst_req_id ; 
logic ldst_mux_mst_rsp_id ; 
logic dff_ldst_mux_id_en; 
logic dff_ldst_mux_id_d ; 
logic dff_ldst_mux_id_q ; 
localparam DEMUX_LDST_N = 1; 
logic [DEMUX_LDST_N-1:0] dcd_ldst_sel; 
ldst_dmx_rsp_info_t ldst_demux_slv_rsp_info; 
logic [DEMUX_LDST_N-1:0] ldst_demux_mst_req_vld ; 
logic [DEMUX_LDST_N-1:0] ldst_demux_mst_req_rdy ; 
logic [DEMUX_LDST_N-1:0] ldst_demux_mst_rsp_vld ; 
ldst_dmx_rsp_info_t [DEMUX_LDST_N-1:0] ldst_demux_mst_rsp_info; 
logic mmr_not_busy; 
logic mmr_iccinv_done; 
`ifdef MSS_EN_MMR 
logic mmr_return_rsp; 
logic mmr_sel_iccen; 
logic mmr_sel_iccinv; 
logic mmr_sel_iccinvaddr; 
logic mmr_sel_eccinject; 
logic mmr_sel_eccbitmap; 
logic mmr_wr_vld; 
logic mmr_wr_iccinv_all; 
logic mmr_wr_iccinv_line; 
logic [31:0] mmr_iccen; 
logic [31:0] mmr_iccinvaddr; 
logic [31:0] mmr_eccinject; 
logic [31:0] mmr_eccbitmap; 
logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_tag_rdata; 
logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_data_rdata; 
logic [(64 / 32)-1:0][(32 + `ITCM_RAS_W)-1:0] itcm_rdata; 
logic [(32 + `DTCM_RAS_W)-1:0] dtcm_rdata; 
assign mmr_sel_iccen = (mmr_req_addr[11:0] == 12'h000); 
assign mmr_sel_iccinv = (mmr_req_addr[11:0] == 12'h004); 
assign mmr_sel_iccinvaddr = (mmr_req_addr[11:0] == 12'h008); 
assign mmr_sel_eccinject = (mmr_req_addr[11:0] == 12'h400); 
assign mmr_sel_eccbitmap = (mmr_req_addr[11:0] == 12'h408); 
assign mmr_wr_vld = mmr_req_vld & mmr_req_rdy & mmr_req_write 
; 
assign mmr_wr_iccinv_all = mmr_wr_vld & mmr_sel_iccinv & mmr_req_wdata[0]; 
assign mmr_wr_iccinv_line = mmr_wr_vld & mmr_sel_iccinv & mmr_req_wdata[1]; 
assign mmr_return_rsp = mmr_req_vld & ~(mmr_wr_iccinv_all | mmr_wr_iccinv_line) | 
mmr_iccinv_done ; 
assign mmr_req_rdy = mmr_not_busy; 
assign mmr_rdata = {32{mmr_sel_iccen }} & mmr_iccen | 
{32{mmr_sel_iccinvaddr }} & mmr_iccinvaddr | 
{32{mmr_sel_eccinject }} & mmr_eccinject | 
{32{mmr_sel_eccbitmap }} & mmr_eccbitmap ; 
`ifdef M130_CORE_SUPPORT_ICACHE 
logic dff_iccen_en; 
logic dff_iccen_d; 
logic dff_iccen_q; 
logic dff_iccinvaddr_en; 
logic [31:0] dff_iccinvaddr_d; 
logic [31:0] dff_iccinvaddr_q; 
logic mmr_iccinv_line_set; 
logic mmr_iccinv_all_set; 
assign dff_iccen_en = mmr_wr_vld & mmr_sel_iccen; 
assign dff_iccen_d = mmr_req_wdata[0]; 
`WDFFER(dff_iccen_q, dff_iccen_d, dff_iccen_en, clk, rst_n) 
assign dff_iccinvaddr_en = mmr_wr_vld & mmr_sel_iccinvaddr; 
assign dff_iccinvaddr_d = mmr_req_wdata; 
`WDFFER(dff_iccinvaddr_q, dff_iccinvaddr_d, dff_iccinvaddr_en, clk, rst_n) 
assign mmr_iccinv_all_set = mmr_wr_iccinv_all; 
assign mmr_iccinv_line_set = mmr_wr_iccinv_line; 
assign mmr_iccen = {31'b0, dff_iccen_q}; 
assign mmr_iccinvaddr = dff_iccinvaddr_q; 
`else 
assign mmr_iccen = 32'b0; 
assign mmr_iccinvaddr = 32'b0; 
`endif 
`ifdef M130_CORE_SUPPORT_ECC 
localparam IC_INJECT_WAY_W = $clog2(2); 
logic dff_eccinject_inj_en; 
logic dff_eccinject_inj_d; 
logic dff_eccinject_inj_q; 
logic dff_eccinject_location_en; 
logic [3:0] dff_eccinject_location_d; 
logic [3:0] dff_eccinject_location_q; 
logic dff_eccinject_way_en; 
logic [IC_INJECT_WAY_W-1:0] dff_eccinject_way_d; 
logic [IC_INJECT_WAY_W-1:0] dff_eccinject_way_q; 
logic dff_eccbitmap_en; 
logic [31:0] dff_eccbitmap_d; 
logic [31:0] dff_eccbitmap_q; 
assign dff_eccinject_inj_en = mmr_wr_vld & mmr_sel_eccinject; 
assign dff_eccinject_inj_d = mmr_req_wdata[0]; 
assign dff_eccinject_location_en = mmr_wr_vld & mmr_sel_eccinject; 
assign dff_eccinject_location_d = mmr_req_wdata[4:1]; 
assign dff_eccinject_way_en = mmr_wr_vld & mmr_sel_eccinject; 
assign dff_eccinject_way_d = mmr_req_wdata[5 +: IC_INJECT_WAY_W]; 
assign dff_eccbitmap_en = mmr_wr_vld & mmr_sel_eccbitmap; 
assign dff_eccbitmap_d = mmr_req_wdata; 
`WDFFER(dff_eccinject_inj_q, dff_eccinject_inj_d, dff_eccinject_inj_en, clk, rst_n) 
`WDFFER(dff_eccinject_location_q, dff_eccinject_location_d, dff_eccinject_location_en, clk, rst_n) 
`WDFFER(dff_eccinject_way_q, dff_eccinject_way_d, dff_eccinject_way_en, clk, rst_n) 
`WDFFER(dff_eccbitmap_q, dff_eccbitmap_d, dff_eccbitmap_en, clk, rst_n) 
assign mmr_eccinject = {{(27-IC_INJECT_WAY_W){1'b0}}, dff_eccinject_way_q, dff_eccinject_location_q, dff_eccinject_inj_q}; 
assign mmr_eccbitmap = dff_eccbitmap_q; 
`ifdef M130_CORE_SUPPORT_ICACHE 
logic [2-1:0] if_eccinject_ic; 
logic [2-1:0][(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))):0] ic_tag_rdata_inject; 
logic [2-1:0][31:0] ic_data_rdata_lo; 
logic [2-1:0][31:0] ic_data_rdata_hi; 
for (genvar i=0; i<2; i++) begin: g_ic_eccinject 
assign if_eccinject_ic[i] = dff_eccinject_inj_q & (i == dff_eccinject_way_q); 
assign ic_tag_rdata_inject[i] = ((dff_eccinject_location_q == 4'b0100) & if_eccinject_ic[i]) ? 
(ic_sram_tag_rdata[i][(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))):0] ^ dff_eccbitmap_q[(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))):0]) : 
ic_sram_tag_rdata[i][(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))):0] ; 
assign ic_tag_rdata[i] = {ic_sram_tag_rdata[i][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1 -: `IC_TAG_SRAM_RAS_W], ic_tag_rdata_inject[i]}; 
assign ic_data_rdata_lo[i] = ((dff_eccinject_location_q == 4'b0110) & if_eccinject_ic[i]) ? 
(ic_sram_data_rdata[i][31:0] ^ dff_eccbitmap_q) : 
ic_sram_data_rdata[i][31:0] ; 
assign ic_data_rdata_hi[i] = ((dff_eccinject_location_q == 4'b0111) & if_eccinject_ic[i]) ? 
(ic_sram_data_rdata[i][63:32] ^ dff_eccbitmap_q) : 
ic_sram_data_rdata[i][63:32] ; 
assign ic_data_rdata[i] ={ic_sram_data_rdata[i][(64 + `IC_DATA_SRAM_RAS_W)-1 -: `IC_DATA_SRAM_RAS_W], ic_data_rdata_hi[i], ic_data_rdata_lo[i]}; 
end 
`endif 
`else 
assign mmr_eccinject = 32'b0; 
assign mmr_eccbitmap = 32'b0; 
`ifdef M130_CORE_SUPPORT_ICACHE 
assign ic_tag_rdata = ic_sram_tag_rdata; 
assign ic_data_rdata = ic_sram_data_rdata; 
`endif 
`endif 
`else 
assign mmr_req_rdy = 1'b1; 
assign mmr_rdata = 32'b0; 
`endif 
assign mmr_rsp_err = 1'b0; 
if (1 == 1) begin : g_mmr_rsp_ppln 
`WDFFR(mmr_rsp_vld, mmr_req_vld, clk, rst_n) 
`WDFFENR(mmr_rsp_rdata, mmr_rdata, mmr_req_vld, clk) 
end else begin : g_mmr_rsp_no_ppln 
assign mmr_rsp_vld = mmr_req_vld; 
assign mmr_rsp_rdata = mmr_rdata; 
end 
assign mss_idle = ~(icache_bmu_ar_valid | dff_ft_bmu_infly_q | 
dcache_bmu_req_vld | dff_ld_float_vld_q ); 
localparam DEMUX_FT_N = 1 
; 
logic [DEMUX_FT_N-1:0] dcd_ft_sel; 
ft_rsp_info_t ft_demux_slv_rsp_info; 
logic [DEMUX_FT_N-1:0] ft_demux_mst_req_vld ; 
logic [DEMUX_FT_N-1:0] ft_demux_mst_req_rdy ; 
logic [DEMUX_FT_N-1:0] ft_demux_mst_rsp_vld ; 
ft_rsp_info_t [DEMUX_FT_N-1:0] ft_demux_mst_rsp_info; 
assign dcd_ft_sel[DMX_CORE_SOC ] = ifu_mss_ar_user[0]; 
wxblite_demux_1xn #( 
.rsp_t ( ft_rsp_info_t ), 
.MST_PORTS_N ( DEMUX_FT_N ), 
.EN_FLUSH ( 1 ), 
.EN_KEEP_ORDER ( 1 ) 
) u_demux_1xn_fetch ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( ifu_mss_flush ), 
.slv_req_vld_i ( ifu_mss_ar_valid ), 
.slv_req_rdy_o ( mss_ifu_ar_ready ), 
.slv_rsp_vld_o ( mss_ifu_r_valid ), 
.slv_rsp_rdy_i ( 1'b1 ), 
.slv_rsp_info_o ( ft_demux_slv_rsp_info ), 
.sel_i ( dcd_ft_sel ), 
.mst_req_vld_o ( ft_demux_mst_req_vld ), 
.mst_req_rdy_i ( ft_demux_mst_req_rdy ), 
.mst_rsp_vld_i ( ft_demux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( ft_demux_mst_rsp_info ) 
); 
assign dff_fetch_addr_en = ifu_mss_ar_valid & mss_ifu_ar_ready; 
assign dff_fetch_addr_d = ifu_mss_ar_addr[31:0]; 
`WDFFER(dff_fetch_addr_q, dff_fetch_addr_d, dff_fetch_addr_en, clk, rst_n) 
assign mss_ifu_r_data = ft_demux_slv_rsp_info.data; 
assign mss_ifu_r_id = {2{1'b0}}; 
assign mss_ifu_r_last = 1'b1; 
assign mss_ifu_r_resp = ft_demux_slv_rsp_info.resp; 
assign mss_ifu_r_user[0] = ft_demux_slv_rsp_info.ecc_err; 
assign mss_ifu_r_user[33-1 -: 32] = dff_fetch_addr_q; 
assign icache_bmu_ar_size = 3'b010; 
assign icache_bmu_ar_len = 8'b0; 
assign icache_bmu_ar_burst = 2'b0; 
assign icache_bmu_ar_id = {1{1'b0}}; 
assign icache_bmu_ar_lock = 1'b0; 
assign icache_bmu_r_ready = 1'b1; 
assign ft_demux_mst_rsp_info[DMX_CORE_SOC].ecc_err = 1'b0; 
assign dff_ft_bmu_infly_set = icache_bmu_ar_valid & bmu_icache_ar_ready; 
assign dff_ft_bmu_infly_clr = bmu_icache_r_valid & icache_bmu_r_ready; 
assign dff_ft_bmu_infly_en = dff_ft_bmu_infly_set | dff_ft_bmu_infly_clr; 
assign dff_ft_bmu_infly_d = dff_ft_bmu_infly_set; 
`WDFFER(dff_ft_bmu_infly_q, dff_ft_bmu_infly_d, dff_ft_bmu_infly_en, clk, rst_n) 
`ifdef M130_CORE_SUPPORT_ICACHE 
m130_mss_ic u_mss_ic( 
.clk (clk ), 
.rst_n (rst_n ), 
.mmr_iccen (dff_iccen_q ), 
.mmr_iccinv_line (mmr_iccinv_line_set ), 
.mmr_iccinv_all (mmr_iccinv_all_set ), 
.mmr_iccinvaddr (dff_iccinvaddr_q ), 
.mmr_not_busy (mmr_not_busy ), 
.mmr_iccinv_done (mmr_iccinv_done ), 
.mss_pcu_icache_init_busy (mss_pcu_icache_init_busy ), 
.ras_ic_tag_ce (ras_ic_tag_ce ), 
.ras_ic_dat_ce (ras_ic_dat_ce ), 
.ras_ic_ce_addr (ras_ic_ce_addr ), 
.ras_bus_fetch_uue (ras_bus_fetch_uue ), 
.ras_bus_refil_uue (ras_bus_refil_uue ), 
.refil_addr (ras_bus_refil_addr ), 
.missbuf_addr (ras_bus_ft_addr ), 
.lsu_mss_fencei_req (lsu_mss_fencei_req ), 
.mss_lsu_fencei_done (mss_lsu_fencei_done ), 
.registered_req_addr (dff_fetch_addr_q ), 
.slv_req_vld_i (ft_demux_mst_req_vld[DMX_CORE_SOC] ), 
.slv_req_rdy_o (ft_demux_mst_req_rdy[DMX_CORE_SOC] ), 
.slv_req_addr (ifu_mss_ar_addr ), 
.slv_req_cache (ifu_mss_ar_cache ), 
.slv_req_prot (ifu_mss_ar_prot ), 
.slv_req_user (ifu_mss_ar_user[3] ), 
.slv_rsp_vld_o (ft_demux_mst_rsp_vld [DMX_CORE_SOC] ), 
.slv_rsp_data (ft_demux_mst_rsp_info[DMX_CORE_SOC].data ), 
.slv_rsp_resp (ft_demux_mst_rsp_info[DMX_CORE_SOC].resp ), 
.ifu_mss_flush (1'b0 ), 
.icache_bmu_ar_valid (icache_bmu_ar_valid ), 
.bmu_icache_ar_ready (bmu_icache_ar_ready ), 
.icache_bmu_ar_addr (icache_bmu_ar_addr ), 
.icache_bmu_ar_cache (icache_bmu_ar_cache ), 
.icache_bmu_ar_prot (icache_bmu_ar_prot ), 
.icache_bmu_ar_user (icache_bmu_ar_user ), 
.bmu_icache_r_valid (bmu_icache_r_valid ), 
.bmu_icache_r_data (bmu_icache_r_data ), 
.bmu_icache_r_id (bmu_icache_r_id ), 
.bmu_icache_r_last (bmu_icache_r_last ), 
.bmu_icache_r_resp (bmu_icache_r_resp ), 
.ic_sram_tag_cs (ic_sram_tag_cs ), 
.ic_sram_tag_wr (ic_sram_tag_wr ), 
.ic_sram_tag_addr (ic_sram_tag_addr ), 
.ic_sram_tag_wdata (ic_sram_tag_wdata ), 
.ic_sram_tag_rdata (ic_tag_rdata ), 
.ic_sram_data_cs (ic_sram_data_cs ), 
.ic_sram_data_wr (ic_sram_data_wr ), 
.ic_sram_data_addr (ic_sram_data_addr ), 
.ic_sram_data_wdata (ic_sram_data_wdata ), 
.ic_sram_data_rdata (ic_data_rdata ) 
); 
`else 
assign mss_pcu_icache_init_busy = '0; 
assign mss_lsu_fencei_done = lsu_mss_fencei_req; 
assign ras_ic_tag_ce = 1'b0; 
assign ras_ic_dat_ce = 1'b0; 
assign ras_ic_ce_addr = 32'b0; 
assign ras_bus_refil_uue = 1'b0; 
assign mmr_not_busy = 1'b1; 
assign mmr_iccinv_done = 1'b0; 
assign ras_bus_refil_addr = 32'b0; 
assign ras_bus_fetch_uue = 1'b0; 
if (64 == 64) begin : g_fetch_width_64 
ft_misc_info_t dmic64_slv_req_info; 
ft_misc_info_t dmic64_mst_req_info; 
assign dmic64_slv_req_info.cache = ifu_mss_ar_cache; 
assign dmic64_slv_req_info.prot = ifu_mss_ar_prot ; 
assign dmic64_slv_req_info.fetch_shv = ifu_mss_ar_user[3]; 
assign icache_bmu_ar_cache = dmic64_mst_req_info.cache; 
assign icache_bmu_ar_prot = dmic64_mst_req_info.prot; 
assign icache_bmu_ar_user = dmic64_mst_req_info.fetch_shv; 
m130_mss_downsizer #( 
.ADDR_W ( 32 ), 
.ERR_W ( 2 ), 
.info_t ( ft_misc_info_t ), 
.REQ_FORWARD ( 0 ) 
) u_downsize_dummy_ic ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_req_vld_i ( ft_demux_mst_req_vld[DMX_CORE_SOC] ), 
.slv_req_rdy_o ( ft_demux_mst_req_rdy[DMX_CORE_SOC] ), 
.slv_req_addr ( ifu_mss_ar_addr ), 
.slv_req_info ( dmic64_slv_req_info ), 
.slv_rsp_vld_o ( ft_demux_mst_rsp_vld[DMX_CORE_SOC] ), 
.slv_rsp_rdy_i ( 1'b1 ), 
.slv_rsp_data ( ft_demux_mst_rsp_info[DMX_CORE_SOC].data ), 
.slv_rsp_err ( ft_demux_mst_rsp_info[DMX_CORE_SOC].resp ), 
.mst_req_vld_o ( icache_bmu_ar_valid ), 
.mst_req_rdy_i ( bmu_icache_ar_ready ), 
.mst_req_addr ( icache_bmu_ar_addr ), 
.mst_req_info ( dmic64_mst_req_info ), 
.mst_rsp_vld_i ( bmu_icache_r_valid ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_data ( bmu_icache_r_data ), 
.mst_rsp_err ( bmu_icache_r_resp ), 
.buf_req_addr ( ras_bus_ft_addr ) 
); 
end else begin : g_fetch_width_32 
ft_req_info_t dmic32_slv_req_info; 
ft_req_info_t dmic32_mst_req_info; 
assign dmic32_slv_req_info.addr = {ifu_mss_ar_addr[31:2], 2'b00}; 
assign dmic32_slv_req_info.cache = ifu_mss_ar_cache; 
assign dmic32_slv_req_info.prot = ifu_mss_ar_prot ; 
assign dmic32_slv_req_info.fetch_shv = ifu_mss_ar_user[3]; 
assign ft_demux_mst_rsp_info[DMX_CORE_SOC].data = bmu_icache_r_data; 
assign ft_demux_mst_rsp_info[DMX_CORE_SOC].resp = bmu_icache_r_resp; 
assign icache_bmu_ar_addr = dmic32_mst_req_info.addr ; 
assign icache_bmu_ar_cache = dmic32_mst_req_info.cache; 
assign icache_bmu_ar_prot = dmic32_mst_req_info.prot ; 
assign icache_bmu_ar_user = dmic32_mst_req_info.fetch_shv; 
assign ras_bus_ft_addr = dff_fetch_addr_q; 
wing_mss_pendbuf #( 
.req_info_t (ft_req_info_t ), 
.EN_REQ_VLD_SYNC (1 ) 
) u_ft_pendbuf ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( ifu_mss_flush ), 
.slv_req_vld_i ( ft_demux_mst_req_vld[DMX_CORE_SOC] ), 
.slv_req_rdy_o ( ft_demux_mst_req_rdy[DMX_CORE_SOC] ), 
.slv_req_info_i ( dmic32_slv_req_info ), 
.slv_rsp_vld_o ( ft_demux_mst_rsp_vld[DMX_CORE_SOC] ), 
.mst_req_vld_o ( icache_bmu_ar_valid ), 
.mst_req_rdy_i ( bmu_icache_ar_ready ), 
.mst_req_info_o ( dmic32_mst_req_info ), 
.mst_rsp_vld_i ( bmu_icache_r_valid ) 
); 
end 
`endif 
assign ras_dtcm_fetch_uue = 1'b0; 
assign ras_dtcm_fetch_ce = 1'b0; 
assign ras_dtcm_fetch_addr = 32'b0; 
assign lsu_mss_ar_info.addr = lsu_mss_ar_addr[31:0] ; 
assign lsu_mss_ar_info.size = lsu_mss_ar_size[1:0] ; 
assign lsu_mss_ar_info.id = lsu_mss_ar_id ; 
assign lsu_mss_ar_info.cache = lsu_mss_ar_cache[3:0] ; 
assign lsu_mss_ar_info.lock = lsu_mss_ar_lock ; 
assign lsu_mss_ar_info.priv = lsu_mss_ar_prot[0] ; 
assign lsu_mss_ar_info.dest = lsu_mss_ar_user[2:0] ; 
assign lsu_mss_ar_info.dm = lsu_mss_ar_user[4] ; 
localparam MSS_LD_PENDBUF = (REGSLICE_MODE_FORWARD == REGSLICE_MODE_FEEDTHROUGH) | (REGSLICE_MODE_FORWARD == REGSLICE_MODE_BACKWARD); 
parameter MSS_LDST_LOCK = MSS_LD_PENDBUF; 
if (MSS_LD_PENDBUF) begin : mss_ld_pendbuf 
wing_mss_pendbuf #( 
.req_info_t (ld_req_info_t ), 
.EN_REQ_VLD_SYNC (1 ) 
) u_ld_pendbuf ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( lsu_mss_load_flush ), 
.slv_req_vld_i ( lsu_mss_ar_valid ), 
.slv_req_rdy_o ( mss_lsu_ar_ready ), 
.slv_req_info_i ( lsu_mss_ar_info ), 
.slv_rsp_vld_o ( mss_lsu_r_valid_pre ), 
.mst_req_vld_o ( ldst_mux_slv_req_vld[MUX_LSU_LD] ), 
.mst_req_rdy_i ( ldst_mux_slv_req_rdy[MUX_LSU_LD] ), 
.mst_req_info_o ( ld_mst_req_info ), 
.mst_rsp_vld_i ( ldst_mux_slv_rsp_vld[MUX_LSU_LD] ) 
); 
assign ldst_mux_slv_req_info[MUX_LSU_LD] = req_info_ld2ldst(ld_mst_req_info); 
end else begin: mss_ld_no_regslice_no_pendbuf 
assign ldst_mux_slv_req_vld [MUX_LSU_LD] = lsu_mss_ar_valid; 
assign ldst_mux_slv_req_info[MUX_LSU_LD] = req_info_ld2ldst(lsu_mss_ar_info); 
assign mss_lsu_ar_ready = ldst_mux_slv_req_rdy[MUX_LSU_LD]; 
assign mss_lsu_r_valid_pre = ldst_mux_slv_rsp_vld[MUX_LSU_LD]; 
end 
assign dff_ld_float_vld_set = lsu_mss_ar_valid & mss_lsu_ar_ready; 
assign dff_ld_float_vld_clr = mss_lsu_r_valid_pre; 
assign dff_ld_float_vld_en = dff_ld_float_vld_set | dff_ld_float_vld_clr; 
assign dff_ld_float_vld_d = dff_ld_float_vld_set; 
`WDFFER(dff_ld_float_vld_q, dff_ld_float_vld_d, dff_ld_float_vld_en, clk, rst_n) 
assign dff_ld_float_flush_set = dff_ld_float_vld_q & lsu_mss_load_flush; 
assign dff_ld_float_flush_clr = mss_lsu_r_valid_pre; 
assign dff_ld_float_flush_en = dff_ld_float_flush_set | dff_ld_float_flush_clr; 
assign dff_ld_float_flush_d = dff_ld_float_flush_set & ~dff_ld_float_flush_clr; 
`WDFFER(dff_ld_float_flush_q, dff_ld_float_flush_d, dff_ld_float_flush_en, clk, rst_n) 
assign mss_lsu_r_valid = mss_lsu_r_valid_pre & ~dff_ld_float_flush_q 
; 
assign mss_lsu_r_data = 
ldst_demux_slv_rsp_info.data; 
assign mss_lsu_r_id = 
ldst_demux_slv_rsp_info.id[2-1:0]; 
assign mss_lsu_r_last = 1'b1; 
assign mss_lsu_r_resp = 
ldst_demux_slv_rsp_info.resp; 
assign mss_lsu_r_user = ldst_demux_slv_rsp_info.ecc_err; 
assign ldst_mux_slv_req_vld [MUX_LSU_ST] = lsu_mss_aw_valid; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].write = 1'b1; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].addr = lsu_mss_aw_addr ; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].size = lsu_mss_aw_size[1:0] ; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].id = lsu_mss_aw_id; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].cache = lsu_mss_aw_cache; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].lock = lsu_mss_aw_lock; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].priv = lsu_mss_aw_prot[0]; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].dest = lsu_mss_aw_user[2:0]; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].wdata = lsu_mss_w_data; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].wstrb = lsu_mss_w_strb; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].atop = lsu_mss_aw_atop; 
assign ldst_mux_slv_req_info[MUX_LSU_ST].dm = lsu_mss_aw_user[4]; 
assign mss_lsu_aw_ready = ldst_mux_slv_req_rdy[MUX_LSU_ST]; 
assign mss_lsu_w_ready = ldst_mux_slv_req_rdy[MUX_LSU_ST]; 
assign mss_lsu_b_valid = ldst_mux_slv_rsp_vld[MUX_LSU_ST]; 
assign mss_lsu_b_id = ldst_demux_slv_rsp_info.id[2-1:0]; 
assign mss_lsu_b_resp = ldst_demux_slv_rsp_info.resp; 
wxblite_mux_nx1 #( 
.req_t ( ldst_mux_req_info_t ), 
.rsp_t ( logic ), 
.SLV_PORTS_N ( 2 ), 
.ARB_MODE ( 0 ), 
.LOCK ( MSS_LDST_LOCK ), 
.EN_FLUSH ( 0 ) 
) u_wxbl_mux_nx1_ldst ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( ldst_mux_slv_req_vld ), 
.slv_req_rdy_o ( ldst_mux_slv_req_rdy ), 
.slv_req_info_i ( ldst_mux_slv_req_info ), 
.slv_rsp_vld_o ( ldst_mux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( 2'b1 ), 
.slv_rsp_info_o ( ), 
.mst_req_vld_o ( ldst_mux_mst_req_vld ), 
.mst_req_rdy_i ( ldst_mux_mst_req_rdy ), 
.mst_req_info_o ( ldst_mux_mst_req_info ), 
.mst_req_id_o ( ldst_mux_mst_req_id ), 
.mst_rsp_vld_i ( ldst_mux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( 1'b0 ), 
.mst_rsp_id_i ( ldst_mux_mst_rsp_id ) 
); 
assign ldst_mux_mst_rsp_id = ldst_demux_slv_rsp_info.id[MSS_ID_W-1]; 
assign dff_dm_en = ldst_mux_mst_req_vld & ldst_mux_mst_req_rdy; 
assign dff_dm_d = ldst_mux_mst_req_info.dm; 
`WDFFER (dff_dm_q, dff_dm_d, dff_dm_en, clk, rst_n) 
assign lsu_mss_dm = dff_dm_q; 
assign dcd_ldst_sel[DMX_CORE_SOC ] = ldst_mux_mst_req_info.dest[0]; 
wxblite_demux_1xn #( 
.rsp_t ( ldst_dmx_rsp_info_t ), 
.MST_PORTS_N ( DEMUX_LDST_N ), 
.EN_FLUSH ( 0 ), 
.EN_KEEP_ORDER ( 1 ) 
) u_demux_1xn_ldst ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( ldst_mux_mst_req_vld ), 
.slv_req_rdy_o ( ldst_mux_mst_req_rdy ), 
.slv_rsp_vld_o ( ldst_mux_mst_rsp_vld ), 
.slv_rsp_rdy_i ( 1'b1 ), 
.slv_rsp_info_o ( ldst_demux_slv_rsp_info ), 
.sel_i ( dcd_ldst_sel ), 
.mst_req_vld_o ( ldst_demux_mst_req_vld ), 
.mst_req_rdy_i ( ldst_demux_mst_req_rdy ), 
.mst_rsp_vld_i ( ldst_demux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( ldst_demux_mst_rsp_info ) 
); 
assign ldst_demux_mst_req_rdy [DMX_CORE_SOC] = bmu_dcache_req_rdy; 
assign ldst_demux_mst_rsp_vld [DMX_CORE_SOC] = bmu_dcache_rsp_vld; 
assign ldst_demux_mst_rsp_info[DMX_CORE_SOC].data = bmu_dcache_rsp_data; 
assign ldst_demux_mst_rsp_info[DMX_CORE_SOC].id = {dff_ldst_mux_id_q, {2-1{1'b0}}, bmu_dcache_rsp_id}; 
assign ldst_demux_mst_rsp_info[DMX_CORE_SOC].resp = {bmu_dcache_rsp_err, 1'b0}; 
assign ldst_demux_mst_rsp_info[DMX_CORE_SOC].ecc_err = 1'b0; 
assign dff_ldst_mux_id_en = ldst_demux_mst_req_vld[DMX_CORE_SOC] & ldst_demux_mst_req_rdy[DMX_CORE_SOC]; 
assign dff_ldst_mux_id_d = ldst_mux_mst_req_id; 
`WDFFER(dff_ldst_mux_id_q, dff_ldst_mux_id_d, dff_ldst_mux_id_en, clk, rst_n) 
assign dcache_bmu_req_vld = ldst_demux_mst_req_vld[DMX_CORE_SOC]; 
assign dcache_bmu_req_addr = ldst_mux_mst_req_info.addr; 
assign dcache_bmu_req_size = ldst_mux_mst_req_info.size; 
assign dcache_bmu_req_write = ldst_mux_mst_req_info.write; 
assign dcache_bmu_req_wdata = ldst_mux_mst_req_info.wdata; 
assign dcache_bmu_req_wstrb = ldst_mux_mst_req_info.wstrb; 
assign dcache_bmu_req_id = ldst_mux_mst_req_info.id[1-1:0]; 
assign dcache_bmu_req_memattr[0] = (ldst_mux_mst_req_info.cache != 4'b0000); 
assign dcache_bmu_req_memattr[1] = (ldst_mux_mst_req_info.cache == 4'b1111); 
assign dcache_bmu_req_prot[0] = 1'b1; 
assign dcache_bmu_req_prot[1] = ldst_mux_mst_req_info.priv; 
assign dcache_bmu_req_dm = ldst_mux_mst_req_info.dm; 
assign dcache_bmu_req_bitband = 1'b0; 
assign dcache_bmu_rsp_rdy = 1'b1; 
logic ras_bus_ld_uue; 
logic ras_bus_st_uue; 
logic ras_itcm_uue; 
logic ras_itcm_ce ; 
logic ras_dtcm_uue; 
logic ras_dtcm_ce ; 
logic ras_bus_uue ; 
logic ras_ic_ce; 
logic dff_sbus_addr_en; 
logic [32-1:0] dff_sbus_addr_d ; 
logic [32-1:0] dff_sbus_addr_q ; 
logic dff_st_addr_en; 
logic [32-1:0] dff_st_addr_d ; 
logic [32-1:0] dff_st_addr_q ; 
logic dff_ld_addr_en; 
logic [32-1:0] dff_ld_addr_d ; 
logic [32-1:0] dff_ld_addr_q ; 
assign ras_bus_ld_uue = bmu_dcache_rsp_vld & bmu_dcache_rsp_err & ~dff_ldst_mux_id_q & mss_lsu_r_valid & ~lsu_mss_dm; 
assign ras_bus_st_uue = bmu_dcache_rsp_vld & bmu_dcache_rsp_err & dff_ldst_mux_id_q & ~lsu_mss_dm; 
assign ras_bus_uue = ras_bus_fetch_uue | ras_bus_ld_uue | ras_bus_st_uue | ras_bus_refil_uue; 
`ifdef M130_CORE_SUPPORT_ICACHE 
assign ras_ic_ce = ras_ic_tag_ce | ras_ic_dat_ce; 
`endif 
assign err_uue = ras_bus_uue 
; 
assign err_ce = 1'b0 
`ifdef M130_CORE_SUPPORT_ICACHE 
| ras_ic_ce 
`endif 
; 
assign err_valid = err_uue | err_ce; 
assign err_priority = 
ras_bus_uue ? 2'd0 : 
`ifdef M130_CORE_SUPPORT_ICACHE 
ras_ic_ce ? 2'd1 : 
`endif 
2'd0 ; 
always_comb begin : g_ras_tt 
if (ras_bus_ld_uue) begin : g_ras_bus_ld_uue 
err_tt = 3'd4; 
err_aec = 4'd1; 
err_addr = dff_ld_addr_q; 
end else if (ras_bus_st_uue) begin : g_rad_bus_st_uue 
err_tt = 3'd5; 
err_aec = 4'd1; 
err_addr = dff_st_addr_q; 
end else if (ras_bus_fetch_uue) begin : g_ras_bus_fetch_uue 
err_tt = 3'd4; 
err_aec = 4'd0; 
err_addr = ras_bus_ft_addr; 
end else if (ras_bus_refil_uue) begin : g_ras_bus_refil_uue 
err_tt = 3'd6; 
err_aec = 4'd4; 
err_addr = ras_bus_refil_addr; 
end else 
`ifdef M130_CORE_SUPPORT_ICACHE 
if (ras_ic_ce) begin : g_ras_ic_ce 
err_tt = 3'd4; 
err_aec = 4'd0; 
err_addr = ras_ic_ce_addr; 
end else 
`endif 
begin : g_ras_default 
err_tt = 3'd0; 
err_aec = 4'd0; 
err_addr = '0; 
end 
end 
assign err_scrub = err_uue ? 1'b0 : 
err_ce ? 1'b1 : 
1'b0 ; 
assign err_ec = 
ras_bus_uue ? 8'd14 : 
`ifdef M130_CORE_SUPPORT_ICACHE 
ras_ic_ce ? 8'd4 : 
`endif 
8'd0 ; 
assign err_tag = (err_ec == 8'd4) & ras_ic_tag_ce; 
assign dff_sbus_addr_en = bmu_tcm_req_vld 
; 
assign dff_sbus_addr_d = bmu_tcm_req_addr; 
`WDFFER(dff_sbus_addr_q, dff_sbus_addr_d, dff_sbus_addr_en, clk, rst_n) 
assign dff_st_addr_en = lsu_mss_aw_valid & mss_lsu_aw_ready; 
assign dff_st_addr_d = lsu_mss_aw_addr; 
`WDFFER(dff_st_addr_q, dff_st_addr_d, dff_st_addr_en, clk, rst_n) 
assign dff_ld_addr_en = lsu_mss_ar_valid & mss_lsu_ar_ready; 
assign dff_ld_addr_d = lsu_mss_ar_addr; 
`WDFFER(dff_ld_addr_q, dff_ld_addr_d, dff_ld_addr_en, clk, rst_n) 
`ifdef M130_CORE_SUPPORT_ICACHE 
assign ic_tagm_ecc_err = ras_ic_tag_ce; 
assign ic_datm_ecc_err = ras_ic_dat_ce; 
`endif 
endmodule
 
 
module m130_mss_ic ( 
input logic clk, 
input logic rst_n, 
input logic mmr_iccen , 
input logic mmr_iccinv_line, 
input logic mmr_iccinv_all, 
input logic [31:0] mmr_iccinvaddr, 
output logic mmr_not_busy, 
output logic mmr_iccinv_done, 
output logic mss_pcu_icache_init_busy, 
output logic ras_ic_tag_ce , 
output logic ras_ic_dat_ce , 
output logic [32-1:0] ras_ic_ce_addr, 
output logic ras_bus_fetch_uue, 
output logic ras_bus_refil_uue, 
output logic [31:0] refil_addr, 
output logic [31:0] missbuf_addr, 
input logic lsu_mss_fencei_req, 
output logic mss_lsu_fencei_done, 
input logic [32-1:0] registered_req_addr, 
input logic slv_req_vld_i, 
output logic slv_req_rdy_o, 
input logic [32-1:0] slv_req_addr , 
input logic [3:0] slv_req_cache, 
input logic [2:0] slv_req_prot , 
input logic slv_req_user , 
output logic slv_rsp_vld_o, 
output logic [64-1:0] slv_rsp_data , 
output logic [1:0] slv_rsp_resp , 
input logic ifu_mss_flush, 
output logic icache_bmu_ar_valid, 
input logic bmu_icache_ar_ready, 
output logic [32-1:0] icache_bmu_ar_addr, 
output logic [3:0] icache_bmu_ar_cache, 
output logic [2:0] icache_bmu_ar_prot, 
output logic icache_bmu_ar_user, 
input logic bmu_icache_r_valid, 
input logic [32-1:0] bmu_icache_r_data, 
input logic [1-1:0] bmu_icache_r_id, 
input logic bmu_icache_r_last, 
input logic [1:0] bmu_icache_r_resp, 
output logic [2-1:0] ic_sram_tag_cs, 
output logic [2-1:0] ic_sram_tag_wr, 
output logic [2-1:0][$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] ic_sram_tag_addr, 
output logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_wdata, 
input logic [2-1:0][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:0] ic_sram_tag_rdata, 
output logic [2-1:0] ic_sram_data_cs, 
output logic [2-1:0] ic_sram_data_wr, 
output logic [2-1:0][($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64)))-1:0] ic_sram_data_addr, 
output logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_wdata, 
input logic [2-1:0][(64 + `IC_DATA_SRAM_RAS_W)-1:0] ic_sram_data_rdata 
); 
localparam MST_IF_DATA_BYTE_W = 32/8; 
localparam REFIL_LEN_N = 32/MST_IF_DATA_BYTE_W; 
localparam REFIL_CNT_W = $clog2(REFIL_LEN_N); 
localparam WAY_IDX_W = $clog2(2); 
localparam TAG_SRAM_ADDR_OFFS_W = $clog2(32); 
localparam DATA_SRAM_ADDR_OFFS_W = $clog2(32/4); 
localparam TAG_MEM_DAT_W = (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + 1; 
typedef struct packed { 
logic vld; 
logic [(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))-1:0] tag; 
} tag_sram_dat_t; 
logic icache_bmu_ar_hdsk; 
logic ic_r_vld; 
logic miss_rsp_vld; 
logic req_refil; 
logic req_fetch; 
logic req_invld_all; 
logic req_invld_line; 
logic req_invld; 
logic arb_grt_invld; 
logic arb_grt_fetch; 
logic arb_grt_fetch_ic; 
logic arb_grt_fetch_nc; 
logic fetch_chk_vld_set; 
logic fetch_chk_vld_clr; 
logic fetch_chk_vld_en; 
logic fetch_chk_vld_d; 
logic fetch_chk_vld_q; 
logic fetch_chk_nc_en; 
logic fetch_chk_nc_d; 
logic fetch_chk_nc_q; 
logic fetch_chk_priv_en; 
logic fetch_chk_priv_d; 
logic fetch_chk_priv_q; 
logic fetch_chk_user_en; 
logic fetch_chk_user_d ; 
logic fetch_chk_user_q ; 
logic [(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))-1:0] fetch_chk_tag; 
logic [2-1:0][$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] ic_sram_tag_addr_pre; 
logic [2-1:0][$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] ic_sram_data_addr_pre; 
logic missbuf_vld_set; 
logic missbuf_vld_clr; 
logic missbuf_vld_en; 
logic missbuf_vld_d; 
logic missbuf_vld_q; 
logic missbuf_sent_set; 
logic missbuf_sent_clr; 
logic missbuf_sent_en; 
logic missbuf_sent_d; 
logic missbuf_sent_q; 
logic missbuf_flush_set; 
logic missbuf_flush_clr; 
logic missbuf_flush_en; 
logic missbuf_flush_d; 
logic missbuf_flush_q; 
logic missbuf_nc_en; 
logic missbuf_nc_d; 
logic missbuf_nc_q; 
logic missbuf_addr_en; 
logic [31:0] missbuf_addr_d ; 
logic [31:0] missbuf_addr_q ; 
logic missbuf_way_idx_en; 
logic [WAY_IDX_W-1:0] missbuf_way_idx_d ; 
logic [WAY_IDX_W-1:0] missbuf_way_idx_q ; 
logic [REFIL_CNT_W-1:0] fetch_addr_offs_init; 
logic missbuf_addr_offs_tx_en; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_tx_d; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_tx_q; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_tx_incr; 
logic missbuf_addr_offs_rx_en; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_rx_d; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_rx_q; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_rx_incr; 
logic missbuf_cnt_tx_en; 
logic [REFIL_CNT_W-1:0] missbuf_cnt_tx_d; 
logic [REFIL_CNT_W-1:0] missbuf_cnt_tx_q; 
logic [REFIL_CNT_W-1:0] missbuf_cnt_tx_incr; 
logic missbuf_cnt_rx_en; 
logic [REFIL_CNT_W-1:0] missbuf_cnt_rx_d; 
logic [REFIL_CNT_W-1:0] missbuf_cnt_rx_incr; 
logic [REFIL_CNT_W-1:0] missbuf_cnt_rx_q; 
logic cnt_tx_reached; 
logic cnt_rx_reached; 
logic nc_fetch_half; 
logic nc_sent_done; 
logic [REFIL_CNT_W-1:0] missbuf_addr_offs_tx; 
logic crit_word_rdy; 
logic dff_init_done_en; 
logic dff_init_done_d ; 
logic dff_init_done_q ; 
logic dff_fencei_set; 
logic dff_fencei_clr; 
logic dff_fencei_en; 
logic dff_fencei_d; 
logic dff_fencei_q; 
logic dff_inv_line_set; 
logic dff_inv_line_clr; 
logic dff_inv_line_en; 
logic dff_inv_line_d; 
logic dff_inv_line_q; 
logic dff_inv_all_set; 
logic dff_inv_all_clr; 
logic dff_inv_all_en; 
logic dff_inv_all_d; 
logic dff_inv_all_q; 
logic dff_cnt_inv_en; 
logic [$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] dff_cnt_inv_d; 
logic [$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] dff_cnt_inv_q; 
logic cnt_inv_reached; 
logic ic_bein_invld_all; 
logic ic_bein_invld_line; 
logic dff_inv_line_r_req_set; 
logic dff_inv_line_r_req_clr; 
logic dff_inv_line_r_req_en; 
logic dff_inv_line_r_req_d; 
logic dff_inv_line_r_req_q; 
logic dff_inv_line_r_rsp_set; 
logic dff_inv_line_r_rsp_clr; 
logic dff_inv_line_r_rsp_en; 
logic dff_inv_line_r_rsp_d; 
logic dff_inv_line_r_rsp_q; 
logic dff_inv_line_wr_set; 
logic dff_inv_line_wr_clr; 
logic dff_inv_line_wr_en; 
logic dff_inv_line_wr_d; 
logic dff_inv_line_wr_q; 
logic iccinv_line_r_req; 
logic iccinv_line_r_rsp; 
logic iccinv_line_wr; 
logic dff_inv_line_cs_en; 
logic [2-1:0] dff_inv_line_cs_d; 
logic [2-1:0] dff_inv_line_cs_q; 
logic dff_lo_data_en; 
logic [31:0] dff_lo_data_d; 
logic [31:0] dff_lo_data_q; 
logic cnt_rx_eq_1st2; 
logic dff_resp_1st2_en; 
logic [1:0] dff_resp_1st2_d; 
logic [1:0] dff_resp_1st2_q; 
logic dff_resp_rest_en; 
logic [1:0] dff_resp_rest_d; 
logic [1:0] dff_resp_rest_q; 
logic [(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))-1:0] ic_chk_tag; 
tag_sram_dat_t [2-1:0] tag_dat_per_way; 
logic [2-1:0] tag_hit_per_way; 
logic [2-1:0] tag_vld_per_way; 
logic [2-1:0] tag_invld_per_way; 
logic tag_hit; 
logic cl_tag_hit_pre; 
logic cl_tag_hit; 
logic cl_tag_miss; 
logic [63:0] cl_mux_data; 
logic iccinv_line_hit; 
logic iccinv_line_miss; 
logic [WAY_IDX_W-1:0] way_invld; 
logic [WAY_IDX_W-1:0] way_replace; 
logic [WAY_IDX_W-1:0] way_random; 
logic ways_all_vld; 
logic [2-1:0] way_replace_oh; 
logic refil_data_vld; 
logic refil_tag_vld; 
logic invld_tag; 
logic [2-1:0] tagm_cs_fetch; 
logic [2-1:0] datm_cs_fetch; 
logic [2-1:0] tagm_cs_refil; 
logic [2-1:0] datm_cs_refil; 
logic [2-1:0] tagm_cs_invld; 
logic [2-1:0] datm_cs_invld; 
logic [2-1:0] tagm_cs_ecc_inv; 
logic [2-1:0] tagm_cs_iccinv_rd; 
logic [2-1:0] tagm_cs_iccinv_wr; 
logic [2-1:0] tagm_cs_invld_all; 
logic [2-1:0] tagm_cs_invld_line; 
logic [$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))-1:0] invld_addr; 
logic mem_ecc_dec_err; 
logic [2-1:0] iccinv_line_ecc_err; 
logic ecc_invld_tag; 
logic ecc_back_press_fetch; 
logic [63:0] datm_data_refil; 
tag_sram_dat_t tagm_data_refil; 
tag_sram_dat_t ic_tag_wdata_pre; 
logic [2-1:0][63:0] datm_rdata; 
logic ras_itcm_uue; 
logic ras_itcm_ce ; 
logic ras_itcm; 
logic ras_dtcm_uue; 
logic ras_dtcm_ce ; 
logic ras_dtcm; 
logic ras_bus_uue ; 
logic ras_ic_ce; 
logic [1:0] sum_ras_uue; 
logic [1:0] sum_ras_ce; 
logic dff_sbus_addr_en; 
logic [32-1:0] dff_sbus_addr_d ; 
logic [32-1:0] dff_sbus_addr_q ; 
logic dff_st_addr_en; 
logic [32-1:0] dff_st_addr_d ; 
logic [32-1:0] dff_st_addr_q ; 
logic [32-1:0] ld_addr; 
assign req_fetch = slv_req_vld_i & ~(lsu_mss_fencei_req | mmr_iccinv_all | mmr_iccinv_line) 
& ~cl_tag_miss 
& ~ecc_back_press_fetch; 
assign arb_grt_invld = req_invld & ~req_refil; 
assign arb_grt_fetch = req_fetch & ~req_invld & ~req_refil; 
assign arb_grt_fetch_ic = arb_grt_fetch & mmr_iccen & ~fetch_chk_nc_d; 
assign arb_grt_fetch_nc = arb_grt_fetch & (mmr_iccen & fetch_chk_nc_d | 
~mmr_iccen); 
assign fetch_chk_vld_set = arb_grt_fetch_ic; 
assign fetch_chk_vld_clr = fetch_chk_vld_q; 
assign fetch_chk_vld_en = fetch_chk_vld_set | fetch_chk_vld_clr; 
assign fetch_chk_vld_d = fetch_chk_vld_set; 
`WDFFER(fetch_chk_vld_q, fetch_chk_vld_d, fetch_chk_vld_en, clk, rst_n) 
assign fetch_chk_nc_en = arb_grt_fetch; 
assign fetch_chk_nc_d = (slv_req_cache == 4'b0011); 
assign fetch_chk_priv_en = arb_grt_fetch; 
assign fetch_chk_priv_d = slv_req_prot[0]; 
assign fetch_chk_user_en = arb_grt_fetch; 
assign fetch_chk_user_d = slv_req_user ; 
`WDFFER(fetch_chk_nc_q, fetch_chk_nc_d, fetch_chk_nc_en, clk, rst_n) 
`WDFFER(fetch_chk_priv_q, fetch_chk_priv_d, fetch_chk_priv_en, clk, rst_n) 
`WDFFER(fetch_chk_user_q, fetch_chk_user_d, fetch_chk_user_en, clk, rst_n) 
assign fetch_chk_tag = registered_req_addr[32-1 -: (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))]; 
assign tagm_cs_fetch = {2{arb_grt_fetch_ic}}; 
assign tagm_cs_ecc_inv = {2{ecc_invld_tag}}; 
assign ic_sram_tag_cs = tagm_cs_fetch | 
tagm_cs_refil | 
tagm_cs_invld | 
tagm_cs_ecc_inv; 
assign ic_sram_tag_wr = {2{refil_tag_vld | 
invld_tag | 
ecc_invld_tag }}; 
assign ic_sram_tag_addr = {2{{$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)){refil_tag_vld }} & refil_addr [TAG_SRAM_ADDR_OFFS_W +: $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))] | 
{$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)){arb_grt_fetch_ic}} & slv_req_addr[TAG_SRAM_ADDR_OFFS_W +: $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))] | 
{$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)){arb_grt_invld }} & invld_addr | 
{$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)){ecc_invld_tag }} & registered_req_addr[TAG_SRAM_ADDR_OFFS_W +: $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))] }}; 
assign ic_tag_wdata_pre = {TAG_MEM_DAT_W{refil_tag_vld}} & tagm_data_refil; 
assign datm_cs_fetch = {2{arb_grt_fetch_ic}}; 
assign ic_sram_data_cs = datm_cs_fetch | 
datm_cs_refil ; 
assign ic_sram_data_wr = {2{refil_data_vld}}; 
assign ic_sram_data_addr = {2{{($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64))){arb_grt_fetch_ic}} & slv_req_addr[DATA_SRAM_ADDR_OFFS_W +: ($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64)))] | 
{($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64))){refil_data_vld }} & refil_addr [DATA_SRAM_ADDR_OFFS_W +: ($clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)) + $clog2(((32 * 8) / 64)))] }}; 
`ifdef M130_CORE_SUPPORT_ECC 
typedef struct packed { 
logic [6:0] code7; 
logic [31:0] data32; 
} tagm_enc_word_t; 
typedef struct packed { 
logic [7:0] code8; 
logic [63:0] data64; 
} datm_enc_word_t; 
tagm_enc_word_t tagm_ecc_enc_word; 
tagm_enc_word_t [2-1:0] tagm_ecc_dec_word; 
logic [2-1:0] tagm_rdata_vld; 
logic [2-1:0][(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))-1:0] tagm_rdata_tag; 
logic [2-1:0] tagm_ecc_dec_err; 
datm_enc_word_t datm_ecc_enc_word; 
datm_enc_word_t [2-1:0] datm_ecc_dec_word; 
logic [2-1:0] datm_ecc_dec_err; 
logic dff_ecc_err_inv_tag_set; 
logic dff_ecc_err_inv_tag_clr; 
logic dff_ecc_err_inv_tag_en; 
logic dff_ecc_err_inv_tag_d ; 
logic dff_ecc_err_inv_tag_q ; 
assign tagm_ecc_enc_word.data32 = {{32-TAG_MEM_DAT_W{1'b0}}, ic_tag_wdata_pre}; 
wing_cbb_ecc_enc32 u_tagm_ecc_enc32 ( 
.data_i ( tagm_ecc_enc_word.data32 ), 
.enc_o ( tagm_ecc_enc_word.code7 ) 
); 
assign ic_sram_tag_wdata = {2{tagm_ecc_enc_word.code7, ic_tag_wdata_pre}}; 
assign datm_ecc_enc_word.data64 = datm_data_refil; 
wing_cbb_ecc_enc64 u_datm_ecc_enc64 ( 
.data_i ( datm_ecc_enc_word.data64 ), 
.enc_o ( datm_ecc_enc_word.code8 ) 
); 
assign ic_sram_data_wdata = {2{datm_ecc_enc_word}}; 
for (genvar i=0; i<2; i++) begin: g_ecc_dec 
assign tagm_rdata_vld[i] = ic_sram_tag_rdata[i][(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))]; 
assign tagm_rdata_tag[i] = ic_sram_tag_rdata[i][(32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))-1:0]; 
assign tagm_ecc_dec_word[i].code7 = ic_sram_tag_rdata[i][(1 + (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))) + `IC_TAG_SRAM_RAS_W)-1:TAG_MEM_DAT_W]; 
assign tagm_ecc_dec_word[i].data32 = {{32-TAG_MEM_DAT_W{1'b0}}, ic_sram_tag_rdata[i][TAG_MEM_DAT_W-1:0]}; 
wing_cbb_ecc_dec32 u_tagm_ecc_dec32 ( 
.data_i ( tagm_ecc_dec_word[i] ), 
.data_o ( ), 
.syndrome_o ( ), 
.error_detected ( tagm_ecc_dec_err[i] ), 
.single_error_o ( ), 
.double_error_o ( ) 
); 
assign datm_ecc_dec_word[i] = datm_enc_word_t'(ic_sram_data_rdata[i]); 
wing_cbb_ecc_dec64 u_datm_ecc_dec64 ( 
.data_i ( datm_ecc_dec_word[i] ), 
.data_o ( ), 
.syndrome_o ( ), 
.error_detected ( datm_ecc_dec_err[i] ), 
.single_error_o ( ), 
.double_error_o ( ) 
); 
assign tag_dat_per_way[i].vld = tagm_rdata_vld[i]; 
assign tag_dat_per_way[i].tag = tagm_rdata_tag[i]; 
assign datm_rdata[i] = datm_ecc_dec_word[i].data64; 
end 
assign mem_ecc_dec_err = |(tagm_ecc_dec_err | 
(datm_ecc_dec_err & tagm_rdata_vld)); 
assign dff_ecc_err_inv_tag_set = fetch_chk_vld_q & mem_ecc_dec_err & ~req_invld & ~ifu_mss_flush ; 
assign dff_ecc_err_inv_tag_clr = dff_ecc_err_inv_tag_q; 
assign dff_ecc_err_inv_tag_en = dff_ecc_err_inv_tag_set | dff_ecc_err_inv_tag_clr; 
assign dff_ecc_err_inv_tag_d = dff_ecc_err_inv_tag_set; 
`WDFFER(dff_ecc_err_inv_tag_q, dff_ecc_err_inv_tag_d, dff_ecc_err_inv_tag_en, clk, rst_n) 
assign ras_ic_tag_ce = (fetch_chk_vld_q | iccinv_line_r_rsp) & |(tagm_ecc_dec_err); 
assign ras_ic_dat_ce = (fetch_chk_vld_q | iccinv_line_r_rsp) & |(datm_ecc_dec_err & tagm_rdata_vld); 
assign ras_ic_ce_addr = ic_bein_invld_line ? mmr_iccinvaddr : registered_req_addr; 
assign ecc_back_press_fetch = (|dff_ecc_err_inv_tag_set) | 
(|dff_ecc_err_inv_tag_q) ; 
assign ecc_invld_tag = dff_ecc_err_inv_tag_q; 
assign iccinv_line_ecc_err = tagm_ecc_dec_err; 
`else 
assign ic_sram_tag_wdata = {2{ic_tag_wdata_pre}}; 
assign ic_sram_data_wdata = {2{datm_data_refil}}; 
for (genvar i=0; i<2; i++) begin: g_ic_rdata 
assign tag_dat_per_way[i] = tag_sram_dat_t'(ic_sram_tag_rdata[i]); 
assign datm_rdata[i] = ic_sram_data_rdata[i]; 
end 
assign ras_ic_tag_ce = 1'b0; 
assign ras_ic_dat_ce = 1'b0; 
assign ras_ic_ce_addr = 32'b0; 
assign ecc_back_press_fetch = 1'b0; 
assign mem_ecc_dec_err = 1'b0; 
assign ecc_invld_tag = 1'b0; 
assign iccinv_line_ecc_err = {2{1'b0}}; 
`endif 
assign ic_chk_tag = ic_bein_invld_line ? mmr_iccinvaddr[32-1 -: (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))] : fetch_chk_tag; 
for (genvar i=0; i<2; i++) begin: g_tag_chk 
assign tag_vld_per_way[i] = tag_dat_per_way[i].vld; 
assign tag_invld_per_way[i] = ~tag_dat_per_way[i].vld; 
assign tag_hit_per_way[i] = tag_vld_per_way[i] & (tag_dat_per_way[i].tag == ic_chk_tag); 
end 
assign tag_hit = |tag_hit_per_way; 
assign cl_tag_hit_pre = tag_hit & ~mem_ecc_dec_err; 
assign cl_tag_hit = fetch_chk_vld_q & cl_tag_hit_pre; 
assign cl_tag_miss = fetch_chk_vld_q & ~cl_tag_hit_pre; 
`WCBB_AOMUX(datm_rdata, tag_hit_per_way, cl_mux_data) 
assign iccinv_line_hit = iccinv_line_r_rsp & (tag_hit | (|iccinv_line_ecc_err)); 
assign iccinv_line_miss = iccinv_line_r_rsp ? ~iccinv_line_hit : 1'b0; 
wing_cbb_lzc #( 
.WIDTH ( 2 ), 
.DIRECTION ( 0 ) 
) 
u_wing_cbb_lzc( 
.vec_i ( tag_invld_per_way ), 
.cnt_o ( way_invld ), 
.empty_o ( ways_all_vld ) 
); 
wing_cbb_lfsr_8bit #( 
.WIDTH ( WAY_IDX_W ) 
) u_lfsr_8bit ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.en_i ( cl_tag_miss ), 
.out_o ( way_random ) 
); 
assign way_replace = ways_all_vld ? way_random : way_invld; 
assign req_refil = missbuf_vld_q; 
assign icache_bmu_ar_hdsk = icache_bmu_ar_valid & bmu_icache_ar_ready; 
assign missbuf_vld_set = cl_tag_miss & ~ifu_mss_flush | 
arb_grt_fetch_nc; 
assign missbuf_vld_clr = refil_tag_vld | (missbuf_nc_q & miss_rsp_vld); 
assign missbuf_vld_en = missbuf_vld_set | missbuf_vld_clr; 
assign missbuf_vld_d = missbuf_vld_set & ~missbuf_vld_clr; 
`WDFFER(missbuf_vld_q, missbuf_vld_d, missbuf_vld_en, clk, rst_n) 
assign missbuf_sent_set = icache_bmu_ar_hdsk & cnt_tx_reached; 
assign missbuf_sent_clr = missbuf_vld_clr; 
assign missbuf_sent_en = missbuf_sent_set | missbuf_sent_clr; 
assign missbuf_sent_d = missbuf_sent_set & ~missbuf_sent_clr; 
assign missbuf_flush_set = missbuf_vld_q & ifu_mss_flush; 
assign missbuf_flush_clr = missbuf_vld_clr; 
assign missbuf_flush_en = missbuf_flush_set | missbuf_flush_clr; 
assign missbuf_flush_d = missbuf_flush_set & ~missbuf_flush_clr; 
assign missbuf_nc_en = missbuf_vld_set; 
assign missbuf_nc_d = arb_grt_fetch_nc ? 1'b1 : 
fetch_chk_nc_q; 
assign missbuf_addr_en = missbuf_vld_set; 
assign missbuf_addr_d = arb_grt_fetch_nc ? slv_req_addr : 
registered_req_addr; 
assign missbuf_way_idx_en = missbuf_vld_set; 
assign missbuf_way_idx_d = way_replace; 
assign missbuf_cnt_tx_en = icache_bmu_ar_hdsk | missbuf_vld_clr; 
assign missbuf_cnt_tx_d = missbuf_vld_clr ? '0 : 
missbuf_cnt_tx_incr; 
assign missbuf_cnt_tx_incr = missbuf_cnt_tx_q + 1'b1; 
assign missbuf_cnt_rx_en = bmu_icache_r_valid | missbuf_vld_clr; 
assign missbuf_cnt_rx_d = missbuf_vld_clr ? '0 : missbuf_cnt_rx_incr; 
assign missbuf_cnt_rx_incr = missbuf_cnt_rx_q + 1'b1; 
`WDFFER(missbuf_sent_q , missbuf_sent_d , missbuf_sent_en , clk, rst_n) 
`WDFFER(missbuf_flush_q , missbuf_flush_d , missbuf_flush_en , clk, rst_n) 
`WDFFER(missbuf_nc_q , missbuf_nc_d , missbuf_nc_en , clk, rst_n) 
`WDFFER(missbuf_addr_q , missbuf_addr_d , missbuf_addr_en , clk, rst_n) 
`WDFFER(missbuf_way_idx_q, missbuf_way_idx_d, missbuf_way_idx_en, clk, rst_n) 
`WDFFER(missbuf_cnt_tx_q , missbuf_cnt_tx_d , missbuf_cnt_tx_en , clk, rst_n) 
`WDFFER(missbuf_cnt_rx_q , missbuf_cnt_rx_d , missbuf_cnt_rx_en , clk, rst_n) 
assign nc_fetch_half = missbuf_nc_q & missbuf_addr_q[2]; 
assign missbuf_addr_offs_tx = nc_fetch_half ? missbuf_addr_q[$clog2(MST_IF_DATA_BYTE_W) +: REFIL_CNT_W] : missbuf_addr_offs_tx_q; 
assign nc_sent_done = missbuf_addr_q[2] ? (missbuf_cnt_tx_q == 3'b0) : (missbuf_cnt_tx_q == 3'b1); 
assign crit_word_rdy = nc_fetch_half ? (missbuf_cnt_rx_q == 3'b0) : (missbuf_cnt_rx_q == 3'b1); 
assign fetch_addr_offs_init = missbuf_addr_d[$clog2(MST_IF_DATA_BYTE_W) +: REFIL_CNT_W]; 
assign missbuf_addr_offs_tx_en = missbuf_vld_set | icache_bmu_ar_hdsk; 
assign missbuf_addr_offs_tx_d = missbuf_vld_set ? {fetch_addr_offs_init[REFIL_CNT_W-1:1], 1'b0} : 
missbuf_addr_offs_tx_incr; 
assign missbuf_addr_offs_tx_incr = missbuf_addr_offs_tx_q + 1'b1; 
assign missbuf_addr_offs_rx_en = missbuf_vld_set | bmu_icache_r_valid; 
assign missbuf_addr_offs_rx_d = missbuf_vld_set ? {fetch_addr_offs_init[REFIL_CNT_W-1:1], 1'b0} : 
missbuf_addr_offs_rx_incr; 
assign missbuf_addr_offs_rx_incr = missbuf_addr_offs_rx_q + 1'b1; 
`WDFFER(missbuf_addr_offs_tx_q, missbuf_addr_offs_tx_d, missbuf_addr_offs_tx_en, clk, rst_n) 
`WDFFER(missbuf_addr_offs_rx_q, missbuf_addr_offs_rx_d, missbuf_addr_offs_rx_en, clk, rst_n) 
assign cnt_tx_reached = missbuf_nc_q & nc_sent_done | 
~missbuf_nc_q & (missbuf_cnt_tx_q == 3'd7) ; 
assign cnt_rx_reached = &missbuf_cnt_rx_q; 
assign dff_lo_data_en = bmu_icache_r_valid & ~missbuf_cnt_rx_q[0]; 
assign dff_lo_data_d = bmu_icache_r_data; 
`WDFFER(dff_lo_data_q, dff_lo_data_d, dff_lo_data_en, clk, rst_n) 
assign cnt_rx_eq_1st2 = (missbuf_cnt_rx_q == 3'd0 | 
missbuf_cnt_rx_q == 3'd1 ); 
assign dff_resp_1st2_en = bmu_icache_r_valid & cnt_rx_eq_1st2 | 
missbuf_vld_clr; 
assign dff_resp_1st2_d = missbuf_vld_clr ? '0 : 
(missbuf_cnt_rx_q == 3'd0) ? bmu_icache_r_resp : 
(bmu_icache_r_resp | dff_resp_1st2_q); 
`WDFFER(dff_resp_1st2_q, dff_resp_1st2_d, dff_resp_1st2_en, clk, rst_n) 
assign dff_resp_rest_en = bmu_icache_r_valid & ~cnt_rx_eq_1st2 | 
missbuf_vld_clr; 
assign dff_resp_rest_d = missbuf_vld_clr ? '0 : 
(missbuf_cnt_rx_q == 3'd2) ? bmu_icache_r_resp : 
(bmu_icache_r_resp | dff_resp_rest_q); 
`WDFFER(dff_resp_rest_q, dff_resp_rest_d, dff_resp_rest_en, clk, rst_n) 
assign refil_data_vld = bmu_icache_r_valid & missbuf_cnt_rx_q[0] & ~missbuf_nc_q; 
assign refil_tag_vld = bmu_icache_r_valid & cnt_rx_reached; 
assign refil_addr = {missbuf_addr_q[31:$clog2(32)], missbuf_addr_offs_rx_q, 2'b00}; 
`WCBB_BIN2ONEHOT(missbuf_way_idx_q, way_replace_oh) 
assign datm_cs_refil = {2{refil_data_vld}} & way_replace_oh; 
assign datm_data_refil = {bmu_icache_r_data, dff_lo_data_q}; 
assign tagm_cs_refil = {2{refil_tag_vld}} & way_replace_oh; 
assign tagm_data_refil.vld = ~|dff_resp_1st2_q & ~dff_resp_rest_q[1] & ~|bmu_icache_r_resp; 
assign tagm_data_refil.tag = refil_addr[32-1 -: (32 - $clog2(32) - $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)))]; 
assign dff_init_done_en = ~dff_init_done_q & cnt_inv_reached; 
assign dff_init_done_d = 1'b1; 
`WDFFER(dff_init_done_q, dff_init_done_d, dff_init_done_en, clk, rst_n) 
assign mss_pcu_icache_init_busy = ~dff_init_done_q; 
assign dff_fencei_set = lsu_mss_fencei_req; 
assign dff_fencei_clr = cnt_inv_reached; 
assign dff_fencei_en = dff_fencei_set | dff_fencei_clr; 
assign dff_fencei_d = dff_fencei_set & ~dff_fencei_clr; 
assign dff_inv_all_set = mmr_iccinv_all; 
assign dff_inv_all_clr = cnt_inv_reached; 
assign dff_inv_all_en = dff_inv_all_set | dff_inv_all_clr; 
assign dff_inv_all_d = dff_inv_all_set & ~dff_inv_all_clr; 
assign dff_inv_line_set = mmr_iccinv_line; 
assign dff_inv_line_clr = cnt_inv_reached | (iccinv_line_miss | iccinv_line_wr); 
assign dff_inv_line_en = dff_inv_line_set | dff_inv_line_clr; 
assign dff_inv_line_d = dff_inv_line_set & ~dff_inv_line_clr; 
`WDFFER(dff_fencei_q, dff_fencei_d, dff_fencei_en, clk, rst_n) 
`WDFFER(dff_inv_all_q, dff_inv_all_d, dff_inv_all_en, clk, rst_n) 
`WDFFER(dff_inv_line_q, dff_inv_line_d, dff_inv_line_en, clk, rst_n) 
assign req_invld_all = dff_fencei_q | dff_inv_all_q | ~dff_init_done_q; 
assign req_invld_line = dff_inv_line_q; 
assign req_invld = req_invld_all | req_invld_line; 
assign ic_bein_invld_all = arb_grt_invld & req_invld_all; 
assign ic_bein_invld_line = arb_grt_invld & ~req_invld_all; 
assign dff_cnt_inv_en = arb_grt_invld & ic_bein_invld_all; 
assign dff_cnt_inv_d = dff_cnt_inv_q + 1'b1; 
`WDFFER(dff_cnt_inv_q, dff_cnt_inv_d, dff_cnt_inv_en, clk, rst_n) 
assign cnt_inv_reached = &dff_cnt_inv_q; 
assign dff_inv_line_r_req_set = mmr_iccinv_line; 
assign dff_inv_line_r_req_clr = ic_bein_invld_line; 
assign dff_inv_line_r_req_en = dff_inv_line_r_req_set | dff_inv_line_r_req_clr; 
assign dff_inv_line_r_req_d = dff_inv_line_r_req_set & ~dff_inv_line_r_req_clr; 
assign dff_inv_line_r_rsp_set = iccinv_line_r_req; 
assign dff_inv_line_r_rsp_clr = dff_inv_line_r_rsp_q; 
assign dff_inv_line_r_rsp_en = dff_inv_line_r_rsp_set | dff_inv_line_r_rsp_clr; 
assign dff_inv_line_r_rsp_d = dff_inv_line_r_rsp_set & ~dff_inv_line_r_rsp_clr; 
assign dff_inv_line_wr_set = iccinv_line_r_rsp & iccinv_line_hit; 
assign dff_inv_line_wr_clr = dff_inv_line_wr_q; 
assign dff_inv_line_wr_en = dff_inv_line_wr_set | dff_inv_line_wr_clr; 
assign dff_inv_line_wr_d = dff_inv_line_wr_set & ~dff_inv_line_wr_clr; 
`WDFFER(dff_inv_line_r_req_q, dff_inv_line_r_req_d, dff_inv_line_r_req_en, clk, rst_n) 
`WDFFER(dff_inv_line_r_rsp_q, dff_inv_line_r_rsp_d, dff_inv_line_r_rsp_en, clk, rst_n) 
`WDFFER(dff_inv_line_wr_q, dff_inv_line_wr_d, dff_inv_line_wr_en, clk, rst_n) 
assign iccinv_line_r_req = dff_inv_line_r_req_q & ic_bein_invld_line; 
assign iccinv_line_r_rsp = dff_inv_line_r_rsp_q & ic_bein_invld_line; 
assign iccinv_line_wr = dff_inv_line_wr_q & ic_bein_invld_line; 
assign dff_inv_line_cs_en = iccinv_line_r_rsp; 
assign dff_inv_line_cs_d = {2{iccinv_line_r_rsp}} & (tag_hit_per_way | iccinv_line_ecc_err); 
`WDFFER(dff_inv_line_cs_q, dff_inv_line_cs_d, dff_inv_line_cs_en, clk, rst_n) 
assign tagm_cs_invld_all = {2{ic_bein_invld_all}}; 
assign tagm_cs_iccinv_rd = {2{iccinv_line_r_req}}; 
assign tagm_cs_iccinv_wr = dff_inv_line_cs_q & {2{iccinv_line_wr}}; 
assign tagm_cs_invld_line = tagm_cs_iccinv_rd | tagm_cs_iccinv_wr; 
assign tagm_cs_invld = tagm_cs_invld_all | tagm_cs_invld_line; 
assign invld_tag = ic_bein_invld_all | iccinv_line_wr; 
assign invld_addr = {$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)){ic_bein_invld_all }} & dff_cnt_inv_q | 
{$clog2((`M130_CORE_ICACHE_SIZE / 2 / 32)){ic_bein_invld_line}} & mmr_iccinvaddr[TAG_SRAM_ADDR_OFFS_W +: $clog2((`M130_CORE_ICACHE_SIZE / 2 / 32))]; 
assign mss_lsu_fencei_done = dff_fencei_q & dff_fencei_clr; 
assign mmr_not_busy = ~(dff_inv_all_q & ~dff_inv_all_clr | dff_inv_line_q & ~dff_inv_line_clr); 
assign mmr_iccinv_done = dff_inv_all_q & dff_inv_all_clr | dff_inv_line_q & dff_inv_line_clr; 
assign icache_bmu_ar_valid = missbuf_vld_q & ~missbuf_sent_q; 
assign icache_bmu_ar_addr = {missbuf_addr_q[31:$clog2(32)], missbuf_addr_offs_tx, 2'b00}; 
assign icache_bmu_ar_cache = fetch_chk_nc_q ? 4'b0011 : 4'b1111; 
assign icache_bmu_ar_prot = {1'b1, 1'b1, fetch_chk_priv_q}; 
assign icache_bmu_ar_user = fetch_chk_user_q; 
assign slv_req_rdy_o = arb_grt_fetch; 
assign miss_rsp_vld = bmu_icache_r_valid & crit_word_rdy; 
assign ic_r_vld = cl_tag_hit | miss_rsp_vld ; 
assign slv_rsp_vld_o = ic_r_vld; 
assign slv_rsp_data = cl_tag_hit ? cl_mux_data : datm_data_refil; 
assign slv_rsp_resp = cl_tag_hit ? 2'b00 : (dff_resp_1st2_q | bmu_icache_r_resp); 
assign ras_bus_fetch_uue = 1'b0; 
assign ras_bus_refil_uue = missbuf_vld_q & ~cnt_rx_eq_1st2 & ~missbuf_flush_q & 
bmu_icache_r_valid & bmu_icache_r_resp[1]; 
assign missbuf_addr = missbuf_addr_q; 
endmodule
 
 
module m130_mss_downsizer #( 
parameter ADDR_W = 32, 
parameter ERR_W = 1, 
parameter type info_t = logic, 
parameter REQ_FORWARD = 0 
) ( 
input logic clk, 
input logic rst_n, 
input logic slv_req_vld_i, 
output logic slv_req_rdy_o, 
input logic [ADDR_W-1:0] slv_req_addr, 
input info_t slv_req_info, 
output logic slv_rsp_vld_o, 
input logic slv_rsp_rdy_i, 
output logic [63:0] slv_rsp_data, 
output logic [ERR_W-1:0] slv_rsp_err, 
output logic mst_req_vld_o, 
input logic mst_req_rdy_i, 
output logic [ADDR_W-1:0] mst_req_addr, 
output info_t mst_req_info, 
input logic mst_rsp_vld_i, 
output logic mst_rsp_rdy_o, 
input logic [31:0] mst_rsp_data, 
input logic [ERR_W-1:0] mst_rsp_err, 
output logic [ADDR_W-1:0] buf_req_addr 
); 
logic slv_req_hdsk; 
logic mst_req_hdsk; 
logic mst_rsp_hdsk; 
logic missbuf_have_room; 
logic missbuf_vld_clr_from_split; 
logic missbuf_vld_set; 
logic missbuf_vld_clr; 
logic missbuf_vld_en; 
logic missbuf_vld_d; 
logic missbuf_vld_q; 
logic missbuf_sent_en; 
logic missbuf_sent_d; 
logic missbuf_sent_q; 
logic missbuf_addr_en; 
logic [31:0] missbuf_addr_d ; 
logic [31:0] missbuf_addr_q ; 
logic missbuf_info_en; 
info_t missbuf_info_d; 
info_t missbuf_info_q; 
logic dff_sent_lo_set; 
logic dff_sent_lo_clr; 
logic dff_sent_lo_en; 
logic dff_sent_lo_d; 
logic dff_sent_lo_q; 
logic dff_sent_hi_set; 
logic dff_sent_hi_clr; 
logic dff_sent_hi_en; 
logic dff_sent_hi_d; 
logic dff_sent_hi_q; 
logic [1:0] vec_sent; 
logic dff_rcv_lo_set; 
logic dff_rcv_lo_clr; 
logic dff_rcv_lo_en; 
logic dff_rcv_lo_d; 
logic dff_rcv_lo_q; 
logic dff_data_lo_en; 
logic [31:0] dff_data_lo_d; 
logic [31:0] dff_data_lo_q; 
logic dff_rsp_lo_en; 
logic [ERR_W-1:0] dff_rsp_lo_d; 
logic [ERR_W-1:0] dff_rsp_lo_q; 
logic only_sent_lo; 
logic req_addr_offs; 
assign slv_req_rdy_o = missbuf_have_room; 
assign slv_req_hdsk = slv_req_vld_i & slv_req_rdy_o; 
assign mst_rsp_rdy_o = 1'b1; 
assign mst_req_hdsk = mst_req_vld_o & mst_req_rdy_i; 
assign mst_rsp_hdsk = mst_rsp_vld_i & mst_rsp_rdy_o; 
assign missbuf_vld_clr_from_split = ~missbuf_addr_q[2] & (dff_rcv_lo_q | only_sent_lo) | 
missbuf_addr_q[2]; 
assign missbuf_vld_set = slv_req_hdsk ; 
assign missbuf_vld_clr = mst_rsp_hdsk & missbuf_vld_clr_from_split; 
assign missbuf_vld_en = missbuf_vld_set | missbuf_vld_clr; 
assign missbuf_vld_d = missbuf_vld_set; 
`WDFFER(missbuf_vld_q, missbuf_vld_d, missbuf_vld_en, clk, rst_n) 
assign missbuf_have_room = ~missbuf_vld_q | missbuf_vld_clr; 
assign missbuf_sent_en = mst_req_hdsk | missbuf_vld_clr; 
if (REQ_FORWARD == 1) begin : g_sent_d_forward 
assign missbuf_sent_d = mst_req_hdsk & 
(missbuf_have_room ? 
slv_req_addr[2] : 
(missbuf_vld_q & (~missbuf_addr_q[2] & dff_sent_lo_q 
| missbuf_addr_q[2]))); 
end else begin :g_sent_d_non_forward 
assign missbuf_sent_d = mst_req_hdsk & missbuf_vld_q & (~missbuf_addr_q[2] & dff_sent_lo_q | 
missbuf_addr_q[2]); 
end 
`WDFFER(missbuf_sent_q , missbuf_sent_d , missbuf_sent_en , clk, rst_n) 
assign missbuf_info_en = missbuf_vld_set; 
assign missbuf_info_d = slv_req_info; 
`WDFFER(missbuf_info_q, missbuf_info_d, missbuf_info_en, clk, rst_n) 
assign missbuf_addr_en = missbuf_vld_set; 
assign missbuf_addr_d = slv_req_addr; 
`WDFFER(missbuf_addr_q, missbuf_addr_d, missbuf_addr_en, clk, rst_n) 
assign buf_req_addr = missbuf_addr_q; 
assign only_sent_lo = missbuf_sent_q & (vec_sent == 2'b01); 
if (REQ_FORWARD == 1) begin : g_sent_lo_forward 
assign dff_sent_lo_set = mst_req_hdsk & (missbuf_vld_q & ~missbuf_addr_q[2] | 
missbuf_have_room & ~slv_req_addr[2]); 
end else begin : g_sent_lo_non_forward 
assign dff_sent_lo_set = mst_req_hdsk & missbuf_vld_q & ~missbuf_addr_q[2]; 
end 
assign dff_sent_lo_clr = missbuf_vld_clr; 
assign dff_sent_lo_en = dff_sent_lo_set | dff_sent_lo_clr; 
assign dff_sent_lo_d = dff_sent_lo_set; 
assign dff_sent_hi_set = mst_req_hdsk & (~missbuf_addr_q[2] & (vec_sent == 2'b01) | 
missbuf_addr_q[2]); 
assign dff_sent_hi_clr = missbuf_vld_clr; 
assign dff_sent_hi_en = dff_sent_hi_set | dff_sent_hi_clr; 
assign dff_sent_hi_d = dff_sent_hi_set & ~dff_sent_hi_clr; 
`WDFFER(dff_sent_lo_q, dff_sent_lo_d, dff_sent_lo_en, clk, rst_n) 
`WDFFER(dff_sent_hi_q, dff_sent_hi_d, dff_sent_hi_en, clk, rst_n) 
assign vec_sent = {dff_sent_hi_q, dff_sent_lo_q}; 
assign dff_rcv_lo_set = mst_rsp_hdsk & ~missbuf_addr_q[2] & ~dff_rcv_lo_q & ~only_sent_lo; 
assign dff_rcv_lo_clr = mst_rsp_hdsk & dff_rcv_lo_q; 
assign dff_rcv_lo_en = dff_rcv_lo_set | dff_rcv_lo_clr; 
assign dff_rcv_lo_d = dff_rcv_lo_set & ~dff_rcv_lo_clr; 
`WDFFER(dff_rcv_lo_q, dff_rcv_lo_d, dff_rcv_lo_en, clk, rst_n) 
assign dff_data_lo_en = dff_rcv_lo_set; 
assign dff_data_lo_d = mst_rsp_data; 
`WDFFER(dff_data_lo_q, dff_data_lo_d, dff_data_lo_en, clk, rst_n) 
assign dff_rsp_lo_en = dff_rcv_lo_set | missbuf_vld_clr; 
assign dff_rsp_lo_d = {ERR_W{dff_rcv_lo_set}} & mst_rsp_err; 
`WDFFER(dff_rsp_lo_q, dff_rsp_lo_d, dff_rsp_lo_en, clk, rst_n) 
assign req_addr_offs = ~missbuf_addr_q[2] & dff_sent_lo_q | 
missbuf_addr_q[2]; 
if (REQ_FORWARD == 1) begin : g_mst_interface_forward 
assign mst_req_vld_o = missbuf_have_room & slv_req_vld_i | 
missbuf_vld_q & ~missbuf_sent_q ; 
assign mst_req_addr = missbuf_have_room ? {slv_req_addr[ADDR_W-1:2], 2'b00} : 
{missbuf_addr_q[ADDR_W-1:3], req_addr_offs, 2'b00}; 
assign mst_req_info = missbuf_have_room ? missbuf_info_d : 
missbuf_info_q ; 
end else begin :g_mst_interface_non_forward 
assign mst_req_vld_o = missbuf_vld_q & ~missbuf_sent_q; 
assign mst_req_addr = {missbuf_addr_q[ADDR_W-1:3], req_addr_offs, 2'b00}; 
assign mst_req_info = missbuf_info_q ; 
end 
assign slv_rsp_vld_o = mst_rsp_vld_i & ((~missbuf_addr_q[2] & dff_rcv_lo_q) | missbuf_addr_q[2]); 
assign slv_rsp_data = {mst_rsp_data, dff_data_lo_q}; 
assign slv_rsp_err = mst_rsp_err | dff_rsp_lo_q; 
endmodule
 
 

 
 
module wing_mss_pendbuf #( 
parameter type req_info_t = logic, 
parameter EN_REQ_VLD_SYNC = 0 
) ( 
input logic clk , 
input logic rst_n , 
input logic flush , 
input logic slv_req_vld_i , 
output logic slv_req_rdy_o , 
input req_info_t slv_req_info_i, 
output logic slv_rsp_vld_o, 
output logic mst_req_vld_o, 
input logic mst_req_rdy_i, 
output req_info_t mst_req_info_o, 
input logic mst_rsp_vld_i 
); 
logic slv_req_hdsk; 
logic mst_req_hdsk; 
logic dff_float_flush_set; 
logic dff_float_flush_clr; 
logic dff_float_flush_en ; 
logic dff_float_flush_d ; 
logic dff_float_flush_q ; 
logic dff_pendbuf_vld_set; 
logic dff_pendbuf_vld_clr; 
logic dff_pendbuf_vld_en ; 
logic dff_pendbuf_vld_d ; 
logic dff_pendbuf_vld_q ; 
logic dff_pendbuf_flush_set; 
logic dff_pendbuf_flush_clr; 
logic dff_pendbuf_flush_en ; 
logic dff_pendbuf_flush_d ; 
logic dff_pendbuf_flush_q ; 
logic dff_pendbuf_info_en; 
req_info_t dff_pendbuf_info_d; 
req_info_t dff_pendbuf_info_q; 
logic flush_all; 
logic flush_pendbuf; 
if (EN_REQ_VLD_SYNC == 1) begin : g_sync_slv_req_vld 
logic dff_sync_slv_req_d; 
logic dff_sync_slv_req_q; 
logic slv_req_cancel; 
assign dff_sync_slv_req_d = slv_req_vld_i; 
`WDFFR(dff_sync_slv_req_q, dff_sync_slv_req_d, clk, rst_n) 
assign slv_req_cancel = ~slv_req_vld_i & dff_sync_slv_req_q; 
assign flush_all = slv_req_cancel | flush | dff_pendbuf_flush_q; 
assign flush_pendbuf = slv_req_cancel | flush; 
end else begin : g_no_sync 
assign flush_all = dff_pendbuf_flush_q | flush; 
assign flush_pendbuf = flush; 
end 
assign slv_req_hdsk = slv_req_vld_i & slv_req_rdy_o; 
assign mst_req_hdsk = mst_req_vld_o & mst_req_rdy_i; 
assign dff_float_flush_set = mst_req_hdsk & dff_pendbuf_vld_q & flush_all; 
assign dff_float_flush_clr = mst_rsp_vld_i; 
assign dff_float_flush_en = dff_float_flush_set | dff_float_flush_clr; 
assign dff_float_flush_d = dff_float_flush_set; 
`WDFFER(dff_float_flush_q, dff_float_flush_d, dff_float_flush_en, clk, rst_n) 
assign dff_pendbuf_vld_set = slv_req_vld_i & ~slv_req_rdy_o & ~dff_pendbuf_vld_q; 
assign dff_pendbuf_vld_clr = mst_req_hdsk; 
assign dff_pendbuf_vld_en = dff_pendbuf_vld_set | dff_pendbuf_vld_clr; 
assign dff_pendbuf_vld_d = dff_pendbuf_vld_set; 
`WDFFER(dff_pendbuf_vld_q, dff_pendbuf_vld_d, dff_pendbuf_vld_en, clk, rst_n) 
assign dff_pendbuf_flush_set = dff_pendbuf_vld_q & flush_pendbuf; 
assign dff_pendbuf_flush_clr = mst_req_hdsk; 
assign dff_pendbuf_flush_en = dff_pendbuf_flush_set | dff_pendbuf_flush_clr; 
assign dff_pendbuf_flush_d = dff_pendbuf_flush_set & ~dff_pendbuf_flush_clr; 
`WDFFER(dff_pendbuf_flush_q, dff_pendbuf_flush_d, dff_pendbuf_flush_en, clk, rst_n) 
assign dff_pendbuf_info_en = dff_pendbuf_vld_set; 
assign dff_pendbuf_info_d = slv_req_info_i; 
`WDFFENR(dff_pendbuf_info_q, dff_pendbuf_info_d, dff_pendbuf_info_en, clk) 
assign slv_rsp_vld_o = mst_rsp_vld_i & ~dff_float_flush_q; 
assign mst_req_vld_o = slv_req_vld_i | dff_pendbuf_vld_q; 
assign mst_req_info_o = dff_pendbuf_vld_q ? dff_pendbuf_info_q : slv_req_info_i; 
assign slv_req_rdy_o = mst_req_rdy_i & ~(dff_pendbuf_vld_q & flush_all); 
endmodule
 
 
module wing_clic ( 
input logic clk , 
input logic cfg_clk, 
input logic rst_n, 
output logic cfg_clk_en, 
input logic srcr_rst_n , 
output logic soft_rst_req , 
input logic refclk , 
output logic [63:0] mtime , 
input logic [32+1:0] timer_calibration , 
input logic soc_clic_ext_int , 
input logic ras_int, 
input logic bmu_clic_req_vld , 
output logic clic_bmu_req_rdy , 
input logic [31:0] bmu_clic_req_addr , 
input logic [ 1:0] bmu_clic_req_size , 
input logic bmu_clic_req_write, 
input logic [32/8-1:0] bmu_clic_req_wstrb, 
input logic [32-1:0] bmu_clic_req_wdata, 
input logic [1:0] bmu_clic_req_dest , 
output logic clic_bmu_rsp_vld , 
input logic bmu_clic_rsp_rdy , 
output logic [32-1:0] clic_bmu_rsp_rdata, 
output logic clic_bmu_rsp_err , 
input logic pcu_clic_stoptime , 
input logic [64-1:0] soc_clic_loc_int , 
output logic clic_pcu_int_vld , 
output logic [$clog2(64 + 32)-1:0] clic_pcu_int_id , 
output logic [8-1:0] clic_pcu_int_lvl , 
output logic clic_pcu_int_shv , 
output logic [1:0] clic_pcu_int_priv , 
input logic pcu_clic_rsp_vld , 
input logic [$clog2(64 + 32)-1:0] pcu_clic_rsp_id , 
input logic pcu_clic_rsp_clr_ip, 
output logic clic_tm_intctl_vld 
); 
//`include "wing_clic_mmr_config.svh" 
typedef struct packed { 
logic[2:0] static_flag; 
logic[4-1:0] ctl ; 
logic[1:0] trig ; 
logic shv ; 
} clic_mmr_cfg_s; 
localparam clic_mmr_cfg_s [64-1 : 0] CLIC_UNIT_MMR_CFG = { 
64{3'b000, {4{1'b0}}, 2'b00, 1'b1} 
}; 

parameter NUM_INTERRUPT = 64 + 32; 
localparam ADDR_OFFS_CLINT_MTIME_LO = 12'h000; 
localparam ADDR_OFFS_CLINT_MTIME_HI = 12'h004; 
localparam ADDR_OFFS_CLINT_MTIMECMP_LO = 12'h008; 
localparam ADDR_OFFS_CLINT_MTIMECMP_HI = 12'h00C; 
localparam ADDR_OFFS_CLINT_MTIMECTL = 12'h010; 
localparam ADDR_OFFS_CLINT_MSIP = 12'h020; 
localparam ADDR_OFFS_CLINT_MTIMECALIB_LO = 12'h028; 
localparam ADDR_OFFS_CLINT_MTIMECALIB_HI = 12'h02C; 
localparam ADDR_OFFS_CLINT_SRCR = 12'h030; 
localparam ADDR_OFFS_CLIC_CFG = 15'h0000; 
localparam ADDR_OFFS_CLIC_TRIG = 15'h0040; 
localparam ADDR_OFFS_CLIC_UNIT = 15'h1000; 
localparam UNIT_OFFS_IP = 0; 
localparam UNIT_OFFS_IE = 8; 
localparam UNIT_OFFS_SHV = 16; 
localparam UNIT_OFFS_TRIG = 17; 
localparam UNIT_OFFS_CTL = 24; 
localparam CLIC_EN_INT_PPLN = 1; 
localparam CLIC_ASYNC_STAGE_N = 2; 
localparam logic [11:0] CLIC_ARB_PPLN_VLD = 12'b0; 
localparam logic [31:0] CLINT_INT_VLD = {1'b1, 19'b0, 1'b1, 3'b0, 1'b1, 3'b0, 1'b1, 3'b0}; 
logic [NUM_INTERRUPT-1:0] unit_int_src; 
logic [NUM_INTERRUPT-1:0] unit_int_en_pd; 
logic [NUM_INTERRUPT-1:0] unit_ip_hw_clear; 
logic [NUM_INTERRUPT-1:0] clic2unit_wvld_clicintip ; 
logic [NUM_INTERRUPT-1:0] clic2unit_wvld_clicintie ; 
logic [NUM_INTERRUPT-1:0] clic2unit_wvld_clicintattr; 
logic [NUM_INTERRUPT-1:0] clic2unit_wvld_clicintctl ; 
logic clic2unit_wdata_ip ; 
logic clic2unit_wdata_ie ; 
logic clic2unit_wdata_shv ; 
logic [1:0] clic2unit_wdata_trig ; 
logic [7:0] clic2unit_wdata_ctl ; 
logic [NUM_INTERRUPT-1:0] unit2clic_reg_ip ; 
logic [NUM_INTERRUPT-1:0] unit2clic_reg_ie ; 
logic [NUM_INTERRUPT-1:0] unit2clic_reg_shv ; 
logic [NUM_INTERRUPT-1:0][1:0] unit2clic_reg_trig ; 
logic [NUM_INTERRUPT-1:0][1:0] unit2clic_reg_mode ; 
logic [NUM_INTERRUPT-1:0][7:0] unit2clic_reg_ctl ; 
logic [$clog2(64 + 32)-1:0] vld_en_pd_int ; 
logic bmu_sel_mtime_lo; 
logic bmu_sel_mtime_hi; 
logic bmu_sel_mtimecmp_lo; 
logic bmu_sel_mtimecmp_hi; 
logic bmu_sel_mtimectrl ; 
logic bmu_sel_msip ; 
logic bmu_sel_mtimecalib_lo; 
logic bmu_sel_mtimecalib_hi; 
logic bmu_sel_srcr ; 
logic bmu_sel_mcliccfg ; 
logic bmu_sel_clictrig ; 
logic [NUM_INTERRUPT-1:0] bmu_sel_unit ; 
logic [NUM_INTERRUPT-1:0][15:0] unit_addr_offs ; 
logic [31:0] bmu_rdata ; 
logic bmu_err; 
logic bmu_clint_err; 
logic bmu_clic_err; 
logic bmu_access_err; 
logic clic_bmu_req_hdsk; 
logic postsync_ext_int; 
logic [64-1:0] postsync_loc_int; 
logic [31:0] mmr_srcr; 
logic ff_msip_en; 
logic ff_msip_d ; 
logic ff_msip_q ; 
logic [31:0] mmr_msip; 
logic ff_mtimecmp_hi_en; 
logic ff_mtimecmp_lo_en; 
logic ff_mtimecmp_en; 
logic [32-1:0] ff_mtimecmp_d; 
logic [32-1:0] ff_mtimecmp_q; 
logic [63:0] mmr_mtimecmp; 
logic [63:0] mmr_mtime; 
logic [31:0] mmr_mtimectrl; 
logic [31:0] mmr_mtimecalib_lo; 
logic [31:0] mmr_mtimecalib_hi; 
logic int_src_msip; 
logic int_src_mtip; 
logic clic_csip ; 
logic ff_mcliccfg_nlbits_en; 
logic [3:0] ff_mcliccfg_nlbits_d ; 
logic [3:0] ff_mcliccfg_nlbits_q ; 
logic [31:0] mmr_mcliccfg ; 
logic [31:0] trig_sel_rdata ; 
logic bmu_clic_dest ; 
logic bmu_clint_dest ; 
logic bmu_selected_ip ; 
logic bmu_selected_ie ; 
logic bmu_selected_shv ; 
logic [7:0] bmu_selected_ctl ; 
logic [1:0] bmu_selected_trig; 
logic [1:0] bmu_selected_mode; 
logic bmu_clint_wr_vld ; 
logic bmu_clic_wr_vld ; 
logic [NUM_INTERRUPT-1:0] pcu_clic_rsp_clr_vec ; 
logic arb_shv ; 
logic [7:0] arb_lvl ; 
logic [$clog2(64 + 32)-1:0] arb_id ; 
logic [31:0] unit_sel_rdata; 
assign cfg_clk_en = bmu_clic_req_vld && bmu_clic_req_write; 
assign bmu_clint_dest = bmu_clic_req_dest[0]; 
assign bmu_clic_dest = bmu_clic_req_dest[1]; 
assign bmu_sel_mtime_lo = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIME_LO[11:0]); 
assign bmu_sel_mtime_hi = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIME_HI[11:0]); 
assign bmu_sel_mtimecmp_lo = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIMECMP_LO[11:0]); 
assign bmu_sel_mtimecmp_hi = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIMECMP_HI[11:0]); 
assign bmu_sel_mtimectrl = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIMECTL[11:0]); 
assign bmu_sel_msip = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MSIP[11:0]); 
assign bmu_sel_mtimecalib_lo = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIMECALIB_LO[11:0]); 
assign bmu_sel_mtimecalib_hi = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_MTIMECALIB_HI[11:0]); 
assign bmu_sel_srcr = bmu_clint_dest & (bmu_clic_req_addr[11:0] == ADDR_OFFS_CLINT_SRCR[11:0]); 
assign bmu_sel_mcliccfg = bmu_clic_dest & (bmu_clic_req_addr[15:0] == ADDR_OFFS_CLIC_CFG); 
for (genvar i=0; i<NUM_INTERRUPT; i++) begin: G_SEL_CLICUNIT 
assign unit_addr_offs[i] = ADDR_OFFS_CLIC_UNIT + 4*i; 
assign bmu_sel_unit[i] = bmu_clic_dest & (bmu_clic_req_addr[15:2] == unit_addr_offs[i][15:2]); 
end 
assign bmu_clint_wr_vld = bmu_clic_req_vld & bmu_clic_req_write 
& bmu_clint_dest & ~bmu_clint_err; 
assign bmu_clic_wr_vld = bmu_clic_req_vld & bmu_clic_req_write 
& bmu_clic_dest & ~bmu_clic_err ; 
assign bmu_rdata = {32{bmu_sel_mtime_lo }} & mmr_mtime[31: 0] | 
{32{bmu_sel_mtime_hi }} & mmr_mtime[63:32] | 
{32{bmu_sel_mtimecmp_lo }} & mmr_mtimecmp[31: 0] | 
{32{bmu_sel_mtimecmp_hi }} & mmr_mtimecmp[63:32] | 
{32{bmu_sel_mtimectrl }} & mmr_mtimectrl | 
{32{bmu_sel_mtimecalib_lo}} & mmr_mtimecalib_lo | 
{32{bmu_sel_mtimecalib_hi}} & mmr_mtimecalib_hi | 
{32{bmu_sel_msip }} & mmr_msip | 
{32{bmu_sel_srcr }} & mmr_srcr | 
{32{bmu_sel_mcliccfg }} & mmr_mcliccfg | 
{32{bmu_sel_clictrig }} & trig_sel_rdata | 
{32{|bmu_sel_unit }} & unit_sel_rdata ; 
assign clic_bmu_req_rdy = 1'b1; 
assign clic_bmu_req_hdsk = bmu_clic_req_vld & clic_bmu_req_rdy; 
assign bmu_access_err = (bmu_clic_req_size != 2'b10) 
| (bmu_clic_req_write & ~&bmu_clic_req_wstrb); 
assign bmu_clint_err = bmu_access_err; 
assign bmu_clic_err = bmu_access_err & 
~(bmu_clic_req_addr[15:0] >= 16'h1000 & bmu_clic_req_addr[15:0] <= 16'h4FFF); 
assign bmu_err = bmu_clic_req_vld & ( bmu_clint_dest & bmu_clint_err 
| bmu_clic_dest & bmu_clic_err ); 
if (1 == 1) begin : g_mmr_ppln 
`WDFFR(clic_bmu_rsp_vld, clic_bmu_req_hdsk, clk, rst_n) 
`WDFFER(clic_bmu_rsp_err, bmu_err, clic_bmu_req_hdsk, clk, rst_n) 
`WDFFENR(clic_bmu_rsp_rdata, bmu_rdata, clic_bmu_req_hdsk, clk) 
end else begin : g_mmr_feedthrough 
assign clic_bmu_rsp_vld = bmu_clic_req_vld; 
assign clic_bmu_rsp_rdata = bmu_rdata; 
assign clic_bmu_rsp_err = bmu_err; 
end 
assign mtime = mmr_mtime; 
assign postsync_ext_int = soc_clic_ext_int; 
logic ff_srcr_rreq_en; 
logic ff_srcr_rreq_d; 
logic ff_srcr_rreq_q; 
assign ff_srcr_rreq_en = bmu_clint_wr_vld & bmu_sel_srcr; 
assign ff_srcr_rreq_d = bmu_clic_req_wdata[0]; 
`WDFFER(ff_srcr_rreq_q, ff_srcr_rreq_d, ff_srcr_rreq_en, cfg_clk, srcr_rst_n) 
assign soft_rst_req = ff_srcr_rreq_q; 
assign mmr_srcr = {31'b0, ff_srcr_rreq_q}; 
if ( CLINT_INT_VLD[3] == 1'b1 ) begin : MSIP_VLD 
assign ff_msip_en = bmu_clint_wr_vld & bmu_sel_msip; 
assign ff_msip_d = bmu_clic_req_wdata[0]; 
`WDFFER(ff_msip_q, ff_msip_d, ff_msip_en, cfg_clk, rst_n) 
assign mmr_msip = {31'b0, ff_msip_q}; 
assign int_src_msip = ff_msip_q; 
end else begin : MSIP_UNVLD 
assign mmr_msip = 32'b0; 
assign int_src_msip = 1'b0; 
end 
if ( CLINT_INT_VLD[7] == 1'b1) begin : MTIP_VLD 
assign ff_mtimecmp_lo_en = bmu_clint_wr_vld & bmu_sel_mtimecmp_lo; 
assign ff_mtimecmp_hi_en = bmu_clint_wr_vld & bmu_sel_mtimecmp_hi; 
assign ff_mtimecmp_en = ff_mtimecmp_lo_en | ff_mtimecmp_hi_en; 
for (genvar i=0; i<32; i++) begin : g_mtimecmp_wdata 
if (i<32) begin : lo_mtimecmp_d 
assign ff_mtimecmp_d[i] = ff_mtimecmp_lo_en ? bmu_clic_req_wdata[i] 
: ff_mtimecmp_q[i]; 
end else begin : hi_mtimecmp_d 
assign ff_mtimecmp_d[i] = ff_mtimecmp_hi_en ? bmu_clic_req_wdata[i-32] 
: ff_mtimecmp_q[i]; 
end 
end 
`WDFFER(ff_mtimecmp_q, ff_mtimecmp_d, ff_mtimecmp_en, cfg_clk, rst_n) 
assign mmr_mtimecmp = {{64-32{1'b0}}, ff_mtimecmp_q}; 
assign int_src_mtip = (mmr_mtimecmp[32-1:0] <= mmr_mtime[32-1:0]); 
end else begin : MTIP_UNVLD 
assign mmr_mtimecmp = 64'b0; 
assign int_src_mtip = 1'b0; 
end 
logic dff_mtimectrl_wr_en; 
logic dff_mtimectrl_en_d ; 
logic dff_mtimectrl_en_q ; 
logic dff_mtimectrl_clr_d ; 
logic dff_mtimectrl_clr_q ; 
logic dff_mtime_en; 
logic [32-1:0] dff_mtime_incr; 
logic [32-1:0] dff_mtime_d; 
logic [32-1:0] dff_mtime_q; 
logic [32-1:0] dff_mtime_bmu_wdata; 
logic dff_refclk_ppln_d; 
logic dff_refclk_ppln_q; 
logic mtime_tick; 
logic mtime_clr ; 
logic mtime_wr_lo; 
logic mtime_wr_hi; 
logic mtime_wr; 
assign dff_mtimectrl_wr_en = bmu_clint_wr_vld & bmu_sel_mtimectrl; 
assign dff_mtimectrl_en_d = bmu_clic_req_wdata[0]; 
assign dff_mtimectrl_clr_d = bmu_clic_req_wdata[1]; 
`WDFFER(dff_mtimectrl_en_q, dff_mtimectrl_en_d, dff_mtimectrl_wr_en, cfg_clk, rst_n) 
`WDFFER(dff_mtimectrl_clr_q, dff_mtimectrl_clr_d, dff_mtimectrl_wr_en, cfg_clk, rst_n) 
assign mmr_mtimectrl[1:0] = {dff_mtimectrl_clr_q, dff_mtimectrl_en_q}; 
assign dff_refclk_ppln_d = refclk; 
`WDFFR(dff_refclk_ppln_q, dff_refclk_ppln_d, clk, rst_n) 
assign mtime_clr = dff_mtimectrl_clr_q & int_src_mtip; 
assign mtime_tick = dff_mtimectrl_en_q & 
(dff_refclk_ppln_q == 1'b0) && (dff_refclk_ppln_d == 1'b1) 
&& (~pcu_clic_stoptime) 
; 
assign mtime_wr_lo = bmu_clint_wr_vld & bmu_sel_mtime_lo; 
assign mtime_wr_hi = bmu_clint_wr_vld & bmu_sel_mtime_hi; 
assign mtime_wr = mtime_wr_lo | mtime_wr_hi; 
for (genvar i=0; i<32; i++) begin : g_mtime_wdata 
if (i<32) begin : lo_mtime_wdata_sel 
assign dff_mtime_bmu_wdata[i] = mtime_wr_lo ? bmu_clic_req_wdata[i] : 
dff_mtime_q[i] ; 
end else begin : hi_mtime_wdata_sel 
assign dff_mtime_bmu_wdata[i] = mtime_wr_hi ? bmu_clic_req_wdata[i-32] : 
dff_mtime_q[i] ; 
end 
end 
assign dff_mtime_en = mtime_wr | mtime_clr | mtime_tick; 
assign dff_mtime_d = mtime_wr ? dff_mtime_bmu_wdata : 
mtime_clr ? {32{1'b0}} : 
dff_mtime_incr ; 
assign dff_mtime_incr = dff_mtime_q + 1'b1; 
`WDFFER(dff_mtime_q, dff_mtime_d, dff_mtime_en, clk, rst_n) 
assign mmr_mtime = {{64-32{1'b0}}, dff_mtime_q}; 
assign mmr_mtimectrl[29:2] = 28'b0; 
assign mmr_mtimectrl[30] = timer_calibration[32+1]; 
assign mmr_mtimectrl[31] = timer_calibration[32]; 
if( 32 <= 32 ) begin : g_mtimecalib_le32 
assign mmr_mtimecalib_lo = timer_calibration[32-1:0]; 
assign mmr_mtimecalib_hi = 32'b0; 
end else begin : g_mtimecalib_gt32 
assign mmr_mtimecalib_lo = timer_calibration[31:0]; 
assign mmr_mtimecalib_hi = { {64-32{1'b0}}, timer_calibration[32-1:32]}; 
end 
assign clic_csip = 0; 
assign postsync_loc_int = soc_clic_loc_int; 
assign ff_mcliccfg_nlbits_en = bmu_clic_wr_vld & bmu_sel_mcliccfg; 
assign ff_mcliccfg_nlbits_d = bmu_clic_req_wdata[3:0]; 
`WDFFER(ff_mcliccfg_nlbits_q, ff_mcliccfg_nlbits_d, ff_mcliccfg_nlbits_en, cfg_clk, rst_n) 
assign mmr_mcliccfg = {28'd0, ff_mcliccfg_nlbits_q}; 
if (0>0) begin : EN_TRIG 
logic [0-1:0] ff_clictrig_en_en ; 
logic [0-1:0] ff_clictrig_en_d ; 
logic [0-1:0] ff_clictrig_en_q ; 
logic [0-1:0][12:0] ff_clictrig_inum_en; 
logic [0-1:0][12:0] ff_clictrig_inum_d ; 
logic [0-1:0][12:0] ff_clictrig_inum_q ; 
logic [0-1:0] clic_trig_fire ; 
logic [0-1:0] bmu_sel_clictrig_per_ent ; 
logic [12:0] bmu_selected_clictrig_inum; 
logic bmu_selected_clictrig_en ; 
for (genvar i=0; i<0; i++) begin : G_SEL_TRIG 
assign bmu_sel_clictrig_per_ent[i] = bmu_clic_dest & (bmu_clic_req_addr[15:0] == (ADDR_OFFS_CLIC_TRIG + 4*i)); 
end 
assign bmu_sel_clictrig = |bmu_sel_clictrig_per_ent; 
for (genvar i=0; i<0; i++) begin : G_CLICINTTRIG 
assign ff_clictrig_en_en[i] = bmu_sel_clictrig_per_ent[i] & bmu_clic_wr_vld; 
assign ff_clictrig_en_d [i] = bmu_clic_req_wdata[31]; 
`WDFFER(ff_clictrig_en_q[i], ff_clictrig_en_d[i], ff_clictrig_en_en[i], cfg_clk, rst_n) 
for(genvar j=0; j<13; j++) begin : G_CLICINTTRIG_INUM 
assign ff_clictrig_inum_en[i][j] = bmu_sel_clictrig_per_ent[i] & bmu_clic_wr_vld; 
assign ff_clictrig_inum_d [i][j] = bmu_clic_req_wdata[j]; 
`WDFFER(ff_clictrig_inum_q[i][j], ff_clictrig_inum_d[i][j], ff_clictrig_inum_en[i][j], cfg_clk, rst_n) 
end 
assign clic_trig_fire[i] = pcu_clic_rsp_vld & ff_clictrig_en_q[i] & (ff_clictrig_inum_q[i] == 13'(pcu_clic_rsp_id)); 
end 
`WCBB_AOMUX(ff_clictrig_inum_q, bmu_sel_clictrig_per_ent, bmu_selected_clictrig_inum) 
`WCBB_AOMUX(ff_clictrig_en_q , bmu_sel_clictrig_per_ent, bmu_selected_clictrig_en ) 
assign trig_sel_rdata = {bmu_selected_clictrig_en, 18'h0, bmu_selected_clictrig_inum}; 
assign clic_tm_intctl_vld = |clic_trig_fire; 
end else begin : UNSUPPORT_TRIG 
assign bmu_sel_clictrig = 'b0; 
assign trig_sel_rdata = 'b0; 
assign clic_tm_intctl_vld = 'b0; 
end 
for (genvar i=0; i<NUM_INTERRUPT; i++) begin : g_clr_ip_vec 
assign pcu_clic_rsp_clr_vec[i] = (pcu_clic_rsp_id == i); 
end 
assign unit_ip_hw_clear = {NUM_INTERRUPT{pcu_clic_rsp_clr_ip & pcu_clic_rsp_vld}} 
& pcu_clic_rsp_clr_vec; 
for (genvar i=0; i<NUM_INTERRUPT; i++) begin: G_WR_UNIT 
assign clic2unit_wvld_clicintip [i] = bmu_sel_unit[i] & bmu_clic_wr_vld & bmu_clic_req_wstrb[0]; 
assign clic2unit_wvld_clicintie [i] = bmu_sel_unit[i] & bmu_clic_wr_vld & bmu_clic_req_wstrb[1]; 
assign clic2unit_wvld_clicintattr[i] = bmu_sel_unit[i] & bmu_clic_wr_vld & bmu_clic_req_wstrb[2]; 
assign clic2unit_wvld_clicintctl [i] = bmu_sel_unit[i] & bmu_clic_wr_vld & bmu_clic_req_wstrb[3]; 
end 
assign clic2unit_wdata_ip = bmu_clic_req_wdata[UNIT_OFFS_IP]; 
assign clic2unit_wdata_ie = bmu_clic_req_wdata[UNIT_OFFS_IE]; 
assign clic2unit_wdata_shv = bmu_clic_req_wdata[UNIT_OFFS_SHV]; 
assign clic2unit_wdata_trig = bmu_clic_req_wdata[UNIT_OFFS_TRIG +: 2]; 
if (4 != 0) begin : CTL_REG_PROT 
assign clic2unit_wdata_ctl = {bmu_clic_req_wdata[31 -: 4], {8-4{1'b1}}}; 
end else begin : TIE_CTL_REG 
assign clic2unit_wdata_ctl = 8'd255; 
end 
`WCBB_AOMUX (unit2clic_reg_ip , bmu_sel_unit, bmu_selected_ip ) 
`WCBB_AOMUX (unit2clic_reg_ie , bmu_sel_unit, bmu_selected_ie ) 
`WCBB_AOMUX (unit2clic_reg_shv , bmu_sel_unit, bmu_selected_shv ) 
`WCBB_AOMUX (unit2clic_reg_trig, bmu_sel_unit, bmu_selected_trig) 
`WCBB_AOMUX (unit2clic_reg_mode, bmu_sel_unit, bmu_selected_mode) 
if (4 != 0) begin: G_SEL_CTL 
`WCBB_AOMUX (unit2clic_reg_ctl, bmu_sel_unit, bmu_selected_ctl) 
end else begin: G_TIE_CTL 
assign bmu_selected_ctl = 8'd255; 
end 
assign unit_sel_rdata = { bmu_selected_ctl, 
{ bmu_selected_mode, 3'h0, bmu_selected_trig, bmu_selected_shv}, 
{ 7'h0, bmu_selected_ie}, 
{ 7'h0, bmu_selected_ip} }; 
for (genvar i=0; i<NUM_INTERRUPT; i++) begin: G_RSVD_INT_SRC 
if (i == 3) begin : id_is_3 
assign unit_int_src[i] = int_src_msip; 
end 
else if (i == 7) begin : id_is_7 
assign unit_int_src[i] = int_src_mtip; 
end 
else if (i == 11) begin : id_is_11 
assign unit_int_src[i] = postsync_ext_int; 
end 
else if (i == 16) begin : id_is_16 
assign unit_int_src[i] = clic_csip; 
end 
else if (i == 31) begin : id_is_31 
assign unit_int_src[i] = ras_int; 
end 
else if (i >= 32) begin : extra_int 
assign unit_int_src[i] = postsync_loc_int[i-32]; 
end 
else begin : not_support_int 
assign unit_int_src[i] = '0; 
end 
end 
for (genvar i=0; i<NUM_INTERRUPT; i++) begin: G_CLIC_UNIT 
if (i < 32) begin : CLINT_INT 
if (CLINT_INT_VLD[i] == 1'b0) begin : G_DUMMY 
assign unit2clic_reg_ip [i] = '0; 
assign unit2clic_reg_ie [i] = '0; 
assign unit2clic_reg_shv [i] = '0; 
assign unit2clic_reg_trig[i] = '0; 
assign unit2clic_reg_mode[i] = '0; 
assign unit2clic_reg_ctl [i] = '0; 
assign unit_int_en_pd [i] = '0; 
end else begin : G_UNIT 
wing_clic_unit #( 
.static_flag ( 3'b000 ), 
.ctl ( {4{1'b0}} ), 
.trig ( 2'b0 ), 
.shv ( 1'b0 ) 
) u_clic_unit ( 
.clk ( clk ), 
.cfg_clk ( cfg_clk ), 
.rst_n ( rst_n ), 
.unit_int_src ( unit_int_src[i] ), 
.unit_ip_hw_clear ( unit_ip_hw_clear[i] ), 
.clic2unit_wvld_clicintip ( clic2unit_wvld_clicintip [i] ), 
.clic2unit_wvld_clicintie ( clic2unit_wvld_clicintie [i] ), 
.clic2unit_wvld_clicintattr ( clic2unit_wvld_clicintattr[i] ), 
.clic2unit_wvld_clicintctl ( clic2unit_wvld_clicintctl [i] ), 
.clic2unit_wdata_ip ( clic2unit_wdata_ip ), 
.clic2unit_wdata_ie ( clic2unit_wdata_ie ), 
.clic2unit_wdata_shv ( clic2unit_wdata_shv ), 
.clic2unit_wdata_trig ( clic2unit_wdata_trig ), 
.clic2unit_wdata_ctl ( clic2unit_wdata_ctl ), 
.unit2clic_reg_ip ( unit2clic_reg_ip [i] ), 
.unit2clic_reg_ie ( unit2clic_reg_ie [i] ), 
.unit2clic_reg_shv ( unit2clic_reg_shv [i] ), 
.unit2clic_reg_trig ( unit2clic_reg_trig[i] ), 
.unit2clic_reg_mode ( unit2clic_reg_mode[i] ), 
.unit2clic_reg_ctl ( unit2clic_reg_ctl [i] ), 
.unit_int_en_pd ( unit_int_en_pd [i] ) 
); 
end 
end else begin : CLIC_INT 
wing_clic_unit #( 
.static_flag ( CLIC_UNIT_MMR_CFG[i-32].static_flag ), 
.ctl ( CLIC_UNIT_MMR_CFG[i-32].ctl ), 
.trig ( CLIC_UNIT_MMR_CFG[i-32].trig ), 
.shv ( CLIC_UNIT_MMR_CFG[i-32].shv ) 
) u_clic_unit ( 
.clk ( clk ), 
.cfg_clk ( cfg_clk ), 
.rst_n ( rst_n ), 
.unit_int_src ( unit_int_src[i] ), 
.unit_ip_hw_clear ( unit_ip_hw_clear[i] ), 
.clic2unit_wvld_clicintip ( clic2unit_wvld_clicintip [i] ), 
.clic2unit_wvld_clicintie ( clic2unit_wvld_clicintie [i] ), 
.clic2unit_wvld_clicintattr ( clic2unit_wvld_clicintattr[i] ), 
.clic2unit_wvld_clicintctl ( clic2unit_wvld_clicintctl [i] ), 
.clic2unit_wdata_ip ( clic2unit_wdata_ip ), 
.clic2unit_wdata_ie ( clic2unit_wdata_ie ), 
.clic2unit_wdata_shv ( clic2unit_wdata_shv ), 
.clic2unit_wdata_trig ( clic2unit_wdata_trig ), 
.clic2unit_wdata_ctl ( clic2unit_wdata_ctl ), 
.unit2clic_reg_ip ( unit2clic_reg_ip [i] ), 
.unit2clic_reg_ie ( unit2clic_reg_ie [i] ), 
.unit2clic_reg_shv ( unit2clic_reg_shv [i] ), 
.unit2clic_reg_trig ( unit2clic_reg_trig[i] ), 
.unit2clic_reg_mode ( unit2clic_reg_mode[i] ), 
.unit2clic_reg_ctl ( unit2clic_reg_ctl [i] ), 
.unit_int_en_pd ( unit_int_en_pd [i] ) 
); 
end 
end 
if (4 == 0) begin: G_ARB_NOBITS 
localparam NUM_INT_ROUND_N = 2 ** $clog2(64 + 32); 
logic [$clog2(64 + 32)-1:0] en_pd_lead_zero_cnt; 
wing_cbb_lzc #( 
.WIDTH ( NUM_INT_ROUND_N ), 
.DIRECTION ( 1 ) 
) u_cnt_pd_oh_zero ( 
.vec_i ( {{NUM_INT_ROUND_N-NUM_INTERRUPT{1'b0}}, unit_int_en_pd}), 
.cnt_o ( en_pd_lead_zero_cnt ), 
.empty_o ( vld_en_pd_int[0] ) 
); 
assign arb_id = ~en_pd_lead_zero_cnt; 
assign arb_lvl = vld_en_pd_int[0] ? 8'd0 : 8'd255; 
always_comb begin : g_arb_shv 
arb_shv = 1'b0; 
for (int i=0; i<NUM_INTERRUPT; i++) begin 
arb_shv = arb_shv | ((i == arb_id) & unit2clic_reg_shv[i]); 
end 
end 
end else begin : G_ARB_NORMAL 
localparam NUM_INT_ROUND_N = 2 ** $clog2(64 + 32); 
localparam CTL_NODE_W = 4+1; 
logic [4-1:0] ctl_level ; 
logic [4-1:0] nlbits_mask_ctl ; 
logic [NUM_INTERRUPT -1:0][$clog2(64 + 32)-1:0] int_id_node ; 
logic [4-1:0] arb_ctl_reg ; 
logic [NUM_INT_ROUND_N-1:0][CTL_NODE_W-1:0] sel_max_ctl_node ; 
logic [NUM_INT_ROUND_N-1:0][CTL_NODE_W-1:0] sel_max_ctl_node_d; 
logic [NUM_INT_ROUND_N-1:0][$clog2(64 + 32)-1:0] sel_int_id_node ; 
logic [NUM_INT_ROUND_N-1:0][$clog2(64 + 32)-1:0] sel_int_id_node_d ; 
logic [NUM_INTERRUPT -1:0][CTL_NODE_W-1:0] ctl_reg_enpd ; 
for (genvar j = 0; j < NUM_INTERRUPT; j++) begin : g_int_id_node 
assign int_id_node [j] = j; 
assign ctl_reg_enpd[j] = {unit2clic_reg_ctl[j][7-:4] & {4{unit_int_en_pd[j]}}, unit_int_en_pd[j]} ; 
end 
assign arb_ctl_reg = sel_max_ctl_node[0][4:1]; 
for (genvar level = 0; level < $clog2(64 + 32); level++) begin : g_levels 
if (level == $clog2(64 + 32) - 1) begin : ground_floor 
for (genvar k = 0; k < 2 ** level; k++) begin : ground_floor_level_num 
if (k * 2 < NUM_INTERRUPT - 1) begin : ground_floor_arb 
assign sel_max_ctl_node[2 ** level - 1 + k] = (ctl_reg_enpd[k * 2 + 1] < ctl_reg_enpd[k * 2 ])? 
ctl_reg_enpd[k * 2 ] : ctl_reg_enpd[k * 2 + 1]; 
assign sel_int_id_node [2 ** level - 1 + k] = (ctl_reg_enpd[k * 2 + 1] < ctl_reg_enpd[k * 2 ])? 
int_id_node [k * 2 ] : int_id_node [k * 2 + 1]; 
end else if (k * 2 == NUM_INTERRUPT - 1) begin : ground_floor_last 
assign sel_max_ctl_node[2 ** level - 1 + k] = ctl_reg_enpd[k * 2]; 
assign sel_int_id_node [2 ** level - 1 + k] = int_id_node [k * 2]; 
end else begin : ground_floor_compensate 
assign sel_max_ctl_node[2 ** level - 1 + k] = ctl_reg_enpd[NUM_INTERRUPT-1]; 
assign sel_int_id_node [2 ** level - 1 + k] = int_id_node [NUM_INTERRUPT-1]; 
end 
end 
assign vld_en_pd_int[level] = |unit_int_en_pd; 
end else begin : g_upper_level 
for (genvar l = 0; l < 2 ** level; l++) begin : upper_level 
assign sel_max_ctl_node_d[2 ** level - 1 + l] = (sel_max_ctl_node[2 ** (level + 1)-1 + l * 2 + 1] < sel_max_ctl_node[2 ** (level + 1)-1 + l * 2 ]) ? 
sel_max_ctl_node[2 ** (level + 1)-1 + l * 2 ] : sel_max_ctl_node[2 ** (level + 1)-1 + l * 2 + 1]; 
assign sel_int_id_node_d [2 ** level - 1 + l] = (sel_max_ctl_node[2 ** (level + 1)-1 + l * 2 + 1] < sel_max_ctl_node[2 ** (level + 1)-1 + l * 2 ]) ? 
sel_int_id_node [2 ** (level + 1)-1 + l * 2 ] : sel_int_id_node [2 ** (level + 1)-1 + l * 2 + 1]; 
if(CLIC_ARB_PPLN_VLD[level] == 1'b1)begin : LEVEL_PPLN 
`WDFFNR(sel_max_ctl_node[2 ** level - 1 + l], sel_max_ctl_node_d[2 ** level - 1 + l], clk) 
`WDFFNR(sel_int_id_node [2 ** level - 1 + l], sel_int_id_node_d [2 ** level - 1 + l], clk) 
end else begin : NOT_PPLN 
assign sel_max_ctl_node[2 ** level - 1 + l] = sel_max_ctl_node_d[2 ** level - 1 + l]; 
assign sel_int_id_node [2 ** level - 1 + l] = sel_int_id_node_d [2 ** level - 1 + l]; 
end 
end 
if(CLIC_ARB_PPLN_VLD[level] == 1'b1)begin : VLD_PPLN 
`WDFFNR(vld_en_pd_int[level], vld_en_pd_int[level + 1], clk) 
end else begin : NO_PPLN 
assign vld_en_pd_int[level] = vld_en_pd_int[level + 1]; 
end 
end 
end 
for (genvar i=0; i<4; i++) begin: G_CTL_MASK 
assign nlbits_mask_ctl[i] = 4'(4-1-i) < ff_mcliccfg_nlbits_q; 
end 
assign ctl_level = nlbits_mask_ctl & arb_ctl_reg 
| ~nlbits_mask_ctl & {4{vld_en_pd_int[0]}}; 
assign arb_id = sel_int_id_node[0]; 
assign arb_lvl = {ctl_level, {8-4{vld_en_pd_int[0]}}}; 
always_comb begin : g_arb_shv 
arb_shv = 1'b0; 
for (int i=0; i<NUM_INTERRUPT; i++) begin 
arb_shv = arb_shv | ((i == arb_id) & unit2clic_reg_shv[i]); 
end 
end 
end 
assign clic_pcu_int_priv = 2'b11; 
if (CLIC_EN_INT_PPLN == 1) begin: G_EN_O_PPLN 
logic ff_o_data_en ; 
logic ff_o_il_en ; 
logic ff_o_int_vld_q; 
logic ff_o_int_vld_d; 
logic [7:0] ff_o_il_d ; 
logic [7:0] ff_o_il_q ; 
logic [$clog2(64 + 32)-1:0] ff_o_id_d ; 
logic [$clog2(64 + 32)-1:0] ff_o_id_q ; 
logic ff_o_shv_d; 
logic ff_o_shv_q; 
assign ff_o_data_en = |arb_lvl; 
assign ff_o_id_d = arb_id ; 
assign ff_o_shv_d = arb_shv ; 
assign ff_o_il_en = |arb_lvl | ff_o_int_vld_q ; 
assign ff_o_il_d = arb_lvl ; 
assign ff_o_int_vld_d = |arb_lvl; 
assign clic_pcu_int_vld = ff_o_int_vld_q; 
assign clic_pcu_int_lvl = ff_o_il_q; 
assign clic_pcu_int_id = ff_o_id_q; 
assign clic_pcu_int_shv = ff_o_shv_q; 
`WDFFER (ff_o_int_vld_q, ff_o_int_vld_d, ff_o_il_en, clk, rst_n) 
`WDFFER (ff_o_il_q, ff_o_il_d, ff_o_il_en, clk, rst_n) 
`WDFFENR(ff_o_id_q, ff_o_id_d, ff_o_data_en, clk) 
`WDFFENR(ff_o_shv_q, ff_o_shv_d, ff_o_data_en, clk) 
end else begin: G_NO_O_PPLN 
assign clic_pcu_int_lvl = arb_lvl; 
assign clic_pcu_int_id = arb_id ; 
assign clic_pcu_int_shv = arb_shv; 
assign clic_pcu_int_vld = |clic_pcu_int_lvl; 
end 
endmodule
 
 
module wing_clic_unit #( 
parameter static_flag = 3'b0, 
parameter ctl = {4{1'b0}}, 
parameter trig = 2'b0, 
parameter shv = 1'b0 
)( 
input logic clk, 
input logic cfg_clk, 
input logic rst_n, 
input logic unit_int_src, 
input logic unit_ip_hw_clear, 
input logic clic2unit_wvld_clicintip , 
input logic clic2unit_wvld_clicintie , 
input logic clic2unit_wvld_clicintattr, 
input logic clic2unit_wvld_clicintctl , 
input logic clic2unit_wdata_ip , 
input logic clic2unit_wdata_ie , 
input logic clic2unit_wdata_shv , 
input logic [1:0] clic2unit_wdata_trig , 
input logic [7:0] clic2unit_wdata_ctl , 
output logic unit2clic_reg_ip , 
output logic unit2clic_reg_ie , 
output logic unit2clic_reg_shv , 
output logic [1:0] unit2clic_reg_trig , 
output logic [1:0] unit2clic_reg_mode , 
output logic [7:0] unit2clic_reg_ctl , 
output logic unit_int_en_pd 
); 
logic trig_edge_pos; 
logic trig_edge_neg; 
logic trig_edge_fire; 
logic ff_ip_lvl_d; 
logic ff_int_src_q; 
logic ff_ip_edge_set; 
logic ff_ip_edge_clr; 
logic ff_ip_en; 
logic ff_ip_d; 
logic ff_ip_q; 
logic ff_ie_en; 
logic ff_ie_d; 
logic ff_ie_q; 
logic ff_attr_trig_en; 
logic [1:0] ff_attr_trig_d; 
logic [1:0] ff_attr_trig_q; 
logic ff_attr_shv_en; 
logic ff_attr_shv_d; 
logic ff_attr_shv_q; 
`WDFFR(ff_int_src_q, unit_int_src, clk, rst_n) 
assign trig_edge_pos = ~ff_int_src_q & unit_int_src; 
assign trig_edge_neg = ff_int_src_q & ~unit_int_src; 
assign trig_edge_fire = ff_attr_trig_q[1] ? trig_edge_neg : trig_edge_pos; 
assign ff_ip_edge_set = trig_edge_fire 
| clic2unit_wvld_clicintip & clic2unit_wdata_ip ; 
assign ff_ip_edge_clr = unit_ip_hw_clear 
| clic2unit_wvld_clicintip & ~clic2unit_wdata_ip; 
assign ff_ip_lvl_d = unit_int_src & ~ff_attr_trig_q[1] 
| ~unit_int_src & ff_attr_trig_q[1] ; 
assign ff_ip_en = ff_attr_trig_q[0] ? (ff_ip_edge_set | ff_ip_edge_clr) 
: 1'b1; 
assign ff_ip_d = ff_attr_trig_q[0] ? ff_ip_edge_set 
: ff_ip_lvl_d; 
`WDFFER(ff_ip_q, ff_ip_d, ff_ip_en, clk, rst_n) 
assign ff_ie_en = clic2unit_wvld_clicintie; 
assign ff_ie_d = clic2unit_wdata_ie; 
`WDFFER(ff_ie_q, ff_ie_d, ff_ie_en, cfg_clk, rst_n) 
if(static_flag[0] == 0) begin :shv_cfg 
assign ff_attr_shv_en = clic2unit_wvld_clicintattr; 
assign ff_attr_shv_d = clic2unit_wdata_shv; 
`WDFFER(ff_attr_shv_q, ff_attr_shv_d, ff_attr_shv_en, cfg_clk, rst_n) 
end else begin : shv_tie 
assign ff_attr_shv_q = shv; 
end 
if( static_flag[1] == 0) begin :trig_cfg 
assign ff_attr_trig_en = clic2unit_wvld_clicintattr; 
assign ff_attr_trig_d = clic2unit_wdata_trig; 
`WDFFER(ff_attr_trig_q, ff_attr_trig_d, ff_attr_trig_en, cfg_clk, rst_n) 
end else begin : trig_tie 
assign ff_attr_trig_q = trig; 
end 
generate 
if (4 != 0) begin : G_FF_CTL 
if(static_flag[2] == 0 ) begin : ctl_cfg 
logic ff_ctl_en; 
logic [4-1:0] ff_ctl_d; 
logic [4-1:0] ff_ctl_q; 
assign ff_ctl_en = clic2unit_wvld_clicintctl; 
assign ff_ctl_d = clic2unit_wdata_ctl[7 -: 4]; 
`WDFFER(ff_ctl_q, ff_ctl_d, ff_ctl_en, cfg_clk, rst_n) 
assign unit2clic_reg_ctl = {ff_ctl_q, {8-4{1'b1}}}; 
end else begin : ctl_tie 
assign unit2clic_reg_ctl = {ctl, {8-4{1'b1}}}; 
end 
end else begin: G_EMPTY_CTL 
assign unit2clic_reg_ctl = 8'd255; 
end 
endgenerate 
assign unit_int_en_pd = ff_ip_q & ff_ie_q; 
assign unit2clic_reg_ip = ff_ip_q; 
assign unit2clic_reg_ie = ff_ie_q; 
assign unit2clic_reg_shv = ff_attr_shv_q; 
assign unit2clic_reg_trig = ff_attr_trig_q; 
assign unit2clic_reg_mode = 2'b11; 
endmodule
 
 
module m130_ds_top ( 
input logic clk, 
input logic pwrup_rst_n, 
input logic dm_sft_rst_n, 
output logic dm_top_ndmreset, 
output logic dm_top_core_reset, 
output logic dm_top_dmactive, 
input logic top_dm_auth_bit_unfused, 
input logic trst_n, 
input logic tck, 
input logic tms, 
input logic tdi, 
output logic tdo, 
output logic tdo_en, 
input logic dft_icg_scan_en, 
input logic dft_scan_mode, 
input logic dft_scan_rst_n, 
output logic dm_pcu_halt_req, 
output logic dm_pcu_halt_on_reset, 
output logic dm_pcu_resume_req, 
input logic pcu_dm_halted, 
input logic pcu_dm_havereset, 
output logic dm_pcu_ack_havereset, 
input logic pcu_dm_unavail, 
output logic dm_ifu_inst_vld, 
input logic ifu_dm_inst_rdy, 
output logic[32-1:0] dm_ifu_inst_data, 
input logic pcu_dm_cmd_done, 
input logic pcu_dm_cmd_excp, 
input logic pcu_dm_bus_err, 
output logic dm_pcu_dsch0_write, 
input logic[32-1:0] pcu_dm_dsch0_rdata, 
output logic[32-1:0] dm_pcu_dsch0_wdata 
); 
logic dmi_dm_psel; 
logic dmi_dm_penable; 
logic dmi_dm_pwrite; 
logic[32-1:0] dmi_dm_paddr; 
logic[32-1:0] dmi_dm_pwdata; 
logic[32-1:0] dm_dmi_prdata; 
logic dm_dmi_pready; 
logic dm_dmi_pslverr; 
m130_dtm_top i_dtm_top ( 
.clk (clk), 
.pwrup_rst_n (pwrup_rst_n), 
.trst_n (trst_n), 
.tck (tck), 
.tms (tms), 
.tdi (tdi), 
.tdo (tdo), 
.tdo_en (tdo_en), 
.dft_icg_scan_en (dft_icg_scan_en), 
.dft_scan_mode (dft_scan_mode), 
.dft_scan_rst_n (dft_scan_rst_n), 
.dmi_dm_psel (dmi_dm_psel), 
.dmi_dm_penable (dmi_dm_penable), 
.dmi_dm_pwrite (dmi_dm_pwrite), 
.dmi_dm_paddr (dmi_dm_paddr), 
.dmi_dm_pwdata (dmi_dm_pwdata), 
.dm_dmi_prdata (dm_dmi_prdata), 
.dm_dmi_pready (dm_dmi_pready), 
.dm_dmi_pslverr (dm_dmi_pslverr) 
); 
m130_dm i_dm ( 
.dm_pcu_halt_req (dm_pcu_halt_req), 
.dm_pcu_halt_on_reset (dm_pcu_halt_on_reset), 
.dm_pcu_resume_req (dm_pcu_resume_req), 
.pcu_dm_halted (pcu_dm_halted), 
.pcu_dm_havereset (pcu_dm_havereset), 
.dm_pcu_ack_havereset (dm_pcu_ack_havereset), 
.pcu_dm_unavail (pcu_dm_unavail), 
.dm_ifu_inst_vld (dm_ifu_inst_vld), 
.ifu_dm_inst_rdy (ifu_dm_inst_rdy), 
.dm_ifu_inst_data (dm_ifu_inst_data), 
.pcu_dm_cmd_done (pcu_dm_cmd_done), 
.pcu_dm_cmd_excp (pcu_dm_cmd_excp), 
.pcu_dm_bus_err (pcu_dm_bus_err), 
.dm_pcu_dsch0_write (dm_pcu_dsch0_write), 
.pcu_dm_dsch0_rdata (pcu_dm_dsch0_rdata), 
.dm_pcu_dsch0_wdata (dm_pcu_dsch0_wdata), 
.dmi_dm_psel (dmi_dm_psel), 
.dmi_dm_penable (dmi_dm_penable), 
.dmi_dm_pwrite (dmi_dm_pwrite), 
.dmi_dm_paddr (dmi_dm_paddr), 
.dmi_dm_pwdata (dmi_dm_pwdata), 
.dm_dmi_prdata (dm_dmi_prdata), 
.dm_dmi_pready (dm_dmi_pready), 
.dm_dmi_pslverr (dm_dmi_pslverr), 
.dm_top_ndmreset (dm_top_ndmreset), 
.dm_top_core_reset (dm_top_core_reset), 
.dm_top_dmactive (dm_top_dmactive), 
.top_dm_auth_bit_unfused (top_dm_auth_bit_unfused), 
.clk (clk), 
.pwrup_rst_n (pwrup_rst_n), 
.dm_sft_rst_n (dm_sft_rst_n) 
); 
endmodule
 
 
module m130_dm( 
output logic dm_pcu_halt_req, 
output logic dm_pcu_halt_on_reset, 
output logic dm_pcu_resume_req, 
input logic pcu_dm_halted, 
input logic pcu_dm_havereset, 
output logic dm_pcu_ack_havereset, 
input logic pcu_dm_unavail, 
output logic dm_ifu_inst_vld, 
input logic ifu_dm_inst_rdy, 
output logic[32-1:0] dm_ifu_inst_data, 
input logic pcu_dm_cmd_done, 
input logic pcu_dm_cmd_excp, 
input logic pcu_dm_bus_err, 
output logic dm_pcu_dsch0_write, 
input logic[32-1:0] pcu_dm_dsch0_rdata, 
output logic[32-1:0] dm_pcu_dsch0_wdata, 
input logic dmi_dm_psel, 
input logic dmi_dm_penable, 
input logic dmi_dm_pwrite, 
input logic[32-1:0] dmi_dm_paddr, 
input logic[32-1:0] dmi_dm_pwdata, 
output logic[32-1:0] dm_dmi_prdata, 
output logic dm_dmi_pready, 
output logic dm_dmi_pslverr, 
output logic dm_top_ndmreset, 
output logic dm_top_core_reset, 
output logic dm_top_dmactive, 
input logic top_dm_auth_bit_unfused, 
input logic clk, 
input logic pwrup_rst_n, 
input logic dm_sft_rst_n 
); 
localparam DM_EBREAK = 32'h00100073; 
localparam DM_C_EBREAK = 16'h9002; 
localparam logic[3-1:0] ERR_NONE = 'd0; 
localparam logic[3-1:0] ERR_BUSY = 'd1; 
localparam logic[3-1:0] ERR_NOT_SUPPORT = 'd2; 
localparam logic[3-1:0] ERR_EXCEPTION = 'd3; 
localparam logic[3-1:0] ERR_STAT_WRONG = 'd4; 
localparam logic[3-1:0] ERR_BUS_ERROR = 'd5; 
localparam logic[3-1:0] ERR_OTHER = 'd7; 
localparam RW_READ = 1'b0; 
localparam RW_WRITE = 1'b1; 
localparam logic[$clog2(8)-1:0] DCMD_AR_CSR = 'd0; 
localparam logic[$clog2(8)-1:0] DCMD_AR_GPR = 'd1; 
localparam logic[$clog2(8)-1:0] DCMD_AM_LD = 'd2; 
localparam logic[$clog2(8)-1:0] DCMD_AM_ST = 'd3; 
localparam logic[$clog2(8)-1:0] DCMD_GPR_PUSH = 'd4; 
localparam logic[$clog2(8)-1:0] DCMD_GPR_POP = 'd5; 
localparam REGNO_DSCRATCH0 = 12'h7b2; 
localparam REGNO_DSCRATCH2 = 12'h7c0; 
localparam REGNO_DSCRATCH3 = 12'h7c1; 
localparam FUNCT3_CSRRS = 3'b010; 
localparam FUNCT3_CSRRW = 3'b001; 
localparam OPCODE_SYSTEM = 7'b1110011; 
localparam OPCODE_LOAD = 7'b0000011; 
localparam OPCODE_STORE = 7'b0100011; 
localparam GPR_X0 = 5'd0; 
localparam GPR_X5 = 5'd5; 
localparam GPR_X6 = 5'd6; 
localparam DM_NOP = 32'h7b202073; 
typedef enum logic { 
R_IDLE, 
R_RESUME_REQ 
} runctr_state_e; 
typedef enum logic[2:0] { 
AR_IDLE, 
AR_GPR_STASH, 
AR_REG_RW, 
AR_GPR_POP, 
AR_ADD_REGNO, 
AR_PROG_EXEC 
} ar_state_e; 
typedef enum logic[1:0] { 
QA_IDLE, 
QA_HALT_REQ, 
QA_PROG_EXEC, 
QA_RESUME_REQ 
} qa_state_e; 
typedef enum logic[2:0] { 
AM_IDLE, 
AM_GPR_STASH, 
AM_MEM_RW, 
AM_GPR_POP, 
AM_POST_INCR 
} am_state_e; 
typedef enum logic[1:0] { 
PR_IDLE, 
PR_INST_REQ, 
PR_INST_EXEC 
} prog_stat_e; 
typedef struct packed { 
logic ndmresetpending; 
logic stickyunavail; 
logic impebreak; 
logic allhavereset; 
logic anyhavereset; 
logic allresumeack; 
logic anyresumeack; 
logic allnonexistent; 
logic anynonexistent; 
logic allunavail; 
logic anyunavail; 
logic allrunning; 
logic anyrunning; 
logic allhalted; 
logic anyhalted; 
logic authenticated; 
logic authbusy; 
logic hasresethaltreq; 
logic confstrptrvalid; 
logic[3:0] version; 
} dmr_dmstatus_t; 
typedef struct packed { 
logic haltreq; 
logic resumereq; 
logic hartreset; 
logic ackhavereset; 
logic ackunavail; 
logic hasel; 
logic[9:0] hartsello; 
logic[9:0] hartselhi; 
logic setkeepalive; 
logic clrkeepalive; 
logic setresethaltreq; 
logic clrresethaltreq; 
logic ndmreset; 
logic dmactive; 
} dmr_dmcontrol_t; 
typedef struct packed { 
logic haltreq; 
logic hartreset; 
logic ndmreset; 
} dmr_dmcontrol_reg_t; 
typedef struct packed { 
logic[4:0] progbufsize; 
logic busy; 
logic relaxedpriv; 
logic[2:0] cmderr; 
logic[3:0] datacount; 
} dmr_abstractcs_t; 
typedef struct packed { 
logic[2:0] cmderr; 
} dmr_abstractcs_reg_t; 
typedef struct packed { 
logic[7:0] cmdtype; 
logic[23:0] control; 
} dmr_command_t; 
typedef struct packed { 
logic[2:0] aarsize; 
logic aarpostincrement; 
logic postexec; 
logic transfer; 
logic write; 
logic[15:0] regno; 
} dmr_cmd_ar_t; 
typedef struct packed { 
logic aamvirtual; 
logic[2:0] aamsize; 
logic aampostincrement; 
logic write; 
logic[1:0] target_specific; 
} dmr_cmd_am_t; 
typedef struct packed { 
logic[15:0] autoexecprogbuf; 
logic[11:0] autoexecdata; 
} dmr_abstractauto_t; 
typedef struct packed { 
logic[2-1:0] autoexecprogbuf; 
logic[2-1:0] autoexecdata; 
} dmr_abstractauto_reg_t; 
typedef enum logic[32-2-1:0] { 
DMR_DMSTATUS_ADDR = 30'h11, 
DMR_DMCONTROL_ADDR = 30'h10, 
DMR_ABSTRACTCS_ADDR = 30'h16, 
DMR_COMMAND_ADDR = 30'h17, 
DMR_ABSTRACTAUTO_ADDR = 30'h18, 
DMR_NEXTDM_ADDR = 30'h1d, 
DMR_AUTHDATA_ADDR = 30'h30, 
DMR_DATA0_ADDR = 30'h4 , 
DMR_DATA1_ADDR = 30'h5 , 
DMR_PROGBUF0_ADDR = 30'h20, 
DMR_PROGBUF1_ADDR = 30'h21, 
DMR_PROGBUF2_ADDR = 30'h22, 
DMR_PROGBUF3_ADDR = 30'h23, 
DMR_PROGBUF4_ADDR = 30'h24, 
DMR_PROGBUF5_ADDR = 30'h25, 
DMR_PROGBUF6_ADDR = 30'h26, 
DMR_PROGBUF7_ADDR = 30'h27, 
DMR_PROGBUF8_ADDR = 30'h28, 
DMR_PROGBUF9_ADDR = 30'h29, 
DMR_PROGBUF10_ADDR = 30'h2a, 
DMR_PROGBUF11_ADDR = 30'h2b, 
DMR_PROGBUF12_ADDR = 30'h2c, 
DMR_PROGBUF13_ADDR = 30'h2d, 
DMR_PROGBUF14_ADDR = 30'h2e, 
DMR_PROGBUF15_ADDR = 30'h2f 
} dmr_addr_e; 
logic rst_n; 
assign rst_n = dm_sft_rst_n; 
logic resume_ack_q, resume_ack_d, resume_ack_upd; 
logic halt_on_reset_q, halt_on_reset_d, halt_on_reset_upd; 
logic abs_inst_send; 
logic dscratch0_trans; 
logic dscratch0_rw; 
logic dmi_vld; 
logic dmi_rw; 
logic dmi_write; 
logic dmi_read; 
logic non_active; 
logic dm_authed; 
logic ndmreset_ongoing; 
logic dm_normal; 
logic abs_busy; 
logic dmi_wr_haltreq_1; 
logic dmi_wr_haltreq_0; 
logic dmi_wr_resumereq_1; 
logic dmi_wr_clrhtrst; 
logic dmi_wr_sethtrst; 
logic cmd_touch_en; 
logic cmd_written; 
logic absauto_start; 
logic cmd_will_start; 
logic cmd_start; 
logic touch_command; 
logic touch_abstractcs; 
logic touch_abstractauto; 
logic touch_pbdt; 
logic abscmd_running; 
logic illegal_touch; 
logic abscmd_end; 
logic busy_err; 
logic abs_fail; 
logic abs_inst_running; 
logic core_cmd_done_noexcp; 
logic core_cmd_done_with_excp; 
logic ar_push; 
logic ar_pop; 
logic am_push; 
logic am_pop; 
logic dcmd_vld; 
logic ar_cmd_rw; 
logic abs_inst_sending; 
logic cmd_is_ar; 
logic ar_rw_s1; 
logic ar_rw_s2; 
logic ar_rw_s3; 
logic ar_csr; 
logic ar_gpr; 
logic ar_upc_fail; 
logic ar_fail1; 
logic ar_fail2; 
logic ar_fail3; 
logic ar_idle; 
logic ar_reg_rw; 
logic cmd_is_am; 
logic am_rw_s1; 
logic am_rw_s2; 
logic am_rw_s3; 
logic am_rw_s4; 
logic am_rw_s5; 
logic am_upc_fail; 
logic am_fail1; 
logic am_fail2; 
logic fst_push_succ; 
logic fst_pop_succ; 
logic am_idle; 
logic am_mem_rw; 
logic am_store; 
logic am_load; 
logic core_halted; 
logic core_running; 
logic core_unavail; 
logic core_havereset; 
logic core_inst_done; 
logic core_inst_excp; 
logic core_inst_excp_bus; 
logic core_inst_done_with_excp; 
logic sel_dmactive; 
logic abs_wrong_type; 
logic dm_stat_active_q, dm_stat_active_d, dm_stat_active_upd; 
logic dm_is_active; 
logic dm_ifu_inst_data_upd; 
logic [31:0] dmi_rdata; 
logic [32-1:0] inst_to_send; 
logic [$clog2(2)-1:0] trans_data_n; 
logic [2:0] am_incr_addr; 
logic [32-2-1:0] dmi_addr; 
logic [31:0] dmi_wdata; 
logic [3-1:0] abs_fail_cause; 
logic [$clog2(8)-1:0] dcmd_op; 
logic [3-1:0] ar_fail1_cause; 
logic [3-1:0] ar_fail2_cause; 
logic [3-1:0] ar_fail3_cause; 
logic [15:0] am_0_bits; 
logic [3-1:0] am_fail1_cause; 
logic [3-1:0] am_fail2_cause; 
logic [31:0] data_rdata; 
logic [31:0] pbdt_rdata; 
logic [2-1:0] data_sel_vec; 
logic [2-1:0] auto_data_vec; 
logic [2-1:0] touch_data_vec; 
logic [11:0] csr_no; 
logic [4:0] gpr_no; 
logic cmd_start_q, cmd_start_d, cmd_start_upd; 
runctr_state_e runctr_stat_q, runctr_stat_d; 
logic runctr_stat_upd; 
ar_state_e ar_stat_q, ar_stat_d; 
logic ar_stat_upd; 
am_state_e am_stat_q, am_stat_d; 
logic am_stat_upd; 
logic log_excp_q, log_excp_d, log_excp_upd; 
logic log_excp_bus_q, log_excp_bus_d, log_excp_bus_upd; 
logic gpr_stack_ptr_q, gpr_stack_ptr_d, gpr_stack_ptr_upd; 
logic set_busy_q, set_busy_d, set_busy_upd; 
logic abs_inst_running_q, abs_inst_running_d, abs_inst_running_upd; 
logic[1:0] ar_cnt_q, ar_cnt_d; 
logic ar_cnt_upd; 
logic[2:0] am_cnt_q, am_cnt_d; 
logic am_cnt_upd; 
dmr_dmstatus_t dmstatus_out; 
dmr_dmcontrol_t dmcontrol_in, dmcontrol_out; 
dmr_dmcontrol_reg_t dmcontrol_d, dmcontrol_q; 
logic dmcontrol_upd, sel_dmcontrol; 
logic dmactive_d, dmactive_q; 
logic dmactive_upd; 
dmr_abstractcs_t abstractcs_in, abstractcs_out; 
dmr_abstractcs_reg_t abstractcs_d, abstractcs_q; 
logic abstractcs_upd,sel_abstractcs; 
dmr_command_t command_d, command_q, command_out; 
logic command_upd, sel_command; 
dmr_cmd_ar_t cmd_ar; 
dmr_cmd_am_t cmd_am; 
dmr_abstractauto_t abstractauto_in, abstractauto_out; 
dmr_abstractauto_reg_t abstractauto_d, abstractauto_q; 
logic abstractauto_upd, sel_abstractauto; 
logic[31:0] nextdm_out; 
logic[31:0] authdata_out; 
logic[2-1:0][31:0] data_d, data_q; 
logic[2-1:0] data_upd, data_sel; 
logic pbip_add1; 
logic pr_idle; 
logic pr_inst_req; 
logic pr_inst_exec; 
logic prog_start; 
logic is_ebreak; 
logic pr_fail; 
logic prog_stat_back2idle; 
logic prog_end_noexcp; 
logic prog_inst_sending; 
logic qa_start; 
logic qa_req_halt; 
logic [3-1:0] pr_fail_cause ; 
logic [32-1:0] prog_inst_to_send; 
logic [31:0] prog_buf_rdata; 
logic [2-1:0] prog_sel_vec; 
logic [$clog2(2)-1:0] pbip; 
logic [2-1:0] auto_prog_vec; 
logic [2-1:0] touch_prog_vec; 
qa_state_e qa_stat_q, qa_stat_d; 
logic qa_stat_upd; 
prog_stat_e pr_stat_q, pr_stat_d; 
logic pr_stat_upd; 
logic [$clog2(2+1)-1:0] pbip_q, pbip_d; 
logic pbip_upd; 
logic[2-1:0][31:0] progbuf_d, progbuf_q; 
logic[2-1:0] progbuf_upd, progbuf_sel; 
logic cmd_is_qa; 
logic qa_idle; 
logic qa_upc_fail; 
logic qa_fail; 
logic [3-1:0] qa_fail_cause; 
logic runctr_stat_resumereq; 
`WDFFER(dmactive_q, dmactive_d, dmactive_upd, clk, pwrup_rst_n) 
`WDFFER(dmcontrol_q, dmcontrol_d, dmcontrol_upd, clk, rst_n) 
`WDFFER(abstractcs_q, abstractcs_d, abstractcs_upd, clk, rst_n) 
`WDFFER(command_q, command_d, command_upd, clk, rst_n) 
`WDFFER(abstractauto_q, abstractauto_d, abstractauto_upd, clk, rst_n) 
for(genvar i=0; i<2; i++) begin : data_regs_gen 
`WDFFENR(data_q[i], data_d[i], data_upd[i], clk) 
end 
for(genvar i=0; i<2; i++) begin : prog_buf_gen 
`WDFFENR(progbuf_q[i], progbuf_d[i], progbuf_upd[i], clk) 
end 
assign dmi_vld = dmi_dm_psel && dmi_dm_penable; 
assign dmi_rw = dmi_dm_pwrite; 
assign dmi_addr = dmi_dm_paddr[32-1:2]; 
assign dmi_wdata = dmi_dm_pwdata; 
assign dm_dmi_prdata = dmi_rdata; 
assign dm_dmi_pready = 1'b1; 
assign dm_dmi_pslverr = 1'b0; 
assign dmi_write = dmi_vld && (dmi_rw == 1'b1); 
assign dmi_read = dmi_vld && (dmi_rw == 1'b0); 
assign sel_dmcontrol = dm_is_active && dm_authed && (dmi_addr == DMR_DMCONTROL_ADDR); 
assign sel_dmactive = dmi_addr == DMR_DMCONTROL_ADDR; 
assign sel_abstractcs = dm_is_active && dm_authed && !ndmreset_ongoing && (dmi_addr == DMR_ABSTRACTCS_ADDR); 
assign sel_command = dm_is_active && dm_authed && !ndmreset_ongoing && (dmi_addr == DMR_COMMAND_ADDR); 
assign sel_abstractauto = dm_is_active && dm_authed && !ndmreset_ongoing && (dmi_addr == DMR_ABSTRACTAUTO_ADDR); 
assign dmcontrol_in.haltreq = dmi_wdata[31]; 
assign dmcontrol_in.resumereq = dmi_wdata[30]; 
assign dmcontrol_in.hartreset = dmi_wdata[29]; 
assign dmcontrol_in.ackhavereset = dmi_wdata[28]; 
assign dmcontrol_in.ackunavail = dmi_wdata[27]; 
assign dmcontrol_in.hasel = dmi_wdata[26]; 
assign dmcontrol_in.hartsello = dmi_wdata[25:16]; 
assign dmcontrol_in.hartselhi = dmi_wdata[15:6]; 
assign dmcontrol_in.setkeepalive = dmi_wdata[5]; 
assign dmcontrol_in.clrkeepalive = dmi_wdata[4]; 
assign dmcontrol_in.setresethaltreq = dmi_wdata[3]; 
assign dmcontrol_in.clrresethaltreq = dmi_wdata[2]; 
assign dmcontrol_in.ndmreset = dmi_wdata[1]; 
assign dmcontrol_in.dmactive = dmi_wdata[0]; 
assign abstractcs_in.progbufsize = dmi_wdata[28:24]; 
assign abstractcs_in.busy = dmi_wdata[12]; 
assign abstractcs_in.relaxedpriv = dmi_wdata[11]; 
assign abstractcs_in.cmderr = dmi_wdata[10:8]; 
assign abstractcs_in.datacount = dmi_wdata[3:0]; 
assign abstractauto_in.autoexecprogbuf = dmi_wdata[31:16]; 
assign abstractauto_in.autoexecdata = dmi_wdata[11:0]; 
for(genvar i=0; i<2; i++) begin : data_regs_r_sel 
assign data_sel_vec[i] = dmi_addr == (DMR_DATA0_ADDR + 30'(i)); 
end 
always_comb begin 
data_rdata = {32{1'b0}}; 
for(int i=0; i<2; i++) begin 
data_rdata = ({32{data_sel_vec[i]}} & data_q[i]) | data_rdata; 
end 
end 
for(genvar i=0; i<2; i++) begin : prog_buf_r_sel 
assign prog_sel_vec[i] = dmi_addr == (DMR_PROGBUF0_ADDR + 30'(i)); 
end 
always_comb begin 
prog_buf_rdata = {32{1'b0}}; 
for(int i=0; i<2; i++) begin 
prog_buf_rdata = ({32{prog_sel_vec[i]}} & progbuf_q[i]) | prog_buf_rdata; 
end 
end 
assign pbdt_rdata = ({32{|data_sel_vec}} & data_rdata) 
| ({32{|prog_sel_vec}} & prog_buf_rdata) 
; 
always_comb begin 
dmi_rdata = '0; 
if(dmi_read && dm_authed && !ndmreset_ongoing) begin 
case(dmi_addr) 
DMR_DMSTATUS_ADDR : begin 
dmi_rdata[24] = dmstatus_out.ndmresetpending; 
dmi_rdata[23] = dmstatus_out.stickyunavail; 
dmi_rdata[22] = dmstatus_out.impebreak; 
dmi_rdata[19] = dmstatus_out.allhavereset; 
dmi_rdata[18] = dmstatus_out.anyhavereset; 
dmi_rdata[17] = dmstatus_out.allresumeack; 
dmi_rdata[16] = dmstatus_out.anyresumeack; 
dmi_rdata[15] = dmstatus_out.allnonexistent; 
dmi_rdata[14] = dmstatus_out.anynonexistent; 
dmi_rdata[13] = dmstatus_out.allunavail; 
dmi_rdata[12] = dmstatus_out.anyunavail; 
dmi_rdata[11] = dmstatus_out.allrunning; 
dmi_rdata[10] = dmstatus_out.anyrunning; 
dmi_rdata[9] = dmstatus_out.allhalted; 
dmi_rdata[8] = dmstatus_out.anyhalted; 
dmi_rdata[7] = dmstatus_out.authenticated; 
dmi_rdata[6] = dmstatus_out.authbusy; 
dmi_rdata[5] = dmstatus_out.hasresethaltreq; 
dmi_rdata[4] = dmstatus_out.confstrptrvalid; 
dmi_rdata[3:0] = dmstatus_out.version; 
end 
DMR_DMCONTROL_ADDR : begin 
dmi_rdata[31] = dmcontrol_out.haltreq; 
dmi_rdata[30] = dmcontrol_out.resumereq; 
dmi_rdata[29] = dmcontrol_out.hartreset; 
dmi_rdata[28] = dmcontrol_out.ackhavereset; 
dmi_rdata[27] = dmcontrol_out.ackunavail; 
dmi_rdata[26] = dmcontrol_out.hasel; 
dmi_rdata[25:16] = dmcontrol_out.hartsello; 
dmi_rdata[15:6] = dmcontrol_out.hartselhi; 
dmi_rdata[5] = dmcontrol_out.setkeepalive; 
dmi_rdata[4] = dmcontrol_out.clrkeepalive; 
dmi_rdata[3] = dmcontrol_out.setresethaltreq; 
dmi_rdata[2] = dmcontrol_out.clrresethaltreq; 
dmi_rdata[1] = dmcontrol_out.ndmreset; 
dmi_rdata[0] = dmcontrol_out.dmactive; 
end 
DMR_ABSTRACTCS_ADDR : begin 
dmi_rdata[28:24] = abstractcs_out.progbufsize; 
dmi_rdata[12] = abstractcs_out.busy; 
dmi_rdata[11] = abstractcs_out.relaxedpriv; 
dmi_rdata[10:8] = abstractcs_out.cmderr; 
dmi_rdata[3:0] = abstractcs_out.datacount; 
end 
DMR_COMMAND_ADDR : begin 
dmi_rdata[31:24] = command_out.cmdtype; 
dmi_rdata[23:0] = command_out.control; 
end 
DMR_ABSTRACTAUTO_ADDR : begin 
dmi_rdata[31:16] = abstractauto_out.autoexecprogbuf; 
dmi_rdata[11:0] = abstractauto_out.autoexecdata; 
end 
DMR_PROGBUF0_ADDR, 
DMR_PROGBUF1_ADDR, 
DMR_PROGBUF2_ADDR, 
DMR_PROGBUF3_ADDR, 
DMR_PROGBUF4_ADDR, 
DMR_PROGBUF5_ADDR, 
DMR_PROGBUF6_ADDR, 
DMR_PROGBUF7_ADDR, 
DMR_PROGBUF8_ADDR, 
DMR_PROGBUF9_ADDR, 
DMR_PROGBUF10_ADDR, 
DMR_PROGBUF11_ADDR, 
DMR_PROGBUF12_ADDR, 
DMR_PROGBUF13_ADDR, 
DMR_PROGBUF14_ADDR, 
DMR_PROGBUF15_ADDR, 
DMR_DATA0_ADDR, 
DMR_DATA1_ADDR : dmi_rdata = pbdt_rdata; 
DMR_NEXTDM_ADDR, 
DMR_AUTHDATA_ADDR : dmi_rdata = 32'b0; 
default : dmi_rdata = 32'b0; 
endcase 
end else if(dmi_read && !dm_authed) begin 
case(dmi_addr) 
DMR_DMSTATUS_ADDR : begin 
dmi_rdata = 32'b0; 
dmi_rdata[7] = dmstatus_out.authenticated; 
dmi_rdata[6] = dmstatus_out.authbusy; 
dmi_rdata[3:0] = dmstatus_out.version; 
end 
DMR_DMCONTROL_ADDR : begin 
dmi_rdata = 32'b0; 
dmi_rdata[0] = dmcontrol_out.dmactive; 
end 
default : dmi_rdata = 32'b0; 
endcase 
end else if(dmi_read && ndmreset_ongoing) begin 
case(dmi_addr) 
DMR_DMSTATUS_ADDR : begin 
dmi_rdata = 32'b0; 
dmi_rdata[22] = dmstatus_out.impebreak; 
dmi_rdata[7] = dmstatus_out.authenticated; 
dmi_rdata[6] = dmstatus_out.authbusy; 
dmi_rdata[3:0] = dmstatus_out.version; 
end 
DMR_DMCONTROL_ADDR : begin 
dmi_rdata[31] = dmcontrol_out.haltreq; 
dmi_rdata[30] = dmcontrol_out.resumereq; 
dmi_rdata[29] = dmcontrol_out.hartreset; 
dmi_rdata[28] = dmcontrol_out.ackhavereset; 
dmi_rdata[27] = dmcontrol_out.ackunavail; 
dmi_rdata[26] = dmcontrol_out.hasel; 
dmi_rdata[25:16] = dmcontrol_out.hartsello; 
dmi_rdata[15:6] = dmcontrol_out.hartselhi; 
dmi_rdata[5] = dmcontrol_out.setkeepalive; 
dmi_rdata[4] = dmcontrol_out.clrkeepalive; 
dmi_rdata[3] = dmcontrol_out.setresethaltreq; 
dmi_rdata[2] = dmcontrol_out.clrresethaltreq; 
dmi_rdata[1] = dmcontrol_out.ndmreset; 
dmi_rdata[0] = dmcontrol_out.dmactive; 
end 
default : dmi_rdata = 32'b0; 
endcase 
end 
end 
`WDFFERVAL(dm_stat_active_q, dm_stat_active_d, dm_stat_active_upd, clk, rst_n, 1'b0) 
assign dm_stat_active_d = 1'b1; 
assign dm_stat_active_upd = 1'b1; 
assign dm_is_active = dm_stat_active_q; 
assign dm_authed = top_dm_auth_bit_unfused; 
assign ndmreset_ongoing = dmcontrol_q.ndmreset; 
assign dm_normal = dm_authed && !ndmreset_ongoing; 
assign abs_busy = abscmd_running; 
`WDFFERVAL(runctr_stat_q, runctr_stat_d, runctr_stat_upd, clk, rst_n, R_IDLE) 
assign runctr_stat_upd = 1'b1; 
assign dmi_wr_haltreq_1 = dmi_write && sel_dmcontrol && dmcontrol_in.haltreq == 1'b1; 
assign dmi_wr_haltreq_0 = dmi_write && sel_dmcontrol && dmcontrol_in.haltreq == 1'b0; 
assign dmi_wr_resumereq_1 = dmi_write && sel_dmcontrol && dmcontrol_in.resumereq == 1'b1; 
always_comb begin 
runctr_stat_d = R_IDLE; 
case(runctr_stat_q) 
R_IDLE : runctr_stat_d = (dmi_wr_resumereq_1 && core_halted && !dmi_wr_haltreq_1) ? R_RESUME_REQ : R_IDLE; 
R_RESUME_REQ : runctr_stat_d = (core_running || dmi_wr_haltreq_1) ? R_IDLE : R_RESUME_REQ; 
default : runctr_stat_d = R_IDLE; 
endcase 
end 
assign runctr_stat_resumereq = runctr_stat_q == R_RESUME_REQ; 
`WDFFER(resume_ack_q, resume_ack_d, resume_ack_upd, clk, rst_n) 
assign resume_ack_upd = (dmi_wr_resumereq_1 && dmi_wr_haltreq_0) || (core_running && runctr_stat_resumereq); 
assign resume_ack_d = (core_running && runctr_stat_resumereq) ? 1'b1 : 1'b0; 
`WDFFER(halt_on_reset_q, halt_on_reset_d, halt_on_reset_upd, clk, rst_n) 
assign dmi_wr_clrhtrst = dmi_write && sel_dmcontrol && dmcontrol_in.clrresethaltreq == 1'b1; 
assign dmi_wr_sethtrst = dmi_write && sel_dmcontrol && dmcontrol_in.setresethaltreq == 1'b1; 
assign halt_on_reset_upd = dmi_wr_clrhtrst || dmi_wr_sethtrst; 
assign halt_on_reset_d = dmi_wr_clrhtrst ? 1'b0 : 1'b1; 
`WDFFER(cmd_start_q, cmd_start_d, cmd_start_upd, clk, rst_n) 
assign cmd_start_upd = 1'b1; 
assign cmd_touch_en = !abscmd_running && (abstractcs_q.cmderr == 3'b0) && dm_normal; 
assign cmd_written = dmi_write && sel_command && cmd_touch_en; 
for(genvar i=0; i<2; i++) begin : abs_auto_data 
assign touch_data_vec[i] = (dmi_read || dmi_write) && (dmi_addr == DMR_DATA0_ADDR + 30'(i)); 
assign auto_data_vec[i] = abstractauto_out.autoexecdata[i] && touch_data_vec[i]; 
end 
for(genvar i=0; i<2; i++) begin : abs_auto_prog 
assign touch_prog_vec[i] = (dmi_read || dmi_write) && (dmi_addr == DMR_PROGBUF0_ADDR + 30'(i)); 
assign auto_prog_vec[i] = abstractauto_out.autoexecprogbuf[i] && touch_prog_vec[i]; 
end 
assign absauto_start = ((|auto_data_vec) 
|| (|auto_prog_vec) 
) && cmd_touch_en; 
assign cmd_start_d = cmd_written || absauto_start; 
assign cmd_will_start = (cmd_written || absauto_start) && cmd_start_upd; 
assign cmd_start = cmd_start_q; 
`WDFFER(log_excp_q, log_excp_d, log_excp_upd, clk, rst_n) 
assign log_excp_upd = cmd_will_start || ((ar_stat_q == AR_REG_RW || am_stat_q == AM_MEM_RW) && core_inst_done_with_excp); 
assign log_excp_d = cmd_will_start ? 1'b0 : 1'b1; 
`WDFFER(log_excp_bus_q, log_excp_bus_d, log_excp_bus_upd, clk, rst_n) 
assign log_excp_bus_upd = cmd_will_start || ((ar_stat_q == AR_REG_RW || am_stat_q == AM_MEM_RW) && core_inst_done_with_excp && core_inst_excp_bus); 
assign log_excp_bus_d = cmd_will_start ? 1'b0 : 1'b1; 
`WDFFER(gpr_stack_ptr_q, gpr_stack_ptr_d, gpr_stack_ptr_upd, clk, rst_n) 
assign gpr_stack_ptr_upd = fst_push_succ || fst_pop_succ; 
assign gpr_stack_ptr_d = fst_push_succ ? 1'b1 : 1'b0; 
`WDFFER(set_busy_q, set_busy_d, set_busy_upd, clk, rst_n) 
assign touch_command = dmi_write && sel_command; 
assign touch_abstractcs = dmi_write && sel_abstractcs; 
assign touch_abstractauto = dmi_write && sel_abstractauto; 
assign touch_pbdt = 
(|touch_prog_vec) || 
(|touch_data_vec); 
assign abscmd_running = cmd_start || (!ar_idle || !am_idle 
|| !qa_idle 
); 
assign illegal_touch = abscmd_running && (touch_command || touch_abstractauto || touch_abstractcs || touch_pbdt); 
assign set_busy_upd = cmd_will_start || illegal_touch; 
assign set_busy_d = cmd_will_start ? 1'b0 : 1'b1; 
assign abscmd_end = (!ar_idle && ar_stat_d == AR_IDLE && ar_stat_upd) || 
(!am_idle && am_stat_d == AM_IDLE && am_stat_upd) 
|| (!qa_idle && qa_stat_d == QA_IDLE && qa_stat_upd) 
; 
assign busy_err = abscmd_end && (set_busy_q || illegal_touch); 
assign abs_wrong_type = (command_q.cmdtype > 8'd2 
) && cmd_start; 
assign abs_fail = ar_fail1 || ar_fail2 || ar_fail3 || am_fail1 || am_fail2 || busy_err || abs_wrong_type 
|| qa_fail || pr_fail 
; 
assign abs_fail_cause = busy_err ? ERR_BUSY : 
(({3{ar_fail1}} & ar_fail1_cause) 
| ({3{ar_fail2}} & ar_fail2_cause) 
| ({3{ar_fail3}} & ar_fail3_cause) 
| ({3{am_fail1}} & am_fail1_cause) 
| ({3{am_fail2}} & am_fail2_cause) 
| ({3{qa_fail }} & qa_fail_cause ) 
| ({3{pr_fail }} & pr_fail_cause ) 
| ({3{abs_wrong_type}} & ERR_NOT_SUPPORT)); 
`WDFFER(abs_inst_running_q, abs_inst_running_d, abs_inst_running_upd, clk, rst_n) 
assign abs_inst_running_upd = abs_inst_sending || core_inst_done; 
assign abs_inst_running_d = abs_inst_sending ? 1'b1 : 1'b0; 
assign abs_inst_running = abs_inst_running_q; 
assign core_cmd_done_noexcp = dscratch0_trans || (core_inst_done && !core_inst_excp); 
assign core_cmd_done_with_excp = core_inst_done_with_excp; 
assign ar_push = (ar_stat_q == AR_GPR_STASH); 
assign ar_pop = (ar_stat_q == AR_GPR_POP); 
assign am_push = (am_stat_q == AM_GPR_STASH); 
assign am_pop = (am_stat_q == AM_GPR_POP); 
assign dcmd_op = ({$clog2(8){ar_reg_rw && ar_csr }} & DCMD_AR_CSR ) 
| ({$clog2(8){ar_reg_rw && ar_gpr }} & DCMD_AR_GPR ) 
| ({$clog2(8){am_mem_rw && am_store}} & DCMD_AM_ST ) 
| ({$clog2(8){am_mem_rw && am_load }} & DCMD_AM_LD ) 
| ({$clog2(8){ar_push || am_push }} & DCMD_GPR_PUSH ) 
| ({$clog2(8){ar_pop || am_pop }} & DCMD_GPR_POP ); 
assign dcmd_vld = ar_reg_rw || am_mem_rw || ar_push || ar_pop || am_push || am_pop; 
assign ar_cmd_rw = cmd_ar.write; 
always_comb begin 
abs_inst_send = 1'b0; 
inst_to_send = DM_NOP; 
dscratch0_trans = 1'b0; 
dscratch0_rw = 1'b0; 
trans_data_n = {$clog2(2){1'b0}}; 
if(dcmd_vld) 
case (dcmd_op) 
DCMD_AR_CSR : begin 
if(ar_cmd_rw == RW_READ) begin 
case (ar_cnt_q) 
2'b00 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {csr_no, GPR_X0, FUNCT3_CSRRS, GPR_X5, OPCODE_SYSTEM}; 
end 
2'b01 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X5, FUNCT3_CSRRW, GPR_X0, OPCODE_SYSTEM}; 
end 
2'b10 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_READ; 
trans_data_n = {$clog2(2){1'b0}}; 
end 
default: begin end 
endcase 
end else if(ar_cmd_rw == RW_WRITE) begin 
case(ar_cnt_q) 
2'b00 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_WRITE; 
trans_data_n = {$clog2(2){1'b0}}; 
end 
2'b01 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X0, FUNCT3_CSRRS, GPR_X5, OPCODE_SYSTEM}; 
end 
2'b10 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {csr_no, GPR_X5, FUNCT3_CSRRW, GPR_X0, OPCODE_SYSTEM}; 
end 
default: begin end 
endcase 
end 
end 
DCMD_AR_GPR : begin 
if(ar_cmd_rw == RW_READ) begin 
case (ar_cnt_q) 
2'b00 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, gpr_no, FUNCT3_CSRRW, GPR_X0, OPCODE_SYSTEM}; 
end 
2'b01 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_READ; 
trans_data_n = {$clog2(2){1'b0}}; 
end 
default: begin end 
endcase 
end else if(ar_cmd_rw == RW_WRITE) begin 
case(ar_cnt_q) 
2'b00 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_WRITE; 
trans_data_n = {$clog2(2){1'b0}}; 
end 
2'b01 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X0, FUNCT3_CSRRS, gpr_no, OPCODE_SYSTEM}; 
end 
default: begin end 
endcase 
end 
end 
DCMD_AM_LD : begin 
case(am_cnt_q) 
3'd0 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_WRITE; 
trans_data_n = {{$clog2(2)-1{1'b0}}, 1'b1}; 
end 
3'd1 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X0, FUNCT3_CSRRS, GPR_X6, OPCODE_SYSTEM}; 
end 
3'd2 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {12'b0, GPR_X6, cmd_am.aamsize, GPR_X5, OPCODE_LOAD}; 
end 
3'd3 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X5, FUNCT3_CSRRW, GPR_X0, OPCODE_SYSTEM}; 
end 
3'd4 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_READ; 
trans_data_n = {$clog2(2){1'b0}}; 
end 
default: begin end 
endcase 
end 
DCMD_AM_ST : begin 
case(am_cnt_q) 
3'd0 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_WRITE; 
trans_data_n = {{$clog2(2)-1{1'b0}}, 1'b1}; 
end 
3'd1 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X0, FUNCT3_CSRRS, GPR_X6, OPCODE_SYSTEM}; 
end 
3'd2 : begin 
dscratch0_trans = 1'b1; 
dscratch0_rw = RW_WRITE; 
trans_data_n = {$clog2(2){1'b0}}; 
end 
3'd3 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {REGNO_DSCRATCH0, GPR_X0, FUNCT3_CSRRS, GPR_X5, OPCODE_SYSTEM}; 
end 
3'd4 : begin 
abs_inst_send = 1'b1; 
inst_to_send = {7'b0, GPR_X5, GPR_X6, cmd_am.aamsize, 5'b0, OPCODE_STORE}; 
end 
default: begin end 
endcase 
end 
DCMD_GPR_PUSH : begin 
abs_inst_send = 1'b1; 
if(gpr_stack_ptr_q == 1'b0) inst_to_send = {REGNO_DSCRATCH2, GPR_X5, FUNCT3_CSRRW, GPR_X0, OPCODE_SYSTEM}; 
else if(gpr_stack_ptr_q == 1'b1) inst_to_send = {REGNO_DSCRATCH3, GPR_X6, FUNCT3_CSRRW, GPR_X0, OPCODE_SYSTEM}; 
end 
DCMD_GPR_POP : begin 
abs_inst_send = 1'b1; 
if(gpr_stack_ptr_q == 1'b1) inst_to_send = {REGNO_DSCRATCH3, GPR_X0, FUNCT3_CSRRS, GPR_X6, OPCODE_SYSTEM}; 
else if(gpr_stack_ptr_q == 1'b0) inst_to_send = {REGNO_DSCRATCH2, GPR_X0, FUNCT3_CSRRS, GPR_X5, OPCODE_SYSTEM}; 
end 
default: begin end 
endcase 
end 
assign abs_inst_sending = abs_inst_send && !abs_inst_running; 
assign cmd_is_ar = command_q.cmdtype == 8'd0; 
assign cmd_ar.aarsize = command_q[22:20]; 
assign cmd_ar.aarpostincrement = command_q[19]; 
assign cmd_ar.postexec = command_q[18]; 
assign cmd_ar.transfer = command_q[17]; 
assign cmd_ar.write = command_q[16]; 
assign cmd_ar.regno = command_q[15:0]; 
`WDFFERVAL(ar_stat_q, ar_stat_d, ar_stat_upd, clk, rst_n, AR_IDLE) 
assign ar_stat_upd = cmd_is_ar; 
`WDFFER(ar_cnt_q, ar_cnt_d, ar_cnt_upd, clk, rst_n) 
assign ar_cnt_upd = ((ar_stat_q == AR_REG_RW) && core_cmd_done_noexcp) || cmd_start; 
assign ar_cnt_d = cmd_start ? 2'b0 : (ar_cnt_q + 1'b1); 
assign ar_rw_s1 = ar_cnt_q == 2'b00; 
assign ar_rw_s2 = ar_cnt_q == 2'b01; 
assign ar_rw_s3 = ar_cnt_q == 2'b10; 
assign ar_csr = cmd_ar.regno < 16'h1000; 
assign ar_gpr = (cmd_ar.regno >= 16'h1000) && (cmd_ar.regno <= 16'h101f); 
assign csr_no = cmd_ar.regno[11:0]; 
assign gpr_no = cmd_ar.regno[4:0]; 
assign ar_upc_fail = (((cmd_ar.aarsize != 3'd2) || (cmd_ar.regno > 16'h101f)) && cmd_ar.transfer) || !core_halted || (command_q[23] != 1'b0) 
; 
always_comb begin 
ar_stat_d = AR_IDLE; 
case(ar_stat_q) 
AR_IDLE : ar_stat_d = ((cmd_start && !ar_upc_fail) && cmd_ar.transfer && ar_csr) ? AR_GPR_STASH : 
((cmd_start && !ar_upc_fail) && cmd_ar.transfer && ar_gpr) ? AR_REG_RW : 
((cmd_start && !ar_upc_fail) && !cmd_ar.transfer && cmd_ar.postexec) ? AR_PROG_EXEC : 
AR_IDLE; 
AR_GPR_STASH : ar_stat_d = core_inst_done ? AR_REG_RW : AR_GPR_STASH; 
AR_REG_RW : ar_stat_d = (ar_csr && ((ar_rw_s3 && core_cmd_done_noexcp) || core_cmd_done_with_excp)) ? AR_GPR_POP : 
(ar_gpr && (ar_rw_s2 && core_cmd_done_noexcp)) ? AR_ADD_REGNO : 
(ar_gpr && core_cmd_done_with_excp) ? AR_IDLE : AR_REG_RW; 
AR_GPR_POP : ar_stat_d = (core_inst_done && (log_excp_q == 1'b0)) ? AR_ADD_REGNO : 
(core_inst_done && (log_excp_q == 1'b1)) ? AR_IDLE : AR_GPR_POP; 
AR_ADD_REGNO : ar_stat_d = 
cmd_ar.postexec ? AR_PROG_EXEC : 
AR_IDLE; 
AR_PROG_EXEC : ar_stat_d = prog_stat_back2idle ? AR_IDLE : AR_PROG_EXEC; 
default : ar_stat_d = AR_IDLE; 
endcase 
end 
assign ar_fail1 = cmd_is_ar && cmd_start && ar_upc_fail; 
assign ar_fail1_cause = ((command_q[23] != 1'b0) 
) ? ERR_NOT_SUPPORT : 
(((cmd_ar.aarsize != 3'd2) || (cmd_ar.regno > 16'h101f)) && cmd_ar.transfer) ? ERR_EXCEPTION : 
(!core_halted) ? ERR_STAT_WRONG : ERR_NONE; 
assign ar_fail2 = (ar_stat_q == AR_REG_RW) && ar_gpr && core_cmd_done_with_excp; 
assign ar_fail2_cause = core_inst_excp_bus ? ERR_BUS_ERROR : ERR_EXCEPTION; 
assign ar_fail3 = (ar_stat_q == AR_GPR_POP) && core_inst_done && (log_excp_q == 1'b1) && ar_stat_upd; 
assign ar_fail3_cause = log_excp_bus_q ? ERR_BUS_ERROR : ERR_EXCEPTION; 
assign ar_idle = ar_stat_q == AR_IDLE; 
assign ar_reg_rw = ar_stat_q == AR_REG_RW; 
assign cmd_is_qa = command_q.cmdtype == 8'd1; 
`WDFFERVAL(qa_stat_q, qa_stat_d, qa_stat_upd, clk, rst_n, QA_IDLE) 
assign qa_stat_upd = cmd_is_qa; 
assign qa_idle = qa_stat_q == QA_IDLE; 
assign qa_upc_fail = !core_running || (command_q.control != 24'b0); 
always_comb begin 
qa_stat_d = QA_IDLE; 
case(qa_stat_q) 
QA_IDLE : qa_stat_d = (cmd_start && !qa_upc_fail) ? QA_HALT_REQ : QA_IDLE; 
QA_HALT_REQ : qa_stat_d = core_halted ? QA_PROG_EXEC : QA_HALT_REQ; 
QA_PROG_EXEC : qa_stat_d = (prog_stat_back2idle && prog_end_noexcp) ? QA_RESUME_REQ : 
(prog_stat_back2idle && !prog_end_noexcp) ? QA_IDLE : QA_PROG_EXEC; 
QA_RESUME_REQ : qa_stat_d = core_running ? QA_IDLE : QA_RESUME_REQ; 
default : qa_stat_d = QA_IDLE; 
endcase 
end 
assign qa_start = cmd_start && cmd_is_qa; 
assign qa_req_halt = (qa_stat_q == QA_HALT_REQ) || (qa_stat_q == QA_IDLE && qa_start && !qa_upc_fail); 
assign qa_fail = cmd_is_qa && cmd_start && qa_upc_fail; 
assign qa_fail_cause = (command_q.control != 24'b0) ? ERR_NOT_SUPPORT : (!core_running ? ERR_STAT_WRONG : ERR_NONE); 
assign cmd_is_am = command_q.cmdtype == 8'd2; 
assign cmd_am.aamvirtual = command_q[23]; 
assign cmd_am.aamsize = command_q[22:20]; 
assign cmd_am.aampostincrement = command_q[19]; 
assign cmd_am.write = command_q[16]; 
assign cmd_am.target_specific = command_q[15:14]; 
`WDFFERVAL(am_stat_q, am_stat_d, am_stat_upd, clk, rst_n, AM_IDLE) 
assign am_stat_upd = cmd_is_am; 
assign am_cnt_upd = ((am_stat_q == AM_MEM_RW) && core_cmd_done_noexcp) || cmd_start; 
assign am_cnt_d = cmd_start ? 3'd0 : (am_cnt_q + 1'b1); 
`WDFFER(am_cnt_q, am_cnt_d, am_cnt_upd, clk, rst_n) 
assign am_rw_s1 = am_cnt_q == 3'd0; 
assign am_rw_s2 = am_cnt_q == 3'd1; 
assign am_rw_s3 = am_cnt_q == 3'd2; 
assign am_rw_s4 = am_cnt_q == 3'd3; 
assign am_rw_s5 = am_cnt_q == 3'd4; 
assign am_0_bits = {command_q[18:17], command_q[13:0]}; 
assign am_upc_fail = (cmd_am.aamsize > 3'd2) || (am_0_bits != 16'b0) || !core_halted; 
always_comb begin 
am_stat_d = AM_IDLE; 
case(am_stat_q) 
AM_IDLE : am_stat_d = (cmd_start && !am_upc_fail) ? AM_GPR_STASH : AM_IDLE; 
AM_GPR_STASH : am_stat_d = (gpr_stack_ptr_q == 1'b1 && core_inst_done) ? AM_MEM_RW : AM_GPR_STASH; 
AM_MEM_RW : am_stat_d = ((am_rw_s5 && core_cmd_done_noexcp) || core_cmd_done_with_excp) ? AM_GPR_POP : AM_MEM_RW; 
AM_GPR_POP : am_stat_d = (gpr_stack_ptr_q == 1'b0 && core_inst_done && !log_excp_q) ? AM_POST_INCR : 
(gpr_stack_ptr_q == 1'b0 && core_inst_done && log_excp_q ) ? AM_IDLE : AM_GPR_POP; 
AM_POST_INCR : am_stat_d = AM_IDLE; 
default : am_stat_d = AM_IDLE; 
endcase 
end 
assign am_fail1 = cmd_is_am && cmd_start && am_upc_fail; 
assign am_fail1_cause = (am_0_bits != 16'b0) ? ERR_NOT_SUPPORT : 
(cmd_am.aamsize > 3'd2) ? ERR_EXCEPTION : 
!core_halted ? ERR_STAT_WRONG : ERR_NONE; 
assign am_fail2 = (am_stat_q == AM_GPR_POP) && core_inst_done && (log_excp_q == 1'b1) && am_stat_upd; 
assign am_fail2_cause = log_excp_bus_q ? ERR_BUS_ERROR : ERR_EXCEPTION; 
assign fst_push_succ = (gpr_stack_ptr_q == 1'b0) && (am_stat_q == AM_GPR_STASH) && core_inst_done; 
assign fst_pop_succ = (gpr_stack_ptr_q == 1'b1) && (am_stat_q == AM_GPR_POP) && core_inst_done; 
always_comb begin 
am_incr_addr = 3'd0; 
case(cmd_am.aamsize) 
3'd0: am_incr_addr = 3'd1; 
3'd1: am_incr_addr = 3'd2; 
3'd2: am_incr_addr = 3'd4; 
default : am_incr_addr = 3'd0; 
endcase 
end 
assign am_idle = am_stat_q == AM_IDLE; 
assign am_mem_rw = am_stat_q == AM_MEM_RW; 
assign am_store = cmd_am.write == 1'b1; 
assign am_load = cmd_am.write == 1'b0; 
`WDFFER(pbip_q, pbip_d, pbip_upd, clk, rst_n) 
assign pbip_add1 = pr_inst_exec && core_inst_done && !core_inst_excp; 
assign pbip_upd = prog_start || pbip_add1; 
assign pbip_d = pbip_add1 ? (pbip_q + 1'b1) : {$clog2(2+1){1'b0}}; 
assign pbip = pbip_q[$clog2(2)-1:0]; 
`WDFFERVAL(pr_stat_q, pr_stat_d, pr_stat_upd, clk, rst_n, PR_IDLE) 
assign pr_stat_upd = prog_start || (ar_stat_q == AR_PROG_EXEC) || (qa_stat_q == QA_PROG_EXEC); 
assign pr_idle = pr_stat_q == PR_IDLE; 
assign pr_inst_req = pr_stat_q == PR_INST_REQ; 
assign pr_inst_exec = pr_stat_q == PR_INST_EXEC; 
assign prog_start = core_halted && ((ar_stat_q == AR_IDLE && cmd_start && !cmd_ar.transfer && cmd_ar.postexec && ar_stat_upd && !ar_upc_fail) 
|| (ar_stat_q == AR_ADD_REGNO && cmd_ar.postexec && ar_stat_upd) 
|| (qa_stat_q == QA_HALT_REQ && core_halted)); 
assign is_ebreak = (progbuf_q[pbip] == DM_EBREAK) || (progbuf_q[pbip][15:0] == DM_C_EBREAK) || (pbip_q == 2); 
always_comb begin 
pr_stat_d = PR_IDLE; 
case (pr_stat_q) 
PR_IDLE : pr_stat_d = prog_start ? PR_INST_REQ : PR_IDLE; 
PR_INST_REQ : pr_stat_d = is_ebreak ? PR_IDLE : 
!is_ebreak ? PR_INST_EXEC : PR_INST_REQ; 
PR_INST_EXEC : pr_stat_d = (core_inst_done && !core_inst_excp) ? PR_INST_REQ : 
(core_inst_done && core_inst_excp ) ? PR_IDLE : PR_INST_EXEC; 
default : pr_stat_d = PR_IDLE; 
endcase 
end 
assign pr_fail = pr_inst_exec && core_inst_done_with_excp; 
assign pr_fail_cause = core_inst_excp_bus ? ERR_BUS_ERROR : ERR_EXCEPTION; 
assign prog_stat_back2idle = !pr_idle && pr_stat_d == PR_IDLE && pr_stat_upd; 
assign prog_end_noexcp = pr_inst_req && is_ebreak && pr_stat_upd; 
assign prog_inst_sending = pr_inst_req && !is_ebreak; 
assign prog_inst_to_send = progbuf_q[pbip]; 
always_comb begin 
dmactive_d = dmactive_q; 
dmcontrol_d = dmcontrol_q; 
abstractcs_d = abstractcs_q; 
command_d = command_q; 
abstractauto_d = abstractauto_q; 
dmactive_upd = '0; 
dmcontrol_upd = '0; 
abstractcs_upd = '0; 
command_upd = '0; 
abstractauto_upd = '0; 
data_d = data_q; 
progbuf_d = progbuf_q; 
progbuf_upd = '0; 
data_upd = '0; 
if(abscmd_running) begin 
if(ar_stat_q == AR_ADD_REGNO && cmd_ar.aarpostincrement && cmd_ar.transfer) begin 
command_upd = 1'b1; 
if(cmd_ar.regno != 16'hffff) 
command_d = command_q + 1'b1; 
else 
command_d = {command_q[31:16], 16'h0000}; 
end 
if(am_stat_q == AM_POST_INCR && cmd_am.aampostincrement) begin 
data_upd[1] = 1'b1; 
data_d[1] = data_q[1] + am_incr_addr; 
end else if(dscratch0_trans && (dscratch0_rw == RW_READ)) begin 
data_upd[trans_data_n] = 1'b1; 
data_d[trans_data_n] = pcu_dm_dsch0_rdata; 
end 
if(abs_fail) begin 
abstractcs_upd = 1'b1; 
abstractcs_d.cmderr = abs_fail_cause; 
end 
if(dmi_write) begin 
case(dmi_addr) 
DMR_DMCONTROL_ADDR : begin 
dmcontrol_upd = 1'b1; 
dmcontrol_d.haltreq = dmcontrol_in.haltreq; 
dmcontrol_d.hartreset = dmcontrol_in.hartreset; 
dmcontrol_d.ndmreset = dmcontrol_in.ndmreset; 
dmactive_upd = 1'b1; 
dmactive_d = dmcontrol_in.dmactive; 
end 
default : begin end 
endcase 
end 
end else if(dmi_write && dm_authed) begin 
if(!ndmreset_ongoing) begin 
case(dmi_addr) 
DMR_DMCONTROL_ADDR : begin 
dmcontrol_upd = 1'b1; 
dmcontrol_d.haltreq = dmcontrol_in.haltreq; 
dmcontrol_d.hartreset = dmcontrol_in.hartreset; 
dmcontrol_d.ndmreset = dmcontrol_in.ndmreset; 
dmactive_upd = 1'b1; 
dmactive_d = dmcontrol_in.dmactive; 
end 
DMR_ABSTRACTCS_ADDR : begin 
abstractcs_upd = 1'b1; 
abstractcs_d.cmderr = ~abstractcs_in.cmderr & abstractcs_q.cmderr; 
end 
DMR_COMMAND_ADDR : begin 
if(abstractcs_q.cmderr == 3'b0) begin 
command_upd = 1'b1; 
command_d.cmdtype = dmi_wdata[31:24]; 
command_d.control = dmi_wdata[23:0]; 
end 
end 
DMR_ABSTRACTAUTO_ADDR : begin 
abstractauto_upd = 1'b1; 
abstractauto_d.autoexecprogbuf = abstractauto_in.autoexecprogbuf[2-1:0]; 
abstractauto_d.autoexecdata = abstractauto_in.autoexecdata[2-1:0]; 
end 
DMR_PROGBUF0_ADDR, 
DMR_PROGBUF1_ADDR, 
DMR_PROGBUF2_ADDR, 
DMR_PROGBUF3_ADDR, 
DMR_PROGBUF4_ADDR, 
DMR_PROGBUF5_ADDR, 
DMR_PROGBUF6_ADDR, 
DMR_PROGBUF7_ADDR, 
DMR_PROGBUF8_ADDR, 
DMR_PROGBUF9_ADDR, 
DMR_PROGBUF10_ADDR, 
DMR_PROGBUF11_ADDR, 
DMR_PROGBUF12_ADDR, 
DMR_PROGBUF13_ADDR, 
DMR_PROGBUF14_ADDR, 
DMR_PROGBUF15_ADDR, 
DMR_DATA0_ADDR, 
DMR_DATA1_ADDR : begin 
for(int i=0; i<2; i++) begin 
data_upd[i] = data_sel_vec[i]; 
data_d[i] = dmi_wdata[31:0]; 
end 
for(int i=0; i<2; i++) begin 
progbuf_upd[i] = prog_sel_vec[i]; 
progbuf_d[i] = dmi_wdata[31:0]; 
end 
end 
default : begin end 
endcase 
end else if(ndmreset_ongoing) begin 
case(dmi_addr) 
DMR_DMCONTROL_ADDR : begin 
dmcontrol_upd = 1'b1; 
dmcontrol_d.haltreq = dmcontrol_in.haltreq; 
dmcontrol_d.hartreset = dmcontrol_in.hartreset; 
dmcontrol_d.ndmreset = dmcontrol_in.ndmreset; 
dmactive_upd = 1'b1; 
dmactive_d = dmcontrol_in.dmactive; 
end 
default : begin end 
endcase 
end 
end else if(dmi_write && !dm_authed) begin 
case(dmi_addr) 
DMR_DMCONTROL_ADDR : begin 
dmactive_upd = 1'b1; 
dmactive_d = dmcontrol_in.dmactive; 
end 
default : begin end 
endcase 
end 
end 
assign dm_pcu_halt_req = dmcontrol_q.haltreq 
|| qa_req_halt 
; 
assign dm_pcu_halt_on_reset = halt_on_reset_q; 
assign dm_pcu_resume_req = runctr_stat_q == R_RESUME_REQ 
|| qa_stat_q == QA_RESUME_REQ 
; 
assign core_halted = pcu_dm_halted; 
assign core_running = !pcu_dm_unavail && !pcu_dm_halted; 
assign core_unavail = pcu_dm_unavail; 
assign core_havereset = pcu_dm_havereset; 
assign dm_pcu_ack_havereset = dmi_write && sel_dmcontrol && dmcontrol_in.ackhavereset; 
logic dm_ifu_inst_vld_q, dm_ifu_inst_vld_d, dm_ifu_inst_vld_upd; 
`WDFFER(dm_ifu_inst_vld_q, dm_ifu_inst_vld_d, dm_ifu_inst_vld_upd, clk, rst_n) 
logic [32-1:0] dm_ifu_inst_data_q, dm_ifu_inst_data_d; 
`WDFFENR(dm_ifu_inst_data_q, dm_ifu_inst_data_d, dm_ifu_inst_data_upd, clk) 
assign dm_ifu_inst_vld_upd = ifu_dm_inst_rdy; 
assign dm_ifu_inst_vld_d = abs_inst_sending 
|| prog_inst_sending 
; 
assign dm_ifu_inst_data_d = ({32{abs_inst_sending }} & inst_to_send) 
| ({32{prog_inst_sending}} & prog_inst_to_send) 
; 
assign dm_ifu_inst_data_upd = (abs_inst_sending 
|| prog_inst_sending 
); 
assign dm_ifu_inst_vld = dm_ifu_inst_vld_q; 
assign dm_ifu_inst_data = dm_ifu_inst_data_q; 
assign dm_pcu_dsch0_write = dscratch0_trans && (dscratch0_rw == RW_WRITE); 
assign dm_pcu_dsch0_wdata = data_q[trans_data_n]; 
assign core_inst_done = pcu_dm_cmd_done; 
assign core_inst_excp = pcu_dm_cmd_excp; 
assign core_inst_excp_bus = pcu_dm_bus_err; 
assign core_inst_done_with_excp = core_inst_done && core_inst_excp; 
assign dm_top_ndmreset = dmcontrol_out.ndmreset; 
assign dm_top_core_reset = dmcontrol_out.hartreset; 
assign dm_top_dmactive = dmactive_q; 
assign dmstatus_out.ndmresetpending = 1'b0; 
assign dmstatus_out.stickyunavail = 1'b0; 
assign dmstatus_out.impebreak = 1'b1; 
assign dmstatus_out.allhavereset = core_havereset; 
assign dmstatus_out.allresumeack = resume_ack_q; 
assign dmstatus_out.allnonexistent = 1'b0; 
assign dmstatus_out.allunavail = core_unavail; 
assign dmstatus_out.allrunning = core_running; 
assign dmstatus_out.allhalted = core_halted; 
assign dmstatus_out.anyhavereset = core_havereset; 
assign dmstatus_out.anyresumeack = resume_ack_q; 
assign dmstatus_out.anynonexistent = 1'b0; 
assign dmstatus_out.anyunavail = core_unavail; 
assign dmstatus_out.anyrunning = core_running; 
assign dmstatus_out.anyhalted = core_halted; 
assign dmstatus_out.authenticated = dm_authed; 
assign dmstatus_out.authbusy = 1'b0; 
assign dmstatus_out.hasresethaltreq = 1'b1; 
assign dmstatus_out.confstrptrvalid = 1'b0; 
assign dmstatus_out.version = 4'd3; 
assign dmcontrol_out.haltreq = 1'b0; 
assign dmcontrol_out.resumereq = 1'b0; 
assign dmcontrol_out.hartreset = dmcontrol_q.hartreset; 
assign dmcontrol_out.ackhavereset = 1'b0; 
assign dmcontrol_out.ackunavail = 1'b0; 
assign dmcontrol_out.hasel = 1'b0; 
assign dmcontrol_out.hartsello = 10'b0; 
assign dmcontrol_out.hartselhi = 10'b0; 
assign dmcontrol_out.setkeepalive = 1'b0; 
assign dmcontrol_out.clrkeepalive = 1'b0; 
assign dmcontrol_out.setresethaltreq = 1'b0; 
assign dmcontrol_out.clrresethaltreq = 1'b0; 
assign dmcontrol_out.ndmreset = dmcontrol_q.ndmreset; 
assign dmcontrol_out.dmactive = dm_sft_rst_n; 
assign abstractcs_out.progbufsize = 5'd2; 
assign abstractcs_out.busy = abs_busy; 
assign abstractcs_out.relaxedpriv = 1'b0; 
assign abstractcs_out.cmderr = abstractcs_q.cmderr; 
assign abstractcs_out.datacount = 4'd2; 
assign command_out.cmdtype = 8'b0; 
assign command_out.control = 24'b0; 
assign abstractauto_out.autoexecprogbuf = {{(16-2){1'b0}},abstractauto_q.autoexecprogbuf}; 
assign abstractauto_out.autoexecdata = {{(12-2){1'b0}},abstractauto_q.autoexecdata}; 
assign nextdm_out = 32'b0; 
assign authdata_out = 32'b0; 
endmodule
 







 
 
module m130_dtm_dmi ( 
input logic rst_n, 
input logic clk, 
input logic tapcsync2dmi_ch_sel_i, 
input logic [2-1:0] tapcsync2dmi_ch_id_i, 
input logic tapcsync2dmi_ch_capture_i, 
input logic tapcsync2dmi_ch_shift_i, 
input logic tapcsync2dmi_ch_pre_shift_i, 
input logic tapcsync2dmi_ch_update_i, 
input logic tapcsync2dmi_ch_tdi_i, 
output logic dmi2tapcsync_ch_tdo_o, 
output logic dmi_dm_psel, 
output logic dmi_dm_penable, 
output logic dmi_dm_pwrite, 
output logic[32-1:0] dmi_dm_paddr, 
output logic[32-1:0] dmi_dm_pwdata, 
input logic[32-1:0] dm_dmi_prdata, 
input logic dm_dmi_pready, 
input logic dm_dmi_pslverr 
); 
 
parameter M130_DTM_DBG_DMI_ADDR_WIDTH = 6'd7; 
parameter M130_DTM_DBG_DMI_DATA_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_OP_WIDTH = 2'd2; 
parameter M130_DTM_DBG_DMI_DR_DTMCS_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH = (M130_DTM_DBG_DMI_OP_WIDTH + 
M130_DTM_DBG_DMI_DATA_WIDTH + 
M130_DTM_DBG_DMI_ADDR_WIDTH); 
parameter M130_DTM_DBG_DATA0 = 7'h4; 
parameter M130_DTM_DBG_DATA1 = 7'h5; 
parameter M130_DTM_DBG_DMCONTROL = 7'h10; 
parameter M130_DTM_DBG_DMSTATUS = 7'h11; 
parameter M130_DTM_DBG_HARTINFO = 7'h12; 
parameter M130_DTM_DBG_ABSTRACTCS = 7'h16; 
parameter M130_DTM_DBG_COMMAND = 7'h17; 
parameter M130_DTM_DBG_ABSTRACTAUTO = 7'h18; 
parameter M130_DTM_DBG_PROGBUF0 = 7'h20; 
parameter M130_DTM_DBG_PROGBUF1 = 7'h21; 
parameter M130_DTM_DBG_PROGBUF2 = 7'h22; 
parameter M130_DTM_DBG_PROGBUF3 = 7'h23; 
parameter M130_DTM_DBG_PROGBUF4 = 7'h24; 
parameter M130_DTM_DBG_PROGBUF5 = 7'h25; 
parameter M130_DTM_DBG_HALTSUM0 = 7'h40; 
parameter M130_DTM_DBG_DMCONTROL_HALTREQ = 5'd31; 
parameter M130_DTM_DBG_DMCONTROL_RESUMEREQ = 5'd30; 
parameter M130_DTM_DBG_DMCONTROL_HARTRESET = 5'd29; 
parameter M130_DTM_DBG_DMCONTROL_ACKHAVERESET = 5'd28; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDB = 5'd27; 
parameter M130_DTM_DBG_DMCONTROL_HASEL = 5'd26; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_HI = 5'd25; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_LO = 5'd16; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_HI = 5'd15; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_LO = 5'd6; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_HI = 5'd5; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_LO = 5'd2; 
parameter M130_DTM_DBG_DMCONTROL_NDMRESET = 5'd1; 
parameter M130_DTM_DBG_DMCONTROL_DMACTIVE = 5'd0; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_HI = 5'd31; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_LO = 5'd23; 
parameter M130_DTM_DBG_DMSTATUS_IMPEBREAK = 5'd22; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_HI = 5'd21; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_LO = 5'd20; 
parameter M130_DTM_DBG_DMSTATUS_ALLHAVERESET = 5'd19; 
parameter M130_DTM_DBG_DMSTATUS_ANYHAVERESET = 5'd18; 
parameter M130_DTM_DBG_DMSTATUS_ALLRESUMEACK = 5'd17; 
parameter M130_DTM_DBG_DMSTATUS_ANYRESUMEACK = 5'd16; 
parameter M130_DTM_DBG_DMSTATUS_ALLNONEXISTENT = 5'd15; 
parameter M130_DTM_DBG_DMSTATUS_ANYNONEXISTENT = 5'd14; 
parameter M130_DTM_DBG_DMSTATUS_ALLUNAVAIL = 5'd13; 
parameter M130_DTM_DBG_DMSTATUS_ANYUNAVAIL = 5'd12; 
parameter M130_DTM_DBG_DMSTATUS_ALLRUNNING = 5'd11; 
parameter M130_DTM_DBG_DMSTATUS_ANYRUNNING = 5'd10; 
parameter M130_DTM_DBG_DMSTATUS_ALLHALTED = 5'd9; 
parameter M130_DTM_DBG_DMSTATUS_ANYHALTED = 5'd8; 
parameter M130_DTM_DBG_DMSTATUS_AUTHENTICATED = 5'd7; 
parameter M130_DTM_DBG_DMSTATUS_AUTHBUSY = 5'd6; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDA = 5'd5; 
parameter M130_DTM_DBG_DMSTATUS_DEVTREEVALID = 5'd4; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_HI = 5'd3; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_TYPE_HI = 5'd31; 
parameter M130_DTM_DBG_COMMAND_TYPE_LO = 5'd24; 
parameter M130_DTM_DBG_COMMAND_TYPE_WDTH = M130_DTM_DBG_COMMAND_TYPE_HI 
- M130_DTM_DBG_COMMAND_TYPE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDB = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_WDTH = M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI 
- M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDA = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_POSTEXEC = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_TRANSFER = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_HI = 5'd15; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMVIRTUAL = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMPOSTINC = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_HI = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_LO = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_HI = 5'd13; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_LO = 5'd0; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_HI = 5'd31; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_LO = 5'd29; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_HI = 5'd28; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_LO = 5'd24; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_HI = 5'd23; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_LO = 5'd13; 
parameter M130_DTM_DBG_ABSTRACTCS_BUSY = 5'd12; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDB = 5'd11; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_HI = 5'd10; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_LO = 5'd8; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_WDTH = M130_DTM_DBG_ABSTRACTCS_CMDERR_HI 
- M130_DTM_DBG_ABSTRACTCS_CMDERR_LO; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_HI = 5'd7; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_LO = 5'd4; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_HI = 5'd3; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_LO = 5'd0; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_HI = 5'd31; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_LO = 5'd24; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_HI = 5'd23; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_LO = 5'd20; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_HI = 5'd19; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_LO = 5'd17; 
parameter M130_DTM_DBG_HARTINFO_DATAACCESS = 5'd16; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_HI = 5'd15; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_LO = 5'd12; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_HI = 5'd11; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_LO = 5'd0; 
localparam int unsigned M130_DTM_TAP_STATE_WIDTH = 4; 
localparam int unsigned M130_DTM_TAP_INSTRUCTION_WIDTH = 5; 
localparam int unsigned M130_DTM_TAP_DR_IDCODE_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BLD_ID_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BYPASS_WIDTH = 1; 
localparam bit [M130_DTM_TAP_DR_BLD_ID_WIDTH-1:0] M130_DTM_TAP_BLD_ID_VALUE = 32'h0; 
typedef enum logic [M130_DTM_TAP_STATE_WIDTH-1:0] { 
M130_DTM_TAP_STATE_RESET, 
M130_DTM_TAP_STATE_IDLE, 
M130_DTM_TAP_STATE_DR_SEL_SCAN, 
M130_DTM_TAP_STATE_DR_CAPTURE, 
M130_DTM_TAP_STATE_DR_SHIFT, 
M130_DTM_TAP_STATE_DR_EXIT1, 
M130_DTM_TAP_STATE_DR_PAUSE, 
M130_DTM_TAP_STATE_DR_EXIT2, 
M130_DTM_TAP_STATE_DR_UPDATE, 
M130_DTM_TAP_STATE_IR_SEL_SCAN, 
M130_DTM_TAP_STATE_IR_CAPTURE, 
M130_DTM_TAP_STATE_IR_SHIFT, 
M130_DTM_TAP_STATE_IR_EXIT1, 
M130_DTM_TAP_STATE_IR_PAUSE, 
M130_DTM_TAP_STATE_IR_EXIT2, 
M130_DTM_TAP_STATE_IR_UPDATE 
 
 
 
 
} type_m130_dtm_tap_state_e; 
typedef enum logic [M130_DTM_TAP_INSTRUCTION_WIDTH - 1:0] { 
M130_DTM_TAP_INSTR_IDCODE = 5'h01, 
M130_DTM_TAP_INSTR_BLD_ID = 5'h04, 
M130_DTM_TAP_INSTR_SCU_ACCESS = 5'h09, 
M130_DTM_TAP_INSTR_DTMCS = 5'h10, 
M130_DTM_TAP_INSTR_DMI_ACCESS = 5'h11, 
M130_DTM_TAP_INSTR_BYPASS = 5'h1F 
 
 
 
 
} type_m130_dtm_tap_instr_e; 
 
logic dm2dmi_resp_i; 
logic [M130_DTM_DBG_DMI_DATA_WIDTH-1:0] dm2dmi_rdata_i; 
logic dmi2dm_req_o; 
logic dmi2dm_wr_o; 
logic [M130_DTM_DBG_DMI_ADDR_WIDTH-1:0] dmi2dm_addr_o; 
logic [M130_DTM_DBG_DMI_DATA_WIDTH-1:0] dmi2dm_wdata_o; 
logic tap_dr0_upd; 
assign dmi_dm_psel = 1'b1; 
assign dmi_dm_penable = dmi2dm_req_o; 
assign dmi_dm_pwrite = dmi2dm_wr_o; 
assign dmi_dm_paddr = {23'b0, dmi2dm_addr_o, 2'b00}; 
assign dmi_dm_pwdata = dmi2dm_wdata_o; 
assign dm2dmi_rdata_i = dm_dmi_prdata; 
assign dm2dmi_resp_i = dm_dmi_pready; 
localparam DTMCS_RESERVEDB_HI = 5'd31; 
localparam DTMCS_RESERVEDB_LO = 5'd18; 
localparam DTMCS_DMIHARDRESET = 5'd17; 
localparam DTMCS_DMIRESET = 5'd16; 
localparam DTMCS_RESERVEDA = 5'd15; 
localparam DTMCS_IDLE_HI = 5'd14; 
localparam DTMCS_IDLE_LO = 5'd12; 
localparam DTMCS_DMISTAT_HI = 5'd11; 
localparam DTMCS_DMISTAT_LO = 5'd10; 
localparam DTMCS_ABITS_HI = 5'd9; 
localparam DTMCS_ABITS_LO = 5'd4; 
localparam DTMCS_VERSION_HI = 5'd3; 
localparam DTMCS_VERSION_LO = 5'd0; 
localparam DMI_OP_LO = 5'd0; 
localparam DMI_OP_HI = DMI_OP_LO + M130_DTM_DBG_DMI_OP_WIDTH - 1; 
localparam DMI_DATA_LO = DMI_OP_HI + 1; 
localparam DMI_DATA_HI = DMI_DATA_LO + M130_DTM_DBG_DMI_DATA_WIDTH - 1; 
localparam DMI_ADDR_LO = DMI_DATA_HI + 1; 
localparam DMI_ADDR_HI = DMI_ADDR_LO + M130_DTM_DBG_DMI_ADDR_WIDTH - 1; 
logic tap_dr_upd; 
logic [M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH-1:0] tap_dr_ff; 
logic tap_dr0_ff; 
logic tap_dr0_next; 
logic [M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH-1:0] tap_dr_shift; 
logic [M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH-1:0] tap_dr_rdata; 
logic [M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH-1:0] tap_dr_next; 
logic dm_rdata_upd; 
logic [M130_DTM_DBG_DMI_DATA_WIDTH-1:0] dm_rdata_ff; 
logic tapc_dmi_access_req; 
logic tapc_dtmcs_sel; 
assign tapc_dtmcs_sel = (tapcsync2dmi_ch_id_i == 1'd1); 
always_comb begin 
tap_dr_rdata = '0; 
if(tapc_dtmcs_sel) begin 
tap_dr_rdata[DTMCS_RESERVEDB_HI:DTMCS_RESERVEDB_LO] = 'b0; 
tap_dr_rdata[DTMCS_DMIHARDRESET] = 'b0; 
tap_dr_rdata[DTMCS_DMIRESET] = 'b0; 
tap_dr_rdata[DTMCS_RESERVEDA] = 'b0; 
tap_dr_rdata[DTMCS_IDLE_HI:DTMCS_IDLE_LO] = 'b0; 
tap_dr_rdata[DTMCS_DMISTAT_HI:DTMCS_DMISTAT_LO] = 'b0; 
tap_dr_rdata[DTMCS_ABITS_HI :DTMCS_ABITS_LO] = M130_DTM_DBG_DMI_ADDR_WIDTH; 
tap_dr_rdata[DTMCS_VERSION_LO] = 1'b1; 
end else begin 
tap_dr_rdata[DMI_ADDR_HI:DMI_ADDR_LO] = 'b0; 
tap_dr_rdata[DMI_DATA_HI:DMI_DATA_LO] = dm_rdata_ff; 
tap_dr_rdata[DMI_OP_HI :DMI_OP_LO] = 'b0; 
end 
end 
assign tap_dr_shift = tapc_dtmcs_sel 
? {9'b0, tapcsync2dmi_ch_tdi_i, tap_dr_ff[M130_DTM_DBG_DMI_DR_DTMCS_WIDTH-1:1]} 
: {tapcsync2dmi_ch_tdi_i, tap_dr_ff[M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH-1:1]}; 
assign tap_dr_upd = tapcsync2dmi_ch_capture_i | tapcsync2dmi_ch_shift_i; 
assign tap_dr0_upd = tapcsync2dmi_ch_capture_i | tapcsync2dmi_ch_pre_shift_i; 
always_ff @(posedge clk, negedge rst_n) begin 
if (~rst_n) begin 
tap_dr_ff <= '0; 
end else if(tap_dr_upd) begin 
tap_dr_ff <= tap_dr_next; 
end 
end 
assign tap_dr_next = tapcsync2dmi_ch_capture_i ? tap_dr_rdata 
: tapcsync2dmi_ch_shift_i ? tap_dr_shift 
: tap_dr_ff; 
always_ff @(posedge clk, negedge rst_n) begin 
if(~rst_n) begin 
tap_dr0_ff <= '0; 
end else if(tap_dr0_upd) begin 
tap_dr0_ff <= tap_dr0_next; 
end 
end 
assign tap_dr0_next = tapcsync2dmi_ch_capture_i ? tap_dr_rdata[0] 
: tapcsync2dmi_ch_pre_shift_i ? tap_dr_shift[0] 
: tap_dr_ff[0]; 
assign dmi2tapcsync_ch_tdo_o = tap_dr0_ff; 
assign tapc_dmi_access_req = tapcsync2dmi_ch_update_i & tapcsync2dmi_ch_sel_i 
& (tapcsync2dmi_ch_id_i == 2'd2); 
always_comb begin 
dmi2dm_req_o = 1'b0; 
dmi2dm_wr_o = 1'b0; 
dmi2dm_addr_o = 1'b0; 
dmi2dm_wdata_o = 1'b0; 
if(tapc_dmi_access_req) begin 
dmi2dm_req_o = tap_dr_ff[DMI_OP_HI :DMI_OP_LO] != 2'b00; 
dmi2dm_wr_o = tap_dr_ff[DMI_OP_HI :DMI_OP_LO] == 2'b10; 
dmi2dm_addr_o = tap_dr_ff[DMI_ADDR_HI:DMI_ADDR_LO]; 
dmi2dm_wdata_o = tap_dr_ff[DMI_DATA_HI:DMI_DATA_LO]; 
end 
end 
assign dm_rdata_upd = dmi2dm_req_o & dm2dmi_resp_i & ~dmi2dm_wr_o; 
always_ff @(posedge clk, negedge rst_n) begin 
if (~rst_n) begin 
dm_rdata_ff <= '0; 
end else if (dm_rdata_upd) begin 
dm_rdata_ff <= dm2dmi_rdata_i; 
end 
end 
endmodule
 
 
module m130_dtm_top ( 
input logic clk, 
input logic pwrup_rst_n, 
input logic trst_n, 
input logic tck, 
input logic tms, 
input logic tdi, 
output logic tdo, 
output logic tdo_en, 
input logic dft_icg_scan_en, 
input logic dft_scan_mode, 
input logic dft_scan_rst_n, 
output logic dmi_dm_psel, 
output logic dmi_dm_penable, 
output logic dmi_dm_pwrite, 
output logic[32-1:0] dmi_dm_paddr, 
output logic[32-1:0] dmi_dm_pwdata, 
input logic[32-1:0] dm_dmi_prdata, 
input logic dm_dmi_pready, 
input logic dm_dmi_pslverr 
); 
 
parameter M130_DTM_DBG_DMI_ADDR_WIDTH = 6'd7; 
parameter M130_DTM_DBG_DMI_DATA_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_OP_WIDTH = 2'd2; 
parameter M130_DTM_DBG_DMI_DR_DTMCS_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH = (M130_DTM_DBG_DMI_OP_WIDTH + 
M130_DTM_DBG_DMI_DATA_WIDTH + 
M130_DTM_DBG_DMI_ADDR_WIDTH); 
parameter M130_DTM_DBG_DATA0 = 7'h4; 
parameter M130_DTM_DBG_DATA1 = 7'h5; 
parameter M130_DTM_DBG_DMCONTROL = 7'h10; 
parameter M130_DTM_DBG_DMSTATUS = 7'h11; 
parameter M130_DTM_DBG_HARTINFO = 7'h12; 
parameter M130_DTM_DBG_ABSTRACTCS = 7'h16; 
parameter M130_DTM_DBG_COMMAND = 7'h17; 
parameter M130_DTM_DBG_ABSTRACTAUTO = 7'h18; 
parameter M130_DTM_DBG_PROGBUF0 = 7'h20; 
parameter M130_DTM_DBG_PROGBUF1 = 7'h21; 
parameter M130_DTM_DBG_PROGBUF2 = 7'h22; 
parameter M130_DTM_DBG_PROGBUF3 = 7'h23; 
parameter M130_DTM_DBG_PROGBUF4 = 7'h24; 
parameter M130_DTM_DBG_PROGBUF5 = 7'h25; 
parameter M130_DTM_DBG_HALTSUM0 = 7'h40; 
parameter M130_DTM_DBG_DMCONTROL_HALTREQ = 5'd31; 
parameter M130_DTM_DBG_DMCONTROL_RESUMEREQ = 5'd30; 
parameter M130_DTM_DBG_DMCONTROL_HARTRESET = 5'd29; 
parameter M130_DTM_DBG_DMCONTROL_ACKHAVERESET = 5'd28; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDB = 5'd27; 
parameter M130_DTM_DBG_DMCONTROL_HASEL = 5'd26; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_HI = 5'd25; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_LO = 5'd16; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_HI = 5'd15; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_LO = 5'd6; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_HI = 5'd5; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_LO = 5'd2; 
parameter M130_DTM_DBG_DMCONTROL_NDMRESET = 5'd1; 
parameter M130_DTM_DBG_DMCONTROL_DMACTIVE = 5'd0; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_HI = 5'd31; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_LO = 5'd23; 
parameter M130_DTM_DBG_DMSTATUS_IMPEBREAK = 5'd22; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_HI = 5'd21; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_LO = 5'd20; 
parameter M130_DTM_DBG_DMSTATUS_ALLHAVERESET = 5'd19; 
parameter M130_DTM_DBG_DMSTATUS_ANYHAVERESET = 5'd18; 
parameter M130_DTM_DBG_DMSTATUS_ALLRESUMEACK = 5'd17; 
parameter M130_DTM_DBG_DMSTATUS_ANYRESUMEACK = 5'd16; 
parameter M130_DTM_DBG_DMSTATUS_ALLNONEXISTENT = 5'd15; 
parameter M130_DTM_DBG_DMSTATUS_ANYNONEXISTENT = 5'd14; 
parameter M130_DTM_DBG_DMSTATUS_ALLUNAVAIL = 5'd13; 
parameter M130_DTM_DBG_DMSTATUS_ANYUNAVAIL = 5'd12; 
parameter M130_DTM_DBG_DMSTATUS_ALLRUNNING = 5'd11; 
parameter M130_DTM_DBG_DMSTATUS_ANYRUNNING = 5'd10; 
parameter M130_DTM_DBG_DMSTATUS_ALLHALTED = 5'd9; 
parameter M130_DTM_DBG_DMSTATUS_ANYHALTED = 5'd8; 
parameter M130_DTM_DBG_DMSTATUS_AUTHENTICATED = 5'd7; 
parameter M130_DTM_DBG_DMSTATUS_AUTHBUSY = 5'd6; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDA = 5'd5; 
parameter M130_DTM_DBG_DMSTATUS_DEVTREEVALID = 5'd4; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_HI = 5'd3; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_TYPE_HI = 5'd31; 
parameter M130_DTM_DBG_COMMAND_TYPE_LO = 5'd24; 
parameter M130_DTM_DBG_COMMAND_TYPE_WDTH = M130_DTM_DBG_COMMAND_TYPE_HI 
- M130_DTM_DBG_COMMAND_TYPE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDB = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_WDTH = M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI 
- M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDA = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_POSTEXEC = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_TRANSFER = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_HI = 5'd15; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMVIRTUAL = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMPOSTINC = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_HI = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_LO = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_HI = 5'd13; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_LO = 5'd0; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_HI = 5'd31; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_LO = 5'd29; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_HI = 5'd28; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_LO = 5'd24; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_HI = 5'd23; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_LO = 5'd13; 
parameter M130_DTM_DBG_ABSTRACTCS_BUSY = 5'd12; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDB = 5'd11; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_HI = 5'd10; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_LO = 5'd8; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_WDTH = M130_DTM_DBG_ABSTRACTCS_CMDERR_HI 
- M130_DTM_DBG_ABSTRACTCS_CMDERR_LO; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_HI = 5'd7; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_LO = 5'd4; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_HI = 5'd3; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_LO = 5'd0; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_HI = 5'd31; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_LO = 5'd24; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_HI = 5'd23; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_LO = 5'd20; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_HI = 5'd19; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_LO = 5'd17; 
parameter M130_DTM_DBG_HARTINFO_DATAACCESS = 5'd16; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_HI = 5'd15; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_LO = 5'd12; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_HI = 5'd11; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_LO = 5'd0; 
localparam int unsigned M130_DTM_TAP_STATE_WIDTH = 4; 
localparam int unsigned M130_DTM_TAP_INSTRUCTION_WIDTH = 5; 
localparam int unsigned M130_DTM_TAP_DR_IDCODE_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BLD_ID_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BYPASS_WIDTH = 1; 
localparam bit [M130_DTM_TAP_DR_BLD_ID_WIDTH-1:0] M130_DTM_TAP_BLD_ID_VALUE = 32'h0; 
typedef enum logic [M130_DTM_TAP_STATE_WIDTH-1:0] { 
M130_DTM_TAP_STATE_RESET, 
M130_DTM_TAP_STATE_IDLE, 
M130_DTM_TAP_STATE_DR_SEL_SCAN, 
M130_DTM_TAP_STATE_DR_CAPTURE, 
M130_DTM_TAP_STATE_DR_SHIFT, 
M130_DTM_TAP_STATE_DR_EXIT1, 
M130_DTM_TAP_STATE_DR_PAUSE, 
M130_DTM_TAP_STATE_DR_EXIT2, 
M130_DTM_TAP_STATE_DR_UPDATE, 
M130_DTM_TAP_STATE_IR_SEL_SCAN, 
M130_DTM_TAP_STATE_IR_CAPTURE, 
M130_DTM_TAP_STATE_IR_SHIFT, 
M130_DTM_TAP_STATE_IR_EXIT1, 
M130_DTM_TAP_STATE_IR_PAUSE, 
M130_DTM_TAP_STATE_IR_EXIT2, 
M130_DTM_TAP_STATE_IR_UPDATE 
 
 
 
 
} type_m130_dtm_tap_state_e; 
typedef enum logic [M130_DTM_TAP_INSTRUCTION_WIDTH - 1:0] { 
M130_DTM_TAP_INSTR_IDCODE = 5'h01, 
M130_DTM_TAP_INSTR_BLD_ID = 5'h04, 
M130_DTM_TAP_INSTR_SCU_ACCESS = 5'h09, 
M130_DTM_TAP_INSTR_DTMCS = 5'h10, 
M130_DTM_TAP_INSTR_DMI_ACCESS = 5'h11, 
M130_DTM_TAP_INSTR_BYPASS = 5'h1F 
 
 
 
 
} type_m130_dtm_tap_instr_e; 
 
localparam int unsigned M130_DTM_CLUSTER_TOP_RST_SYNC_STAGES_NUM = 2; 
logic pwrup_rst_n_sync; 
logic dtm_trst_n; 
logic tapc_scu_ch_sel; 
logic tapc_scu_ch_sel_tapout; 
logic tapc_ch_tdo; 
logic tapc_dmi_ch_sel; 
logic [2-1:0] tapc_dmi_ch_id; 
logic tapc_dmi_ch_capture; 
logic tapc_dmi_ch_shift; 
logic tapc_dmi_ch_pre_shift; 
logic tapc_dmi_ch_update; 
logic tapc_dmi_ch_tdi; 
logic tapc_dmi_ch_tdo; 
logic tapc_dmi_ch_sel_tapout; 
logic [2-1:0] tapc_dmi_ch_id_tapout; 
logic tapc_dmi_ch_capture_tapout; 
logic tapc_dmi_ch_shift_tapout; 
logic tapc_dmi_ch_update_tapout; 
logic tapc_dmi_ch_tdi_tapout; 
logic tapc_dmi_ch_tdo_tapin; 
logic tapc_tck; 
logic tapc_tms; 
logic tapc_tdi; 
logic tapc_tdo; 
logic tapc_tdo_en; 
logic tapc_trst_n; 
logic dft_tapc_rst_n; 
assign dft_tapc_rst_n = dtm_trst_n; 
assign tapc_tck = tck; 
assign tapc_tms = tms; 
assign tapc_tdi = tdi; 
assign tdo = tapc_tdo; 
assign tdo_en = tapc_tdo_en; 
assign tapc_trst_n = dtm_trst_n; 
logic dtm_trst_n_undft ; 
assign dtm_trst_n_undft = trst_n & pwrup_rst_n; 
wing_cbb_dft_mux2 dtm_trst_n_inst (.din0(dtm_trst_n_undft), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(dtm_trst_n)); 
m130_dtm_tapc i_tapc ( 
.tapc_trst_n (dft_tapc_rst_n ), 
.tapc_tck (tapc_tck ), 
.tapc_tms (tapc_tms ), 
.tapc_tdi (tapc_tdi ), 
.tapc_tdo (tapc_tdo ), 
.tapc_tdo_en (tapc_tdo_en ), 
.soc2tapc_fuse_idcode_i (32'h1DA00EF7 ), 
.tapc2tapcsync_scu_ch_sel_o (tapc_scu_ch_sel_tapout ), 
.tapc2tapcsync_dmi_ch_sel_o (tapc_dmi_ch_sel_tapout ), 
.tapc2tapcsync_ch_id_o (tapc_dmi_ch_id_tapout ), 
.tapc2tapcsync_ch_capture_o (tapc_dmi_ch_capture_tapout ), 
.tapc2tapcsync_ch_shift_o (tapc_dmi_ch_shift_tapout ), 
.tapc2tapcsync_ch_update_o (tapc_dmi_ch_update_tapout ), 
.tapc2tapcsync_ch_tdi_o (tapc_dmi_ch_tdi_tapout ), 
.tapcsync2tapc_ch_tdo_i (tapc_dmi_ch_tdo_tapin ) 
); 
m130_dtm_tapc_synchronizer i_tapc_synchronizer ( 
.pwrup_rst_n (pwrup_rst_n ), 
.d_rst_n (pwrup_rst_n ), 
.clk (clk ), 
.tapc_trst_n (dft_tapc_rst_n ), 
.tapc_tck (tapc_tck ), 
.tapc2tapcsync_scu_ch_sel_i (tapc_scu_ch_sel_tapout ), 
.tapcsync2scu_ch_sel_o (tapc_scu_ch_sel ), 
.tapc2tapcsync_dmi_ch_sel_i (tapc_dmi_ch_sel_tapout ), 
.tapcsync2dmi_ch_sel_o (tapc_dmi_ch_sel ), 
.tapc2tapcsync_ch_id_i (tapc_dmi_ch_id_tapout ), 
.tapcsync2core_ch_id_o (tapc_dmi_ch_id ), 
.tapc2tapcsync_ch_capture_i (tapc_dmi_ch_capture_tapout ), 
.tapcsync2core_ch_capture_o (tapc_dmi_ch_capture ), 
.tapc2tapcsync_ch_shift_i (tapc_dmi_ch_shift_tapout ), 
.tapcsync2core_ch_shift_o (tapc_dmi_ch_shift ), 
.tapcsync2core_ch_pre_shift_o (tapc_dmi_ch_pre_shift ), 
.tapc2tapcsync_ch_update_i (tapc_dmi_ch_update_tapout ), 
.tapcsync2core_ch_update_o (tapc_dmi_ch_update ), 
.tapc2tapcsync_ch_tdi_i (tapc_dmi_ch_tdi_tapout ), 
.tapcsync2core_ch_tdi_o (tapc_dmi_ch_tdi ), 
.tapc2tapcsync_ch_tdo_i (tapc_dmi_ch_tdo_tapin ), 
.tapcsync2core_ch_tdo_o (tapc_ch_tdo ) 
); 
assign tapc_ch_tdo = tapc_dmi_ch_tdo & tapc_dmi_ch_sel; 
m130_dtm_dmi i_dmi ( 
.rst_n (pwrup_rst_n ), 
.clk (clk ), 
.tapcsync2dmi_ch_sel_i (tapc_dmi_ch_sel ), 
.tapcsync2dmi_ch_id_i (tapc_dmi_ch_id ), 
.tapcsync2dmi_ch_capture_i (tapc_dmi_ch_capture), 
.tapcsync2dmi_ch_shift_i (tapc_dmi_ch_shift ), 
.tapcsync2dmi_ch_pre_shift_i (tapc_dmi_ch_pre_shift ), 
.tapcsync2dmi_ch_update_i (tapc_dmi_ch_update ), 
.tapcsync2dmi_ch_tdi_i (tapc_dmi_ch_tdi ), 
.dmi2tapcsync_ch_tdo_o (tapc_dmi_ch_tdo ), 
.dmi_dm_psel (dmi_dm_psel ), 
.dmi_dm_penable (dmi_dm_penable ), 
.dmi_dm_pwrite (dmi_dm_pwrite ), 
.dmi_dm_paddr (dmi_dm_paddr ), 
.dmi_dm_pwdata (dmi_dm_pwdata ), 
.dm_dmi_prdata (dm_dmi_prdata ), 
.dm_dmi_pready (dm_dmi_pready ), 
.dm_dmi_pslverr (dm_dmi_pslverr ) 
); 
endmodule
 
 
module m130_dtm_tapc_synchronizer ( 
input logic pwrup_rst_n, 
input logic d_rst_n, 
input logic clk, 
input logic tapc_trst_n, 
input logic tapc_tck, 
input logic tapc2tapcsync_scu_ch_sel_i, 
output logic tapcsync2scu_ch_sel_o, 
input logic tapc2tapcsync_dmi_ch_sel_i, 
output logic tapcsync2dmi_ch_sel_o, 
input logic [2-1:0] tapc2tapcsync_ch_id_i, 
output logic [2-1:0] tapcsync2core_ch_id_o, 
input logic tapc2tapcsync_ch_capture_i, 
output logic tapcsync2core_ch_capture_o, 
output logic tapcsync2core_ch_pre_shift_o, 
input logic tapc2tapcsync_ch_shift_i, 
output logic tapcsync2core_ch_shift_o, 
input logic tapc2tapcsync_ch_update_i, 
output logic tapcsync2core_ch_update_o, 
input logic tapc2tapcsync_ch_tdi_i, 
output logic tapcsync2core_ch_tdi_o, 
output logic tapc2tapcsync_ch_tdo_i, 
input logic tapcsync2core_ch_tdo_o 
); 
 
parameter M130_DTM_DBG_DMI_ADDR_WIDTH = 6'd7; 
parameter M130_DTM_DBG_DMI_DATA_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_OP_WIDTH = 2'd2; 
parameter M130_DTM_DBG_DMI_DR_DTMCS_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH = (M130_DTM_DBG_DMI_OP_WIDTH + 
M130_DTM_DBG_DMI_DATA_WIDTH + 
M130_DTM_DBG_DMI_ADDR_WIDTH); 
parameter M130_DTM_DBG_DATA0 = 7'h4; 
parameter M130_DTM_DBG_DATA1 = 7'h5; 
parameter M130_DTM_DBG_DMCONTROL = 7'h10; 
parameter M130_DTM_DBG_DMSTATUS = 7'h11; 
parameter M130_DTM_DBG_HARTINFO = 7'h12; 
parameter M130_DTM_DBG_ABSTRACTCS = 7'h16; 
parameter M130_DTM_DBG_COMMAND = 7'h17; 
parameter M130_DTM_DBG_ABSTRACTAUTO = 7'h18; 
parameter M130_DTM_DBG_PROGBUF0 = 7'h20; 
parameter M130_DTM_DBG_PROGBUF1 = 7'h21; 
parameter M130_DTM_DBG_PROGBUF2 = 7'h22; 
parameter M130_DTM_DBG_PROGBUF3 = 7'h23; 
parameter M130_DTM_DBG_PROGBUF4 = 7'h24; 
parameter M130_DTM_DBG_PROGBUF5 = 7'h25; 
parameter M130_DTM_DBG_HALTSUM0 = 7'h40; 
parameter M130_DTM_DBG_DMCONTROL_HALTREQ = 5'd31; 
parameter M130_DTM_DBG_DMCONTROL_RESUMEREQ = 5'd30; 
parameter M130_DTM_DBG_DMCONTROL_HARTRESET = 5'd29; 
parameter M130_DTM_DBG_DMCONTROL_ACKHAVERESET = 5'd28; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDB = 5'd27; 
parameter M130_DTM_DBG_DMCONTROL_HASEL = 5'd26; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_HI = 5'd25; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_LO = 5'd16; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_HI = 5'd15; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_LO = 5'd6; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_HI = 5'd5; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_LO = 5'd2; 
parameter M130_DTM_DBG_DMCONTROL_NDMRESET = 5'd1; 
parameter M130_DTM_DBG_DMCONTROL_DMACTIVE = 5'd0; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_HI = 5'd31; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_LO = 5'd23; 
parameter M130_DTM_DBG_DMSTATUS_IMPEBREAK = 5'd22; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_HI = 5'd21; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_LO = 5'd20; 
parameter M130_DTM_DBG_DMSTATUS_ALLHAVERESET = 5'd19; 
parameter M130_DTM_DBG_DMSTATUS_ANYHAVERESET = 5'd18; 
parameter M130_DTM_DBG_DMSTATUS_ALLRESUMEACK = 5'd17; 
parameter M130_DTM_DBG_DMSTATUS_ANYRESUMEACK = 5'd16; 
parameter M130_DTM_DBG_DMSTATUS_ALLNONEXISTENT = 5'd15; 
parameter M130_DTM_DBG_DMSTATUS_ANYNONEXISTENT = 5'd14; 
parameter M130_DTM_DBG_DMSTATUS_ALLUNAVAIL = 5'd13; 
parameter M130_DTM_DBG_DMSTATUS_ANYUNAVAIL = 5'd12; 
parameter M130_DTM_DBG_DMSTATUS_ALLRUNNING = 5'd11; 
parameter M130_DTM_DBG_DMSTATUS_ANYRUNNING = 5'd10; 
parameter M130_DTM_DBG_DMSTATUS_ALLHALTED = 5'd9; 
parameter M130_DTM_DBG_DMSTATUS_ANYHALTED = 5'd8; 
parameter M130_DTM_DBG_DMSTATUS_AUTHENTICATED = 5'd7; 
parameter M130_DTM_DBG_DMSTATUS_AUTHBUSY = 5'd6; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDA = 5'd5; 
parameter M130_DTM_DBG_DMSTATUS_DEVTREEVALID = 5'd4; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_HI = 5'd3; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_TYPE_HI = 5'd31; 
parameter M130_DTM_DBG_COMMAND_TYPE_LO = 5'd24; 
parameter M130_DTM_DBG_COMMAND_TYPE_WDTH = M130_DTM_DBG_COMMAND_TYPE_HI 
- M130_DTM_DBG_COMMAND_TYPE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDB = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_WDTH = M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI 
- M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDA = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_POSTEXEC = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_TRANSFER = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_HI = 5'd15; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMVIRTUAL = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMPOSTINC = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_HI = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_LO = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_HI = 5'd13; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_LO = 5'd0; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_HI = 5'd31; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_LO = 5'd29; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_HI = 5'd28; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_LO = 5'd24; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_HI = 5'd23; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_LO = 5'd13; 
parameter M130_DTM_DBG_ABSTRACTCS_BUSY = 5'd12; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDB = 5'd11; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_HI = 5'd10; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_LO = 5'd8; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_WDTH = M130_DTM_DBG_ABSTRACTCS_CMDERR_HI 
- M130_DTM_DBG_ABSTRACTCS_CMDERR_LO; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_HI = 5'd7; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_LO = 5'd4; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_HI = 5'd3; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_LO = 5'd0; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_HI = 5'd31; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_LO = 5'd24; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_HI = 5'd23; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_LO = 5'd20; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_HI = 5'd19; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_LO = 5'd17; 
parameter M130_DTM_DBG_HARTINFO_DATAACCESS = 5'd16; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_HI = 5'd15; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_LO = 5'd12; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_HI = 5'd11; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_LO = 5'd0; 
localparam int unsigned M130_DTM_TAP_STATE_WIDTH = 4; 
localparam int unsigned M130_DTM_TAP_INSTRUCTION_WIDTH = 5; 
localparam int unsigned M130_DTM_TAP_DR_IDCODE_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BLD_ID_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BYPASS_WIDTH = 1; 
localparam bit [M130_DTM_TAP_DR_BLD_ID_WIDTH-1:0] M130_DTM_TAP_BLD_ID_VALUE = 32'h0; 
typedef enum logic [M130_DTM_TAP_STATE_WIDTH-1:0] { 
M130_DTM_TAP_STATE_RESET, 
M130_DTM_TAP_STATE_IDLE, 
M130_DTM_TAP_STATE_DR_SEL_SCAN, 
M130_DTM_TAP_STATE_DR_CAPTURE, 
M130_DTM_TAP_STATE_DR_SHIFT, 
M130_DTM_TAP_STATE_DR_EXIT1, 
M130_DTM_TAP_STATE_DR_PAUSE, 
M130_DTM_TAP_STATE_DR_EXIT2, 
M130_DTM_TAP_STATE_DR_UPDATE, 
M130_DTM_TAP_STATE_IR_SEL_SCAN, 
M130_DTM_TAP_STATE_IR_CAPTURE, 
M130_DTM_TAP_STATE_IR_SHIFT, 
M130_DTM_TAP_STATE_IR_EXIT1, 
M130_DTM_TAP_STATE_IR_PAUSE, 
M130_DTM_TAP_STATE_IR_EXIT2, 
M130_DTM_TAP_STATE_IR_UPDATE 
 
 
 
 
} type_m130_dtm_tap_state_e; 
typedef enum logic [M130_DTM_TAP_INSTRUCTION_WIDTH - 1:0] { 
M130_DTM_TAP_INSTR_IDCODE = 5'h01, 
M130_DTM_TAP_INSTR_BLD_ID = 5'h04, 
M130_DTM_TAP_INSTR_SCU_ACCESS = 5'h09, 
M130_DTM_TAP_INSTR_DTMCS = 5'h10, 
M130_DTM_TAP_INSTR_DMI_ACCESS = 5'h11, 
M130_DTM_TAP_INSTR_BYPASS = 5'h1F 
 
 
 
 
} type_m130_dtm_tap_instr_e; 
 
logic tck_divpos; 
logic tck_divneg; 
logic tck_rise_load; 
logic tck_rise_reset; 
logic tck_fall_load; 
logic tck_fall_reset; 
logic [1:0] tck_divpos_sync; 
logic [1:0] tck_divneg_sync; 
logic dmi_ch_capture; 
logic dmi_ch_shift; 
logic dmi_ch_update; 
logic dmi_ch_capture_sync; 
logic dmi_ch_shift_sync; 
logic dmi_ch_tdi_sync_bit; 
logic dmi_ch_tdi_sync; 
logic tck_divpos_sync_bit; 
logic tck_divneg_sync_bit; 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tck_divpos <= 1'b0; 
end else begin 
tck_divpos <= ~tck_divpos; 
end 
end 
always_ff @(negedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tck_divneg <= 1'b0; 
end else begin 
tck_divneg <= ~tck_divneg; 
end 
end 
wing_cbb_bit_sync #( 
.STAGE_N (2 ), 
.DATA_W (1 ), 
.RST_VAL (1'b0 ) 
) u_tck_divpos_sync_bit ( 
.clk (clk ), 
.rst_n (pwrup_rst_n ), 
.data_i (tck_divpos ), 
.data_o (tck_divpos_sync_bit ) 
); 
wing_cbb_bit_sync #( 
.STAGE_N (2 ), 
.DATA_W (1 ), 
.RST_VAL (1'b0 ) 
) u_tck_divneg_sync_bit ( 
.clk (clk ), 
.rst_n (pwrup_rst_n ), 
.data_i (tck_divneg ), 
.data_o (tck_divneg_sync_bit ) 
); 
always_ff @(posedge clk, negedge pwrup_rst_n) begin 
if (~pwrup_rst_n) begin 
tck_divpos_sync <= 2'd0; 
tck_divneg_sync <= 2'd0; 
end else begin 
tck_divpos_sync <= {tck_divpos_sync[0], tck_divpos_sync_bit}; 
tck_divneg_sync <= {tck_divneg_sync[0], tck_divneg_sync_bit}; 
end 
end 
assign tck_rise_load = tck_divpos_sync[0] ^ tck_divpos_sync_bit; 
assign tck_rise_reset = tck_divpos_sync[1] ^ tck_divpos_sync[0]; 
assign tck_fall_load = tck_divneg_sync[0] ^ tck_divneg_sync_bit; 
assign tck_fall_reset = tck_divneg_sync[1] ^ tck_divneg_sync[0]; 
always_ff @(negedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
dmi_ch_capture <= '0; 
dmi_ch_shift <= '0; 
dmi_ch_update <= '0; 
end else begin 
dmi_ch_capture <= tapc2tapcsync_ch_capture_i; 
dmi_ch_shift <= tapc2tapcsync_ch_shift_i; 
dmi_ch_update <= tapc2tapcsync_ch_update_i; 
end 
end 
assign tapcsync2core_ch_update_o = tck_fall_load && dmi_ch_update; 
wing_cbb_bit_sync #( 
.STAGE_N (2 ), 
.DATA_W (1 ), 
.RST_VAL (1'b0 ) 
) u_dmi_ch_capture_sync ( 
.clk (clk ), 
.rst_n (pwrup_rst_n ), 
.data_i (dmi_ch_capture ), 
.data_o (dmi_ch_capture_sync ) 
); 
wing_cbb_bit_sync #( 
.STAGE_N (2 ), 
.DATA_W (1 ), 
.RST_VAL (1'b0 ) 
) u_dmi_ch_shift_sync ( 
.clk (clk ), 
.rst_n (pwrup_rst_n ), 
.data_i (dmi_ch_shift ), 
.data_o (dmi_ch_shift_sync ) 
); 
wing_cbb_bit_sync #( 
.STAGE_N (2 ), 
.DATA_W (1 ), 
.RST_VAL (1'b0 ) 
) u_dmi_ch_tdi_sync_bit ( 
.clk (clk ), 
.rst_n (pwrup_rst_n ), 
.data_i (tapc2tapcsync_ch_tdi_i ), 
.data_o (dmi_ch_tdi_sync_bit ) 
); 
always_ff @(posedge clk, negedge pwrup_rst_n) begin 
if (~pwrup_rst_n) begin 
dmi_ch_tdi_sync <= 1'b0; 
end else begin 
dmi_ch_tdi_sync <= dmi_ch_tdi_sync_bit; 
end 
end 
assign tapcsync2core_ch_shift_o = tck_rise_load & dmi_ch_shift_sync; 
assign tapcsync2core_ch_tdi_o = tck_rise_load & dmi_ch_tdi_sync; 
assign tapcsync2core_ch_capture_o = tck_fall_load & dmi_ch_capture; 
assign tapcsync2core_ch_pre_shift_o = tck_fall_load & dmi_ch_shift; 
always_ff @(posedge clk, negedge d_rst_n) begin 
if (~d_rst_n) begin 
tapcsync2dmi_ch_sel_o <= '0; 
tapcsync2core_ch_id_o <= '0; 
end else begin 
if (tck_fall_load) begin 
tapcsync2dmi_ch_sel_o <= tapc2tapcsync_dmi_ch_sel_i; 
tapcsync2core_ch_id_o <= tapc2tapcsync_ch_id_i; 
end 
end 
end 
always_ff @(posedge clk, negedge pwrup_rst_n) begin 
if (~pwrup_rst_n) begin 
tapcsync2scu_ch_sel_o <= '0; 
end else begin 
if (tck_rise_load) begin 
tapcsync2scu_ch_sel_o <= tapc2tapcsync_scu_ch_sel_i; 
end 
end 
end 
assign tapc2tapcsync_ch_tdo_i = tapcsync2core_ch_tdo_o; 
endmodule
 
 
module m130_dtm_tapc_shift_reg #( 
parameter int unsigned M130_DTM_WIDTH = 8, 
parameter logic [M130_DTM_WIDTH-1:0] M130_DTM_RESET_VALUE = '0 
) ( 
input logic clk, 
input logic rst_n, 
input logic rst_n_sync, 
input logic fsm_dr_select, 
input logic fsm_dr_capture, 
input logic fsm_dr_shift, 
input logic din_serial, 
input logic [M130_DTM_WIDTH-1:0] din_parallel, 
output logic dout_serial, 
output logic [M130_DTM_WIDTH-1:0] dout_parallel 
); 
 
parameter M130_DTM_DBG_DMI_ADDR_WIDTH = 6'd7; 
parameter M130_DTM_DBG_DMI_DATA_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_OP_WIDTH = 2'd2; 
parameter M130_DTM_DBG_DMI_DR_DTMCS_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH = (M130_DTM_DBG_DMI_OP_WIDTH + 
M130_DTM_DBG_DMI_DATA_WIDTH + 
M130_DTM_DBG_DMI_ADDR_WIDTH); 
parameter M130_DTM_DBG_DATA0 = 7'h4; 
parameter M130_DTM_DBG_DATA1 = 7'h5; 
parameter M130_DTM_DBG_DMCONTROL = 7'h10; 
parameter M130_DTM_DBG_DMSTATUS = 7'h11; 
parameter M130_DTM_DBG_HARTINFO = 7'h12; 
parameter M130_DTM_DBG_ABSTRACTCS = 7'h16; 
parameter M130_DTM_DBG_COMMAND = 7'h17; 
parameter M130_DTM_DBG_ABSTRACTAUTO = 7'h18; 
parameter M130_DTM_DBG_PROGBUF0 = 7'h20; 
parameter M130_DTM_DBG_PROGBUF1 = 7'h21; 
parameter M130_DTM_DBG_PROGBUF2 = 7'h22; 
parameter M130_DTM_DBG_PROGBUF3 = 7'h23; 
parameter M130_DTM_DBG_PROGBUF4 = 7'h24; 
parameter M130_DTM_DBG_PROGBUF5 = 7'h25; 
parameter M130_DTM_DBG_HALTSUM0 = 7'h40; 
parameter M130_DTM_DBG_DMCONTROL_HALTREQ = 5'd31; 
parameter M130_DTM_DBG_DMCONTROL_RESUMEREQ = 5'd30; 
parameter M130_DTM_DBG_DMCONTROL_HARTRESET = 5'd29; 
parameter M130_DTM_DBG_DMCONTROL_ACKHAVERESET = 5'd28; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDB = 5'd27; 
parameter M130_DTM_DBG_DMCONTROL_HASEL = 5'd26; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_HI = 5'd25; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_LO = 5'd16; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_HI = 5'd15; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_LO = 5'd6; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_HI = 5'd5; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_LO = 5'd2; 
parameter M130_DTM_DBG_DMCONTROL_NDMRESET = 5'd1; 
parameter M130_DTM_DBG_DMCONTROL_DMACTIVE = 5'd0; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_HI = 5'd31; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_LO = 5'd23; 
parameter M130_DTM_DBG_DMSTATUS_IMPEBREAK = 5'd22; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_HI = 5'd21; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_LO = 5'd20; 
parameter M130_DTM_DBG_DMSTATUS_ALLHAVERESET = 5'd19; 
parameter M130_DTM_DBG_DMSTATUS_ANYHAVERESET = 5'd18; 
parameter M130_DTM_DBG_DMSTATUS_ALLRESUMEACK = 5'd17; 
parameter M130_DTM_DBG_DMSTATUS_ANYRESUMEACK = 5'd16; 
parameter M130_DTM_DBG_DMSTATUS_ALLNONEXISTENT = 5'd15; 
parameter M130_DTM_DBG_DMSTATUS_ANYNONEXISTENT = 5'd14; 
parameter M130_DTM_DBG_DMSTATUS_ALLUNAVAIL = 5'd13; 
parameter M130_DTM_DBG_DMSTATUS_ANYUNAVAIL = 5'd12; 
parameter M130_DTM_DBG_DMSTATUS_ALLRUNNING = 5'd11; 
parameter M130_DTM_DBG_DMSTATUS_ANYRUNNING = 5'd10; 
parameter M130_DTM_DBG_DMSTATUS_ALLHALTED = 5'd9; 
parameter M130_DTM_DBG_DMSTATUS_ANYHALTED = 5'd8; 
parameter M130_DTM_DBG_DMSTATUS_AUTHENTICATED = 5'd7; 
parameter M130_DTM_DBG_DMSTATUS_AUTHBUSY = 5'd6; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDA = 5'd5; 
parameter M130_DTM_DBG_DMSTATUS_DEVTREEVALID = 5'd4; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_HI = 5'd3; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_TYPE_HI = 5'd31; 
parameter M130_DTM_DBG_COMMAND_TYPE_LO = 5'd24; 
parameter M130_DTM_DBG_COMMAND_TYPE_WDTH = M130_DTM_DBG_COMMAND_TYPE_HI 
- M130_DTM_DBG_COMMAND_TYPE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDB = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_WDTH = M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI 
- M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDA = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_POSTEXEC = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_TRANSFER = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_HI = 5'd15; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMVIRTUAL = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMPOSTINC = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_HI = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_LO = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_HI = 5'd13; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_LO = 5'd0; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_HI = 5'd31; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_LO = 5'd29; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_HI = 5'd28; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_LO = 5'd24; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_HI = 5'd23; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_LO = 5'd13; 
parameter M130_DTM_DBG_ABSTRACTCS_BUSY = 5'd12; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDB = 5'd11; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_HI = 5'd10; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_LO = 5'd8; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_WDTH = M130_DTM_DBG_ABSTRACTCS_CMDERR_HI 
- M130_DTM_DBG_ABSTRACTCS_CMDERR_LO; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_HI = 5'd7; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_LO = 5'd4; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_HI = 5'd3; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_LO = 5'd0; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_HI = 5'd31; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_LO = 5'd24; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_HI = 5'd23; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_LO = 5'd20; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_HI = 5'd19; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_LO = 5'd17; 
parameter M130_DTM_DBG_HARTINFO_DATAACCESS = 5'd16; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_HI = 5'd15; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_LO = 5'd12; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_HI = 5'd11; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_LO = 5'd0; 
localparam int unsigned M130_DTM_TAP_STATE_WIDTH = 4; 
localparam int unsigned M130_DTM_TAP_INSTRUCTION_WIDTH = 5; 
localparam int unsigned M130_DTM_TAP_DR_IDCODE_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BLD_ID_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BYPASS_WIDTH = 1; 
localparam bit [M130_DTM_TAP_DR_BLD_ID_WIDTH-1:0] M130_DTM_TAP_BLD_ID_VALUE = 32'h0; 
typedef enum logic [M130_DTM_TAP_STATE_WIDTH-1:0] { 
M130_DTM_TAP_STATE_RESET, 
M130_DTM_TAP_STATE_IDLE, 
M130_DTM_TAP_STATE_DR_SEL_SCAN, 
M130_DTM_TAP_STATE_DR_CAPTURE, 
M130_DTM_TAP_STATE_DR_SHIFT, 
M130_DTM_TAP_STATE_DR_EXIT1, 
M130_DTM_TAP_STATE_DR_PAUSE, 
M130_DTM_TAP_STATE_DR_EXIT2, 
M130_DTM_TAP_STATE_DR_UPDATE, 
M130_DTM_TAP_STATE_IR_SEL_SCAN, 
M130_DTM_TAP_STATE_IR_CAPTURE, 
M130_DTM_TAP_STATE_IR_SHIFT, 
M130_DTM_TAP_STATE_IR_EXIT1, 
M130_DTM_TAP_STATE_IR_PAUSE, 
M130_DTM_TAP_STATE_IR_EXIT2, 
M130_DTM_TAP_STATE_IR_UPDATE 
 
 
 
 
} type_m130_dtm_tap_state_e; 
typedef enum logic [M130_DTM_TAP_INSTRUCTION_WIDTH - 1:0] { 
M130_DTM_TAP_INSTR_IDCODE = 5'h01, 
M130_DTM_TAP_INSTR_BLD_ID = 5'h04, 
M130_DTM_TAP_INSTR_SCU_ACCESS = 5'h09, 
M130_DTM_TAP_INSTR_DTMCS = 5'h10, 
M130_DTM_TAP_INSTR_DMI_ACCESS = 5'h11, 
M130_DTM_TAP_INSTR_BYPASS = 5'h1F 
 
 
 
 
} type_m130_dtm_tap_instr_e; 
 
logic [M130_DTM_WIDTH-1:0] shift_reg; 
generate 
if (M130_DTM_WIDTH > 1) 
begin : dr_shift_reg 
always_ff @(posedge clk, negedge rst_n) 
begin 
if (~rst_n) begin 
shift_reg <= M130_DTM_RESET_VALUE; 
end 
else if (~rst_n_sync) begin 
shift_reg <= M130_DTM_RESET_VALUE; 
end 
else if (fsm_dr_select & fsm_dr_capture) begin 
shift_reg <= din_parallel; 
end 
else if (fsm_dr_select & fsm_dr_shift) begin 
shift_reg <= {din_serial, shift_reg[M130_DTM_WIDTH-1:1]}; 
end 
end 
end 
else begin : dr_shift_reg 
always_ff @(posedge clk, negedge rst_n) 
begin 
if (~rst_n) begin 
shift_reg <= M130_DTM_RESET_VALUE; 
end 
else if (~rst_n_sync) begin 
shift_reg <= M130_DTM_RESET_VALUE; 
end 
else if (fsm_dr_select & fsm_dr_capture) begin 
shift_reg <= din_parallel; 
end 
else if (fsm_dr_select & fsm_dr_shift) begin 
shift_reg <= din_serial; 
end 
end 
end 
endgenerate 
assign dout_parallel = shift_reg; 
assign dout_serial = shift_reg[0]; 
endmodule
 
 
module m130_dtm_tapc ( 
input logic tapc_trst_n, 
input logic tapc_tck, 
input logic tapc_tms, 
input logic tapc_tdi, 
output logic tapc_tdo, 
output logic tapc_tdo_en, 
input logic [31:0] soc2tapc_fuse_idcode_i, 
output logic tapc2tapcsync_scu_ch_sel_o, 
output logic tapc2tapcsync_dmi_ch_sel_o, 
output logic [2-1:0] tapc2tapcsync_ch_id_o, 
output logic tapc2tapcsync_ch_capture_o, 
output logic tapc2tapcsync_ch_shift_o, 
output logic tapc2tapcsync_ch_update_o, 
output logic tapc2tapcsync_ch_tdi_o, 
input logic tapcsync2tapc_ch_tdo_i 
); 
 
parameter M130_DTM_DBG_DMI_ADDR_WIDTH = 6'd7; 
parameter M130_DTM_DBG_DMI_DATA_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_OP_WIDTH = 2'd2; 
parameter M130_DTM_DBG_DMI_DR_DTMCS_WIDTH = 6'd32; 
parameter M130_DTM_DBG_DMI_DR_DMI_ACCESS_WIDTH = (M130_DTM_DBG_DMI_OP_WIDTH + 
M130_DTM_DBG_DMI_DATA_WIDTH + 
M130_DTM_DBG_DMI_ADDR_WIDTH); 
parameter M130_DTM_DBG_DATA0 = 7'h4; 
parameter M130_DTM_DBG_DATA1 = 7'h5; 
parameter M130_DTM_DBG_DMCONTROL = 7'h10; 
parameter M130_DTM_DBG_DMSTATUS = 7'h11; 
parameter M130_DTM_DBG_HARTINFO = 7'h12; 
parameter M130_DTM_DBG_ABSTRACTCS = 7'h16; 
parameter M130_DTM_DBG_COMMAND = 7'h17; 
parameter M130_DTM_DBG_ABSTRACTAUTO = 7'h18; 
parameter M130_DTM_DBG_PROGBUF0 = 7'h20; 
parameter M130_DTM_DBG_PROGBUF1 = 7'h21; 
parameter M130_DTM_DBG_PROGBUF2 = 7'h22; 
parameter M130_DTM_DBG_PROGBUF3 = 7'h23; 
parameter M130_DTM_DBG_PROGBUF4 = 7'h24; 
parameter M130_DTM_DBG_PROGBUF5 = 7'h25; 
parameter M130_DTM_DBG_HALTSUM0 = 7'h40; 
parameter M130_DTM_DBG_DMCONTROL_HALTREQ = 5'd31; 
parameter M130_DTM_DBG_DMCONTROL_RESUMEREQ = 5'd30; 
parameter M130_DTM_DBG_DMCONTROL_HARTRESET = 5'd29; 
parameter M130_DTM_DBG_DMCONTROL_ACKHAVERESET = 5'd28; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDB = 5'd27; 
parameter M130_DTM_DBG_DMCONTROL_HASEL = 5'd26; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_HI = 5'd25; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELLO_LO = 5'd16; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_HI = 5'd15; 
parameter M130_DTM_DBG_DMCONTROL_HARTSELHI_LO = 5'd6; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_HI = 5'd5; 
parameter M130_DTM_DBG_DMCONTROL_RESERVEDA_LO = 5'd2; 
parameter M130_DTM_DBG_DMCONTROL_NDMRESET = 5'd1; 
parameter M130_DTM_DBG_DMCONTROL_DMACTIVE = 5'd0; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_HI = 5'd31; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDC_LO = 5'd23; 
parameter M130_DTM_DBG_DMSTATUS_IMPEBREAK = 5'd22; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_HI = 5'd21; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDB_LO = 5'd20; 
parameter M130_DTM_DBG_DMSTATUS_ALLHAVERESET = 5'd19; 
parameter M130_DTM_DBG_DMSTATUS_ANYHAVERESET = 5'd18; 
parameter M130_DTM_DBG_DMSTATUS_ALLRESUMEACK = 5'd17; 
parameter M130_DTM_DBG_DMSTATUS_ANYRESUMEACK = 5'd16; 
parameter M130_DTM_DBG_DMSTATUS_ALLNONEXISTENT = 5'd15; 
parameter M130_DTM_DBG_DMSTATUS_ANYNONEXISTENT = 5'd14; 
parameter M130_DTM_DBG_DMSTATUS_ALLUNAVAIL = 5'd13; 
parameter M130_DTM_DBG_DMSTATUS_ANYUNAVAIL = 5'd12; 
parameter M130_DTM_DBG_DMSTATUS_ALLRUNNING = 5'd11; 
parameter M130_DTM_DBG_DMSTATUS_ANYRUNNING = 5'd10; 
parameter M130_DTM_DBG_DMSTATUS_ALLHALTED = 5'd9; 
parameter M130_DTM_DBG_DMSTATUS_ANYHALTED = 5'd8; 
parameter M130_DTM_DBG_DMSTATUS_AUTHENTICATED = 5'd7; 
parameter M130_DTM_DBG_DMSTATUS_AUTHBUSY = 5'd6; 
parameter M130_DTM_DBG_DMSTATUS_RESERVEDA = 5'd5; 
parameter M130_DTM_DBG_DMSTATUS_DEVTREEVALID = 5'd4; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_HI = 5'd3; 
parameter M130_DTM_DBG_DMSTATUS_VERSION_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_TYPE_HI = 5'd31; 
parameter M130_DTM_DBG_COMMAND_TYPE_LO = 5'd24; 
parameter M130_DTM_DBG_COMMAND_TYPE_WDTH = M130_DTM_DBG_COMMAND_TYPE_HI 
- M130_DTM_DBG_COMMAND_TYPE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDB = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_WDTH = M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_HI 
- M130_DTM_DBG_COMMAND_ACCESSREG_SIZE_LO; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_RESERVEDA = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_POSTEXEC = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_TRANSFER = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_HI = 5'd15; 
parameter M130_DTM_DBG_COMMAND_ACCESSREG_REGNO_LO = 5'd0; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMVIRTUAL = 5'd23; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_HI = 5'd22; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMSIZE_LO = 5'd20; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_AAMPOSTINC = 5'd19; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_HI = 5'd18; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDB_LO = 5'd17; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_WRITE = 5'd16; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_HI = 5'd13; 
parameter M130_DTM_DBG_COMMAND_ACCESSMEM_RESERVEDA_LO = 5'd0; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_HI = 5'd31; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDD_LO = 5'd29; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_HI = 5'd28; 
parameter M130_DTM_DBG_ABSTRACTCS_PROGBUFSIZE_LO = 5'd24; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_HI = 5'd23; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDC_LO = 5'd13; 
parameter M130_DTM_DBG_ABSTRACTCS_BUSY = 5'd12; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDB = 5'd11; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_HI = 5'd10; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_LO = 5'd8; 
parameter M130_DTM_DBG_ABSTRACTCS_CMDERR_WDTH = M130_DTM_DBG_ABSTRACTCS_CMDERR_HI 
- M130_DTM_DBG_ABSTRACTCS_CMDERR_LO; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_HI = 5'd7; 
parameter M130_DTM_DBG_ABSTRACTCS_RESERVEDA_LO = 5'd4; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_HI = 5'd3; 
parameter M130_DTM_DBG_ABSTRACTCS_DATACOUNT_LO = 5'd0; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_HI = 5'd31; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDB_LO = 5'd24; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_HI = 5'd23; 
parameter M130_DTM_DBG_HARTINFO_NSCRATCH_LO = 5'd20; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_HI = 5'd19; 
parameter M130_DTM_DBG_HARTINFO_RESERVEDA_LO = 5'd17; 
parameter M130_DTM_DBG_HARTINFO_DATAACCESS = 5'd16; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_HI = 5'd15; 
parameter M130_DTM_DBG_HARTINFO_DATASIZE_LO = 5'd12; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_HI = 5'd11; 
parameter M130_DTM_DBG_HARTINFO_DATAADDR_LO = 5'd0; 
localparam int unsigned M130_DTM_TAP_STATE_WIDTH = 4; 
localparam int unsigned M130_DTM_TAP_INSTRUCTION_WIDTH = 5; 
localparam int unsigned M130_DTM_TAP_DR_IDCODE_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BLD_ID_WIDTH = 32; 
localparam int unsigned M130_DTM_TAP_DR_BYPASS_WIDTH = 1; 
localparam bit [M130_DTM_TAP_DR_BLD_ID_WIDTH-1:0] M130_DTM_TAP_BLD_ID_VALUE = 32'h0; 
typedef enum logic [M130_DTM_TAP_STATE_WIDTH-1:0] { 
M130_DTM_TAP_STATE_RESET, 
M130_DTM_TAP_STATE_IDLE, 
M130_DTM_TAP_STATE_DR_SEL_SCAN, 
M130_DTM_TAP_STATE_DR_CAPTURE, 
M130_DTM_TAP_STATE_DR_SHIFT, 
M130_DTM_TAP_STATE_DR_EXIT1, 
M130_DTM_TAP_STATE_DR_PAUSE, 
M130_DTM_TAP_STATE_DR_EXIT2, 
M130_DTM_TAP_STATE_DR_UPDATE, 
M130_DTM_TAP_STATE_IR_SEL_SCAN, 
M130_DTM_TAP_STATE_IR_CAPTURE, 
M130_DTM_TAP_STATE_IR_SHIFT, 
M130_DTM_TAP_STATE_IR_EXIT1, 
M130_DTM_TAP_STATE_IR_PAUSE, 
M130_DTM_TAP_STATE_IR_EXIT2, 
M130_DTM_TAP_STATE_IR_UPDATE 
 
 
 
 
} type_m130_dtm_tap_state_e; 
typedef enum logic [M130_DTM_TAP_INSTRUCTION_WIDTH - 1:0] { 
M130_DTM_TAP_INSTR_IDCODE = 5'h01, 
M130_DTM_TAP_INSTR_BLD_ID = 5'h04, 
M130_DTM_TAP_INSTR_SCU_ACCESS = 5'h09, 
M130_DTM_TAP_INSTR_DTMCS = 5'h10, 
M130_DTM_TAP_INSTR_DMI_ACCESS = 5'h11, 
M130_DTM_TAP_INSTR_BYPASS = 5'h1F 
 
 
 
 
} type_m130_dtm_tap_instr_e; 
 
logic trst_n_int; 
type_m130_dtm_tap_state_e tap_fsm_ff; 
type_m130_dtm_tap_state_e tap_fsm_next; 
logic tap_fsm_reset; 
logic tap_fsm_ir_upd; 
logic tap_fsm_ir_cap; 
logic tap_fsm_ir_shft; 
logic tap_fsm_ir_shift_ff; 
logic tap_fsm_ir_shift_next; 
logic tap_fsm_dr_capture_ff; 
logic tap_fsm_dr_capture_next; 
logic tap_fsm_dr_shift_ff; 
logic tap_fsm_dr_shift_next; 
logic tap_fsm_dr_update_ff; 
logic tap_fsm_dr_update_next; 
logic [M130_DTM_TAP_INSTRUCTION_WIDTH-1:0] tap_ir_shift_ff; 
logic [M130_DTM_TAP_INSTRUCTION_WIDTH-1:0] tap_ir_shift_next; 
logic [M130_DTM_TAP_INSTRUCTION_WIDTH-1:0] tap_ir_ff; 
logic [M130_DTM_TAP_INSTRUCTION_WIDTH-1:0] tap_ir_next; 
logic dr_bypass_sel; 
logic dr_bypass_tdo; 
logic dr_idcode_sel; 
logic dr_idcode_tdo; 
logic dr_bld_id_sel; 
logic dr_bld_id_tdo; 
logic dr_out; 
logic tdo_en_ff; 
logic tdo_en_next; 
logic tdo_out_ff; 
logic tdo_out_next; 
always_ff @(negedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
trst_n_int <= 1'b0; 
end else begin 
trst_n_int <= ~tap_fsm_reset; 
end 
end 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_fsm_ff <= M130_DTM_TAP_STATE_RESET; 
end else begin 
tap_fsm_ff <= tap_fsm_next; 
end 
end 
always_comb begin 
case (tap_fsm_ff) 
M130_DTM_TAP_STATE_RESET : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_RESET : M130_DTM_TAP_STATE_IDLE; 
M130_DTM_TAP_STATE_IDLE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_SEL_SCAN : M130_DTM_TAP_STATE_IDLE; 
M130_DTM_TAP_STATE_DR_SEL_SCAN: tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_IR_SEL_SCAN : M130_DTM_TAP_STATE_DR_CAPTURE; 
M130_DTM_TAP_STATE_DR_CAPTURE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_EXIT1 : M130_DTM_TAP_STATE_DR_SHIFT; 
M130_DTM_TAP_STATE_DR_SHIFT : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_EXIT1 : M130_DTM_TAP_STATE_DR_SHIFT; 
M130_DTM_TAP_STATE_DR_EXIT1 : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_UPDATE : M130_DTM_TAP_STATE_DR_PAUSE; 
M130_DTM_TAP_STATE_DR_PAUSE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_EXIT2 : M130_DTM_TAP_STATE_DR_PAUSE; 
M130_DTM_TAP_STATE_DR_EXIT2 : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_UPDATE : M130_DTM_TAP_STATE_DR_SHIFT; 
M130_DTM_TAP_STATE_DR_UPDATE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_SEL_SCAN : M130_DTM_TAP_STATE_IDLE; 
M130_DTM_TAP_STATE_IR_SEL_SCAN: tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_RESET : M130_DTM_TAP_STATE_IR_CAPTURE; 
M130_DTM_TAP_STATE_IR_CAPTURE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_IR_EXIT1 : M130_DTM_TAP_STATE_IR_SHIFT; 
M130_DTM_TAP_STATE_IR_SHIFT : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_IR_EXIT1 : M130_DTM_TAP_STATE_IR_SHIFT; 
M130_DTM_TAP_STATE_IR_EXIT1 : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_IR_UPDATE : M130_DTM_TAP_STATE_IR_PAUSE; 
M130_DTM_TAP_STATE_IR_PAUSE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_IR_EXIT2 : M130_DTM_TAP_STATE_IR_PAUSE; 
M130_DTM_TAP_STATE_IR_EXIT2 : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_IR_UPDATE : M130_DTM_TAP_STATE_IR_SHIFT; 
M130_DTM_TAP_STATE_IR_UPDATE : tap_fsm_next = tapc_tms ? M130_DTM_TAP_STATE_DR_SEL_SCAN : M130_DTM_TAP_STATE_IDLE; 
default : tap_fsm_next = tap_fsm_ff; 
endcase 
end 
assign tap_fsm_reset = (tap_fsm_ff == M130_DTM_TAP_STATE_RESET); 
assign tap_fsm_ir_upd = (tap_fsm_ff == M130_DTM_TAP_STATE_IR_UPDATE); 
assign tap_fsm_ir_cap = (tap_fsm_ff == M130_DTM_TAP_STATE_IR_CAPTURE); 
assign tap_fsm_ir_shft = (tap_fsm_ff == M130_DTM_TAP_STATE_IR_SHIFT); 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_ir_shift_ff <= '0; 
end else if (~trst_n_int) begin 
tap_ir_shift_ff <= '0; 
end else begin 
tap_ir_shift_ff <= tap_ir_shift_next; 
end 
end 
assign tap_ir_shift_next = tap_fsm_ir_cap ? {{($bits(tap_ir_shift_ff)-1){1'b0}}, 1'b1} 
: tap_fsm_ir_shft ? {tapc_tdi, tap_ir_shift_ff[$left(tap_ir_shift_ff):1]} 
: tap_ir_shift_ff; 
always_ff @(negedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_ir_ff <= M130_DTM_TAP_INSTR_IDCODE; 
end else if (~trst_n_int) begin 
tap_ir_ff <= M130_DTM_TAP_INSTR_IDCODE; 
end else begin 
tap_ir_ff <= tap_ir_next; 
end 
end 
assign tap_ir_next = tap_fsm_ir_upd ? tap_ir_shift_ff : tap_ir_ff; 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_fsm_ir_shift_ff <= 1'b0; 
end else if (~trst_n_int) begin 
tap_fsm_ir_shift_ff <= 1'b0; 
end else begin 
tap_fsm_ir_shift_ff <= tap_fsm_ir_shift_next; 
end 
end 
assign tap_fsm_ir_shift_next = (tap_fsm_next == M130_DTM_TAP_STATE_IR_SHIFT); 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_fsm_dr_capture_ff <= 1'b0; 
end else if (~trst_n_int) begin 
tap_fsm_dr_capture_ff <= 1'b0; 
end else begin 
tap_fsm_dr_capture_ff <= tap_fsm_dr_capture_next; 
end 
end 
assign tap_fsm_dr_capture_next = (tap_fsm_next == M130_DTM_TAP_STATE_DR_CAPTURE); 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_fsm_dr_shift_ff <= 1'b0; 
end else if (~trst_n_int) begin 
tap_fsm_dr_shift_ff <= 1'b0; 
end else begin 
tap_fsm_dr_shift_ff <= tap_fsm_dr_shift_next; 
end 
end 
assign tap_fsm_dr_shift_next = (tap_fsm_next == M130_DTM_TAP_STATE_DR_SHIFT); 
always_ff @(posedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tap_fsm_dr_update_ff <= 1'b0; 
end else if (~trst_n_int) begin 
tap_fsm_dr_update_ff <= 1'b0; 
end else begin 
tap_fsm_dr_update_ff <= tap_fsm_dr_update_next; 
end 
end 
assign tap_fsm_dr_update_next = (tap_fsm_next == M130_DTM_TAP_STATE_DR_UPDATE); 
always_comb begin 
dr_bypass_sel = 1'b0; 
dr_idcode_sel = 1'b0; 
dr_bld_id_sel = 1'b0; 
tapc2tapcsync_scu_ch_sel_o = 1'b0; 
tapc2tapcsync_dmi_ch_sel_o = 1'b0; 
case (tap_ir_ff) 
M130_DTM_TAP_INSTR_DTMCS : tapc2tapcsync_dmi_ch_sel_o = 1'b1; 
M130_DTM_TAP_INSTR_DMI_ACCESS: tapc2tapcsync_dmi_ch_sel_o = 1'b1; 
M130_DTM_TAP_INSTR_IDCODE : dr_idcode_sel = 1'b1; 
M130_DTM_TAP_INSTR_BYPASS : dr_bypass_sel = 1'b1; 
M130_DTM_TAP_INSTR_BLD_ID : dr_bld_id_sel = 1'b1; 
default : dr_bypass_sel = 1'b1; 
endcase 
end 
always_comb begin 
tapc2tapcsync_ch_id_o = '0; 
case (tap_ir_ff) 
M130_DTM_TAP_INSTR_DTMCS : tapc2tapcsync_ch_id_o = 'd1; 
M130_DTM_TAP_INSTR_DMI_ACCESS: tapc2tapcsync_ch_id_o = 'd2; 
default : tapc2tapcsync_ch_id_o = '0; 
endcase 
end 
always_comb begin 
dr_out = 1'b0; 
case (tap_ir_ff) 
M130_DTM_TAP_INSTR_DTMCS : dr_out = tapcsync2tapc_ch_tdo_i; 
M130_DTM_TAP_INSTR_DMI_ACCESS: dr_out = tapcsync2tapc_ch_tdo_i; 
M130_DTM_TAP_INSTR_IDCODE : dr_out = dr_idcode_tdo; 
M130_DTM_TAP_INSTR_BYPASS : dr_out = dr_bypass_tdo; 
M130_DTM_TAP_INSTR_BLD_ID : dr_out = dr_bld_id_tdo; 
M130_DTM_TAP_INSTR_SCU_ACCESS: dr_out = tapcsync2tapc_ch_tdo_i; 
default : dr_out = dr_bypass_tdo; 
endcase 
end 
always_ff @(negedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tdo_en_ff <= 1'b0; 
end else if (~trst_n_int) begin 
tdo_en_ff <= 1'b0; 
end else begin 
tdo_en_ff <= tdo_en_next; 
end 
end 
assign tdo_en_next = tap_fsm_dr_shift_ff | tap_fsm_ir_shift_ff; 
always_ff @(negedge tapc_tck, negedge tapc_trst_n) begin 
if (~tapc_trst_n) begin 
tdo_out_ff <= 1'b0; 
end else if (~trst_n_int) begin 
tdo_out_ff <= 1'b0; 
end else begin 
tdo_out_ff <= tdo_out_next; 
end 
end 
assign tdo_out_next = tap_fsm_dr_shift_ff ? dr_out 
: tap_fsm_ir_shift_ff ? tap_ir_shift_ff[0] 
: 1'b0; 
assign tapc_tdo_en = tdo_en_ff; 
assign tapc_tdo = tdo_out_ff; 
m130_dtm_tapc_shift_reg #( 
.M130_DTM_WIDTH (M130_DTM_TAP_DR_BYPASS_WIDTH), 
.M130_DTM_RESET_VALUE ({M130_DTM_TAP_DR_BYPASS_WIDTH{1'b0}}) 
) i_bypass_reg ( 
.clk (tapc_tck ), 
.rst_n (tapc_trst_n ), 
.rst_n_sync (trst_n_int ), 
.fsm_dr_select (dr_bypass_sel ), 
.fsm_dr_capture (tap_fsm_dr_capture_ff ), 
.fsm_dr_shift (tap_fsm_dr_shift_ff ), 
.din_serial (tapc_tdi ), 
.din_parallel (1'b0 ), 
.dout_serial (dr_bypass_tdo ), 
.dout_parallel ( ) 
); 
m130_dtm_tapc_shift_reg #( 
.M130_DTM_WIDTH (M130_DTM_TAP_DR_IDCODE_WIDTH), 
.M130_DTM_RESET_VALUE ({M130_DTM_TAP_DR_IDCODE_WIDTH{1'b0}}) 
) i_tap_idcode_reg ( 
.clk (tapc_tck ), 
.rst_n (tapc_trst_n ), 
.rst_n_sync (trst_n_int ), 
.fsm_dr_select (dr_idcode_sel ), 
.fsm_dr_capture (tap_fsm_dr_capture_ff ), 
.fsm_dr_shift (tap_fsm_dr_shift_ff ), 
.din_serial (tapc_tdi ), 
.din_parallel (soc2tapc_fuse_idcode_i ), 
.dout_serial (dr_idcode_tdo ), 
.dout_parallel ( ) 
); 
m130_dtm_tapc_shift_reg #( 
.M130_DTM_WIDTH (M130_DTM_TAP_DR_BLD_ID_WIDTH), 
.M130_DTM_RESET_VALUE ({M130_DTM_TAP_DR_BLD_ID_WIDTH{1'b0}}) 
) i_tap_dr_bld_id_reg ( 
.clk (tapc_tck ), 
.rst_n (tapc_trst_n ), 
.rst_n_sync (trst_n_int ), 
.fsm_dr_select (dr_bld_id_sel ), 
.fsm_dr_capture (tap_fsm_dr_capture_ff ), 
.fsm_dr_shift (tap_fsm_dr_shift_ff ), 
.din_serial (tapc_tdi ), 
.din_parallel (M130_DTM_TAP_BLD_ID_VALUE), 
.dout_serial (dr_bld_id_tdo ), 
.dout_parallel ( ) 
); 
assign tapc2tapcsync_ch_tdi_o = tapc_tdi; 
assign tapc2tapcsync_ch_capture_o = tap_fsm_dr_capture_ff; 
assign tapc2tapcsync_ch_shift_o = tap_fsm_dr_shift_ff; 
assign tapc2tapcsync_ch_update_o = tap_fsm_dr_update_ff; 
endmodule
 
 

 
 
module m130_tm( 
input logic[1:0] core_priv_mode, 
input logic core_dbg_mode, 
input logic pcu_csr_vld, 
input logic[1:0] pcu_csr_opcode, 
input logic[11:0] pcu_csr_index, 
input logic[32-1:0] pcu_csr_wdata, 
output logic tm_pcu_csr_done, 
output logic[32-1:0] tm_pcu_csr_rdata, 
output logic tm_pcu_csr_excp, 
output logic tm_pcu_csr_match, 
input logic[2-1:0] pcu_tm_pc_vld, 
input logic[2-1:0][32-1:0] pcu_tm_pc, 
input logic[2-1:0] pcu_tm_inst_size, 
input logic pcu_tm_trap_vld, 
input logic[32-1:0] pcu_tm_trap_cause, 
input logic[2-1:0] pcu_tm_inst_retire, 
input logic pcu_tm_trigger_hit, 
input logic[4-1:0] pcu_tm_trigger_hit_idx, 
input logic pcu_tm_tcontrol_mte, 
output logic[2-1:0] tm_pcu_trigger_pc_vld, 
output logic[2-1:0] tm_pcu_trigger_pc_debug, 
output logic[2-1:0][4-1:0] tm_pcu_trigger_hit_pc_idx, 
input logic lsu_tm_a_vld, 
input logic[32-1:0] lsu_tm_a_addr, 
input logic[2:0] lsu_tm_a_ldst, 
input logic[2-1:0] lsu_tm_a_size, 
output logic tm_lsu_trigger_a_vld, 
output logic tm_lsu_trigger_debug_a, 
input logic lsu_tm_d_vld, 
input logic[32-1:0] lsu_tm_d_data, 
input logic[2:0] lsu_tm_d_ldst, 
input logic[2-1:0] lsu_tm_d_size, 
output logic tm_lsu_trigger_d_vld, 
output logic tm_lsu_trigger_debug_d, 
output logic[4-1:0] tm_lsu_trigger_hit_a_idx, 
input logic clic_tm_intctl_vld, 
input logic[15:0] top_tm_vld_vec, 
input logic clk, 
input logic rst_n 
); 
 
 
 
localparam CSROP_READ = 2'b00; 
localparam CSROP_WRITE = 2'b01; 
localparam CSROP_SET = 2'b10; 
localparam CSROP_CLEAR = 2'b11; 
localparam PRIV_M_MODE = 2'b11; 
localparam PRIV_U_MODE = 2'b00; 
localparam TM_TYPE_NONE = 4'd0; 
localparam TM_TYPE_ICOUNT = 4'd3; 
localparam TM_TYPE_ITRIGGER = 4'd4; 
localparam TM_TYPE_ETRIGGER = 4'd5; 
localparam TM_TYPE_MCONTROL6 = 4'd6; 
localparam TM_TYPE_TMEXTTRIGGER = 4'd7; 
localparam TM_TYPE_DISABLED = 4'hf; 
localparam TM_ACTION_BREAKPOINT = 4'd0; 
localparam TM_ACTION_DEBUG_MODE = 4'd1; 
localparam TM_ACTION_TRACE_ON = 4'd2; 
localparam TM_ACTION_TRACE_OFF = 4'd3; 
localparam TM_ACTION_TRACE_NOTIFY = 4'd4; 
localparam TM_ACTION_EXT0 = 4'd6; 
localparam TM_ACTION_EXT1 = 4'd7; 
localparam TM_SIZE_BYTE = 2'b00; 
localparam TM_SIZE_HALF = 2'b01; 
localparam TM_SIZE_WORD = 2'b10; 
localparam TM_LDST_LD = 3'b001; 
localparam TM_LDST_ST = 3'b010; 
localparam TM_LDST_AMO = 3'b100; 
typedef struct packed { 
logic[3:0] tm_type; 
logic dmode; 
logic[32-6:0] data; 
} tmr_tdata1_t; 
typedef struct packed { 
logic[3:0] tm_type; 
logic dmode; 
logic uncertain; 
logic hit1; 
logic vs; 
logic vu; 
logic hit0; 
logic select; 
logic[2:0] size; 
logic[3:0] action; 
logic chain; 
logic[3:0] match; 
logic m; 
logic uncertainen; 
logic s; 
logic u; 
logic execute; 
logic store; 
logic load; 
} tmr_mcontrol6_t; 
typedef struct packed { 
logic[3:0] tm_type; 
logic dmode; 
logic[1:0] size; 
logic action; 
logic m; 
logic u; 
logic execute; 
logic store; 
logic load; 
} tmr_mcontrol6_reg_t; 
 
 
typedef enum logic[12-1:0] { 
TMR_TSELECT_ADDR = 12'h7a0, 
TMR_TDATA1_ADDR = 12'h7a1, 
TMR_TDATA2_ADDR = 12'h7a2, 
TMR_TDATA3_ADDR = 12'h7a3, 
TMR_TINFO_ADDR = 12'h7a4, 
TMR_HCONTEXT_ADDR = 12'h6a8, 
TMR_SCONTEXT_ADDR = 12'h5a8, 
TMR_MCONTEXT_ADDR = 12'h7a8, 
TMR_MSCONTEXT_ADDR = 12'h7aa 
} tmr_addr_e; 
logic mode_is_m; 
logic mode_is_u; 
logic m_brkp_en; 
logic tmr_vld; 
logic tmr_write; 
logic mem_a_vld; 
logic mem_a_ldst; 
logic mem_a_ld; 
logic mem_a_st; 
logic mem_a_amo; 
logic[2-1:0][4-1:0] pc_hit_vec; 
logic[2-1:0][4-1:0] pc_dbg_vec; 
logic[2-1:0] pc_hit; 
logic[2-1:0] pc_dbg; 
logic[4-1:0] mem_a_hit_vec; 
logic[4-1:0] mem_a_dbg_vec; 
logic[4-1:0] mcontrol6_fire_en; 
logic mem_a_hit; 
logic mem_a_dbg; 
logic[$clog2(4)-1:0] tidx; 
logic[$clog2(4)-1:0] midx; 
logic[12-1:0] tmr_addr; 
logic[32-1:0] tmr_wdata; 
logic[2-1:0] pc_vld; 
logic[2-1:0][32-1:0] pc_value; 
logic[32-1:0] mem_a_addr; 
logic[2-1:0] mem_a_size; 
logic[32-1:0] tselect_in; 
logic[32-1:0] tselect_out; 
logic[$clog2(4)-1:0] tselect_q, tselect_d; 
logic tselect_upd; 
tmr_tdata1_t[4-1:0] tdata1_in, tdata1_out; 
logic[4-1:0][32-1:0] tdata2_q, tdata2_d; 
logic[4-1:0] tdata2_upd; 
tmr_mcontrol6_t[4-1:0] mcontrol6, mcontrol6_in; 
tmr_mcontrol6_reg_t[4-1:0] mcontrol6_q, mcontrol6_d; 
logic[4-1:0] mcontrol6_upd; 
logic[4-1:0] mcontrol6_hit0_wdata; 
logic[4-1:0] mcontrol6_hit0_set; 
logic[4-1:0] mcontrol6_hit0_d; 
logic[4-1:0] mcontrol6_hit0_q; 
logic[4-1:0] mcontrol6_hit0_upd; 
logic[32-1:0] tinfo_out; 
`WDFFER(tselect_q, tselect_d, tselect_upd, clk, rst_n) 
for(genvar i=0; i<4; i++) begin : tdata2_reg_gen 
`WDFFER(tdata2_q[i], tdata2_d[i], tdata2_upd[i], clk, rst_n) 
end 
for(genvar i=0; i<4; i++) begin : mcontrol6_reg_gen 
`WDFFERVAL(mcontrol6_q[i], mcontrol6_d[i], mcontrol6_upd[i], clk, rst_n, {TM_TYPE_DISABLED, 9'b0}) 
`WDFFER(mcontrol6_hit0_q[i], mcontrol6_hit0_d[i], mcontrol6_hit0_upd[i], clk, rst_n) 
end 
assign tidx = tselect_q; 
assign midx = tidx; 
assign mode_is_m = core_priv_mode == PRIV_M_MODE; 
assign mode_is_u = core_priv_mode == PRIV_U_MODE; 
assign m_brkp_en = pcu_tm_tcontrol_mte; 
assign tmr_vld = pcu_csr_vld; 
assign tmr_write = (pcu_csr_opcode == CSROP_WRITE) && tmr_vld; 
assign tmr_addr = pcu_csr_index; 
assign tmr_wdata = pcu_csr_wdata; 
logic[32-1:0] tmr_rdata; 
logic tmr_excp; 
logic tmr_match; 
assign tm_pcu_csr_done = tmr_vld; 
assign tm_pcu_csr_rdata = tmr_rdata; 
assign tm_pcu_csr_excp = tmr_excp; 
assign tm_pcu_csr_match = tmr_match; 
assign tselect_in = tmr_wdata; 
assign tselect_out = {{(32-$clog2(4)){1'b0}},tselect_q}; 
for(genvar i=0; i<4; i++) begin : tdata1_write_gen 
assign tdata1_in[i].tm_type = tmr_wdata[32-1:32-4]; 
assign tdata1_in[i].dmode = tmr_wdata[32-5]; 
assign tdata1_in[i].data = tmr_wdata[32-6:0]; 
end 
for(genvar i=0; i<4; i++) begin : mcontrol6_in_gen 
assign mcontrol6_in[i].tm_type = tdata1_in[i].tm_type; 
assign mcontrol6_in[i].dmode = tdata1_in[i].dmode; 
assign mcontrol6_in[i].uncertain = tdata1_in[i].data[26]; 
assign mcontrol6_in[i].hit1 = tdata1_in[i].data[25]; 
assign mcontrol6_in[i].vs = tdata1_in[i].data[24]; 
assign mcontrol6_in[i].vu = tdata1_in[i].data[23]; 
assign mcontrol6_in[i].hit0 = tdata1_in[i].data[22]; 
assign mcontrol6_in[i].select = tdata1_in[i].data[21]; 
assign mcontrol6_in[i].size = tdata1_in[i].data[18:16]; 
assign mcontrol6_in[i].action = tdata1_in[i].data[15:12]; 
assign mcontrol6_in[i].chain = tdata1_in[i].data[11]; 
assign mcontrol6_in[i].match = tdata1_in[i].data[10:7]; 
assign mcontrol6_in[i].m = tdata1_in[i].data[6]; 
assign mcontrol6_in[i].uncertainen = tdata1_in[i].data[5]; 
assign mcontrol6_in[i].s = tdata1_in[i].data[4]; 
assign mcontrol6_in[i].u = tdata1_in[i].data[3]; 
assign mcontrol6_in[i].execute = tdata1_in[i].data[2]; 
assign mcontrol6_in[i].store = tdata1_in[i].data[1]; 
assign mcontrol6_in[i].load = tdata1_in[i].data[0]; 
end 
always_comb begin 
tmr_rdata = '0; 
tmr_excp = '0; 
tmr_match = '0; 
tinfo_out = 32'h01000040; 
if(tmr_vld && mode_is_m) begin 
case(tmr_addr) 
TMR_TSELECT_ADDR: begin 
tmr_match = 1'b1; 
tmr_excp = 1'b0; 
tmr_rdata = tselect_out; 
end 
TMR_TDATA1_ADDR: begin 
tmr_match = 1'b1; 
tmr_rdata = tdata1_out[tidx]; 
tmr_excp = 1'b0; 
end 
TMR_TDATA2_ADDR: begin 
tmr_match = 1'b1; 
tmr_excp = 1'b0; 
tmr_rdata = tdata2_q[tidx]; 
end 
TMR_TINFO_ADDR: begin 
tmr_match = 1'b1; 
tmr_excp = 1'b0; 
tmr_rdata = tinfo_out; 
end 
endcase 
end else if(tmr_vld && mode_is_u) begin 
case(tmr_addr) 
TMR_TSELECT_ADDR, 
TMR_TDATA1_ADDR, 
TMR_TDATA2_ADDR, 
TMR_TINFO_ADDR: begin 
tmr_match = 1'b1; 
tmr_excp = 1'b1; 
end 
endcase 
end 
end 
always_comb begin 
tselect_upd = '0; 
tdata2_upd = '0; 
mcontrol6_upd = '0; 
mcontrol6_hit0_wdata = mcontrol6_hit0_q; 
tselect_d = tselect_q; 
tdata2_d = tdata2_q; 
mcontrol6_d = mcontrol6_q; 
if(tmr_write && mode_is_m) begin 
case(tmr_addr) 
TMR_TSELECT_ADDR: begin 
tselect_upd = 1'b1; 
if(tselect_in > 4 -1) 
tselect_d = '0; 
else 
tselect_d = tselect_in[$clog2(4)-1:0]; 
end 
TMR_TDATA1_ADDR: begin 
if(!(tdata1_out[tidx].dmode == 1'b1 && !core_dbg_mode)) begin 
mcontrol6_upd[midx] = 1'b1; 
if(tdata1_in[tidx].tm_type == TM_TYPE_MCONTROL6) begin 
mcontrol6_hit0_wdata[midx] = mcontrol6_in[midx].hit0; 
mcontrol6_d[midx].tm_type = mcontrol6_in[midx].tm_type; 
mcontrol6_d[midx].dmode = core_dbg_mode ? mcontrol6_in[midx].dmode 
: mcontrol6_q[midx].dmode; 
mcontrol6_d[midx].size = mcontrol6_in[midx].size[2] ? 3'b0 
: mcontrol6_in[midx].size[1:0]; 
mcontrol6_d[midx].action = (|mcontrol6_in[midx].action[3:1]) ? 4'b0 
: mcontrol6_in[midx].action[0] && mcontrol6_d[midx].dmode; 
mcontrol6_d[midx].m = mcontrol6_in[midx].m; 
mcontrol6_d[midx].u = mcontrol6_in[midx].u; 
mcontrol6_d[midx].execute = mcontrol6_in[midx].execute; 
mcontrol6_d[midx].store = mcontrol6_in[midx].store; 
mcontrol6_d[midx].load = mcontrol6_in[midx].load; 
end else begin 
mcontrol6_d[midx].tm_type = TM_TYPE_DISABLED; 
end 
end 
end 
TMR_TDATA2_ADDR: begin 
if(!(tdata1_out[tidx].dmode == 1'b1 && !core_dbg_mode)) begin 
tdata2_upd[tidx] = 1'b1; 
tdata2_d[tidx] = tmr_wdata; 
end 
end 
default: begin end 
endcase 
end 
end 
for(genvar i=0; i<4; i++) begin : match_control_type_6_gen 
always_comb begin 
mcontrol6[i].tm_type = mcontrol6_q[i].tm_type; 
mcontrol6[i].dmode = mcontrol6_q[i].dmode; 
mcontrol6[i].uncertain = 1'b0; 
mcontrol6[i].hit1 = 1'b0; 
mcontrol6[i].vs = 1'b0; 
mcontrol6[i].vu = 1'b0; 
mcontrol6[i].hit0 = mcontrol6_hit0_q[i]; 
mcontrol6[i].select = 1'b0; 
mcontrol6[i].size = {1'b0, mcontrol6_q[i].size}; 
mcontrol6[i].action = {3'b0, mcontrol6_q[i].action}; 
mcontrol6[i].chain = 1'b0; 
mcontrol6[i].match = 4'b0; 
mcontrol6[i].m = mcontrol6_q[i].m; 
mcontrol6[i].uncertainen= 1'b0; 
mcontrol6[i].s = 1'b0; 
mcontrol6[i].u = mcontrol6_q[i].u; 
mcontrol6[i].execute = mcontrol6_q[i].execute; 
mcontrol6[i].store = mcontrol6_q[i].store; 
mcontrol6[i].load = mcontrol6_q[i].load; 
tdata1_out[i].tm_type = mcontrol6[i].tm_type; 
tdata1_out[i].dmode = mcontrol6[i].dmode; 
tdata1_out[i].data[26] = mcontrol6[i].uncertain; 
tdata1_out[i].data[25] = mcontrol6[i].hit1; 
tdata1_out[i].data[24] = mcontrol6[i].vs; 
tdata1_out[i].data[23] = mcontrol6[i].vu; 
tdata1_out[i].data[22] = mcontrol6[i].hit0; 
tdata1_out[i].data[21] = mcontrol6[i].select; 
tdata1_out[i].data[20:19] = 2'b0; 
tdata1_out[i].data[18:16] = mcontrol6[i].size; 
tdata1_out[i].data[15:12] = mcontrol6[i].action; 
tdata1_out[i].data[11] = mcontrol6[i].chain; 
tdata1_out[i].data[10:7] = mcontrol6[i].match; 
tdata1_out[i].data[6] = mcontrol6[i].m; 
tdata1_out[i].data[5] = mcontrol6[i].uncertainen; 
tdata1_out[i].data[4] = mcontrol6[i].s; 
tdata1_out[i].data[3] = mcontrol6[i].u; 
tdata1_out[i].data[2] = mcontrol6[i].execute; 
tdata1_out[i].data[1] = mcontrol6[i].store; 
tdata1_out[i].data[0] = mcontrol6[i].load; 
end 
end 
assign pc_vld = pcu_tm_pc_vld; 
assign pc_value = pcu_tm_pc; 
assign mem_a_vld = lsu_tm_a_vld; 
assign mem_a_addr = lsu_tm_a_addr; 
assign mem_a_ld = lsu_tm_a_ldst[0]; 
assign mem_a_st = lsu_tm_a_ldst[1]; 
assign mem_a_amo = lsu_tm_a_ldst[2]; 
assign mem_a_size = lsu_tm_a_size; 
assign mem_a_hit = |mem_a_hit_vec; 
assign mem_a_dbg = |mem_a_dbg_vec; 
assign tm_lsu_trigger_a_vld = mem_a_hit; 
assign tm_lsu_trigger_debug_a = mem_a_dbg; 
assign tm_lsu_trigger_hit_a_idx = mem_a_dbg ? mem_a_hit_vec & mem_a_dbg_vec 
: mem_a_hit_vec; 
for(genvar i=0; i<2; i++) begin : tm_pcu_pc_hit_gen 
assign pc_hit[i] = |pc_hit_vec[i]; 
assign pc_dbg[i] = |pc_dbg_vec[i]; 
assign tm_pcu_trigger_pc_vld[i] = pc_hit[i]; 
assign tm_pcu_trigger_pc_debug[i] = pc_dbg[i]; 
assign tm_pcu_trigger_hit_pc_idx[i] = pc_dbg[i] ? pc_hit_vec[i] & pc_dbg_vec[i] 
: pc_hit_vec[i]; 
end 
for(genvar i=0; i<4; i++) begin : tm_mcontrol6_hit_gen 
assign mcontrol6_hit0_set[i] = pcu_tm_trigger_hit & pcu_tm_trigger_hit_idx[i]; 
assign mcontrol6_hit0_d[i] = mcontrol6_hit0_set[i] ? mcontrol6_hit0_set[i] 
: mcontrol6_hit0_wdata[i]; 
assign mcontrol6_hit0_upd[i] = mcontrol6_hit0_set[i] | mcontrol6_upd[i]; 
end 
always_comb begin 
pc_hit_vec = '0; 
pc_dbg_vec = '0; 
mem_a_hit_vec = '0; 
mem_a_dbg_vec = '0; 
for(int i=0; i<4; i++) begin 
mcontrol6_fire_en[i] = (mcontrol6[i].tm_type != TM_TYPE_DISABLED) & ~core_dbg_mode; 
for(int j=0; j<2; j++) begin 
if((pc_vld[j] && mcontrol6[i].execute) 
&& (pc_value[j] == tdata2_q[i]) 
&& ((mode_is_m && mcontrol6[i].m) || (mode_is_u && mcontrol6[i].u)) 
&& (mcontrol6[i].size == 3'b0 || (pcu_tm_inst_size[j] == (mcontrol6[i].size - 3'b10)))) 
begin 
pc_dbg_vec[j][i] = (mcontrol6[i].action == TM_ACTION_DEBUG_MODE) && mcontrol6_fire_en[i]; 
if(mcontrol6[i].action == TM_ACTION_DEBUG_MODE) 
pc_hit_vec[j][i] = mcontrol6[i].dmode ? mcontrol6_fire_en[i] : 1'b0; 
else if(mcontrol6[i].action == TM_ACTION_BREAKPOINT) 
pc_hit_vec[j][i] = (m_brkp_en | !mode_is_m) ? mcontrol6_fire_en[i] : 1'b0; 
end 
end 
if(mem_a_vld 
&& ((mcontrol6[i].load && (mem_a_ld | mem_a_amo)) || (mcontrol6[i].store && (mem_a_st | mem_a_amo))) 
&& (mem_a_addr == tdata2_q[i]) 
&& ((mode_is_m && mcontrol6[i].m) || (mode_is_u && mcontrol6[i].u)) 
&& (mcontrol6[i].size == 3'b0 || (mem_a_size == (mcontrol6[i].size[1:0]-1'b1)))) 
begin 
mem_a_dbg_vec[i] = mcontrol6[i].action == TM_ACTION_DEBUG_MODE && mcontrol6_fire_en[i]; 
if(mcontrol6[i].action == TM_ACTION_DEBUG_MODE) 
mem_a_hit_vec[i] = mcontrol6[i].dmode ? mcontrol6_fire_en[i] : 1'b0; 
else if(mcontrol6[i].action == TM_ACTION_BREAKPOINT) 
mem_a_hit_vec[i] = (m_brkp_en | !mode_is_m) ? mcontrol6_fire_en[i] : 1'b0; 
end 
end 
end 
assign tm_lsu_trigger_d_vld = '0; 
assign tm_lsu_trigger_debug_d = '0; 
endmodule
 
 
module m130_bmu_top ( 
input logic clk , 
input logic always_on_clk , 
input logic rst_n , 
output logic bmu_idle , 
input logic [32-1:0] core_itcm_base_addr, 
input logic [32-1:0] core_dtcm_base_addr, 
input logic [32-1:0] core_mmr_base_addr , 
input endianess , 
input logic sleep_mode , 
input logic icache_bmu_ar_valid , 
output logic bmu_icache_ar_ready , 
input logic [32-1:0] icache_bmu_ar_addr , 
input logic [2:0] icache_bmu_ar_size , 
input logic [7:0] icache_bmu_ar_len , 
input logic [1:0] icache_bmu_ar_burst , 
input logic [1-1:0] icache_bmu_ar_id , 
input logic [3:0] icache_bmu_ar_cache , 
input logic icache_bmu_ar_lock , 
input logic [2:0] icache_bmu_ar_prot , 
input logic icache_bmu_ar_user , 
output logic bmu_icache_r_valid , 
input logic icache_bmu_r_ready , 
output logic [32-1:0] bmu_icache_r_data , 
output logic [1-1:0] bmu_icache_r_id , 
output logic bmu_icache_r_last , 
output logic [1:0] bmu_icache_r_resp , 
input logic dcache_bmu_req_vld , 
output logic bmu_dcache_req_rdy , 
input logic [32-1:0] dcache_bmu_req_addr , 
input logic [1:0] dcache_bmu_req_size , 
input logic dcache_bmu_req_write , 
input logic [32-1:0] dcache_bmu_req_wdata , 
input logic [32/8-1:0] dcache_bmu_req_wstrb , 
input logic [1-1:0] dcache_bmu_req_id , 
input logic [1:0] dcache_bmu_req_memattr, 
input logic [1:0] dcache_bmu_req_prot , 
input logic dcache_bmu_req_dm , 
input logic dcache_bmu_req_bitband, 
output logic bmu_dcache_rsp_vld , 
input logic dcache_bmu_rsp_rdy , 
output logic [32-1:0] bmu_dcache_rsp_data , 
output logic [1-1:0] bmu_dcache_rsp_id , 
output logic bmu_dcache_rsp_err , 
output logic bmu_clic_req_vld , 
input logic clic_bmu_req_rdy , 
output logic [31:0] bmu_clic_req_addr , 
output logic [ 1:0] bmu_clic_req_size , 
output logic bmu_clic_req_write , 
output logic [32/8-1:0] bmu_clic_req_wstrb , 
output logic [32-1:0] bmu_clic_req_wdata , 
output logic [1:0] bmu_clic_req_dest , 
input logic clic_bmu_rsp_vld , 
output logic bmu_clic_rsp_rdy , 
input logic [32-1:0] clic_bmu_rsp_rdata , 
input logic clic_bmu_rsp_err , 
output logic bmu_reri_req_vld , 
input logic reri_bmu_req_rdy , 
output logic [31:0] bmu_reri_req_addr , 
output logic bmu_reri_req_write, 
output logic [32-1:0] bmu_reri_req_wdata, 
input logic reri_bmu_rsp_vld , 
output logic bmu_reri_rsp_rdy , 
input logic [32-1:0] reri_bmu_rsp_rdata, 
input logic reri_bmu_rsp_err , 
output logic bmu_mss_req_vld , 
input logic mss_bmu_req_rdy , 
output logic [31:0] bmu_mss_req_addr , 
output logic bmu_mss_req_write, 
output logic [32-1:0] bmu_mss_req_wdata, 
input logic mss_bmu_rsp_vld , 
output logic bmu_mss_rsp_rdy , 
input logic [32-1:0] mss_bmu_rsp_rdata, 
input logic mss_bmu_rsp_err , 
output logic bmu_tcm_req_vld , 
input logic tcm_bmu_req_rdy , 
output logic [31:0] bmu_tcm_req_addr , 
output logic bmu_tcm_req_write , 
output logic [32/8-1:0] bmu_tcm_req_wstrb , 
output logic [32-1:0] bmu_tcm_req_wdata , 
output logic [1:0] bmu_tcm_req_dest , 
input logic tcm_bmu_rsp_vld , 
output logic bmu_tcm_rsp_rdy , 
input logic [32-1:0] tcm_bmu_rsp_rdata , 
input logic tcm_bmu_rsp_err 
,output logic [32-1:0] d_bus_ahb_haddr , 
output logic [2:0] d_bus_ahb_hburst , 
output logic d_bus_ahb_hmastlock , 
output logic [3:0] d_bus_ahb_hprot , 
output logic [2:0] d_bus_ahb_hsize , 
output logic [1:0] d_bus_ahb_hmaster , 
output logic [1:0] d_bus_ahb_htrans , 
output logic d_bus_ahb_hwrite , 
output logic [32-1:0] d_bus_ahb_hwdata , 
input logic [32-1:0] d_bus_ahb_hrdata , 
input logic d_bus_ahb_hready , 
input logic d_bus_ahb_hresp 
,output logic [32-1:0] i_bus_ahb_haddr , 
output logic [2:0] i_bus_ahb_hburst , 
output logic i_bus_ahb_hmastlock , 
output logic [3:0] i_bus_ahb_hprot , 
output logic [2:0] i_bus_ahb_hsize , 
output logic [1:0] i_bus_ahb_hmaster , 
output logic [1:0] i_bus_ahb_htrans , 
output logic i_bus_ahb_hwrite , 
output logic [32-1:0] i_bus_ahb_hwdata , 
input logic [32-1:0] i_bus_ahb_hrdata , 
input logic i_bus_ahb_hready , 
input logic i_bus_ahb_hresp 
,output logic [32-1:0] m_bus_ahb_haddr , 
output logic [2:0] m_bus_ahb_hburst , 
output logic m_bus_ahb_hmastlock , 
output logic [3:0] m_bus_ahb_hprot , 
output logic [2:0] m_bus_ahb_hsize , 
output logic [1:0] m_bus_ahb_hmaster , 
output logic [1:0] m_bus_ahb_htrans , 
output logic m_bus_ahb_hwrite , 
output logic [32-1:0] m_bus_ahb_hwdata , 
input logic [32-1:0] m_bus_ahb_hrdata , 
input logic m_bus_ahb_hready , 
input logic m_bus_ahb_hresp 
); 
typedef enum logic [1:0] { 
REGSLICE_MODE_FEEDTHROUGH = 2'b00, 
REGSLICE_MODE_FORWARD = 2'b01, 
REGSLICE_MODE_BACKWARD = 2'b10, 
REGSLICE_MODE_BIDIRECTION = 2'b11 
} regslice_mode_e; 
typedef struct packed { 
logic [31:0] addr ; 
logic [ 1:0] size ; 
logic [ 1:0] prot ; 
logic [ 1:0] memattr; 
logic [1-1:0] id; 
logic [1:0] master; 
} wxbl_req_info_ro_t; 
typedef struct packed { 
logic [31:0] addr ; 
logic [ 1:0] size ; 
logic write; 
logic [ 3:0] wstrb; 
logic [ 1:0] prot ; 
logic [ 1:0] memattr; 
logic [1-1:0] id; 
logic [1:0] master; 
} wxbl_req_info_t; 
typedef struct packed { 
logic [31:0] rdata; 
logic err ; 
logic [1-1:0] id; 
} wxbl_rsp_info_t; 
localparam FT_DEMUX_N = 4; 
localparam LDST_DEMUX_N = 6; 
typedef enum logic [1:0] { 
DMX_FT_IBUS = 2'd0, 
DMX_FT_MBUS = 2'd1, 
DMX_FT_DBUS = 2'd2, 
DMX_FT_DUMMY = 2'd3 
} bmu_demux_id_ft_e; 
typedef enum logic [2:0] { 
DMX_LS_DBUS = 3'd0, 
DMX_LS_MBUS = 3'd1, 
DMX_LS_UNCORE = 3'd2, 
DMX_LS_IBUS = 3'd3, 
DMX_LS_PBUS = 3'd4, 
DMX_LS_DUMMY = 3'd5 
} bmu_demux_id_ldst_e; 
localparam SBUS_DEMUX_N = 2; 
typedef enum logic [1:0] { 
DMX_S_UNCORE = 2'd0, 
DMX_S_DUMMY = 2'd1, 
DMX_S_TCM = 2'd2 
} bmu_demux_id_sbus_e; 
localparam BUS_MUX_N = 2; 
localparam BUS_MUX_ID_W = $clog2(BUS_MUX_N); 
localparam BUS_SOC_ID_W = BUS_MUX_ID_W + 1; 
typedef enum logic { 
MUX_IBUS_FT = 1'b0, 
MUX_IBUS_LDST = 1'b1 
} bmu_mux_id_ibus_e; 
typedef enum logic { 
MUX_LS_LDST = 1'b0, 
MUX_LS_FT = 1'b1 
} bmu_mux_id_lse; 
localparam MUX_ID_SBUS = 1'b1; 
localparam UC_MUX_N = 1; 
localparam UC_MUX_ID_W = 1; 
localparam UC_ALL_ID_W = UC_MUX_ID_W + 1; 
localparam UC_DEMUX_N = 6; 
typedef enum logic [2:0] { 
DMX_UC_DUMMY = 3'd0, 
DMX_UC_CLIC = 3'd1, 
DMX_UC_TRACE = 3'd2, 
DMX_UC_RERI = 3'd3, 
DMX_UC_HPU = 3'd4, 
DMX_UC_MSS = 3'd5 
} bmu_demux_id_uncore_e; 
typedef struct packed { 
logic [31:0] addr ; 
logic [ 1:0] size ; 
logic write; 
logic [ 3:0] wstrb; 
logic [31:0] wdata; 
logic [1-1:0] id; 
} wxbl_req_info_mmr_t; 
typedef struct packed { 
logic [31:0] rdata; 
logic err ; 
} wxbl_rsp_info_mmr_t; 
localparam WSTRB_SPLIT_MODE = 1; 
localparam FT_DEMUX_KEEP_ORDER = (REGSLICE_MODE_FEEDTHROUGH==REGSLICE_MODE_FEEDTHROUGH) ? 1 : 0; 
localparam LDST_DEMUX_KEEP_ORDER = (REGSLICE_MODE_FORWARD ==REGSLICE_MODE_FEEDTHROUGH) ? 1 : 0; 
localparam RESET_ALL_REG = 0; 
function automatic wxbl_req_info_t wxbl_req_info_ro2rw (input wxbl_req_info_ro_t req_ro_info); 
wxbl_req_info_t req_rw_info; 
req_rw_info.addr = req_ro_info.addr; 
req_rw_info.size = req_ro_info.size; 
req_rw_info.write = 1'b0; 
req_rw_info.wstrb = 4'b0; 
req_rw_info.prot = req_ro_info.prot ; 
req_rw_info.memattr = req_ro_info.memattr; 
req_rw_info.id = req_ro_info.id ; 
req_rw_info.master = req_ro_info.master ; 
return req_rw_info; 
endfunction 
logic [FT_DEMUX_N-1:0] dcd_ft_sel; 
logic [LDST_DEMUX_N-1:0] dcd_ldst_sel; 
logic ft_wxbl_req_vld ; 
logic ft_wxbl_req_rdy ; 
wxbl_req_info_ro_t ft_wxbl_req_info; 
logic ft_wxbl_rsp_vld ; 
wxbl_rsp_info_t ft_wxbl_rsp_info; 
logic ft_demux_slv_req_vld ; 
logic ft_demux_slv_req_rdy ; 
wxbl_req_info_ro_t ft_demux_slv_req_info; 
logic ft_demux_slv_rsp_vld ; 
wxbl_rsp_info_t ft_demux_slv_rsp_info; 
logic [FT_DEMUX_N-1:0] ft_demux_mst_req_vld ; 
logic [FT_DEMUX_N-1:0] ft_demux_mst_req_rdy ; 
logic [FT_DEMUX_N-1:0] ft_demux_mst_rsp_vld ; 
wxbl_rsp_info_t [FT_DEMUX_N-1:0] ft_demux_mst_rsp_info; 
logic ldst_wxbl_req_vld ; 
logic ldst_wxbl_req_rdy ; 
wxbl_req_info_t ldst_wxbl_req_info; 
logic ldst_wxbl_rsp_vld ; 
wxbl_rsp_info_t ldst_wxbl_rsp_info; 
logic ldst_demux_slv_req_vld ; 
logic ldst_demux_slv_req_rdy ; 
wxbl_req_info_t ldst_demux_slv_req_info; 
logic ldst_demux_slv_rsp_vld ; 
wxbl_rsp_info_t ldst_demux_slv_rsp_info; 
logic [LDST_DEMUX_N-1:0] ldst_demux_mst_req_vld ; 
logic [LDST_DEMUX_N-1:0] ldst_demux_mst_req_rdy ; 
logic [LDST_DEMUX_N-1:0] ldst_demux_mst_rsp_vld ; 
wxbl_rsp_info_t [LDST_DEMUX_N-1:0] ldst_demux_mst_rsp_info; 
logic dbus_ppln_wdata_en; 
logic mbus_ppln_wdata_en; 
logic bus_ppln_wdata_en; 
logic dff_st_wdata_en; 
logic [31:0] dff_st_wdata_d; 
logic [31:0] dff_st_wdata_q; 
logic [31:0] nonahb_wdata; 
logic [UC_MUX_N-1:0] uncore_mux_slv_req_vld ; 
logic [UC_MUX_N-1:0] uncore_mux_slv_req_rdy ; 
wxbl_req_info_mmr_t [UC_MUX_N-1:0] uncore_mux_slv_req_info; 
logic [UC_MUX_N-1:0] uncore_mux_slv_rsp_vld ; 
wxbl_rsp_info_t [UC_MUX_N-1:0] uncore_mux_slv_rsp_info; 
logic uncore_mux_mst_req_vld ; 
logic uncore_mux_mst_req_rdy ; 
wxbl_req_info_mmr_t uncore_mux_mst_req_info; 
logic uncore_mux_mst_rsp_vld ; 
wxbl_rsp_info_t uncore_mux_mst_rsp_info; 
logic [UC_MUX_ID_W-1:0] uncore_mux_mst_req_id; 
logic [UC_MUX_ID_W-1:0] uncore_mux_mst_rsp_id; 
logic [UC_ALL_ID_W-1:0] uncore_mmr_req_id; 
logic [UC_ALL_ID_W-1:0] uncore_mmr_rsp_id; 
wxbl_rsp_info_mmr_t uncore_regslice_mst_rsp_info; 
logic uncore_demux_slv_req_vld ; 
logic uncore_demux_slv_req_rdy ; 
wxbl_req_info_mmr_t uncore_demux_slv_req_info; 
logic uncore_demux_slv_rsp_vld ; 
wxbl_rsp_info_mmr_t uncore_demux_slv_rsp_info; 
logic [UC_DEMUX_N-1:0] uncore_demux_mst_req_vld ; 
logic [UC_DEMUX_N-1:0] uncore_demux_mst_req_rdy ; 
logic [UC_DEMUX_N-1:0] uncore_demux_mst_rsp_vld ; 
wxbl_rsp_info_mmr_t [UC_DEMUX_N-1:0] uncore_demux_mst_rsp_info; 
logic uncore_sel_clic ; 
logic uncore_sel_clint; 
logic [UC_DEMUX_N-1:0] dcd_uncore_sel; 
logic uncore_access_err; 
logic ibus_bridge_req_vld ; 
logic ibus_bridge_req_rdy ; 
wxbl_req_info_t ibus_bridge_req_info; 
logic ibus_bridge_rsp_vld ; 
wxbl_rsp_info_t ibus_bridge_rsp_info; 
logic [BUS_SOC_ID_W-1:0] ibus_bridge_req_id; 
logic [BUS_SOC_ID_W-1:0] ibus_bridge_rsp_id; 
logic [32-1:0] i_bus_pol_hwdata; 
logic [32-1:0] i_bus_pol_hrdata; 
logic [32-1:0] ibus_ppln_slv_haddr ; 
logic [2:0] ibus_ppln_slv_hburst ; 
logic ibus_ppln_slv_hmastlock ; 
logic [3:0] ibus_ppln_slv_hprot ; 
logic [2:0] ibus_ppln_slv_hsize ; 
logic [1:0] ibus_ppln_slv_hmaster ; 
logic [1:0] ibus_ppln_slv_htrans ; 
logic ibus_ppln_slv_hwrite ; 
logic [32-1:0] ibus_ppln_slv_hwdata ; 
logic [32-1:0] ibus_ppln_slv_hrdata ; 
logic ibus_ppln_slv_hready ; 
logic ibus_ppln_slv_hresp ; 
logic [32-1:0] ibus_ppln_mst_haddr ; 
logic [2:0] ibus_ppln_mst_hburst ; 
logic ibus_ppln_mst_hmastlock ; 
logic [3:0] ibus_ppln_mst_hprot ; 
logic [2:0] ibus_ppln_mst_hsize ; 
logic [1:0] ibus_ppln_mst_hmaster ; 
logic [1:0] ibus_ppln_mst_htrans ; 
logic ibus_ppln_mst_hwrite ; 
logic [32-1:0] ibus_ppln_mst_hwdata ; 
logic [32-1:0] ibus_ppln_mst_hrdata ; 
logic ibus_ppln_mst_hready ; 
logic ibus_ppln_mst_hresp ; 
logic dbus_bridge_req_vld ; 
logic dbus_bridge_req_rdy ; 
wxbl_req_info_t dbus_bridge_req_info; 
logic dbus_bridge_rsp_vld ; 
wxbl_rsp_info_t dbus_bridge_rsp_info; 
logic [BUS_SOC_ID_W-1:0] dbus_bridge_req_id; 
logic [BUS_SOC_ID_W-1:0] dbus_bridge_rsp_id; 
logic [32-1:0] d_bus_pol_hwdata; 
logic [32-1:0] d_bus_pol_hrdata; 
logic [32-1:0] dbus_ppln_slv_haddr ; 
logic [2:0] dbus_ppln_slv_hburst ; 
logic dbus_ppln_slv_hmastlock ; 
logic [3:0] dbus_ppln_slv_hprot ; 
logic [2:0] dbus_ppln_slv_hsize ; 
logic [1:0] dbus_ppln_slv_hmaster ; 
logic [1:0] dbus_ppln_slv_htrans ; 
logic dbus_ppln_slv_hwrite ; 
logic [32-1:0] dbus_ppln_slv_hwdata ; 
logic [32-1:0] dbus_ppln_slv_hrdata ; 
logic dbus_ppln_slv_hready ; 
logic dbus_ppln_slv_hresp ; 
logic [32-1:0] d_bus_hwdata_tmp ; 
logic[1:0] d_addr_offset_q ; 
logic[1:0] d_bus_ahb_hsize_q; 
logic d_wdata_updt_en ; 
logic [BUS_MUX_N-1:0] mbus_mux_slv_req_vld ; 
logic [BUS_MUX_N-1:0] mbus_mux_slv_req_rdy ; 
wxbl_req_info_t [BUS_MUX_N-1:0] mbus_mux_slv_req_info; 
logic [BUS_MUX_N-1:0] mbus_mux_slv_rsp_vld ; 
wxbl_rsp_info_t [BUS_MUX_N-1:0] mbus_mux_slv_rsp_info; 
logic mbus_mux_mst_req_vld ; 
logic mbus_mux_mst_req_rdy ; 
wxbl_req_info_t mbus_mux_mst_req_info; 
logic mbus_mux_mst_rsp_vld ; 
wxbl_rsp_info_t mbus_mux_mst_rsp_info; 
logic [BUS_MUX_ID_W-1:0] mbus_mux_mst_req_id; 
logic [BUS_MUX_ID_W-1:0] mbus_mux_mst_rsp_id; 
logic [BUS_SOC_ID_W-1:0] mbus_bridge_req_id; 
logic [BUS_SOC_ID_W-1:0] mbus_bridge_rsp_id; 
logic [32-1:0] m_bus_pol_hwdata; 
logic [32-1:0] m_bus_pol_hrdata; 
logic [32-1:0] m_haddr ; 
logic [2:0] m_hburst ; 
logic m_hmastlock; 
logic [3:0] m_hprot ; 
logic [2:0] m_hsize ; 
logic [1:0] m_hmaster ; 
logic [1:0] m_htrans ; 
logic m_hwrite ; 
logic [32-1:0] m_hwdata ; 
logic [32-1:0] m_hrdata ; 
logic m_hready ; 
logic m_hresp ; 
logic [32-1:0] mbus_ppln_slv_haddr ; 
logic [2:0] mbus_ppln_slv_hburst ; 
logic mbus_ppln_slv_hmastlock ; 
logic [3:0] mbus_ppln_slv_hprot ; 
logic [2:0] mbus_ppln_slv_hsize ; 
logic [1:0] mbus_ppln_slv_hmaster ; 
logic [1:0] mbus_ppln_slv_htrans ; 
logic mbus_ppln_slv_hwrite ; 
logic [32-1:0] mbus_ppln_slv_hwdata ; 
logic [32-1:0] mbus_ppln_slv_hrdata ; 
logic mbus_ppln_slv_hready ; 
logic mbus_ppln_slv_hresp ; 
logic [32-1:0] m_bus_hwdata_tmp ; 
logic[1:0] m_addr_offset_q ; 
logic[1:0] m_bus_ahb_hsize_q; 
logic m_wdata_updt_en ; 
logic regslice_busy_ft; 
logic regslice_busy_ldst; 
logic regslice_busy_ibus; 
logic regslice_busy_dbus; 
logic regslice_busy_mbus; 
assign ft_wxbl_req_vld = icache_bmu_ar_valid; 
assign bmu_icache_ar_ready = ft_wxbl_req_rdy; 
assign ft_wxbl_req_info.addr = icache_bmu_ar_addr; 
assign ft_wxbl_req_info.size = icache_bmu_ar_size[1:0]; 
assign ft_wxbl_req_info.master = {1'b1, icache_bmu_ar_user}; 
assign ft_wxbl_req_info.prot[1] = icache_bmu_ar_prot[0]; 
assign ft_wxbl_req_info.prot[0] = ~icache_bmu_ar_prot[2]; 
assign ft_wxbl_req_info.memattr[1] = (icache_bmu_ar_cache == 4'b1111); 
assign ft_wxbl_req_info.memattr[0] = 1'b1; 
assign ft_wxbl_req_info.id = icache_bmu_ar_id; 
assign bmu_icache_r_valid = ft_wxbl_rsp_vld; 
assign bmu_icache_r_data = ft_wxbl_rsp_info.rdata; 
assign bmu_icache_r_id = ft_wxbl_rsp_info.id; 
assign bmu_icache_r_last = 1'b1; 
assign bmu_icache_r_resp = {ft_wxbl_rsp_info.err, 1'b0}; 
assign ldst_wxbl_req_vld = dcache_bmu_req_vld; 
assign ldst_wxbl_req_info.addr = dcache_bmu_req_addr; 
assign ldst_wxbl_req_info.size = dcache_bmu_req_size; 
assign ldst_wxbl_req_info.write = dcache_bmu_req_write; 
assign ldst_wxbl_req_info.wstrb = dcache_bmu_req_wstrb; 
assign ldst_wxbl_req_info.id = dcache_bmu_req_id; 
assign ldst_wxbl_req_info.prot = dcache_bmu_req_prot; 
assign ldst_wxbl_req_info.memattr = dcache_bmu_req_memattr; 
assign ldst_wxbl_req_info.master = {1'b0, dcache_bmu_req_dm}; 
assign bmu_dcache_req_rdy = ldst_wxbl_req_rdy; 
assign bmu_dcache_rsp_vld = ldst_wxbl_rsp_vld; 
assign bmu_dcache_rsp_data = ldst_wxbl_rsp_info.rdata; 
assign bmu_dcache_rsp_id = ldst_wxbl_rsp_info.id; 
assign bmu_dcache_rsp_err = ldst_wxbl_rsp_info.err; 
if (REGSLICE_MODE_FEEDTHROUGH != REGSLICE_MODE_FEEDTHROUGH) begin : fetch_regslice 
wxblite_regslice #( 
.req_t ( wxbl_req_info_ro_t ), 
.rsp_t ( wxbl_rsp_info_t ), 
.MODE ( REGSLICE_MODE_FEEDTHROUGH ) 
) u_wxblite_regslice_ft ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_req_vld_i ( ft_wxbl_req_vld ), 
.slv_req_rdy_o ( ft_wxbl_req_rdy ), 
.slv_req_info_i ( ft_wxbl_req_info ), 
.slv_rsp_vld_o ( ft_wxbl_rsp_vld ), 
.slv_rsp_info_o ( ft_wxbl_rsp_info ), 
.mst_req_vld_o ( ft_demux_slv_req_vld ), 
.mst_req_rdy_i ( ft_demux_slv_req_rdy ), 
.mst_req_info_o ( ft_demux_slv_req_info ), 
.mst_rsp_vld_i ( ft_demux_slv_rsp_vld ), 
.mst_rsp_info_i ( ft_demux_slv_rsp_info ) 
); 
assign regslice_busy_ft = ft_demux_slv_req_vld; 
end else begin : fetch_no_regslice 
assign ft_demux_slv_req_vld = ft_wxbl_req_vld ; 
assign ft_demux_slv_req_info = ft_wxbl_req_info; 
assign ft_wxbl_req_rdy = ft_demux_slv_req_rdy ; 
assign ft_wxbl_rsp_vld = ft_demux_slv_rsp_vld ; 
assign ft_wxbl_rsp_info = ft_demux_slv_rsp_info; 
assign regslice_busy_ft = 1'b0; 
end 
if (REGSLICE_MODE_FORWARD != REGSLICE_MODE_FEEDTHROUGH) begin : ldst_regslice 
wxblite_regslice #( 
.req_t ( wxbl_req_info_t ), 
.rsp_t ( wxbl_rsp_info_t ), 
.MODE ( REGSLICE_MODE_FORWARD ) 
) u_wxblite_regslice_ldst ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_req_vld_i ( ldst_wxbl_req_vld ), 
.slv_req_rdy_o ( ldst_wxbl_req_rdy ), 
.slv_req_info_i ( ldst_wxbl_req_info ), 
.slv_rsp_vld_o ( ldst_wxbl_rsp_vld ), 
.slv_rsp_info_o ( ldst_wxbl_rsp_info ), 
.mst_req_vld_o ( ldst_demux_slv_req_vld ), 
.mst_req_rdy_i ( ldst_demux_slv_req_rdy ), 
.mst_req_info_o ( ldst_demux_slv_req_info ), 
.mst_rsp_vld_i ( ldst_demux_slv_rsp_vld ), 
.mst_rsp_info_i ( ldst_demux_slv_rsp_info ) 
); 
assign nonahb_wdata = dff_st_wdata_q; 
assign regslice_busy_ldst = ldst_demux_slv_req_vld; 
end else begin : ldst_no_regslice 
assign ldst_demux_slv_req_vld = ldst_wxbl_req_vld ; 
assign ldst_demux_slv_req_info = ldst_wxbl_req_info ; 
assign ldst_wxbl_req_rdy = ldst_demux_slv_req_rdy ; 
assign ldst_wxbl_rsp_vld = ldst_demux_slv_rsp_vld ; 
assign ldst_wxbl_rsp_info = ldst_demux_slv_rsp_info; 
assign nonahb_wdata = dff_st_wdata_d; 
assign regslice_busy_ldst = 1'b0; 
end 
if (WSTRB_SPLIT_MODE == 1) begin : direct_take_wdata 
assign dff_st_wdata_en = dcache_bmu_req_vld & bmu_dcache_req_rdy & dcache_bmu_req_write; 
end else begin : indirect_take_wdata 
assign bus_ppln_wdata_en = dbus_ppln_wdata_en | mbus_ppln_wdata_en; 
assign dff_st_wdata_en = dcache_bmu_req_vld & dcache_bmu_req_write & bus_ppln_wdata_en; 
end 
assign dff_st_wdata_d = dcache_bmu_req_wdata; 
`WDFFER(dff_st_wdata_q, dff_st_wdata_d, dff_st_wdata_en, clk, rst_n) 
assign dcd_ldst_sel[DMX_LS_UNCORE] = (ldst_demux_slv_req_info.addr[31:18] == 
core_mmr_base_addr[31:18]); 
if (32'h1000_0000==32'h0000_0000 && 32'h1FFF_FFFF==32'h1FFF_FFFF) begin: dmx_ft_ibus 
assign dcd_ft_sel[DMX_FT_IBUS] = ~|ft_demux_slv_req_info.addr[31:29]; 
end else begin : dmx_ft_ibus_notfix 
assign dcd_ft_sel[DMX_FT_IBUS] = (ft_demux_slv_req_info.addr >= 32'h1000_0000) 
& (ft_demux_slv_req_info.addr <= 32'h1FFF_FFFF ); 
end 
assign dcd_ldst_sel[DMX_LS_IBUS] = (ldst_demux_slv_req_info.addr >= 32'h1000_0000) 
& (ldst_demux_slv_req_info.addr <= 32'h1FFF_FFFF ) 
& ~dcd_ldst_sel[DMX_LS_UNCORE]; 
if (32'h2000_0000==32'h0000_0000 && 32'h2FFF_FFFF==32'h1FFF_FFFF) begin : dmx_ls_dbus 
assign dcd_ldst_sel[DMX_LS_DBUS] = ~|ldst_demux_slv_req_info.addr[31:29] 
& ~dcd_ldst_sel[DMX_LS_UNCORE]; 
end else begin : dmx_ls_dbus_notfix 
assign dcd_ldst_sel[DMX_LS_DBUS] = (ldst_demux_slv_req_info.addr >= 32'h2000_0000) 
& (ldst_demux_slv_req_info.addr <= 32'h2FFF_FFFF ) 
& ~dcd_ldst_sel[DMX_LS_UNCORE]; 
end 
assign dcd_ft_sel[DMX_FT_DBUS] = (ft_demux_slv_req_info.addr >= 32'h2000_0000) 
& (ft_demux_slv_req_info.addr <= 32'h2FFF_FFFF ); 
if (32'h3000_0000==32'h2000_0000 && 32'hFFFF_FFFF==32'hFFFF_FFFF) begin: dmx_ft_mbus 
assign dcd_ft_sel[DMX_FT_MBUS] = |ft_demux_slv_req_info.addr[31:29]; 
end else begin :dmx_ft_mbus_notfix 
assign dcd_ft_sel[DMX_FT_MBUS] = (ft_demux_slv_req_info.addr >= 32'h3000_0000) 
& (ft_demux_slv_req_info.addr <= 32'hFFFF_FFFF ); 
end 
if (32'h3000_0000==32'h2000_0000 && 32'hFFFF_FFFF==32'hFFFF_FFFF) begin: dmux_ls_mbus 
assign dcd_ldst_sel[DMX_LS_MBUS] = |ldst_demux_slv_req_info.addr[31:29] 
& ~dcd_ldst_sel[DMX_LS_UNCORE]; 
end else begin : dmux_ls_mbus_notfix 
assign dcd_ldst_sel[DMX_LS_MBUS] = (ldst_demux_slv_req_info.addr >= 32'h3000_0000) 
& (ldst_demux_slv_req_info.addr <= 32'hFFFF_FFFF ) 
& ~dcd_ldst_sel[DMX_LS_UNCORE]; 
end 
assign dcd_ldst_sel[DMX_LS_PBUS] = 1'b0; 
assign dcd_ft_sel [DMX_FT_DUMMY] = ~|dcd_ft_sel[FT_DEMUX_N-2:0]; 
assign dcd_ldst_sel[DMX_LS_DUMMY] = ~|dcd_ldst_sel[LDST_DEMUX_N-2:0]; 
wxblite_demux_1xn #( 
.rsp_t ( wxbl_rsp_info_t ), 
.MST_PORTS_N ( FT_DEMUX_N ), 
.EN_FLUSH ( 0 ), 
.EN_KEEP_ORDER ( FT_DEMUX_KEEP_ORDER ) 
) u_wxbl_demux_1xn_ft ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( ft_demux_slv_req_vld ), 
.slv_req_rdy_o ( ft_demux_slv_req_rdy ), 
.slv_rsp_vld_o ( ft_demux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( 1'b1 ), 
.slv_rsp_info_o ( ft_demux_slv_rsp_info ), 
.sel_i ( dcd_ft_sel ), 
.mst_req_vld_o ( ft_demux_mst_req_vld ), 
.mst_req_rdy_i ( ft_demux_mst_req_rdy ), 
.mst_rsp_vld_i ( ft_demux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( ft_demux_mst_rsp_info ) 
); 
wxblite_demux_1xn #( 
.rsp_t ( wxbl_rsp_info_t ), 
.MST_PORTS_N ( LDST_DEMUX_N ), 
.EN_FLUSH ( 0 ), 
.EN_KEEP_ORDER ( LDST_DEMUX_KEEP_ORDER ) 
) u_wxbl_demux_1xn_ldst ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( ldst_demux_slv_req_vld ), 
.slv_req_rdy_o ( ldst_demux_slv_req_rdy ), 
.slv_rsp_vld_o ( ldst_demux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( 1'b1 ), 
.slv_rsp_info_o ( ldst_demux_slv_rsp_info ), 
.sel_i ( dcd_ldst_sel ), 
.mst_req_vld_o ( ldst_demux_mst_req_vld ), 
.mst_req_rdy_i ( ldst_demux_mst_req_rdy ), 
.mst_rsp_vld_i ( ldst_demux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( ldst_demux_mst_rsp_info ) 
); 
localparam logic IBUS_EN_WSTRB_SPLIT = 1; 
logic [BUS_MUX_N-1:0] ibus_mux_slv_req_vld ; 
logic [BUS_MUX_N-1:0] ibus_mux_slv_req_rdy ; 
wxbl_req_info_t [BUS_MUX_N-1:0] ibus_mux_slv_req_info; 
logic [BUS_MUX_N-1:0] ibus_mux_slv_rsp_vld ; 
wxbl_rsp_info_t [BUS_MUX_N-1:0] ibus_mux_slv_rsp_info; 
logic [BUS_MUX_ID_W-1:0] ibus_mux_mst_req_id; 
logic [BUS_MUX_ID_W-1:0] ibus_mux_mst_rsp_id; 
assign ibus_mux_slv_req_vld [MUX_IBUS_FT] = ft_demux_mst_req_vld[DMX_FT_IBUS]; 
assign ibus_mux_slv_req_info[MUX_IBUS_FT] = wxbl_req_info_ro2rw(ft_demux_slv_req_info); 
assign ft_demux_mst_req_rdy [DMX_FT_IBUS] = ibus_mux_slv_req_rdy [MUX_IBUS_FT]; 
assign ft_demux_mst_rsp_vld [DMX_FT_IBUS] = ibus_mux_slv_rsp_vld [MUX_IBUS_FT]; 
assign ft_demux_mst_rsp_info[DMX_FT_IBUS] = ibus_mux_slv_rsp_info[MUX_IBUS_FT]; 
assign ibus_mux_slv_req_vld [MUX_IBUS_LDST] = ldst_demux_mst_req_vld[DMX_LS_IBUS]; 
assign ibus_mux_slv_req_info[MUX_IBUS_LDST] = ldst_demux_slv_req_info; 
assign ldst_demux_mst_req_rdy [DMX_LS_IBUS] = ibus_mux_slv_req_rdy [MUX_IBUS_LDST]; 
assign ldst_demux_mst_rsp_vld [DMX_LS_IBUS] = ibus_mux_slv_rsp_vld [MUX_IBUS_LDST]; 
assign ldst_demux_mst_rsp_info[DMX_LS_IBUS] = ibus_mux_slv_rsp_info[MUX_IBUS_LDST]; 
wxblite_mux_nx1 #( 
.req_t ( wxbl_req_info_t ), 
.rsp_t ( wxbl_rsp_info_t ), 
.SLV_PORTS_N ( BUS_MUX_N ), 
.ARB_MODE ( 0 ), 
.LOCK ( 1 ), 
.EN_FLUSH ( 0 ) 
) u_wxbl_mux_nx1_ibus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( ibus_mux_slv_req_vld ), 
.slv_req_rdy_o ( ibus_mux_slv_req_rdy ), 
.slv_req_info_i ( ibus_mux_slv_req_info ), 
.slv_rsp_vld_o ( ibus_mux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( {BUS_MUX_N{1'b1}} ), 
.slv_rsp_info_o ( ibus_mux_slv_rsp_info ), 
.mst_req_vld_o ( ibus_bridge_req_vld ), 
.mst_req_rdy_i ( ibus_bridge_req_rdy ), 
.mst_req_info_o ( ibus_bridge_req_info ), 
.mst_req_id_o ( ibus_mux_mst_req_id ), 
.mst_rsp_vld_i ( ibus_bridge_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( ibus_bridge_rsp_info ), 
.mst_rsp_id_i ( ibus_mux_mst_rsp_id ) 
); 
assign ibus_bridge_req_id = {ibus_mux_mst_req_id, ibus_bridge_req_info.id}; 
assign {ibus_mux_mst_rsp_id, ibus_bridge_rsp_info.id} = ibus_bridge_rsp_id; 
wxblite_ahblite_bridge #( 
.ADDR_W ( 32 ), 
.DATA_W ( 32 ), 
.PPLN_WDATA ( 0 ), 
.EN_WSTRB_SPLIT ( IBUS_EN_WSTRB_SPLIT ), 
.WSTRB_SPLIT_MODE ( WSTRB_SPLIT_MODE ), 
.EN_ID ( 1 ), 
.ID_W ( BUS_SOC_ID_W ), 
.MASTER_W ( 2 ), 
.user_t ( logic ) 
) u_wxbl_ahbl_bridge_ibus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.ppln_wdata_en ( ), 
.wxbl_req_vld ( ibus_bridge_req_vld ), 
.wxbl_req_rdy ( ibus_bridge_req_rdy ), 
.wxbl_req_addr ( ibus_bridge_req_info.addr ), 
.wxbl_req_size ( ibus_bridge_req_info.size ), 
.wxbl_req_write ( ibus_bridge_req_info.write ), 
.wxbl_req_wdata ( dff_st_wdata_q ), 
.wxbl_req_wstrb ( ibus_bridge_req_info.wstrb ), 
.wxbl_req_prot ( ibus_bridge_req_info.prot ), 
.wxbl_req_memattr ( ibus_bridge_req_info.memattr ), 
.wxbl_req_master ( ibus_bridge_req_info.master ), 
.wxbl_req_id ( ibus_bridge_req_id ), 
.wxbl_req_user ( 1'b0 ), 
.wxbl_rsp_vld ( ibus_bridge_rsp_vld ), 
.wxbl_rsp_err ( ibus_bridge_rsp_info.err ), 
.wxbl_rsp_rdata ( ibus_bridge_rsp_info.rdata ), 
.wxbl_rsp_id ( ibus_bridge_rsp_id ), 
.ahb_haddr ( ibus_ppln_slv_haddr ), 
.ahb_hburst ( ibus_ppln_slv_hburst ), 
.ahb_hmastlock ( ibus_ppln_slv_hmastlock ), 
.ahb_hprot ( ibus_ppln_slv_hprot ), 
.ahb_hsize ( ibus_ppln_slv_hsize ), 
.ahb_master ( ibus_ppln_slv_hmaster ), 
.ahb_htrans ( ibus_ppln_slv_htrans ), 
.ahb_hwrite ( ibus_ppln_slv_hwrite ), 
.ahb_hwdata ( ibus_ppln_slv_hwdata ), 
.ahb_user ( ), 
.ahb_hrdata ( ibus_ppln_slv_hrdata ), 
.ahb_hready ( ibus_ppln_slv_hready ), 
.ahb_hresp ( ibus_ppln_slv_hresp ) 
); 
ahb_regslice #( 
.ADDR_W ( 32 ), 
.DATA_W ( 32 ), 
.MODE ( REGSLICE_MODE_FEEDTHROUGH ), 
.EN_HBURST ( 0 ), 
.EN_HMLOCK ( 0 ), 
.EN_MISC ( 0 ), 
.misc_t ( logic ) 
) u_ahb_regslice_ibus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_haddr ( ibus_ppln_slv_haddr ), 
.slv_hburst ( ibus_ppln_slv_hburst ), 
.slv_hmastlock ( ibus_ppln_slv_hmastlock ), 
.slv_hprot ( ibus_ppln_slv_hprot ), 
.slv_hsize ( ibus_ppln_slv_hsize ), 
.slv_hmaster ( ibus_ppln_slv_hmaster ), 
.slv_htrans ( ibus_ppln_slv_htrans ), 
.slv_misc ( 1'b0 ), 
.slv_hwrite ( ibus_ppln_slv_hwrite ), 
.slv_hwdata ( ibus_ppln_slv_hwdata ), 
.slv_hrdata ( ibus_ppln_slv_hrdata ), 
.slv_hready ( ibus_ppln_slv_hready ), 
.slv_hresp ( ibus_ppln_slv_hresp ), 
.mst_haddr ( ibus_ppln_mst_haddr ), 
.mst_hburst ( ibus_ppln_mst_hburst ), 
.mst_hmastlock ( ibus_ppln_mst_hmastlock ), 
.mst_hprot ( ibus_ppln_mst_hprot ), 
.mst_hsize ( ibus_ppln_mst_hsize ), 
.mst_hmaster ( ibus_ppln_mst_hmaster ), 
.mst_htrans ( ibus_ppln_mst_htrans ), 
.mst_misc ( ), 
.mst_hwrite ( ibus_ppln_mst_hwrite ), 
.mst_hwdata ( ibus_ppln_mst_hwdata ), 
.mst_hrdata ( ibus_ppln_mst_hrdata ), 
.mst_hready ( ibus_ppln_mst_hready ), 
.mst_hresp ( ibus_ppln_mst_hresp ) 
); 
if (REGSLICE_MODE_FEEDTHROUGH != REGSLICE_MODE_FEEDTHROUGH) begin : ibus_regslice 
assign regslice_busy_ibus = ibus_ppln_mst_htrans[1]; 
end else begin : ibus_no_regslice 
assign regslice_busy_ibus = 1'b0; 
end 
assign i_bus_ahb_haddr = ibus_ppln_mst_haddr ; 
assign i_bus_ahb_hburst = ibus_ppln_mst_hburst ; 
assign i_bus_ahb_hmastlock = ibus_ppln_mst_hmastlock ; 
assign i_bus_ahb_hprot = ibus_ppln_mst_hprot ; 
assign i_bus_ahb_hsize = ibus_ppln_mst_hsize ; 
assign i_bus_ahb_hmaster = ibus_ppln_mst_hmaster ; 
assign i_bus_ahb_htrans = ibus_ppln_mst_htrans ; 
assign i_bus_ahb_hwrite = ibus_ppln_mst_hwrite ; 
assign i_bus_pol_hwdata = ibus_ppln_mst_hwdata; 
assign ibus_ppln_mst_hrdata = i_bus_pol_hrdata; 
assign ibus_ppln_mst_hready = i_bus_ahb_hready; 
assign ibus_ppln_mst_hresp = i_bus_ahb_hresp ; 
assign i_bus_ahb_hwdata = i_bus_pol_hwdata; 
assign i_bus_pol_hrdata = i_bus_ahb_hrdata; 
logic [BUS_MUX_N-1:0] dbus_mux_slv_req_vld ; 
logic [BUS_MUX_N-1:0] dbus_mux_slv_req_rdy ; 
wxbl_req_info_t [BUS_MUX_N-1:0] dbus_mux_slv_req_info; 
logic [BUS_MUX_N-1:0] dbus_mux_slv_rsp_vld ; 
wxbl_rsp_info_t [BUS_MUX_N-1:0] dbus_mux_slv_rsp_info; 
logic [BUS_MUX_ID_W-1:0] dbus_mux_mst_req_id; 
logic [BUS_MUX_ID_W-1:0] dbus_mux_mst_rsp_id; 
assign dbus_mux_slv_req_vld [MUX_LS_FT] = ft_demux_mst_req_vld[DMX_FT_DBUS]; 
assign dbus_mux_slv_req_info[MUX_LS_FT] = wxbl_req_info_ro2rw(ft_demux_slv_req_info); 
assign ft_demux_mst_req_rdy [DMX_FT_DBUS] = dbus_mux_slv_req_rdy [MUX_LS_FT]; 
assign ft_demux_mst_rsp_vld [DMX_FT_DBUS] = dbus_mux_slv_rsp_vld [MUX_LS_FT]; 
assign ft_demux_mst_rsp_info[DMX_FT_DBUS] = dbus_mux_slv_rsp_info[MUX_LS_FT]; 
assign dbus_mux_slv_req_vld [MUX_LS_LDST] = ldst_demux_mst_req_vld[DMX_LS_DBUS]; 
assign dbus_mux_slv_req_info[MUX_LS_LDST] = ldst_demux_slv_req_info; 
assign ldst_demux_mst_req_rdy [DMX_LS_DBUS] = dbus_mux_slv_req_rdy [MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_vld [DMX_LS_DBUS] = dbus_mux_slv_rsp_vld [MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_info[DMX_LS_DBUS] = dbus_mux_slv_rsp_info[MUX_LS_LDST]; 
wxblite_mux_nx1 #( 
.req_t ( wxbl_req_info_t ), 
.rsp_t ( wxbl_rsp_info_t ), 
.SLV_PORTS_N ( BUS_MUX_N ), 
.ARB_MODE ( 0 ), 
.LOCK ( 1 ), 
.EN_FLUSH ( 0 ) 
) u_wxbl_mux_nx1_dbus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( dbus_mux_slv_req_vld ), 
.slv_req_rdy_o ( dbus_mux_slv_req_rdy ), 
.slv_req_info_i ( dbus_mux_slv_req_info ), 
.slv_rsp_vld_o ( dbus_mux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( {BUS_MUX_N{1'b1}} ), 
.slv_rsp_info_o ( dbus_mux_slv_rsp_info ), 
.mst_req_vld_o ( dbus_bridge_req_vld ), 
.mst_req_rdy_i ( dbus_bridge_req_rdy ), 
.mst_req_info_o ( dbus_bridge_req_info ), 
.mst_req_id_o ( dbus_mux_mst_req_id ), 
.mst_rsp_vld_i ( dbus_bridge_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( dbus_bridge_rsp_info ), 
.mst_rsp_id_i ( dbus_mux_mst_rsp_id ) 
); 
assign dbus_bridge_req_id = {dbus_mux_mst_req_id, dbus_bridge_req_info.id}; 
assign {dbus_mux_mst_rsp_id, dbus_bridge_rsp_info.id} = dbus_bridge_rsp_id; 
wxblite_ahblite_bridge #( 
.ADDR_W ( 32 ), 
.DATA_W ( 32 ), 
.PPLN_WDATA ( 0 ), 
.EN_WSTRB_SPLIT ( 1 ), 
.WSTRB_SPLIT_MODE ( WSTRB_SPLIT_MODE ), 
.EN_ID ( 1 ), 
.ID_W ( BUS_SOC_ID_W ), 
.MASTER_W ( 2 ), 
.user_t ( logic ) 
) u_wxbl_ahbl_bridge_dbus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.ppln_wdata_en ( dbus_ppln_wdata_en ), 
.wxbl_req_vld ( dbus_bridge_req_vld ), 
.wxbl_req_rdy ( dbus_bridge_req_rdy ), 
.wxbl_req_addr ( dbus_bridge_req_info.addr ), 
.wxbl_req_size ( dbus_bridge_req_info.size ), 
.wxbl_req_write ( dbus_bridge_req_info.write ), 
.wxbl_req_wdata ( dff_st_wdata_q ), 
.wxbl_req_wstrb ( dbus_bridge_req_info.wstrb ), 
.wxbl_req_prot ( dbus_bridge_req_info.prot ), 
.wxbl_req_memattr ( dbus_bridge_req_info.memattr ), 
.wxbl_req_master ( dbus_bridge_req_info.master ), 
.wxbl_req_id ( dbus_bridge_req_id ), 
.wxbl_req_user ( 1'b0 ), 
.wxbl_rsp_vld ( dbus_bridge_rsp_vld ), 
.wxbl_rsp_err ( dbus_bridge_rsp_info.err ), 
.wxbl_rsp_rdata ( dbus_bridge_rsp_info.rdata ), 
.wxbl_rsp_id ( dbus_bridge_rsp_id ), 
.ahb_haddr ( dbus_ppln_slv_haddr ), 
.ahb_hburst ( dbus_ppln_slv_hburst ), 
.ahb_hmastlock ( dbus_ppln_slv_hmastlock ), 
.ahb_hprot ( dbus_ppln_slv_hprot ), 
.ahb_hsize ( dbus_ppln_slv_hsize ), 
.ahb_master ( dbus_ppln_slv_hmaster ), 
.ahb_htrans ( dbus_ppln_slv_htrans ), 
.ahb_hwrite ( dbus_ppln_slv_hwrite ), 
.ahb_hwdata ( dbus_ppln_slv_hwdata ), 
.ahb_user ( ), 
.ahb_hrdata ( dbus_ppln_slv_hrdata ), 
.ahb_hready ( dbus_ppln_slv_hready ), 
.ahb_hresp ( dbus_ppln_slv_hresp ) 
); 
ahb_regslice #( 
.ADDR_W ( 32 ), 
.DATA_W ( 32 ), 
.MODE ( REGSLICE_MODE_FEEDTHROUGH ), 
.EN_HBURST ( 0 ), 
.EN_HMLOCK ( 0 ), 
.EN_MISC ( 0 ), 
.misc_t ( logic ) 
) u_ahb_regslice_dbus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_haddr ( dbus_ppln_slv_haddr ), 
.slv_hburst ( dbus_ppln_slv_hburst ), 
.slv_hmastlock ( dbus_ppln_slv_hmastlock ), 
.slv_hprot ( dbus_ppln_slv_hprot ), 
.slv_hsize ( dbus_ppln_slv_hsize ), 
.slv_hmaster ( dbus_ppln_slv_hmaster ), 
.slv_htrans ( dbus_ppln_slv_htrans ), 
.slv_misc ( 1'b0 ), 
.slv_hwrite ( dbus_ppln_slv_hwrite ), 
.slv_hwdata ( dbus_ppln_slv_hwdata ), 
.slv_hrdata ( dbus_ppln_slv_hrdata ), 
.slv_hready ( dbus_ppln_slv_hready ), 
.slv_hresp ( dbus_ppln_slv_hresp ), 
.mst_haddr ( d_bus_ahb_haddr ), 
.mst_hburst ( d_bus_ahb_hburst ), 
.mst_hmastlock ( d_bus_ahb_hmastlock ), 
.mst_hprot ( d_bus_ahb_hprot ), 
.mst_hsize ( d_bus_ahb_hsize ), 
.mst_hmaster ( d_bus_ahb_hmaster ), 
.mst_htrans ( d_bus_ahb_htrans ), 
.mst_misc ( ), 
.mst_hwrite ( d_bus_ahb_hwrite ), 
.mst_hwdata ( d_bus_hwdata_tmp ), 
.mst_hrdata ( d_bus_pol_hrdata ), 
.mst_hready ( d_bus_ahb_hready ), 
.mst_hresp ( d_bus_ahb_hresp ) 
); 
if (REGSLICE_MODE_FEEDTHROUGH != REGSLICE_MODE_FEEDTHROUGH) begin : dbus_regslice 
assign regslice_busy_dbus = d_bus_ahb_htrans[1]; 
end else begin : dbus_no_regslice 
assign regslice_busy_dbus = 1'b0; 
end 
assign d_wdata_updt_en = d_bus_ahb_htrans[1] & d_bus_ahb_hready ; 
`WDFFER(d_bus_ahb_hsize_q, d_bus_ahb_hsize[1:0], d_wdata_updt_en, clk, rst_n) 
`WDFFER(d_addr_offset_q , d_bus_ahb_haddr[1:0], d_wdata_updt_en, clk, rst_n) 
assign d_bus_pol_hwdata = d_bus_hwdata_tmp; 
assign d_bus_ahb_hwdata = d_bus_pol_hwdata; 
assign d_bus_pol_hrdata = d_bus_ahb_hrdata; 
assign mbus_mux_slv_req_vld [MUX_LS_FT] = ft_demux_mst_req_vld[DMX_FT_MBUS]; 
assign mbus_mux_slv_req_info[MUX_LS_FT] = wxbl_req_info_ro2rw(ft_demux_slv_req_info); 
assign ft_demux_mst_req_rdy [DMX_FT_MBUS] = mbus_mux_slv_req_rdy[MUX_LS_FT]; 
assign ft_demux_mst_rsp_vld [DMX_FT_MBUS] = mbus_mux_slv_rsp_vld[MUX_LS_FT]; 
assign ft_demux_mst_rsp_info[DMX_FT_MBUS] = mbus_mux_slv_rsp_info[MUX_LS_FT]; 
assign mbus_mux_slv_req_vld [MUX_LS_LDST] = ldst_demux_mst_req_vld[DMX_LS_MBUS]; 
assign mbus_mux_slv_req_info [MUX_LS_LDST] = ldst_demux_slv_req_info; 
assign ldst_demux_mst_req_rdy [DMX_LS_MBUS] = mbus_mux_slv_req_rdy [MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_vld [DMX_LS_MBUS] = mbus_mux_slv_rsp_vld [MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_info[DMX_LS_MBUS] = mbus_mux_slv_rsp_info[MUX_LS_LDST]; 
wxblite_mux_nx1 #( 
.req_t ( wxbl_req_info_t ), 
.rsp_t ( wxbl_rsp_info_t ), 
.SLV_PORTS_N ( BUS_MUX_N ), 
.ARB_MODE ( 0 ), 
.LOCK ( 1 ), 
.EN_FLUSH ( 0 ) 
) u_wxbl_mux_nx1_mbus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( mbus_mux_slv_req_vld ), 
.slv_req_rdy_o ( mbus_mux_slv_req_rdy ), 
.slv_req_info_i ( mbus_mux_slv_req_info ), 
.slv_rsp_vld_o ( mbus_mux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( {2{1'b1}} ), 
.slv_rsp_info_o ( mbus_mux_slv_rsp_info ), 
.mst_req_vld_o ( mbus_mux_mst_req_vld ), 
.mst_req_rdy_i ( mbus_mux_mst_req_rdy ), 
.mst_req_info_o ( mbus_mux_mst_req_info ), 
.mst_req_id_o ( mbus_mux_mst_req_id ), 
.mst_rsp_vld_i ( mbus_mux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( mbus_mux_mst_rsp_info ), 
.mst_rsp_id_i ( mbus_mux_mst_rsp_id ) 
); 
assign mbus_bridge_req_id = {mbus_mux_mst_req_id, mbus_mux_mst_req_info.id}; 
assign {mbus_mux_mst_rsp_id, mbus_mux_mst_rsp_info.id} = mbus_bridge_rsp_id; 
wxblite_ahblite_bridge #( 
.ADDR_W ( 32 ), 
.DATA_W ( 32 ), 
.PPLN_WDATA ( 0 ), 
.EN_WSTRB_SPLIT ( 1 ), 
.WSTRB_SPLIT_MODE ( WSTRB_SPLIT_MODE ), 
.EN_ID ( 1 ), 
.ID_W ( BUS_SOC_ID_W ), 
.MASTER_W ( 2 ), 
.user_t ( logic ) 
) u_wxbl_ahbl_bridge_mbus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.ppln_wdata_en ( mbus_ppln_wdata_en ), 
.wxbl_req_vld ( mbus_mux_mst_req_vld ), 
.wxbl_req_rdy ( mbus_mux_mst_req_rdy ), 
.wxbl_req_addr ( mbus_mux_mst_req_info.addr ), 
.wxbl_req_size ( mbus_mux_mst_req_info.size ), 
.wxbl_req_write ( mbus_mux_mst_req_info.write ), 
.wxbl_req_wdata ( dff_st_wdata_q ), 
.wxbl_req_wstrb ( mbus_mux_mst_req_info.wstrb ), 
.wxbl_req_prot ( mbus_mux_mst_req_info.prot ), 
.wxbl_req_memattr ( mbus_mux_mst_req_info.memattr ), 
.wxbl_req_master ( mbus_mux_mst_req_info.master ), 
.wxbl_req_id ( mbus_bridge_req_id ), 
.wxbl_req_user ( 1'b0 ), 
.wxbl_rsp_vld ( mbus_mux_mst_rsp_vld ), 
.wxbl_rsp_err ( mbus_mux_mst_rsp_info.err ), 
.wxbl_rsp_rdata ( mbus_mux_mst_rsp_info.rdata ), 
.wxbl_rsp_id ( mbus_bridge_rsp_id ), 
.ahb_haddr ( m_haddr ), 
.ahb_hburst ( m_hburst ), 
.ahb_hmastlock ( m_hmastlock ), 
.ahb_hprot ( m_hprot ), 
.ahb_hsize ( m_hsize ), 
.ahb_master ( m_hmaster ), 
.ahb_htrans ( m_htrans ), 
.ahb_hwrite ( m_hwrite ), 
.ahb_hwdata ( m_hwdata ), 
.ahb_user ( ), 
.ahb_hrdata ( m_hrdata ), 
.ahb_hready ( m_hready ), 
.ahb_hresp ( m_hresp ) 
); 
assign mbus_ppln_slv_haddr = m_haddr ; 
assign mbus_ppln_slv_hburst = m_hburst ; 
assign mbus_ppln_slv_hmastlock = m_hmastlock; 
assign mbus_ppln_slv_hprot = m_hprot ; 
assign mbus_ppln_slv_hsize = m_hsize ; 
assign mbus_ppln_slv_hmaster = m_hmaster ; 
assign mbus_ppln_slv_htrans = m_htrans ; 
assign mbus_ppln_slv_hwrite = m_hwrite ; 
assign mbus_ppln_slv_hwdata = m_hwdata ; 
assign m_hrdata = mbus_ppln_slv_hrdata ; 
assign m_hready = mbus_ppln_slv_hready ; 
assign m_hresp = mbus_ppln_slv_hresp ; 
ahb_regslice #( 
.ADDR_W ( 32 ), 
.DATA_W ( 32 ), 
.MODE ( REGSLICE_MODE_FEEDTHROUGH ), 
.EN_HBURST ( 0 ), 
.EN_HMLOCK ( 0 ), 
.EN_MISC ( 0 ), 
.misc_t ( logic ) 
) u_ahb_regslice_mbus ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_haddr ( mbus_ppln_slv_haddr ), 
.slv_hburst ( mbus_ppln_slv_hburst ), 
.slv_hmastlock ( mbus_ppln_slv_hmastlock ), 
.slv_hprot ( mbus_ppln_slv_hprot ), 
.slv_hsize ( mbus_ppln_slv_hsize ), 
.slv_hmaster ( mbus_ppln_slv_hmaster ), 
.slv_htrans ( mbus_ppln_slv_htrans ), 
.slv_misc ( 1'b0 ), 
.slv_hwrite ( mbus_ppln_slv_hwrite ), 
.slv_hwdata ( mbus_ppln_slv_hwdata ), 
.slv_hrdata ( mbus_ppln_slv_hrdata ), 
.slv_hready ( mbus_ppln_slv_hready ), 
.slv_hresp ( mbus_ppln_slv_hresp ), 
.mst_haddr ( m_bus_ahb_haddr ), 
.mst_hburst ( m_bus_ahb_hburst ), 
.mst_hmastlock ( m_bus_ahb_hmastlock ), 
.mst_hprot ( m_bus_ahb_hprot ), 
.mst_hsize ( m_bus_ahb_hsize ), 
.mst_hmaster ( m_bus_ahb_hmaster ), 
.mst_htrans ( m_bus_ahb_htrans ), 
.mst_misc ( ), 
.mst_hwrite ( m_bus_ahb_hwrite ), 
.mst_hwdata ( m_bus_hwdata_tmp ), 
.mst_hrdata ( m_bus_pol_hrdata ), 
.mst_hready ( m_bus_ahb_hready ), 
.mst_hresp ( m_bus_ahb_hresp ) 
); 
if (REGSLICE_MODE_FEEDTHROUGH != REGSLICE_MODE_FEEDTHROUGH) begin : mbus_regslice 
assign regslice_busy_mbus = m_bus_ahb_htrans[1]; 
end else begin : mbus_no_regslice 
assign regslice_busy_mbus = 1'b0; 
end 
assign m_wdata_updt_en = m_bus_ahb_htrans[1] & m_bus_ahb_hready ; 
`WDFFER(m_bus_ahb_hsize_q, m_bus_ahb_hsize[1:0], m_wdata_updt_en, clk, rst_n) 
`WDFFER(m_addr_offset_q , m_bus_ahb_haddr[1:0], m_wdata_updt_en, clk, rst_n) 
assign m_bus_pol_hwdata = m_bus_hwdata_tmp; 
assign m_bus_ahb_hwdata = m_bus_pol_hwdata; 
assign m_bus_pol_hrdata = m_bus_ahb_hrdata; 
assign ldst_demux_mst_req_rdy[DMX_LS_PBUS] = 1'b0; 
assign ldst_demux_mst_rsp_vld[DMX_LS_PBUS] = 1'b0; 
assign ldst_demux_mst_rsp_info[DMX_LS_PBUS] = '0; 
typedef logic [1-1:0] bus_id_t; 
logic [BUS_MUX_N-1:0] dummy_mux_slv_req_vld ; 
logic [BUS_MUX_N-1:0] dummy_mux_slv_req_rdy ; 
logic [BUS_MUX_N-1:0] dummy_mux_slv_rsp_vld ; 
bus_id_t [BUS_MUX_N-1:0] dummy_mux_slv_req_info; 
logic dummy_mux_mst_req_vld ; 
logic dummy_mux_mst_req_rdy ; 
bus_id_t dummy_mux_mst_req_info; 
logic dummy_mux_mst_rsp_vld ; 
logic [BUS_MUX_ID_W-1:0] dummy_mux_mst_req_id ; 
logic [BUS_MUX_ID_W-1:0] dummy_mux_mst_rsp_id ; 
logic [BUS_SOC_ID_W-1:0] dummy_bridge_req_id ; 
logic [BUS_SOC_ID_W-1:0] dummy_bridge_rsp_id ; 
wxbl_rsp_info_t dummy_bridge_rsp_info ; 
assign dummy_mux_slv_req_vld [MUX_LS_LDST] = ldst_demux_mst_req_vld[DMX_LS_DUMMY]; 
assign dummy_mux_slv_req_info[MUX_LS_LDST] = ldst_demux_slv_req_info.id; 
assign dummy_mux_slv_req_vld [MUX_LS_FT ] = ft_demux_mst_req_vld [DMX_FT_DUMMY]; 
assign dummy_mux_slv_req_info[MUX_LS_FT ] = ft_demux_slv_req_info.id; 
assign ldst_demux_mst_req_rdy [DMX_LS_DUMMY] = dummy_mux_slv_req_rdy[MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_vld [DMX_LS_DUMMY] = dummy_mux_slv_rsp_vld[MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_info[DMX_LS_DUMMY] = dummy_bridge_rsp_info; 
assign ft_demux_mst_req_rdy [DMX_FT_DUMMY] = dummy_mux_slv_req_rdy[MUX_LS_FT ]; 
assign ft_demux_mst_rsp_vld [DMX_FT_DUMMY] = dummy_mux_slv_rsp_vld[MUX_LS_FT ]; 
assign ft_demux_mst_rsp_info [DMX_FT_DUMMY] = dummy_bridge_rsp_info; 
wxblite_mux_nx1 #( 
.req_t ( bus_id_t ), 
.rsp_t ( logic ), 
.SLV_PORTS_N ( BUS_MUX_N ), 
.ARB_MODE ( 0 ), 
.LOCK ( 0 ), 
.EN_FLUSH ( 0 ) 
) u_wxbl_mux_nx1_dummy ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( dummy_mux_slv_req_vld ), 
.slv_req_rdy_o ( dummy_mux_slv_req_rdy ), 
.slv_req_info_i ( dummy_mux_slv_req_info ), 
.slv_rsp_vld_o ( dummy_mux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( {BUS_MUX_N{1'b1}} ), 
.slv_rsp_info_o ( ), 
.mst_req_vld_o ( dummy_mux_mst_req_vld ), 
.mst_req_rdy_i ( dummy_mux_mst_req_rdy ), 
.mst_req_info_o ( dummy_mux_mst_req_info ), 
.mst_req_id_o ( dummy_mux_mst_req_id ), 
.mst_rsp_vld_i ( dummy_mux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( 1'b0 ), 
.mst_rsp_id_i ( dummy_mux_mst_rsp_id ) 
); 
assign dummy_bridge_req_id = {dummy_mux_mst_req_id, dummy_mux_mst_req_info}; 
assign {dummy_mux_mst_rsp_id, dummy_bridge_rsp_info.id} = dummy_bridge_rsp_id; 
assign dummy_bridge_rsp_info.rdata = 32'b0; 
wxblite_dummy_slave #( 
.EN_ID ( 1 ), 
.ID_W ( BUS_SOC_ID_W ) 
) u_wxbl_dummy_slave_mst ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.wxbl_req_vld ( dummy_mux_mst_req_vld ), 
.wxbl_req_rdy ( dummy_mux_mst_req_rdy ), 
.wxbl_req_id ( dummy_bridge_req_id ), 
.wxbl_rsp_vld ( dummy_mux_mst_rsp_vld ), 
.wxbl_rsp_rdy ( 1'b1 ), 
.wxbl_rsp_err ( dummy_bridge_rsp_info.err ), 
.wxbl_rsp_id ( dummy_bridge_rsp_id ) 
); 
assign bmu_tcm_req_vld = 1'b0; 
assign bmu_tcm_req_addr = 32'b0; 
assign bmu_tcm_req_write = 1'b0; 
assign bmu_tcm_req_wstrb = 4'b0; 
assign bmu_tcm_req_wdata = 32'b0; 
assign bmu_tcm_req_dest = 2'b0; 
assign bmu_tcm_rsp_rdy = 1'b0; 
assign uncore_mux_slv_req_vld [MUX_LS_LDST] = ldst_demux_mst_req_vld[DMX_LS_UNCORE]; 
assign uncore_mux_slv_req_info[MUX_LS_LDST].addr = ldst_demux_slv_req_info.addr ; 
assign uncore_mux_slv_req_info[MUX_LS_LDST].size = ldst_demux_slv_req_info.size ; 
assign uncore_mux_slv_req_info[MUX_LS_LDST].write = ldst_demux_slv_req_info.write; 
assign uncore_mux_slv_req_info[MUX_LS_LDST].wstrb = ldst_demux_slv_req_info.wstrb; 
assign uncore_mux_slv_req_info[MUX_LS_LDST].wdata = nonahb_wdata ; 
assign uncore_mux_slv_req_info[MUX_LS_LDST].id = ldst_demux_slv_req_info.id ; 
assign ldst_demux_mst_req_rdy [DMX_LS_UNCORE] = uncore_mux_slv_req_rdy [MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_vld [DMX_LS_UNCORE] = uncore_mux_slv_rsp_vld [MUX_LS_LDST]; 
assign ldst_demux_mst_rsp_info[DMX_LS_UNCORE] = uncore_mux_slv_rsp_info[MUX_LS_LDST]; 
wxblite_mux_nx1 #( 
.req_t ( wxbl_req_info_mmr_t ), 
.rsp_t ( wxbl_rsp_info_t ), 
.SLV_PORTS_N ( UC_MUX_N ), 
.ARB_MODE ( 0 ), 
.LOCK ( 0 ), 
.EN_FLUSH ( 0 ), 
.IDX_W ( UC_MUX_ID_W ) 
) u_wxbl_mux_nx1_uncore ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( uncore_mux_slv_req_vld ), 
.slv_req_rdy_o ( uncore_mux_slv_req_rdy ), 
.slv_req_info_i ( uncore_mux_slv_req_info ), 
.slv_rsp_vld_o ( uncore_mux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( {UC_MUX_N{1'b1}} ), 
.slv_rsp_info_o ( uncore_mux_slv_rsp_info ), 
.mst_req_vld_o ( uncore_mux_mst_req_vld ), 
.mst_req_rdy_i ( uncore_mux_mst_req_rdy ), 
.mst_req_info_o ( uncore_mux_mst_req_info ), 
.mst_req_id_o ( uncore_mux_mst_req_id ), 
.mst_rsp_vld_i ( uncore_mux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( uncore_mux_mst_rsp_info ), 
.mst_rsp_id_i ( uncore_mux_mst_rsp_id ) 
); 
assign uncore_demux_slv_req_vld = uncore_mux_mst_req_vld ; 
assign uncore_demux_slv_req_info = uncore_mux_mst_req_info; 
assign uncore_mux_mst_req_rdy = uncore_demux_slv_req_rdy; 
assign uncore_mmr_req_id = {uncore_demux_slv_req_info.id, uncore_mux_mst_req_id}; 
`WDFFER(uncore_mmr_rsp_id, uncore_mmr_req_id, uncore_mux_mst_req_vld & uncore_mux_mst_req_rdy, clk, rst_n) 
assign uncore_mux_mst_rsp_id = uncore_mmr_rsp_id[UC_MUX_ID_W-1:0]; 
wing_cbb_regslice #( 
.info_t ( wxbl_rsp_info_mmr_t ), 
.MODE ( REGSLICE_MODE_FORWARD ), 
.RAR ( RESET_ALL_REG ) 
) u_regslice_uncore_rsp ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.slv_vld_i ( uncore_demux_slv_rsp_vld ), 
.slv_rdy_o ( ), 
.slv_info_i ( uncore_demux_slv_rsp_info ), 
.mst_vld_o ( uncore_mux_mst_rsp_vld ), 
.mst_rdy_i ( 1'b1 ), 
.mst_info_o ( uncore_regslice_mst_rsp_info ) 
); 
assign uncore_mux_mst_rsp_info.rdata = uncore_regslice_mst_rsp_info.rdata; 
assign uncore_mux_mst_rsp_info.err = uncore_regslice_mst_rsp_info.err ; 
assign uncore_mux_mst_rsp_info.id = uncore_mmr_rsp_id[UC_ALL_ID_W-1 : UC_MUX_ID_W]; 
assign uncore_access_err = ( uncore_demux_slv_req_info.size != 2'b10) 
| ( uncore_demux_slv_req_info.write & (~&uncore_demux_slv_req_info.wstrb)); 
assign uncore_sel_clic = (uncore_demux_slv_req_info.addr[17:16] == 2'd1); 
assign uncore_sel_clint = (uncore_demux_slv_req_info.addr[17:12] == 6'd0) & ~uncore_access_err; 
assign dcd_uncore_sel[DMX_UC_CLIC] = uncore_sel_clic | uncore_sel_clint; 
assign dcd_uncore_sel[DMX_UC_TRACE] = 1'b0; 
assign dcd_uncore_sel[DMX_UC_RERI] = (uncore_demux_slv_req_info.addr[17:16] == 2'd2) & ~uncore_access_err; 
assign dcd_uncore_sel[DMX_UC_HPU] = 1'b0; 
assign dcd_uncore_sel[DMX_UC_MSS] = (uncore_demux_slv_req_info.addr[17:12] == 6'd4 ) & ~uncore_access_err; 
assign dcd_uncore_sel[DMX_UC_DUMMY] = ~|dcd_uncore_sel[UC_DEMUX_N-1:1]; 
wxblite_demux_1xn #( 
.rsp_t ( wxbl_rsp_info_mmr_t ), 
.MST_PORTS_N ( UC_DEMUX_N ), 
.EN_FLUSH ( 0 ), 
.EN_KEEP_ORDER ( 2 ) 
) u_wxbl_demux_1xn_uncore ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.flush ( 1'b0 ), 
.slv_req_vld_i ( uncore_demux_slv_req_vld ), 
.slv_req_rdy_o ( uncore_demux_slv_req_rdy ), 
.slv_rsp_vld_o ( uncore_demux_slv_rsp_vld ), 
.slv_rsp_rdy_i ( 1'b1 ), 
.slv_rsp_info_o ( uncore_demux_slv_rsp_info ), 
.sel_i ( dcd_uncore_sel ), 
.mst_req_vld_o ( uncore_demux_mst_req_vld ), 
.mst_req_rdy_i ( uncore_demux_mst_req_rdy ), 
.mst_rsp_vld_i ( uncore_demux_mst_rsp_vld ), 
.mst_rsp_rdy_o ( ), 
.mst_rsp_info_i ( uncore_demux_mst_rsp_info ) 
); 
wxblite_dummy_slave #( 
.EN_ID ( 0 ), 
.ID_W ( 1 ), 
.EN_REG ( 1 ) 
) u_wxbl_dummy_slave_uncore ( 
.clk ( clk ), 
.rst_n ( rst_n ), 
.wxbl_req_vld ( uncore_demux_mst_req_vld[DMX_UC_DUMMY] ), 
.wxbl_req_rdy ( uncore_demux_mst_req_rdy[DMX_UC_DUMMY] ), 
.wxbl_req_id ( 1'b0 ), 
.wxbl_rsp_vld ( uncore_demux_mst_rsp_vld[DMX_UC_DUMMY] ), 
.wxbl_rsp_rdy ( 1'b1 ), 
.wxbl_rsp_err ( uncore_demux_mst_rsp_info[DMX_UC_DUMMY].err ), 
.wxbl_rsp_id ( ) 
); 
assign uncore_demux_mst_rsp_info[DMX_UC_DUMMY].rdata = 32'b0; 
assign bmu_clic_req_vld = uncore_demux_mst_req_vld [DMX_UC_CLIC]; 
assign bmu_clic_req_addr = uncore_demux_slv_req_info.addr ; 
assign bmu_clic_req_size = uncore_demux_slv_req_info.size ; 
assign bmu_clic_req_write = uncore_demux_slv_req_info.write; 
assign bmu_clic_req_wstrb = uncore_demux_slv_req_info.wstrb; 
assign bmu_clic_req_wdata = uncore_demux_slv_req_info.wdata; 
assign bmu_clic_req_dest = {uncore_sel_clic, uncore_sel_clint}; 
assign bmu_clic_rsp_rdy = 1'b1; 
assign uncore_demux_mst_req_rdy[DMX_UC_CLIC] = clic_bmu_req_rdy; 
assign uncore_demux_mst_rsp_vld [DMX_UC_CLIC] = clic_bmu_rsp_vld; 
assign uncore_demux_mst_rsp_info[DMX_UC_CLIC].rdata = clic_bmu_rsp_rdata; 
assign uncore_demux_mst_rsp_info[DMX_UC_CLIC].err = clic_bmu_rsp_err; 
assign uncore_demux_mst_req_rdy [DMX_UC_TRACE] = 1'b0; 
assign uncore_demux_mst_rsp_vld [DMX_UC_TRACE] = '0; 
assign uncore_demux_mst_rsp_info[DMX_UC_TRACE].rdata = '0; 
assign uncore_demux_mst_rsp_info[DMX_UC_TRACE].err = '0; 
assign bmu_reri_req_vld = uncore_demux_mst_req_vld [DMX_UC_RERI]; 
assign bmu_reri_req_addr = uncore_demux_slv_req_info.addr ; 
assign bmu_reri_req_write = uncore_demux_slv_req_info.write; 
assign bmu_reri_req_wdata = uncore_demux_slv_req_info.wdata; 
assign bmu_reri_rsp_rdy = 1'b1; 
assign uncore_demux_mst_req_rdy[DMX_UC_RERI] = reri_bmu_req_rdy; 
assign uncore_demux_mst_rsp_vld [DMX_UC_RERI] = reri_bmu_rsp_vld; 
assign uncore_demux_mst_rsp_info[DMX_UC_RERI].rdata = reri_bmu_rsp_rdata; 
assign uncore_demux_mst_rsp_info[DMX_UC_RERI].err = reri_bmu_rsp_err; 
assign uncore_demux_mst_req_rdy [DMX_UC_HPU] = 1'b0; 
assign uncore_demux_mst_rsp_vld [DMX_UC_HPU] = '0; 
assign uncore_demux_mst_rsp_info[DMX_UC_HPU].rdata = '0; 
assign uncore_demux_mst_rsp_info[DMX_UC_HPU].err = '0; 
assign bmu_mss_req_vld = uncore_demux_mst_req_vld[DMX_UC_MSS]; 
assign bmu_mss_req_addr = uncore_demux_slv_req_info.addr; 
assign bmu_mss_req_write = uncore_demux_slv_req_info.write; 
assign bmu_mss_req_wdata = uncore_demux_slv_req_info.wdata; 
assign bmu_mss_rsp_rdy = 1'b1; 
assign uncore_demux_mst_req_rdy[DMX_UC_MSS] = mss_bmu_req_rdy; 
assign uncore_demux_mst_rsp_vld [DMX_UC_MSS] = mss_bmu_rsp_vld; 
assign uncore_demux_mst_rsp_info[DMX_UC_MSS].rdata = mss_bmu_rsp_rdata; 
assign uncore_demux_mst_rsp_info[DMX_UC_MSS].err = mss_bmu_rsp_err; 
assign bmu_idle = ~(regslice_busy_ft | regslice_busy_ldst | regslice_busy_ibus | regslice_busy_dbus | regslice_busy_mbus); 
endmodule
 
 

 
 

 
 
 
module m130_crg ( 
input logic always_on_clk , 
input logic clk , 
input logic clic_cfg_clk_en , 
output logic clic_cfg_clk , 
input logic pmp_cfg_clk_en , 
output logic pmp_cfg_clk , 
input logic core_sleep_mode , 
input logic core_deep_sleep , 
input logic core_sleep_wakeup , 
output logic gated_clk , 
input logic pwrup_rst_n , 
input logic dm_core_reset , 
input logic dm_dmactive , 
input logic ndm_rst_n , 
output logic core_rst_n , 
output logic clic_rst_n , 
output logic tm_rst_n , 
output logic dm_active_rst_n , 
output logic dm_rst_n , 
input logic soft_rst_n , 
output logic core_sleeping , 
output logic core_deep_sleeping, 
output logic clk_gate_en , 
input logic dft_icg_scan_en , 
input logic dft_scan_mode , 
input logic dft_scan_rst_n 
); 
localparam Q_EXIT = 3'b001; 
localparam Q_RUN = 3'b011; 
localparam Q_REQUEST = 3'b010; 
localparam Q_STOPPED = 3'b000; 
localparam Q_CONTINUE = 3'b111; 
localparam Q_DENIED = 3'b110; 
logic ndm_rst_n_1ff ; 
logic soft_rst_n_1ff ; 
logic pwrup_rst_n_logic_dft ; 
logic clic_cfg_clk_icg_en ; 
logic pmp_cfg_clk_icg_en ; 
assign gated_clk = clk; 
assign core_sleeping = core_sleep_mode; 
assign core_deep_sleeping = core_sleep_mode & core_deep_sleep; 
assign clic_cfg_clk_icg_en = clic_cfg_clk_en 
; 
assign pmp_cfg_clk_icg_en = pmp_cfg_clk_en 
; 
wing_cbb_icg_wrap u_clic_cfg_clk_gen ( 
.clk (always_on_clk ), 
.clk_en (clic_cfg_clk_icg_en ), 
.scan_en (dft_icg_scan_en ), 
.gated_clk (clic_cfg_clk ) 
); 
wing_cbb_icg_wrap u_pmp_cfg_clk_gen ( 
.clk (always_on_clk ), 
.clk_en (pmp_cfg_clk_icg_en ), 
.scan_en (dft_icg_scan_en ), 
.gated_clk (pmp_cfg_clk ) 
); 
assign clk_gate_en = core_sleeping; 
wing_cbb_dft_mux2 pwrup_rst_n_logic_dft_inst (.din0(pwrup_rst_n), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(pwrup_rst_n_logic_dft)); 
`WDFFR(ndm_rst_n_1ff, ndm_rst_n, always_on_clk, pwrup_rst_n_logic_dft) 
wing_cbb_dft_mux2 dm_active_rst_n_inst (.din0(pwrup_rst_n ), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(dm_active_rst_n)); 
`WDFFR(soft_rst_n_1ff, soft_rst_n, always_on_clk, pwrup_rst_n_logic_dft) 
wing_cbb_dft_mux2 core_rst_n_inst (.din0((pwrup_rst_n & ndm_rst_n_1ff & soft_rst_n_1ff & !dm_core_reset)), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(core_rst_n)); 
wing_cbb_dft_mux2 clic_rst_n_inst (.din0((pwrup_rst_n & ndm_rst_n_1ff & !dm_core_reset)), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(clic_rst_n)); 
wing_cbb_dft_mux2 tm_rst_n_inst (.din0((pwrup_rst_n & ndm_rst_n_1ff & !dm_core_reset)), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(tm_rst_n)); 
wing_cbb_dft_mux2 dm_rst_n_inst (.din0((pwrup_rst_n & dm_dmactive)), .din1(dft_scan_rst_n ), .sel(dft_scan_mode), .dout(dm_rst_n)); 
endmodule
 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 

 
 
 
module m130_reri ( 
input logic clk, 
input logic rst_n, 
input logic [15:0] inst_id, 
input logic err_valid, 
input logic err_ce, 
input logic err_ude, 
input logic err_uue, 
input logic [1:0] err_priority, 
input logic [2:0] err_tt, 
input logic err_scrub, 
input logic [7:0] err_ec, 
input logic [32-1:0] err_addr, 
input logic [3:0] err_aec, 
input logic err_tag, 
output logic ras_int, 
input logic mmr_req_vld, 
output logic mmr_req_rdy, 
input logic [31:0] mmr_req_addr, 
input logic mmr_req_write, 
input logic [31:0] mmr_req_wdata, 
output logic mmr_rsp_vld, 
input logic mmr_rsp_rdy, 
output logic [31:0] mmr_rsp_rdata, 
output logic mmr_rsp_err 
); 
logic dff_status_v_en; 
logic dff_status_v_d; 
logic dff_status_v_q; 
logic dff_status_ce_en; 
logic dff_status_ce_d; 
logic dff_status_ce_q; 
logic dff_status_uue_en; 
logic dff_status_uue_d; 
logic dff_status_uue_q; 
logic dff_status_pri_en; 
logic [1:0] dff_status_pri_d; 
logic [1:0] dff_status_pri_q; 
logic dff_status_mo_en; 
logic dff_status_mo_d; 
logic dff_status_mo_q; 
logic tt_wr_legal; 
logic dff_status_tt_en; 
logic [2:0] dff_status_tt_d; 
logic [2:0] dff_status_tt_q; 
logic dff_status_iv_en; 
logic dff_status_iv_d ; 
logic dff_status_iv_q ; 
logic dff_status_scrub_en; 
logic dff_status_scrub_d; 
logic dff_status_scrub_q; 
logic dff_status_ceco_en; 
logic dff_status_ceco_d; 
logic dff_status_ceco_q; 
logic ec_wr_legal; 
logic dff_status_ec_en; 
logic [7:0] dff_status_ec_d; 
logic [7:0] dff_status_ec_q; 
logic cec_tick; 
logic dff_status_cec_en; 
logic [15:0] dff_status_cec_incr; 
logic [15:0] dff_status_cec_d; 
logic [15:0] dff_status_cec_q; 
logic dff_addr_en; 
logic [32-1:0] dff_addr_d; 
logic [32-1:0] dff_addr_q; 
logic aec_wr_legal; 
logic dff_info_aec_en; 
logic [3:0] dff_info_aec_d; 
logic [3:0] dff_info_aec_q; 
logic dff_info_tag_en; 
logic dff_info_tag_d; 
logic dff_info_tag_q; 
logic cec_overflow; 
logic wr_new_err; 
logic ow_severity; 
logic ow_priority; 
logic higher_severity; 
logic equal_severity; 
logic higher_priority; 
logic overwrite; 
logic log_err_vld; 
logic dff_ctrl_else_en; 
logic dff_ctrl_else_d ; 
logic dff_ctrl_else_q ; 
logic dff_ctrl_cece_en; 
logic dff_ctrl_cece_d ; 
logic dff_ctrl_cece_q ; 
logic dff_ctrl_ces_en; 
logic [1:0] dff_ctrl_ces_d ; 
logic [1:0] dff_ctrl_ces_q ; 
logic dff_ctrl_uues_en; 
logic [1:0] dff_ctrl_uues_d ; 
logic [1:0] dff_ctrl_uues_q ; 
logic bmu_sel_vendor_n_imp_id_lo; 
logic bmu_sel_vendor_n_imp_id_hi; 
logic bmu_sel_bank_info_lo; 
logic bmu_sel_bank_info_hi; 
logic bmu_sel_valid_summary_lo; 
logic bmu_sel_valid_summary_hi; 
logic bmu_sel_control_lo; 
logic bmu_sel_control_hi; 
logic bmu_sel_status_lo; 
logic bmu_sel_status_hi; 
logic bmu_sel_addr_lo; 
logic bmu_sel_addr_hi; 
logic bmu_sel_info_lo; 
logic bmu_sel_info_hi; 
logic bmu_req_hdsk; 
logic bmu_wr_vld; 
logic bmu_wr_status_lo; 
logic bmu_wr_status_hi; 
logic bmu_wr_control_lo; 
logic bmu_wr_control_sinv; 
logic [31:0] mmr_rdata; 
logic [63:0] mmr_vendor_n_imp_id; 
logic [63:0] mmr_bank_info; 
logic [63:0] mmr_valid_summary; 
logic [63:0] mmr_control; 
logic [63:0] mmr_status; 
logic [63:0] mmr_addr; 
logic [63:0] mmr_info; 
assign bmu_sel_vendor_n_imp_id_lo = (mmr_req_addr[15:0] == 16'h0000); 
assign bmu_sel_vendor_n_imp_id_hi = (mmr_req_addr[15:0] == 16'h0004); 
assign bmu_sel_bank_info_lo = (mmr_req_addr[15:0] == 16'h0008); 
assign bmu_sel_bank_info_hi = (mmr_req_addr[15:0] == 16'h000C); 
assign bmu_sel_valid_summary_lo = (mmr_req_addr[15:0] == 16'h0010); 
assign bmu_sel_valid_summary_hi = (mmr_req_addr[15:0] == 16'h0014); 
assign bmu_sel_control_lo = (mmr_req_addr[15:0] == 16'h0040); 
assign bmu_sel_control_hi = (mmr_req_addr[15:0] == 16'h0044); 
assign bmu_sel_status_lo = (mmr_req_addr[15:0] == 16'h0048); 
assign bmu_sel_status_hi = (mmr_req_addr[15:0] == 16'h004C); 
assign bmu_sel_addr_lo = (mmr_req_addr[15:0] == 16'h0050); 
assign bmu_sel_addr_hi = (mmr_req_addr[15:0] == 16'h0054); 
assign bmu_sel_info_lo = (mmr_req_addr[15:0] == 16'h0058); 
assign bmu_sel_info_hi = (mmr_req_addr[15:0] == 16'h005C); 
assign bmu_req_hdsk = mmr_req_vld & mmr_req_rdy; 
assign bmu_wr_vld = bmu_req_hdsk & mmr_req_write; 
assign bmu_wr_status_lo = bmu_wr_vld & bmu_sel_status_lo; 
assign bmu_wr_status_hi = bmu_wr_vld & bmu_sel_status_hi; 
assign bmu_wr_control_lo = bmu_wr_vld & bmu_sel_control_lo; 
assign bmu_wr_control_sinv = bmu_wr_control_lo & mmr_req_wdata[2]; 
assign mmr_req_rdy = 1'b1; 
assign mmr_rsp_err = 1'b0; 
assign mmr_rdata = {32{bmu_sel_vendor_n_imp_id_lo }} & mmr_vendor_n_imp_id[31:0] | 
{32{bmu_sel_vendor_n_imp_id_hi }} & mmr_vendor_n_imp_id[63:32] | 
{32{bmu_sel_bank_info_lo }} & mmr_bank_info[31:0] | 
{32{bmu_sel_bank_info_hi }} & mmr_bank_info[63:32] | 
{32{bmu_sel_valid_summary_lo }} & mmr_valid_summary[31:0] | 
{32{bmu_sel_valid_summary_hi }} & mmr_valid_summary[63:32] | 
{32{bmu_sel_control_lo }} & mmr_control[31:0] | 
{32{bmu_sel_control_hi }} & mmr_control[63:32] | 
{32{bmu_sel_status_lo }} & mmr_status[31:0] | 
{32{bmu_sel_status_hi }} & mmr_status[63:32] | 
{32{bmu_sel_addr_lo }} & mmr_addr[31:0] | 
{32{bmu_sel_addr_hi }} & mmr_addr[63:32] | 
{32{bmu_sel_info_lo }} & mmr_info[31:0] | 
{32{bmu_sel_info_hi }} & mmr_info[63:32] ; 
if (1 == 1) begin : g_reri_mmr_rsp_ppln 
`WDFFR(mmr_rsp_vld, mmr_req_vld, clk, rst_n) 
`WDFFENR(mmr_rsp_rdata, mmr_rdata, mmr_req_vld, clk) 
end else begin : g_reri_mmr_rsp_no_ppln 
assign mmr_rsp_vld = mmr_req_vld; 
assign mmr_rsp_rdata = mmr_rdata; 
end 
assign log_err_vld = err_valid & dff_ctrl_else_q; 
assign higher_severity = err_uue & ~dff_status_uue_q 
| ~err_uue & ~dff_status_uue_q & err_ce & ~dff_status_ce_q; 
assign equal_severity = (err_uue & dff_status_uue_q 
| ~err_uue & ~dff_status_uue_q & err_ce & dff_status_ce_q); 
assign higher_priority = equal_severity & 
(err_priority > dff_status_pri_q); 
assign wr_new_err = log_err_vld & ~dff_status_v_q; 
assign ow_severity = log_err_vld & dff_status_v_q & higher_severity; 
assign ow_priority = log_err_vld & dff_status_v_q & higher_priority; 
assign overwrite = wr_new_err | ow_severity | ow_priority; 
assign dff_status_v_en = log_err_vld | bmu_wr_status_lo | bmu_wr_control_sinv; 
assign dff_status_v_d = bmu_wr_status_lo ? mmr_req_wdata[0] : 
bmu_wr_control_sinv ? 1'b0 : 
log_err_vld ; 
`WDFFER (dff_status_v_q, dff_status_v_d, dff_status_v_en, clk, rst_n) 
assign dff_status_ce_en = wr_new_err | ow_severity | bmu_wr_status_lo; 
assign dff_status_ce_d = bmu_wr_status_lo ? mmr_req_wdata[1] : 
wr_new_err ? (~err_uue & err_ce) : 
(dff_status_ce_q | err_ce) ; 
`WDFFER (dff_status_ce_q, dff_status_ce_d, dff_status_ce_en, clk, rst_n) 
assign dff_status_uue_en = wr_new_err | ow_severity | bmu_wr_status_lo; 
assign dff_status_uue_d = bmu_wr_status_lo ? mmr_req_wdata[3] : 
wr_new_err ? err_uue : 
(dff_status_uue_q | err_uue); 
`WDFFER (dff_status_uue_q, dff_status_uue_d, dff_status_uue_en, clk, rst_n) 
assign dff_status_pri_en = overwrite | bmu_wr_status_lo; 
assign dff_status_pri_d = bmu_wr_status_lo ? mmr_req_wdata[5:4] : err_priority; 
`WDFFER (dff_status_pri_q, dff_status_pri_d, dff_status_pri_en, clk, rst_n) 
assign dff_status_mo_en = log_err_vld | bmu_wr_status_lo; 
assign dff_status_mo_d = bmu_wr_status_lo ? mmr_req_wdata[6] : 
(dff_status_v_q ? (higher_severity ? 1'b0 : 
equal_severity ? 1'b1 : dff_status_mo_q) 
: 1'b0); 
`WDFFER (dff_status_mo_q, dff_status_mo_d, dff_status_mo_en, clk, rst_n) 
assign dff_status_tt_en = overwrite | bmu_wr_status_lo; 
assign dff_status_tt_d = bmu_wr_status_lo ? 
(tt_wr_legal ? mmr_req_wdata[10:8] : 3'b000) : err_tt; 
assign tt_wr_legal = (mmr_req_wdata[10:8] == 3'b000) | 
(mmr_req_wdata[10:8] == 3'b100) | 
(mmr_req_wdata[10:8] == 3'b101) | 
(mmr_req_wdata[10:8] == 3'b110) | 
(mmr_req_wdata[10:8] == 3'b111) ; 
`WDFFER (dff_status_tt_q, dff_status_tt_d, dff_status_tt_en, clk, rst_n) 
assign dff_status_iv_en = overwrite | bmu_wr_status_lo; 
assign dff_status_iv_d = bmu_wr_status_lo ? mmr_req_wdata[11] : 1'b1; 
`WDFFER (dff_status_iv_q, dff_status_iv_d, dff_status_iv_en, clk, rst_n) 
assign dff_status_scrub_en = overwrite | bmu_wr_status_lo; 
assign dff_status_scrub_d = bmu_wr_status_lo ? mmr_req_wdata[20] : err_scrub; 
`WDFFER (dff_status_scrub_q, dff_status_scrub_d, dff_status_scrub_en, clk, rst_n) 
assign dff_status_ceco_en = cec_overflow | bmu_wr_status_lo; 
assign dff_status_ceco_d = bmu_wr_status_lo ? mmr_req_wdata[21] : 1'b1; 
`WDFFER (dff_status_ceco_q, dff_status_ceco_d, dff_status_ceco_en, clk, rst_n) 
assign dff_status_ec_en = overwrite | bmu_wr_status_lo; 
assign dff_status_ec_d = bmu_wr_status_lo ? 
(ec_wr_legal ? mmr_req_wdata[31:24] : 8'h00) : err_ec; 
assign ec_wr_legal = (mmr_req_wdata[31:24] == 8'h00) | 
(mmr_req_wdata[31:24] == 8'h03) | 
(mmr_req_wdata[31:24] == 8'h04) | 
(mmr_req_wdata[31:24] == 8'h0e) | 
(mmr_req_wdata[31:24] == 8'h40) | 
(mmr_req_wdata[31:24] == 8'h41) ; 
`WDFFER (dff_status_ec_q, dff_status_ec_d, dff_status_ec_en, clk, rst_n) 
assign dff_status_cec_en = cec_tick | bmu_wr_status_hi; 
assign dff_status_cec_d = bmu_wr_status_hi ? mmr_req_wdata[31:16] : dff_status_cec_incr; 
`WDFFER (dff_status_cec_q, dff_status_cec_d, dff_status_cec_en, clk, rst_n) 
assign dff_status_cec_incr = dff_status_cec_q + 1'b1; 
assign cec_tick = log_err_vld & err_ce & dff_ctrl_cece_q; 
assign cec_overflow = &dff_status_cec_q & cec_tick; 
assign dff_addr_en = overwrite | (bmu_wr_vld & bmu_sel_addr_lo); 
assign dff_addr_d = (bmu_wr_vld & bmu_sel_addr_lo) ? mmr_req_wdata[31:0] : err_addr; 
`WDFFER (dff_addr_q, dff_addr_d, dff_addr_en, clk, rst_n) 
assign dff_info_aec_en = overwrite | (bmu_wr_vld & bmu_sel_info_lo); 
assign dff_info_aec_d = (bmu_wr_vld & bmu_sel_info_lo) ? 
(aec_wr_legal ? mmr_req_wdata[3:0] : 4'h0) : err_aec; 
assign aec_wr_legal = (mmr_req_wdata[3:0] == 4'h0) | 
(mmr_req_wdata[3:0] == 4'h1) | 
(mmr_req_wdata[3:0] == 4'h3) | 
(mmr_req_wdata[3:0] == 4'h4) ; 
`WDFFER (dff_info_aec_q, dff_info_aec_d, dff_info_aec_en, clk, rst_n) 
assign dff_info_tag_en = overwrite | (bmu_wr_vld & bmu_sel_info_lo); 
assign dff_info_tag_d = (bmu_wr_vld & bmu_sel_info_lo) ? mmr_req_wdata[4] : err_tag; 
`WDFFER (dff_info_tag_q, dff_info_tag_d, dff_info_tag_en, clk, rst_n) 
assign dff_ctrl_else_en = bmu_wr_control_lo; 
assign dff_ctrl_else_d = mmr_req_wdata[0]; 
`WDFFER(dff_ctrl_else_q, dff_ctrl_else_d, dff_ctrl_else_en, clk, rst_n) 
assign dff_ctrl_cece_en = bmu_wr_control_lo; 
assign dff_ctrl_cece_d = mmr_req_wdata[1]; 
`WDFFER(dff_ctrl_cece_q, dff_ctrl_cece_d, dff_ctrl_cece_en, clk, rst_n) 
assign dff_ctrl_ces_en = bmu_wr_control_lo; 
assign dff_ctrl_ces_d = (mmr_req_wdata[5:4] == 2'b00) ? 2'b00 : 2'b11; 
`WDFFER(dff_ctrl_ces_q, dff_ctrl_ces_d, dff_ctrl_ces_en, clk, rst_n) 
assign dff_ctrl_uues_en = bmu_wr_control_lo; 
assign dff_ctrl_uues_d = (mmr_req_wdata[9:8] == 2'b00) ? 2'b00 : 2'b11; 
`WDFFER(dff_ctrl_uues_q, dff_ctrl_uues_d, dff_ctrl_uues_en, clk, rst_n) 
assign ras_int = dff_ctrl_else_q & 
( (dff_status_v_q & (&dff_ctrl_uues_q) & dff_status_uue_q) 
| ((&dff_ctrl_ces_q) & dff_ctrl_cece_q & dff_status_ceco_q) ); 
assign mmr_vendor_n_imp_id[31: 0] = 32'h0000_077B; 
assign mmr_vendor_n_imp_id[47:32] = 16'b0010_0011_0001_0001; 
assign mmr_vendor_n_imp_id[63:48] = 16'b0; 
assign mmr_bank_info[15: 0] = inst_id; 
assign mmr_bank_info[31:16] = 16'b1; 
assign mmr_bank_info[55:32] = 23'b0; 
assign mmr_bank_info[63:56] = 8'b1; 
assign mmr_valid_summary[63:1] = 63'b0; 
assign mmr_valid_summary[0] = 1'b0; 
assign mmr_control[0] = dff_ctrl_else_q; 
assign mmr_control[1] = dff_ctrl_cece_q; 
assign mmr_control[2] = 1'b0; 
assign mmr_control[3] = 1'b0; 
assign mmr_control[ 5: 4] = dff_ctrl_ces_q; 
assign mmr_control[ 7: 6] = 2'b0; 
assign mmr_control[ 9: 8] = dff_ctrl_uues_q; 
assign mmr_control[31:10] = '0; 
assign mmr_control[47:32] = '0; 
assign mmr_control[55:48] = '0; 
assign mmr_control[63:56] = '0; 
assign mmr_status[0] = dff_status_v_q; 
assign mmr_status[1] = dff_status_ce_q; 
assign mmr_status[2] = 1'b0; 
assign mmr_status[3] = dff_status_uue_q; 
assign mmr_status[5:4] = dff_status_pri_q; 
assign mmr_status[6] = dff_status_mo_q; 
assign mmr_status[7] = 1'b0; 
assign mmr_status[10:8] = dff_status_tt_q; 
assign mmr_status[11] = dff_status_iv_q; 
assign mmr_status[15:12] = 4'b0001; 
assign mmr_status[16] = '0; 
assign mmr_status[17] = '0; 
assign mmr_status[19:18] = '0; 
assign mmr_status[20] = dff_status_scrub_q; 
assign mmr_status[21] = dff_status_ceco_q; 
assign mmr_status[23:22] = '0; 
assign mmr_status[31:24] = dff_status_ec_q; 
assign mmr_status[47:32] = '0; 
assign mmr_status[63:48] = dff_status_cec_q; 
assign mmr_addr[31:0] = dff_addr_q; 
assign mmr_addr[63:32] = '0; 
assign mmr_info[3:0] = dff_info_aec_q; 
assign mmr_info[4] = dff_info_tag_q; 
assign mmr_info[63:5] = '0; 
endmodule
//`include "wing_m130_undefine.sv" 
`ifdef M130_CORE_SUPPORT_ICACHE 
`undef M130_CORE_SUPPORT_ICACHE 
`endif 
`ifdef M130_CORE_SUPPORT_ECC 
`undef M130_CORE_SUPPORT_ECC 
`endif 
`ifdef M130_CORE_ICACHE_SIZE 
`undef M130_CORE_ICACHE_SIZE 
`endif 
`ifdef IC_TAG_SRAM_RAS_W 
`undef IC_TAG_SRAM_RAS_W 
`endif 
`ifdef IC_DATA_SRAM_RAS_W 
`undef IC_DATA_SRAM_RAS_W 
`endif 
`ifdef ITCM_RAS_W 
`undef ITCM_RAS_W 
`endif 
`ifdef DTCM_RAS_W 
`undef DTCM_RAS_W 
`endif 
`ifdef MSS_EN_MMR 
`undef MSS_EN_MMR 
`endif 
