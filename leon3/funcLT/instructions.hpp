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


#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <instructionBase.hpp>
#include <string>
#include <customExceptions.hpp>
#include <trap_utils.hpp>
#include <registers.hpp>
#include <alias.hpp>
#include <externalPorts.hpp>
#include <externalPins.hpp>
#include <sstream>
#include <systemc.h>

#define FUNC_MODEL
#define LT_IF

#define RESET 0
#define DATA_STORE_ERROR 1
#define INSTR_ACCESS_MMU_MISS 2
#define INSTR_ACCESS_ERROR 3
#define R_REGISTER_ACCESS_ERROR 4
#define INSTR_ACCESS_EXC 5
#define PRIVILEDGE_INSTR 6
#define ILLEGAL_INSTR 7
#define FP_DISABLED 8
#define CP_DISABLED 9
#define UNIMPL_FLUSH 10
#define WATCHPOINT_DETECTED 11
#define WINDOW_OVERFLOW 12
#define WINDOW_UNDERFLOW 13
#define MEM_ADDR_NOT_ALIGNED 14
#define FP_EXCEPTION 15
#define CP_EXCEPTION 16
#define DATA_ACCESS_ERROR 17
#define DATA_ACCESS_MMU_MISS 18
#define DATA_ACCESS_EXC 19
#define TAG_OVERFLOW 20
#define DIV_ZERO 21
#define TRAP_INSTRUCTION 22
#define IRQ_LEV_15 23
#define IRQ_LEV_14 24
#define IRQ_LEV_13 25
#define IRQ_LEV_12 26
#define IRQ_LEV_11 27
#define IRQ_LEV_10 28
#define IRQ_LEV_9 29
#define IRQ_LEV_8 30
#define IRQ_LEV_7 31
#define IRQ_LEV_6 32
#define IRQ_LEV_5 33
#define IRQ_LEV_4 34
#define IRQ_LEV_3 35
#define IRQ_LEV_2 36
#define IRQ_LEV_1 37
#define IMPL_DEP_EXC 38

namespace leon3_funclt_trap{

    class Instruction : public InstructionBase{

        protected:
        inline void IncrementPC() throw(){
            unsigned int npc = NPC;
            PC = npc;
            npc += 4;
            NPC = npc;
        }
        Reg32_0 & PSR;
        Reg32_1 & WIM;
        Reg32_2 & TBR;
        Reg32_3 & Y;
        Reg32_3 & PC;
        Reg32_3 & NPC;
        RegisterBankClass & GLOBAL;
        Reg32_3 * WINREGS;
        Reg32_3 * ASR;
        Alias & FP;
        Alias & LR;
        Alias & SP;
        Alias & PCR;
        Alias * REGS;
        TLMMemory & instrMem;
        TLMMemory & dataMem;
        PinTLM_out_32 & irqAck;
        const unsigned int NUM_REG_WIN;
        const bool PIPELINED_MULT;

        public:
        Instruction( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual unsigned int behavior() = 0;
        virtual Instruction * replicate() const throw() = 0;
        virtual void setParams( const unsigned int & bitString ) throw() = 0;
        virtual std::string getInstructionName() const throw() = 0;
        virtual std::string getMnemonic() const throw() = 0;
        virtual unsigned int getId() const throw() = 0;
        inline void annull(){
            throw annull_exception();
        }
        inline void flush(){

        }
        inline void stall( const unsigned int & numCycles ){
            this->totalInstrCycles += numCycles;
        }
        bool IncrementRegWindow() throw();
        bool DecrementRegWindow() throw();
        int SignExtend( unsigned int bitSeq, unsigned int bitSeq_length ) const throw();
        void RaiseException( unsigned int pcounter, unsigned int npcounter, unsigned int \
            exceptionId, unsigned int customTrapOffset = 0 );
        bool checkIncrementWin() const throw();
        bool checkDecrementWin() const throw();
        unsigned int totalInstrCycles;
        virtual ~Instruction();
    };

};

namespace leon3_funclt_trap{

    class WB_plain_op : public virtual Instruction{

