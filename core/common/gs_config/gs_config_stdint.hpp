#include <stdint.h>

#if 0
/// Template specialization for gs_config<uint8_t>.
/// Default value = 0.
template<>
class gs_config<uint8_t>
    : public gs_config_t<uint8_t> {
    /// Typedef for the value.
    typedef uint8_t val_type;
  public:
    GS_CONFIG_HEAD;
    // To resolve the correct = operator
    //using gs_config_t<val_type>::operator =;
    my_type& operator = (const val_type& v) { 
        //cout << "operator = (val_type) value="<<convertValueToString(v) <<endl;
        setValue(v);
        return *this;
    }

    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("uint8_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_UCHAR;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);
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
            target_val = (uint8_t) tmp;
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
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }
};

/// Template specialization for gs_config<int8_t>.
/// Default value = 0.
template<>
class gs_config<int8_t>
    : public gs_config_t<int8_t> {
    /// Typedef for the value.
    typedef int8_t val_type;
  public:
    GS_CONFIG_HEAD;

    // To resolve the correct = operator
    //using gs_config_t<val_type>::operator =;
    my_type& operator = (const val_type& v) { 
        //cout << "operator = (val_type) value="<<convertValueToString(v) <<endl;
        setValue(v);
        return *this;
    }

    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("int8_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_CHAR;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
        return static_convertValueToString(val);
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
            target_val = (int8_t) tmp;
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
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }
};


/// Template specialization for gs_config<unsigned short>.
/// Default value = 0.
template<>
class gs_config<uint16_t>
    : public gs_config_t<uint16_t>
{
    /// Typedef for the value.
    typedef uint16_t val_type;

public:
    GS_CONFIG_HEAD;
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("uint16_t");
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
            GS_PARAM_DUMP_WITHNAME("gs_config", "(uint16_t) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("gs_config", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); // TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(uint16_t) stream eof and not fail"); 
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }    
};

#endif  // 0

/// Template specialization for gs_config<int16_t>.
/// Default value = 0.
template<>
class gs_config<int16_t>
    : public gs_config_t<int16_t> {
    /// Typedef for the value.
    typedef int16_t val_type;
  public:
    GS_CONFIG_HEAD;
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("int16_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_INT;
    }

    /// Overloads gs_config_t<T>::convertValueToString
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
        GS_PARAM_DUMP_WITHNAME("gs_config", "deserialize: string='"<< ss.str().c_str() << "'");
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
            GS_PARAM_DUMP_WITHNAME("gs_config", "(int16_t) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("gs_config", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); // TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(int16_t) stream eof and not fail"); 
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }    

};

#if 0
/// Template specialization for gs_config<unsigned int>.
/// Default value = 0.
template<>
class gs_config<uint32_t>
    : public gs_config_t<uint32_t> {
    /// Typedef for the value.
    typedef uint32_t val_type;

public:
    GS_CONFIG_HEAD;
    explicit gs_config(const std::string &nam, const int &val) : gs_config_t<val_type>(nam        )  { gs_config_t<uint32_t>::init((uint32_t)val);  }
    explicit gs_config(const char *nam,        const int &val) : gs_config_t<val_type>(string(nam))  { gs_config_t<uint32_t>::init((uint32_t)val);  }
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("uint32_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_UINT;
    }

    /// Overloads gs_param_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
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
            GS_PARAM_DUMP_WITHNAME("gs_param", "(uint32_t) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); // TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_param", "(uint32_t) stream eof and not fail"); 
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }

};

/// Template specialization for gs_config<int32_t>.
/// Default value = 0.
template<>
class gs_config<int32_t>
    : public gs_config_t<int32_t> {
    /// Typedef for the value.
    typedef int32_t val_type;
  public:
    GS_CONFIG_HEAD;
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("int32_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_INT;
    }

    /// Overloads gs_param_t<T>::convertValueToString
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
            GS_PARAM_DUMP_WITHNAME("gs_param", "(int32_t) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("gs_param", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); // TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_param", "(int32_t) stream eof and not fail"); 
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }    

};

#endif  // 0

