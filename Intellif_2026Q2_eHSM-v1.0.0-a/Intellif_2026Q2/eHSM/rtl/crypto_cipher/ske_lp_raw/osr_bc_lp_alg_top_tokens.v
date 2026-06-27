
/*
                    instances: 0
                        nodes: 37 (0)
                  node widths: 2284 (0)
                   contassign:  24 (0)
                        ports: 10 (0)
                        ports: 4 (0)
                 portconnects: 8 (0)
*/

//`portcoerce
//`inline
//`timescale 1 ns / 1 ns
module Neda64(Ugda64, Ahda64, Jhda64, Thda64, Cida64, Lida64, Sida64, Ajda64,
Ijda64, Pjda64);

parameter Dgda64 = 0;

input Ugda64;
input [1:0] Ahda64;
input Jhda64;
input Thda64;
input Cida64;
input [127:0] Lida64;
input [127:0] Sida64;
output [127:0] Ajda64;
output [31:0] Ijda64;
input [31:0] Pjda64;

wire Wjda64;
wire Jkda64;
wire Wkda64;
wire Jlda64;
wire Wlda64;
wire [127:0] Gmda64;
wire [127:0] Rmda64;
wire [127:0] Zmda64;
wire [127:0] Lnda64;
wire [31:0] Unda64;
wire [31:0] Foda64;
wire [31:0] Noda64;
wire [31:0] Zoda64;
wire [127:0] Ipda64;
wire [127:0] Tpda64;
wire [127:0] Bqda64;
wire [127:0] Nqda64;
wire [31:0] Wqda64;
wire [31:0] Lrda64;
wire [31:0] Asda64;
wire [31:0] Psda64;
wire [127:0] Etda64;
wire [127:0] Ttda64;
wire [127:0] Iuda64;
wire [127:0] Xuda64;
wire Mvda64;
wire [31:0] Yvda64;

