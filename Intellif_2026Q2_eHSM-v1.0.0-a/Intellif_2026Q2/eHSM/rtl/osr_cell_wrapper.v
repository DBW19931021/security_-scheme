module osr_ro_inv(input wire A, output wire Z);
//       ___     ___    
//A  ___|   |___|   |___
//   ___     ___     ___
//Z     |___|   |___|   
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        assign Z = ~A;
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        assign Z = ~A;
    `else                       // !! YOU MUST change the "INVM1D" below to you technology library standard cell which has the same function !!
        INVM1D OSR_DONTTOUCH_inv_cell(.A (A), .Z (Z));
    `endif
endmodule


module osr_inv(input wire A, output wire Z);
//       ___     ___    
//A  ___|   |___|   |___
//   ___     ___     ___
//Z     |___|   |___|   
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        assign Z = ~A;
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        assign Z = ~A;
    `else                       // !! YOU MUST change the "INVM1D" below to you technology library standard cell which has the same function !!
        INVM1D OSR_DONTTOUCH_inv_cell(.A (A), .Z (Z));
    `endif
endmodule


module osr_nand2(input wire A, input wire B, output wire Z);
//       ___     ___    
//A  ___|   |___|   |___
//           _______    
//B  _______|       |___
//   ___________     ___
//Z             |___|   
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        assign Z = ~(A & B);                                                                                                              
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        assign Z = ~(A & B);                                                                                                              
    `else                       // !! YOU MUST change the "ND2M1D" below to you technology library standard cell which has the same function !!
        ND2M1D OSR_DONTTOUCH_nand2_cell(.A (A), .B (B), .Z (Z));
    `endif
endmodule


module osr_xor2(input wire A, input wire B, output wire Z);
//       ___     ___    
//A  ___|   |___|   |___
//           _______    
//B  _______|       |___
//       _______        
//Z  ___|       |_______
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        assign Z = A ^ B;
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        assign Z = A ^ B;
    `else                       // !! YOU MUST change the "XOR2M4D" below to you technology library standard cell which has the same function !!
        XOR2M4D OSR_DONTTOUCH_xor2_cell(.A (A), .B (B), .Z (Z));
    `endif
endmodule


module osr_icg(input wire CK, input wire E, input wire SE, output wire GCK);
//Note: You must use a monospaced font to view the waveform in this file!
//        __    __    __    __    __    __    __    __    __   
//CK  ___|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
//        _____       ___________                              
//E   ___|     |_____|           |_____________________________
//        _____                         _________________      
//SE  ___|     |_______________________|                 |_____
//              __          __    __          __    __    __   
//GCK _________|  |________|  |__|  |________|  |__|  |__|  |__
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        reg ENL;
        wire en = E | SE;
        always @(*) begin
            if(~CK)
                ENL <= en;
        end
        assign GCK = CK & ENL;
    `elsif OSR_FPGA_CLOCK_GATING    // !! DO NOT define the OSR_FPGA when synthesis !!
        reg ENL;
        wire en = E | SE;
        always @(*) begin
            if(~CK)
                ENL <= en;
        end
        assign GCK = CK & ENL;
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        assign GCK = CK ;
    `else                       // !! YOU MUST change the "LAGCESM16D" below to you technology library standard cell which has the same function !!
        LAGCESM16D OSR_DONTTOUCH_icg_cell(.CK (CK), .E (E), .SE (SE), .GCK (GCK));
    `endif
endmodule


module osr_mux2(input wire A, input wire B, input wire S, output wire Z);
//       ___     ___     ___     ___    
//A  ___|   |___|   |___|   |___|   |___
//   ___     ___     ___     ___     ___
//B     |___|   |___|   |___|   |___|   
//                   ___________________
//S  _______________|                   
//       ___     _______     ___     ___
//Z  ___|   |___|       |___|   |___|   
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        assign Z =  S ? B : A;                                                                                                              
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        assign Z =  S ? B : A;                                                                                                              
    `else                       // !! YOU MUST change the "MUX2M2D" below to you technology library standard cell which has the same function !!
        MUX2M2D OSR_DONTTOUCH_mux2_cell(.A (A), .B (B), .S (S), .Z (Z));
    `endif
endmodule

module osr_dff(input wire CK, input wire RB, input wire D, output wire Q);
//        __    __    __    __    __    __    __    __    __   
//CK  ___|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
//                    _____________________________      
//RB  _______________|                             |___________
//        _____             _____       _______________________
//D   ___|     |___________|     |_____|                       
//                                _____       _____
//Q  ____________________________|     |_____|     |___________

    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        reg DFF;
        always @(posedge CK or negedge RB) begin
            if(!RB) begin
                DFF <= 1'h0;
            end else begin
                DFF <= D;
            end
        end
        assign Q = DFF;
    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        reg DFF;
        always @(posedge CK or negedge RB) begin
            if(!RB) begin
                DFF <= 1'h0;
            end else begin
                DFF <= D;
            end
        end
        assign Q = DFF;
    `else                       // !! YOU MUST change the "DFQRM2D" below to you technology library standard cell which has the same function !!
        DFQRM2D OSR_DONTTOUCH_dfqrm2d_cell(.CK (CK), .RB (RB), .D (D), .Q (Q));
    `endif
endmodule


module osr_sync_cell(
		input	wire	clk		,
		input	wire	rstn	,
		input 	wire	i_async	,
		output 	wire	o_sync
);
//           __    __    __    __    __    __    __    __   
//clk     __|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
//                 _________________________________________
//rstn    ________|                                         
//                           _______________________________
//i_async __________________|                                 
//                                   _______________________
//o_sync  __________________________|                       
//
    `ifdef OSR_SIM_CELL         // !! DO NOT define the OSR_SIM_CELL when synthesis !!
        reg  sync0    ; 
        reg  sync1    ; 
        
        wire  sync0_anxt = i_async ;
        wire  sync1_anxt = sync0   ;
        
        always @(posedge clk or negedge rstn) begin : sync
            if(!rstn) begin
                sync0         <= 1'b0 ; 
                sync1         <= 1'b0 ; 
            end else begin
                sync0         <= sync0_anxt    ; 
                sync1         <= sync1_anxt    ; 
            end
        end
        
        assign o_sync  = sync1 ;


    `elsif OSR_FPGA                 // !! DO NOT define the OSR_FPGA when synthesis !!
        reg  sync0    ; 
        reg  sync1    ; 
        
        wire  sync0_anxt = i_async ;
        wire  sync1_anxt = sync0   ;
        
        always @(posedge clk or negedge rstn) begin : sync
            if(!rstn) begin
                sync0         <= 1'b0 ; 
                sync1         <= 1'b0 ; 
            end else begin
                sync0         <= sync0_anxt    ; 
                sync1         <= sync1_anxt    ; 
            end
        end

        assign o_sync  = sync1 ;

    `else                       // !! YOU MUST instantiate you technology library standard cell which has the same function to replace the logic below !!
        reg  sync0    ; 
        reg  sync1    ; 
        
        wire  sync0_anxt = i_async ;
        wire  sync1_anxt = sync0   ;
        
        always @(posedge clk or negedge rstn) begin : sync
            if(!rstn) begin
                sync0         <= 1'b0 ; 
                sync1         <= 1'b0 ; 
            end else begin
                sync0         <= sync0_anxt    ; 
                sync1         <= sync1_anxt    ; 
            end
        end
        
        assign o_sync  = sync1 ;

    `endif
endmodule 



