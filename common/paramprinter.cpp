// ********************************************************************
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
// ********************************************************************
// Title:      json_parser.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Timo Veit
// Reviewed:
// ********************************************************************

#include "paramprinter.h"
#include <stdint.h>
#include <vmap.h>

paramprinter::paramprinter()
{
	mApi = gs::cnf::GCnf_Api::getApiInstance(NULL);
}

paramprinter::~paramprinter()
{
}

void paramprinter::printParams()
{
	std::cout << "Available System Options:" << std::endl;
	std::vector<std::string> paramList = mApi->getParamList();
	for(uint32_t i = 0; i < paramList.size(); i++) 
	{
  		std::cout << " " << paramList[i] << std::endl;
  	}
}

void paramprinter::printParams(std::string key)
{
	std::cout << "System Options containing " << key << ":" << std::endl;
	std::vector<std::string> paramList = mApi->getParamList();
	for(uint32_t i = 0; i < paramList.size(); i++) 
	{
		if(paramList[i].find(key) != std::string::npos)
		{
  			std::cout << " " << paramList[i] << std::endl;
		}
  	}
}

void paramprinter::printConfigs()
{
	std::cout << "gs_configs:" << std::endl;
	std::vector<gs::gs_param_base*> paramList = mApi->getParams();
	for(uint32_t i = 0; i < paramList.size(); i++) 
	{
		if(dynamic_cast<gs::cnf::gs_config_base*>(paramList[i]) != 0) 
		{
			std::cout << " " << paramList[i]->getName() << std::endl;
			gs::cnf::gs_config_base* config = dynamic_cast<gs::cnf::gs_config_base*>(paramList[i]);
			vmap<std::string, std::string> descriptionMap = config->getProperties();
			for(vmap<std::string, std::string>::iterator it = descriptionMap.begin(); it != descriptionMap.end(); ++it)
			{
				std::cout << "\t" << it->first << ": \t" << it->second << std::endl;
			}
		}
	}
}

void paramprinter::printConfigs(std::string key)
{
	std::cout << "gs_configs:" << std::endl;
	std::vector<gs::gs_param_base*> paramList = mApi->getParams();
	for(uint32_t i = 0; i < paramList.size(); i++) 
	{
		if(dynamic_cast<gs::cnf::gs_config_base*>(paramList[i]) != 0) 
		{
			if(paramList[i]->getName().find(key) != std::string::npos)
			{
				std::cout << " " << paramList[i]->getName() << std::endl;
				gs::cnf::gs_config_base* config = dynamic_cast<gs::cnf::gs_config_base*>(paramList[i]);
				vmap<std::string, std::string> descriptionMap = config->getProperties();
				for(vmap<std::string, std::string>::iterator it = descriptionMap.begin(); it != descriptionMap.end(); ++it)
				{
					std::cout << "\t" << it->first << ": \t" << it->second << std::endl;
				}
			}
		}
	}
}