module Spec_tb;
  // Inputs
  reg [9:0] monitor;
  reg [8:0] system;

  // Outputs
  wire [10:0] out;

  // Instantiate the Unit Under Test (UUT)
  Spec uut (
    .monitor(monitor),
    .system(system),
    .out(out)
  );

  // Extract fault and monnxt from the output
  wire fault = out[10];
  wire [9:0] monnxt = out[9:0];

  // Helper variables for expected outputs
  reg expected_pred;
  reg expected_fault;

  // Calculate expected values for comparison
  always @(*) begin
    expected_pred =
      ~system[8] |
      (~system[6] & ~system[7]) |
      (~system[4] & ~system[5] & ~system[7]) |
      (~system[3] & ~system[5] & ~system[7]) |
      (~system[2] & ~system[5] & ~system[7]) |
      (~system[1] & ~system[5] & ~system[7]) |
      (~system[0] & ~system[5] & ~system[7]);

    expected_fault = (10'd100 <= monitor && monitor <= 10'd700 && ~expected_pred) ? 1'b1 : 1'b0;
  end

  initial begin
    // Initialize inputs
    monitor = 0;
    system = 0;

    // Wait for stabilization
    #10;

    // Test cases

    // Case 1: Monitor outside range (below), should not trigger fault regardless of system
    monitor = 10'd50;
    system = 9'd511;  // All bits set, would fail predicate
    #10;
    if (fault !== 0) begin
      $display("Test Case 1 Failed: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Case 1 Passed: fault = %d, expected = 0", fault);
    end

    // Case 2: Monitor at lower bound, with valid system value
    monitor = 10'd100;
    system = 9'd0;  // All bits clear, should satisfy predicate
    #10;
    if (fault !== 0) begin
      $display("Test Case 2 Failed: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Case 2 Passed: fault = %d, expected = 0", fault);
    end

    // Case 3: Monitor at lower bound, with invalid system value
    monitor = 10'd100;
    system = 9'd511;  // All bits set, should fail predicate
    #10;
    if (fault !== 1) begin
      $display("Test Case 3 Failed: fault = %d, expected = 1", fault);
      $finish;
    end else begin
      $display("Test Case 3 Passed: fault = %d, expected = 1", fault);
    end

    // Case 4: Monitor in range, with valid system (condition 1: ~p9)
    monitor = 10'd400;
    system = 9'd255;  // p9 bit clear, should pass predicate
    #10;
    if (fault !== 0) begin
      $display("Test Case 4 Failed: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Case 4 Passed: fault = %d, expected = 0", fault);
    end

    // Case 5: Monitor in range, with valid system (condition 2: ~p7 & ~p8)
    monitor = 10'd500;
    system = 9'd383;  // p7 and p8 bits clear, should pass predicate
    #10;
    if (fault !== 1) begin
      $display("Test Case 5 Failed: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Case 5 Passed: fault = %d, expected = 0", fault);
    end

    // Case 6: Monitor at upper bound, with invalid system value
    monitor = 10'd700;
    system = 9'd511;  // All bits set, should fail predicate
    #10;
    if (fault !== 1) begin
      $display("Test Case 6 Failed: fault = %d, expected = 1", fault);
      $finish;
    end else begin
      $display("Test Case 6 Passed: fault = %d, expected = 1", fault);
    end

    // Case 7: Monitor outside upper bound, should not trigger fault regardless of system
    monitor = 10'd800;
    system = 9'd511;  // All bits set, would fail predicate
    #10;
    if (fault !== 0) begin
      $display("Test Case 7 Failed: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Case 7 Passed: fault = %d, expected = 0", fault);
    end

    // Test specific conditions in the predicate
    monitor = 10'd300;  // In range

    // Test condition 3: ~p5 & ~p6 & ~p8
    system = 9'd415;  // Only bits 5, 6, and 8 clear
    #10;
    if (fault !== 1) begin
      $display("Test Failed for condition 3: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Passed for condition 3: fault = %d, expected = 0", fault);
    end

    // Test condition 4: ~p4 & ~p6 & ~p8
    system = 9'd431;  // Only bits 4, 6, and 8 clear
    #10;
    if (fault !== 1) begin
      $display("Test Failed for condition 4: fault = %d, expected = 0", fault);
      $finish;
    end else begin
      $display("Test Passed for condition 4: fault = %d, expected = 0", fault);
    end

    // Test condition 5, 6, 7 with various system values
    // Testing system value that should fail all conditions
    system = 9'd511;  // All bits set
    #10;
    if (fault !== 1) begin
      $display("Test Failed for failing system: fault = %d, expected = 1", fault);
      $finish;
    end else begin
      $display("Test Passed for failing system: fault = %d, expected = 1", fault);
    end

    // Check monnxt calculation
    if (monnxt !== monitor + 1) begin
      $display("Error: monnxt = %d, expected = %d", monnxt, monitor + 1);
      $finish;
    end else begin
      $display("monnxt calculation is correct: %d", monnxt);
    end

    $display("All tests passed successfully!");
    $finish;
  end

endmodule
