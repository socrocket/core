//-----------------------------------------------------------------------------
// Title:      tb_ahb_mem.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Provide a test bench memory class with AHB slave interface.
//
// Method:     Memory is modeled with a map.
//             Considered field of the tlm_generic_payload are:
//                - command
//                - address
//                - data_ptr
//                - data_length
//                - response
//             All other fields are ignored.
//             Delays are not modeled.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Soeren Brinkmann
// Reviewed:
//-----------------------------------------------------------------------------

#include "tb_ahb_mem.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include "verbose.h"

/// Constructor
Ctb_ahb_mem::Ctb_ahb_mem(sc_core::sc_module_name nm,   // Module name
                           uint16_t haddr,   // AMBA AHB address (12 bit)
                           uint16_t hmask,   // AMBA AHB address mask (12 bit)
                           char infile[],    // Memory initialization file
                           uint32_t addr) :  // Address for memory initalization
   sc_module(nm),
   pnpahb(
      0x04, // vendor_id: ESA
      0,    // device: TODO: get real device ID
      0,    //
      0,    // IRQ
      GrlibBAR(CGrlibDevice::AHBMEM, hmask, 0, 0, haddr),    // GrlibBAR 0
      0,    // GrlibBAR 1
      0,    // GrlibBAR 2
      0     // GrlibBAR 3
   ),
   ahb(
      "ahb",            // sc_module_name
      amba::amba_AHB,   // bus type
      amba::amba_LT,    // abstraction level
      false             // is arbiter
   ), name(nm) {

      ahb.register_b_transport(this, &Ctb_ahb_mem::b_transport);
      if(infile!=NULL) {
         readmem(infile, addr);
      }
}  // End constructor


/// Destructor
Ctb_ahb_mem::~Ctb_ahb_mem() {

//    cout << name << " Memory content: \n" << endl;
//    for(std::map<unsigned int, unsigned char>::iterator it=mem.begin();
//        it!=mem.end(); it++) {
//       printf("@%#8.8x: %#2.2x\n", it->first, it->second);
//    }
   mem.clear();
}  // End destructor

/// TLM blocking transport function
void Ctb_ahb_mem::b_transport(tlm::tlm_generic_payload &gp,
                               sc_core::sc_time &delay) {

   switch(gp.get_command()) {
      // Read command
      case tlm::TLM_READ_COMMAND:
         for(uint32_t i=0; i<gp.get_data_length(); i++) {

            *(gp.get_data_ptr() + i) = mem[i];

         }
         gp.set_response_status(tlm::TLM_OK_RESPONSE);
         break;

      // Write command
      case tlm::TLM_WRITE_COMMAND:
         for(uint32_t i=0; i<gp.get_data_length(); i++) {

            mem[i] = *(gp.get_data_ptr() + i);

         }
         gp.set_response_status(tlm::TLM_OK_RESPONSE);
         break;

      // Neither read or write command
      default:
         VWARN << name << ": Received unknown command.\n";
         gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
         break;
   }
}  // void Ctb_ahb_mem::b_transport()

/// Method to initialize memory contents from a text file
int Ctb_ahb_mem::readmem(char infile_[], uint32_t addr) {

   std::ifstream infile(infile_, ios::in);
   char buffer = 0;
   uint8_t nibble_buffer = 0;
   unsigned char data = 0;
   unsigned int nibble = 0;
   bool process_char = 0;

   // Parse input file
   if(infile.good()) {
      while(!infile.eof()) {
         // Read one byte from file
         infile.read(&buffer, 1);
         // Convert char into binary
         nibble_buffer = char2nibble(&buffer);

         // combine two ascii chars into one byte and write into memory
         if(!(nibble_buffer & 0xf0)) {
            if(nibble) {
               data |= nibble_buffer;
               mem[addr++] = data;
               data = 0x0;
               nibble = 0;
            } else {
               data |= (nibble_buffer << 4);
               nibble++;
            }
            process_char = 0;
         }
      }  // while(!infile.eof())

      // Warn if data stream ended unexpected
      if(nibble) {
         VWARN << name << ": Incomplete byte detected in memory file\n";
      }

      // close file
      infile.close();
      return 0;
   } else {
      VERROR << name << ": File \"" << infile_ << "\" not found or readable\n";
      return infile.good();
   }
}  // int Ctb_ahb_mem::readmem(char infile_[], unsigned int addr)


/// Method to convert ascii characters into their binary representation
uint8_t Ctb_ahb_mem::char2nibble(const char *ch) const {
   switch(*ch) {
      case '\n':
         return 0x10;
         break;
      case ' ':
         return 0x20;
         break;
      case '\t':
         return 0x20;
         break;
      case '0':
         return 0x00;
         break;
      case '1':
         return 0x01;
         break;
      case '2':
         return 0x02;
         break;
      case '3':
         return 0x03;
         break;
      case '4':
         return 0x04;
         break;
      case '5':
         return 0x05;
         break;
      case '6':
         return 0x06;
         break;
      case '7':
         return 0x07;
         break;
      case '8':
         return 0x08;
         break;
      case '9':
         return 0x09;
         break;
      case 'a':
         return 0x0a;
         break;
      case 'b':
         return 0x0b;
         break;
      case 'c':
         return 0x0c;
         break;
      case 'd':
         return 0x0d;
         break;
      case 'e':
         return 0x0e;
         break;
      case 'f':
         return 0x0f;
         break;
      case 'A':
         return 0x0a;
         break;
      case 'B':
         return 0x0b;
         break;
      case 'C':
         return 0x0c;
         break;
      case 'D':
         return 0x0d;
         break;
      case 'E':
         return 0x0e;
         break;
      case 'F':
         return 0x0f;
         break;
      default:
         VWARN << name << ": Illegal character in memory file: \'"
            << *ch << "\'\n";
         return 0x80;
         break;
   }
}  // uint8_t Ctb_ahb_mem::char2nibble(char *ch) const


/// Method to dump the memory content into an text file
int Ctb_ahb_mem::dumpmem(char outfile_[]) {

   std::ofstream outfile(outfile_, ios::out);

   if(outfile.good()) {
      for(std::map<unsigned int, unsigned char>::iterator it=mem.begin();
          it!=mem.end(); it++) {
         outfile << "@0x" << std::hex << std::setw(8) << std::setfill('0')
            << it->first << ": 0x" << std::setw(2)
            << static_cast<unsigned int>(it->second) << endl;
      }
      return 0;
   } else {
      VERROR << name << ": Unable to open dump file\n";
      return outfile.good();
   }
}  // int tb_ahb_mem::dumpmem(char outfile_[])
