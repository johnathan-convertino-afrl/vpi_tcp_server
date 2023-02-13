//******************************************************************************
/// @file    tb_vpi.v
/// @author  JAY CONVERTINO
/// @date    2023.01.01
/// @brief   Test vpi binary file read/write
///
/// @LICENSE MIT
///  Copyright 2023 Jay Convertino
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to 
///  deal in the Software without restriction, including without limitation the
///  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
///  sell copies of the Software, and to permit persons to whom the Software is 
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in 
///  all copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
///  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
///  IN THE SOFTWARE.
//******************************************************************************


`timescale 1 ns/10 ps

// test normal operation
module tb_vpi;
  reg tb_data_clk = 0;

  reg [39:0] test_vector1;
  reg [23:0] test_vector2;
  
  integer num_read  = 0;
  integer num_wrote = 0;
  
  localparam CLK_PERIOD = 500;
  
  initial begin
    $display("START SIMULATION");
    $dumpfile ("tb_vpi.fst");
    $dumpvars (0, tb_vpi);
  end

  //clock
  always
  begin
    tb_data_clk <= ~tb_data_clk;
    
    #(CLK_PERIOD/4);
  end
  
  //product data
  always @(posedge tb_data_clk)
  begin
    num_read = $read_binary_file("count_test.bin", test_vector1);
    
    if(num_read != 0)
    begin
      num_wrote = $write_binary_file("count_test_out.bin", test_vector1);
      
      $display("READ: %h %d  WRITE: %d", test_vector1, num_read, num_wrote);
      
      if(num_read < 0)
      begin
        $display("END SIMULATION");
        $finish;
      end
    end
    
//     num_read = $read_binary_file("binary_file_io.o", test_vector2);
//     
//     if(num_read != 0)
//     begin
//       num_wrote = $write_binary_file("binary_file_io2.o", test_vector2);
//       
//       $display("READ: %h %d  WRITE: %d", test_vector2, num_read, num_wrote);
//     end
  end
endmodule
