//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_sync_level_3 (
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
//                                            _________________
//o_sync     ________________________________|            

wire  sync0   ; 
wire  sync1   ; 

osr_dff u_dff0(.CK (i_clk), .RB (i_rst_n), .D (i_async), .Q (sync0 ));
osr_dff u_dff1(.CK (i_clk), .RB (i_rst_n), .D (sync0  ), .Q (sync1 ));
osr_dff u_dff2(.CK (i_clk), .RB (i_rst_n), .D (sync1  ), .Q (o_sync));

endmodule // osr_sync_cell_3 
