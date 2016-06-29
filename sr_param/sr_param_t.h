// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sr_param_t.h
/// 
///
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///
#ifndef SR_PARAM_T_H_
#define SR_PARAM_T_H_

// included by gs_params.h
// #include "gs_param_base" is done there
//#include "config.h"

/// Template specialized base class for configuration parameters.
template <typename T>
class sr_param_t : public sr_param_base {
    /// Typedef for this specialized class.
    typedef sr_param_t<T> my_type;

    /// Typedef for the value.
    typedef T val_type;

  protected:
    /// Value of this parameter
    val_type my_value;

    /// String whose reference can be returned as string value
    mutable std::string return_string;

  public:
    using gs_param_base::set;

    // templated operators
    my_type& operator +=  (val_type);
    my_type& operator -=  (val_type);
    my_type& operator /=  (val_type);
    my_type& operator *=  (val_type);
    val_type operator +  (val_type);
    val_type operator -  (val_type);
    val_type operator /  (val_type);
    val_type operator *  (val_type);
    my_type& operator %=  (val_type);
    my_type& operator ^=  (val_type);
    my_type& operator &=  (val_type);
    my_type& operator |=  (val_type);
    my_type& operator <<= (val_type);
    my_type& operator >>= (val_type);

    // templated decrement and increment
    my_type& operator -- ();    // prefix
    val_type operator -- (int); // postfix
    my_type& operator ++ ();    // prefix
    val_type operator ++ (int); // postfix


    // //////////////////////////////////////////////////////////////////// //
    // //////////////   constructors and destructor   ///////////////////// //


    /// Empty constructor. Name will be set in base. Avoid using it! You MUST call init() after instantiation!
    /**
    */
    explicit sr_param_t()
        : sr_param_base("", true, NULL, false), m_mirror_param(NULL), m_mirror_val(NULL)
    {  }

    /// Constructor with the special parameters. Name will be set in base. Avoid using it! You MUST call init() after instantiation!
    /**
    * @param register_at_db  see gs::cnf::gs_param_base, default = true.
    * @param parent_array    see gs::cnf::gs_param_base, default=NULL.
    * @param force_top_level_name If the given name n should be a top-level name (then no prefeix is attached to the name),
    *                             default false,
    *                             be carefull in using this.
    */
    explicit sr_param_t(const bool force_top_level_name, gs_param_array* parent_array, const bool register_at_db)
        : sr_param_base("", register_at_db, parent_array, force_top_level_name), m_mirror_param(NULL), m_mirror_val(NULL)
    {  }

    /// Constructor with (local or hierarchical) name. You MUST call init() after instantiation!
    /**
    * This constructor may be called with local or hierarchical name.
    *
    * Explicit constructor to avoid implicit construction of parameters.
    *
    * gs_param MUST call init() or init(value) after construction to
    * add the parameter to the plugin database!!!
    *
    * @param nam  The local (or full hierarchical) parameter name (local: not including points)
    *             (local: unique inside a module, hierarchical: unique in the system).
    */
    explicit sr_param_t(const std::string &nam)
        : sr_param_base(nam, true, NULL, false), m_mirror_param(NULL), m_mirror_val(NULL)
    {  }

    /// Constructor with (local or hierarchical) name and special parameters. You MUST call init() after instantiation!
    /**
    * This constructor may be called with local or hierarchical name.
    *
    * Explicit constructor to avoid implicit construction of parameters.
    *
    * gs_param MUST call init() or init(value) after construction to
    * add the parameter to the plugin database!!!
    *
    * @param nam  The local (or full hierarchical) parameter name (local: not including points)
    *             (local: unique inside a module, hierarchical: unique in the system).
    * @param register_at_db  see gs::cnf::gs_param_base, default = true.
    * @param parent_array    see gs::cnf::gs_param_base, default=NULL.
    * @param force_top_level_name If the given name n should be a top-level name (then no prefeix is attached to the name),
    *                             default false,
    *                             be carefull in using this.
    */
    explicit sr_param_t(const std::string &nam, const bool force_top_level_name,
        gs_param_array* parent_array, const bool register_at_db)
        : sr_param_base(nam, register_at_db, parent_array, force_top_level_name), m_mirror_param(NULL), m_mirror_val(NULL)
    {  }

    /// Init method to set the value and add the parameter to the plugin db.
    /**
    * Has to be called by the gs_param class after construction!
    * Does set the value and add the parameter to the plugin db.
    */
    void init(const val_type &default_val) {
        GS_PARAM_DUMP("Init sr_param_t "<< m_par_name.c_str());
        // set my_value
        my_value = default_val;
        setProperty("default", convertValueToString(my_value));
        // add to plugin database
        if (m_register_at_db) {
            assert(m_api != NULL);
            m_api->addPar(this);
        }
    }
    /// Init method without value @see gs::cnf::gs_param_t::init(val_type &default_val)
    void init() {
        GS_PARAM_DUMP("Init sr_param_t "<< m_par_name.c_str());
        my_value = convertStringToValue(std::string(""));
        setProperty("default", convertValueToString(my_value));
        // add to plugin database
        if (m_register_at_db) {
            assert(m_api != NULL);
            m_api->addPar(this);
        }
    }

    /// Destructor
    virtual ~sr_param_t() {
    }


    // //////////////////////////////////////////////////////////////////// //
    // ///////   set and get with value   ///////////////////////////////// //


    /// Set the value of this parameter to the value of another gs_param.
    /**
    * To resolve the correct = operator in the (not specialized) class
    * which inherits from this class has to use
    * \code using gs_param_t<val_type>::operator =; \endcode
    *
    * @param v  Parameter where the value should be read from.
    * @return   Pointer to this.
    */
    my_type& operator = (const my_type& v) {
        setValue(v.getValue()); //m_api->setParam(m_par_name, const_cast<my_type&>(v).get());
        return *this;
    }

