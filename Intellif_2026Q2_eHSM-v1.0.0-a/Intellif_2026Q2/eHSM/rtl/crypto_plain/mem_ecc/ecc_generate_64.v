module ecc_generate_64
  (
   input  wire [80:0] data_i,
   input  wire [7:0]  tm_chkbits,         // test mode check bits
   input  wire [7:0]  tm_sel_ecc_code,    // test mode select signal   
   output wire [88:0] ecc_o
  );

  genvar mask_iter;

  wire [80:0] masks [7:0];
  wire       [7:0] ecc;

  localparam [31:0] odd_even = 32'sb00000000000000000000000000010100 ;

  generate
    for (mask_iter = 0; mask_iter < 8; mask_iter = mask_iter+1) begin : gen_ecc
    ecc_mask_64 #( .MASK          (mask_iter) ) u_ecc_mask (.masks_o(masks[mask_iter]));

     assign ecc[mask_iter] = ^(data_i & masks[mask_iter]) ^ odd_even[mask_iter];
    end
  endgenerate


  assign ecc_o = (tm_sel_ecc_code == 8'h5a) ? {tm_chkbits,data_i} : {ecc,data_i};

endmodule

