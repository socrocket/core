#include <string>
#include <vmap.h>

class easy_init;
namespace gs {
    namespace cnf {
        class gs_config_base
        {
        public:  
class easy_init
{
public:
    easy_init::easy_init(gs::cnf::gs_config_base* owner) : m_owner(owner)
    {
    }

    easy_init::~easy_init()
    {

    }

    easy_init& easy_init::operator()(std::string key, std::string val)
    {
        m_owner->setProperty(key, val);
        return *this;
    }
private:
    gs::cnf::gs_config_base* m_owner;
};

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
        private:
            vmap<std::string, std::string> m_properties;
        };
    }
}