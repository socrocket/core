// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file gs_config_class.h
/// 
///
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author 
///

// doxygen comments

#ifndef __GS_CONFIG_INTERNAL_H__
#define __GS_CONFIG_INTERNAL_H__

#include <string>
#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp> // for parameter array!

#include "common/gs_config/gs_config_t.h"
#include "common/gs_config/gs_config_operator_macros.h"


namespace gs {
    namespace cnf {

#define GS_CONFIG_HEAD                                                  \
protected:                                                             \
    typedef gs_config<val_type> my_type;                                  \
    using gs_config_t<val_type>::my_value;                                \
    using gs_config_t<val_type>::m_api;                                   \
    using gs_config_t<val_type>::m_par_name;                              \
    using gs_config_t<val_type>::convertStringToValue;                    \
private:                                                               \
    explicit gs_config(const val_type &val) { sc_assert(false); }         \
    \
public:                                                                \
    explicit gs_config() : gs_config_t<val_type>(false, NULL, true) { gs_config_t<val_type>::init();  } \
    \
    explicit gs_config(const std::string &nam) : gs_config_t<val_type>(nam        , false, NULL, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const char *nam       ) : gs_config_t<val_type>(string(nam), false, NULL, true) { gs_config_t<val_type>::init(); } \
    \
    explicit gs_config(const std::string &nam, const std::string &val, const bool force_top_level_name = false) : gs_config_t<val_type>(nam        , force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(val));         } \
    explicit gs_config(const char *nam,        const char *val       , const bool force_top_level_name = false) : gs_config_t<val_type>(string(nam), force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(string(val))); } \
    explicit gs_config(const std::string &nam, const char *val       , const bool force_top_level_name = false) : gs_config_t<val_type>(nam        , force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(string(val))); } \
    explicit gs_config(const char *nam,        const std::string &val, const bool force_top_level_name = false) : gs_config_t<val_type>(string(nam), force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(val));         } \
    \
    explicit gs_config(const std::string &nam, const val_type &val, const bool force_top_level_name = false) : gs_config_t<val_type>(nam        , force_top_level_name, NULL, true )  { gs_config_t<val_type>::init(val); }    \
    explicit gs_config(const char *nam,        const val_type &val, const bool force_top_level_name = false) : gs_config_t<val_type>(string(nam), force_top_level_name, NULL, true )  { gs_config_t<val_type>::init(val); }    \
    \
    explicit gs_config(                                                gs_param_array* parent_array) : gs_config_t<val_type>(             false, parent_array, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const std::string &nam                        , gs_param_array* parent_array) : gs_config_t<val_type>(nam        , false, parent_array, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const char *nam                               , gs_param_array* parent_array) : gs_config_t<val_type>(string(nam), false, parent_array, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array* parent_array) : gs_config_t<val_type>(nam        , false, parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val)); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const char *nam,        const char *val       , gs_param_array* parent_array) : gs_config_t<val_type>(string(nam), false, parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(string(val))); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const std::string &nam, const char *val       , gs_param_array* parent_array) : gs_config_t<val_type>(nam        , false, parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(string(val))); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const char *nam,        const std::string &val, gs_param_array* parent_array) : gs_config_t<val_type>(string(nam), false, parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val)); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const std::string &nam, const val_type &val   , gs_param_array* parent_array) : gs_config_t<val_type>(nam        , false, parent_array, true) { gs_config_t<val_type>::init(val); gs_config_base::addProperties()("default", convertValueToString(val))("type", gs_param_base::getTypeString()); }    \
    explicit gs_config(const char *nam,        const val_type &val   , gs_param_array* parent_array) : gs_config_t<val_type>(string(nam), false, parent_array, true) { gs_config_t<val_type>::init(val); gs_config_base::addProperties()("default", convertValueToString(val))("type", gs_param_base::getTypeString()); }    \
    \
    explicit gs_config(                                                gs_param_array& parent_array) : gs_config_t<val_type>(             false, &parent_array, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const std::string &nam                        , gs_param_array& parent_array) : gs_config_t<val_type>(nam        , false, &parent_array, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const char *nam                               , gs_param_array& parent_array) : gs_config_t<val_type>(string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(); } \
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array& parent_array) : gs_config_t<val_type>(nam        , false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val)); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const char *nam,        const char *val       , gs_param_array& parent_array) : gs_config_t<val_type>(string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(string(val))); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const std::string &nam, const char *val       , gs_param_array& parent_array) : gs_config_t<val_type>(nam        , false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(string(val))); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const char *nam,        const std::string &val, gs_param_array& parent_array) : gs_config_t<val_type>(string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val)); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString()); } \
    explicit gs_config(const std::string &nam, const val_type &val   , gs_param_array& parent_array) : gs_config_t<val_type>(nam        , false, &parent_array, true) { gs_config_t<val_type>::init(val); gs_config_base::addProperties()("default", convertValueToString(val))("type", gs_param_base::getTypeString()); }    \
    explicit gs_config(const char *nam,        const val_type &val   , gs_param_array& parent_array) : gs_config_t<val_type>(string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(val); gs_config_base::addProperties()("default", convertValueToString(val))("type", gs_param_base::getTypeString());}    \
    \
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array* parent_array, const bool force_top_level_name, const bool register_at_db) : gs_config_t<val_type>(nam, force_top_level_name,  parent_array, register_at_db) { gs_config_t<val_type>::init(convertStringToValue(val)); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString());        } \
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array& parent_array, const bool force_top_level_name, const bool register_at_db) : gs_config_t<val_type>(nam, force_top_level_name, &parent_array, register_at_db) { gs_config_t<val_type>::init(convertStringToValue(val)); gs_config_base::addProperties()("default", val)("type", gs_param_base::getTypeString());        } \
    \
    virtual ~gs_config() { gs_config_t<val_type>::destruct_gs_param(); }              \
    \
    \
    const bool deserialize(val_type &target_val, const std::string& str) {          \
    return static_deserialize(target_val, str);                                   \
    }                                                                               \
    \
    using gs_config_t<val_type>::operator =;                                         \
    \
    using gs_config_t<val_type>::name;                                \
    using gs_config_t<val_type>::setString;                                          \
    using gs_config_t<val_type>::getString;                                          \
    using gs_config_t<val_type>::setValue;                                           \
    using gs_config_t<val_type>::getValue


        /// The parameters, gs_config class, templated.
        /**
        * Each explicitely supported data type has a template specialisation.
        * See specialisations.
        * All data types for which no specialization is existing, will use this class.
        */
        template<typename T>
        class gs_config
            : public gs_config_t<T>
        {
            /// Typedef for the value.
            typedef T val_type;

        public:
            GS_CONFIG_HEAD;

            /// Overloads gs_config_t<T>::convertValueToString
            std::string convertValueToString(const val_type &val) const {
                std::ostringstream ss;
                ss << val;
                return ss.str();
            }

            /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
            /**
            * Does not change the target_val if str == ""
            */
            inline static bool static_deserialize(val_type &target_val, const std::string& str) {
                //cout << "gs_config_t<T>::string_to_value("<<str<<", "<<val<<"), set val to: ";
                std::istringstream ss(str);
                val_type tmp;
                if (str.length() == 0) {
                    return true;
                }
                ss >> tmp;
                // no conversion error
                if (ss.eof() && !ss.fail() && !ss.bad()) {
                    target_val = tmp;
                }
                else {
                    std::stringstream ess;
                    ess << "Conversion error: '" << str << "'";
                    SC_REPORT_WARNING("deserialize", ess.str().c_str());
                    return false;
                }
                return true;
            }

        };

        template<class T> bool operator == (gs_config<T> &p1, gs_config<T>& p2) {
            return p1.getValue() == p2.getValue();
        }

        template<class T> bool operator == (gs_config<T> &p1, T& p2) {
            return p1.getValue() == p2;
        }

        template<class T> bool operator == (T& p1, gs_config<T>& p2) {
            return p1 == p2.getValue();
        }

        // for compatibility
//#define gcnf_param gs_config;


        // /////////////////////////////////////////////////////////////////////////////// //
        // /////////////////// NATIVE DATA TYPES ///////////////////////////////////////// //
        // /////////////////////////////////////////////////////////////////////////////// //

#include "common/gs_config/gs_config_native.hpp"



        // /////////////////////////////////////////////////////////////////////////////// //
        // /////////////////// SystemC DATA TYPES //////////////////////////////////////// //
        // /////////////////////////////////////////////////////////////////////////////// //

#include "common/gs_config/gs_config_systemc.hpp"

    } // end namespace cnf
    using cnf::gs_config; // make gs_config available in gs namespace
} // end namespace gs

#endif
/// @}