        protected:
        inline void WB_plain( Alias & rd, unsigned int & rd_bit, unsigned int & result ) throw(){

            rd = result;
        }

        public:
        WB_plain_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~WB_plain_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeLogic_op : public virtual Instruction{

        protected:
        inline void ICC_writeLogic( unsigned int & result ) throw(){

            PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
            PSR[key_ICC_z] = (result == 0);
            PSR[key_ICC_v] = 0;
            PSR[key_ICC_c] = 0;
        }

        public:
        ICC_writeLogic_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeLogic_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeTSub_op : public virtual Instruction{

        protected:
        inline void ICC_writeTSub( unsigned int & result, bool & temp_V, unsigned int & rs1_op, \
            unsigned int & rs2_op ) throw(){

            PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
            PSR[key_ICC_z] = (result == 0);
            PSR[key_ICC_v] = temp_V;
            PSR[key_ICC_c] = ((unsigned int)(((~rs1_op) & rs2_op) | (((~rs1_op) | rs2_op) & result))) \
                >> 31;
        }

        public:
        ICC_writeTSub_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeTSub_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeDiv_op : public virtual Instruction{

        protected:
        inline void ICC_writeDiv( bool & exception, unsigned int & result, bool & temp_V \
            ) throw(){

            if(!exception){
                PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
                PSR[key_ICC_z] = (result == 0);
                PSR[key_ICC_v] = temp_V;
                PSR[key_ICC_c] = 0;
            }
        }

        public:
        ICC_writeDiv_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeDiv_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeAdd_op : public virtual Instruction{

        protected:
        inline void ICC_writeAdd( unsigned int & result, unsigned int & rs1_op, unsigned \
            int & rs2_op ) throw(){

            PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
            PSR[key_ICC_z] = (result == 0);
            PSR[key_ICC_v] = ((unsigned int)((rs1_op & rs2_op & (~result)) | ((~rs1_op) & (~rs2_op) \
                & result))) >> 31;
            PSR[key_ICC_c] = ((unsigned int)((rs1_op & rs2_op) | ((rs1_op | rs2_op) & (~result)))) \
                >> 31;
        }

        public:
        ICC_writeAdd_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeAdd_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeSub_op : public virtual Instruction{

        protected:
        inline void ICC_writeSub( unsigned int & result, unsigned int & rs1_op, unsigned \
            int & rs2_op ) throw(){

            PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
            PSR[key_ICC_z] = (result == 0);
            PSR[key_ICC_v] = ((unsigned int)((rs1_op & (~rs2_op) & (~result)) | ((~rs1_op) & \
                rs2_op & result))) >> 31;
            PSR[key_ICC_c] = ((unsigned int)(((~rs1_op) & rs2_op) | (((~rs1_op) | rs2_op) & result))) \
                >> 31;
        }

        public:
        ICC_writeSub_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeSub_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeTAdd_op : public virtual Instruction{

        protected:
        inline void ICC_writeTAdd( unsigned int & result, bool & temp_V, unsigned int & rs1_op, \
            unsigned int & rs2_op ) throw(){

            PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
            PSR[key_ICC_z] = (result == 0);
            PSR[key_ICC_v] = temp_V;
            PSR[key_ICC_c] = ((unsigned int)((rs1_op & rs2_op) | ((rs1_op | rs2_op) & (~result)))) \
                >> 31;
        }

        public:
        ICC_writeTAdd_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeTAdd_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeTVSub_op : public virtual Instruction{

        protected:
        inline void ICC_writeTVSub( unsigned int & result, bool & temp_V, unsigned int & \
            rs1_op, unsigned int & rs2_op ) throw(){

            if(!temp_V){
                PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
                PSR[key_ICC_z] = (result == 0);
                PSR[key_ICC_v] = temp_V;
                PSR[key_ICC_c] = ((unsigned int)(((~rs1_op) & rs2_op) | (((~rs1_op) | rs2_op) & result))) \
                    >> 31;
            }
        }

        public:
        ICC_writeTVSub_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeTVSub_op();
    };

};

namespace leon3_funclt_trap{

    class WB_tv_op : public virtual Instruction{

        protected:
        inline void WB_tv( Alias & rd, unsigned int & rd_bit, unsigned int & result, bool \
            & temp_V ) throw(){

            if(!temp_V){
                rd = result;
            }
        }

        public:
        WB_tv_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        virtual ~WB_tv_op();
    };

};

namespace leon3_funclt_trap{

    class ICC_writeTVAdd_op : public virtual Instruction{

        protected:
        inline void ICC_writeTVAdd( unsigned int & result, bool & temp_V, unsigned int & \
            rs1_op, unsigned int & rs2_op ) throw(){

            if(!temp_V){
                PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
                PSR[key_ICC_z] = (result == 0);
                PSR[key_ICC_v] = 0;
                PSR[key_ICC_c] = ((unsigned int)((rs1_op & rs2_op) | ((rs1_op | rs2_op) & (~result)))) \
                    >> 31;
            }
        }

        public:
        ICC_writeTVAdd_op( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        virtual ~ICC_writeTVAdd_op();
    };

};

namespace leon3_funclt_trap{

    class InvalidInstr : public Instruction{

        public:
        InvalidInstr( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getInstructionName() const throw();
        std::string getMnemonic() const throw();
        unsigned int getId() const throw();
        virtual ~InvalidInstr();
    };

};

namespace leon3_funclt_trap{

    class READasr : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        unsigned int asr;

        protected:
        unsigned int asr_temp;

        public:
        READasr( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~READasr();
    };

};

namespace leon3_funclt_trap{

    class WRITEY_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;

        public:
        WRITEY_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEY_reg();
    };

};

namespace leon3_funclt_trap{

    class XNOR_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        XNOR_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XNOR_reg();
    };

};

namespace leon3_funclt_trap{

    class ANDNcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ANDNcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ANDNcc_reg();
    };

};

namespace leon3_funclt_trap{

    class LDSB_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int address;
        unsigned int readValue;

        public:
        LDSB_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSB_imm();
    };

};

namespace leon3_funclt_trap{

    class WRITEpsr_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int rd;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisorException;
        bool illegalCWP;
        unsigned int result;

        public:
        WRITEpsr_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEpsr_imm();
    };

};

namespace leon3_funclt_trap{

    class READy : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;

        protected:
        unsigned int y_temp;

        public:
        READy( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~READy();
    };

};

namespace leon3_funclt_trap{

    class XNORcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        XNORcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XNORcc_reg();
    };

};

namespace leon3_funclt_trap{

    class READpsr : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        unsigned int asr;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        unsigned int psr_temp;

        public:
        READpsr( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~READpsr();
    };

};

namespace leon3_funclt_trap{

    class ANDN_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        ANDN_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ANDN_imm();
    };

};

namespace leon3_funclt_trap{

