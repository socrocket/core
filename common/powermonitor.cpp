//*****************************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*****************************************************************************
// Title:      powermonitor.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Demonstration of a power monitor for the SoCRocket
//             Virtual Platform
//
// Modified on $Date: 2011-06-09 08:49:53 +0200 (Thu, 09 Jun 2011) $
//          at $Revision: 452 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*****************************************************************************

#include "powermonitor.h"

// Constructor
powermonitor::powermonitor(sc_core::sc_module_name name,
                           sc_core::sc_time report_time,
                           bool exram) : 
  sc_module(name),
  m_report_time(report_time),
  m_exram(exram)
{

  if (report_time != SC_ZERO_TIME) {

    SC_THREAD(report_trigger);

  } 

}

void powermonitor::report_trigger() {

  while(1) {

    wait(m_report_time);
    gen_report();

    break;

  }
}

std::string powermonitor::get_model_name(std::string &param) {

  std::string sel;
  std::vector<std::string> tmp;

  // Get one parameter from the list
  boost::split(tmp, param, boost::is_any_of("."));

  // Extract model name
  switch (tmp.size()) {

    case 3: sel = tmp[0];
            break;
    case 4: sel = tmp[0] + "." + tmp[1];
            break;
    default:
            v::warn << this->name() << "Module name not valid!" << v::endl;

  }

  return sel;
}

std::vector<std::string> powermonitor::get_IP_params(std::vector<std::string> &params) {

  std::string sel;

  std::vector<std::string> tmp;
  std::vector<std::string> selection;

  // Iterator
  std::vector<std::string>::iterator it;

  if(params.size()!=0) {

    // Extract name of arbitrary model in list
    sel = get_model_name(params[0]);

    for (it=params.begin(); it<params.end(); it++) {

      // Does the parameter belong to the selected model
      if (get_model_name(*it) == sel) {

        // Add Parameter to selection
        selection.push_back(*it);

        // Remove Parameter from list
        params.erase(it);

        // Next iteration - same position
        it--;
      }
    }
  }

  return selection;

}

void powermonitor::gen_report() {

  // Static power of model (pW)
  double model_sta_power; 
  // Module internal power (uW)
  double model_int_power;
  // Module switching power (uW)
  double model_swi_power;

  // Total static power
  double total_sta_power = 0.0;
  // Total internal power (dynamic)
  double total_int_power = 0.0;
  // Total switching power (dynamic)
  double total_swi_power = 0.0;  

  gs::cnf::cnf_api *mApi = gs::cnf::GCnf_Api::getApiInstance(NULL);
  std::string n_power = "power";

  // Vector of all parameters
  std::vector<std::string> param_list = mApi->getParamList();
  // Vector power parameters
  std::vector<std::string> power_list, models_list;
  // Buffer for fields of parameter
  std::vector<std::string> param_fields;

  // *************************************************
  // Isolate power parameters
  
  for(uint32_t i=0; i<param_list.size();i++) {

    boost::split(param_fields, param_list[i], boost::is_any_of("."));
    // ahbctrl.power.dyn_power
    // mmu_cache.dvectorcache.power.sta_power
    if(param_fields.size() > 2) {

      if(param_fields[param_fields.size()-2] == "power") {

        power_list.push_back(param_list[i]);
     
      }
    }
  }

  // *************************************************
  // 

  while(power_list.size() != 0) {

    models_list = get_IP_params(power_list);

    if (!((!m_exram)&&((get_model_name(models_list[0])=="rom")||(get_model_name(models_list[0])=="sram")||(get_model_name(models_list[0])=="sdram")||(get_model_name(models_list[0])=="io")))) {

      v::info << name() << " ***************************************************** " << v::endl;
      v::info << name() << " * Component: " << get_model_name(models_list[0]) << v::endl;
      v::info << name() << " * --------------------------------------------------- " << v::endl;

      // Model static power
      if (mApi->existsParam(std::string(get_model_name(models_list[0]) + ".power.sta_power"))) {

        // Read models' static power
        mApi->getValue(std::string(get_model_name(models_list[0]) + ".power.sta_power"), model_sta_power);
      
        v::info << name() << " * Static power (leakage): " << model_sta_power << " pW" << v::endl;

        total_sta_power += model_sta_power;

      } else {

        //v::warn << name() << " * Model provides no static power information! " << v::endl;

      }

      // Does the model provide switching independent dynamic power (internal power) information
      if (mApi->existsParam(std::string(get_model_name(models_list[0]) + ".power.int_power"))) {

        mApi->getValue(std::string(get_model_name(models_list[0]) + ".power.int_power"), model_int_power);

        v::info << name() << " * Internal power (dynamic): " << model_int_power << " uW" << v::endl;

        total_int_power += model_int_power;

      } else {

        //v::warn << name() << " * Model provides no internal power information!" << v::endl;

      }    

      // Does the model induce switching dependent dynamic read power
      if (mApi->existsParam(std::string(get_model_name(models_list[0]) + ".power.swi_power"))) {
 
        mApi->getValue(std::string(get_model_name(models_list[0]) + ".power.swi_power"), model_swi_power);
        
        v::info << name() << " * Switching power (dynamic): " << model_swi_power << " uW" << v::endl;

        total_swi_power += model_swi_power;

      } else {

        //v::warn << name() << " * Model provides no switching power information!" << v::endl;

      }

      v::info << name() << " ***************************************************** " << v::endl;
    } else {

      v::info << name() << " ***************************************************** " << v::endl;
      v::info << name() << " * Component: " << get_model_name(models_list[0]) << " excluded!" << v::endl;
      v::info << name() << " ***************************************************** " << v::endl;
    }
  }

  v::info << name() << " ***************************************************** " << v::endl;
  v::info << name() << " * Power Summary: " << v::endl;
  v::info << name() << " * --------------------------------------------------- " << v::endl;
  v::info << name() << " * Static power (leakage): " << total_sta_power / 10e+6 << " uW" << v::endl;
  v::info << name() << " * Internal power (dynamic): " << total_int_power << " uW" << v::endl;
  v::info << name() << " * Switching power (dynamic): " << total_swi_power << " uW" << v::endl;
  v::info << name() << " * --------------------------------------------------- " << v::endl;
  v::info << name() << " * Total power: " << total_swi_power + (total_sta_power / 10e+6) << " uW" << v::endl;
  v::info << name() << " ***************************************************** " << v::endl;
}

// Collect power data at end of simulation
void powermonitor::end_of_simulation() {

  if (m_report_time == SC_ZERO_TIME) {

    gen_report();

  }

}
