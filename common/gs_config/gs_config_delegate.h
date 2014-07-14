#ifndef COMMON_GS_CONFIG_DELEGATE_H_
#define COMMON_GS_CONFIG_DELEGATE_H_

#define GS_CONFIG_DELEGATE \
template<T> \
class gs_config_delegate<T> { \
  public: \
 \
    gs_config_delegate(std::string name) { \
       param = new gs_param<T>(name); \
    } \
 \
    ~gs_config_delegate() { \
      GC_UNREGISTER_CALLBACKS(); \
    } \
 \
    /* Static power callback */ \
    gs::cnf::callback_return_type callback( \
        gs::gs_param_base& changed_param,  /* NOLINT(runtime/references) */ \
        gs::cnf::callback_type reason) { \
      param.setValue(changed_param.getValue); \
      return GC_RETURN_OK; \
    } \
 \
  private: \
    gs_param<T> *param; \
}; \
gs_config_delegate *delegate(std::string name) { \
    gs_config_delegate *delegate = new gs_config_delegate(name); \
    GC_REGISTER_TYPED_PARAM_CALLBACK(this, gs::cnf::post_write, gs_config_delegate, callback); \
    return delegate; \
}



#endif  // COMMON_GS_CONFIG_DELEGATE_H
