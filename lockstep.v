module Spec #(parameter WIDTH=10) (monitor, system, out);
  input [WIDTH-1:0] monitor;
  input [WIDTH-1:0] system;
  output reg [1:0] out;

  reg ok;
  reg fault;

  always @(*) begin
    ok = 0;
    fault = 0;
    if (monitor != system) begin
      fault = 1;
    end else begin
      ok = 1;
    end
    out = {fault, ok};
  end
endmodule
