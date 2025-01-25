module testbench;
 parameter NLOCKS = 3;  // Test with 3 locks

 // Inputs
 reg [NLOCKS-1:0] monitor;
 reg [2*NLOCKS-1:0] system;
 
 // Output
 wire [NLOCKS:0] out;
 
 // Instantiate the Unit Under Test (UUT)
 Spec #(
   .NLOCKS(NLOCKS)
 ) uut (
   .monitor(monitor),
   .system(system),
   .out(out)
 );
 
 initial begin
   $dumpfile("test.vcd");
   $dumpvars(0, testbench);
   
   // Test Case 1: Initial state - all unlocked
   monitor = 3'b000;  // all unlocked
   system = 6'b000000;  // skip all
   #10;
   
   // Test Case 2: Lock first lock
   monitor = 3'b000;
   system = 6'b100000;  // lock[0], skip others
   #10;
   
   // Test Case 3: Invalid - try to lock already locked
   monitor = 3'b001;
   system = 6'b100000;  // try to lock[0] again
   #10;

   // Test Case 4: Multiple operations
   monitor = 3'b001;
   system = 6'b101100;  // lock[1], unlock[0]
   #10;
   
   $finish;
 end
endmodule

