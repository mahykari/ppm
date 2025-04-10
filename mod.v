module Spec
  #(parameter WORDLEN=`WORDLEN, parameter MODULUS=`MODULUS)
    (monitor, system, out);
  input [WORDLEN-1:0] monitor;
  input [1:0] system;
  output reg [WORDLEN:0] out;

  reg [WORDLEN-1:0] monnxt;
  reg fault;

  always @(*) begin
    monnxt = monitor;
    fault = 1;
    if (monitor == 0)
      fault = 0;
    if (system[0])
      monnxt = (monitor + 1 == MODULUS) ? 0 : monitor + 1;

    out = {fault, monnxt};
 end
endmodule