    class ANDcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ANDcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ANDcc_reg();
    };

};

namespace leon3_funclt_trap{

    class TSUBcc_imm : public ICC_writeTSub_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        TSUBcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TSUBcc_imm();
    };

};

namespace leon3_funclt_trap{

    class LDSBA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool supervisor;

        public:
        LDSBA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSBA_reg();
    };

};

namespace leon3_funclt_trap{

    class LDUH_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool notAligned;

        public:
        LDUH_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDUH_imm();
    };

};

namespace leon3_funclt_trap{

    class STA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        bool notAligned;
        unsigned int address;
        unsigned int toWrite;

        public:
        STA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STA_reg();
    };

};

namespace leon3_funclt_trap{

    class ORN_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ORN_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ORN_reg();
    };

};

namespace leon3_funclt_trap{

    class LDSHA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool supervisor;
        bool notAligned;

        public:
        LDSHA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSHA_reg();
    };

};

namespace leon3_funclt_trap{

    class STBA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        unsigned int address;
        unsigned char toWrite;

        public:
        STBA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STBA_reg();
    };

};

namespace leon3_funclt_trap{

    class ST_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        unsigned int toWrite;

        public:
        ST_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ST_imm();
    };

};

namespace leon3_funclt_trap{

    class READtbr : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        unsigned int asr;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        unsigned int tbr_temp;

