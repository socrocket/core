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
   ) {

      ahb.register_b_transport(this, &Ctb_ahb_mem::b_transport);
      if(infile!=NULL) {
         readmem(infile, addr);
      }
}  // End constructor


/// Destructor
Ctb_ahb_mem::~Ctb_ahb_mem() {

   // Delete memory contents
   mem.clear();
}  // End destructor

/// TLM blocking transport function
void Ctb_ahb_mem::b_transport(unsigned int id,
                              tlm::tlm_generic_payload &gp,
                              sc_core::sc_time &delay) {

   switch(gp.get_command()) {
      // Read command
      case tlm::TLM_READ_COMMAND:
         for(uint32_t i=0; i<gp.get_data_length(); i++) {

            *(gp.get_data_ptr() + i) = mem[gp.get_address() + i];

         }
         gp.set_response_status(tlm::TLM_OK_RESPONSE);
         break;

      // Write command
      case tlm::TLM_WRITE_COMMAND:
         for(uint32_t i=0; i<gp.get_data_length(); i++) {

            mem[gp.get_address() + i] = *(gp.get_data_ptr() + i);

         }
         gp.set_response_status(tlm::TLM_OK_RESPONSE);
         break;

      // Neither read or write command
      default:
         v::warn << name() << "Received unknown command.\n";
         gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
         break;
   }
}  // void Ctb_ahb_mem::b_transport()

/// Method to initialize memory contents from a text file
int Ctb_ahb_mem::readmem(char infile_[], uint32_t addr) {

   std::ifstream infile(infile_, ios::in);
   char buffer          = 0;
   uint8_t nibbleBuffer = 0;
   unsigned char data   = 0;
   unsigned int nibble  = 0;
   bool processChar     = 0;

   // Parse input file
   if(infile.good()) {
      while(!infile.eof()) {
         // Read one byte from file
         infile.read(&buffer, 1);
         // Convert char into binary
         nibbleBuffer = char2nibble(&buffer);

         // combine two ascii chars into one byte and write into memory
         if(!(nibbleBuffer & 0xf0)) {
            if(nibble) {
               data |= nibbleBuffer;
               mem[addr++] = data;
               data = 0x0;
               nibble = 0;
            } else {
               data |= (nibbleBuffer << 4);
               nibble++;
            }
            processChar = 0;
         }
      }  // while(!infile.eof())

      // Warn if data stream ended unexpected
      if(nibble) {
        v::warn << name() << "Incomplete byte detected in memory file\n";
      }

      // close file
      infile.close();
      return 0;
   } else {
     v::error << name() << "File \"" << infile_ << "\" not found or readable\n";
      return 1;
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
         v::warn << name() << "Illegal character in memory file: \'"
            << *ch << "\'\n";
         return 0x80;
         break;
   }
}  // uint8_t Ctb_ahb_mem::char2nibble(char *ch) const


/// Method to dump the memory content into an text file
int Ctb_ahb_mem::dumpmem(char outfile_[]) {

   // check if memory is filled
   if(mem.empty()) {
     v::info << name() << "Memory is empty. Nothing do dump.\n";
      return 1;
   }

   std::ofstream outfile(outfile_, ios::out);
   uint32_t word     = 0;
   int      position = 0;
   // create map iterator and initialize to first element
   std::map<uint32_t, uint8_t>::iterator it      = mem.begin();
   std::map<uint32_t, uint8_t>::iterator it_next = mem.begin();
   it_next++;

   if(outfile.good()) {
      // print first address in file
      outfile << "@" << std::hex << std::setw(8) << std::setfill('0')
         << (it->first & 0xfffffffc) << endl;

      for(it=mem.begin(); it!=mem.end(); it++, it_next++) {

         position = it->first % 4;
         word |= (it->second << (8*position));

         // Next byte not within current word
         if( (3 - position - static_cast<int>(it_next->first - it->first)) < 0) {
            // print word
            outfile << std::hex << std::setw(8) << std::setfill('0') << word
               << endl;
            word = 0;

            // print address if necessary
            if( (7 - position - static_cast<int>(it_next->first - it->first)) < 0) {
               outfile << "@" << std::hex << std::setw(8) << std::setfill('0')
                  << (it_next->first & 0xfffffffc) << endl;
            }
         }
      }
      // print last word
      outfile << std::hex << std::setw(8) << std::setfill('0') << word
         << endl;
      return 0;
   } else {
     v::error << name() << "Unable to open dump file\n";
      return 1;
   }
}  // int tb_ahb_mem::dumpmem(char outfile_[])
