#include "gs_config.h"

namespace gs {
    namespace cnf {
            void gs_config_base::setProperty(std::string key, std::string value)
            {
                m_properties[key] = value;
            }

            std::string gs_config_base::getProperty(std::string key)
            {
                vmap<std::string, std::string>::iterator it;
                it = m_properties.find(key);
                if (it != m_properties.end())
                {
                    return it->second;
                }
                else return "Property not found";
            }

            vmap<std::string, std::string> gs_config_base::getProperties()
            {
                vmap<std::string, std::string>::iterator it;
                vmap<std::string, std::string> properties;
                for (it = m_properties.begin(); it != m_properties.end(); ++it)
                {
                    properties[it->first] = it->second;
                }
                return properties;
            }

            void gs_config_base::deleteProperty(std::string key)
            {
                vmap<std::string, std::string>::iterator it;
                it = m_properties.find(key);
                m_properties.erase(it);
            }

            bool gs_config_base::exists(std::string key)
            {
                vmap<std::string, std::string>::iterator it;
                it = m_properties.find(key);
                if (it != m_properties.end())
                {
                    return true;
                }
                else return false;
            }
            easy_init gs_config_base::addProperties()            
            {
                return easy_init(this);
            }
      }
}