/// Template specialization for gs_config<uint64_t>.
/// Default value = 0.
template<>
class gs_config<uint64_t>
    : public gs_config_t<uint64_t>
{
    /// Typedef for the value.
    typedef uint64_t val_type;

public:
    GS_CONFIG_HEAD;
    /// Constructor with unsigned int value
    explicit gs_config(const std::string &nam, const unsigned int &val) : gs_config_t<uint64_t>(nam         )  { gs_config_t<uint64_t>::init((uint64_t)val); }    
    explicit gs_config(const char *nam,        const unsigned int &val) : gs_config_t<uint64_t>(string(nam) )  { gs_config_t<uint64_t>::init((uint64_t)val); }    
    explicit gs_config(const std::string nam,  const unsigned int &val, gs_param_array* parent_array) : gs_config_t<uint64_t>(nam, false, parent_array, true) { gs_config_t<uint64_t>::init(val);}
    explicit gs_config(const char *nam,        const unsigned int &val, gs_param_array* parent_array) : gs_config_t<uint64_t>(string(nam), false, parent_array, true) { gs_config_t<uint64_t>::init(val);}
    explicit gs_config(const std::string nam,  const unsigned int &val, gs_param_array& parent_array) : gs_config_t<uint64_t>(nam, false, &parent_array, true) { gs_config_t<uint64_t>::init(val);}
    explicit gs_config(const char *nam,        const unsigned int &val, gs_param_array& parent_array) : gs_config_t<uint64_t>(string(nam), false, &parent_array, true) { gs_config_t<uint64_t>::init(val);}

    explicit gs_config(const char *nam,        const unsigned long long int &val) : gs_config_t<uint64_t>(string(nam) )  { gs_config_t<uint64_t>::init((uint64_t)val); }    
    explicit gs_config(const std::string nam,  const unsigned long long int &val, gs_param_array* parent_array) : gs_config_t<uint64_t>(nam, false, parent_array, true) { gs_config_t<uint64_t>::init(val);}
    explicit gs_config(const char *nam,        const unsigned long long int &val, gs_param_array* parent_array) : gs_config_t<uint64_t>(string(nam), false, parent_array, true) { gs_config_t<uint64_t>::init(val);}
    explicit gs_config(const std::string nam,  const unsigned long long int &val, gs_param_array& parent_array) : gs_config_t<uint64_t>(nam, false, &parent_array, true) { gs_config_t<uint64_t>::init(val);}
    explicit gs_config(const char *nam,        const unsigned long long int &val, gs_param_array& parent_array) : gs_config_t<uint64_t>(string(nam), false, &parent_array, true) { gs_config_t<uint64_t>::init(val);}
    /// Constructor with int value
    explicit gs_config(const std::string &nam, const int &val) : gs_config_t<uint64_t>(nam         )  { gs_config_t<uint64_t>::init((uint64_t)val); }    
    explicit gs_config(const char *nam,        const int &val) : gs_config_t<uint64_t>(string(nam) )  { gs_config_t<uint64_t>::init((uint64_t)val); }    
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("uint64_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_ULONGLONG;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
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
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(uint64_t) ignored decimal point");
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
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }

};

/// Template specialization for gs_config<int64_t>.
/// Default value = 0.
template<>
class gs_config<int64_t>
    : public gs_config_t<int64_t> {
    /// Typedef for the value.
    typedef int64_t val_type;

public:
    GS_CONFIG_HEAD;
    /// Constructor with int value
    explicit gs_config(const std::string &nam, const int &val) : gs_config_t<int64_t>(nam         )  { gs_config_t<int64_t>::init(val); }    
    explicit gs_config(const char *nam,        const int &val) : gs_config_t<int64_t>(string(nam) )  { gs_config_t<int64_t>::init(val); }    
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    GS_CONFIG_DELEGATE;

    /// Overloads gs_config::getTypeString
    const std::string getTypeString() const {
        return string("int64_t");
    }

    /// Overloads gs_config::getType
    const Param_type getType() const {
        return PARTYPE_LONGLONG;
    }

    /// Overloads gs_config_t<T>::convertValueToString
    std::string convertValueToString(const val_type &val) const {
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
        if (ss.str().length() == 0) {
            target_val = 0;
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("gs_config", "(int64_t) ignored decimal point");
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
        } else {
            std::stringstream ess;
            ess << "Conversion error: '" << str << "'";
            SC_REPORT_WARNING("deserialize", ess.str().c_str());
            return false;
        }
        return true;
    }
};
