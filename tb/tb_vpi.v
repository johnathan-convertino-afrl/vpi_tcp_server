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
module tb_vpi #(
  parameter IN_FILE_NAME = "in.bin",
  parameter OUT_FILE_NAME = "out.bin",
  parameter BIT_WIDTH = 32);
  
  localparam CLK_PERIOD = 500;
  
  reg tb_data_clk = 0;

  reg [BIT_WIDTH-1:0] test_vector1;
  
  integer num_read  = 0;
  integer num_wrote = 0;
  
  initial begin
    $dumpfile ("tb_vpi.fst");
    $dumpvars (0, tb_vpi);
  end

  //clock
  always
  begin
    tb_data_clk <= ~tb_data_clk;
    
    #(CLK_PERIOD/4);
  end
  
  //process data
  always @(posedge tb_data_clk)
  begin
    num_read = $read_binary_file(IN_FILE_NAME, test_vector1);
    
    if(num_read != 0)
    begin
      num_wrote = $write_binary_file(OUT_FILE_NAME, test_vector1);
      
      if(num_read < 0)
      begin
        $finish;
      end
    end
  end
endmodule
