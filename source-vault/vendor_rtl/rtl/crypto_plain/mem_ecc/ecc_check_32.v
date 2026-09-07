

module ecc_check_32
  (

   input  wire [6:0]    ecc_i,
   input  wire [48:0]   data_i,
   input  wire [55:0]   xor_dedin,            // ECC for test rom data
   output wire [6:0]    syndrome_o

  );

  genvar    mask_iter;
  genvar    bit_iter;


  wire       [6:0] one_hot   [6:0];
  wire [48:0] masks     [6:0];
  wire       [6:0] syndrome;

  localparam [31:0] odd_even = 32'sb00000000000000000000000000000110 ;
  
  wire[55:0] din = (xor_dedin ^ {ecc_i, data_i});

  generate
    for (mask_iter = 0; mask_iter < 7; mask_iter = mask_iter+1) begin : gen_syndrome
      for (bit_iter = 0; bit_iter < 7; bit_iter = bit_iter+1) begin : gen_mask_2
        if (mask_iter == bit_iter) begin : one
          assign one_hot[mask_iter][bit_iter] = 1'b1;
        end else begin : zero
          assign one_hot[mask_iter][bit_iter] = 1'b0;
        end
      end
      ecc_mask_32 #( .MASK          (mask_iter) ) u_ecc_mask (.masks_o(masks[mask_iter]));

      assign syndrome[mask_iter] = ^(din & {one_hot[mask_iter], masks[mask_iter]}) ^ odd_even[mask_iter];
    end
  endgenerate

  assign syndrome_o = syndrome;

endmodule


