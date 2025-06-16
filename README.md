# VPI TCP server
### TCP server functions for VPI

![image](docs/manual/img/AFRL.png)

---

   author: Jay Convertino   
   
   date: 2024.02.24
   
   details: TCP server functions for VPI. This is threaded and does all recv/send IO outside of the simulation function call.
   
   license: MIT   
   
---

### Version
#### Current
  - V1.0.0 - initial release

#### Previous
  - none
  
### DOCUMENTATION
  For detailed usage information, please navigate to one of the following sources. They are the same, just in a different format.

  - [vpi_tcp_server.pdf](docs/manual/vpi_tcp_server.pdf)
  - [github page](https://johnathan-convertino-afrl.github.io/vpi_tcp_server/)

### DEPENDENCIES
#### Build
  - AFRL:utility:sim_helper
  - AFRL:utility:generators:1.0.0

### IP USAGE
#### INSTRUCTIONS

This library provides three functions.
* setup_tcp_server(ADDRESS, PORT)
  * RETURNS File Descriptor (FD)
* recv_tcp_server(PORT, VECTOR)
  * RETURNS number of bytes received (non-blocking, 0 is nothing available)
* send_tcp_server(PORT, VECTOR)
  * RETURNS number of bytes send (non-blocking, 0 is nothing written)

Library supports up to 256 TCP server instances. Each instance is setup by
setup_tcp_server. This returns a descriptor for that instance. Then that descriptor
is used for nothing. The field PORT is used to associate setup_tcp_server with a recv_tcp_server
and a send_tcp_server. This can be done in multiple calls.

### COMPONENTS
#### SRC

* tcp_server.c
* tcp_server.h
* tcp_server.sft
* recv_tcp_server.c
* recv_tcp_server.h
* send_tcp_server.c
* send_tcp_server.h
  
#### TB

* tb_vpi.v

### fusesoc

* fusesoc_info.core created.
* Simulation uses icarus to run data through the core.

#### TARGETS

* RUN WITH: (fusesoc run --target=sim VENDER:CORE:NAME:VERSION)
  - default (for IP integration builds)
  - sim
