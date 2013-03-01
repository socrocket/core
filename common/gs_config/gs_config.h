#include <string>
#include <vmap.h>

namespace gs {
    namespace cnf {
        class gs_config_base
        {
        public:  
            void setProperty(std::string key, std::string value)
            {
                m_properties[key] = value;
            }

            std::string getProperty(std::string key)
            {
                vmap<std::string, std::string>::iterator it;
                it = m_properties.find(key);
                if (it != m_properties.end())
                {
                    return it->second;
                }
                else return "Property not found";
            }

            vmap<std::string, std::string> getProperties()
            {
                vmap<std::string, std::string>::iterator it;
                vmap<std::string, std::string> properties;
                for (it = m_properties.begin(); it != m_properties.end(); ++it)
                {
                    properties[it->first] = it->second;
                }
                return properties;
            }

            void deleteProperty(std::string key)
            {
                vmap<std::string, std::string>::iterator it;
                it = m_properties.find(key);
                m_properties.erase(it);
            }

            bool exists(std::string key)
            {
                vmap<std::string, std::string>::iterator it;
                it = m_properties.find(key);
                if (it != m_properties.end())
                {
                    return true;
                }
                else return false;
            }

        private:
            vmap<std::string, std::string> m_properties;
        };
    }
}