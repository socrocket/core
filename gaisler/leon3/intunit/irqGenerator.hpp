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

#ifndef LT_IRQGENERATOR_HPP
#define LT_IRQGENERATOR_HPP

#include <iostream>

#include "core/common/systemc.h"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include <boost/lexical_cast.hpp>

#include "core/common/sr_signal.h"

//Now some stuff for generating random numbers
#include <ctime>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#ifdef BOOST_NO_STDC_NAMESPACE
namespace std {
  using ::time;
}
#endif

class IrqGenerator : public sr_signal::sr_signal_module<IrqGenerator>, public sc_module{
    private:
        sc_time latency;
        int lastIrq;
        // Random number generator to be used during algorithm execution
        boost::minstd_rand generator;

    public:
        //The initiator signal is used for sending out, raising interrupts;
        //the target signal for receiving the acknowledgement
        signal< std::pair<unsigned int, bool> >::out initSignal;
        signal< unsigned int >::in targSignal;

        SC_HAS_PROCESS( IrqGenerator );
        IrqGenerator( sc_module_name generatorName, sc_time latency ) : sc_module(generatorName),
                            targSignal(&IrqGenerator::callbackMethod, ("irq_target_" + boost::lexical_cast<std::string>(generatorName)).c_str()),
                                        latency(latency), generator(static_cast<unsigned int>(std::time(NULL))){
            this->lastIrq = -1;
            SC_THREAD(generateIrq);
            end_module();
        }

        //Method used for receiving acknowledgements of interrupts; the ack consists of
        //uninteresting data and the address corresponds to the interrupt signal to
        //be lowered
        //As a response, I lower the interrupt by sending a NULL pointer on the initSocket
        void callbackMethod(const unsigned int& value, const sc_time& delay){
            if(this->lastIrq < 0){
                THROW_EXCEPTION("Error, lowering an interrupt which hasn't been raised yet!!");
            }
            if(this->lastIrq != value){
                THROW_EXCEPTION("Error, lowering interrupt " << std::hex << std::showbase << value << " while " << std::hex << std::showbase << this->lastIrq << " was raised");
            }
            else{
                //finally I can really lower the interrupt: I send 0 on
                //the initSocked
                std::pair<unsigned int, bool> response(value, false);
                this->initSignal = response;
                this->lastIrq = -1;
            }
        }

        //Simple systemc thread which keeps on generating interrupts;
        //the number of the interrupt is printed to the screen before sending it to
        //the processor
        void generateIrq(){
            while(true){

                //An interrupt transaction is composed of a data pointer (containing
                //0 if the interrupt has to be lowered, different if raised) and an
                //address, corrisponding to the ID of the interrupt
                if(this->lastIrq == -1){
                    unsigned char data = 1;
                    tlm::tlm_generic_payload trans;
                    boost::uniform_int<> degen_dist(0x1, 0xe);
                    boost::variate_generator<boost::minstd_rand&, boost::uniform_int<> > deg(this->generator, degen_dist);
                    this->lastIrq = deg();
                    std::cerr << "Sending out IRQ id=" << std::hex << std::showbase << this->lastIrq << std::endl;

                    std::pair<unsigned int, bool> response((unsigned int)this->lastIrq, true);
                    this->initSignal = response;

                }
                wait(this->latency);
            }
        }
};

#endif

