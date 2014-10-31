//   GreenControl framework
//
// LICENSETEXT
//
//   Copyright (C) 2007 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
//   Wolfgang Klingauf <klingauf@eis.cs.tu-bs.de>
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
//
//
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT
using std::istringstream;

// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< INT > /////////////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<int>.
/**
* Default value = 0.
*/
template<>
class gs_config<int>
    : public gs_config_t<int>
{
    /// Typedef for the value.
    typedef int val_type;

public:
    GS_CONFIG_HEAD;

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("int");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_INT;
    }

    /// Overloads gs_param_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const{
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        GS_PARAM_DUMP_WITHNAME("gs_param", "deserialize: string='"<< ss.str().c_str() << "'");
        if (ss.str().length() == 0) {
            target_val = 0;
            GS_PARAM_DUMP_WITHNAME("gs_param", "deserialize: set default value");
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_param", "(int) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("gs_param", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); /// @TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_param", "(int) stream eof and not fail"); 
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




// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< UNSIGNED INT > ////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<unsigned int>.
/**
* Default value = 0.
*/
template<>
class gs_config<unsigned int>
    : public gs_config_t<unsigned int>
{
    /// Typedef for the value.
    typedef unsigned int val_type;

public:
    GS_CONFIG_HEAD;

    explicit gs_config(const std::string &nam, const int &val) : gs_config_t<val_type>(nam        )  { gs_config_t<unsigned int>::init((unsigned int)val);  }
    explicit gs_config(const char *nam,        const int &val) : gs_config_t<val_type>(string(nam))  { gs_config_t<unsigned int>::init((unsigned int)val);  }

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;


    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("unsigned int");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_UINT;
    }

    /// Overloads gs_param_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        GS_PARAM_DUMP_WITHNAME("gs_param", "deserialize: string='" << ss.str().c_str() <<"'");
        if (ss.str().length() == 0) {
            target_val = 0;
            GS_PARAM_DUMP_WITHNAME("gs_param", "deserialize: set default value");
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_param", "(unsigned int) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); /// @TODO(all): changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_param", "(unsigned int) stream eof and not fail"); 
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




// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< BOOL > ////////////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<bool>.
/**
* Default value = false.
*/
template<>
class gs_config<bool>
    : public gs_config_t<bool>
{
    /// Typedef for the value.
    typedef bool val_type;

public:
    GS_CONFIG_HEAD;

    // ///////////////////////
    //  operators

    // unsafe for bool: (according to MSVS)
    //GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    // unsafe for bool: (according to MSVS)
    //GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    // not allowed for bool: 
    // GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("bool");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_BOOL;
    }

    /// Overloads gs_param_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        GS_PARAM_DUMP_WITHNAME("gs_param", "deserialize: string='"<<str.c_str()<<"'");
        if (str.length() == 0) {
            GS_PARAM_DUMP_WITHNAME("gs_param", "getValue: set default value (false)");
            target_val = false;
        }
        else if (strcmp( str.c_str(), "true" ) == 0) {
            GS_PARAM_DUMP_WITHNAME("gs_param", "getValue: set value true");
            target_val = true;
        }
        else if (strcmp( str.c_str(), "false" ) == 0) {
            GS_PARAM_DUMP_WITHNAME("gs_param", "getValue: set value false");
            target_val = false;
        }
        else {
            std::istringstream ss(str);
            //ss.setf(ios::boolalpha);
            val_type tmp;
            ss >> tmp;
            // no conversion error
            if (ss.eof() && !ss.fail() && !ss.bad()) {
                target_val = tmp;
                GS_PARAM_DUMP_WITHNAME("gs_param", "(bool) stream eof and not fail"); 
            }
            else {
                std::stringstream ess;
                ess << "Conversion error: '" << str << "'";
                SC_REPORT_WARNING("deserialize", ess.str().c_str());
                return false;
            }
        }
        return true;
    }


};




// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< DOUBLE > //////////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<double>.
/**
* Default value = 0.
*/
template<>
class gs_config<double>
    : public gs_config_t<double>
{
    /// Typedef for the value.
    typedef double val_type;

public:
    GS_CONFIG_HEAD;

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    // not allowed for double:
    // GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("double");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_DOUBLE;
    }

    /// Overloads gs_param_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }
        val_type tmp;
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




