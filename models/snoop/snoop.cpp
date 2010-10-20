#include "amba.h"

//NOTE the inclusion below!
#include "greensocket/monitor/green_socket_observer_base.h"


/*
REMARK:
Coded in a quick and dirty style. Tried not to make it too complex.
Hope it helps. Drop mails if assistance is needed.
*/

/*
Okay. First a simple master and slave that exchange stuff.
They print some info to see the txn and phase is transmitted correctly.
*/

SC_MODULE(master)
{

 amba::amba_master_socket<> socket;

 SC_CTOR(master)
   //play around with the bus type and abstraction layer to see that the configs are correctly exchanged through the probe
   : socket("socket", amba::amba_AXI, amba::amba_AT, false) 
 {
   SC_THREAD(run);
   socket.register_nb_transport_bw(this, &master::nb_trans);
   gs::socket::timing_info my_info;
   my_info.set_start_time(tlm::BEGIN_REQ, sc_core::sc_time(3,sc_core::SC_PS)); //just to have some non-default timing
   socket.set_initiator_timing(my_info); //tell our socket about our timing
   //if the probe doesn't suck, it should arrive at the slave
 }

 void run()
 {
   tlm::tlm_generic_payload* p_gp=socket.get_transaction();
   tlm::tlm_phase ph=tlm::BEGIN_REQ;
   sc_core::sc_time t;
   //add/play around with extensions here if you wanna see them dumped
   socket.validate_extension<amba::amba_imprecise_burst>(*p_gp);
   socket.validate_extension<amba::amba_burst_size>(*p_gp);
   amba::amba_burst_size* size;
   socket.get_extension<amba::amba_burst_size>(size, *p_gp);
   size->value=42;

   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" SEND "<<ph<<" with gp ptr="<<p_gp<<" and time="<<t<<std::endl;
   std::string ret;
   switch(socket->nb_transport_fw(*p_gp,ph,t))
   {
     case tlm::TLM_ACCEPTED: ret="TLM_ACCEPTED"; break;
     case tlm::TLM_UPDATED: ret="TLM_UPDATED"; break;
     case tlm::TLM_COMPLETED: ret="TLM_COMPLETED"; break;
   }
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" target returned "<<ret<<std::endl;
 }

 tlm::tlm_sync_enum nb_trans(tlm::tlm_generic_payload& gp, tlm::tlm_phase& ph, sc_core::sc_time& t)
 {
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" GOT "<<ph<<" with gp ptr="<<&gp<<" and time="<<t<<std::endl;
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" returning TLM_COMPLETED"<<std::endl;
   socket.release_transaction(&gp);
   return tlm::TLM_COMPLETED;
 }


};

SC_MODULE(slave)
{

 amba::amba_slave_socket<> socket;

 SC_CTOR(slave)
   : socket("socket", amba::amba_AXI, amba::amba_AT, false)
 {
   socket.set_timing_listener_callback(this, &slave::time_cb);
   SC_METHOD(run);
   sensitive<<m_event;
   dont_initialize();
   socket.register_nb_transport_fw(this, &slave::nb_trans);
 }

 void run()
 {
   tlm::tlm_phase ph=tlm::BEGIN_RESP;
   sc_core::sc_time t(20,sc_core::SC_NS);
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" SEND "<<ph<<" with gp ptr="<<m_p_gp<<" and time="<<t<<std::endl;
   std::string ret;
   switch(socket->nb_transport_bw(*m_p_gp,ph,t))
   {
     case tlm::TLM_ACCEPTED: ret="TLM_ACCEPTED"; break;
     case tlm::TLM_UPDATED: ret="TLM_UPDATED"; break;
     case tlm::TLM_COMPLETED: ret="TLM_COMPLETED"; break;
   }
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" init returned "<<ret<<std::endl;

 }

 tlm::tlm_sync_enum nb_trans(tlm::tlm_generic_payload& gp, tlm::tlm_phase& ph, sc_core::sc_time& t)
 {
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" GOT "<<ph<<" with gp ptr="<<&gp<<" and time="<<t<<std::endl;
   m_p_gp=&gp;
   ph=tlm::END_REQ;
   std::cout<<sc_core::sc_time_stamp()<<" "<<sc_module::name()<<" returning TLM_UPDATED:"<<ph<<std::endl;

   //add/play around with extensions here if you wanna see them dumped
   socket.invalidate_extension<amba::amba_burst_size>(gp); //inval bsize here to see that it disapears from probe output

   m_event.notify(t+sc_core::sc_time(10,sc_core::SC_NS));
   return tlm::TLM_UPDATED;
 }

 void time_cb(gs::socket::timing_info master_info)
 {
   std::cout<<"master sent timing info !"<<std::endl;
   std::cout<<" time for BEGIN_REQ is "<<master_info.get_start_time(tlm::BEGIN_REQ)<<std::endl;
 }

 tlm::tlm_generic_payload* m_p_gp;
 sc_core::sc_event m_event;

};

/*
Now the probe. Once in a while GreenSocket has advantages:
 It already comes with a probing facility which you can
 use. Just do as below and implement the four virtual functions.
 Then you have your probe.
 Since the virtual functions provide everything you need to re-assemble the
 original call, it should be not too hard to hook that into some TLM probes
 you already have. Note that the arguments you get are const, because I don't
 want to allow people to mess up the transmitted info in a probe.
 However, that does not work for the payload, because it is legitimate for a
 probe to add some extensions. So the payload cannot be const. Yuk.
*/


