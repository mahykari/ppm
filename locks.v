module Spec
  # (parameter NLOCKS = `NLOCKS)
  (monitor, system, out);
  // Each lock is represented by 1 bit.
  // 0 = unlocked, 1 = locked.
  input [NLOCKS-1:0] monitor;
  // Each system command is represented by 2 bits.
  // First bit  to unlock (0) or lock (1),
  // second bit to skip.
  input [2*NLOCKS-1:0] system;
  // Output is NLOCKS+1 bits;
  // an extra bit to indicate fault.
  output reg [NLOCKS:0] out;
  reg [NLOCKS-1:0] monnxt;
  reg fault;

  reg badseq;
  reg [NLOCKS-1:0] updated_locks;
  integer i;

  wire [1:0] commands [NLOCKS-1:0];
  genvar g;
  generate
    for (g = 0; g < NLOCKS; g = g + 1) begin : split_cmds
      assign commands[g] = system[2*g + 1 : 2*g];
    end
  endgenerate


  reg skip, cmd;
  always @(*) begin
    // Default values
    monnxt = monitor;
    fault = 0;
    {badseq, updated_locks} = 0;

    for (i = 0; i < NLOCKS; i = i + 1) begin
      {skip, cmd} = commands[i];
      // A bad sequence has occurred
      // if a lock is not skipped, and is double-(un)locked.
      if (skip)
        badseq = badseq;
      else
        badseq = badseq | (cmd == monitor[i]);
    end

    for (i = 0; i < NLOCKS; i = i + 1) begin
      {skip, cmd} = commands[i];
      // Update the locks.
      if (skip)
        updated_locks[i] = monitor[i];
      else
        updated_locks[i] = cmd;
    end
    // Check if any command is violating.
    // If not, update the locks normally.
    if (badseq)
      fault = 1;
    else
      monnxt = updated_locks;
    out = {fault, monnxt};
  end
endmodule