// /////////////////////////////////////////////////////////////////////////////// //
// //////////////////  gs_config< FLOAT >  //////////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<float>.
/**
* Default value = 0.
*/
template<>
class gs_config<float>
    : public gs_config_t<float>
{
    /// Typedef for the value.
    typedef float val_type;

public:
    GS_CONFIG_HEAD;

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    // not allowed for float:
    // GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("float");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_FLOAT;
    }

    /// Overloads gs_param_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }
        val_type tmp;
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




// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< STRING > //////////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //


/// Template specialization for gs_config<std::string>.
/**
* 
*/
template<>
class gs_config<std::string>
    : public gs_config_t<std::string>
{
protected:
    /// Typedef for this specialized class.
    typedef gs_config<std::string> my_type;
    /// Typedef for the value.
    typedef std::string val_type;

    using gs_config_t<val_type>::my_value;
    using gs_config_t<val_type>::m_api;
    using gs_config_t<val_type>::m_par_name;

    using gs_config_t<val_type>::convertStringToValue;

private:
    // String constructor is allowed for string parameter

public:
    // Explicit constructors to avoid implicit construction of parameters.

    /// Empty constructor. Name will be set in base
    explicit gs_config() : gs_config_t<val_type>() { gs_config_t<val_type>::init();  }

    /// Constructor with (local/hierarchical) name.
    explicit gs_config(const std::string &nam) : gs_config_t<val_type>(nam             ) { gs_config_t<val_type>::init(); } 
    explicit gs_config(const char *nam       ) : gs_config_t<val_type>(std::string(nam)) { gs_config_t<val_type>::init(); } 

    /// Constructor with (local/hierarchical) name and string representation of initial value.
    explicit gs_config(const std::string &nam, const std::string &val, const bool force_top_level_name = false) : gs_config_t<val_type>(nam             , force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(val));         }
    explicit gs_config(const char *nam,        const char *val       , const bool force_top_level_name = false) : gs_config_t<val_type>(std::string(nam), force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(string(val))); } 
    explicit gs_config(const std::string &nam, const char *val       , const bool force_top_level_name = false) : gs_config_t<val_type>(nam             , force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(string(val))); } 
    explicit gs_config(const char *nam,        const std::string &val, const bool force_top_level_name = false) : gs_config_t<val_type>(std::string(nam), force_top_level_name, NULL, true ) { gs_config_t<val_type>::init(convertStringToValue(val));         } 

    // Constructors with parent array
    explicit gs_config(                                                gs_param_array* parent_array) : gs_config_t<val_type>(                  false,  parent_array, true) { gs_config_t<val_type>::init(); }
    explicit gs_config(const std::string &nam                        , gs_param_array* parent_array) : gs_config_t<val_type>(nam             , false,  parent_array, true) { gs_config_t<val_type>::init(); } 
    explicit gs_config(const char *nam                               , gs_param_array* parent_array) : gs_config_t<val_type>(std::string(nam), false,  parent_array, true) { gs_config_t<val_type>::init(); } 
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array* parent_array) : gs_config_t<val_type>(nam             , false,  parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val));         }
    explicit gs_config(const char *nam,        const char *val       , gs_param_array* parent_array) : gs_config_t<val_type>(std::string(nam), false,  parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(std::string(val))); } 
    explicit gs_config(const std::string &nam, const char *val       , gs_param_array* parent_array) : gs_config_t<val_type>(nam             , false,  parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(std::string(val))); } 
    explicit gs_config(const char *nam,        const std::string &val, gs_param_array* parent_array) : gs_config_t<val_type>(std::string(nam), false,  parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val));         } 
    explicit gs_config(                                                gs_param_array& parent_array) : gs_config_t<val_type>(                  false, &parent_array, true) { gs_config_t<val_type>::init(); }
    explicit gs_config(const std::string &nam                        , gs_param_array& parent_array) : gs_config_t<val_type>(nam             , false, &parent_array, true) { gs_config_t<val_type>::init(); } 
    explicit gs_config(const char *nam                               , gs_param_array& parent_array) : gs_config_t<val_type>(std::string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(); } 
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array& parent_array) : gs_config_t<val_type>(nam             , false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val));         }
    explicit gs_config(const char *nam,        const char *val       , gs_param_array& parent_array) : gs_config_t<val_type>(std::string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(std::string(val))); } 
    explicit gs_config(const std::string &nam, const char *val       , gs_param_array& parent_array) : gs_config_t<val_type>(nam             , false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(std::string(val))); } 
    explicit gs_config(const char *nam,        const std::string &val, gs_param_array& parent_array) : gs_config_t<val_type>(std::string(nam), false, &parent_array, true) { gs_config_t<val_type>::init(convertStringToValue(val));         } 

    // Constructors with register_at_db bool
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array* parent_array, const bool force_top_level_name, const bool register_at_db) : gs_config_t<val_type>(nam, force_top_level_name,  parent_array, register_at_db) { gs_config_t<val_type>::init(convertStringToValue(val));         } 
    explicit gs_config(const std::string &nam, const std::string &val, gs_param_array& parent_array, const bool force_top_level_name, const bool register_at_db) : gs_config_t<val_type>(nam, force_top_level_name, &parent_array, register_at_db) { gs_config_t<val_type>::init(convertStringToValue(val));         } 

    GS_CONFIG_DELEGATE;
    const val_type convertStringToValue(const std::string& str) {    
        return str; 
    }                                                                 
    void serialize(const val_type &val) {                             
        my_value = val;                                      
    }

    /// Overloads gs_config_t<T>::deserialize in gs_config_t<T>
    const bool deserialize(val_type &target_val, const std::string& str) {
        return static_deserialize(target_val, str);
    }

    /// To resolve the correct = operator
    using gs_config_t<val_type>::operator =;
    /// @TODO(all): other operators??

    //using gs_config::name;
    using gs_config_t<val_type>::setString;
    using gs_config_t<val_type>::getString;
    using gs_config_t<val_type>::setValue;
    using gs_config_t<val_type>::getValue;

    /// Set the value of this parameter with char.
    /**
    * @param v  Char value which has to be set.
    * @return   Pointer to this.
    */
    my_type& operator = (const char *v) { 
        setValue(envvar_subst(v, m_par_name));
        return *this;
    }

    /// Desctructor
    ~gs_config() { gs_config_t<val_type>::destruct_gs_param(); }

    // ///////////////////////
    //  operators
    // see outside the class definition

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("string");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_STRING;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return val;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        return val;//m_api->setParam(m_par_name, val);
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        target_val = str;
        return true;
    }

};


