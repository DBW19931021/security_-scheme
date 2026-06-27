

module ecc_repair_32
  (
   input  wire [48:0]  only_data_i,
   input  wire [6:0]   syndrome_i,
   input  wire [7:0]   repair_en_i,
   output wire         any_ecc_err_o,
   output wire         fatal_ecc_err_o,
   output wire [48:0]  repaired_data_o
  );

  genvar       mask_iter;
  genvar       bit_iter;

  wire           [6:                   0] one_hot    [6:0];
  wire     [48:17]  masks      [6:0];
  wire           [6:                   0] err_matrix [55:17];
  wire [55:17]  single_err;


  generate
    for (mask_iter = 0; mask_iter < 7; mask_iter = mask_iter+1) begin : gen_syndrome
      for (bit_iter = 0; bit_iter < 7; bit_iter = bit_iter+1) begin : gen_mask_2
        if (mask_iter == bit_iter) begin : one
          assign one_hot[mask_iter][bit_iter] = 1'b1;
        end else begin : zero
          assign one_hot[mask_iter][bit_iter] = 1'b0;
        end
      end
      ecc_mask_32_rep #( .MASK          (mask_iter) ) u_ecc_mask (.masks_o(masks[mask_iter]));
    end
  endgenerate

  generate
    for (bit_iter = 17; bit_iter < 56; bit_iter = bit_iter+1) begin : gen_mask_transpose_row
      for (mask_iter = 0; mask_iter < 7; mask_iter = mask_iter+1) begin : gen_mask_transpose_col
        if (bit_iter < 49) begin : from_mask
          assign err_matrix[bit_iter][mask_iter] = (masks[mask_iter][bit_iter]) ?  syndrome_i[mask_iter] :
                                                                                  ~syndrome_i[mask_iter];
        end else begin : from_one_hot
          assign err_matrix[bit_iter][mask_iter] = (one_hot[mask_iter][bit_iter-49]) ?  syndrome_i[mask_iter] :
                                                                                                             ~syndrome_i[mask_iter];
        end
      end

      assign single_err[bit_iter] = &(err_matrix[bit_iter]) & (repair_en_i != 8'h5A);
    end
  endgenerate

  assign any_ecc_err_o   = (|syndrome_i) & (|single_err) & (repair_en_i != 8'h5A);
  assign fatal_ecc_err_o = (|syndrome_i) & ~(|single_err) & (repair_en_i != 8'h5A);
  assign repaired_data_o = {(only_data_i[48:17] ^ single_err[48:17]),only_data_i[16:0]};

endmodule