//The probe here has just the crappy BUSWIDTH template arg. Nothing else
template<unsigned int BUSWIDTH> 
struct amba_probe 
 : gs::socket::monitor<BUSWIDTH>
 , gs::socket::gp_observer_base_t<tlm::tlm_base_protocol_types>
{
 amba_probe()
   :  gs::socket::monitor<BUSWIDTH>(sc_core::sc_gen_unique_name("amba_probe"))
   ,  gs::socket::gp_observer_base_t<tlm::tlm_base_protocol_types>((gs::socket::monitor<BUSWIDTH>*)this)
 {}

 //small helper to avoid repeated "calculations" of the name
 std::string& get_my_name()
 {
   if (!m_name.size())
   {
     std::cout<<"create name"<<std::endl;
     m_name="AMBA Probe for the link between ";
     //the connected init is returned to us as a bindability base ptr. 
     // if you need something else dyn cast the thing into what you need
     // ATTENTION: if one of the connected sockets is no greensocket the bindability base ptr would be NULL
     //            so you better check for that before you call seomthing like ->get_name() (but in this example we don't check)
     m_name+=gs::socket::gp_observer_base::get_connected_initiator()->get_name(); 
     m_name+=" and ";
     m_name+=gs::socket::gp_observer_base::get_connected_target()->get_name();
   }
   return m_name;
 }

 //helper for dumping extensions
 void dump_exts(tlm::tlm_generic_payload& txn)
 {
   std::cout<<"Let's check the extensions "<<std::endl;
   //And now something that I think is another advantage of greensocket (and its extensions): you can introspect the extensions like that
   for (unsigned int i=0; i<tlm::max_num_extensions(); i++)
   {
     if (txn.get_extension(i) && gs::ext::extension_cast()[i]) //there is an extension and it is a greensocket style one
     {
       gs::ext::gs_extension_base* tmp=gs::ext::extension_cast()[i](txn.get_extension(i));
       assert(tmp);
       unsigned int potential_guard_id;
       switch(tmp->get_type(potential_guard_id)){
         case gs::ext::gs_data:
           std::cout<<"   Found a GS extension named: "<<tmp->get_name()<<" and the content is "<<tmp->dump()<<std::endl;
           break;
         case gs::ext::gs_guarded_data:
           if (txn.get_extension(potential_guard_id) && tmp->is_valid()) //only dump if guard is there
             std::cout<<"   Found a GS extension named: "<<tmp->get_name()<<" and the content is "<<tmp->dump()<<std::endl;
           break;
         case gs::ext::gs_array_guard:; //don't ask what that is. Just ignore it.
       }
     }
     else
     if (txn.get_extension(i))
     {
       std::cout<<"   There is a plain TLM extension. All I know is its ID: "<<i<<std::endl;
     }
   }
   std::cout<<"    Done checking the extensions "<<std::endl;  
 }

 //we get this one after nb_transport is called but before it reaches the callee
 virtual void nb_call_callback(bool fwNbw, tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase, const sc_core::sc_time& time)
 {
   //now some "analysis" to see it works
   std::cout<<get_my_name()<<": at "<<sc_core::sc_time_stamp()<<" I see a CALL to:"<<std::endl
            <<" nb_transport_"<<(fwNbw?"fw":"bw")<<std::endl
            <<"   the payload ptr is "<<&txn<<std::endl
            <<"   the phase is "<<phase<<std::endl
            <<"   the time is "<<time<<std::endl;
   dump_exts(txn);
 }

 //we get this one after nb_transport has returned from the callee but before it returns  at the caller side
 virtual void nb_return_callback(bool fwNbw, tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase, const sc_core::sc_time& time, tlm::tlm_sync_enum retVal)
 {
   std::cout<<get_my_name()<<": at "<<sc_core::sc_time_stamp()<<" I see a RETURN from:"<<std::endl
            <<" nb_transport_"<<(fwNbw?"fw":"bw")<<std::endl
            <<"   the payload ptr is "<<&txn<<std::endl
            <<"   the phase is "<<phase<<std::endl
            <<"   the time is "<<time<<std::endl
            <<"   the return value is "<<((retVal==tlm::TLM_ACCEPTED)?"TLM_ACCEPTED":(retVal==tlm::TLM_UPDATED)?"TLM_UPDATED":"TLM_COMPLETED")<<std::endl;
   dump_exts(txn);
 }

 //for this example I do not care about b_transport. Works like the nb_trans_callbacks above.
 virtual void b_call_callback(tlm::tlm_generic_payload& txn, const sc_core::sc_time& time){}
 virtual void b_return_callback(tlm::tlm_generic_payload& txn, const sc_core::sc_time& time){}

 std::string m_name;

};

int sc_main(int, char**)
{
 master m("m");
 slave  s("s");

 amba_probe<32> probe1;

 //remove the probe and the only diff in output
 // should be the output created by the probe.
 // the transmitted info should not change

 /*BTW: in the ouput (as generated by the undmodified example)
   you will see that in the last output from the probe
   NO extensions are found. The reason is that the master
   releases the transaction within the nb_trans_bw call.
   And the slave had not acquired the txn. So the ref count
   hits zero before the call returns and all extensions
   drop off. So the observed output is correct!
 */

 m.socket(probe1.t_socket);
 probe1.i_socket(s.socket);

 sc_core::sc_start();
 return 0;
}
