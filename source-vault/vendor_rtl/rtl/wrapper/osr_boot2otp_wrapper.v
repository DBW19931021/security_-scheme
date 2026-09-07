//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_boot2otp_wrapper (
    input  wire           i_clk                 ,
    input  wire           i_rst_n               ,

    input  wire           i_otp_en              ,
    input  wire [31:0]    i_otp_addr            ,
    output wire           o_otp_vld             ,
    output wire [31:0]    o_otp_data            ,
    input  wire           i_hw_boot_done_pulse  ,

    output wire           o_hsel                ,
    output wire [31:0]    o_haddr               ,
    output wire [2:0]     o_hsize               ,
    output wire [1:0]     o_htrans              ,
    output wire           o_hwrite              ,
    output wire [31:0]    o_hwdata              ,
    output wire           o_hready              ,
    output wire           o_hmastlock           ,
    output wire [2:0]     o_hburst              , 
    output wire [3:0]     o_hprot               ,

    input  wire [31:0]    i_hrdata              ,
    input  wire [1:0]     i_hresp               ,
    input  wire           i_hreadyout                 
);

osr_ram2ahb u_r2h (
    .i_clk          ( i_clk       ), 
    .i_rst_n        ( i_rst_n     ), 
    .i_otp_en       ( i_otp_en    ), 
    .i_otp_addr     ( i_otp_addr  ), 
    .o_otp_vld      ( o_otp_vld   ), 
    .o_otp_data     ( o_otp_data  ), 
    .o_hsel         ( o_hsel      ), 
    .o_haddr        ( o_haddr     ), 
    .o_hsize        ( o_hsize     ), 
    .o_htrans       ( o_htrans    ), 
    .o_hwrite       ( o_hwrite    ), 
    .o_hwdata       ( o_hwdata    ), 
    .o_hready       ( o_hready    ), 
    .o_hmastlock    ( o_hmastlock ), 
    .o_hburst       ( o_hburst    ), 
    .o_hprot        ( o_hprot     ), 
    .i_hrdata       ( i_hrdata    ), 
    .i_hresp        ( i_hresp     ), 
    .i_hreadyout    ( i_hreadyout )  
    ); 

endmodule 