        public:
        READtbr( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~READtbr();
    };

};

namespace leon3_funclt_trap{

    class UDIVcc_imm : public ICC_writeDiv_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        bool exception;
        unsigned int result;
        bool temp_V;
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UDIVcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UDIVcc_imm();
    };

};

namespace leon3_funclt_trap{

    class SWAPA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        bool notAligned;
        unsigned int address;
        unsigned int readValue;
        unsigned int toWrite;

        public:
        SWAPA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SWAPA_reg();
    };

};

namespace leon3_funclt_trap{

    class ADDXcc_imm : public ICC_writeAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADDXcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADDXcc_imm();
    };

};

namespace leon3_funclt_trap{

    class STB_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int address;
        unsigned char toWrite;

        public:
        STB_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STB_imm();
    };

};

namespace leon3_funclt_trap{

    class SUBXcc_imm : public ICC_writeSub_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUBXcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUBXcc_imm();
    };

};

namespace leon3_funclt_trap{

    class STH_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        unsigned short int toWrite;

        public:
        STH_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STH_reg();
    };

};

namespace leon3_funclt_trap{

    class SRL_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        SRL_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SRL_imm();
    };

};

namespace leon3_funclt_trap{

    class WRITEasr_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int rd;
        unsigned int simm13;

        protected:
        unsigned int result;

        public:
        WRITEasr_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEasr_imm();
    };

};

namespace leon3_funclt_trap{

    class UMULcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UMULcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UMULcc_reg();
    };

};

namespace leon3_funclt_trap{

    class LDSTUB_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int address;
        unsigned int readValue;

        public:
        LDSTUB_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSTUB_reg();
    };

};

namespace leon3_funclt_trap{

    class XOR_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        XOR_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XOR_imm();
    };

};

namespace leon3_funclt_trap{

    class SMAC_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SMAC_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SMAC_reg();
    };

};

namespace leon3_funclt_trap{

    class WRITEasr_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        unsigned int rd;

        protected:
        unsigned int result;

        public:
        WRITEasr_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEasr_reg();
    };

};

namespace leon3_funclt_trap{

    class LD_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool notAligned;

        public:
        LD_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LD_reg();
    };

};

namespace leon3_funclt_trap{

    class ST_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        unsigned int toWrite;

        public:
        ST_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ST_reg();
    };

};

namespace leon3_funclt_trap{

    class SUBcc_reg : public ICC_writeSub_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUBcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUBcc_reg();
    };

};

namespace leon3_funclt_trap{

    class LDD_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        sc_dt::uint64 readValue;
        bool notAligned;

        public:
        LDD_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDD_reg();
    };

};

namespace leon3_funclt_trap{

    class ADDcc_imm : public ICC_writeAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADDcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADDcc_imm();
    };

};

namespace leon3_funclt_trap{

    class LDUH_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool notAligned;

        public:
        LDUH_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDUH_reg();
    };

};

namespace leon3_funclt_trap{

    class SRL_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SRL_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SRL_reg();
    };

};

namespace leon3_funclt_trap{

    class SAVE_imm : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool okNewWin;
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int newCwp;

        public:
        SAVE_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SAVE_imm();
    };

};

namespace leon3_funclt_trap{

    class MULScc_reg : public ICC_writeAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        MULScc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~MULScc_reg();
    };

};

namespace leon3_funclt_trap{

    class OR_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        OR_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~OR_imm();
    };

};

namespace leon3_funclt_trap{

    class STD_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        sc_dt::uint64 toWrite;

        public:
        STD_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STD_imm();
    };

};

namespace leon3_funclt_trap{

    class SUBXcc_reg : public ICC_writeSub_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUBXcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUBXcc_reg();
    };

};

namespace leon3_funclt_trap{

    class ADDX_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADDX_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADDX_imm();
    };

};

namespace leon3_funclt_trap{

    class SWAP_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        unsigned int readValue;
        unsigned int toWrite;

        public:
        SWAP_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SWAP_imm();
    };

};

namespace leon3_funclt_trap{

    class UMUL_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UMUL_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UMUL_reg();
    };

};

