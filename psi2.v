module Spec
    (monitor, system, out);
  input [9:0] monitor;
  input [8:0] system;
  output reg [10:0] out;

  reg [8:0] p;
  reg [9:0] monnxt;
  reg fault;
  reg pred;

  always @(*) begin
    p = system;

    pred =
      ~p[8] |
      (~p[6] & ~p[7]) |
      (~p[4] & ~p[5] & ~p[7]) |
      (~p[3] & ~p[5] & ~p[7]) |
      (~p[2] & ~p[5] & ~p[7]) |
      (~p[1] & ~p[5] & ~p[7]) |
      (~p[0] & ~p[5] & ~p[7]);
    monnxt = monitor + 1;

    if (10'd100 <= monitor && monitor <= 10'd700 && ~pred)
      fault = 1;
    else
      fault = 0;

    out = {fault, monnxt};
 end
endmodule