    /// Set the value of this parameter.
    /**
    * To resolve the correct = operator in the (not specialized) class
    * which inherits from this class has to use
    * \code using gs_param_t<val_type>::operator =; \endcode
    *
    * @param v  Value which has to be set.
    * @return   Pointer to this.
    */
    my_type& operator = (const val_type& v) {
        setValue(v);
        return *this;
    }

    /// Get the value of this parameter.
    /**
    * @return Value of the parameter.
    */
    operator const val_type& () const {
        return getValue();
    }

    /// Set the value of this parameter.
    /**
    * Calls conversion value type --> string.
    *
    * @param val  The new value for this parameter.
    */
    void setValue(const val_type &val) {
        make_pre_write_callbacks();
        my_value = val;
        make_post_write_callbacks();
    }

    /// Returns the value of this parameter.
    /**
    * @return Value
    */
    const val_type& getValue() const {
        make_pre_read_callbacks();
        //make_post_read_callbacks(); // TODO not possible here, returning tmp object would change behavior!
        return my_value;
    }

    /// @see gs::cnf::gs_param_base::get_value_pointer
    const void* get_value_pointer() const{
        make_pre_read_callbacks();
        //make_post_read_callbacks(); // TODO not possible here, returning tmp object would change behavior!
        return &my_value;
    }

    // //////////////////////////////////////////////////////////////////// //
    // ///////   set and get with string representation   ///////////////// //


    /// Set the value of this parameter with a string.
    /**
    * @param str  The new value for this parameter, represented as a string.
    * @return If setting was successful
    */
    bool setString(const std::string &str) {
        make_pre_write_callbacks();
        bool success = deserialize(my_value, gs::cnf::envvar_subst(str, m_par_name));
        make_post_write_callbacks();
        return success;
    }

    /// Get the value of this parameter as a string.
    /**
    * @return String representation of the current value of this parameter.
    */
    const std::string& getString() const {
        make_pre_read_callbacks();
        return_string = convertValueToString(my_value);
        //make_post_read_callbacks(); // TODO possible but deactivated
        return return_string;
    }

    // //////////////////////////////////////////////////////////////////// //
    // ///////   virtual conversion methods string <-> value   //////////// //


    /// Conversion method value type --> string. To be implemented by the specialization.
    /**
    * May not make use of m_par_name because it is called inside constructor!
    *
    * @param val  Value that should be converted to string.
    * @return String representation of the value.
    */
    virtual std::string convertValueToString(const val_type &val) const = 0;

    /// Deserialize for this parameter. To be implemented by the specialization.
    /**
    * Conversion string --> value type.
    *
    * User implemented for each template specialization of gs_param:
    * Do not write to target_val if deserialization fails!
    *
    * Set target_val to the default value if str is empty (=="").
    *
    * Do not use ANY member variables. Act as if the method is static.
    *
    * @param  target_val  Reference to the value that should be set.
    * @param  str         String that should be converted to a value.
    * @return If the convertion was successfull
    */
    virtual const bool deserialize(val_type &target_val, const std::string& str) = 0;

    /// Serialize this parameter.
    /** Uses the convertValueToString method */
    //void serialize(const val_type &val) {
    //  convertValueToString(val);
    //}

    void mirror(T &original) {
      if(!m_mirror_param && !m_mirror_val) {
        this->registerParamCallback(new ::gs::cnf::ParamTypedCallbAdapt<sr_param_t<T> >(this, &sr_param_t<T>::mirror_callback, this, this), gs::cnf::pre_read);
        this->registerParamCallback(new ::gs::cnf::ParamTypedCallbAdapt<sr_param_t<T> >(this, &sr_param_t<T>::mirror_callback, this, this), gs::cnf::post_write);
      }
      this->setProperty("mirrors", "variable");
      m_mirror_val = &original;
    }
    void mirror(sr_param_t<T> &original) {
      if(!m_mirror_param && ! m_mirror_val) {
        this->registerParamCallback(new ::gs::cnf::ParamTypedCallbAdapt<sr_param_t<T> >(this, &sr_param_t<T>::mirror_callback, this, this), gs::cnf::pre_read);
        this->registerParamCallback(new ::gs::cnf::ParamTypedCallbAdapt<sr_param_t<T> >(this, &sr_param_t<T>::mirror_callback, this, this), gs::cnf::post_write);
      }
      this->setProperty("mirrors", original.getName());
      m_mirror_param = &original;
    }
protected:
    gs::cnf::callback_return_type mirror_callback(
        gs::gs_param_base& changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason) {
      if(m_mirror_param) {
        if(reason == gs::cnf::pre_read) {
          this->setValue(m_mirror_param->getValue());
        } else if(reason == gs::cnf::post_write) {
          m_mirror_param->setValue(this->getValue());
        }
      }
      if(m_mirror_val) {
        if(reason == gs::cnf::pre_read) {
          this->setValue(m_mirror_val->getValue());
        } else if(reason == gs::cnf::post_write) {
          m_mirror_val->setValue(this->getValue());
        }
      }
      return GC_RETURN_OK;
    }

    sr_param_t<T> *m_mirror_param;
    T *m_mirror_val;

    /// Get the value the string. Needed for construction of gs_param.
    /**
    * Conversion string --> value type.
    *
    * @param  str String that should be converted to a value
    * @return If the convert was successfull
    */
    const val_type& convertStringToValue(const std::string& str) {
        deserialize(my_value, gs::cnf::envvar_subst(str, m_par_name));
        return my_value;
    }

};

#endif  // SR_PARAM_T_H_
/// @}