namespace leon3_funclt_trap{

    class WRITEY_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;

        public:
        WRITEY_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEY_imm();
    };

};

namespace leon3_funclt_trap{

    class AND_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        AND_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~AND_reg();
    };

};

namespace leon3_funclt_trap{

    class FLUSH_imm : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        public:
        FLUSH_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~FLUSH_imm();
    };

};

namespace leon3_funclt_trap{

    class SRA_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SRA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SRA_reg();
    };

};

namespace leon3_funclt_trap{

    class STH_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        unsigned short int toWrite;

        public:
        STH_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STH_imm();
    };

};

namespace leon3_funclt_trap{

    class WRITEwim_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int rd;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool raiseException;
        unsigned int result;

        public:
        WRITEwim_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEwim_imm();
    };

};

namespace leon3_funclt_trap{

    class LDD_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        sc_dt::uint64 readValue;
        bool notAligned;

        public:
        LDD_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDD_imm();
    };

};

namespace leon3_funclt_trap{

    class SLL_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        SLL_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SLL_imm();
    };

};

namespace leon3_funclt_trap{

    class LDUHA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool supervisor;
        bool notAligned;

        public:
        LDUHA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDUHA_reg();
    };

};

namespace leon3_funclt_trap{

    class TADDcc_reg : public ICC_writeTAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        TADDcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TADDcc_reg();
    };

};

namespace leon3_funclt_trap{

    class TADDcc_imm : public ICC_writeTAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        TADDcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TADDcc_imm();
    };

};

namespace leon3_funclt_trap{

    class SDIV_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int pcounter;
        unsigned int npcounter;
        bool exception;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SDIV_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SDIV_imm();
    };

};

namespace leon3_funclt_trap{

    class TSUBccTV_imm : public ICC_writeTVSub_op, public WB_tv_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        TSUBccTV_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TSUBccTV_imm();
    };

};

namespace leon3_funclt_trap{

    class FLUSH_reg : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        public:
        FLUSH_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~FLUSH_reg();
    };

};

namespace leon3_funclt_trap{

    class ORNcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ORNcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ORNcc_reg();
    };

};

namespace leon3_funclt_trap{

    class RETT_imm : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int targetAddr;
        unsigned int newCwp;
        bool exceptionEnabled;
        bool invalidWin;
        bool notAligned;
        bool supervisor;

        public:
        RETT_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~RETT_imm();
    };

};

namespace leon3_funclt_trap{

    class SDIVcc_reg : public ICC_writeDiv_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        bool exception;
        unsigned int result;
        bool temp_V;
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SDIVcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SDIVcc_reg();
    };

};

namespace leon3_funclt_trap{

    class ADD_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADD_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADD_reg();
    };

};

namespace leon3_funclt_trap{

    class TRAP_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int reserved1;
        unsigned int cond;
        unsigned int reserved2;
        unsigned int imm7;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool raiseException;

        public:
        TRAP_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TRAP_imm();
    };

};

namespace leon3_funclt_trap{

    class WRITEtbr_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int rd;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool raiseException;
        unsigned int result;

        public:
        WRITEtbr_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEtbr_imm();
    };

};

namespace leon3_funclt_trap{

    class LDUB_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int address;
        unsigned int readValue;

        public:
        LDUB_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDUB_reg();
    };

};

namespace leon3_funclt_trap{

    class RESTORE_reg : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool okNewWin;
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int newCwp;

        public:
        RESTORE_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~RESTORE_reg();
    };

};

namespace leon3_funclt_trap{

    class ADDXcc_reg : public ICC_writeAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADDXcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADDXcc_reg();
    };

};

namespace leon3_funclt_trap{

    class STB_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int address;
        unsigned char toWrite;

        public:
        STB_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STB_reg();
    };

};

namespace leon3_funclt_trap{

    class AND_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        AND_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~AND_imm();
    };

};

namespace leon3_funclt_trap{

    class SMUL_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SMUL_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SMUL_imm();
    };

};

namespace leon3_funclt_trap{

    class ADD_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADD_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADD_imm();
    };

};

