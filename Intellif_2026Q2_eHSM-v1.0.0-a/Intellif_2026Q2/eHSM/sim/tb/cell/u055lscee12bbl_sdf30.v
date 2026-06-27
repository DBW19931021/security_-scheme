`default_nettype wire
`celldefine
module INVM1D( Z, A );
input A;
output Z;

   `ifdef FUNCTIONAL  //  functional //

	INVM1D_func INVM1D_behav_inst(.Z(Z),.A(A));

   `else

	INVM1D_func INVM1D_inst(.Z(Z),.A(A));


	// spec_gates_begin


	// spec_gates_end



   specify

	// specify_block_begin 

	// comb arc A --> Z
	 (A => Z) = (1.0,1.0);

	// specify_block_end 

   endspecify

   `endif 

endmodule
`endcelldefine

`celldefine
module ND2M1D( Z, A, B );
input A, B;
output Z;

   `ifdef FUNCTIONAL  //  functional //

	ND2M1D_func ND2M1D_behav_inst(.Z(Z),.A(A),.B(B));

   `else

	ND2M1D_func ND2M1D_inst(.Z(Z),.A(A),.B(B));


	// spec_gates_begin


	// spec_gates_end



   specify

	// specify_block_begin 

	// comb arc A --> Z
	 (A => Z) = (1.0,1.0);

	// comb arc B --> Z
	 (B => Z) = (1.0,1.0);

	// specify_block_end 

   endspecify

   `endif 

endmodule
`endcelldefine

`celldefine
module XOR2M4D( Z, A, B );
input A, B;
output Z;

   `ifdef FUNCTIONAL  //  functional //

	XOR2M4D_func XOR2M4D_behav_inst(.Z(Z),.A(A),.B(B));

   `else

	XOR2M4D_func XOR2M4D_inst(.Z(Z),.A(A),.B(B));


	// spec_gates_begin


	// spec_gates_end



   specify

	// specify_block_begin 

	ifnone
	// comb arc posedge A --> (Z:A)
	 (posedge A => (Z:A)) = (1.0,1.0);

	ifnone
	// comb arc negedge A --> (Z:A)
	 (negedge A => (Z:A)) = (1.0,1.0);

	ifnone
	// comb arc posedge B --> (Z:B)
	 (posedge B => (Z:B)) = (1.0,1.0);

	ifnone
	// comb arc negedge B --> (Z:B)
	 (negedge B => (Z:B)) = (1.0,1.0);

	// specify_block_end 

   endspecify

   `endif 

endmodule
`endcelldefine

`celldefine
module DFQRM2D( Q, CK, D, RB );
input CK, D, RB;
output Q;
reg notifier;

   `ifdef FUNCTIONAL  //  functional //

	DFQRM2D_func DFQRM2D_behav_inst(.Q(Q),.CK(CK),.D(D),.RB(RB),.notifier(notifier));

   `else

	wire CK$delay ;

	wire D$delay ;

	wire RB$delay ;

	DFQRM2D_func DFQRM2D_inst(.Q(Q),.CK(CK$delay),.D(D$delay),.RB(RB$delay),.notifier(notifier));


	// spec_gates_begin


	not MGM_G0(MGM_W0,D$delay);


	and MGM_G1(ENABLE_NOT_D_AND_RB,RB$delay,MGM_W0);


	and MGM_G2(ENABLE_D_AND_RB,RB$delay,D$delay);


	buf MGM_G3(ENABLE_RB,RB$delay);


	not MGM_G4(MGM_W1,CK$delay);


	not MGM_G5(MGM_W2,D$delay);


	and MGM_G6(ENABLE_NOT_CK_AND_NOT_D,MGM_W2,MGM_W1);


	not MGM_G7(MGM_W3,CK$delay);


	and MGM_G8(ENABLE_NOT_CK_AND_D,D$delay,MGM_W3);


	not MGM_G9(MGM_W4,D$delay);


	and MGM_G10(ENABLE_CK_AND_NOT_D,MGM_W4,CK$delay);


	and MGM_G11(ENABLE_CK_AND_D,D$delay,CK$delay);


	// spec_gates_end



   specify

	// specify_block_begin 

	// seq arc CK --> Q
	(posedge CK => (Q : D))  = (1.0,1.0);

	// seq arc RB --> Q
	(RB => Q)  = (1.0,1.0);

	$width(negedge CK &&& (ENABLE_NOT_D_AND_RB === 1'b1)
		,1.0,0,notifier);

	$width(posedge CK &&& (ENABLE_NOT_D_AND_RB === 1'b1)
		,1.0,0,notifier);

	$width(negedge CK &&& (ENABLE_D_AND_RB === 1'b1)
		,1.0,0,notifier);

	$width(posedge CK &&& (ENABLE_D_AND_RB === 1'b1)
		,1.0,0,notifier);

	// setuphold D- CK-LH
	$setuphold(posedge CK &&& (ENABLE_RB === 1'b1),
		posedge D &&& (ENABLE_RB === 1'b1),
		1.0,1.0,notifier,,,CK$delay,D$delay);

	// setuphold D- CK-LH
	$setuphold(posedge CK &&& (ENABLE_RB === 1'b1),
		negedge D &&& (ENABLE_RB === 1'b1),
		1.0,1.0,notifier,,,CK$delay,D$delay);

	// recrem RB-CK-posedge
	$recrem(posedge RB,posedge CK,1.0,1.0,notifier,,,RB$delay,CK$delay);

	$width(negedge RB &&& (ENABLE_NOT_CK_AND_NOT_D === 1'b1)
		,1.0,0,notifier);

	$width(negedge RB &&& (ENABLE_NOT_CK_AND_D === 1'b1)
		,1.0,0,notifier);

	$width(negedge RB &&& (ENABLE_CK_AND_NOT_D === 1'b1)
		,1.0,0,notifier);

	$width(negedge RB &&& (ENABLE_CK_AND_D === 1'b1)
		,1.0,0,notifier);

	// mpw CK_lh 
	$width(posedge CK,1.0,0,notifier);

	// mpw CK_hl 
	$width(negedge CK,1.0,0,notifier);

	// mpw RB_hl 
	$width(negedge RB,1.0,0,notifier);

	// specify_block_end 

   endspecify

   `endif 

