//========================================================================================
//         Copyright (C) 2024 Open Security Research Inc. - All Rights Reserved           
//----------------------------------------------------------------------------------------

//Level synchronization module built using Standard Cell in osr_sync_level.v
//              __    __    __    __    __    __    __    __   
//i_clk      __|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
//                    _________________________________________
//i_rst_n    ________|                                         
//                            _________________________________
//i_async[i] ________________|                                 
//                                      _______________________
//o_sync[i]  __________________________|                       
//
module osr_sync_level #(
    parameter   WIDTH = 1
)(
    input   wire                i_clk           ,
    input   wire                i_rst_n         ,

    input   wire [WIDTH-1:0]    i_async         ,
    output  wire [WIDTH-1:0]    o_sync          
);


wire [WIDTH-1:0]    w_sync;

generate 
genvar i;
for (i=0;i<WIDTH;i=i+1) begin
	osr_sync_cell u_sync(.clk (i_clk), .rstn(i_rst_n), .i_async (i_async[i]), .o_sync (w_sync[i]));
end
endgenerate

assign o_sync  = w_sync ;

endmodule 