#define CNSC_STRING_OPERATOR_IMPLEMENTATIONS(otype)                     \
    inline bool operator otype (const gs_config<string>& lhs,              \
    const gs_config<string>& rhs)              \
{                                                                     \
    return const_cast<gs_config<string>& >(lhs).getValue() otype         \
    const_cast<gs_config<string>& >(rhs).getValue();                   \
}                                                                     \
    \
    inline bool operator otype (const string& lhs, const gs_config<string>& rhs) \
{                                                                     \
    return lhs otype const_cast<gs_config<string>& >(rhs).getValue();    \
}                                                                     \
    \
    inline bool operator otype (const gs_config<string>& lhs, const string& rhs) \
{                                                                     \
    return const_cast<gs_config<string>& >(lhs).getValue() otype rhs;    \
}                                                                     \
    \
    \
    \
    inline bool operator otype (const char *lhs, const gs_config<string>& rhs) \
{                                                                     \
    return string(lhs) otype const_cast<gs_config<string>& >(rhs).getValue(); \
}                                                                     \
    \
    inline bool operator otype (const gs_config<string>& lhs, const char *rhs) \
{                                                                     \
    return const_cast<gs_config<string>& >(lhs).getValue() otype string(rhs); \
}                                                                     \

CNSC_STRING_OPERATOR_IMPLEMENTATIONS(==)
    CNSC_STRING_OPERATOR_IMPLEMENTATIONS(!=)
    CNSC_STRING_OPERATOR_IMPLEMENTATIONS(<)
    CNSC_STRING_OPERATOR_IMPLEMENTATIONS(<=)
    CNSC_STRING_OPERATOR_IMPLEMENTATIONS(>)
    CNSC_STRING_OPERATOR_IMPLEMENTATIONS(>=)

#undef CNSC_STRING_OPERATOR_IMPLEMENTATIONS


    inline std::ostream& operator << (std::ostream& os, const gs_config<string>& str)
{
    os << const_cast<gs_config<string>& >(str).getValue();
    return os;
}

/// @TODO(all): check if works!
inline std::istream& operator>>(std::istream& is, gs_config<std::string>& str)
{
    string ret_str(const_cast<gs_config<std::string>& >(str).getValue());
    is >> ret_str;
    const_cast<gs_config<string>& >(str).setString(ret_str);
    return is;
}

