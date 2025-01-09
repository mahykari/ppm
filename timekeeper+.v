module Spec
  #(parameter N_EX=`N_EX,
    parameter N_IN=`N_IN,
    parameter WORDLEN=`WORDLEN
  ) (monitor, system, out);
  input [2*WORDLEN-1:0] monitor;
  // Internal doors are  first (more significant).
  // External doors are second (less significant).
  input [4*WORDLEN*(N_EX+N_IN)-1:0] system;
  output reg [2*WORDLEN:0] out;

  reg [2*WORDLEN-1:0] monnxt;
  reg fault;
  reg [WORDLEN-1:0] cntA, cntB;
  reg [WORDLEN-1:0] enteredA, exitedA, enteredB, exitedB;
  integer i;

  // First, let's split system into door-sized chunks.
  // Remember that external doors are less significant.
  wire [4*WORDLEN-1:0] door_data [N_EX-1:0];
  genvar g;
  generate
    for (g = 0; g < N_EX; g = g + 1) begin : split_doors
      assign door_data[g] = system[4*WORDLEN*(g+1)-1 : 4*WORDLEN*g];
    end
  endgenerate

  always @(*) begin
    {cntA, cntB} = monitor;
    monnxt = monitor;
    fault = 0;

    // For each door
    for (i = 0; i < N_EX; i = i + 1) begin
      {enteredA, exitedA, enteredB, exitedB} = door_data[i];
      cntA = cntA + enteredA - exitedA;
      cntB = cntB + enteredB - exitedB;
    end

    if (cntA < cntB)
      fault = 1;
    else
      monnxt = {cntA, cntB};

    out = {fault, monnxt};
 end
endmodule
