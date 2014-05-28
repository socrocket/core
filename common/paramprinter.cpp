// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file paramprinter.cpp
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Timo Veit
///

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
/// @}