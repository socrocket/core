#ifndef SR_PARAM_DELEGATE_H_
#define SR_PARAM_DELEGATE_H_

#define SR_PARAM_DELEGATE \
class sr_param_delegate { \
  public: \
    GC_HAS_CALLBACKS(); \
 \
    sr_param_delegate(gs::cnf::gs_param_base *orig, std::string name) { \
       param = new gs::cnf::gs_param<val_type>(name); \
       GC_REGISTER_TYPED_PARAM_CALLBACK(orig, gs::cnf::post_write, sr_param_delegate, callback); \
    } \
 \
    ~sr_param_delegate() { \
      GC_UNREGISTER_CALLBACKS(); \
      delete param; \
    } \
 \
    /* Static power callback */ \
    gs::cnf::callback_return_type callback( \
        gs::cnf::gs_param_base& changed_param,  /* NOLINT(runtime/references) */ \
        gs::cnf::callback_type reason) { \
      val_type value; \
      changed_param.getValue(value); \
      param->setValue(value); \
      return GC_RETURN_OK; \
    } \
 \
  private: \
           gs::cnf::gs_param<val_type> *param; \
}; \
sr_param_delegate *delegate(std::string name) { \
    return new sr_param_delegate(this, name); \
}

#endif  // SR_PARAM_DELEGATE_H
