/***************************************************************************\
 *
 *   
 *         _/        _/_/_/_/    _/_/    _/      _/   _/_/_/
 *        _/        _/        _/    _/  _/_/    _/         _/
 *       _/        _/_/_/    _/    _/  _/  _/  _/     _/_/
 *      _/        _/        _/    _/  _/    _/_/         _/
 *     _/_/_/_/  _/_/_/_/    _/_/    _/      _/   _/_/_/
 *   
 *
 *
 *   
 *   This file is part of LEON3.
 *   
 *   LEON3 is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *   
 *
 *
 *   (c) Luca Fossati, fossati.l@gmail.com
 *
\***************************************************************************/



#include "gaisler/leon3/intunit/memory.hpp"
#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/systemc.h"

using namespace leon3_funclt_trap;
using namespace trap;
/*
sc_dt::uint64 leon3_funclt_trap::MemoryInterface::read_dword_dbg( const unsigned \
    int & address ){
    return this->read_dword(address, 0x8, 0, 0);
}

unsigned int leon3_funclt_trap::MemoryInterface::read_word_dbg( const unsigned int \
    & address ){
    return this->read_word(address, 0x8, 0, 0);
}

unsigned short int leon3_funclt_trap::MemoryInterface::read_half_dbg( const unsigned \
    int & address ){
    return this->read_half(address, 0x8, 0, 0);
}

unsigned char leon3_funclt_trap::MemoryInterface::read_byte_dbg( const unsigned int \
    & address ){
    return this->read_byte(address, 0x8, 0, 0);
}

void leon3_funclt_trap::MemoryInterface::write_dword_dbg( const unsigned int & address, \
    sc_dt::uint64 datum ){
    this->write_dword(address, datum, 0x8, 0, 0);
}

void leon3_funclt_trap::MemoryInterface::write_word_dbg( const unsigned int & address, \
    unsigned int datum ){
    this->write_word(address, datum, 0x8, 0, 0);
}

void leon3_funclt_trap::MemoryInterface::write_half_dbg( const unsigned int & address, \
    unsigned short int datum ){
    this->write_half(address, datum, 0x8, 0, 0);
}

void leon3_funclt_trap::MemoryInterface::write_byte_dbg( const unsigned int & address, \
    unsigned char datum ){
    this->write_byte(address, datum, 0x8, 0, 0);
}

leon3_funclt_trap::MemoryInterface::~MemoryInterface(){

}
*/
void leon3_funclt_trap::LocalMemory::setDebugger( MemoryToolsIf< unsigned int > * \
    debugger ){
    this->debugger = debugger;
}

sc_dt::uint64 leon3_funclt_trap::LocalMemory::read_dword( const unsigned int & address, \
							  const unsigned int asi,
							  const unsigned int flush,
							  const unsigned int lock ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    sc_dt::uint64 datum = *(sc_dt::uint64 *)(this->memory + (unsigned long)address);
    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    return datum;
}

unsigned short int leon3_funclt_trap::LocalMemory::read_half( const unsigned int & address,
							      const unsigned int asi,
							      const unsigned int flush,
							      const unsigned int lock ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    unsigned short int datum = *(unsigned short int *)(this->memory + (unsigned long)address);
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    return datum;
}

unsigned char leon3_funclt_trap::LocalMemory::read_byte( const unsigned int & address,
							 const unsigned int asi,
							 const unsigned int flush,
							 const unsigned int lock ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    unsigned char datum = *(unsigned char *)(this->memory + (unsigned long)address);

    return datum;
}

sc_dt::uint64 leon3_funclt_trap::LocalMemory::read_dword_dbg( const unsigned int \
    & address ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    sc_dt::uint64 datum = *(sc_dt::uint64 *)(this->memory + (unsigned long)address);
    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    return datum;
}

unsigned int leon3_funclt_trap::LocalMemory::read_word_dbg( const unsigned int & \
    address ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    unsigned int datum = *(unsigned int *)(this->memory + (unsigned long)address);
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    return datum;
}

unsigned short int leon3_funclt_trap::LocalMemory::read_half_dbg( const unsigned \
    int & address ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    unsigned short int datum = *(unsigned short int *)(this->memory + (unsigned long)address);
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    return datum;
}

unsigned char leon3_funclt_trap::LocalMemory::read_byte_dbg( const unsigned int & \
    address ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }

    unsigned char datum = *(unsigned char *)(this->memory + (unsigned long)address);

    return datum;
}

void leon3_funclt_trap::LocalMemory::write_dword( const unsigned int & address, 
						  sc_dt::uint64 datum,
						  const unsigned int asi,
						  const unsigned int flush,
						  const unsigned int lock ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    *(sc_dt::uint64 *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::write_half( const unsigned int & address, 
						 unsigned short int datum,
						 const unsigned int asi,
						 const unsigned int flush,
						 const unsigned int lock ) throw(){
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    *(unsigned short int *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::write_byte( const unsigned int & address, 
						 unsigned char datum,
						 const unsigned int asi,
						 const unsigned int flush,
						 const unsigned int lock ) throw(){
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }


    *(unsigned char *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::write_dword_dbg( const unsigned int & address, \
    sc_dt::uint64 datum ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    *(sc_dt::uint64 *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::write_word_dbg( const unsigned int & address, \
    unsigned int datum ) throw(){
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    *(unsigned int *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::write_half_dbg( const unsigned int & address, \
    unsigned short int datum ) throw(){
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    *(unsigned short int *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::write_byte_dbg( const unsigned int & address, \
    unsigned char datum ) throw(){
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    if(address >= this->size){
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
    }
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }


    *(unsigned char *)(this->memory + (unsigned long)address) = datum;
}

void leon3_funclt_trap::LocalMemory::lock(){

}

void leon3_funclt_trap::LocalMemory::unlock(){

}

leon3_funclt_trap::LocalMemory::LocalMemory( unsigned int size ) : size(size){
    this->memory = new char[size];
    this->debugger = NULL;
}

leon3_funclt_trap::LocalMemory::~LocalMemory(){
    delete [] this->memory;
}

