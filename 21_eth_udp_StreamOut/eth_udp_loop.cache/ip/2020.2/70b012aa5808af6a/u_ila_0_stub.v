// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2020.2 (win64) Build 3064766 Wed Nov 18 09:12:45 MST 2020
// Date        : Fri Aug  4 11:25:33 2023
// Host        : WorkPC running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix -prefix
//               decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix_ u_ila_0_stub.v
// Design      : u_ila_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-2
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "ila,Vivado 2020.2" *)
module decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix(clk, probe0, probe1, probe2, probe3, probe4, probe5, 
  probe6, probe7, probe8, probe9, probe10)
/* synthesis syn_black_box black_box_pad_pin="clk,probe0[15:0],probe1[7:0],probe2[2:0],probe3[7:0],probe4[15:0],probe5[2:0],probe6[63:0],probe7[63:0],probe8[63:0],probe9[63:0],probe10[63:0]" */;
  input clk;
  input [15:0]probe0;
  input [7:0]probe1;
  input [2:0]probe2;
  input [7:0]probe3;
  input [15:0]probe4;
  input [2:0]probe5;
  input [63:0]probe6;
  input [63:0]probe7;
  input [63:0]probe8;
  input [63:0]probe9;
  input [63:0]probe10;
endmodule
