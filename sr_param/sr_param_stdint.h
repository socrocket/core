#include <stdint.h>

#if 0
/// Template specialization for sr_param<uint8_t>.
/// Default value = 0.
template<>
class sr_param<uint8_t>
    : public sr_param_t<uint8_t> {
    /// Typedef for the value.
    typedef uint8_t val_type;
  public:
    SR_PARAM_HEAD;
    // To resolve the correct = operator
    //using sr_param_t<val_type>::operator =;
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
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("uint8_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_UCHAR;
    }

    /// Overloads sr_param_t<T>::convertValueToString
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

/// Template specialization for sr_param<int8_t>.
/// Default value = 0.
template<>
class sr_param<int8_t>
    : public sr_param_t<int8_t> {
    /// Typedef for the value.
    typedef int8_t val_type;
  public:
    SR_PARAM_HEAD;

    // To resolve the correct = operator
    //using sr_param_t<val_type>::operator =;
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
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("int8_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_CHAR;
    }

    /// Overloads sr_param_t<T>::convertValueToString
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


/// Template specialization for sr_param<unsigned short>.
/// Default value = 0.
template<>
class sr_param<uint16_t>
    : public sr_param_t<uint16_t>
{
    /// Typedef for the value.
    typedef uint16_t val_type;

public:
    SR_PARAM_HEAD;
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("uint16_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_USHORT;
    }

    /// Overloads sr_param_t<T>::convertValueToString
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
        GS_PARAM_DUMP_WITHNAME("sr_param", "deserialize: string='"<< ss.str().c_str()<<"'");
        if (ss.str().length() == 0) {
            target_val = 0;
            GS_PARAM_DUMP_WITHNAME("sr_param", "deserialize: set default value");
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("sr_param", "(uint16_t) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("sr_param", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); // TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("sr_param", "(uint16_t) stream eof and not fail");
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

/// Template specialization for sr_param<int16_t>.
/// Default value = 0.
template<>
class sr_param<int16_t>
    : public sr_param_t<int16_t> {
    /// Typedef for the value.
    typedef int16_t val_type;
  public:
    SR_PARAM_HEAD;
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("int16_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_INT;
    }

    /// Overloads sr_param_t<T>::convertValueToString
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
        GS_PARAM_DUMP_WITHNAME("sr_param", "deserialize: string='"<< ss.str().c_str() << "'");
        if (ss.str().length() == 0) {
            target_val = 0;
            GS_PARAM_DUMP_WITHNAME("sr_param", "deserialize: set default value");
            return true;
        }
        val_type tmp;
        ss >> tmp;
        // if next char is a decimal point, ignore
        if (!ss.eof() && ss.peek() == '.') {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("sr_param", "(int16_t) ignored decimal point");
            return true;
        }
        // if error try hex
        if (!ss.eof() || ss.fail() || ss.bad()) {
            GS_PARAM_DUMP_WITHNAME("sr_param", "stream fail or not eof, try hex");
            ss.~istringstream();
            new ( (void *) &ss ) std::istringstream(str); // TODO: changed m_api->getParam(m_par_name) to str; correct??
            ss >> (std::hex) >> tmp;
        }
        // no conversion error
        if (ss.eof() && !ss.fail() && !ss.bad()) {
            target_val = tmp;
            GS_PARAM_DUMP_WITHNAME("sr_param", "(int16_t) stream eof and not fail");
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
/// Template specialization for sr_param<unsigned int>.
/// Default value = 0.
template<>
class sr_param<uint32_t>
    : public sr_param_t<uint32_t> {
    /// Typedef for the value.
    typedef uint32_t val_type;

public:
    SR_PARAM_HEAD;
    explicit sr_param(const std::string &nam, const int &val) : sr_param_t<val_type>(nam        )  { sr_param_t<uint32_t>::init((uint32_t)val);  }
    explicit sr_param(const char *nam,        const int &val) : sr_param_t<val_type>(std::string(nam))  { sr_param_t<uint32_t>::init((uint32_t)val);  }
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("uint32_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_UINT;
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

/// Template specialization for sr_param<int32_t>.
/// Default value = 0.
template<>
class sr_param<int32_t>
    : public sr_param_t<int32_t> {
    /// Typedef for the value.
    typedef int32_t val_type;
  public:
    SR_PARAM_HEAD;
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("int32_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_INT;
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

#if not defined(_WIN32)
/// Template specialization for sr_param<uint64_t>.
/// Default value = 0.
template<>
class sr_param<uint64_t>
    : public sr_param_t<uint64_t>
{
    /// Typedef for the value.
    typedef uint64_t val_type;

public:
    SR_PARAM_HEAD;
    /// Constructor with unsigned int value
    explicit sr_param(const std::string &nam, const unsigned int &val) : sr_param_t<uint64_t>(nam         )  { sr_param_t<uint64_t>::init((uint64_t)val); }
    explicit sr_param(const char *nam,        const unsigned int &val) : sr_param_t<uint64_t>(std::string(nam) )  { sr_param_t<uint64_t>::init((uint64_t)val); }
    explicit sr_param(const std::string nam,  const unsigned int &val, gs_param_array* parent_array) : sr_param_t<uint64_t>(nam, false, parent_array, true) { sr_param_t<uint64_t>::init(val);}
    explicit sr_param(const char *nam,        const unsigned int &val, gs_param_array* parent_array) : sr_param_t<uint64_t>(std::string(nam), false, parent_array, true) { sr_param_t<uint64_t>::init(val);}
    explicit sr_param(const std::string nam,  const unsigned int &val, gs_param_array& parent_array) : sr_param_t<uint64_t>(nam, false, &parent_array, true) { sr_param_t<uint64_t>::init(val);}
    explicit sr_param(const char *nam,        const unsigned int &val, gs_param_array& parent_array) : sr_param_t<uint64_t>(std::string(nam), false, &parent_array, true) { sr_param_t<uint64_t>::init(val);}

    explicit sr_param(const char *nam,        const unsigned long long int &val) : sr_param_t<uint64_t>(std::string(nam) )  { sr_param_t<uint64_t>::init((uint64_t)val); }
    explicit sr_param(const std::string nam,  const unsigned long long int &val, gs_param_array* parent_array) : sr_param_t<uint64_t>(nam, false, parent_array, true) { sr_param_t<uint64_t>::init(val);}
    explicit sr_param(const char *nam,        const unsigned long long int &val, gs_param_array* parent_array) : sr_param_t<uint64_t>(std::string(nam), false, parent_array, true) { sr_param_t<uint64_t>::init(val);}
    explicit sr_param(const std::string nam,  const unsigned long long int &val, gs_param_array& parent_array) : sr_param_t<uint64_t>(nam, false, &parent_array, true) { sr_param_t<uint64_t>::init(val);}
    explicit sr_param(const char *nam,        const unsigned long long int &val, gs_param_array& parent_array) : sr_param_t<uint64_t>(std::string(nam), false, &parent_array, true) { sr_param_t<uint64_t>::init(val);}
    /// Constructor with int value
    explicit sr_param(const std::string &nam, const int &val) : sr_param_t<uint64_t>(nam         )  { sr_param_t<uint64_t>::init((uint64_t)val); }
    explicit sr_param(const char *nam,        const int &val) : sr_param_t<uint64_t>(std::string(nam) )  { sr_param_t<uint64_t>::init((uint64_t)val); }
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("uint64_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_ULONGLONG;
    }

    /// Overloads sr_param_t<T>::convertValueToString
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
            GS_PARAM_DUMP_WITHNAME("sr_param", "(uint64_t) ignored decimal point");
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

/// Template specialization for sr_param<int64_t>.
/// Default value = 0.
template<>
class sr_param<int64_t>
    : public sr_param_t<int64_t> {
    /// Typedef for the value.
    typedef int64_t val_type;

public:
    SR_PARAM_HEAD;
    /// Constructor with int value
    explicit sr_param(const std::string &nam, const int &val) : sr_param_t<int64_t>(nam         )  { sr_param_t<int64_t>::init(val); }
    explicit sr_param(const char *nam,        const int &val) : sr_param_t<int64_t>(std::string(nam) )  { sr_param_t<int64_t>::init(val); }
    //  operators
    GC_SPECIALISATIONS_ARITHMETIC_OPERATORS;
    GC_SPECIALISATIONS_BINARY_OPERATORS;
    GC_SPECIALISATIONS_INCREMENT_OPERATORS;
    GC_SPECIALISATIONS_DECREMENT_OPERATORS;
    SR_PARAM_DELEGATE;

    /// Overloads sr_param::getTypeString
    const std::string getTypeString() const {
        return std::string("int64_t");
    }

    /// Overloads sr_param::getType
    gs::cnf::Param_type getType() const {
        return gs::cnf::PARTYPE_LONGLONG;
    }

    /// Overloads sr_param_t<T>::convertValueToString
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
            GS_PARAM_DUMP_WITHNAME("sr_param", "(int64_t) ignored decimal point");
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
#endif
