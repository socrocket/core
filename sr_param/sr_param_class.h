// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_param
/// @{
/// @file sr_param_class.h
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef SR_PARAM_CLASS_H_
#define SR_PARAM_CLASS_H_

#include <string>
#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp> // for parameter array!

#include "core/common/common.h"
#include "sr_param_t.h"
#include "sr_param_operator_macros.h"
#include "sr_param_delegate.h"

#define SR_PARAM_HEAD                                                    \
protected:                                                                \
    typedef sr_param<val_type> my_type;                                  \
    using sr_param_t<val_type>::my_value;                                \
    using sr_param_t<val_type>::m_api;                                   \
    using sr_param_t<val_type>::m_par_name;                              \
    using sr_param_t<val_type>::convertStringToValue;                    \
private:                                                                  \
    explicit sr_param(const val_type &val) { sc_assert(false); }         \
                                                                          \
public:                                                                   \
    explicit sr_param() : sr_param_t<val_type>(false, NULL, true) { sr_param_t<val_type>::init();  } \
    \
    explicit sr_param(const std::string &nam) : sr_param_t<val_type>(nam        , false, NULL, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const char *nam       ) : sr_param_t<val_type>(std::string(nam), false, NULL, true) { sr_param_t<val_type>::init(); } \
    \
    explicit sr_param(const std::string &nam, const std::string &val, const bool force_top_level_name = false) : sr_param_t<val_type>(nam        , force_top_level_name, NULL, true ) { sr_param_t<val_type>::init(convertStringToValue(val));         } \
    explicit sr_param(const char *nam,        const char *val       , const bool force_top_level_name = false) : sr_param_t<val_type>(std::string(nam), force_top_level_name, NULL, true ) { sr_param_t<val_type>::init(convertStringToValue(std::string(val))); } \
    explicit sr_param(const std::string &nam, const char *val       , const bool force_top_level_name = false) : sr_param_t<val_type>(nam        , force_top_level_name, NULL, true ) { sr_param_t<val_type>::init(convertStringToValue(std::string(val))); } \
    explicit sr_param(const char *nam,        const std::string &val, const bool force_top_level_name = false) : sr_param_t<val_type>(std::string(nam), force_top_level_name, NULL, true ) { sr_param_t<val_type>::init(convertStringToValue(val));         } \
    \
    explicit sr_param(const std::string &nam, const val_type &val, const bool force_top_level_name = false) : sr_param_t<val_type>(nam        , force_top_level_name, NULL, true )  { sr_param_t<val_type>::init(val); }    \
    explicit sr_param(const char *nam,        const val_type &val, const bool force_top_level_name = false) : sr_param_t<val_type>(std::string(nam), force_top_level_name, NULL, true )  { sr_param_t<val_type>::init(val); }    \
    \
    explicit sr_param(                                                gs_param_array* parent_array) : sr_param_t<val_type>(             false, parent_array, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const std::string &nam                        , gs_param_array* parent_array) : sr_param_t<val_type>(nam        , false, parent_array, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const char *nam                               , gs_param_array* parent_array) : sr_param_t<val_type>(std::string(nam), false, parent_array, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const std::string &nam, const std::string &val, gs_param_array* parent_array) : sr_param_t<val_type>(nam        , false, parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(val)); } \
    explicit sr_param(const char *nam,        const char *val       , gs_param_array* parent_array) : sr_param_t<val_type>(std::string(nam), false, parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(std::string(val))); } \
    explicit sr_param(const std::string &nam, const char *val       , gs_param_array* parent_array) : sr_param_t<val_type>(nam        , false, parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(std::string(val))); } \
    explicit sr_param(const char *nam,        const std::string &val, gs_param_array* parent_array) : sr_param_t<val_type>(std::string(nam), false, parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(val)); } \
    explicit sr_param(const std::string &nam, const val_type &val   , gs_param_array* parent_array) : sr_param_t<val_type>(nam        , false, parent_array, true) { sr_param_t<val_type>::init(val); }    \
    explicit sr_param(const char *nam,        const val_type &val   , gs_param_array* parent_array) : sr_param_t<val_type>(std::string(nam), false, parent_array, true) { sr_param_t<val_type>::init(val); }    \
    \
    explicit sr_param(                                                gs_param_array& parent_array) : sr_param_t<val_type>(             false, &parent_array, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const std::string &nam                        , gs_param_array& parent_array) : sr_param_t<val_type>(nam        , false, &parent_array, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const char *nam                               , gs_param_array& parent_array) : sr_param_t<val_type>(std::string(nam), false, &parent_array, true) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const std::string &nam, const std::string &val, gs_param_array& parent_array) : sr_param_t<val_type>(nam        , false, &parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(val)); } \
    explicit sr_param(const char *nam,        const char *val       , gs_param_array& parent_array) : sr_param_t<val_type>(std::string(nam), false, &parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(std::string(val))); } \
    explicit sr_param(const std::string &nam, const char *val       , gs_param_array& parent_array) : sr_param_t<val_type>(nam        , false, &parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(std::string(val))); } \
    explicit sr_param(const char *nam,        const std::string &val, gs_param_array& parent_array) : sr_param_t<val_type>(std::string(nam), false, &parent_array, true) { sr_param_t<val_type>::init(convertStringToValue(val)); } \
    explicit sr_param(const std::string &nam, const val_type &val   , gs_param_array& parent_array) : sr_param_t<val_type>(nam        , false, &parent_array, true) { sr_param_t<val_type>::init(val); }    \
    explicit sr_param(const char *nam,        const val_type &val   , gs_param_array& parent_array) : sr_param_t<val_type>(std::string(nam), false, &parent_array, true) { sr_param_t<val_type>::init(val); }    \
    \
    explicit sr_param(const std::string &nam, gs_param_array* parent_array, const bool force_top_level_name, const bool register_at_db) : sr_param_t<val_type>(nam, force_top_level_name,  parent_array, register_at_db) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const std::string &nam, gs_param_array& parent_array, const bool force_top_level_name, const bool register_at_db) : sr_param_t<val_type>(nam, force_top_level_name, &parent_array, register_at_db) { sr_param_t<val_type>::init(); } \
    explicit sr_param(const std::string &nam, const std::string &val, gs_param_array* parent_array, const bool force_top_level_name, const bool register_at_db) : sr_param_t<val_type>(nam, force_top_level_name,  parent_array, register_at_db) { sr_param_t<val_type>::init(convertStringToValue(val)); } \
    explicit sr_param(const std::string &nam, const std::string &val, gs_param_array& parent_array, const bool force_top_level_name, const bool register_at_db) : sr_param_t<val_type>(nam, force_top_level_name, &parent_array, register_at_db) { sr_param_t<val_type>::init(convertStringToValue(val)); } \
    \
    virtual ~sr_param() { sr_param_t<val_type>::destruct_gs_param(); }              \
    \
    \
    const bool deserialize(val_type &target_val, const std::string& str) {          \
    return static_deserialize(target_val, str);                                   \
    }                                                                               \
    \
    using sr_param_t<val_type>::operator =;                                         \
    \
    using sr_param_t<val_type>::name;                                \
    using sr_param_t<val_type>::setString;                                          \
    using sr_param_t<val_type>::getString;                                          \
    using sr_param_t<val_type>::setValue;                                           \
    using sr_param_t<val_type>::getValue


        /// The parameters, sr_param class, templated.
        /**
        * Each explicitely supported data type has a template specialisation.
        * See specialisations.
        * All data types for which no specialization is existing, will use this class.
        */
        template<typename T>
        class sr_param : public sr_param_t<T> {
            /// Typedef for the value.
            typedef T val_type;

          public:
            SR_PARAM_HEAD;

            /// Overloads sr_param_t<T>::convertValueToString
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
                //cout << "sr_param_t<T>::string_to_value("<<str<<", "<<val<<"), set val to: ";
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

        template<class T> bool operator == (sr_param<T> &p1, sr_param<T>& p2) {
            return p1.getValue() == p2.getValue();
        }

        template<class T> bool operator == (sr_param<T> &p1, T& p2) {
            return p1.getValue() == p2;
        }

        template<class T> bool operator == (T& p1, sr_param<T>& p2) {
            return p1 == p2.getValue();
        }

#include "sr_param_native.h"
#include "sr_param_stdint.h"
#include "sr_param_systemc.h"

#endif  // COMMON_SR_PARAM_SR_PARAM_CLASS_H_
/// @}