assign Mvda64 = (((!Ugda64) && Cida64) | (Ugda64 && Thda64));
assign Nqda64 = (Sida64 ^ Lida64);
assign Zmda64 = Nqda64;
assign Ipda64 = (((!Ugda64) && Cida64) ? Rmda64 : (((!Ugda64) && (!Cida64)) ? 
Rmda64 : 128'b0));
assign Tpda64 = (Ipda64 ^ Lida64);
assign Gmda64 = Sida64;
assign Wjda64 = ((!Jhda64) && (Ahda64 == 2'b0));
assign Jkda64 = ((!Jhda64) && (Ahda64 == 2'b1));
assign Wkda64 = ((!Jhda64) && (Ahda64 == 2'h2));
assign Jlda64 = ((!Jhda64) && (Ahda64 == 2'h3));
assign Wlda64 = Jhda64;
assign Wqda64 = {Sida64[127-:8], Sida64[87-:8], Sida64[47-:8], Sida64[7-:8]};
assign Lrda64 = {Sida64[95-:8], Sida64[55-:8], Sida64[15-:8], Sida64[103-:8]};
assign Asda64 = {Sida64[63-:8], Sida64[23-:8], Sida64[111-:8], Sida64[71-:8]};
assign Psda64 = {Sida64[31-:8], Sida64[119-:8], Sida64[79-:8], Sida64[39-:8]};
assign Ijda64 = ((Ugda64 && Mvda64) ? Noda64 : (Ugda64 ? Zoda64 : (Wjda64 ? 
Wqda64 : (Jkda64 ? Lrda64 : (Wkda64 ? Asda64 : (Jlda64 ? Psda64 : 32'b0))))));
assign Unda64 = Pjda64;
assign Noda64 = (Wjda64 ? Rmda64[127-:32] : (Jkda64 ? Rmda64[95-:32] : (Wkda64 ?
Rmda64[63-:32] : (Jlda64 ? Rmda64[31-:32] : 32'b0))));
assign Yvda64 = (Mvda64 ? Unda64 : (Ugda64 ? Pjda64 : Foda64));
assign Etda64 = {Yvda64[31-:8], Sida64[119-:32], Yvda64[23-:8], Sida64[79-:32],
Yvda64[15-:8], Sida64[39-:32], Yvda64[7-:8]};
assign Ttda64 = {Sida64[127-:24], Yvda64[7-:8], Yvda64[31-:8], Sida64[87-:32],
Yvda64[23-:8], Sida64[47-:32], Yvda64[15-:8], Sida64[7-:8]};
assign Iuda64 = {Sida64[127-:16], Yvda64[15-:8], Sida64[103-:32], Yvda64[7-:8],
Yvda64[31-:8], Sida64[55-:32], Yvda64[23-:8], Sida64[15-:16]};
assign Xuda64 = {Sida64[127-:8], Yvda64[23-:8], Sida64[111-:32], Yvda64[15-:8],
Sida64[71-:32], Yvda64[7-:8], Yvda64[31-:8], Sida64[23-:24]};
assign Ajda64 = (Wjda64 ? Etda64 : (Jkda64 ? Ttda64 : (Wkda64 ? Iuda64 : (Jlda64
? Xuda64 : ((Wlda64 && (!Ugda64)) ? Tpda64 : (((Wlda64 && Ugda64) && Cida64) ? 
Zmda64 : ((Wlda64 && Ugda64) ? Lnda64 : 128'b0)))))));

Mwda64 Gxda64( .Yxda64 (Zmda64),  .Eyda64 (Lnda64));
Kyda64 Czda64( .Yxda64 (Gmda64),  .Eyda64 (Rmda64));
Szda64 P0ea64( .K1ea64 (Unda64),  .T1ea64 (Foda64));
C2ea64 B3ea64( .Y3ea64 (Noda64),  .I4ea64 (Zoda64));
endmodule

/*
                    instances: 0
                        nodes: 17 (0)
                  node widths: 624 (0)
                   contassign:  14 (0)
                        ports: 7 (0)
*/

//`timescale 1 ns / 1 ns
module X6ea64(U8ea64, Ugda64, D9ea64, Lida64, K9ea64, Ijda64, Pjda64);

parameter L8ea64 = 128;
localparam R9ea64 = (L8ea64 / 32);

input [1:0] U8ea64;
input Ugda64;
input [7:0] D9ea64;
input [(L8ea64 - 1):0] Lida64;
output [(L8ea64 - 1):0] K9ea64;
output [31:0] Ijda64;
input [31:0] Pjda64;

wire [31:0] Caea64;
wire [31:0] Jaea64;
wire [7:0] Raea64;
wire [31:0] Bbea64;
wire [127:0] Lbea64;
wire [127:0] Wbea64;
wire [127:0] Hcea64;
wire [(L8ea64 - 1):0] Wcea64;
genvar  Idea64;
genvar  Ieea64;

assign Caea64 = (Ugda64 ? Wbea64[96+:32] : Lida64[31:0]);
assign Ijda64 = ((Caea64 << 8) | (Caea64 >> 24));
assign Bbea64 = Pjda64;
assign Raea64 = (D9ea64 ^ Bbea64[31:24]);
assign Jaea64 = {Raea64, Bbea64[23:0]};
assign Lbea64[0+:32] = (Wcea64[0+:32] ^ Jaea64);
assign Lbea64[32+:32] = (Lbea64[0+:32] ^ Wcea64[32+:32]);
assign Lbea64[64+:32] = (Lbea64[32+:32] ^ Wcea64[64+:32]);
assign Lbea64[96+:32] = (Lbea64[64+:32] ^ Wcea64[96+:32]);
assign Wbea64[0+:32] = (Wcea64[0+:32] ^ Jaea64);
assign Wbea64[32+:32] = (Wcea64[0+:32] ^ Wcea64[32+:32]);
assign Wbea64[64+:32] = (Wcea64[32+:32] ^ Wcea64[64+:32]);
assign Wbea64[96+:32] = (Wcea64[64+:32] ^ Wcea64[96+:32]);
assign Hcea64 = (Ugda64 ? Wbea64 : Lbea64);

for (Idea64 = 0; (Idea64 < R9ea64); Idea64 = (Idea64 + 1)) begin : Odea64 


assign Wcea64[(32 * Idea64)+:32] = Lida64[(L8ea64 - (32 * (Idea64 + 1)))+:32];
end

for (Ieea64 = 0; (Ieea64 < R9ea64); Ieea64 = (Ieea64 + 1)) begin : Oeea64 


assign K9ea64[(32 * Ieea64)+:32] = Hcea64[(L8ea64 - (32 * (Ieea64 + 1)))+:32];
end
endmodule

/*
                    instances: 0
                        nodes: 3 (0)
                  node widths: 17 (0)
                        ports: 3 (0)
*/

//`timescale 1 ns / 1 ns
module Mgea64(Ugda64, Aiea64, Giea64);

parameter Shea64 = "ALG";

input Ugda64;
input [7:0] Aiea64;
output [7:0] Giea64;

if (Shea64 == "LUT") begin : Niea64 

assign Giea64 = (Ugda64 ? Yiea64(Aiea64) : Bjea64(Aiea64));

function [7:0] Bjea64;

input reg [7:0] Djea64;
case (Djea64)
8'b0:
Bjea64 = 8'h63;
8'b1:
Bjea64 = 8'h7c;
8'h02:
Bjea64 = 8'h77;
8'h03:
Bjea64 = 8'h7b;
8'h04:
Bjea64 = 8'hf2;
8'h05:
Bjea64 = 8'h6b;
8'h06:
Bjea64 = 8'h6f;
8'h07:
Bjea64 = 8'hc5;
8'h08:
Bjea64 = 8'h30;
8'h09:
Bjea64 = 8'b1;
8'h0a:
Bjea64 = 8'h67;
8'h0b:
Bjea64 = 8'h2b;
8'h0c:
Bjea64 = 8'hfe;
8'h0d:
Bjea64 = 8'hd7;
8'h0e:
Bjea64 = 8'hab;
8'h0f:
Bjea64 = 8'h76;
8'h10:
Bjea64 = 8'hca;
8'h11:
Bjea64 = 8'h82;
8'h12:
Bjea64 = 8'hc9;
8'h13:
Bjea64 = 8'h7d;
8'h14:
Bjea64 = 8'hfa;
8'h15:
Bjea64 = 8'h59;
8'h16:
Bjea64 = 8'h47;
8'h17:
Bjea64 = 8'hf0;
8'h18:
Bjea64 = 8'had;
8'h19:
Bjea64 = 8'hd4;
8'h1a:
Bjea64 = 8'ha2;
8'h1b:
Bjea64 = 8'haf;
8'h1c:
Bjea64 = 8'h9c;
8'h1d:
Bjea64 = 8'ha4;
8'h1e:
Bjea64 = 8'h72;
8'h1f:
Bjea64 = 8'hc0;
8'h20:
Bjea64 = 8'hb7;
8'h21:
Bjea64 = 8'hfd;
8'h22:
Bjea64 = 8'h93;
8'h23:
Bjea64 = 8'h26;
8'h24:
Bjea64 = 8'h36;
8'h25:
Bjea64 = 8'h3f;
8'h26:
Bjea64 = 8'hf7;
8'h27:
Bjea64 = 8'hcc;
8'h28:
Bjea64 = 8'h34;
8'h29:
Bjea64 = 8'ha5;
8'h2a:
Bjea64 = 8'he5;
8'h2b:
Bjea64 = 8'hf1;
8'h2c:
Bjea64 = 8'h71;
8'h2d:
Bjea64 = 8'hd8;
8'h2e:
Bjea64 = 8'h31;
8'h2f:
Bjea64 = 8'h15;
8'h30:
Bjea64 = 8'h04;
8'h31:
Bjea64 = 8'hc7;
8'h32:
Bjea64 = 8'h23;
8'h33:
Bjea64 = 8'hc3;
8'h34:
Bjea64 = 8'h18;
8'h35:
Bjea64 = 8'h96;
8'h36:
Bjea64 = 8'h05;
8'h37:
Bjea64 = 8'h9a;
8'h38:
Bjea64 = 8'h07;
8'h39:
Bjea64 = 8'h12;
8'h3a:
Bjea64 = 8'h80;
8'h3b:
Bjea64 = 8'he2;
8'h3c:
Bjea64 = 8'heb;
8'h3d:
Bjea64 = 8'h27;
8'h3e:
Bjea64 = 8'hb2;
8'h3f:
Bjea64 = 8'h75;
8'h40:
Bjea64 = 8'h09;
8'h41:
Bjea64 = 8'h83;
8'h42:
Bjea64 = 8'h2c;
8'h43:
Bjea64 = 8'h1a;
8'h44:
Bjea64 = 8'h1b;
8'h45:
Bjea64 = 8'h6e;
8'h46:
Bjea64 = 8'h5a;
8'h47:
Bjea64 = 8'ha0;
8'h48:
Bjea64 = 8'h52;
8'h49:
Bjea64 = 8'h3b;
8'h4a:
Bjea64 = 8'hd6;
8'h4b:
Bjea64 = 8'hb3;
8'h4c:
Bjea64 = 8'h29;
8'h4d:
Bjea64 = 8'he3;
8'h4e:
Bjea64 = 8'h2f;
8'h4f:
Bjea64 = 8'h84;
8'h50:
Bjea64 = 8'h53;
8'h51:
Bjea64 = 8'hd1;
8'h52:
Bjea64 = 8'b0;
8'h53:
Bjea64 = 8'hed;
8'h54:
Bjea64 = 8'h20;
8'h55:
Bjea64 = 8'hfc;
8'h56:
Bjea64 = 8'hb1;
8'h57:
Bjea64 = 8'h5b;
8'h58:
Bjea64 = 8'h6a;
8'h59:
Bjea64 = 8'hcb;
8'h5a:
Bjea64 = 8'hbe;
8'h5b:
Bjea64 = 8'h39;
8'h5c:
Bjea64 = 8'h4a;
8'h5d:
Bjea64 = 8'h4c;
8'h5e:
Bjea64 = 8'h58;
8'h5f:
Bjea64 = 8'hcf;
8'h60:
Bjea64 = 8'hd0;
8'h61:
Bjea64 = 8'hef;
8'h62:
Bjea64 = 8'haa;
8'h63:
Bjea64 = 8'hfb;
8'h64:
Bjea64 = 8'h43;
8'h65:
Bjea64 = 8'h4d;
8'h66:
Bjea64 = 8'h33;
8'h67:
Bjea64 = 8'h85;
8'h68:
Bjea64 = 8'h45;
8'h69:
Bjea64 = 8'hf9;
8'h6a:
Bjea64 = 8'h02;
8'h6b:
Bjea64 = 8'h7f;
8'h6c:
Bjea64 = 8'h50;
8'h6d:
Bjea64 = 8'h3c;
8'h6e:
Bjea64 = 8'h9f;
8'h6f:
Bjea64 = 8'ha8;
8'h70:
Bjea64 = 8'h51;
8'h71:
Bjea64 = 8'ha3;
8'h72:
Bjea64 = 8'h40;
8'h73:
Bjea64 = 8'h8f;
8'h74:
Bjea64 = 8'h92;
8'h75:
Bjea64 = 8'h9d;
8'h76:
Bjea64 = 8'h38;
8'h77:
Bjea64 = 8'hf5;
8'h78:
Bjea64 = 8'hbc;
8'h79:
Bjea64 = 8'hb6;
8'h7a:
Bjea64 = 8'hda;
8'h7b:
Bjea64 = 8'h21;
8'h7c:
Bjea64 = 8'h10;
8'h7d:
Bjea64 = 8'hff;
8'h7e:
Bjea64 = 8'hf3;
8'h7f:
Bjea64 = 8'hd2;
8'h80:
Bjea64 = 8'hcd;
8'h81:
Bjea64 = 8'h0c;
8'h82:
Bjea64 = 8'h13;
8'h83:
Bjea64 = 8'hec;
8'h84:
Bjea64 = 8'h5f;
8'h85:
Bjea64 = 8'h97;
8'h86:
Bjea64 = 8'h44;
8'h87:
Bjea64 = 8'h17;
8'h88:
Bjea64 = 8'hc4;
8'h89:
Bjea64 = 8'ha7;
8'h8a:
Bjea64 = 8'h7e;
8'h8b:
Bjea64 = 8'h3d;
8'h8c:
Bjea64 = 8'h64;
8'h8d:
Bjea64 = 8'h5d;
8'h8e:
Bjea64 = 8'h19;
8'h8f:
Bjea64 = 8'h73;
8'h90:
Bjea64 = 8'h60;
8'h91:
Bjea64 = 8'h81;
8'h92:
Bjea64 = 8'h4f;
8'h93:
Bjea64 = 8'hdc;
8'h94:
Bjea64 = 8'h22;
8'h95:
Bjea64 = 8'h2a;
8'h96:
Bjea64 = 8'h90;
8'h97:
Bjea64 = 8'h88;
8'h98:
Bjea64 = 8'h46;
8'h99:
Bjea64 = 8'hee;
8'h9a:
Bjea64 = 8'hb8;
8'h9b:
Bjea64 = 8'h14;
8'h9c:
Bjea64 = 8'hde;
8'h9d:
Bjea64 = 8'h5e;
8'h9e:
Bjea64 = 8'h0b;
8'h9f:
Bjea64 = 8'hdb;
8'ha0:
Bjea64 = 8'he0;
8'ha1:
Bjea64 = 8'h32;
8'ha2:
Bjea64 = 8'h3a;
8'ha3:
Bjea64 = 8'h0a;
8'ha4:
Bjea64 = 8'h49;
8'ha5:
Bjea64 = 8'h06;
8'ha6:
Bjea64 = 8'h24;
8'ha7:
Bjea64 = 8'h5c;
8'ha8:
Bjea64 = 8'hc2;
8'ha9:
Bjea64 = 8'hd3;
8'haa:
Bjea64 = 8'hac;
8'hab:
Bjea64 = 8'h62;
8'hac:
Bjea64 = 8'h91;
8'had:
Bjea64 = 8'h95;
8'hae:
Bjea64 = 8'he4;
8'haf:
Bjea64 = 8'h79;
8'hb0:
Bjea64 = 8'he7;
8'hb1:
Bjea64 = 8'hc8;
8'hb2:
Bjea64 = 8'h37;
8'hb3:
Bjea64 = 8'h6d;
8'hb4:
Bjea64 = 8'h8d;
8'hb5:
Bjea64 = 8'hd5;
8'hb6:
Bjea64 = 8'h4e;
8'hb7:
Bjea64 = 8'ha9;
8'hb8:
Bjea64 = 8'h6c;
8'hb9:
Bjea64 = 8'h56;
8'hba:
Bjea64 = 8'hf4;
8'hbb:
Bjea64 = 8'hea;
8'hbc:
Bjea64 = 8'h65;
8'hbd:
Bjea64 = 8'h7a;
8'hbe:
Bjea64 = 8'hae;
8'hbf:
Bjea64 = 8'h08;
8'hc0:
Bjea64 = 8'hba;
8'hc1:
Bjea64 = 8'h78;
8'hc2:
Bjea64 = 8'h25;
8'hc3:
Bjea64 = 8'h2e;
8'hc4:
Bjea64 = 8'h1c;
8'hc5:
Bjea64 = 8'ha6;
8'hc6:
Bjea64 = 8'hb4;
8'hc7:
Bjea64 = 8'hc6;
8'hc8:
Bjea64 = 8'he8;
8'hc9:
Bjea64 = 8'hdd;
8'hca:
Bjea64 = 8'h74;
8'hcb:
Bjea64 = 8'h1f;
8'hcc:
Bjea64 = 8'h4b;
8'hcd:
Bjea64 = 8'hbd;
8'hce:
Bjea64 = 8'h8b;
8'hcf:
Bjea64 = 8'h8a;
8'hd0:
Bjea64 = 8'h70;
8'hd1:
Bjea64 = 8'h3e;
8'hd2:
Bjea64 = 8'hb5;
8'hd3:
Bjea64 = 8'h66;
8'hd4:
Bjea64 = 8'h48;
8'hd5:
Bjea64 = 8'h03;
8'hd6:
Bjea64 = 8'hf6;
8'hd7:
Bjea64 = 8'h0e;
8'hd8:
Bjea64 = 8'h61;
8'hd9:
Bjea64 = 8'h35;
8'hda:
Bjea64 = 8'h57;
8'hdb:
Bjea64 = 8'hb9;
8'hdc:
Bjea64 = 8'h86;
8'hdd:
Bjea64 = 8'hc1;
8'hde:
Bjea64 = 8'h1d;
8'hdf:
Bjea64 = 8'h9e;
8'he0:
Bjea64 = 8'he1;
8'he1:
Bjea64 = 8'hf8;
8'he2:
Bjea64 = 8'h98;
8'he3:
Bjea64 = 8'h11;
8'he4:
Bjea64 = 8'h69;
8'he5:
Bjea64 = 8'hd9;
8'he6:
Bjea64 = 8'h8e;
8'he7:
Bjea64 = 8'h94;
8'he8:
Bjea64 = 8'h9b;
8'he9:
Bjea64 = 8'h1e;
8'hea:
Bjea64 = 8'h87;
8'heb:
Bjea64 = 8'he9;
8'hec:
Bjea64 = 8'hce;
8'hed:
Bjea64 = 8'h55;
8'hee:
Bjea64 = 8'h28;
8'hef:
Bjea64 = 8'hdf;
8'hf0:
Bjea64 = 8'h8c;
8'hf1:
Bjea64 = 8'ha1;
8'hf2:
Bjea64 = 8'h89;
8'hf3:
Bjea64 = 8'h0d;
8'hf4:
Bjea64 = 8'hbf;
8'hf5:
Bjea64 = 8'he6;
8'hf6:
Bjea64 = 8'h42;
8'hf7:
Bjea64 = 8'h68;
8'hf8:
Bjea64 = 8'h41;
8'hf9:
Bjea64 = 8'h99;
8'hfa:
Bjea64 = 8'h2d;
8'hfb:
Bjea64 = 8'h0f;
8'hfc:
Bjea64 = 8'hb0;
8'hfd:
Bjea64 = 8'h54;
8'hfe:
Bjea64 = 8'hbb;
8'hff:
Bjea64 = 8'h16;
endcase
endfunction

function [7:0] Yiea64;

input reg [7:0] Djea64;
case (Djea64)
8'b0:
Yiea64 = 8'h52;
8'b1:
Yiea64 = 8'h09;
8'h02:
Yiea64 = 8'h6a;
8'h03:
Yiea64 = 8'hd5;
8'h04:
Yiea64 = 8'h30;
8'h05:
Yiea64 = 8'h36;
8'h06:
Yiea64 = 8'ha5;
8'h07:
Yiea64 = 8'h38;
8'h08:
Yiea64 = 8'hbf;
8'h09:
Yiea64 = 8'h40;
8'h0a:
Yiea64 = 8'ha3;
8'h0b:
Yiea64 = 8'h9e;
8'h0c:
Yiea64 = 8'h81;
8'h0d:
Yiea64 = 8'hf3;
8'h0e:
Yiea64 = 8'hd7;
8'h0f:
Yiea64 = 8'hfb;
8'h10:
Yiea64 = 8'h7c;
8'h11:
Yiea64 = 8'he3;
8'h12:
Yiea64 = 8'h39;
8'h13:
Yiea64 = 8'h82;
8'h14:
Yiea64 = 8'h9b;
8'h15:
Yiea64 = 8'h2f;
8'h16:
Yiea64 = 8'hff;
8'h17:
Yiea64 = 8'h87;
8'h18:
Yiea64 = 8'h34;
8'h19:
Yiea64 = 8'h8e;
8'h1a:
Yiea64 = 8'h43;
8'h1b:
Yiea64 = 8'h44;
8'h1c:
Yiea64 = 8'hc4;
8'h1d:
Yiea64 = 8'hde;
8'h1e:
Yiea64 = 8'he9;
8'h1f:
Yiea64 = 8'hcb;
8'h20:
Yiea64 = 8'h54;
8'h21:
Yiea64 = 8'h7b;
8'h22:
Yiea64 = 8'h94;
8'h23:
Yiea64 = 8'h32;
8'h24:
Yiea64 = 8'ha6;
8'h25:
Yiea64 = 8'hc2;
8'h26:
Yiea64 = 8'h23;
8'h27:
Yiea64 = 8'h3d;
8'h28:
Yiea64 = 8'hee;
8'h29:
Yiea64 = 8'h4c;
8'h2a:
Yiea64 = 8'h95;
8'h2b:
Yiea64 = 8'h0b;
8'h2c:
Yiea64 = 8'h42;
8'h2d:
Yiea64 = 8'hfa;
8'h2e:
Yiea64 = 8'hc3;
8'h2f:
Yiea64 = 8'h4e;
8'h30:
Yiea64 = 8'h08;
8'h31:
Yiea64 = 8'h2e;
8'h32:
Yiea64 = 8'ha1;
8'h33:
Yiea64 = 8'h66;
8'h34:
Yiea64 = 8'h28;
8'h35:
Yiea64 = 8'hd9;
8'h36:
Yiea64 = 8'h24;
8'h37:
Yiea64 = 8'hb2;
8'h38:
Yiea64 = 8'h76;
8'h39:
Yiea64 = 8'h5b;
8'h3a:
Yiea64 = 8'ha2;
8'h3b:
Yiea64 = 8'h49;
8'h3c:
Yiea64 = 8'h6d;
8'h3d:
Yiea64 = 8'h8b;
8'h3e:
Yiea64 = 8'hd1;
8'h3f:
Yiea64 = 8'h25;
8'h40:
Yiea64 = 8'h72;
8'h41:
Yiea64 = 8'hf8;
8'h42:
Yiea64 = 8'hf6;
8'h43:
Yiea64 = 8'h64;
8'h44:
Yiea64 = 8'h86;
8'h45:
Yiea64 = 8'h68;
8'h46:
Yiea64 = 8'h98;
8'h47:
Yiea64 = 8'h16;
8'h48:
Yiea64 = 8'hd4;
8'h49:
Yiea64 = 8'ha4;
8'h4a:
Yiea64 = 8'h5c;
8'h4b:
Yiea64 = 8'hcc;
8'h4c:
Yiea64 = 8'h5d;
8'h4d:
Yiea64 = 8'h65;
8'h4e:
Yiea64 = 8'hb6;
8'h4f:
Yiea64 = 8'h92;
8'h50:
Yiea64 = 8'h6c;
8'h51:
Yiea64 = 8'h70;
8'h52:
Yiea64 = 8'h48;
8'h53:
Yiea64 = 8'h50;
8'h54:
Yiea64 = 8'hfd;
8'h55:
Yiea64 = 8'hed;
8'h56:
Yiea64 = 8'hb9;
8'h57:
Yiea64 = 8'hda;
8'h58:
Yiea64 = 8'h5e;
8'h59:
Yiea64 = 8'h15;
8'h5a:
Yiea64 = 8'h46;
8'h5b:
Yiea64 = 8'h57;
8'h5c:
Yiea64 = 8'ha7;
8'h5d:
Yiea64 = 8'h8d;
8'h5e:
Yiea64 = 8'h9d;
8'h5f:
Yiea64 = 8'h84;
8'h60:
Yiea64 = 8'h90;
8'h61:
Yiea64 = 8'hd8;
8'h62:
Yiea64 = 8'hab;
8'h63:
Yiea64 = 8'b0;
8'h64:
Yiea64 = 8'h8c;
8'h65:
Yiea64 = 8'hbc;
8'h66:
Yiea64 = 8'hd3;
8'h67:
Yiea64 = 8'h0a;
8'h68:
Yiea64 = 8'hf7;
8'h69:
Yiea64 = 8'he4;
8'h6a:
Yiea64 = 8'h58;
8'h6b:
Yiea64 = 8'h05;
8'h6c:
Yiea64 = 8'hb8;
8'h6d:
Yiea64 = 8'hb3;
8'h6e:
Yiea64 = 8'h45;
8'h6f:
Yiea64 = 8'h06;
8'h70:
Yiea64 = 8'hd0;
8'h71:
Yiea64 = 8'h2c;
8'h72:
Yiea64 = 8'h1e;
8'h73:
Yiea64 = 8'h8f;
8'h74:
Yiea64 = 8'hca;
8'h75:
Yiea64 = 8'h3f;
8'h76:
Yiea64 = 8'h0f;
8'h77:
Yiea64 = 8'h02;
8'h78:
Yiea64 = 8'hc1;
8'h79:
Yiea64 = 8'haf;
8'h7a:
Yiea64 = 8'hbd;
8'h7b:
Yiea64 = 8'h03;
8'h7c:
Yiea64 = 8'b1;
8'h7d:
Yiea64 = 8'h13;
8'h7e:
Yiea64 = 8'h8a;
8'h7f:
Yiea64 = 8'h6b;
8'h80:
Yiea64 = 8'h3a;
8'h81:
Yiea64 = 8'h91;
8'h82:
Yiea64 = 8'h11;
8'h83:
Yiea64 = 8'h41;
8'h84:
Yiea64 = 8'h4f;
8'h85:
Yiea64 = 8'h67;
8'h86:
Yiea64 = 8'hdc;
8'h87:
Yiea64 = 8'hea;
8'h88:
Yiea64 = 8'h97;
8'h89:
Yiea64 = 8'hf2;
8'h8a:
Yiea64 = 8'hcf;
8'h8b:
Yiea64 = 8'hce;
8'h8c:
Yiea64 = 8'hf0;
8'h8d:
Yiea64 = 8'hb4;
8'h8e:
Yiea64 = 8'he6;
8'h8f:
Yiea64 = 8'h73;
8'h90:
Yiea64 = 8'h96;
8'h91:
Yiea64 = 8'hac;
8'h92:
Yiea64 = 8'h74;
8'h93:
Yiea64 = 8'h22;
8'h94:
Yiea64 = 8'he7;
8'h95:
Yiea64 = 8'had;
8'h96:
Yiea64 = 8'h35;
8'h97:
Yiea64 = 8'h85;
8'h98:
Yiea64 = 8'he2;
8'h99:
Yiea64 = 8'hf9;
8'h9a:
Yiea64 = 8'h37;
8'h9b:
Yiea64 = 8'he8;
8'h9c:
Yiea64 = 8'h1c;
8'h9d:
Yiea64 = 8'h75;
8'h9e:
Yiea64 = 8'hdf;
8'h9f:
Yiea64 = 8'h6e;
8'ha0:
Yiea64 = 8'h47;
8'ha1:
Yiea64 = 8'hf1;
8'ha2:
Yiea64 = 8'h1a;
8'ha3:
Yiea64 = 8'h71;
8'ha4:
Yiea64 = 8'h1d;
8'ha5:
Yiea64 = 8'h29;
8'ha6:
Yiea64 = 8'hc5;
8'ha7:
Yiea64 = 8'h89;
8'ha8:
Yiea64 = 8'h6f;
8'ha9:
Yiea64 = 8'hb7;
8'haa:
Yiea64 = 8'h62;
8'hab:
Yiea64 = 8'h0e;
8'hac:
Yiea64 = 8'haa;
8'had:
Yiea64 = 8'h18;
8'hae:
Yiea64 = 8'hbe;
8'haf:
Yiea64 = 8'h1b;
8'hb0:
Yiea64 = 8'hfc;
8'hb1:
Yiea64 = 8'h56;
8'hb2:
Yiea64 = 8'h3e;
8'hb3:
Yiea64 = 8'h4b;
8'hb4:
Yiea64 = 8'hc6;
8'hb5:
Yiea64 = 8'hd2;
8'hb6:
Yiea64 = 8'h79;
8'hb7:
Yiea64 = 8'h20;
8'hb8:
Yiea64 = 8'h9a;
8'hb9:
Yiea64 = 8'hdb;
8'hba:
Yiea64 = 8'hc0;
8'hbb:
Yiea64 = 8'hfe;
8'hbc:
Yiea64 = 8'h78;
8'hbd:
Yiea64 = 8'hcd;
8'hbe:
Yiea64 = 8'h5a;
8'hbf:
Yiea64 = 8'hf4;
8'hc0:
Yiea64 = 8'h1f;
8'hc1:
Yiea64 = 8'hdd;
8'hc2:
Yiea64 = 8'ha8;
8'hc3:
Yiea64 = 8'h33;
8'hc4:
Yiea64 = 8'h88;
8'hc5:
Yiea64 = 8'h07;
8'hc6:
Yiea64 = 8'hc7;
8'hc7:
Yiea64 = 8'h31;
8'hc8:
Yiea64 = 8'hb1;
8'hc9:
Yiea64 = 8'h12;
8'hca:
Yiea64 = 8'h10;
8'hcb:
Yiea64 = 8'h59;
8'hcc:
Yiea64 = 8'h27;
8'hcd:
Yiea64 = 8'h80;
8'hce:
Yiea64 = 8'hec;
8'hcf:
Yiea64 = 8'h5f;
8'hd0:
Yiea64 = 8'h60;
8'hd1:
Yiea64 = 8'h51;
8'hd2:
Yiea64 = 8'h7f;
8'hd3:
Yiea64 = 8'ha9;
8'hd4:
Yiea64 = 8'h19;
8'hd5:
Yiea64 = 8'hb5;
8'hd6:
Yiea64 = 8'h4a;
8'hd7:
Yiea64 = 8'h0d;
8'hd8:
Yiea64 = 8'h2d;
8'hd9:
Yiea64 = 8'he5;
8'hda:
Yiea64 = 8'h7a;
8'hdb:
Yiea64 = 8'h9f;
8'hdc:
Yiea64 = 8'h93;
8'hdd:
Yiea64 = 8'hc9;
8'hde:
Yiea64 = 8'h9c;
8'hdf:
Yiea64 = 8'hef;
8'he0:
Yiea64 = 8'ha0;
8'he1:
Yiea64 = 8'he0;
8'he2:
Yiea64 = 8'h3b;
8'he3:
Yiea64 = 8'h4d;
8'he4:
Yiea64 = 8'hae;
8'he5:
Yiea64 = 8'h2a;
8'he6:
Yiea64 = 8'hf5;
8'he7:
Yiea64 = 8'hb0;
8'he8:
Yiea64 = 8'hc8;
8'he9:
Yiea64 = 8'heb;
8'hea:
Yiea64 = 8'hbb;
8'heb:
Yiea64 = 8'h3c;
8'hec:
Yiea64 = 8'h83;
8'hed:
Yiea64 = 8'h53;
8'hee:
Yiea64 = 8'h99;
8'hef:
Yiea64 = 8'h61;
8'hf0:
Yiea64 = 8'h17;
8'hf1:
Yiea64 = 8'h2b;
8'hf2:
Yiea64 = 8'h04;
8'hf3:
Yiea64 = 8'h7e;
8'hf4:
Yiea64 = 8'hba;
8'hf5:
Yiea64 = 8'h77;
8'hf6:
Yiea64 = 8'hd6;
8'hf7:
Yiea64 = 8'h26;
8'hf8:
Yiea64 = 8'he1;
8'hf9:
Yiea64 = 8'h69;
8'hfa:
Yiea64 = 8'h14;
8'hfb:
Yiea64 = 8'h63;
8'hfc:
Yiea64 = 8'h55;
8'hfd:
Yiea64 = 8'h21;
8'hfe:
Yiea64 = 8'h0c;
8'hff:
Yiea64 = 8'h7d;
endcase
endfunction
end
else 
if (Shea64 == "ALG") begin : Fjea64 

wire [7:0] Qjea64;
wire [7:0] Ujea64;
wire [7:0] Yjea64;
wire [7:0] Fkea64;
wire [3:0] Jkea64;
wire [3:0] Okea64;
wire [3:0] Tkea64;
wire [3:0] Zkea64;
wire [3:0] Elea64;
wire [1:0] Klea64;
wire [1:0] Plea64;
wire [1:0] Ulea64;
wire [1:0] Bmea64;
wire [1:0] Imea64;

assign Qjea64[0] = (Ugda64 ? ((Aiea64[7] ~^ Aiea64[5]) ^ Aiea64[2]) : Aiea64[0])
;
assign Qjea64[1] = (Ugda64 ? ((Aiea64[0] ^ Aiea64[6]) ^ Aiea64[3]) : Aiea64[1]);
assign Qjea64[2] = (Ugda64 ? ((Aiea64[1] ^ Aiea64[7]) ~^ Aiea64[4]) : Aiea64[2])
;
assign Qjea64[3] = (Ugda64 ? ((Aiea64[2] ^ Aiea64[0]) ^ Aiea64[5]) : Aiea64[3]);
assign Qjea64[4] = (Ugda64 ? ((Aiea64[3] ^ Aiea64[1]) ^ Aiea64[6]) : Aiea64[4]);
assign Qjea64[5] = (Ugda64 ? ((Aiea64[4] ^ Aiea64[2]) ^ Aiea64[7]) : Aiea64[5]);
assign Qjea64[6] = (Ugda64 ? ((Aiea64[5] ^ Aiea64[3]) ^ Aiea64[0]) : Aiea64[6]);
assign Qjea64[7] = (Ugda64 ? ((Aiea64[6] ^ Aiea64[4]) ^ Aiea64[1]) : Aiea64[7]);
assign Ujea64[0] = ((((Qjea64[6] ^ Qjea64[3]) ^ Qjea64[2]) ^ Qjea64[1]) ^ 
Qjea64[0]);
assign Ujea64[1] = ((Qjea64[6] ^ Qjea64[5]) ^ Qjea64[0]);
assign Ujea64[2] = Qjea64[0];
assign Ujea64[3] = ((((Qjea64[7] ^ Qjea64[4]) ^ Qjea64[3]) ^ Qjea64[1]) ^ 
Qjea64[0]);
assign Ujea64[4] = (((Qjea64[7] ^ Qjea64[6]) ^ Qjea64[5]) ^ Qjea64[0]);
assign Ujea64[5] = (((Qjea64[6] ^ Qjea64[5]) ^ Qjea64[1]) ^ Qjea64[0]);
assign Ujea64[6] = (((Qjea64[6] ^ Qjea64[5]) ^ Qjea64[4]) ^ Qjea64[0]);
assign Ujea64[7] = (((((Qjea64[7] ^ Qjea64[6]) ^ Qjea64[5]) ^ Qjea64[2]) ^ 
Qjea64[1]) ^ Qjea64[0]);
assign Jkea64 = Ujea64[7:4];
assign Okea64 = Ujea64[3:0];
assign Tkea64[3] = ((((((Jkea64[1] ^ Jkea64[3]) & (Okea64[1] ^ Okea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Okea64[0] ^ Okea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Okea64[1] ^ Okea64[2]))) ^ ((Jkea64[0] ^ Jkea64[1]) & (Okea64[0] ^ Okea64[1])))
^ (Jkea64[3] & Okea64[3]));
assign Tkea64[2] = (((((Jkea64[2] ^ Jkea64[3]) & (Okea64[2] ^ Okea64[3])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Okea64[0] ^ Okea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Okea64[1] ^ Okea64[3]))) ^ (Jkea64[2] & Okea64[2]));
assign Tkea64[1] = ((((((Jkea64[1] ^ Jkea64[3]) & (Okea64[1] ^ Okea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Okea64[0] ^ Okea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Okea64[1] ^ Okea64[2]))) ^ ((Jkea64[2] ^ Jkea64[3]) & (Okea64[2] ^ Okea64[3])))
^ (Jkea64[1] & Okea64[1]));
assign Tkea64[0] = (((((Jkea64[0] ^ Jkea64[1]) & (Okea64[0] ^ Okea64[1])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Okea64[0] ^ Okea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Okea64[1] ^ Okea64[3]))) ^ (Jkea64[0] & Okea64[0]));
assign Zkea64[3] = (((Jkea64[0] ^ Okea64[0]) ^ Jkea64[2]) ^ Okea64[2]);
assign Zkea64[2] = (((Jkea64[1] ^ Okea64[1]) ^ Jkea64[3]) ^ Okea64[3]);
assign Zkea64[1] = (((Jkea64[0] ^ Okea64[0]) ^ Jkea64[1]) ^ Okea64[1]);
assign Zkea64[0] = (Jkea64[0] ^ Okea64[0]);
assign {Klea64, Plea64} = (Tkea64 ^ Zkea64);
assign Bmea64[0] = (((Klea64[1] ^ Klea64[0]) & (Plea64[0] ^ Plea64[1])) ^ (
Klea64[0] & Plea64[0]));
assign Bmea64[1] = (((Klea64[1] ^ Klea64[0]) & (Plea64[0] ^ Plea64[1])) ^ (
Klea64[1] & Plea64[1]));
assign Imea64[0] = (((Klea64[1] ^ Plea64[1]) ^ Klea64[0]) ^ Plea64[0]);
assign Imea64[1] = (Klea64[1] ^ Plea64[1]);
assign {Ulea64[0], Ulea64[1]} = (Bmea64 ^ Imea64);
assign Elea64[0] = (((Klea64[0] ^ Klea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Klea64[0] & Ulea64[0]));
assign Elea64[1] = (((Klea64[0] ^ Klea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Klea64[1] & Ulea64[1]));
assign Elea64[2] = (((Plea64[0] ^ Plea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Plea64[0] & Ulea64[0]));
assign Elea64[3] = (((Plea64[0] ^ Plea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Plea64[1] & Ulea64[1]));
assign Yjea64[7] = ((((((Okea64[1] ^ Okea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Okea64[0] ^ Okea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Okea64[1] ^ Okea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Okea64[3] & Elea64[3])) ^ ((Okea64[0] ^ Okea64[1]) 
& (Elea64[0] ^ Elea64[1])));
assign Yjea64[5] = ((((((Okea64[1] ^ Okea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Okea64[0] ^ Okea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Okea64[1] ^ Okea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Okea64[1] & Elea64[1])) ^ ((Okea64[2] ^ Okea64[3]) 
& (Elea64[2] ^ Elea64[3])));
assign Yjea64[3] = ((((((Jkea64[1] ^ Jkea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Jkea64[3] & Elea64[3])) ^ ((Jkea64[0] ^ Jkea64[1]) 
& (Elea64[0] ^ Elea64[1])));
assign Yjea64[1] = ((((((Jkea64[1] ^ Jkea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Jkea64[1] & Elea64[1])) ^ ((Jkea64[2] ^ Jkea64[3]) 
& (Elea64[2] ^ Elea64[3])));
assign Yjea64[6] = (((((Okea64[2] ^ Okea64[3]) & (Elea64[2] ^ Elea64[3])) ^ ((
Okea64[0] ^ Okea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Okea64[1] ^ Okea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Okea64[2] & Elea64[2]));
assign Yjea64[4] = (((((Okea64[0] ^ Okea64[1]) & (Elea64[0] ^ Elea64[1])) ^ ((
Okea64[0] ^ Okea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Okea64[1] ^ Okea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Okea64[0] & Elea64[0]));
assign Yjea64[2] = (((((Jkea64[2] ^ Jkea64[3]) & (Elea64[2] ^ Elea64[3])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Jkea64[2] & Elea64[2]));
assign Yjea64[0] = (((((Jkea64[0] ^ Jkea64[1]) & (Elea64[0] ^ Elea64[1])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Jkea64[0] & Elea64[0]));
assign Fkea64[0] = Yjea64[2];
assign Fkea64[1] = (Yjea64[5] ^ Yjea64[1]);
assign Fkea64[2] = (((Yjea64[7] ^ Yjea64[5]) ^ Yjea64[4]) ^ Yjea64[1]);
assign Fkea64[3] = (((((Yjea64[6] ^ Yjea64[5]) ^ Yjea64[4]) ^ Yjea64[3]) ^ 
Yjea64[2]) ^ Yjea64[1]);
assign Fkea64[4] = (Yjea64[6] ^ Yjea64[1]);
assign Fkea64[5] = (((((Yjea64[7] ^ Yjea64[6]) ^ Yjea64[5]) ^ Yjea64[3]) ^ 
Yjea64[2]) ^ Yjea64[0]);
assign Fkea64[6] = (((((Yjea64[7] ^ Yjea64[6]) ^ Yjea64[5]) ^ Yjea64[3]) ^ 
Yjea64[1]) ^ Yjea64[0]);
assign Fkea64[7] = (Yjea64[4] ^ Yjea64[1]);
assign Giea64[0] = (Ugda64 ? Fkea64[0] : ((((Fkea64[0] ~^ Fkea64[7]) ^ Fkea64[6]
) ^ Fkea64[5]) ^ Fkea64[4]));
assign Giea64[1] = (Ugda64 ? Fkea64[1] : ((((Fkea64[1] ~^ Fkea64[0]) ^ Fkea64[7]
) ^ Fkea64[6]) ^ Fkea64[5]));
assign Giea64[2] = (Ugda64 ? Fkea64[2] : ((((Fkea64[2] ^ Fkea64[1]) ^ Fkea64[0])
^ Fkea64[7]) ^ Fkea64[6]));
assign Giea64[3] = (Ugda64 ? Fkea64[3] : ((((Fkea64[3] ^ Fkea64[2]) ^ Fkea64[1])
^ Fkea64[0]) ^ Fkea64[7]));
assign Giea64[4] = (Ugda64 ? Fkea64[4] : ((((Fkea64[4] ^ Fkea64[3]) ^ Fkea64[2])
^ Fkea64[1]) ^ Fkea64[0]));
assign Giea64[5] = (Ugda64 ? Fkea64[5] : ((((Fkea64[5] ~^ Fkea64[4]) ^ Fkea64[3]
) ^ Fkea64[2]) ^ Fkea64[1]));
assign Giea64[6] = (Ugda64 ? Fkea64[6] : ((((Fkea64[6] ~^ Fkea64[5]) ^ Fkea64[4]
) ^ Fkea64[3]) ^ Fkea64[2]));
assign Giea64[7] = (Ugda64 ? Fkea64[7] : ((((Fkea64[7] ^ Fkea64[6]) ^ Fkea64[5])
^ Fkea64[4]) ^ Fkea64[3]));
end
else  begin : Omea64 

end
endmodule

/*
                    instances: 0
                        nodes: 3 (0)
                  node widths: 17 (0)
                        ports: 3 (0)
*/

//`timescale 1 ns / 1 ns
module Hoea64(Ugda64, Aiea64, Giea64);

parameter Shea64 = "ALG";

input Ugda64;
input [7:0] Aiea64;
output [7:0] Giea64;

if (Shea64 == "LUT") begin : Niea64 

assign Giea64 = (Ugda64 ? Yiea64(Aiea64) : Bjea64(Aiea64));

function [7:0] Bjea64;

input reg [7:0] Djea64;
case (Djea64)
8'b0:
Bjea64 = 8'h63;
8'b1:
Bjea64 = 8'h7c;
8'h02:
Bjea64 = 8'h77;
8'h03:
Bjea64 = 8'h7b;
8'h04:
Bjea64 = 8'hf2;
8'h05:
Bjea64 = 8'h6b;
8'h06:
Bjea64 = 8'h6f;
8'h07:
Bjea64 = 8'hc5;
8'h08:
Bjea64 = 8'h30;
8'h09:
Bjea64 = 8'b1;
8'h0a:
Bjea64 = 8'h67;
8'h0b:
Bjea64 = 8'h2b;
8'h0c:
Bjea64 = 8'hfe;
8'h0d:
Bjea64 = 8'hd7;
8'h0e:
Bjea64 = 8'hab;
8'h0f:
Bjea64 = 8'h76;
8'h10:
Bjea64 = 8'hca;
8'h11:
Bjea64 = 8'h82;
8'h12:
Bjea64 = 8'hc9;
8'h13:
Bjea64 = 8'h7d;
8'h14:
Bjea64 = 8'hfa;
8'h15:
Bjea64 = 8'h59;
8'h16:
Bjea64 = 8'h47;
8'h17:
Bjea64 = 8'hf0;
8'h18:
Bjea64 = 8'had;
8'h19:
Bjea64 = 8'hd4;
8'h1a:
Bjea64 = 8'ha2;
8'h1b:
Bjea64 = 8'haf;
8'h1c:
Bjea64 = 8'h9c;
8'h1d:
Bjea64 = 8'ha4;
8'h1e:
Bjea64 = 8'h72;
8'h1f:
Bjea64 = 8'hc0;
8'h20:
Bjea64 = 8'hb7;
8'h21:
Bjea64 = 8'hfd;
8'h22:
Bjea64 = 8'h93;
8'h23:
Bjea64 = 8'h26;
8'h24:
Bjea64 = 8'h36;
8'h25:
Bjea64 = 8'h3f;
8'h26:
Bjea64 = 8'hf7;
8'h27:
Bjea64 = 8'hcc;
8'h28:
Bjea64 = 8'h34;
8'h29:
Bjea64 = 8'ha5;
8'h2a:
Bjea64 = 8'he5;
8'h2b:
Bjea64 = 8'hf1;
8'h2c:
Bjea64 = 8'h71;
8'h2d:
Bjea64 = 8'hd8;
8'h2e:
Bjea64 = 8'h31;
8'h2f:
Bjea64 = 8'h15;
8'h30:
Bjea64 = 8'h04;
8'h31:
Bjea64 = 8'hc7;
8'h32:
Bjea64 = 8'h23;
8'h33:
Bjea64 = 8'hc3;
8'h34:
Bjea64 = 8'h18;
8'h35:
Bjea64 = 8'h96;
8'h36:
Bjea64 = 8'h05;
8'h37:
Bjea64 = 8'h9a;
8'h38:
Bjea64 = 8'h07;
8'h39:
Bjea64 = 8'h12;
8'h3a:
Bjea64 = 8'h80;
8'h3b:
Bjea64 = 8'he2;
8'h3c:
Bjea64 = 8'heb;
8'h3d:
Bjea64 = 8'h27;
8'h3e:
Bjea64 = 8'hb2;
8'h3f:
Bjea64 = 8'h75;
8'h40:
Bjea64 = 8'h09;
8'h41:
Bjea64 = 8'h83;
8'h42:
Bjea64 = 8'h2c;
8'h43:
Bjea64 = 8'h1a;
8'h44:
Bjea64 = 8'h1b;
8'h45:
Bjea64 = 8'h6e;
8'h46:
Bjea64 = 8'h5a;
8'h47:
Bjea64 = 8'ha0;
8'h48:
Bjea64 = 8'h52;
8'h49:
Bjea64 = 8'h3b;
8'h4a:
Bjea64 = 8'hd6;
8'h4b:
Bjea64 = 8'hb3;
8'h4c:
Bjea64 = 8'h29;
8'h4d:
Bjea64 = 8'he3;
8'h4e:
Bjea64 = 8'h2f;
8'h4f:
Bjea64 = 8'h84;
8'h50:
Bjea64 = 8'h53;
8'h51:
Bjea64 = 8'hd1;
8'h52:
Bjea64 = 8'b0;
8'h53:
Bjea64 = 8'hed;
8'h54:
Bjea64 = 8'h20;
8'h55:
Bjea64 = 8'hfc;
8'h56:
Bjea64 = 8'hb1;
8'h57:
Bjea64 = 8'h5b;
8'h58:
Bjea64 = 8'h6a;
8'h59:
Bjea64 = 8'hcb;
8'h5a:
Bjea64 = 8'hbe;
8'h5b:
Bjea64 = 8'h39;
8'h5c:
Bjea64 = 8'h4a;
8'h5d:
Bjea64 = 8'h4c;
8'h5e:
Bjea64 = 8'h58;
8'h5f:
Bjea64 = 8'hcf;
8'h60:
Bjea64 = 8'hd0;
8'h61:
Bjea64 = 8'hef;
8'h62:
Bjea64 = 8'haa;
8'h63:
Bjea64 = 8'hfb;
8'h64:
Bjea64 = 8'h43;
8'h65:
Bjea64 = 8'h4d;
8'h66:
Bjea64 = 8'h33;
8'h67:
Bjea64 = 8'h85;
8'h68:
Bjea64 = 8'h45;
8'h69:
Bjea64 = 8'hf9;
8'h6a:
Bjea64 = 8'h02;
8'h6b:
Bjea64 = 8'h7f;
8'h6c:
Bjea64 = 8'h50;
8'h6d:
Bjea64 = 8'h3c;
8'h6e:
Bjea64 = 8'h9f;
8'h6f:
Bjea64 = 8'ha8;
8'h70:
Bjea64 = 8'h51;
8'h71:
Bjea64 = 8'ha3;
8'h72:
Bjea64 = 8'h40;
8'h73:
Bjea64 = 8'h8f;
8'h74:
Bjea64 = 8'h92;
8'h75:
Bjea64 = 8'h9d;
8'h76:
Bjea64 = 8'h38;
8'h77:
Bjea64 = 8'hf5;
8'h78:
Bjea64 = 8'hbc;
8'h79:
Bjea64 = 8'hb6;
8'h7a:
Bjea64 = 8'hda;
8'h7b:
Bjea64 = 8'h21;
8'h7c:
Bjea64 = 8'h10;
8'h7d:
Bjea64 = 8'hff;
8'h7e:
Bjea64 = 8'hf3;
8'h7f:
Bjea64 = 8'hd2;
8'h80:
Bjea64 = 8'hcd;
8'h81:
Bjea64 = 8'h0c;
8'h82:
Bjea64 = 8'h13;
8'h83:
Bjea64 = 8'hec;
8'h84:
Bjea64 = 8'h5f;
8'h85:
Bjea64 = 8'h97;
8'h86:
Bjea64 = 8'h44;
8'h87:
Bjea64 = 8'h17;
8'h88:
Bjea64 = 8'hc4;
8'h89:
Bjea64 = 8'ha7;
8'h8a:
Bjea64 = 8'h7e;
8'h8b:
Bjea64 = 8'h3d;
8'h8c:
Bjea64 = 8'h64;
8'h8d:
Bjea64 = 8'h5d;
8'h8e:
Bjea64 = 8'h19;
8'h8f:
Bjea64 = 8'h73;
8'h90:
Bjea64 = 8'h60;
8'h91:
Bjea64 = 8'h81;
8'h92:
Bjea64 = 8'h4f;
8'h93:
Bjea64 = 8'hdc;
8'h94:
Bjea64 = 8'h22;
8'h95:
Bjea64 = 8'h2a;
8'h96:
Bjea64 = 8'h90;
8'h97:
Bjea64 = 8'h88;
8'h98:
Bjea64 = 8'h46;
8'h99:
Bjea64 = 8'hee;
8'h9a:
Bjea64 = 8'hb8;
8'h9b:
Bjea64 = 8'h14;
8'h9c:
Bjea64 = 8'hde;
8'h9d:
Bjea64 = 8'h5e;
8'h9e:
Bjea64 = 8'h0b;
8'h9f:
Bjea64 = 8'hdb;
8'ha0:
Bjea64 = 8'he0;
8'ha1:
Bjea64 = 8'h32;
8'ha2:
Bjea64 = 8'h3a;
8'ha3:
Bjea64 = 8'h0a;
8'ha4:
Bjea64 = 8'h49;
8'ha5:
Bjea64 = 8'h06;
8'ha6:
Bjea64 = 8'h24;
8'ha7:
Bjea64 = 8'h5c;
8'ha8:
Bjea64 = 8'hc2;
8'ha9:
Bjea64 = 8'hd3;
8'haa:
Bjea64 = 8'hac;
8'hab:
Bjea64 = 8'h62;
8'hac:
Bjea64 = 8'h91;
8'had:
Bjea64 = 8'h95;
8'hae:
Bjea64 = 8'he4;
8'haf:
Bjea64 = 8'h79;
8'hb0:
Bjea64 = 8'he7;
8'hb1:
Bjea64 = 8'hc8;
8'hb2:
Bjea64 = 8'h37;
8'hb3:
Bjea64 = 8'h6d;
8'hb4:
Bjea64 = 8'h8d;
8'hb5:
Bjea64 = 8'hd5;
8'hb6:
Bjea64 = 8'h4e;
8'hb7:
Bjea64 = 8'ha9;
8'hb8:
Bjea64 = 8'h6c;
8'hb9:
Bjea64 = 8'h56;
8'hba:
Bjea64 = 8'hf4;
8'hbb:
Bjea64 = 8'hea;
8'hbc:
Bjea64 = 8'h65;
8'hbd:
Bjea64 = 8'h7a;
8'hbe:
Bjea64 = 8'hae;
8'hbf:
Bjea64 = 8'h08;
8'hc0:
Bjea64 = 8'hba;
8'hc1:
Bjea64 = 8'h78;
8'hc2:
Bjea64 = 8'h25;
8'hc3:
Bjea64 = 8'h2e;
8'hc4:
Bjea64 = 8'h1c;
8'hc5:
Bjea64 = 8'ha6;
8'hc6:
Bjea64 = 8'hb4;
8'hc7:
Bjea64 = 8'hc6;
8'hc8:
Bjea64 = 8'he8;
8'hc9:
Bjea64 = 8'hdd;
8'hca:
Bjea64 = 8'h74;
8'hcb:
Bjea64 = 8'h1f;
8'hcc:
Bjea64 = 8'h4b;
8'hcd:
Bjea64 = 8'hbd;
8'hce:
Bjea64 = 8'h8b;
8'hcf:
Bjea64 = 8'h8a;
8'hd0:
Bjea64 = 8'h70;
8'hd1:
Bjea64 = 8'h3e;
8'hd2:
Bjea64 = 8'hb5;
8'hd3:
Bjea64 = 8'h66;
8'hd4:
Bjea64 = 8'h48;
8'hd5:
Bjea64 = 8'h03;
8'hd6:
Bjea64 = 8'hf6;
8'hd7:
Bjea64 = 8'h0e;
8'hd8:
Bjea64 = 8'h61;
8'hd9:
Bjea64 = 8'h35;
8'hda:
Bjea64 = 8'h57;
8'hdb:
Bjea64 = 8'hb9;
8'hdc:
Bjea64 = 8'h86;
8'hdd:
Bjea64 = 8'hc1;
8'hde:
Bjea64 = 8'h1d;
8'hdf:
Bjea64 = 8'h9e;
8'he0:
Bjea64 = 8'he1;
8'he1:
Bjea64 = 8'hf8;
8'he2:
Bjea64 = 8'h98;
8'he3:
Bjea64 = 8'h11;
8'he4:
Bjea64 = 8'h69;
8'he5:
Bjea64 = 8'hd9;
8'he6:
Bjea64 = 8'h8e;
8'he7:
Bjea64 = 8'h94;
8'he8:
Bjea64 = 8'h9b;
8'he9:
Bjea64 = 8'h1e;
8'hea:
Bjea64 = 8'h87;
8'heb:
Bjea64 = 8'he9;
8'hec:
Bjea64 = 8'hce;
8'hed:
Bjea64 = 8'h55;
8'hee:
Bjea64 = 8'h28;
8'hef:
Bjea64 = 8'hdf;
8'hf0:
Bjea64 = 8'h8c;
8'hf1:
Bjea64 = 8'ha1;
8'hf2:
Bjea64 = 8'h89;
8'hf3:
Bjea64 = 8'h0d;
8'hf4:
Bjea64 = 8'hbf;
8'hf5:
Bjea64 = 8'he6;
8'hf6:
Bjea64 = 8'h42;
8'hf7:
Bjea64 = 8'h68;
8'hf8:
Bjea64 = 8'h41;
8'hf9:
Bjea64 = 8'h99;
8'hfa:
Bjea64 = 8'h2d;
8'hfb:
Bjea64 = 8'h0f;
8'hfc:
Bjea64 = 8'hb0;
8'hfd:
Bjea64 = 8'h54;
8'hfe:
Bjea64 = 8'hbb;
8'hff:
Bjea64 = 8'h16;
endcase
endfunction

function [7:0] Yiea64;

input reg [7:0] Djea64;
case (Djea64)
8'b0:
Yiea64 = 8'h52;
8'b1:
Yiea64 = 8'h09;
8'h02:
Yiea64 = 8'h6a;
8'h03:
Yiea64 = 8'hd5;
8'h04:
Yiea64 = 8'h30;
8'h05:
Yiea64 = 8'h36;
8'h06:
Yiea64 = 8'ha5;
8'h07:
Yiea64 = 8'h38;
8'h08:
Yiea64 = 8'hbf;
8'h09:
Yiea64 = 8'h40;
8'h0a:
Yiea64 = 8'ha3;
8'h0b:
Yiea64 = 8'h9e;
8'h0c:
Yiea64 = 8'h81;
8'h0d:
Yiea64 = 8'hf3;
8'h0e:
Yiea64 = 8'hd7;
8'h0f:
Yiea64 = 8'hfb;
8'h10:
Yiea64 = 8'h7c;
8'h11:
Yiea64 = 8'he3;
8'h12:
Yiea64 = 8'h39;
8'h13:
Yiea64 = 8'h82;
8'h14:
Yiea64 = 8'h9b;
8'h15:
Yiea64 = 8'h2f;
8'h16:
Yiea64 = 8'hff;
8'h17:
Yiea64 = 8'h87;
8'h18:
Yiea64 = 8'h34;
8'h19:
Yiea64 = 8'h8e;
8'h1a:
Yiea64 = 8'h43;
8'h1b:
Yiea64 = 8'h44;
8'h1c:
Yiea64 = 8'hc4;
8'h1d:
Yiea64 = 8'hde;
8'h1e:
Yiea64 = 8'he9;
8'h1f:
Yiea64 = 8'hcb;
8'h20:
Yiea64 = 8'h54;
8'h21:
Yiea64 = 8'h7b;
8'h22:
Yiea64 = 8'h94;
8'h23:
Yiea64 = 8'h32;
8'h24:
Yiea64 = 8'ha6;
8'h25:
Yiea64 = 8'hc2;
8'h26:
Yiea64 = 8'h23;
8'h27:
Yiea64 = 8'h3d;
8'h28:
Yiea64 = 8'hee;
8'h29:
Yiea64 = 8'h4c;
8'h2a:
Yiea64 = 8'h95;
8'h2b:
Yiea64 = 8'h0b;
8'h2c:
Yiea64 = 8'h42;
8'h2d:
Yiea64 = 8'hfa;
8'h2e:
Yiea64 = 8'hc3;
8'h2f:
Yiea64 = 8'h4e;
8'h30:
Yiea64 = 8'h08;
8'h31:
Yiea64 = 8'h2e;
8'h32:
Yiea64 = 8'ha1;
8'h33:
Yiea64 = 8'h66;
8'h34:
Yiea64 = 8'h28;
8'h35:
Yiea64 = 8'hd9;
8'h36:
Yiea64 = 8'h24;
8'h37:
Yiea64 = 8'hb2;
8'h38:
Yiea64 = 8'h76;
8'h39:
Yiea64 = 8'h5b;
8'h3a:
Yiea64 = 8'ha2;
8'h3b:
Yiea64 = 8'h49;
8'h3c:
Yiea64 = 8'h6d;
8'h3d:
Yiea64 = 8'h8b;
8'h3e:
Yiea64 = 8'hd1;
8'h3f:
Yiea64 = 8'h25;
8'h40:
Yiea64 = 8'h72;
8'h41:
Yiea64 = 8'hf8;
8'h42:
Yiea64 = 8'hf6;
8'h43:
Yiea64 = 8'h64;
8'h44:
Yiea64 = 8'h86;
8'h45:
Yiea64 = 8'h68;
8'h46:
Yiea64 = 8'h98;
8'h47:
Yiea64 = 8'h16;
8'h48:
Yiea64 = 8'hd4;
8'h49:
Yiea64 = 8'ha4;
8'h4a:
Yiea64 = 8'h5c;
8'h4b:
Yiea64 = 8'hcc;
8'h4c:
Yiea64 = 8'h5d;
8'h4d:
Yiea64 = 8'h65;
8'h4e:
Yiea64 = 8'hb6;
8'h4f:
Yiea64 = 8'h92;
8'h50:
Yiea64 = 8'h6c;
8'h51:
Yiea64 = 8'h70;
8'h52:
Yiea64 = 8'h48;
8'h53:
Yiea64 = 8'h50;
8'h54:
Yiea64 = 8'hfd;
8'h55:
Yiea64 = 8'hed;
8'h56:
Yiea64 = 8'hb9;
8'h57:
Yiea64 = 8'hda;
8'h58:
Yiea64 = 8'h5e;
8'h59:
Yiea64 = 8'h15;
8'h5a:
Yiea64 = 8'h46;
8'h5b:
Yiea64 = 8'h57;
8'h5c:
Yiea64 = 8'ha7;
8'h5d:
Yiea64 = 8'h8d;
8'h5e:
Yiea64 = 8'h9d;
8'h5f:
Yiea64 = 8'h84;
8'h60:
Yiea64 = 8'h90;
8'h61:
Yiea64 = 8'hd8;
8'h62:
Yiea64 = 8'hab;
8'h63:
Yiea64 = 8'b0;
8'h64:
Yiea64 = 8'h8c;
8'h65:
Yiea64 = 8'hbc;
8'h66:
Yiea64 = 8'hd3;
8'h67:
Yiea64 = 8'h0a;
8'h68:
Yiea64 = 8'hf7;
8'h69:
Yiea64 = 8'he4;
8'h6a:
Yiea64 = 8'h58;
8'h6b:
Yiea64 = 8'h05;
8'h6c:
Yiea64 = 8'hb8;
8'h6d:
Yiea64 = 8'hb3;
8'h6e:
Yiea64 = 8'h45;
8'h6f:
Yiea64 = 8'h06;
8'h70:
Yiea64 = 8'hd0;
8'h71:
Yiea64 = 8'h2c;
8'h72:
Yiea64 = 8'h1e;
8'h73:
Yiea64 = 8'h8f;
8'h74:
Yiea64 = 8'hca;
8'h75:
Yiea64 = 8'h3f;
8'h76:
Yiea64 = 8'h0f;
8'h77:
Yiea64 = 8'h02;
8'h78:
Yiea64 = 8'hc1;
8'h79:
Yiea64 = 8'haf;
8'h7a:
Yiea64 = 8'hbd;
8'h7b:
Yiea64 = 8'h03;
8'h7c:
Yiea64 = 8'b1;
8'h7d:
Yiea64 = 8'h13;
8'h7e:
Yiea64 = 8'h8a;
8'h7f:
Yiea64 = 8'h6b;
8'h80:
Yiea64 = 8'h3a;
8'h81:
Yiea64 = 8'h91;
8'h82:
Yiea64 = 8'h11;
8'h83:
Yiea64 = 8'h41;
8'h84:
Yiea64 = 8'h4f;
8'h85:
Yiea64 = 8'h67;
8'h86:
Yiea64 = 8'hdc;
8'h87:
Yiea64 = 8'hea;
8'h88:
Yiea64 = 8'h97;
8'h89:
Yiea64 = 8'hf2;
8'h8a:
Yiea64 = 8'hcf;
8'h8b:
Yiea64 = 8'hce;
8'h8c:
Yiea64 = 8'hf0;
8'h8d:
Yiea64 = 8'hb4;
8'h8e:
Yiea64 = 8'he6;
8'h8f:
Yiea64 = 8'h73;
8'h90:
Yiea64 = 8'h96;
8'h91:
Yiea64 = 8'hac;
8'h92:
Yiea64 = 8'h74;
8'h93:
Yiea64 = 8'h22;
8'h94:
Yiea64 = 8'he7;
8'h95:
Yiea64 = 8'had;
8'h96:
Yiea64 = 8'h35;
8'h97:
Yiea64 = 8'h85;
8'h98:
Yiea64 = 8'he2;
8'h99:
Yiea64 = 8'hf9;
8'h9a:
Yiea64 = 8'h37;
8'h9b:
Yiea64 = 8'he8;
8'h9c:
Yiea64 = 8'h1c;
8'h9d:
Yiea64 = 8'h75;
8'h9e:
Yiea64 = 8'hdf;
8'h9f:
Yiea64 = 8'h6e;
8'ha0:
Yiea64 = 8'h47;
8'ha1:
Yiea64 = 8'hf1;
8'ha2:
Yiea64 = 8'h1a;
8'ha3:
Yiea64 = 8'h71;
8'ha4:
Yiea64 = 8'h1d;
8'ha5:
Yiea64 = 8'h29;
8'ha6:
Yiea64 = 8'hc5;
8'ha7:
Yiea64 = 8'h89;
8'ha8:
Yiea64 = 8'h6f;
8'ha9:
Yiea64 = 8'hb7;
8'haa:
Yiea64 = 8'h62;
8'hab:
Yiea64 = 8'h0e;
8'hac:
Yiea64 = 8'haa;
8'had:
Yiea64 = 8'h18;
8'hae:
Yiea64 = 8'hbe;
8'haf:
Yiea64 = 8'h1b;
8'hb0:
Yiea64 = 8'hfc;
8'hb1:
Yiea64 = 8'h56;
8'hb2:
Yiea64 = 8'h3e;
8'hb3:
Yiea64 = 8'h4b;
8'hb4:
Yiea64 = 8'hc6;
8'hb5:
Yiea64 = 8'hd2;
8'hb6:
Yiea64 = 8'h79;
8'hb7:
Yiea64 = 8'h20;
8'hb8:
Yiea64 = 8'h9a;
8'hb9:
Yiea64 = 8'hdb;
8'hba:
Yiea64 = 8'hc0;
8'hbb:
Yiea64 = 8'hfe;
8'hbc:
Yiea64 = 8'h78;
8'hbd:
Yiea64 = 8'hcd;
8'hbe:
Yiea64 = 8'h5a;
8'hbf:
Yiea64 = 8'hf4;
8'hc0:
Yiea64 = 8'h1f;
8'hc1:
Yiea64 = 8'hdd;
8'hc2:
Yiea64 = 8'ha8;
8'hc3:
Yiea64 = 8'h33;
8'hc4:
Yiea64 = 8'h88;
8'hc5:
Yiea64 = 8'h07;
8'hc6:
Yiea64 = 8'hc7;
8'hc7:
Yiea64 = 8'h31;
8'hc8:
Yiea64 = 8'hb1;
8'hc9:
Yiea64 = 8'h12;
8'hca:
Yiea64 = 8'h10;
8'hcb:
Yiea64 = 8'h59;
8'hcc:
Yiea64 = 8'h27;
8'hcd:
Yiea64 = 8'h80;
8'hce:
Yiea64 = 8'hec;
8'hcf:
Yiea64 = 8'h5f;
8'hd0:
Yiea64 = 8'h60;
8'hd1:
Yiea64 = 8'h51;
8'hd2:
Yiea64 = 8'h7f;
8'hd3:
Yiea64 = 8'ha9;
8'hd4:
Yiea64 = 8'h19;
8'hd5:
Yiea64 = 8'hb5;
8'hd6:
Yiea64 = 8'h4a;
8'hd7:
Yiea64 = 8'h0d;
8'hd8:
Yiea64 = 8'h2d;
8'hd9:
Yiea64 = 8'he5;
8'hda:
Yiea64 = 8'h7a;
8'hdb:
Yiea64 = 8'h9f;
8'hdc:
Yiea64 = 8'h93;
8'hdd:
Yiea64 = 8'hc9;
8'hde:
Yiea64 = 8'h9c;
8'hdf:
Yiea64 = 8'hef;
8'he0:
Yiea64 = 8'ha0;
8'he1:
Yiea64 = 8'he0;
8'he2:
Yiea64 = 8'h3b;
8'he3:
Yiea64 = 8'h4d;
8'he4:
Yiea64 = 8'hae;
8'he5:
Yiea64 = 8'h2a;
8'he6:
Yiea64 = 8'hf5;
8'he7:
Yiea64 = 8'hb0;
8'he8:
Yiea64 = 8'hc8;
8'he9:
Yiea64 = 8'heb;
8'hea:
Yiea64 = 8'hbb;
8'heb:
Yiea64 = 8'h3c;
8'hec:
Yiea64 = 8'h83;
8'hed:
Yiea64 = 8'h53;
8'hee:
Yiea64 = 8'h99;
8'hef:
Yiea64 = 8'h61;
8'hf0:
Yiea64 = 8'h17;
8'hf1:
Yiea64 = 8'h2b;
8'hf2:
Yiea64 = 8'h04;
8'hf3:
Yiea64 = 8'h7e;
8'hf4:
Yiea64 = 8'hba;
8'hf5:
Yiea64 = 8'h77;
8'hf6:
Yiea64 = 8'hd6;
8'hf7:
Yiea64 = 8'h26;
8'hf8:
Yiea64 = 8'he1;
8'hf9:
Yiea64 = 8'h69;
8'hfa:
Yiea64 = 8'h14;
8'hfb:
Yiea64 = 8'h63;
8'hfc:
Yiea64 = 8'h55;
8'hfd:
Yiea64 = 8'h21;
8'hfe:
Yiea64 = 8'h0c;
8'hff:
Yiea64 = 8'h7d;
endcase
endfunction
end
else 
if (Shea64 == "ALG") begin : Fjea64 

wire [7:0] Qjea64;
wire [7:0] Ujea64;
wire [7:0] Yjea64;
wire [7:0] Fkea64;
wire [3:0] Jkea64;
wire [3:0] Okea64;
wire [3:0] Tkea64;
wire [3:0] Zkea64;
wire [3:0] Elea64;
wire [1:0] Klea64;
wire [1:0] Plea64;
wire [1:0] Ulea64;
wire [1:0] Bmea64;
wire [1:0] Imea64;

assign Qjea64[0] = (Ugda64 ? ((Aiea64[7] ~^ Aiea64[5]) ^ Aiea64[2]) : Aiea64[0])
;
assign Qjea64[1] = (Ugda64 ? ((Aiea64[0] ^ Aiea64[6]) ^ Aiea64[3]) : Aiea64[1]);
assign Qjea64[2] = (Ugda64 ? ((Aiea64[1] ^ Aiea64[7]) ~^ Aiea64[4]) : Aiea64[2])
;
assign Qjea64[3] = (Ugda64 ? ((Aiea64[2] ^ Aiea64[0]) ^ Aiea64[5]) : Aiea64[3]);
assign Qjea64[4] = (Ugda64 ? ((Aiea64[3] ^ Aiea64[1]) ^ Aiea64[6]) : Aiea64[4]);
assign Qjea64[5] = (Ugda64 ? ((Aiea64[4] ^ Aiea64[2]) ^ Aiea64[7]) : Aiea64[5]);
assign Qjea64[6] = (Ugda64 ? ((Aiea64[5] ^ Aiea64[3]) ^ Aiea64[0]) : Aiea64[6]);
assign Qjea64[7] = (Ugda64 ? ((Aiea64[6] ^ Aiea64[4]) ^ Aiea64[1]) : Aiea64[7]);
assign Ujea64[0] = ((((Qjea64[6] ^ Qjea64[3]) ^ Qjea64[2]) ^ Qjea64[1]) ^ 
Qjea64[0]);
assign Ujea64[1] = ((Qjea64[6] ^ Qjea64[5]) ^ Qjea64[0]);
assign Ujea64[2] = Qjea64[0];
assign Ujea64[3] = ((((Qjea64[7] ^ Qjea64[4]) ^ Qjea64[3]) ^ Qjea64[1]) ^ 
Qjea64[0]);
assign Ujea64[4] = (((Qjea64[7] ^ Qjea64[6]) ^ Qjea64[5]) ^ Qjea64[0]);
assign Ujea64[5] = (((Qjea64[6] ^ Qjea64[5]) ^ Qjea64[1]) ^ Qjea64[0]);
assign Ujea64[6] = (((Qjea64[6] ^ Qjea64[5]) ^ Qjea64[4]) ^ Qjea64[0]);
assign Ujea64[7] = (((((Qjea64[7] ^ Qjea64[6]) ^ Qjea64[5]) ^ Qjea64[2]) ^ 
Qjea64[1]) ^ Qjea64[0]);
assign Jkea64 = Ujea64[7:4];
assign Okea64 = Ujea64[3:0];
assign Tkea64[3] = ((((((Jkea64[1] ^ Jkea64[3]) & (Okea64[1] ^ Okea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Okea64[0] ^ Okea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Okea64[1] ^ Okea64[2]))) ^ ((Jkea64[0] ^ Jkea64[1]) & (Okea64[0] ^ Okea64[1])))
^ (Jkea64[3] & Okea64[3]));
assign Tkea64[2] = (((((Jkea64[2] ^ Jkea64[3]) & (Okea64[2] ^ Okea64[3])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Okea64[0] ^ Okea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Okea64[1] ^ Okea64[3]))) ^ (Jkea64[2] & Okea64[2]));
assign Tkea64[1] = ((((((Jkea64[1] ^ Jkea64[3]) & (Okea64[1] ^ Okea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Okea64[0] ^ Okea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Okea64[1] ^ Okea64[2]))) ^ ((Jkea64[2] ^ Jkea64[3]) & (Okea64[2] ^ Okea64[3])))
^ (Jkea64[1] & Okea64[1]));
assign Tkea64[0] = (((((Jkea64[0] ^ Jkea64[1]) & (Okea64[0] ^ Okea64[1])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Okea64[0] ^ Okea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Okea64[1] ^ Okea64[3]))) ^ (Jkea64[0] & Okea64[0]));
assign Zkea64[3] = (((Jkea64[0] ^ Okea64[0]) ^ Jkea64[2]) ^ Okea64[2]);
assign Zkea64[2] = (((Jkea64[1] ^ Okea64[1]) ^ Jkea64[3]) ^ Okea64[3]);
assign Zkea64[1] = (((Jkea64[0] ^ Okea64[0]) ^ Jkea64[1]) ^ Okea64[1]);
assign Zkea64[0] = (Jkea64[0] ^ Okea64[0]);
assign {Klea64, Plea64} = (Tkea64 ^ Zkea64);
assign Bmea64[0] = (((Klea64[1] ^ Klea64[0]) & (Plea64[0] ^ Plea64[1])) ^ (
Klea64[0] & Plea64[0]));
assign Bmea64[1] = (((Klea64[1] ^ Klea64[0]) & (Plea64[0] ^ Plea64[1])) ^ (
Klea64[1] & Plea64[1]));
assign Imea64[0] = (((Klea64[1] ^ Plea64[1]) ^ Klea64[0]) ^ Plea64[0]);
assign Imea64[1] = (Klea64[1] ^ Plea64[1]);
assign {Ulea64[0], Ulea64[1]} = (Bmea64 ^ Imea64);
assign Elea64[0] = (((Klea64[0] ^ Klea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Klea64[0] & Ulea64[0]));
assign Elea64[1] = (((Klea64[0] ^ Klea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Klea64[1] & Ulea64[1]));
assign Elea64[2] = (((Plea64[0] ^ Plea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Plea64[0] & Ulea64[0]));
assign Elea64[3] = (((Plea64[0] ^ Plea64[1]) & (Ulea64[0] ^ Ulea64[1])) ^ (
Plea64[1] & Ulea64[1]));
assign Yjea64[7] = ((((((Okea64[1] ^ Okea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Okea64[0] ^ Okea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Okea64[1] ^ Okea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Okea64[3] & Elea64[3])) ^ ((Okea64[0] ^ Okea64[1]) 
& (Elea64[0] ^ Elea64[1])));
assign Yjea64[5] = ((((((Okea64[1] ^ Okea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Okea64[0] ^ Okea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Okea64[1] ^ Okea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Okea64[1] & Elea64[1])) ^ ((Okea64[2] ^ Okea64[3]) 
& (Elea64[2] ^ Elea64[3])));
assign Yjea64[3] = ((((((Jkea64[1] ^ Jkea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Jkea64[3] & Elea64[3])) ^ ((Jkea64[0] ^ Jkea64[1]) 
& (Elea64[0] ^ Elea64[1])));
assign Yjea64[1] = ((((((Jkea64[1] ^ Jkea64[3]) & (Elea64[1] ^ Elea64[3])) ^ ((
Jkea64[0] ^ Jkea64[3]) & (Elea64[0] ^ Elea64[3]))) ^ ((Jkea64[1] ^ Jkea64[2]) & 
(Elea64[1] ^ Elea64[2]))) ^ (Jkea64[1] & Elea64[1])) ^ ((Jkea64[2] ^ Jkea64[3]) 
& (Elea64[2] ^ Elea64[3])));
assign Yjea64[6] = (((((Okea64[2] ^ Okea64[3]) & (Elea64[2] ^ Elea64[3])) ^ ((
Okea64[0] ^ Okea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Okea64[1] ^ Okea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Okea64[2] & Elea64[2]));
assign Yjea64[4] = (((((Okea64[0] ^ Okea64[1]) & (Elea64[0] ^ Elea64[1])) ^ ((
Okea64[0] ^ Okea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Okea64[1] ^ Okea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Okea64[0] & Elea64[0]));
assign Yjea64[2] = (((((Jkea64[2] ^ Jkea64[3]) & (Elea64[2] ^ Elea64[3])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Jkea64[2] & Elea64[2]));
assign Yjea64[0] = (((((Jkea64[0] ^ Jkea64[1]) & (Elea64[0] ^ Elea64[1])) ^ ((
Jkea64[0] ^ Jkea64[2]) & (Elea64[0] ^ Elea64[2]))) ^ ((Jkea64[1] ^ Jkea64[3]) & 
(Elea64[1] ^ Elea64[3]))) ^ (Jkea64[0] & Elea64[0]));
assign Fkea64[0] = Yjea64[2];
assign Fkea64[1] = (Yjea64[5] ^ Yjea64[1]);
assign Fkea64[2] = (((Yjea64[7] ^ Yjea64[5]) ^ Yjea64[4]) ^ Yjea64[1]);
assign Fkea64[3] = (((((Yjea64[6] ^ Yjea64[5]) ^ Yjea64[4]) ^ Yjea64[3]) ^ 
Yjea64[2]) ^ Yjea64[1]);
assign Fkea64[4] = (Yjea64[6] ^ Yjea64[1]);
assign Fkea64[5] = (((((Yjea64[7] ^ Yjea64[6]) ^ Yjea64[5]) ^ Yjea64[3]) ^ 
Yjea64[2]) ^ Yjea64[0]);
assign Fkea64[6] = (((((Yjea64[7] ^ Yjea64[6]) ^ Yjea64[5]) ^ Yjea64[3]) ^ 
Yjea64[1]) ^ Yjea64[0]);
assign Fkea64[7] = (Yjea64[4] ^ Yjea64[1]);
assign Giea64[0] = (Ugda64 ? Fkea64[0] : ((((Fkea64[0] ~^ Fkea64[7]) ^ Fkea64[6]
) ^ Fkea64[5]) ^ Fkea64[4]));
assign Giea64[1] = (Ugda64 ? Fkea64[1] : ((((Fkea64[1] ~^ Fkea64[0]) ^ Fkea64[7]
) ^ Fkea64[6]) ^ Fkea64[5]));
assign Giea64[2] = (Ugda64 ? Fkea64[2] : ((((Fkea64[2] ^ Fkea64[1]) ^ Fkea64[0])
^ Fkea64[7]) ^ Fkea64[6]));
assign Giea64[3] = (Ugda64 ? Fkea64[3] : ((((Fkea64[3] ^ Fkea64[2]) ^ Fkea64[1])
^ Fkea64[0]) ^ Fkea64[7]));
assign Giea64[4] = (Ugda64 ? Fkea64[4] : ((((Fkea64[4] ^ Fkea64[3]) ^ Fkea64[2])
^ Fkea64[1]) ^ Fkea64[0]));
assign Giea64[5] = (Ugda64 ? Fkea64[5] : ((((Fkea64[5] ~^ Fkea64[4]) ^ Fkea64[3]
) ^ Fkea64[2]) ^ Fkea64[1]));
assign Giea64[6] = (Ugda64 ? Fkea64[6] : ((((Fkea64[6] ~^ Fkea64[5]) ^ Fkea64[4]
) ^ Fkea64[3]) ^ Fkea64[2]));
assign Giea64[7] = (Ugda64 ? Fkea64[7] : ((((Fkea64[7] ^ Fkea64[6]) ^ Fkea64[5])
^ Fkea64[4]) ^ Fkea64[3]));
end
else  begin : Omea64 

end
endmodule

/*
                    instances: 0
                        nodes: 10 (0)
                  node widths: 128 (0)
                   contassign:  6 (0)
                        ports: 2 (0)
*/

//`timescale 1 ns / 1 ns
module Szda64(K1ea64, T1ea64);

input [31:0] K1ea64;
output [31:0] T1ea64;

wire [7:0] Orea64;
wire [7:0] Trea64;
wire [7:0] Yrea64;
wire [7:0] Dsea64;
wire [7:0] Isea64;
wire [7:0] Osea64;
wire [7:0] Usea64;
wire [7:0] Atea64;

assign {Orea64, Trea64, Yrea64, Dsea64} = K1ea64;
assign Isea64 = (((Gtea64(Orea64) ^ Ntea64(Trea64)) ^ Yrea64) ^ Dsea64);
assign Osea64 = (((Orea64 ^ Gtea64(Trea64)) ^ Ntea64(Yrea64)) ^ Dsea64);
assign Usea64 = (((Orea64 ^ Trea64) ^ Gtea64(Yrea64)) ^ Ntea64(Dsea64));
assign Atea64 = (((Ntea64(Orea64) ^ Trea64) ^ Yrea64) ^ Gtea64(Dsea64));
assign T1ea64 = {Isea64, Osea64, Usea64, Atea64};

function [7:0] Gtea64;

input reg [7:0] Ktea64;
begin
Gtea64 = ({Ktea64[6:0], 1'b0} ^ (8'h1b & {8 {Ktea64[7]}}));
end
endfunction

function [7:0] Ntea64;

input reg [7:0] Ktea64;
begin
Ntea64 = (Gtea64(Ktea64) ^ Ktea64);
end
endfunction
endmodule

/*
                    instances: 0
                        nodes: 10 (0)
                  node widths: 128 (0)
                   contassign:  6 (0)
                        ports: 2 (0)
*/

//`timescale 1 ns / 1 ns
module C2ea64(Y3ea64, I4ea64);

input [31:0] Y3ea64;
output [31:0] I4ea64;

wire [7:0] Orea64;
wire [7:0] Trea64;
wire [7:0] Yrea64;
wire [7:0] Dsea64;
wire [7:0] Cwea64;
wire [7:0] Jwea64;
wire [7:0] Qwea64;
wire [7:0] Xwea64;

assign {Orea64, Trea64, Yrea64, Dsea64} = Y3ea64;
assign Cwea64 = (((Yxea64(Orea64) ^ Qxea64(Trea64)) ^ Uxea64(Yrea64)) ^ Mxea64(
Dsea64));
assign Jwea64 = (((Mxea64(Orea64) ^ Yxea64(Trea64)) ^ Qxea64(Yrea64)) ^ Uxea64(
Dsea64));
assign Qwea64 = (((Uxea64(Orea64) ^ Mxea64(Trea64)) ^ Yxea64(Yrea64)) ^ Qxea64(
Dsea64));
assign Xwea64 = (((Qxea64(Orea64) ^ Uxea64(Trea64)) ^ Mxea64(Yrea64)) ^ Yxea64(
Dsea64));
assign I4ea64 = {Cwea64, Jwea64, Qwea64, Xwea64};

function [7:0] Gtea64;

input reg [7:0] Ktea64;
begin
Gtea64 = ({Ktea64[6:0], 1'b0} ^ (8'h1b & {8 {Ktea64[7]}}));
end
endfunction

function [7:0] Exea64;

input reg [7:0] Ktea64;
begin
Exea64 = Gtea64(Gtea64(Ktea64));
end
endfunction

function [7:0] Ixea64;

input reg [7:0] Ktea64;
begin
Ixea64 = Gtea64(Exea64(Ktea64));
end
endfunction

function [7:0] Mxea64;

input reg [7:0] Ktea64;
begin
Mxea64 = (Ixea64(Ktea64) ^ Ktea64);
end
endfunction

function [7:0] Qxea64;

input reg [7:0] Ktea64;
begin
Qxea64 = (Mxea64(Ktea64) ^ Gtea64(Ktea64));
end
endfunction

function [7:0] Uxea64;

input reg [7:0] Ktea64;
begin
Uxea64 = (Mxea64(Ktea64) ^ Exea64(Ktea64));
end
endfunction

function [7:0] Yxea64;

input reg [7:0] Ktea64;
begin
Yxea64 = ((Ixea64(Ktea64) ^ Exea64(Ktea64)) ^ Gtea64(Ktea64));
end
endfunction
endmodule

/*
                    instances: 0
                        nodes: 10 (0)
                  node widths: 512 (0)
                   contassign:  6 (0)
                        ports: 2 (0)
*/

//`timescale 1 ns / 1 ns
module Kyda64(Yxda64, Eyda64);

input [127:0] Yxda64;
output [127:0] Eyda64;

wire [31:0] Zzea64;
wire [31:0] E0fa64;
wire [31:0] J0fa64;
wire [31:0] O0fa64;
wire [31:0] T0fa64;
wire [31:0] C1fa64;
wire [31:0] L1fa64;
wire [31:0] U1fa64;

assign {Zzea64, E0fa64, J0fa64, O0fa64} = Yxda64;
assign T0fa64 = {Zzea64[31:24], E0fa64[23:16], J0fa64[15:8], O0fa64[7:0]};
assign C1fa64 = {E0fa64[31:24], J0fa64[23:16], O0fa64[15:8], Zzea64[7:0]};
assign L1fa64 = {J0fa64[31:24], O0fa64[23:16], Zzea64[15:8], E0fa64[7:0]};
assign U1fa64 = {O0fa64[31:24], Zzea64[23:16], E0fa64[15:8], J0fa64[7:0]};
assign Eyda64 = {T0fa64, C1fa64, L1fa64, U1fa64};
endmodule

/*
                    instances: 0
                        nodes: 10 (0)
                  node widths: 512 (0)
                   contassign:  6 (0)
                        ports: 2 (0)
*/

//`timescale 1 ns / 1 ns
module Mwda64(Yxda64, Eyda64);

input [127:0] Yxda64;
output [127:0] Eyda64;

wire [31:0] Zzea64;
wire [31:0] E0fa64;
wire [31:0] J0fa64;
wire [31:0] O0fa64;
wire [31:0] T0fa64;
wire [31:0] C1fa64;
wire [31:0] L1fa64;
wire [31:0] U1fa64;

assign {Zzea64, E0fa64, J0fa64, O0fa64} = Yxda64;
assign T0fa64 = {Zzea64[31:24], O0fa64[23:16], J0fa64[15:8], E0fa64[7:0]};
assign C1fa64 = {E0fa64[31:24], Zzea64[23:16], O0fa64[15:8], J0fa64[7:0]};
assign L1fa64 = {J0fa64[31:24], E0fa64[23:16], Zzea64[15:8], O0fa64[7:0]};
assign U1fa64 = {O0fa64[31:24], J0fa64[23:16], E0fa64[15:8], Zzea64[7:0]};
assign Eyda64 = {T0fa64, C1fa64, L1fa64, U1fa64};
endmodule

/*
                    instances: 0
                        nodes: 53 (0)
                  node widths: 79 (0)
                      process: 3 (0)
                   contassign:  41 (0)
                        ports: 20 (0)
*/

//`timescale 1 ns / 1 ns
module O5fa64(G8fa64, K8fa64, Q8fa64, D9fa64, N9fa64, Z9fa64, Nafa64, Abfa64,
Mbfa64, Ybfa64, Lcfa64, Xcfa64, Hdfa64, Sdfa64, Defa64, Qefa64, Cffa64, Offa64,
Xffa64, Jgfa64);

parameter A7fa64 = 1'b1;
parameter Q7fa64 = 1'b1;
localparam Vgfa64 = 3'b0;
localparam Chfa64 = 3'b1;
localparam Lhfa64 = 3'b010;
localparam Vhfa64 = 3'b011;
localparam Gifa64 = 3'b100;
localparam Mifa64 = 3'b101;
localparam Vifa64 = 3'b110;

input G8fa64;
input K8fa64;
input [3:0] Q8fa64;
input D9fa64;
input N9fa64;
output Z9fa64;
output Nafa64;
input Abfa64;
input Mbfa64;
output Ybfa64;
output Lcfa64;
output Xcfa64;
output Hdfa64;
output Sdfa64;
output Defa64;
output Qefa64;
output Cffa64;
output [1:0] Offa64;
output Xffa64;
output Jgfa64;

reg [2:0] Ejfa64;
reg [2:0] Sjfa64;
reg [5:0] Dkfa64;
wire [5:0] Jkfa64;
wire [5:0] Tkfa64;
reg [1:0] Dlfa64;
wire [1:0] Plfa64;
wire [1:0] Fmfa64;
wire Vmfa64;
wire Infa64;
wire Xnfa64;
wire Nofa64;
wire Epfa64;
wire Qpfa64;
wire Fqfa64;
wire Uqfa64;
wire Irfa64;
wire Vrfa64;
wire Isfa64;
reg Tsfa64;
wire Zsfa64;
wire Jtfa64;
wire Cufa64;
wire Wufa64;
wire Jvfa64;
wire Tvfa64;
wire Dwfa64;
wire Wbea64;
wire Owfa64;
wire Bxfa64;
wire Nxfa64;
wire Zxfa64;
wire Lyfa64;

assign Jvfa64 = (Q7fa64 && (Q8fa64 == 2));
assign Wufa64 = (A7fa64 && (Q8fa64 == 1));
assign Vmfa64 = (Ejfa64 == Vgfa64);
assign Infa64 = (Ejfa64 == Chfa64);
assign Xnfa64 = (Ejfa64 == Lhfa64);
assign Nofa64 = (Ejfa64 == Vhfa64);
assign Epfa64 = (Ejfa64 == Gifa64);
assign Qpfa64 = (Ejfa64 == Mifa64);
assign Fqfa64 = (Ejfa64 == Vifa64);
assign Uqfa64 = ((((((((Jvfa64 && Infa64) && (Dkfa64 == (Tkfa64 + 6'd2))) || ((
Jvfa64 && Xnfa64) && (Dkfa64 == (Tkfa64 + 6'd2)))) || ((Jvfa64 && Nofa64) && (
Dkfa64 == (Tkfa64 + 6'd2)))) || ((Wufa64 && Infa64) && (Dkfa64 == (Tkfa64 + 6'b1
)))) || ((Wufa64 && Xnfa64) && (Dkfa64 == (Tkfa64 + 6'b1)))) || ((Wufa64 && 
Nofa64) && (Dkfa64 == (Tkfa64 + 6'b1)))) || (N9fa64 && (!D9fa64)));
assign Irfa64 = ((Vmfa64 || Infa64) && Uqfa64);
assign Isfa64 = (Wufa64 ? (Qpfa64 && (Dkfa64 == Tkfa64)) : (Jvfa64 ? (Qpfa64 && 
(Dkfa64 > Tkfa64)) : 1'b0));
assign Vrfa64 = ((Epfa64 && Wufa64) ? (Dlfa64 == Fmfa64) : ((Epfa64 && Jvfa64) ?
(Dlfa64 == Fmfa64) : 1'b0));
assign Tkfa64 = (Wufa64 ? 6'd10 : (Jvfa64 ? 6'd31 : 6'b0));
assign Jkfa64 = (Uqfa64 ? 6'b0 : (Isfa64 ? 6'b0 : (Infa64 ? (Dkfa64 + 6'b1) : (
Xnfa64 ? (Dkfa64 + 6'b1) : (Nofa64 ? (Dkfa64 + 6'b1) : ((Epfa64 && Vrfa64) ? (
Dkfa64 + 6'b1) : Dkfa64))))));
assign Fmfa64 = (Wufa64 ? 2'd3 : (Jvfa64 ? 2'b0 : 2'b0));
assign Plfa64 = ((Dlfa64 == Fmfa64) ? 2'b0 : (((Epfa64 && Wufa64) && (Dlfa64 != 
Fmfa64)) ? (Dlfa64 + 1'b1) : (((Epfa64 && Jvfa64) && (Dlfa64 != Fmfa64)) ? (
Dlfa64 + 1'b1) : Dlfa64)));
assign Zsfa64 = (N9fa64 ? D9fa64 : (Xnfa64 ? 1'b1 : (Nofa64 ? 1'b0 : Tsfa64)));
assign Jtfa64 = ((!Tsfa64) && Abfa64);
assign Cufa64 = (Tsfa64 && (!Abfa64));
assign Tvfa64 = ((!(|Dkfa64)) && (((Infa64 || Xnfa64) || Nofa64) || Fqfa64));
assign Dwfa64 = (((Infa64 || Xnfa64) || Nofa64) || Qpfa64);
assign Wbea64 = (Infa64 ? 1'b0 : (Xnfa64 ? 1'b0 : (Nofa64 ? 1'b1 : D9fa64)));
assign Owfa64 = ((!Vmfa64) && Uqfa64);
assign Bxfa64 = (Wufa64 ? (Epfa64 || Qpfa64) : (Jvfa64 ? Epfa64 : 1'b0));
assign Nxfa64 = Abfa64;
assign Zxfa64 = ((Qpfa64 || Epfa64) && (!(|Dkfa64)));
assign Lyfa64 = ((Qpfa64 && (Dkfa64 == Tkfa64)) || (Epfa64 && (Dkfa64 == (Tkfa64
- 6'b1))));
assign Z9fa64 = Infa64;
assign Nafa64 = Irfa64;
assign Ybfa64 = (!(Vmfa64 || Infa64));
assign Lcfa64 = Isfa64;
assign Xcfa64 = Tvfa64;
assign Hdfa64 = Dwfa64;
assign Sdfa64 = Wbea64;
assign Defa64 = Owfa64;
assign Qefa64 = Bxfa64;
assign Cffa64 = Nxfa64;
assign Offa64 = Dlfa64;
assign Xffa64 = Zxfa64;
assign Jgfa64 = Lyfa64;

always @(posedge G8fa64 or negedge K8fa64) begin
if (!K8fa64) begin
Ejfa64 <= Vgfa64;
end
else begin
Ejfa64 <=  Sjfa64;
end
end
always @(*) begin
Sjfa64 = Ejfa64;
case (Ejfa64)
Vgfa64:
Sjfa64 = ((N9fa64 && D9fa64) ? Chfa64 : ((Jtfa64 && Mbfa64) ? Lhfa64 : ((Cufa64 
&& Mbfa64) ? Vhfa64 : (Mbfa64 ? Vifa64 : Vgfa64))));
Chfa64:
Sjfa64 = (Uqfa64 ? Vgfa64 : Chfa64);
Lhfa64:
Sjfa64 = (Uqfa64 ? Vifa64 : Lhfa64);
Vhfa64:
Sjfa64 = (Uqfa64 ? Vifa64 : Vhfa64);
Vifa64:
Sjfa64 = ((Jvfa64 && Abfa64) ? Gifa64 : Mifa64);
Gifa64:
Sjfa64 = (Vrfa64 ? Mifa64 : Gifa64);
Mifa64:
Sjfa64 = (Isfa64 ? Vgfa64 : Gifa64);
default:
Sjfa64 = 'bxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
endcase
end
always @(posedge G8fa64 or negedge K8fa64) begin
if (!K8fa64) begin
Dkfa64 <= 6'b0;
Tsfa64 <= 1'b0;
Dlfa64 <= 2'b0;
end
else
begin
Dkfa64 <=  Jkfa64;
Tsfa64 <=  Zsfa64;
Dlfa64 <=  Plfa64;
end
end
endmodule

/*
                    instances: 0
                        nodes: 29 (0)
                  node widths: 984 (0)
                      process: 1 (0)
                   contassign:  6 (0)
                        ports: 19 (0)
*/

//`timescale 1 ns / 1 ns
module F0ga64(G8fa64, K8fa64, Q8fa64, N1ga64, Sida64, Ajda64, A2ga64, Abfa64,
M2ga64, Jhda64, Z2ga64, Ahda64, K3ga64, W3ga64, Lida64, I4ga64, W4ga64, K5ga64,
V5ga64);

parameter A7fa64 = 1;
parameter Q7fa64 = 1;

input G8fa64;
input K8fa64;
input [3:0] Q8fa64;
input [3:0] N1ga64;
input [31:0] Sida64;
output [127:0] Ajda64;
input A2ga64;
input Abfa64;
input M2ga64;
input Jhda64;
input Z2ga64;
input [1:0] Ahda64;
input K3ga64;
input W3ga64;
input [127:0] Lida64;
output [31:0] I4ga64;
input [31:0] W4ga64;
output [31:0] K5ga64;
input [31:0] V5ga64;

genvar  G6ga64;
reg [127:0] I6ga64;
wire [127:0] P6ga64;
wire A7ga64;
wire [127:0] T7ga64;
wire Wufa64;
wire I8ga64;
wire D9ga64;
wire [127:0] Y9ga64;
wire Jvfa64;

assign Jvfa64 = (Q7fa64 & (Q8fa64 == 2));
assign Wufa64 = (A7fa64 & (Q8fa64 == 1));
assign A7ga64 = ((|N1ga64) && (!M2ga64));
assign I8ga64 = (((!Abfa64) && K3ga64) && Jhda64);
assign D9ga64 = ((Abfa64 && W3ga64) && Jhda64);
assign Ajda64 = I6ga64;

always @(posedge G8fa64 or negedge K8fa64) begin
if (!K8fa64) begin
I6ga64 <= 128'b0;
end
else
begin
I6ga64 <=  P6ga64;
end
end

for (G6ga64 = 0; (G6ga64 < 4); G6ga64 = (G6ga64 + 1)) begin : Kaga64 


assign P6ga64[(G6ga64 * 32)+:32] = ((A7ga64 && N1ga64[G6ga64]) ? Sida64 : ((
Jvfa64 && Z2ga64) ? I6ga64[((3 - G6ga64) * 32)+:32] : ((Jvfa64 && A2ga64) ? 
Y9ga64[(G6ga64 * 32)+:32] : ((Wufa64 && I8ga64) ? (I6ga64[(G6ga64 * 32)+:32] ^ 
Lida64[(G6ga64 * 32)+:32]) : ((Wufa64 && D9ga64) ? (I6ga64[(G6ga64 * 32)+:32] ^ 
Lida64[(G6ga64 * 32)+:32]) : ((Wufa64 && A2ga64) ? T7ga64[(G6ga64 * 32)+:32] : 
I6ga64[(G6ga64 * 32)+:32]))))));
end

if (A7fa64) begin : Qaga64 

Neda64 Cbga64( .Ugda64 (Abfa64),  .Ahda64 (Ahda64),  .Jhda64 (Jhda64),  .Thda64
(K3ga64),  .Cida64 (W3ga64),  .Lida64 (Lida64),  .Sida64 (I6ga64),  .Ajda64
(T7ga64),  .Ijda64 (I4ga64),  .Pjda64 (W4ga64));
end

if (Q7fa64) begin : Vbga64 

Hcga64 Cdga64( .Lida64 (Lida64),  .Sida64 (I6ga64),  .Ajda64 (Y9ga64),  .Ijda64
(K5ga64),  .Pjda64 (V5ga64));
end
endmodule

/*
                    instances: 0
                        nodes: 32 (0)
                  node widths: 1330 (0)
                      process: 1 (0)
                   contassign:  8 (0)
                        ports: 16 (0)
*/

//`timescale 1 ns / 1 ns
module Ffga64(G8fa64, K8fa64, Q8fa64, Rgga64, Xgga64, Ihga64, Ohga64, Ciga64,
Miga64, Xiga64, Ijga64, K9ea64, I4ga64, W4ga64, K5ga64, V5ga64);

parameter A7fa64 = 1;
parameter Q7fa64 = 1;
localparam [127:0] Vjga64 = 128'ha3b1bac656aa3350677d9197b27022dc;

input G8fa64;
input K8fa64;
input [3:0] Q8fa64;
input [31:0] Rgga64;
input [3:0] Xgga64;
output [127:0] Ihga64;
input Ohga64;
input Ciga64;
input Miga64;
input Xiga64;
input Ijga64;
output [127:0] K9ea64;
output [31:0] I4ga64;
input [31:0] W4ga64;
output [31:0] K5ga64;
input [31:0] V5ga64;

genvar  G6ga64;
reg [127:0] Ekga64;
wire [127:0] Lkga64;
wire Wkga64;
reg [127:0] Nlga64;
wire [127:0] Ulga64;
wire [127:0] Fmga64;
reg [7:0] Tmga64;
wire [7:0] Anga64;
wire [7:0] Hnga64;
wire [7:0] Snga64;
wire [127:0] Doga64;
reg [31:0] Ooga64;
wire [31:0] Uoga64;
wire Wufa64;
wire Jvfa64;

assign Jvfa64 = (Q7fa64 & (Q8fa64 == 2));
assign Wufa64 = (A7fa64 & (Q8fa64 == 1));
assign Wkga64 = ((|Xgga64) && (!Ohga64));
assign Ulga64 = (((Ciga64 && Jvfa64) && (!Xiga64)) ? (Ekga64 ^ Vjga64) : (Ciga64
? Ekga64 : ((Miga64 && Jvfa64) ? Doga64 : ((Miga64 && Wufa64) ? Fmga64 : Nlga64)
)));
assign Anga64 = {Tmga64[6], Tmga64[5], Tmga64[4], (Tmga64[3] ^ Tmga64[7]),
(Tmga64[2] ^ Tmga64[7]), Tmga64[1], (Tmga64[0] ^ Tmga64[7]), Tmga64[7]};
assign Hnga64 = {Tmga64[0], Tmga64[7], Tmga64[6], Tmga64[5], (Tmga64[4] ^
Tmga64[0]), (Tmga64[3] ^ Tmga64[0]), Tmga64[2], (Tmga64[1] ^ Tmga64[0])};
assign Ihga64 = Ekga64;
assign K9ea64 = Nlga64;

always @(posedge G8fa64 or negedge K8fa64) begin
if (!K8fa64) begin
Ekga64 <= 128'b0;
Nlga64 <= 128'b0;
Tmga64 <= 8'b0;
Ooga64 <= 32'b0;
end
else
begin
Ekga64 <=  Lkga64;
Nlga64 <=  Ulga64;
Tmga64 <=  Snga64;
Ooga64 <=  Uoga64;
end
end

for (G6ga64 = 0; (G6ga64 < 4); G6ga64 = (G6ga64 + 1)) begin : Epga64 


assign Lkga64[(G6ga64 * 32)+:32] = ((Wkga64 && Xgga64[G6ga64]) ? Rgga64 : (((
Ijga64 && Jvfa64) && Xiga64) ? (Nlga64[(G6ga64 * 32)+:32] ^ Vjga64[(G6ga64 *
32)+:32]) : (Ijga64 ? Nlga64[(G6ga64 * 32)+:32] : Ekga64[(G6ga64 * 32)+:32])));
end

if (A7fa64) begin : Kpga64 

assign Snga64 = ((Ciga64 && Xiga64) ? 8'b00110110 : (Ciga64 ? 8'b1 : ((Miga64 &&
Xiga64) ? Hnga64 : (Miga64 ? Anga64 : Tmga64))));
end

if (Q7fa64) begin : Upga64 

assign Uoga64 = ((Ciga64 && Xiga64) ? 32'h646b7279 : (Ciga64 ? 32'h00070e15 : ((
Miga64 && Xiga64) ? {(Ooga64[(3 * 8)+:8] - 8'h1c), (Ooga64[(2 * 8)+:8] - 8'h1c),
(Ooga64[(1 * 8)+:8] - 8'h1c), (Ooga64[(0 * 8)+:8] - 8'h1c)} : (Miga64 ? 
{(Ooga64[(3 * 8)+:8] + 8'h1c), (Ooga64[(2 * 8)+:8] + 8'h1c), (Ooga64[(1 * 8)+:8]
+ 8'h1c), (Ooga64[(0 * 8)+:8] + 8'h1c)} : Ooga64))));
end

if (A7fa64) begin : Dqga64 

X6ea64 #(.L8ea64(128)) Qqga64( .U8ea64 (2'b0),  .Ugda64 (Xiga64),  .D9ea64
(Tmga64),  .Lida64 (Nlga64),  .K9ea64 (Fmga64),  .Ijda64 (I4ga64),  .Pjda64
(W4ga64));
end

if (Q7fa64) begin : Irga64 

Vrga64 #(.L8ea64(128)) Psga64( .U8ea64 (2'b0),  .Ugda64 (Xiga64),  .Htga64
(Ooga64),  .Lida64 (Nlga64),  .K9ea64 (Doga64),  .Ijda64 (K5ga64),  .Pjda64
(V5ga64));
end
endmodule

/*
                    instances: 0
                        nodes: 13 (0)
                  node widths: 261 (0)
                   contassign:  3 (0)
                        ports: 9 (0)
*/

//`timescale 1 ns / 1 ns
module Suga64(D9fa64, Abfa64, Miga64, Wvga64, I4ga64, Mwga64, K5ga64, Zwga64,
Pxga64);

parameter A7fa64 = 1;
parameter Q7fa64 = 1;
parameter Shea64 = "ALG";

input D9fa64;
input Abfa64;
input Miga64;
input [31:0] Wvga64;
output [31:0] I4ga64;
input [31:0] Mwga64;
output [31:0] K5ga64;
input [31:0] Zwga64;
input [31:0] Pxga64;

wire Cyga64;
wire Uyga64;
wire [31:0] Jzga64;
wire [31:0] Yzga64;

assign Cyga64 = (Miga64 ? 1'b0 : Abfa64);
assign Jzga64 = (Miga64 ? Wvga64 : Zwga64);
assign Yzga64 = (Miga64 ? Mwga64 : Pxga64);

if (A7fa64) begin : K0ha64 

Hoea64 #(.Shea64(Shea64)) U0ha64( .Ugda64 (Cyga64),  .Aiea64 (Jzga64[(0 *
8)+:8]),  .Giea64 (I4ga64[(0 * 8)+:8]));
Hoea64 #(.Shea64(Shea64)) G1ha64( .Ugda64 (Cyga64),  .Aiea64 (Jzga64[(1 *
8)+:8]),  .Giea64 (I4ga64[(1 * 8)+:8]));
Hoea64 #(.Shea64(Shea64)) S1ha64( .Ugda64 (Cyga64),  .Aiea64 (Jzga64[(2 *
8)+:8]),  .Giea64 (I4ga64[(2 * 8)+:8]));
Hoea64 #(.Shea64(Shea64)) E2ha64( .Ugda64 (Cyga64),  .Aiea64 (Jzga64[(3 *
8)+:8]),  .Giea64 (I4ga64[(3 * 8)+:8]));
end

if (Q7fa64) begin : Q2ha64 

A3ha64 N3ha64( .Z3ha64 (K5ga64[(0 * 8)+:8]),  .G4ha64 (Yzga64[(0 * 8)+:8]));
A3ha64 N4ha64( .Z3ha64 (K5ga64[(1 * 8)+:8]),  .G4ha64 (Yzga64[(1 * 8)+:8]));
A3ha64 Z4ha64( .Z3ha64 (K5ga64[(2 * 8)+:8]),  .G4ha64 (Yzga64[(2 * 8)+:8]));
A3ha64 L5ha64( .Z3ha64 (K5ga64[(3 * 8)+:8]),  .G4ha64 (Yzga64[(3 * 8)+:8]));
end
endmodule

/*
                    instances: 0
                        nodes: 8 (0)
                  node widths: 544 (0)
                   contassign:  5 (0)
                        ports: 5 (0)
*/

//`timescale 1 ns / 1 ns
module Hcga64(Lida64, Sida64, Ajda64, Ijda64, Pjda64);

input [127:0] Lida64;
input [127:0] Sida64;
output [127:0] Ajda64;
output [31:0] Ijda64;
input [31:0] Pjda64;

wire [31:0] A8ha64;
wire [31:0] L8ha64;
wire [31:0] T8ha64;

assign A8ha64 = (((Sida64[95-:32] ^ Sida64[63-:32]) ^ Sida64[31-:32]) ^ 
Lida64[31:0]);
assign Ijda64 = A8ha64;
assign L8ha64 = Pjda64;
assign T8ha64 = ((((L8ha64 ^ ((L8ha64 << 2) | (L8ha64 >> (32 - 2)))) ^ ((L8ha64 
<< 10) | (L8ha64 >> (32 - 10)))) ^ ((L8ha64 << 18) | (L8ha64 >> (32 - 18)))) ^ (
(L8ha64 << 24) | (L8ha64 >> (32 - 24))));
assign Ajda64 = {Sida64[95:0], (T8ha64 ^ Sida64[127:96])};
endmodule

/*
                    instances: 0
                        nodes: 13 (0)
                  node widths: 289 (0)
                   contassign:  8 (0)
                        ports: 7 (0)
*/

//`timescale 1 ns / 1 ns
module Vrga64(U8ea64, Ugda64, Htga64, Lida64, K9ea64, Ijda64, Pjda64);

parameter L8ea64 = 128;

input [1:0] U8ea64;
input Ugda64;
input [31:0] Htga64;
input [(L8ea64 - 1):0] Lida64;
output [(L8ea64 - 1):0] K9ea64;
output [31:0] Ijda64;
input [31:0] Pjda64;

wire [31:0] A8ha64;
wire [31:0] L8ha64;
wire [31:0] Bbha64;
wire [31:0] Ibha64;
wire [31:0] Sbha64;
wire [31:0] Fcha64;

assign Sbha64 = (Ugda64 ? Lida64[127-:32] : Lida64[31-:32]);
assign Fcha64 = (Ugda64 ? Lida64[31-:32] : Lida64[127-:32]);
assign A8ha64 = (((Lida64[95-:32] ^ Lida64[63-:32]) ^ Sbha64) ^ Htga64);
assign Ijda64 = A8ha64;
assign L8ha64 = Pjda64;
assign Ibha64 = ((L8ha64 ^ ((L8ha64 << 13) | (L8ha64 >> (32 - 13)))) ^ ((L8ha64 
<< 23) | (L8ha64 >> (32 - 23))));
assign Bbha64 = (Ibha64 ^ Fcha64);
assign K9ea64 = (Ugda64 ? {Bbha64, Lida64[127:32]} : {Lida64[95:0], Bbha64});
endmodule

/*
                    instances: 0
                        nodes: 2 (0)
                  node widths: 16 (0)
                   contassign:  1 (0)
                        ports: 2 (0)
*/

//`timescale 1 ns / 1 ns
module Vdha64(Z3ha64, G4ha64);

output [7:0] Z3ha64;
input [7:0] G4ha64;

assign Z3ha64 = Bfha64(G4ha64);

function [7:0] Bfha64;

input reg [7:0] Hfha64;

reg [7:0] Lfha64[0:255];
begin
{Lfha64[0], Lfha64[1], Lfha64[2], Lfha64[3], Lfha64[4], Lfha64[5], Lfha64[6],
Lfha64[7], Lfha64[8], Lfha64[9], Lfha64[10], Lfha64[11], Lfha64[12], Lfha64[13],
Lfha64[14], Lfha64[15], Lfha64[16], Lfha64[17], Lfha64[18], Lfha64[19],
Lfha64[20], Lfha64[21], Lfha64[22], Lfha64[23], Lfha64[24], Lfha64[25],
Lfha64[26], Lfha64[27], Lfha64[28], Lfha64[29], Lfha64[30], Lfha64[31],
Lfha64[32], Lfha64[33], Lfha64[34], Lfha64[35], Lfha64[36], Lfha64[37],
Lfha64[38], Lfha64[39], Lfha64[40], Lfha64[41], Lfha64[42], Lfha64[43],
Lfha64[44], Lfha64[45], Lfha64[46], Lfha64[47], Lfha64[48], Lfha64[49],
Lfha64[50], Lfha64[51], Lfha64[52], Lfha64[53], Lfha64[54], Lfha64[55],
Lfha64[56], Lfha64[57], Lfha64[58], Lfha64[59], Lfha64[60], Lfha64[61],
Lfha64[62], Lfha64[63], Lfha64[64], Lfha64[65], Lfha64[66], Lfha64[67],
Lfha64[68], Lfha64[69], Lfha64[70], Lfha64[71], Lfha64[72], Lfha64[73],
Lfha64[74], Lfha64[75], Lfha64[76], Lfha64[77], Lfha64[78], Lfha64[79],
Lfha64[80], Lfha64[81], Lfha64[82], Lfha64[83], Lfha64[84], Lfha64[85],
Lfha64[86], Lfha64[87], Lfha64[88], Lfha64[89], Lfha64[90], Lfha64[91],
Lfha64[92], Lfha64[93], Lfha64[94], Lfha64[95], Lfha64[96], Lfha64[97],
Lfha64[98], Lfha64[99], Lfha64[100], Lfha64[101], Lfha64[102], Lfha64[103],
Lfha64[104], Lfha64[105], Lfha64[106], Lfha64[107], Lfha64[108], Lfha64[109],
Lfha64[110], Lfha64[111], Lfha64[112], Lfha64[113], Lfha64[114], Lfha64[115],
Lfha64[116], Lfha64[117], Lfha64[118], Lfha64[119], Lfha64[120], Lfha64[121],
Lfha64[122], Lfha64[123], Lfha64[124], Lfha64[125], Lfha64[126], Lfha64[127],
Lfha64[128], Lfha64[129], Lfha64[130], Lfha64[131], Lfha64[132], Lfha64[133],
Lfha64[134], Lfha64[135], Lfha64[136], Lfha64[137], Lfha64[138], Lfha64[139],
Lfha64[140], Lfha64[141], Lfha64[142], Lfha64[143], Lfha64[144], Lfha64[145],
Lfha64[146], Lfha64[147], Lfha64[148], Lfha64[149], Lfha64[150], Lfha64[151],
Lfha64[152], Lfha64[153], Lfha64[154], Lfha64[155], Lfha64[156], Lfha64[157],
Lfha64[158], Lfha64[159], Lfha64[160], Lfha64[161], Lfha64[162], Lfha64[163],
Lfha64[164], Lfha64[165], Lfha64[166], Lfha64[167], Lfha64[168], Lfha64[169],
Lfha64[170], Lfha64[171], Lfha64[172], Lfha64[173], Lfha64[174], Lfha64[175],
Lfha64[176], Lfha64[177], Lfha64[178], Lfha64[179], Lfha64[180], Lfha64[181],
Lfha64[182], Lfha64[183], Lfha64[184], Lfha64[185], Lfha64[186], Lfha64[187],
Lfha64[188], Lfha64[189], Lfha64[190], Lfha64[191], Lfha64[192], Lfha64[193],
Lfha64[194], Lfha64[195], Lfha64[196], Lfha64[197], Lfha64[198], Lfha64[199],
Lfha64[200], Lfha64[201], Lfha64[202], Lfha64[203], Lfha64[204], Lfha64[205],
Lfha64[206], Lfha64[207], Lfha64[208], Lfha64[209], Lfha64[210], Lfha64[211],
Lfha64[212], Lfha64[213], Lfha64[214], Lfha64[215], Lfha64[216], Lfha64[217],
Lfha64[218], Lfha64[219], Lfha64[220], Lfha64[221], Lfha64[222], Lfha64[223],
Lfha64[224], Lfha64[225], Lfha64[226], Lfha64[227], Lfha64[228], Lfha64[229],
Lfha64[230], Lfha64[231], Lfha64[232], Lfha64[233], Lfha64[234], Lfha64[235],
Lfha64[236], Lfha64[237], Lfha64[238], Lfha64[239], Lfha64[240], Lfha64[241],
Lfha64[242], Lfha64[243], Lfha64[244], Lfha64[245], Lfha64[246], Lfha64[247],
Lfha64[248], Lfha64[249], Lfha64[250], Lfha64[251], Lfha64[252], Lfha64[253],
Lfha64[254], Lfha64[255]} = {8'hd6, 8'h90, 8'he9, 8'hfe, 8'hcc, 8'he1, 8'h3d,
8'hb7, 8'h16, 8'hb6, 8'h14, 8'hc2, 8'h28, 8'hfb, 8'h2c, 8'h05, 8'h2b, 8'h67,
8'h9a, 8'h76, 8'h2a, 8'hbe, 8'h04, 8'hc3, 8'haa, 8'h44, 8'h13, 8'h26, 8'h49,
8'h86, 8'h06, 8'h99, 8'h9c, 8'h42, 8'h50, 8'hf4, 8'h91, 8'hef, 8'h98, 8'h7a,
8'h33, 8'h54, 8'h0b, 8'h43, 8'hed, 8'hcf, 8'hac, 8'h62, 8'he4, 8'hb3, 8'h1c,
8'ha9, 8'hc9, 8'h08, 8'he8, 8'h95, 8'h80, 8'hdf, 8'h94, 8'hfa, 8'h75, 8'h8f,
8'h3f, 8'ha6, 8'h47, 8'h07, 8'ha7, 8'hfc, 8'hf3, 8'h73, 8'h17, 8'hba, 8'h83,
8'h59, 8'h3c, 8'h19, 8'he6, 8'h85, 8'h4f, 8'ha8, 8'h68, 8'h6b, 8'h81, 8'hb2,
8'h71, 8'h64, 8'hda, 8'h8b, 8'hf8, 8'heb, 8'h0f, 8'h4b, 8'h70, 8'h56, 8'h9d,
8'h35, 8'h1e, 8'h24, 8'h0e, 8'h5e, 8'h63, 8'h58, 8'hd1, 8'ha2, 8'h25, 8'h22,
8'h7c, 8'h3b, 8'b1, 8'h21, 8'h78, 8'h87, 8'hd4, 8'b0, 8'h46, 8'h57, 8'h9f,
8'hd3, 8'h27, 8'h52, 8'h4c, 8'h36, 8'h02, 8'he7, 8'ha0, 8'hc4, 8'hc8, 8'h9e,
8'hea, 8'hbf, 8'h8a, 8'hd2, 8'h40, 8'hc7, 8'h38, 8'hb5, 8'ha3, 8'hf7, 8'hf2,
8'hce, 8'hf9, 8'h61, 8'h15, 8'ha1, 8'he0, 8'hae, 8'h5d, 8'ha4, 8'h9b, 8'h34,
8'h1a, 8'h55, 8'had, 8'h93, 8'h32, 8'h30, 8'hf5, 8'h8c, 8'hb1, 8'he3, 8'h1d,
8'hf6, 8'he2, 8'h2e, 8'h82, 8'h66, 8'hca, 8'h60, 8'hc0, 8'h29, 8'h23, 8'hab,
8'h0d, 8'h53, 8'h4e, 8'h6f, 8'hd5, 8'hdb, 8'h37, 8'h45, 8'hde, 8'hfd, 8'h8e,
8'h2f, 8'h03, 8'hff, 8'h6a, 8'h72, 8'h6d, 8'h6c, 8'h5b, 8'h51, 8'h8d, 8'h1b,
8'haf, 8'h92, 8'hbb, 8'hdd, 8'hbc, 8'h7f, 8'h11, 8'hd9, 8'h5c, 8'h41, 8'h1f,
8'h10, 8'h5a, 8'hd8, 8'h0a, 8'hc1, 8'h31, 8'h88, 8'ha5, 8'hcd, 8'h7b, 8'hbd,
8'h2d, 8'h74, 8'hd0, 8'h12, 8'hb8, 8'he5, 8'hb4, 8'hb0, 8'h89, 8'h69, 8'h97,
8'h4a, 8'h0c, 8'h96, 8'h77, 8'h7e, 8'h65, 8'hb9, 8'hf1, 8'h09, 8'hc5, 8'h6e,
8'hc6, 8'h84, 8'h18, 8'hf0, 8'h7d, 8'hec, 8'h3a, 8'hdc, 8'h4d, 8'h20, 8'h79,
8'hee, 8'h5f, 8'h3e, 8'hd7, 8'hcb, 8'h39, 8'h48};
Bfha64 = Lfha64[Hfha64];
end
endfunction
endmodule

/*
                    instances: 0
                        nodes: 2 (0)
                  node widths: 16 (0)
                   contassign:  1 (0)
                        ports: 2 (0)
*/

//`timescale 1 ns / 1 ns
module A3ha64(Z3ha64, G4ha64);

output [7:0] Z3ha64;
input [7:0] G4ha64;

assign Z3ha64 = Bfha64(G4ha64);

function [7:0] Bfha64;

input reg [7:0] Hfha64;

reg [7:0] Lfha64[0:255];
begin
{Lfha64[0], Lfha64[1], Lfha64[2], Lfha64[3], Lfha64[4], Lfha64[5], Lfha64[6],
Lfha64[7], Lfha64[8], Lfha64[9], Lfha64[10], Lfha64[11], Lfha64[12], Lfha64[13],
Lfha64[14], Lfha64[15], Lfha64[16], Lfha64[17], Lfha64[18], Lfha64[19],
Lfha64[20], Lfha64[21], Lfha64[22], Lfha64[23], Lfha64[24], Lfha64[25],
Lfha64[26], Lfha64[27], Lfha64[28], Lfha64[29], Lfha64[30], Lfha64[31],
Lfha64[32], Lfha64[33], Lfha64[34], Lfha64[35], Lfha64[36], Lfha64[37],
Lfha64[38], Lfha64[39], Lfha64[40], Lfha64[41], Lfha64[42], Lfha64[43],
Lfha64[44], Lfha64[45], Lfha64[46], Lfha64[47], Lfha64[48], Lfha64[49],
Lfha64[50], Lfha64[51], Lfha64[52], Lfha64[53], Lfha64[54], Lfha64[55],
Lfha64[56], Lfha64[57], Lfha64[58], Lfha64[59], Lfha64[60], Lfha64[61],
Lfha64[62], Lfha64[63], Lfha64[64], Lfha64[65], Lfha64[66], Lfha64[67],
Lfha64[68], Lfha64[69], Lfha64[70], Lfha64[71], Lfha64[72], Lfha64[73],
Lfha64[74], Lfha64[75], Lfha64[76], Lfha64[77], Lfha64[78], Lfha64[79],
Lfha64[80], Lfha64[81], Lfha64[82], Lfha64[83], Lfha64[84], Lfha64[85],
Lfha64[86], Lfha64[87], Lfha64[88], Lfha64[89], Lfha64[90], Lfha64[91],
Lfha64[92], Lfha64[93], Lfha64[94], Lfha64[95], Lfha64[96], Lfha64[97],
Lfha64[98], Lfha64[99], Lfha64[100], Lfha64[101], Lfha64[102], Lfha64[103],
Lfha64[104], Lfha64[105], Lfha64[106], Lfha64[107], Lfha64[108], Lfha64[109],
Lfha64[110], Lfha64[111], Lfha64[112], Lfha64[113], Lfha64[114], Lfha64[115],
Lfha64[116], Lfha64[117], Lfha64[118], Lfha64[119], Lfha64[120], Lfha64[121],
Lfha64[122], Lfha64[123], Lfha64[124], Lfha64[125], Lfha64[126], Lfha64[127],
Lfha64[128], Lfha64[129], Lfha64[130], Lfha64[131], Lfha64[132], Lfha64[133],
Lfha64[134], Lfha64[135], Lfha64[136], Lfha64[137], Lfha64[138], Lfha64[139],
Lfha64[140], Lfha64[141], Lfha64[142], Lfha64[143], Lfha64[144], Lfha64[145],
Lfha64[146], Lfha64[147], Lfha64[148], Lfha64[149], Lfha64[150], Lfha64[151],
Lfha64[152], Lfha64[153], Lfha64[154], Lfha64[155], Lfha64[156], Lfha64[157],
Lfha64[158], Lfha64[159], Lfha64[160], Lfha64[161], Lfha64[162], Lfha64[163],
Lfha64[164], Lfha64[165], Lfha64[166], Lfha64[167], Lfha64[168], Lfha64[169],
Lfha64[170], Lfha64[171], Lfha64[172], Lfha64[173], Lfha64[174], Lfha64[175],
Lfha64[176], Lfha64[177], Lfha64[178], Lfha64[179], Lfha64[180], Lfha64[181],
Lfha64[182], Lfha64[183], Lfha64[184], Lfha64[185], Lfha64[186], Lfha64[187],
Lfha64[188], Lfha64[189], Lfha64[190], Lfha64[191], Lfha64[192], Lfha64[193],
Lfha64[194], Lfha64[195], Lfha64[196], Lfha64[197], Lfha64[198], Lfha64[199],
Lfha64[200], Lfha64[201], Lfha64[202], Lfha64[203], Lfha64[204], Lfha64[205],
Lfha64[206], Lfha64[207], Lfha64[208], Lfha64[209], Lfha64[210], Lfha64[211],
Lfha64[212], Lfha64[213], Lfha64[214], Lfha64[215], Lfha64[216], Lfha64[217],
Lfha64[218], Lfha64[219], Lfha64[220], Lfha64[221], Lfha64[222], Lfha64[223],
Lfha64[224], Lfha64[225], Lfha64[226], Lfha64[227], Lfha64[228], Lfha64[229],
Lfha64[230], Lfha64[231], Lfha64[232], Lfha64[233], Lfha64[234], Lfha64[235],
Lfha64[236], Lfha64[237], Lfha64[238], Lfha64[239], Lfha64[240], Lfha64[241],
Lfha64[242], Lfha64[243], Lfha64[244], Lfha64[245], Lfha64[246], Lfha64[247],
Lfha64[248], Lfha64[249], Lfha64[250], Lfha64[251], Lfha64[252], Lfha64[253],
Lfha64[254], Lfha64[255]} = {8'hd6, 8'h90, 8'he9, 8'hfe, 8'hcc, 8'he1, 8'h3d,
8'hb7, 8'h16, 8'hb6, 8'h14, 8'hc2, 8'h28, 8'hfb, 8'h2c, 8'h05, 8'h2b, 8'h67,
8'h9a, 8'h76, 8'h2a, 8'hbe, 8'h04, 8'hc3, 8'haa, 8'h44, 8'h13, 8'h26, 8'h49,
8'h86, 8'h06, 8'h99, 8'h9c, 8'h42, 8'h50, 8'hf4, 8'h91, 8'hef, 8'h98, 8'h7a,
8'h33, 8'h54, 8'h0b, 8'h43, 8'hed, 8'hcf, 8'hac, 8'h62, 8'he4, 8'hb3, 8'h1c,
8'ha9, 8'hc9, 8'h08, 8'he8, 8'h95, 8'h80, 8'hdf, 8'h94, 8'hfa, 8'h75, 8'h8f,
8'h3f, 8'ha6, 8'h47, 8'h07, 8'ha7, 8'hfc, 8'hf3, 8'h73, 8'h17, 8'hba, 8'h83,
8'h59, 8'h3c, 8'h19, 8'he6, 8'h85, 8'h4f, 8'ha8, 8'h68, 8'h6b, 8'h81, 8'hb2,
8'h71, 8'h64, 8'hda, 8'h8b, 8'hf8, 8'heb, 8'h0f, 8'h4b, 8'h70, 8'h56, 8'h9d,
8'h35, 8'h1e, 8'h24, 8'h0e, 8'h5e, 8'h63, 8'h58, 8'hd1, 8'ha2, 8'h25, 8'h22,
8'h7c, 8'h3b, 8'b1, 8'h21, 8'h78, 8'h87, 8'hd4, 8'b0, 8'h46, 8'h57, 8'h9f,
8'hd3, 8'h27, 8'h52, 8'h4c, 8'h36, 8'h02, 8'he7, 8'ha0, 8'hc4, 8'hc8, 8'h9e,
8'hea, 8'hbf, 8'h8a, 8'hd2, 8'h40, 8'hc7, 8'h38, 8'hb5, 8'ha3, 8'hf7, 8'hf2,
8'hce, 8'hf9, 8'h61, 8'h15, 8'ha1, 8'he0, 8'hae, 8'h5d, 8'ha4, 8'h9b, 8'h34,
8'h1a, 8'h55, 8'had, 8'h93, 8'h32, 8'h30, 8'hf5, 8'h8c, 8'hb1, 8'he3, 8'h1d,
8'hf6, 8'he2, 8'h2e, 8'h82, 8'h66, 8'hca, 8'h60, 8'hc0, 8'h29, 8'h23, 8'hab,
8'h0d, 8'h53, 8'h4e, 8'h6f, 8'hd5, 8'hdb, 8'h37, 8'h45, 8'hde, 8'hfd, 8'h8e,
8'h2f, 8'h03, 8'hff, 8'h6a, 8'h72, 8'h6d, 8'h6c, 8'h5b, 8'h51, 8'h8d, 8'h1b,
8'haf, 8'h92, 8'hbb, 8'hdd, 8'hbc, 8'h7f, 8'h11, 8'hd9, 8'h5c, 8'h41, 8'h1f,
8'h10, 8'h5a, 8'hd8, 8'h0a, 8'hc1, 8'h31, 8'h88, 8'ha5, 8'hcd, 8'h7b, 8'hbd,
8'h2d, 8'h74, 8'hd0, 8'h12, 8'hb8, 8'he5, 8'hb4, 8'hb0, 8'h89, 8'h69, 8'h97,
8'h4a, 8'h0c, 8'h96, 8'h77, 8'h7e, 8'h65, 8'hb9, 8'hf1, 8'h09, 8'hc5, 8'h6e,
8'hc6, 8'h84, 8'h18, 8'hf0, 8'h7d, 8'hec, 8'h3a, 8'hdc, 8'h4d, 8'h20, 8'h79,
8'hee, 8'h5f, 8'h3e, 8'hd7, 8'hcb, 8'h39, 8'h48};
Bfha64 = Lfha64[Hfha64];
end
endfunction
endmodule

/*
                    instances: 0
                        nodes: 33 (0)
                  node widths: 642 (0)
                        ports: 17 (0)
                        ports: 4 (0)
                 portconnects: 64 (0)
*/

//`timescale 1 ns / 1 ns
module osr_bc_lp_alg_top(clk, rst_n, i_cipher_alg, i_key_inv, i_key_size,
i_keyexp_go, o_keyexp_busy, o_keyexp_rdy, i_state_inv, i_state_vld, o_state_busy
, o_state_vld, o_state, i_din, i_key_wren, i_state_wren, o_key);

parameter A7fa64 = 1;
parameter Q7fa64 = 1;
parameter Shea64 = "ALG";

input clk;
input rst_n;
input [3:0] i_cipher_alg;
input i_key_inv;
input [1:0] i_key_size;
input i_keyexp_go;
output o_keyexp_busy;
output o_keyexp_rdy;
input i_state_inv;
input i_state_vld;
output o_state_busy;
output o_state_vld;
output [127:0] o_state;
input [31:0] i_din;
input [3:0] i_key_wren;
input [3:0] i_state_wren;
output [127:0] o_key;

wire Tvfa64;
wire Dwfa64;
wire Wbea64;
wire Owfa64;
wire Bxfa64;
wire Nxfa64;
wire [1:0] Ljha64;
wire Zxfa64;
wire Lyfa64;
wire [127:0] Ujha64;
wire [31:0] Bkha64;
wire [31:0] Skha64;
wire [31:0] Jlha64;
wire [31:0] Ylha64;
wire [31:0] Mmha64;
wire [31:0] Anha64;

O5fa64 #(.A7fa64(A7fa64), .Q7fa64(Q7fa64)) Mnha64( .G8fa64 (clk),  .K8fa64
(rst_n),  .Q8fa64 (i_cipher_alg),  .D9fa64 (i_key_inv),  .N9fa64 (i_keyexp_go), 
.Z9fa64 (o_keyexp_busy),  .Nafa64 (o_keyexp_rdy),  .Abfa64 (i_state_inv), 
.Mbfa64 (i_state_vld),  .Ybfa64 (o_state_busy),  .Lcfa64 (o_state_vld),  .Xcfa64
(Tvfa64),  .Hdfa64 (Dwfa64),  .Sdfa64 (Wbea64),  .Defa64 (Owfa64),  .Qefa64
(Bxfa64),  .Cffa64 (Nxfa64),  .Offa64 (Ljha64),  .Xffa64 (Zxfa64),  .Jgfa64
(Lyfa64));
Ffga64 #(.A7fa64(A7fa64), .Q7fa64(Q7fa64)) Doha64( .G8fa64 (clk),  .K8fa64
(rst_n),  .Q8fa64 (i_cipher_alg),  .Rgga64 (i_din),  .Xgga64 (i_key_wren), 
.Ihga64 (o_key),  .Ohga64 (o_keyexp_busy),  .Ciga64 (Tvfa64),  .Miga64 (Dwfa64),
.Xiga64 (Wbea64),  .Ijga64 (Owfa64),  .K9ea64 (Ujha64),  .I4ga64 (Bkha64), 
.W4ga64 (Jlha64),  .K5ga64 (Ylha64),  .V5ga64 (Anha64));
F0ga64 #(.A7fa64(A7fa64), .Q7fa64(Q7fa64)) Uoha64( .G8fa64 (clk),  .K8fa64
(rst_n),  .Q8fa64 (i_cipher_alg),  .N1ga64 (i_state_wren),  .Sida64 (i_din), 
.Ajda64 (o_state),  .A2ga64 (Bxfa64),  .Abfa64 (Nxfa64),  .M2ga64
(o_state_busy),  .Jhda64 (Dwfa64),  .Z2ga64 (o_state_vld),  .Ahda64 (Ljha64), 
.K3ga64 (Zxfa64),  .W3ga64 (Lyfa64),  .Lida64 (Ujha64),  .I4ga64 (Skha64), 
.W4ga64 (Jlha64),  .K5ga64 (Mmha64),  .V5ga64 (Anha64));
Suga64 #(.A7fa64(A7fa64), .Q7fa64(Q7fa64), .Shea64(Shea64)) Jpha64( .D9fa64
(Wbea64),  .Abfa64 (Nxfa64),  .Miga64 (Dwfa64),  .Wvga64 (Bkha64),  .I4ga64
(Jlha64),  .Mwga64 (Ylha64),  .K5ga64 (Anha64),  .Zwga64 (Skha64),  .Pxga64
(Mmha64));
endmodule

/*     Design Summary
                      modules: 17
                         udps: 0
                mod flatinsts: 0
                udp flatinsts: 0
                        nodes: 285 (0)
                  node widths: 8383 (0)
                      process: 5 (0)
                        gates: 0 (0)
                  contassigns: 135 (0)
                        ports: 128 (0)
                     modinsts: 8 (0)
                     udpinsts: 0 (0)
                 portconnects: 72 (0)
*/
// END: VCS tokens
