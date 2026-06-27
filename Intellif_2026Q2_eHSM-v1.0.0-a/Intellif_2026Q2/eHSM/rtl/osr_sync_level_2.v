//========================================================================================
//         Copyright (C) 2024 Open Security Research Inc. - All Rights Reserved           
//----------------------------------------------------------------------------------------
module osr_sync_level_2 (
    input   wire        i_clk           ,
    input   wire        i_rst_n         ,
    input   wire        i_async         ,
    output  wire        o_sync           
);

//Level synchronization module built using Standard Cell in osr_sync_level.v
//              __    __    __    __    __    __    __    __   
//i_clk      __|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
//                    _________________________________________
//i_rst_n    ________|                                    
//                            _________________________________
//i_async    ________________|                            
//                                      _______________________
//o_sync     __________________________|            

wire  sync0   ; 

osr_dff u_dff0(.CK (i_clk), .RB (i_rst_n), .D (i_async), .Q (sync0 ));
osr_dff u_dff1(.CK (i_clk), .RB (i_rst_n), .D (sync0  ), .Q (o_sync));

endmodule // osr_sync_cell_2 
//========================================================================================

