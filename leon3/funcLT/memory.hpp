/***************************************************************************\
 *
 *   
 *            ___        ___           ___           ___
 *           /  /\      /  /\         /  /\         /  /\
 *          /  /:/     /  /::\       /  /::\       /  /::\
 *         /  /:/     /  /:/\:\     /  /:/\:\     /  /:/\:\
 *        /  /:/     /  /:/~/:/    /  /:/~/::\   /  /:/~/:/
 *       /  /::\    /__/:/ /:/___ /__/:/ /:/\:\ /__/:/ /:/
 *      /__/:/\:\   \  \:\/:::::/ \  \:\/:/__\/ \  \:\/:/
 *      \__\/  \:\   \  \::/~~~~   \  \::/       \  \::/
 *           \  \:\   \  \:\        \  \:\        \  \:\
 *            \  \ \   \  \:\        \  \:\        \  \:\
 *             \__\/    \__\/         \__\/         \__\/
 *   
 *
 *
 *   
 *   This file is part of TRAP.
 *   
 *   TRAP is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *   
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *   
 *
 *
 *   (c) Luca Fossati, fossati@elet.polimi.it
 *
\***************************************************************************/


#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <systemc.h>
#include <ToolsIf.hpp>
#include <trap_utils.hpp>

#define FUNC_MODEL
#define LT_IF
using namespace trap;
namespace leon3_funclt_trap{

    class MemoryInterface{

        public:
        virtual sc_dt::uint64 read_dword( const unsigned int & address ) throw() = 0;
        virtual unsigned int read_word( const unsigned int & address ) throw() = 0;
        virtual unsigned short int read_half( const unsigned int & address ) throw() = 0;
        virtual unsigned char read_byte( const unsigned int & address ) throw() = 0;
        virtual sc_dt::uint64 read_dword_dbg( const unsigned int & address );
        virtual unsigned int read_word_dbg( const unsigned int & address );
        virtual unsigned short int read_half_dbg( const unsigned int & address );
        virtual unsigned char read_byte_dbg( const unsigned int & address );
        virtual void write_dword( const unsigned int & address, sc_dt::uint64 datum ) throw() \
            = 0;
        virtual void write_word( const unsigned int & address, unsigned int datum ) throw() = 0;
        virtual void write_half( const unsigned int & address, unsigned short int datum ) \
            throw() = 0;
        virtual void write_byte( const unsigned int & address, unsigned char datum ) throw() = 0;
        virtual void write_dword_dbg( const unsigned int & address, sc_dt::uint64 datum );
        virtual void write_word_dbg( const unsigned int & address, unsigned int datum );
        virtual void write_half_dbg( const unsigned int & address, unsigned short int datum );
        virtual void write_byte_dbg( const unsigned int & address, unsigned char datum );
        virtual void lock() = 0;
        virtual void unlock() = 0;
        inline void swapEndianess( unsigned int & datum ) const throw(){
            unsigned char helperByte = 0;
            for(int i = 0; i < sizeof(unsigned int)/2; i++){
                helperByte = ((unsigned char *)&datum)[i];
                ((unsigned char *)&datum)[i] = ((unsigned char *)&datum)[sizeof(unsigned int) -1 -i];
                ((unsigned char *)&datum)[sizeof(unsigned int) -1 -i] = helperByte;
            }
        }
        inline void swapEndianess( unsigned short int & datum ) const throw(){
            unsigned char helperByte = 0;
            for(int i = 0; i < sizeof(unsigned short int)/2; i++){
                helperByte = ((unsigned char *)&datum)[i];
                ((unsigned char *)&datum)[i] = ((unsigned char *)&datum)[sizeof(unsigned short int) \
                    -1 -i];
                ((unsigned char *)&datum)[sizeof(unsigned short int) -1 -i] = helperByte;
            }
        }
        virtual ~MemoryInterface();
    };

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
        sc_dt::uint64 read_dword( const unsigned int & address ) throw();
        inline unsigned int read_word( const unsigned int & address ) throw(){
            if(address >= this->size){
                THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
            }

            unsigned int datum = *(unsigned int *)(this->memory + (unsigned long)address);
            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif

            return datum;
        }
        unsigned short int read_half( const unsigned int & address ) throw();
        unsigned char read_byte( const unsigned int & address ) throw();
        sc_dt::uint64 read_dword_dbg( const unsigned int & address ) throw();
        unsigned int read_word_dbg( const unsigned int & address ) throw();
        unsigned short int read_half_dbg( const unsigned int & address ) throw();
        unsigned char read_byte_dbg( const unsigned int & address ) throw();
        void write_dword( const unsigned int & address, sc_dt::uint64 datum ) throw();
        inline void write_word( const unsigned int & address, unsigned int datum ) throw(){
            if(address >= this->size){
                THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
            }
            if(this->debugger != NULL){
                this->debugger->notifyAddress(address, sizeof(datum));
            }

            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif

            *(unsigned int *)(this->memory + (unsigned long)address) = datum;
        }
        void write_half( const unsigned int & address, unsigned short int datum ) throw();
        void write_byte( const unsigned int & address, unsigned char datum ) throw();
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
