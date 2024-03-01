//******************************************************************************
/// @file    tb_vpi.v
/// @author  JAY CONVERTINO
/// @date    2024.02.26
/// @brief   Test vpi tcp server in echo setup
///
/// @LICENSE MIT
///  Copyright 2024 Jay Convertino
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
module tb_vpi ();
  
  localparam CLK_PERIOD = 100;
  
  reg tb_data_clk = 0;
  
  integer return_value0 = 0;
  integer return_value1 = 0;

  reg [31:0] dump_data;
  
  initial begin
    $dumpfile ("tb_vpi.fst");
    $dumpvars (0, tb_vpi);

    $setup_tcp_server("127.0.0.1", 4444);
    $setup_tcp_server("127.0.0.1", 5555);
  end

  //clock
  always
  begin
    tb_data_clk <= ~tb_data_clk;
    
    #(CLK_PERIOD/2);
  end
  
  //process data
  always @(posedge tb_data_clk)
  begin
      return_value0 = $recv_tcp_server(4444, dump_data);

      if(return_value0 > 0)
      begin
        $display(return_value0);
        return_value0 = $send_tcp_server(4444, dump_data);
      end

      return_value1 = $recv_tcp_server(5555, dump_data);

      if(return_value1 > 0)
      begin
        $display(return_value1);
        return_value1 = $send_tcp_server(5555, dump_data);
      end
  end

endmodule
