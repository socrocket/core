#include <string>
#include <vmap.h>

class easy_init;
namespace gs {
    namespace cnf {
        class gs_config_base
        {
        public:  
            void setProperty(std::string key, std::string value);
            std::string getProperty(std::string key);
            vmap<std::string, std::string> getProperties();
            void deleteProperty(std::string key);
            bool exists(std::string key);
				easy_init addProperties();
        private:
            vmap<std::string, std::string> m_properties;
        };
    }
}

class easy_init
{
public:
    easy_init::easy_init(gs::cnf::gs_config_base* owner) : m_owner(owner)
    {
    }

    easy_init::~easy_init()
    {

    }

    easy_init& easy_init::operator()(char* key, char* val)
    {
        m_owner->setProperty(key, val);
        return *this;
    }
private:
    gs::cnf::gs_config_base* m_owner;
};