inline std::string operator+ (const gs_config<std::string>& lhs, const gs_config<std::string>& rhs)
{
    return const_cast<gs_config<std::string>& >(lhs).getValue() + const_cast<gs_config<std::string>& >(rhs).getValue();
}

inline std::string operator+ (const gs_config<std::string>& lhs, const std::string& rhs)
{
    return const_cast<gs_config<std::string>& >(lhs).getValue() + rhs;
}

inline std::string operator+ (const std::string& lhs, const gs_config<std::string>& rhs)
{
    return lhs + const_cast<gs_config<std::string>& >(rhs).getValue();
}

inline std::string operator+ (const gs_config<std::string>& lhs, const char *rhs)
{
    return const_cast<gs_config<std::string>& >(lhs).getValue() + std::string(rhs);
}

inline std::string operator+ (const char *lhs, const gs_config<std::string>& rhs)
{
    return std::string(lhs) + const_cast<gs_config<std::string>& >(rhs).getValue();
}

///////////////////////////////////////////////////////////////////////////////// //
///////////////////// gs_param< std::vector<std::string> > ////////////////////// //
///////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_param< std::vector<std::string> >.
/// Default value = empty vector.
/// @TODO(all): maybe template<typename T>
// class gs_param< std::vector<T> >
// issue: (de)serialization of T
template<>
class gs_config<std::vector<std::string> >
  : public gs_config_t<std::vector<std::string> > {
  /// Typedef for the value.
  typedef std::vector<std::string> val_type;
  public:
    GS_CONFIG_HEAD;

    /////////////////////////
    // additional functions

    bool empty() const {return my_value.empty(); }
    /// @TODO(all) optional: More of the vector functions

    /////////////////////////
    // overloaded functions

    /// Overloads gs_param_base::getTypeString
    const std::string getTypeString() const {
      return std::string("std::vector<std::string>");
    }

    /// Overloads gs_param_base::getType
    const Param_type getType() const {
      return PARTYPE_NOT_AVAILABLE;
    }

    const std::string& operator [] (const int index) { 
      return my_value[index];
    }

    size_t size() {
      return my_value.size();
    }

    /// Overloads gs_param_t<T>::serialize
    /**
     * Serializes the vector to a single line comma separated list
     * of the members.<br>
     * Members surrounded by quotes (").
     * The whole string surrounded by { }
     */
    std::string convertValueToString(const val_type &val) const {
      return static_convertValueToString(val);
    }

    inline static std::string static_convertValueToString(const val_type &val) {
      std::ostringstream ss;
      ss << "{";
      bool first = true;
      val_type::const_iterator iter;

      for (iter = val.begin(); iter != val.end(); iter++) {
        if (!first) {ss << ','; }
        first = false;
        // make sure included "s are handled

        std::string::size_type loc = iter->find('"');
        std::string aux = *iter;

        while (loc != std::string::npos) {
          if (aux[loc - 1] != '\\') {
            aux = aux.substr(0, loc) + "\\" + aux.substr(loc, aux.npos);
          }
          loc = aux.find('"', loc + 1);
        }

        ss << '"' << aux << '"'; // not nescessarily quotes
      }
      ss << "}";
      return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    /**
     * Deserializes a single-line string into a vector.
     * - The string has to look like: "{"value1","value 2","value 3"}"
     * - Optionally there may be spaces between the values and the comma.
     * - so far the value must not contain quotes: TODO!!
     */
    inline static bool static_deserialize(val_type &target_val, const std::string &str) {
      std::istringstream ss(str);

      GS_PARAM_DUMP_WITHNAME("gs_param", "deserialize: string='" << ss.str().c_str() << "'");
      if (ss.str().length() == 0) {
        target_val.clear();
        GS_PARAM_DUMP_WITHNAME("gs_param", "vector<string> deserialize: set default value");
        return true;
      } else {
        target_val.clear();

        std::string::size_type index = str.find("{");
        if (index == str.npos) {
          SC_REPORT_ERROR("gs_param", "vector<string> deserialize error!");
        }
        std::string my_string = str;
        std::string value;
        std::string word;
        std::string::size_type indexB, indexC;
        do {
          if (my_string == "}") {break; }

          index = 0;
          do {
            index = my_string.find("\"", index);
          } while (my_string[index - 1] == '\\' && index != std::string::npos);

          indexB = index;
          do {
            indexB = my_string.find("\"", indexB + 1);
          } while (my_string[indexB - 1] == '\\' && indexB != std::string::npos);

          if ((index == std::string::npos) || (indexB == std::string::npos)) {break; }

          value = my_string.substr(index + 1, indexB - index - 1);

          // now we must handle \" that can be inside the string, and transform them to single "
          indexC = value.find("\\\"");
          while (indexC != std::string::npos) {
            value = value.substr(0, indexC) + value.substr(indexC + 1, value.npos);
            indexC = value.find("\\\"", indexC);
          }

          target_val.push_back(value);
          my_string = my_string.substr(indexB + 1, word.npos);
        } while (index != std::string::npos);
        return true;
      }
      return true;
    }
};

// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< UNSIGNED LONG LONG > //////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<unsigned long long int>.
/**
* Default value = 0.
*/
template<>
class gs_config<unsigned long long>
    : public gs_config_t<unsigned long long>
{
    /// Typedef for the value.
    typedef unsigned long long val_type;

public:
    GS_CONFIG_HEAD;
    /// Constructor with unsigned int value
    //explicit gs_config(const std::string &nam, const unsigned long &val) : gs_config_t<val_type>(nam         )  { gs_config_t<val_type>::init((val_type)val); }    
    //explicit gs_config(const char *nam,        const unsigned long &val) : gs_config_t<val_type>(string(nam) )  { gs_config_t<val_type>::init((val_type)val); }    
    /// Constructor with int value
    //explicit gs_config(const std::string &nam, const long &val) : gs_config_t<val_type>(nam         )  { gs_config_t<val_type>::init((val_type)val); }    
    //explicit gs_config(const char *nam,        const long &val) : gs_config_t<val_type>(string(nam) )  { gs_config_t<val_type>::init((val_type)val); }    

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("unsigned long long");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_ULONGLONG;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(unsigned long long) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.clear(); ss.str(str);
            ss >> (std::hex) >> tmp;
        }
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



// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< LONG LONG > //////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<long long int>.
/**
* Default value = 0.
*/
template<>
class gs_config<long long>
    : public gs_config_t<long long>
{
    /// Typedef for the value.
    typedef long long val_type;

public:
    GS_CONFIG_HEAD;
    /// Constructor with int value
    explicit gs_config(const std::string &nam, const int &val) : gs_config_t<val_type>(nam         )  { gs_config_t<val_type>::init(val); }    
    explicit gs_config(const char *nam,        const int &val) : gs_config_t<val_type>(string(nam) )  { gs_config_t<val_type>::init(val); }    


    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("long long");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_LONGLONG;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(long long) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.clear(); ss.str(str);
            ss >> (std::hex) >> tmp;
        }
        // if still error try deserialize to unsigned long long
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.clear(); ss.str(str);
            unsigned long long ulltmp;
            ss >> ulltmp;
            if (!ss.eof() || ss.fail() || ss.bad()) {
                ss.clear(); ss.str(str);
                ss >> (std::hex) >> ulltmp;
            }
            if (ss.eof() && !ss.fail() && !ss.bad()) {
                tmp = ulltmp;
            }
        }

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



// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< UNSIGNED CHAR > //////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<unsigned char>.
/**
* Default value = 0.
* @todo Regression tests to be done!
*/
template<>
class gs_config<unsigned char>
    : public gs_config_t<unsigned char>
{
    /// Typedef for the value.
    typedef unsigned char val_type;

public:
    GS_CONFIG_HEAD;

    // To resolve the correct = operator
    //using gs_config_t<val_type>::operator =;
    my_type& operator = (const val_type& v) { 
        //cout << "operator = (val_type) value="<<convertValueToString(v) <<endl;
        setValue(v);
        return *this;
    }

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("unsigned char");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_UCHAR;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << (int) val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }

        int tmp=0;
        ss >> tmp;

        if (tmp >= 0 && tmp <= 255) {
            target_val = (unsigned char) tmp;
            return true;
        }

        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.clear(); ss.str(str);
            ss >> (std::hex) >> tmp;
        }
        // if still error, try characters 'x'
        if (!ss.eof() || ss.fail() || ss.bad()) {
            if (str.length() == 3 && str[0] == '\'' && str.at(2) == '\'') {
                target_val = str.at(1);
                return true;
            }
        }

        // no conversion error
        if ((ss.eof() && !ss.fail() && !ss.bad())  && (tmp >= 0 && tmp <= 255)) {
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

// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< SIGNED CHAR > /////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<signed char>.
/**
* Default value = 0.
* @todo Regression tests to be done!
*/
template<>
class gs_config<signed char>
    : public gs_config_t<signed char>
{
    /// Typedef for the value.
    typedef signed char val_type;

public:
    GS_CONFIG_HEAD;

    // To resolve the correct = operator
    //using gs_config_t<val_type>::operator =;
    my_type& operator = (const val_type& v) { 
        //cout << "operator = (val_type) value="<<convertValueToString(v) <<endl;
        setValue(v);
        return *this;
    }

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("signed char");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_SIGNED_CHAR;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << (int) val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }

        int tmp=0;
        ss >> tmp;

        if (tmp <= 127 && tmp >= -128) {
            target_val = (signed char) tmp;
            return true;
        }

        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.clear(); ss.str(str);
            ss >> (std::hex) >> tmp;
        }
        // if still error, try characters 'x'
        if (!ss.eof() || ss.fail() || ss.bad()) {
            if (str.length() == 3 && str[0] == '\'' && str.at(2) == '\'') {
                target_val = str.at(1);
                return true;
            }
        }
        // no conversion error
        if ((ss.eof() && !ss.fail() && !ss.bad()) && (tmp <= 127 && tmp >= -128)) {
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


// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< CHAR > ////////////////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<char>.
/**
* Default value = 0.
* @todo Regression tests to be done!
*/
template<>
class gs_config<char>
    : public gs_config_t<char>
{
    /// Typedef for the value.
    typedef char val_type;

public:
    GS_CONFIG_HEAD;

    // To resolve the correct = operator
    //using gs_config_t<val_type>::operator =;
    my_type& operator = (const val_type& v) { 
        //cout << "operator = (val_type) value="<<convertValueToString(v) <<endl;
        setValue(v);
        return *this;
    }

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("char");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_CHAR;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);;
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << (int) val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        // This is to warn the use once if he is using a char in a numeric way, which is not specified by the C standard
        static bool warned_arithmetic_usage = false;

        std::istringstream ss(str);
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }

        int tmp=0;
        ss >> tmp;

        if (tmp <= 127 && tmp >= -128) {
            if (!warned_arithmetic_usage) {
                SC_REPORT_WARNING("deserialize", "A 'char' parameter gets numbers assigned. Note that the behavior is not specified by the C standard, use 'signed char' or 'unsigned char' instead!");
                warned_arithmetic_usage = true;
            }
            target_val = (char) tmp;
            return true;
        }

        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.clear(); ss.str(str);
            ss >> (std::hex) >> tmp;
            if (ss.eof() && !ss.fail() && !ss.bad() && !warned_arithmetic_usage) {
                SC_REPORT_WARNING("deserialize", "You are using a 'char' parameter with numbers. Note that the behavior is not specified by the C standard, use 'signed char' or 'unsigned char' instead!");
                warned_arithmetic_usage = true;
            }
        }
        // if still error, try characters 'x'
        if (!ss.eof() || ss.fail() || ss.bad()) {
            if (str.length() == 3 && str[0] == '\'' && str.at(2) == '\'') {
                target_val = str.at(1);
                return true;
            }
        }
        // no conversion error
        if ((ss.eof() && !ss.fail() && !ss.bad()) && (tmp <= 127 && tmp >= -128)) {
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


// /////////////////////////////////////////////////////////////////////////////// //
// /////////////////// gs_config< UNSIGNED SHORT > //////////////////////////////// //
// /////////////////////////////////////////////////////////////////////////////// //

/// Template specialization for gs_config<unsigned short>.
/**
* Default value = 0.
*/
template<>
class gs_config<unsigned short>
    : public gs_config_t<unsigned short>
{
    /// Typedef for the value.
    typedef unsigned short val_type;

public:
    GS_CONFIG_HEAD;

    // ///////////////////////
    //  operators

    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;

    GC_SPECIALISATIONS_BINARY_OPERATORS;

    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("unsigned short");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_USHORT;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const{
        return static_convertValueToString(val);
    }  
    inline static std::string static_convertValueToString(const val_type &val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    /// Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
    inline static bool static_deserialize(val_type &target_val, const std::string& str) { 
        std::istringstream ss(str);
        GS_PARAM_DUMP_WITHNAME("gs_config", "deserialize: string='"<< ss.str().c_str()<<"'");
        if (ss.str().length() == 0) {
            target_val = 0;
            GS_PARAM_DUMP_WITHNAME("gs_config", "deserialize: set default value");
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(unsigned short) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("gs_config", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); /// @TODO(all): changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(unsigned short) stream eof and not fail"); 
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

