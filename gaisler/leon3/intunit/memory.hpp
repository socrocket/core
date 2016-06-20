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


#ifndef LT_MEMORY_HPP
#define LT_MEMORY_HPP

#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/systemc.h"

#include "gaisler/leon3/mmucache/cpu_if.h"

#define FUNC_MODEL
#define LT_IF
using namespace trap;
namespace leon3_funclt_trap{
    class MemoryInterface : public cpu_if {};
};

namespace leon3_funclt_trap{

    class LocalMemory : public MemoryInterface{
        private:
        MemoryToolsIf< unsigned int > * debugger;
        char * memory;
        unsigned int size;

        public:
        LocalMemory( unsigned int size );
        void setDebugger( MemoryToolsIf< unsigned int > * debugger );
        sc_dt::uint64 read_dword( const unsigned int & address, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw();
        inline unsigned int read_word( const unsigned int & address, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw(){
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
        unsigned short int read_half( const unsigned int & address, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw();
        unsigned char read_byte( const unsigned int & address, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw();
        sc_dt::uint64 read_dword_dbg( const unsigned int & address ) throw();
        unsigned int read_word_dbg( const unsigned int & address ) throw();
        unsigned short int read_half_dbg( const unsigned int & address ) throw();
        unsigned char read_byte_dbg( const unsigned int & address ) throw();
        void write_dword( const unsigned int & address, sc_dt::uint64 datum, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw();
        inline void write_word( const unsigned int & address, unsigned int datum, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw(){
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
        void write_half( const unsigned int & address, unsigned short int datum, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw();
        void write_byte( const unsigned int & address, unsigned char datum, const unsigned int asi, const unsigned int flush, const unsigned int lock ) throw();
        void write_dword_dbg( const unsigned int & address, sc_dt::uint64 datum ) throw();
        void write_word_dbg( const unsigned int & address, unsigned int datum ) throw();
        void write_half_dbg( const unsigned int & address, unsigned short int datum ) throw();
        void write_byte_dbg( const unsigned int & address, unsigned char datum ) throw();
        void lock();
        void unlock();
        virtual ~LocalMemory();
    };

};



#endif
