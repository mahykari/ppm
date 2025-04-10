`timescale 1ns/1ps

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

  initial begin
    // Initialize inputs
    monitor = 0;
    system = 0;

    // Create waveform file
    $dumpfile("psi4.vcd");
    $dumpvars(0, Spec_tb);

    // Monitor the outputs
    $monitor("Time=%t | monitor=%d | system=%d | out=%b | fault=%b",
              $time, monitor, system, out, out[10]);

    // Test cases

    // Test case 1: Normal operation (outside monitor range)
    #10;
    monitor = 10'd500;
    system = 9'b000000000;
    #10;

    // Test case 2: Within monitor range, pred is true (no fault)
    monitor = 10'd650;
    system = 9'b000000000; // pred = 1 for this value
    #10;

    // Test case 3: Within monitor range, pred is false (fault)
    monitor = 10'd650;
    system = 9'b100000000; // pred = 0 for this value
    #10;

    // Test case 4: Edge of monitor range
    monitor = 10'd600;
    system = 9'b100000000;
    #10;

    monitor = 10'd700;
    system = 9'b100000000;
    #10;

    // Test more system values
    monitor = 10'd650;
    system = 9'b010000000; // Test with p[7] = 1
    #10;

    monitor = 10'd650;
    system = 9'b001000000; // Test with p[6] = 1
    #10;

    // Finish simulation
    #10;
    $finish;
  end
endmodule
