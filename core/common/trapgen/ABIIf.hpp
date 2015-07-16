/***************************************************************************\
*
*   This file is part of TRAP.
*
*   TRAP is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
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
*   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
\ ***************************************************************************/

#ifndef ABIIF_HPP
#define ABIIF_HPP

#include <vector>

#include <boost/circular_buffer.hpp>

#include "core/common/trapgen/instructionBase.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"

namespace trap {
template<class regWidth>
class ABIIf {
  public:
    inline bool matchEndian() const throw() {
#ifdef LITTLE_ENDIAN_BO
      return this->isLittleEndian();
#else
      return !this->isLittleEndian();
#endif
    }
    virtual void preCall() throw() {
    }
    virtual void postCall() throw() {
    }
    virtual void returnFromCall() throw() {
      this->setPC(this->readLR());
    }
    virtual int getProcessorID() const throw() {
      return 0;
    }
    virtual bool isInstrExecuting() const throw() = 0;
    virtual void waitInstrEnd() const throw() = 0;
    virtual bool isLittleEndian() const throw() = 0;
    virtual regWidth readLR() const throw() {
      THROW_ERROR("The LR register is not defined in the processor ABI");
      return 0;
    }
    virtual void setLR(const regWidth &newValue) throw() {
      THROW_ERROR("The LR register is not defined in the processor ABI");
    }
    virtual regWidth readPC() const throw() = 0;
    virtual void setPC(const regWidth &newValue) throw() = 0;
    virtual regWidth readSP() const throw() {
      THROW_ERROR("The SP register is not defined in the processor ABI");
      return 0;
    }
    virtual void setSP(const regWidth &newValue) throw() {
      THROW_ERROR("The SP register is not defined in the processor ABI");
    }
    virtual regWidth readFP() const throw() {
      THROW_ERROR("The FP register is not defined in the processor ABI");
      return 0;
    }
    virtual void setFP(const regWidth &newValue) throw() {
      THROW_ERROR("The FP register is not defined in the processor ABI");
    }
    virtual regWidth readRetVal() const throw() = 0;
    virtual void setRetVal(const regWidth &newValue) throw() = 0;
    virtual std::vector<regWidth> readArgs() const throw() = 0;
    virtual unsigned int nGDBRegs() const throw() = 0;
    virtual void setArgs(const std::vector<regWidth> &args) throw() = 0;
    virtual regWidth readGDBReg(const unsigned int &gdbId) const throw() = 0;
    virtual void setGDBReg(const regWidth &newValue, const unsigned int &gdbId) throw() = 0;
    virtual regWidth readMem(const regWidth &address) = 0;
    virtual unsigned char readCharMem(const regWidth &address) = 0;
    virtual void writeMem(const regWidth &address, regWidth datum) = 0;
    virtual void writeCharMem(const regWidth &address, unsigned char datum) = 0;
    virtual regWidth getCodeLimit() = 0;
    virtual bool isRoutineEntry(const InstructionBase *instr) throw() = 0;
    virtual bool isRoutineExit(const InstructionBase *instr) throw() = 0;
    virtual unsigned char*getState() const throw() = 0;
    virtual void setState(unsigned char *state) throw() = 0;
    virtual void setExitValue(unsigned int value) throw() = 0;
    virtual unsigned int getExitValue() throw() = 0;
    virtual ~ABIIf() {}
};
}

#endif