endmodule
`endcelldefine


`celldefine
module LAGCESM16D( GCK, CK, E, SE);
input CK, E, SE;
output GCK;
reg notifier;

   `ifdef FUNCTIONAL  //  functional //

   `else

	wire CK$delay ;
	wire E$delay ;
	wire SE$delay ;

	LAGCESM16D_func LAGCESM16D_inst(.CK(CK$delay),.E(E$delay),.GCK(GCK),.SE(SE$delay),.notifier(notifier));

   `endif 



   `ifdef FUNCTIONAL  //  functional //

	LAGCESM16D_func LAGCESM16D_inst(.CK(CK),.E(E),.GCK(GCK),.SE(SE),.notifier(notifier));

   `endif 


   `ifdef FUNCTIONAL // functional  //

   `else


	// spec_gates_begin


	not MGM_AG0(MGM_W0,E$delay);


	not MGM_AG1(MGM_W1,SE$delay);


	and MGM_AG2(ENABLE_NOT_E_AND_NOT_SE,MGM_W1,MGM_W0);


	not MGM_AG3(MGM_W2,E$delay);


	and MGM_AG4(ENABLE_NOT_E_AND_SE,SE$delay,MGM_W2);


	not MGM_AG5(MGM_W3,SE$delay);


	and MGM_AG6(ENABLE_E_AND_NOT_SE,MGM_W3,E$delay);


	and MGM_AG7(ENABLE_E_AND_SE,SE$delay,E$delay);


// specify block begins

   specify

	if(E===1'b0 && SE===1'b1)
	// arc CK --> GCK
	 (CK => GCK) = (1.0,1.0);

	if(E===1'b1 && SE===1'b0)
	// arc CK --> GCK
	 (CK => GCK) = (1.0,1.0);

	if(E===1'b1 && SE===1'b1)
	// arc CK --> GCK
	 (CK => GCK) = (1.0,1.0);

	ifnone
	// arc CK --> GCK
	 (CK => GCK) = (1.0,1.0);

	if(E===1'b0 && SE===1'b0)
	// arc CK --> GCK
	 (negedge CK => (GCK : E))  = (1.0,1.0);

	$width(negedge CK &&& (ENABLE_NOT_E_AND_NOT_SE === 1'b1)
		,1.0,0,notifier);
	$width(negedge CK &&& (ENABLE_NOT_E_AND_SE === 1'b1)
		,1.0,0,notifier);
	$width(negedge CK &&& (ENABLE_E_AND_NOT_SE === 1'b1)
		,1.0,0,notifier);
	$width(negedge CK &&& (ENABLE_E_AND_SE === 1'b1)
		,1.0,0,notifier);
	$width(negedge CK,1.0,0,notifier);

	// setuphold E- CK-LH
	$setuphold(posedge CK,negedge E &&& (SE === 1'b0),1.0,1.0,notifier,,,CK$delay,E$delay);

	// setuphold E- CK-LH
	$setuphold(posedge CK,posedge E &&& (SE === 1'b0),1.0,1.0,notifier,,,CK$delay,E$delay);

	// setuphold SE- CK-LH
	$setuphold(posedge CK,negedge SE &&& (E === 1'b0),1.0,1.0,notifier,,,CK$delay,SE$delay);

	// setuphold SE- CK-LH
	$setuphold(posedge CK,posedge SE &&& (E === 1'b0),1.0,1.0,notifier,,,CK$delay,SE$delay);

   endspecify

   `endif 

