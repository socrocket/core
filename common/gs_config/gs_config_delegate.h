#ifndef COMMON_GS_CONFIG_DELEGATE_H_
#define COMMON_GS_CONFIG_DELEGATE_H_

#define GS_CONFIG_DELEGATE \
class gs_config_delegate { \
  public: \
    GC_HAS_CALLBACKS(); \
 \
    gs_config_delegate(gs_param_base *orig, std::string name) { \
       param = new gs_param<val_type>(name); \
       GC_REGISTER_TYPED_PARAM_CALLBACK(orig, gs::cnf::post_write, gs_config_delegate, callback); \
    } \
 \
    ~gs_config_delegate() { \
      GC_UNREGISTER_CALLBACKS(); \
      delete param; \
    } \
 \
    /* Static power callback */ \
    gs::cnf::callback_return_type callback( \
        gs::gs_param_base& changed_param,  /* NOLINT(runtime/references) */ \
        gs::cnf::callback_type reason) { \
      val_type value; \
      changed_param.getValue(value); \
      param->setValue(value); \
      return GC_RETURN_OK; \
    } \
 \
  private: \
    gs_param<val_type> *param; \
}; \
gs_config_delegate *delegate(std::string name) { \
    return new gs_config_delegate(this, name); \
}



#endif  // COMMON_GS_CONFIG_DELEGATE_H
