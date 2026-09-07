//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_seip_kdec #(
    parameter   [5:0]   p_FSRK_ID           = 6'h0
   ,parameter   [5:0]   p_SSRK_ID           = 6'h4
   ,parameter   [127:0] p_FSRK_DEFAULT      = 128'h12568920_ab5897df_86169132_8efcda66
   ,parameter   [127:0] p_SSRK_DEFAULT      = 128'hb068c120_950d8c61_08639896_1e65510a
)(
    input  wire                             clk          
   ,input  wire                             rst_n        
   ,input  wire                             i_scan_mode

   ,input  wire                             i_bc2kdf_go   
   ,input  wire                             i_bc2kdf_done   
   ,output wire                             o_dec_done   
   ,input  wire                             i_sel_root_key
   ,output wire                             o_crc_err_set    
   ,output wire                             o_crc_ok_set    
   ,input  wire                             i_bc2kdf_alg    
   ,input  wire [255:0]                     i_bc2kdf_ekey   
   ,input  wire [31:0]                      i_bc2kdf_crc    
   ,input  wire [63:0]                      i_clc_key
   ,input  wire [255:0]                     i_bc2kdf_mask    
   ,output wire [255:0]                     o_masked_key
    );

    localparam  [3:0]                       p_KEY_CNTR_THRES = p_SSRK_ID[3:0] 
                                                             + 4'd1;
    localparam  [2:0]   s_IDLE              = 3'd0
                       ,s_SKE_LODKEY        = 3'd1 
                       ,s_SKE_EXPKEY        = 3'd2
                       ,s_SKE_LODDAT        = 3'd3
                       ,s_SKE_DECDAT        = 3'd4
                       ,s_CHK_STUS          = 3'd5
                       ,s_DONE              = 3'd6;

    reg     [2:0]                           current_state       ;
    reg     [2:0]                           next_state          ;

    wire                                    w_ske_expkey_sel    ;
    wire                                    w_ske_decdat_sel    ;
    wire                                    w_ske_lodkey_sel    ;
    wire                                    w_ske_loddat_sel    ;

    wire                                    w_ske_1st_kw_sel    ;
    wire                                    w_ske_2nd_kw_sel    ;
    wire                                    w_ske_3rd_kw_sel    ;
    wire                                    w_ske_4th_kw_sel    ;
    wire                                    w_ske_1st_dw_sel    ;
    wire                                    w_ske_2nd_dw_sel    ;
    wire                                    w_ske_3rd_dw_sel    ;
    wire                                    w_ske_4th_dw_sel    ;
    wire                                    w_ske_5th_dw_sel    ;
    wire                                    w_ske_6th_dw_sel    ;
    wire                                    w_ske_7th_dw_sel    ;
    wire                                    w_ske_8th_dw_sel    ;

    wire    [3:0]                           w_wr_cntr_nxt       ;
    wire    [1:0]                           w_round_cntr_nxt    ;
    wire    [3:0]                           w_key_cntr_nxt      ;
    reg     [3:0]                           r_wr_cntr           ;
    reg     [1:0]                           r_round_cntr        ;
    reg     [3:0]                           r_key_cntr          ;

    wire                                    w_clr_vld           ;
    wire    [127:0]                         w_fsrk_nxt          ;
    wire    [127:0]                         w_ssrk_nxt          ;
    reg     [127:0]                         r_fsrk              ;
    reg     [127:0]                         r_ssrk              ;
    wire    [127:0]                         w_nvm_kek           ;
    wire                                    w_dec_fsrk_sel      ;
    wire                                    w_dec_ssrk_sel      ;
    wire    [255:0]                         w_nvm_key_pt_nxt    ;
    reg     [255:0]                         r_nvm_key_pt        ;
    wire    [3:0]                           w_ske_key_wren      ;
    wire    [3:0]                           w_ske_state_wren    ;
    wire                                    w_ske_keyexp_rdy    ;
    wire                                    w_last_round_sel    ;

    wire                                    w_crc_init          ;
    wire                                    w_crc_done          ;
    wire                                    w_crc_data_in_vld   ;
    wire                                    w_crc_err_set_nxt   ;
    wire                                    w_crc_ok_set_nxt    ;
    wire                                    w_crc_rdy_nxt       ;
    reg                                     r_crc_err_set       ;
    reg                                     r_crc_ok_set        ;
    reg                                     r_crc_rdy           ;

    wire    [255:0]                         w_crc_data_in       ;
    wire    [31:0]                          w_key_crc           ;
    wire    [31:0]                          w_dut_crc           ;
    wire    [31:0]                          w_ref_crc           ;

    reg     [127:0]                         r_rtl_key_1d        ;
    reg     [127:0]                         r_rtl_key_2d        ;

    wire    [3:0]                           w_ske_alg           ;
    wire                                    w_ske_data_in_vld   ;
    wire                                    w_load_newdata_vld  ;
    wire                                    w_ske_keyexp_go     ;
    wire                                    w_ske_data_vld      ;
    reg                                     r_ske_data_vld_dly  ;

    wire    [31:0]                          w_ske_data_in       ;
    wire    [127:0]                         w_ske_data_out      ;
    wire    [127:0]                         w_ske_data_out_pre  ;

    assign  w_ske_expkey_sel    = current_state == s_SKE_EXPKEY     ;
    assign  w_ske_decdat_sel    = current_state == s_SKE_DECDAT     ;
    assign  w_ske_lodkey_sel    = current_state == s_SKE_LODKEY     ;
    assign  w_ske_loddat_sel    = current_state == s_SKE_LODDAT     ;

    assign  w_last_round_sel    = r_round_cntr == 2'd1 ; 
    assign  w_load_newdata_vld  = current_state == s_SKE_DECDAT 
                               && next_state == s_SKE_LODDAT;
    assign  w_round_cntr_nxt    = i_bc2kdf_go       ? 2'd0      :
                                  o_dec_done        ? 2'd0      :
                                w_load_newdata_vld  ? r_round_cntr + 2'd1 :
                                                      r_round_cntr;

    assign  w_key_cntr_nxt      = w_clr_vld         ? 4'd0              :
                   (r_key_cntr == p_KEY_CNTR_THRES) ? p_KEY_CNTR_THRES  : 
                                  o_dec_done        ? r_key_cntr + 4'd1 : 
                                                      r_key_cntr        ;

    assign  w_wr_cntr_nxt       = i_bc2kdf_go       ? 4'd0              :
                                  w_ske_expkey_sel  ? 4'd0              :
                                  w_ske_decdat_sel  ? 4'd0              :
                                  w_ske_lodkey_sel  ? r_wr_cntr + 4'd1  :
                                  w_ske_loddat_sel  ? r_wr_cntr + 4'd1  :
                                                      r_wr_cntr         ;

    assign  w_clr_vld           = i_bc2kdf_done;

    always @(posedge clk or negedge rst_n)
        if(~rst_n) begin
            r_ske_data_vld_dly      <= 1'b0;
            r_key_cntr              <= 4'h0;
            r_round_cntr            <= 2'd0;
            r_wr_cntr               <= 4'd0;
        end
        else begin
            r_ske_data_vld_dly      <=  w_ske_data_vld;
            r_key_cntr              <=  w_key_cntr_nxt;
            r_round_cntr            <=  w_round_cntr_nxt ;
            r_wr_cntr               <=  w_wr_cntr_nxt;
        end

    assign  w_dec_fsrk_sel      = r_key_cntr == p_FSRK_ID       ;
    assign  w_dec_ssrk_sel      = r_key_cntr == 6'h1            ;

    assign  w_fsrk_nxt          = w_clr_vld     ? {i_clc_key,i_clc_key} :
                 (w_dec_fsrk_sel && o_dec_done) ? r_nvm_key_pt[127:0]:
                                                  r_fsrk             ;

    assign  w_ssrk_nxt          = w_clr_vld     ? {i_clc_key,i_clc_key} :
                 (w_dec_ssrk_sel && o_dec_done) ? r_nvm_key_pt[127:0]:
                                                  r_ssrk             ;

    wire    rtlkey_sel      = (r_key_cntr == 3'h0);
    wire    lev1key_sel     = i_sel_root_key ;

    wire [127:0] rtlkey_mux     = i_scan_mode ? `OSR_SCAN_RTL_KEY : `OSR_RTL_KEY ;

    assign  w_nvm_kek           = rtlkey_sel  ? rtlkey_mux    : 
                                  lev1key_sel ? r_fsrk        : r_ssrk ;

    assign  w_nvm_key_pt_nxt    = 
(r_round_cntr == 4'd0) && r_ske_data_vld_dly ? {128'b0,w_ske_data_out}:
(r_round_cntr == 4'd1) && r_ske_data_vld_dly ? {w_ske_data_out,r_nvm_key_pt[127:0]}:
                                               r_nvm_key_pt ;
    always @(posedge clk or negedge rst_n)
        if(~rst_n) begin
            r_fsrk                  <= p_FSRK_DEFAULT   ;
            r_ssrk                  <= p_SSRK_DEFAULT   ;
            r_nvm_key_pt            <= 256'd0           ;
        end
        else begin
            r_fsrk                  <=  w_fsrk_nxt    ;
            r_ssrk                  <=  w_ssrk_nxt    ;
            r_nvm_key_pt            <=  w_nvm_key_pt_nxt;
        end

    assign  o_masked_key        = r_nvm_key_pt ^ i_bc2kdf_mask;

    assign  w_crc_data_in       = i_bc2kdf_ekey;
    assign  w_crc_init          = i_bc2kdf_go  ;
    assign  w_crc_data_in_vld   = w_ske_keyexp_rdy;
    assign  w_crc_rdy_nxt       = o_dec_done ? 1'b0         :
                                  w_crc_done ? 1'b1         :
                                               r_crc_rdy    ; 
    assign  w_dut_crc           = w_key_crc    ;
    assign  w_ref_crc           = i_bc2kdf_crc ;

    wire    crc_ok           = (w_dut_crc == w_ref_crc);

    assign  w_crc_err_set_nxt   = w_crc_done & ~crc_ok;
    assign  o_crc_err_set       = r_crc_err_set ;

    assign  w_crc_ok_set_nxt    = w_crc_done & crc_ok ;
    assign  o_crc_ok_set        = r_crc_ok_set  ;

    always @(posedge clk or negedge rst_n)
        if(~rst_n) begin
            r_crc_rdy               <= 1'h0;
            r_crc_err_set           <= 1'h0;
            r_crc_ok_set            <= 1'h0;
        end
        else begin
            r_crc_rdy               <=  w_crc_rdy_nxt       ;
            r_crc_err_set           <=  w_crc_err_set_nxt   ;
            r_crc_ok_set            <=  w_crc_ok_set_nxt    ;
        end

    assign  w_ske_alg           = i_bc2kdf_alg ? 4'h1 : 4'h2;

    assign  w_ske_keyexp_go     = w_ske_lodkey_sel && (r_wr_cntr == 4'd3);
    assign  w_ske_data_in_vld   = w_ske_loddat_sel && (r_wr_cntr == 4'd3);

    assign  w_ske_key_wren      = w_ske_1st_kw_sel ? 4'h1   :
                                  w_ske_2nd_kw_sel ? 4'h2   :
                                  w_ske_3rd_kw_sel ? 4'h4   :
                                  w_ske_4th_kw_sel ? 4'h8   :
                                                     4'h0   ;
    assign  w_ske_state_wren    = w_ske_1st_dw_sel ? 4'h1   :
                                  w_ske_2nd_dw_sel ? 4'h2   :
                                  w_ske_3rd_dw_sel ? 4'h4   :
                                  w_ske_4th_dw_sel ? 4'h8   :
                                  w_ske_5th_dw_sel ? 4'h1   :
                                  w_ske_6th_dw_sel ? 4'h2   :
                                  w_ske_7th_dw_sel ? 4'h4   :
                                  w_ske_8th_dw_sel ? 4'h8   :
                                                     4'h0   ;

    assign  w_ske_1st_kw_sel    = w_ske_lodkey_sel && (r_wr_cntr == 4'd0);
    assign  w_ske_2nd_kw_sel    = w_ske_lodkey_sel && (r_wr_cntr == 4'd1);
    assign  w_ske_3rd_kw_sel    = w_ske_lodkey_sel && (r_wr_cntr == 4'd2);
    assign  w_ske_4th_kw_sel    = w_ske_lodkey_sel && (r_wr_cntr == 4'd3);
    assign  w_ske_1st_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd0) 
                               && (r_round_cntr == 2'd0);
    assign  w_ske_2nd_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd1)
                               && (r_round_cntr == 2'd0);
    assign  w_ske_3rd_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd2)
                               && (r_round_cntr == 2'd0);
    assign  w_ske_4th_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd3) 
                               && (r_round_cntr == 2'd0);
    assign  w_ske_5th_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd0)
                               && (r_round_cntr == 2'd1);
    assign  w_ske_6th_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd1)
                               && (r_round_cntr == 2'd1);
    assign  w_ske_7th_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd2)
                               && (r_round_cntr == 2'd1);
    assign  w_ske_8th_dw_sel    = w_ske_loddat_sel && (r_wr_cntr == 4'd3)
                               && (r_round_cntr == 2'd1);

    assign  w_ske_data_in       = w_ske_1st_kw_sel ? endian_reverse_32(w_nvm_kek[96 +: 32]     ):
                                  w_ske_2nd_kw_sel ? endian_reverse_32(w_nvm_kek[64 +: 32]     ):
                                  w_ske_3rd_kw_sel ? endian_reverse_32(w_nvm_kek[32 +: 32]     ):
                                  w_ske_4th_kw_sel ? endian_reverse_32(w_nvm_kek[ 0 +: 32]     ):
                                  w_ske_1st_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[ 96 +: 32]):
                                  w_ske_2nd_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[ 64 +: 32]):
                                  w_ske_3rd_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[ 32 +: 32]):
                                  w_ske_4th_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[  0 +: 32]):
                                  w_ske_5th_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[224 +: 32]):
                                  w_ske_6th_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[192 +: 32]):
                                  w_ske_7th_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[160 +: 32]):
                                  w_ske_8th_dw_sel ? endian_reverse_32(i_bc2kdf_ekey[128 +: 32]):
                                                     32'd0                   ;

    assign  o_dec_done          = current_state == s_DONE;

    always @ (posedge clk or negedge rst_n)
        if (!rst_n)
            current_state       <= s_IDLE       ;
        else
            current_state       <=  next_state;

    always @* begin
        next_state              = current_state;
        case (current_state) 
            s_IDLE  :
                if (i_bc2kdf_go)
                    next_state          = s_SKE_LODKEY  ;
                else
                    next_state          = s_IDLE        ;
            s_SKE_LODKEY    :
                if (r_wr_cntr == 4'd3)
                    next_state          = s_SKE_EXPKEY  ;
                else
                    next_state          = s_SKE_LODKEY  ;
            s_SKE_EXPKEY    :
                if (w_ske_keyexp_rdy)
                    next_state          = s_SKE_LODDAT  ;
                else
                    next_state          = s_SKE_EXPKEY  ;
            s_SKE_LODDAT    :
                if (r_wr_cntr == 4'd3)
                    next_state          = s_SKE_DECDAT  ;
                else
                    next_state          = s_SKE_LODDAT  ;
            s_SKE_DECDAT    :
                if (r_ske_data_vld_dly && w_last_round_sel)
                    next_state          = s_CHK_STUS    ;
                else if (r_ske_data_vld_dly && !w_last_round_sel)
                    next_state          = s_SKE_LODDAT  ;
                else
                    next_state          = s_SKE_DECDAT  ;
            s_CHK_STUS      :
                if (r_crc_rdy)
                    next_state          = s_DONE        ;
                else
                    next_state          = s_CHK_STUS    ;
            s_DONE          :
                    next_state          = s_IDLE        ;
        endcase
    end

    osr_bc_lp_alg_top u_bc_lp_alg_top
    (
      .clk                (clk                )
   ,  .rst_n              (rst_n              )
   ,  .i_cipher_alg       (w_ske_alg          ) 
   ,  .i_key_inv          (1'b1               )
   ,  .i_key_size         (2'd1               )
   ,  .i_keyexp_go        (w_ske_keyexp_go    )
   ,  .o_keyexp_busy      (                   )
   ,  .o_keyexp_rdy       (w_ske_keyexp_rdy   )
   ,  .i_state_inv        (1'b1               )
   ,  .i_state_vld        (w_ske_data_in_vld  )
   ,  .o_state_busy       (                   )
   ,  .o_state_vld        (w_ske_data_vld     )
   ,  .o_state            (w_ske_data_out_pre )
   ,  .i_din              (w_ske_data_in      )
   ,  .i_key_wren         (w_ske_key_wren     )
   ,  .i_state_wren       (w_ske_state_wren   )
   ,  .o_key              (                   )
    );

assign w_ske_data_out = endian_reverse_128(w_ske_data_out_pre);

    osr_crc32_256 u_crc32_256 (
    .clk                  (clk                )
   ,.rst_n                (rst_n              )
   ,.i_d                  (w_crc_data_in      )
   ,.i_init               (w_crc_init         )
   ,.i_valid              (w_crc_data_in_vld  )
   ,.o_done               (w_crc_done         )
   ,.o_crc                (w_key_crc          )  
    );

function    [127:0]      endian_reverse_128(
    input   [127:0]      i_data
    );

    endian_reverse_128 = {
        i_data[32*0+0*8+:8], i_data[32*0+1*8+:8], i_data[32*0+2*8+:8], i_data[32*0+3*8+:8],
        i_data[32*1+0*8+:8], i_data[32*1+1*8+:8], i_data[32*1+2*8+:8], i_data[32*1+3*8+:8],
        i_data[32*2+0*8+:8], i_data[32*2+1*8+:8], i_data[32*2+2*8+:8], i_data[32*2+3*8+:8],
        i_data[32*3+0*8+:8], i_data[32*3+1*8+:8], i_data[32*3+2*8+:8], i_data[32*3+3*8+:8]
    };

endfunction

function    [31:0]      endian_reverse_32(
    input   [31:0]      i_data
    );

    endian_reverse_32 = {
        i_data[32*0+0*8+:8], i_data[32*0+1*8+:8], i_data[32*0+2*8+:8], i_data[32*0+3*8+:8]
    };

endfunction

endmodule
