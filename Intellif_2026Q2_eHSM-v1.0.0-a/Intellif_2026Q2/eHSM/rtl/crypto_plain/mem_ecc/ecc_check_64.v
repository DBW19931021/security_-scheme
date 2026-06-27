

module ecc_check_64
  (

   input  wire [7:0]       ecc_i,
   input  wire [80:0]      data_i,
   input  wire [88:0]      xor_dedin,            // ECC for test rom data   
   output wire [7:0]       syndrome_o

  );

  genvar    mask_iter;
  genvar    bit_iter;


  wire       [7:0] one_hot   [7:0];
  wire [80:0] masks     [7:0];
  wire       [7:0] syndrome;

  localparam [31:0] odd_even = 32'sb00000000000000000000000000010100 ;

  wire [88:0] din = (xor_dedin ^ {ecc_i, data_i});

  generate
    for (mask_iter = 0; mask_iter < 8; mask_iter = mask_iter+1) begin : gen_syndrome
      for (bit_iter = 0; bit_iter < 8; bit_iter = bit_iter+1) begin : gen_mask_2
        if (mask_iter == bit_iter) begin : one
          assign one_hot[mask_iter][bit_iter] = 1'b1;
        end else begin : zero
          assign one_hot[mask_iter][bit_iter] = 1'b0;
        end
      end
      ecc_mask_64 #( .MASK          (mask_iter) ) u_ecc_mask (.masks_o(masks[mask_iter]));

      assign syndrome[mask_iter] = ^(din & {one_hot[mask_iter], masks[mask_iter]}) ^ odd_even[mask_iter];
    end
  endgenerate

  assign syndrome_o = syndrome;

endmodule