namespace leon3_funclt_trap{

    class UMUL_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UMUL_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UMUL_imm();
    };

};

namespace leon3_funclt_trap{

    class READwim : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        unsigned int asr;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        unsigned int wim_temp;

        public:
        READwim( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~READwim();
    };

};

namespace leon3_funclt_trap{

    class LDSTUB_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int address;
        unsigned int readValue;

        public:
        LDSTUB_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSTUB_imm();
    };

};

namespace leon3_funclt_trap{

    class SMAC_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SMAC_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SMAC_imm();
    };

};

namespace leon3_funclt_trap{

    class LDSB_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int address;
        unsigned int readValue;

        public:
        LDSB_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSB_reg();
    };

};

namespace leon3_funclt_trap{

    class ANDN_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ANDN_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ANDN_reg();
    };

};

namespace leon3_funclt_trap{

    class TSUBccTV_reg : public ICC_writeTVSub_op, public WB_tv_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        TSUBccTV_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TSUBccTV_reg();
    };

};

namespace leon3_funclt_trap{

    class SETHI : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        unsigned int imm22;

        protected:
        unsigned int result;

        public:
        SETHI( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SETHI();
    };

};

namespace leon3_funclt_trap{

    class SRA_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        SRA_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SRA_imm();
    };

};

namespace leon3_funclt_trap{

    class LDSH_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool notAligned;

        public:
        LDSH_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSH_reg();
    };

};

namespace leon3_funclt_trap{

    class UDIVcc_reg : public ICC_writeDiv_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        bool exception;
        unsigned int result;
        bool temp_V;
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UDIVcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UDIVcc_reg();
    };

};

namespace leon3_funclt_trap{

    class ORN_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        ORN_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ORN_imm();
    };

};

namespace leon3_funclt_trap{

    class STD_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        sc_dt::uint64 toWrite;

        public:
        STD_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STD_reg();
    };

};

namespace leon3_funclt_trap{

    class ANDNcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        ANDNcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ANDNcc_imm();
    };

};

namespace leon3_funclt_trap{

    class TADDccTV_imm : public ICC_writeTVAdd_op, public WB_tv_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        TADDccTV_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TADDccTV_imm();
    };

};

namespace leon3_funclt_trap{

    class WRITEtbr_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        unsigned int rd;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool raiseException;
        unsigned int result;

        public:
        WRITEtbr_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEtbr_reg();
    };

};

namespace leon3_funclt_trap{

    class SUBX_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUBX_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUBX_reg();
    };

};

namespace leon3_funclt_trap{

    class XNOR_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        XNOR_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XNOR_imm();
    };

};

namespace leon3_funclt_trap{

    class UDIV_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int pcounter;
        unsigned int npcounter;
        bool exception;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UDIV_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UDIV_imm();
    };

};

namespace leon3_funclt_trap{

    class LDSH_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool notAligned;

        public:
        LDSH_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSH_imm();
    };

};

namespace leon3_funclt_trap{

    class UNIMP : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        unsigned int imm22;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        UNIMP( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UNIMP();
    };

};

namespace leon3_funclt_trap{

    class LDSTUBA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        unsigned int address;
        unsigned int readValue;

        public:
        LDSTUBA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDSTUBA_reg();
    };

};

namespace leon3_funclt_trap{

    class UMULcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UMULcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UMULcc_imm();
    };

};

namespace leon3_funclt_trap{

    class ORcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ORcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ORcc_reg();
    };

};

namespace leon3_funclt_trap{

    class MULScc_imm : public ICC_writeAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        MULScc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~MULScc_imm();
    };

};

namespace leon3_funclt_trap{

    class XORcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        XORcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XORcc_reg();
    };

};

namespace leon3_funclt_trap{

    class SUB_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUB_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUB_reg();
    };

};

namespace leon3_funclt_trap{

    class WRITEwim_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        unsigned int rd;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool raiseException;
        unsigned int result;

        public:
        WRITEwim_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEwim_reg();
    };

};

namespace leon3_funclt_trap{

    class UMAC_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UMAC_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UMAC_imm();
    };

};