endmodule
`endcelldefine

`celldefine
module MUX2M2D( Z, A, B, S );
input A, B, S;
output Z;

   `ifdef FUNCTIONAL  //  functional //

	MUX2M2D_func MUX2M2D_behav_inst(.Z(Z),.A(A),.B(B),.S(S));

   `else

	MUX2M2D_func MUX2M2D_inst(.Z(Z),.A(A),.B(B),.S(S));


	// spec_gates_begin


	// spec_gates_end



   specify

	// specify_block_begin 

	if(B===1'b0)
	// comb arc A --> Z
	 (A => Z) = (1.0,1.0);

	if(B===1'b1)
	// comb arc A --> Z
	 (A => Z) = (1.0,1.0);

	ifnone
	// comb arc A --> Z
	 (A => Z) = (1.0,1.0);

	if(A===1'b0)
	// comb arc B --> Z
	 (B => Z) = (1.0,1.0);

	if(A===1'b1)
	// comb arc B --> Z
	 (B => Z) = (1.0,1.0);

	ifnone
	// comb arc B --> Z
	 (B => Z) = (1.0,1.0);

	ifnone
	// comb arc posedge S --> (Z:S)
	 (posedge S => (Z:S)) = (1.0,1.0);

	ifnone
	// comb arc negedge S --> (Z:S)
	 (negedge S => (Z:S)) = (1.0,1.0);

	// specify_block_end 

   endspecify

   `endif 

endmodule
`endcelldefine

`celldefine
module DFQRM2D_func( Q, CK, D, RB,notifier);
input CK, D, RB;
output Q;
input notifier;

	not MGM_BG_0(MGM_C,RB);

	MGM_IQ_FF_UDP(IQ,MGM_C,1'b0,CK,D,notifier);

	MGM_IQN_FF_UDP(IQN,MGM_C,1'b0,CK,D,notifier);

	buf MGM_BG_1(Q,IQ);
endmodule
`endcelldefine
//--------------------------------------------------------

`celldefine
module INVM1D_func( Z, A);
input A;
output Z;

	not MGM_BG_0(Z,A);
endmodule
`endcelldefine


`celldefine
module ND2M1D_func( Z, A, B);
input A, B;
output Z;

	ADCSIOM2D_udp_0(Z,A,B); 
endmodule
`endcelldefine


`celldefine
module XOR2M4D_func( Z, A, B);
input A, B;
output Z;

	ADHM1D_udp_1(Z,A,B); 
endmodule
`endcelldefine

`celldefine
module LAGCESM16D_func( GCK, CK, E, SE,notifier);
input CK, E, SE;
output GCK;
input notifier;
 
        OR2M0D_udp_0(MGM_EN,E,SE); 
        not NGN_n0 (CKB, CK);
        MGM_IQ_LATCH_UDP(ENL,1'b0,1'b0,CKB,MGM_EN,notifier);
	ADHM1D_udp_0(GCK,CK,ENL); 
endmodule
`endcelldefine

`celldefine
module MUX2M2D_func( Z, A, B, S);
input A, B, S;
output Z;

	CKMUX2M12D_udp_0(Z,A,S,B); 
endmodule
`endcelldefine



//===========================================

primitive ADCSIOM2D_udp_0(CO0B,A, B);
  output CO0B;
  input A, B;
  table
  //A, B: CO0B
    0  ?: 1;
    ?  0: 1;
    1  1: 0;
  endtable
endprimitive


primitive ADHM1D_udp_1(S,A, B);
  output S;
  input A, B;
  table
  //A, B: S
    1  0: 1;
    0  1: 1;
    1  1: 0;
    0  0: 0;
  endtable
endprimitive


primitive OR2M0D_udp_0(Z,A, B);
  output Z;
  input A, B;
  table
  //A, B: Z
    1  ?: 1;
    ?  1: 1;
    0  0: 0;
  endtable
endprimitive


primitive MGM_IQ_LATCH_UDP(Q,C,P,CK,D,N);
output Q;
reg Q;
input C,P,CK,D,N; 
table 
//  C  P  CK  D  N :  Q  :  Q 
    ?  ?  0  *  ?  :  ?  :  -;   // No change CK=0
    ?  0  1  0  ?  :  ?  :  0;   // Latch 0
    ?  0  *  0  ?  :  0  :  0;   // reduce pessimism when D=0
    1  ?  ?  ?  ?  :  ?  :  0;   // Clear : C dominate P
    0  ?  1  1  ?  :  ?  :  1;   // Latch 1
    0  ?  *  1  ?  :  1  :  1;   // reduce pessimism when D=1
    0  1  ?  ?  ?  :  ?  :  1;   // Preset
    *  0  0  ?  ?  :  0  :  0;   // reduce clear pessimism
    *  0  ?  0  ?  :  0  :  0;   // reduce clear pessimism
    0  *  0  ?  ?  :  1  :  1;   // reduce preset pessimism
    0  *  ?  1  ?  :  1  :  1;   // reduce preset pessimism
//  ?  ?  ?  ?  *  :  ?  :  x;   // notifier
                  
endtable
endprimitive

primitive ADHM1D_udp_0(CO,A, B);
  output CO;
  input A, B;
  table
  //A, B: CO
    1  1: 1;
    0  ?: 0;
    ?  0: 0;
  endtable
endprimitive

primitive CKMUX2M12D_udp_0(Z,A, S, B);
  output Z;
  input A, S, B;
  table
  //A, S, B: Z
    1  0  ?: 1;
    ?  1  1: 1;
    0  0  ?: 0;
    ?  1  0: 0;
    1  ?  1: 1;
    0  ?  0: 0;
  endtable
endprimitive

primitive MGM_IQ_FF_UDP(Q,C,P,CK,D,N);
output Q;
reg Q;
input C,P,CK,D,N; 
table 
//  C  P  CK  D  N :  Q  :  Q 
    ?  ?  n  ?  ?  :  ?  :  -;  // no changes on neg CK
    ?  0  r  0  ?  :  ?  :  0;  // CK in 0
    ?  0  p  0  ?  :  0  :  0;  // reduce pessimism D=0
    1  ?  ?  ?  ?  :  ?  :  0;  // clear: C dominate P
    0  ?  r  1  ?  :  ?  :  1;  // CK in 1
    0  ?  p  1  ?  :  1  :  1;  // reduce pessimism D=1
    0  1  ?  ?  ?  :  ?  :  1;  // preset
    ?  ?  b  *  ?  :  ?  :  -;  // ignore D change on steady CK
    *  0  b  ?  ?  :  0  :  0;  // reduce clear pessimism
    *  0  x  0  ?  :  0  :  0;  // reduce clear pessimism
    0  *  b  ?  ?  :  1  :  1;  // reduce preset pessimism
    0  *  x  1  ?  :  1  :  1;  // reduce preset pessimism
//  ?  ?  ?  ?  *  :  ?  :  x;  // notifier change
                  
endtable
endprimitive

primitive MGM_IQN_FF_UDP(QN,C,P,CK,D,N);
output QN;
reg QN;
input C,P,CK,D,N; 
table 
//C  P  CK  D  N :  QN  :  QN 
  ?  ?  n  ?  ?  :  ?  :  -;  // no changes on neg CK
  ?  0  r  0  ?  :  ?  :  1;  // CK in 0
  ?  0  p  0  ?  :  1  :  1;  // reduce pessimism D=0
  1  ?  ?  ?  ?  :  ?  :  1;  // clear: C dominate P
  0  ?  r  1  ?  :  ?  :  0;  // CK in 1
  0  ?  p  1  ?  :  0  :  0;  // reduce pessimism D=1
  0  1  ?  ?  ?  :  ?  :  0;  // preset
  ?  ?  b  *  ?  :  ?  :  -;  // ignore D change on steady CK
  *  0  b  ?  ?  :  1  :  1;  // reduce clear pessimism
  *  0  x  0  ?  :  1  :  1;  // reduce clear pessimism
  0  *  b  ?  ?  :  0  :  0;  // reduce preset pessimism
  0  *  x  1  ?  :  0  :  0;  // reduce preset pessimism
//?  ?  ?  ?  *  :  ?  :  x;  // notifier change
                  
endtable
endprimitive

`default_nettype none
