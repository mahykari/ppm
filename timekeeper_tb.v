module testbench;
 parameter WIDTH = 10;
 parameter NDOORS = 3;

 // Inputs
 reg [2*WIDTH-1:0] monitor;
 reg [4*WIDTH*NDOORS-1:0] system;

 // Output
 wire [2*WIDTH:0] out;

 // Instantiate the Unit Under Test (UUT)
 Spec #(
   .NDOORS(NDOORS),
   .WIDTH(WIDTH)
 ) uut (
   .monitor(monitor),
   .system(system),
   .out(out)
 );

 initial begin
   // Add waveform dump
   $dumpfile("test.vcd");
   $dumpvars(0, testbench);

   // Initialize inputs
   monitor = 0;
   system = 0;
   #10;

   // Test Case 1: Single door entry
   monitor = {10'd5, 10'd3};  // cntA=5, cntB=3
   system = {
     // Door 2: no change
     10'd0, 10'd0, 10'd0, 10'd0,
     // Door 1: no change
     10'd0, 10'd0, 10'd0, 10'd0,
     // Door 0: A enters
     10'd1, 10'd0, 10'd0, 10'd0
   };
   #10;

   // Test Case 2: Multiple doors
   monitor = {10'd6, 10'd3};  // cntA=6, cntB=3
   system = {
     // Door 2: B enters
     10'd0, 10'd0, 10'd1, 10'd0,
     // Door 1: A exits
     10'd0, 10'd1, 10'd0, 10'd0,
     // Door 0: no change
     10'd0, 10'd0, 10'd0, 10'd0
   };
   #10;

   // Test Case 3: Should trigger fault (B > A)
   monitor = {10'd5, 10'd4};  // cntA=5, cntB=4
   system = {
     // Door 2: no change
     10'd0, 10'd0, 10'd0, 10'd0,
     // Door 1: B enters twice
     10'd0, 10'd0, 10'd2, 10'd0,
     // Door 0: A exits
     10'd0, 10'd1, 10'd0, 10'd0
   };
   #10;

   $finish;
 end
endmodule