namespace leon3_funclt_trap{

    class TSUBcc_reg : public ICC_writeTSub_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        TSUBcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TSUBcc_reg();
    };

};

namespace leon3_funclt_trap{

    class BRANCH : public Instruction{
        private:
        unsigned int a;
        unsigned int cond;
        unsigned int disp22;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        BRANCH( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~BRANCH();
    };

};

namespace leon3_funclt_trap{

    class SMULcc_reg : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SMULcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SMULcc_reg();
    };

};

namespace leon3_funclt_trap{

    class SUB_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUB_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUB_imm();
    };

};

namespace leon3_funclt_trap{

    class ADDcc_reg : public ICC_writeAdd_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADDcc_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADDcc_reg();
    };

};

namespace leon3_funclt_trap{

    class XOR_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        XOR_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XOR_reg();
    };

};

namespace leon3_funclt_trap{

    class SUBcc_imm : public ICC_writeSub_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUBcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUBcc_imm();
    };

};

namespace leon3_funclt_trap{

    class TADDccTV_reg : public ICC_writeTVAdd_op, public WB_tv_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        TADDccTV_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TADDccTV_reg();
    };

};

namespace leon3_funclt_trap{

    class SDIV_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int pcounter;
        unsigned int npcounter;
        bool exception;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SDIV_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SDIV_reg();
    };

};

namespace leon3_funclt_trap{

    class SMULcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SMULcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SMULcc_imm();
    };

};

namespace leon3_funclt_trap{

    class SWAP_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool notAligned;
        unsigned int address;
        unsigned int readValue;
        unsigned int toWrite;

        public:
        SWAP_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SWAP_reg();
    };

};

namespace leon3_funclt_trap{

    class SUBX_imm : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SUBX_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SUBX_imm();
    };

};

namespace leon3_funclt_trap{

    class STDA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        bool notAligned;
        unsigned int address;
        sc_dt::uint64 toWrite;

        public:
        STDA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STDA_reg();
    };

};

namespace leon3_funclt_trap{

    class UMAC_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UMAC_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UMAC_reg();
    };

};

namespace leon3_funclt_trap{

    class JUMP_imm : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool trapNotAligned;

        public:
        JUMP_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~JUMP_imm();
    };

};

namespace leon3_funclt_trap{

    class SMUL_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SMUL_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SMUL_reg();
    };

};

namespace leon3_funclt_trap{

    class XORcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        XORcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XORcc_imm();
    };

};

namespace leon3_funclt_trap{

    class ORNcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        ORNcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ORNcc_imm();
    };

};

namespace leon3_funclt_trap{

    class LDUBA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool supervisor;

        public:
        LDUBA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDUBA_reg();
    };

};

namespace leon3_funclt_trap{

    class JUMP_reg : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool trapNotAligned;

        public:
        JUMP_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~JUMP_reg();
    };

};

namespace leon3_funclt_trap{

    class ADDX_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        ADDX_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ADDX_reg();
    };

};

namespace leon3_funclt_trap{

    class UDIV_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int pcounter;
        unsigned int npcounter;
        bool exception;
        bool temp_V;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        UDIV_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~UDIV_reg();
    };

};

namespace leon3_funclt_trap{

    class XNORcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        XNORcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~XNORcc_imm();
    };

};

namespace leon3_funclt_trap{

    class STBAR : public Instruction{

        public:
        STBAR( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STBAR();
    };

};

namespace leon3_funclt_trap{

    class LDA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool supervisor;
        bool notAligned;

        public:
        LDA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDA_reg();
    };

};

namespace leon3_funclt_trap{

    class STHA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisor;
        bool notAligned;
        unsigned int address;
        unsigned short int toWrite;

        public:
        STHA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~STHA_reg();
    };

};

namespace leon3_funclt_trap{

    class LDDA_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        sc_dt::uint64 readValue;
        bool supervisor;
        bool notAligned;

        public:
        LDDA_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDDA_reg();
    };

};

namespace leon3_funclt_trap{

    class SLL_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SLL_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SLL_reg();
    };

};

namespace leon3_funclt_trap{

