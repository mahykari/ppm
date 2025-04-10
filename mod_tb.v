`timescale 1ns/1ps

module Spec_tb;
  // Parameters
  localparam WORDLEN = 8;
  localparam MODULUS = 10;

  // Inputs
  reg [WORDLEN-1:0] monitor;
  reg [1:0] system;

  // Outputs
  wire [WORDLEN:0] out;

  // Extract individual signals from output
  wire fault = out[WORDLEN];
  wire [WORDLEN-1:0] monnxt = out[WORDLEN-1:0];

  // Instantiate the Unit Under Test (UUT)
  Spec #(
    .WORDLEN(WORDLEN),
    .MODULUS(MODULUS)
  ) uut (
    .monitor(monitor),
    .system(system),
    .out(out)
  );

  initial begin
    // Initialize inputs
    monitor = 0;
    system = 0;

    // Create waveform file
    $dumpfile("spec_modulo_waveform.vcd");
    $dumpvars(0, Spec_tb);

    // Display parameter values
    $display("WORDLEN = %d, MODULUS = %d", WORDLEN, MODULUS);

    // Monitor the outputs
    $monitor("Time=%t | monitor=%d | system=%d | monnxt=%d | fault=%b",
              $time, monitor, system, monnxt, fault);

    // Test cases

    // Test case 1: monitor = 0 (fault should be 0)
    #10;
    monitor = 0;
    system = 0;
    #10;

    // Test case 2: monitor = 0, system = 1 (should increment to 1)
    system = 1;
    #10;

    // Test case 3: monitor = 1, system = 0 (no increment)
    monitor = 1;
    system = 0;
    #10;

    // Test case 4: monitor = 1, system = 1 (increment to 2)
    system = 1;
    #10;

    // Test case 5: monitor near MODULUS-1
    monitor = MODULUS - 2;
    system = 1;
    #10;

    // Test case 6: monitor = MODULUS-1, system = 1 (should wrap to 0)
    monitor = MODULUS - 1;
    system = 1;
    #10;

    // Test case 7: non-zero monitor with fault = 1
    monitor = 5;
    system = 0;
    #10;

    // Test additional increments to verify counting
    monitor = 3;
    system = 1;
    #10;

    monitor = 4;
    system = 1;
    #10;

    monitor = 5;
    system = 1;
    #10;

    // Finish simulation
    #10;
    $finish;
  end
endmodule