    class RESTORE_imm : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool okNewWin;
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int newCwp;

        public:
        RESTORE_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~RESTORE_imm();
    };

};

namespace leon3_funclt_trap{

    class LD_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int address;
        unsigned int readValue;
        bool notAligned;

        public:
        LD_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LD_imm();
    };

};

namespace leon3_funclt_trap{

    class TRAP_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        unsigned int reserved1;
        unsigned int cond;
        unsigned int asi;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool raiseException;

        public:
        TRAP_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~TRAP_reg();
    };

};

namespace leon3_funclt_trap{

    class LDUB_imm : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rd;
        unsigned int rd_bit;
        unsigned int simm13;

        protected:
        unsigned int address;
        unsigned int readValue;

        public:
        LDUB_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~LDUB_imm();
    };

};

namespace leon3_funclt_trap{

    class RETT_reg : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int targetAddr;
        unsigned int newCwp;
        bool exceptionEnabled;
        bool invalidWin;
        bool notAligned;
        bool supervisor;

        public:
        RETT_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~RETT_reg();
    };

};

namespace leon3_funclt_trap{

    class SDIVcc_imm : public ICC_writeDiv_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        bool exception;
        unsigned int result;
        bool temp_V;
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        SDIVcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SDIVcc_imm();
    };

};

namespace leon3_funclt_trap{

    class SAVE_reg : public Instruction{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool okNewWin;
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;
        unsigned int newCwp;

        public:
        SAVE_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~SAVE_reg();
    };

};

namespace leon3_funclt_trap{

    class OR_reg : public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;

        protected:
        unsigned int result;
        unsigned int rs1_op;
        unsigned int rs2_op;

        public:
        OR_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~OR_reg();
    };

};

namespace leon3_funclt_trap{

    class ORcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        ORcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ORcc_imm();
    };

};

namespace leon3_funclt_trap{

    class CALL : public Instruction{
        private:
        unsigned int disp30;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        unsigned int oldPC;

        public:
        CALL( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 \
            & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, \
            Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~CALL();
    };

};

namespace leon3_funclt_trap{

    class WRITEpsr_reg : public Instruction{
        private:
        Alias rs1;
        unsigned int rs1_bit;
        Alias rs2;
        unsigned int rs2_bit;
        unsigned int rd;

        protected:
        unsigned int pcounter;
        unsigned int npcounter;
        bool supervisorException;
        bool illegalCWP;
        unsigned int result;

        public:
        WRITEpsr_reg( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & \
            PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~WRITEpsr_reg();
    };

};

namespace leon3_funclt_trap{

    class ANDcc_imm : public ICC_writeLogic_op, public WB_plain_op{
        private:
        Alias rd;
        unsigned int rd_bit;
        Alias rs1;
        unsigned int rs1_bit;
        unsigned int simm13;

        protected:
        unsigned int result;
        unsigned int rs1_op;

        public:
        ANDcc_imm( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
            Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
            & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, TLMMemory \
            & dataMem, PinTLM_out_32 & irqAck );
        unsigned int behavior();
        Instruction * replicate() const throw();
        std::string getInstructionName() const throw();
        unsigned int getId() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getMnemonic() const throw();
        virtual ~ANDcc_imm();
    };

};

namespace leon3_funclt_trap{

    class IRQ_IRQ_Instruction : public Instruction{

        protected:
        unsigned int pcounter;
        unsigned int npcounter;

        public:
        IRQ_IRQ_Instruction( Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 \
            & PC, Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, \
            Alias & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, TLMMemory & instrMem, \
            TLMMemory & dataMem, PinTLM_out_32 & irqAck, unsigned int & IRQ );
        unsigned int behavior();
        Instruction * replicate() const throw();
        void setParams( const unsigned int & bitString ) throw();
        std::string getInstructionName() const throw();
        std::string getMnemonic() const throw();
        unsigned int getId() const throw();
        unsigned int & IRQ;
        inline void setInterruptValue( const unsigned int & interruptValue ) throw(){
            this->IRQ = interruptValue;
        }
        virtual ~IRQ_IRQ_Instruction();
    };

};



#endif
