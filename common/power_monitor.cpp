//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      power_monitor.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Power monitor class
//             
//             
//             
// Modified on $Date: 2011-06-10 12:14:49 +0200 (Fri, 10 Jun 2011) $
//          at $Revision: 458 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Etienne Kleine
// Reviewed:
//*********************************************************************


#include "power_monitor.h"

using namespace std;

//================================================

// standard-constructor
//------------------------------------------------
PM::PM(void){}
//------------------------------------------------

// destructor
//------------------------------------------------
PM::~PM(void){}
//------------------------------------------------

// data
//------------------------------------------------
vector<PowerEntry> PM::MainData;
vector<IpPowerEntry> PM::IpData;
vector< vector<analyzedEntry> > PM::AnalyzedData;

uint32_t PM::debug = 1;
uint32_t PM::maxLevel = 0;
uint64_t PM::EndOfSimulation = 0;

bool PM::LimitedRegion = 0;
uint64_t PM::start_log = 0;
uint64_t PM::end_log;

vector<string> PM::missingIp;
vector<string> PM::missingAction;
//------------------------------------------------

//================================================


// register IP
//------------------------------------------------
void PM::registerIP(sc_module* ip, string name, bool active){
  
  if (active){
    
    string sc_name = ip-> name();
    sc_name = "main."+sc_name;
    
    IpPowerEntry temp; 
    
    temp.ip=ip;
    temp.sc_name=sc_name;
    temp.name=name;
    
    unsigned int level = 0;
    
    // split name, determine level
    size_t pos = sc_name.find_last_of(".");
    while( static_cast<int>(pos) != -1 ){
      level++;
      sc_name = sc_name.substr(0,pos);
      pos = sc_name.find_last_of(".");
    }
    
    
    temp.level = level;                                  // instance level
    if ( level > PM::maxLevel ){ PM::maxLevel = level; } // global maximum level
    
    // add new IP to datavector
    PM::IpData.push_back(temp);
    
    v::info << "Power Monitor" << "IP " << temp.sc_name << " registered at level " << temp.level << endl;
  }

} // end register
//------------------------------------------------


// limit time region logged
//------------------------------------------------
void PM::limit_region(sc_time start, sc_time end){
  PM::LimitedRegion = 1;
  PM::start_log = start.value();
  PM::end_log = end.value();
}
//------------------------------------------------


// send data
// called by IP 
//------------------------------------------------
void PM::send(sc_module* ip, string action, bool start, sc_time timestamp, unsigned int id, bool active){

  if (PM::LimitedRegion == 1){
    if ( (timestamp.value() < PM::start_log) || (timestamp.value() > PM::end_log) ){
      active = 0;
    }
  }

  if (active){

    // summarize data in struct
    IpPowerData data;
    data.action = action;
    data.start = start;
    data.timestamp = timestamp.value();
    data.power = 0;
    data.id = id;
    
    if( data.timestamp > PM::EndOfSimulation ){ PM::EndOfSimulation = data.timestamp; }
    
    // search IP in IpData
    vector<IpPowerEntry>::iterator i=PM::IpData.begin();
    while( (i != PM::IpData.end()) && (ip != i->ip) ){ i++; }
    
    // check if IP registered and insert data
    if( i==PM::IpData.end() ){
      v::warn << "Power Monitor" << "sending IP not registered" << endl;
    }else{
      i->entry.push_back(data);
    }

  }
} // send 
//------------------------------------------------


// send idle date
// called by IP
//------------------------------------------------
void PM::send_idle(sc_module* ip, string action, sc_time timestamp, bool active){

  if (PM::LimitedRegion == 1 && (timestamp.value() > PM::end_log) ){
      active = 0;
  }

  if (active){

    // summarize and complete data in struct
    IpPowerData data;
    data.action = action;
    data.start = 1;
    data.timestamp = timestamp.value();
    data.power = 0;
    data.id = 0;

    if( data.timestamp > PM::EndOfSimulation ){ PM::EndOfSimulation = data.timestamp; }

    // search IP in IpData
    vector<IpPowerEntry>::iterator i=PM::IpData.begin();
    while( (i != PM::IpData.end()) && (ip != i->ip) ){ i++; }

    // check if IP registered and insert data
    if( i==PM::IpData.end() ){
      v::warn << "Power Monitor" << " sending IP not registered" << endl;
    }else{
      if( PM::LimitedRegion && (data.timestamp < PM::start_log) ){
	i->idle.clear();
	data.timestamp = PM::start_log;
	i->idle.push_back(data);
      }else{
	i->idle.push_back(data);
      }
    }

  }

} // send_idle
//------------------------------------------------


// read main data from file
//------------------------------------------------
void PM::readdata(string const &path, string const &infile){
  
  // declaration
  //..............................................
  string temp;
  PowerData pd;
  PowerEntry pe;

  fstream is;
  //..............................................

  // clear existing MainData
  PM::MainData.clear();

  v::info << "Power Monitor" << "reading power data from file : " << infile  << endl;

  // open input stream
  is.open( (path+infile).c_str() , ios::in );

  // read data from file
  //..............................................
  while( !is.eof() ){
    is >> temp;

    // new IP datablock
    if( temp=="!IP!" ){
      
      // IP name
      is >> temp;
      pe.name=temp;
      is >> temp;

      // IP power data
      while( temp!="!ENDIP!" ){
      
	pd.action=temp;
	is >> temp;
	pd.power=atoi( temp.c_str() );
	pe.entry.push_back(pd);
	is >> temp;
      }
      
    }

    // IP finished
    if(temp=="!ENDIP!"){
      PM::MainData.push_back(pe);
      pe.entry.clear();
    }

  }
  // end read main data
  //..............................................

  is.close();
} // end readdata
//------------------------------------------------


// add power information to received data
//------------------------------------------------
void PM::addpower(string const &path, string const &infile){

  // read MainData
  PM::readdata(path, infile);

  v::info << "Power Monitor" << "adding power data" << endl;

  // clear 'missing' vectors
  PM::missingIp.clear();
  PM::missingAction.clear();

  // iterator
  vector<string>::iterator missing;

  // for every registered IP
  for( vector<IpPowerEntry>::iterator ip=PM::IpData.begin(); ip!=PM::IpData.end(); ip++){

    // search IP in MainData
    //............................................
    vector<PowerEntry>::iterator mainip = MainData.begin();
    while( (mainip != PM::MainData.end()) && (mainip->name != ip->name)  ){ mainip++; }
    //............................................

    // print warning if Ip is missing
    //............................................
    if( mainip==PM::MainData.end() ){
      // print warning if needed
      missing = PM::missingIp.begin();
      while( ( missing != PM::missingIp.end() ) && ( (*missing) != ip->name ) ){ missing++; }
      if ( missing == PM::missingIp.end() ){
	PM::missingIp.push_back( ip->name );
	v::warn << "Power Monitor" << "no power data for registered IP " << ip->name << endl;
      }
    //............................................

    }else{

      // assign power data to logged events
      for( vector<IpPowerData>::iterator ipentry = ip->entry.begin() ; ipentry != ip->entry.end(); ipentry++){
	
	// search action in MainData
	//..........................................
	vector<PowerData>::iterator mainentry = mainip->entry.begin();
	while( (mainentry != mainip->entry.end()) && (mainentry->action != ipentry->action) ){ mainentry++; }
	//..........................................

	// print warning if action is missing
	// ........................................
	if( mainentry == mainip->entry.end() ){
	  // set default value
	  ipentry->power = 0;
	  // print warning if needed
	  missing = PM::missingAction.begin();
	  while( ( missing != missingAction.end() ) && ( (*missing) != (ip->name)+(ipentry->action) ) ){ missing++; }
	  if ( missing == PM::missingAction.end() ){
	    PM::missingAction.push_back( (ip->name)+(ipentry->action) );
	    v::warn << "no power data for IP " << ip->name << ", action : " << ipentry->action << ", setting default value 0" << endl;
	  }
	//........................................

	}else{
	  ipentry->power = mainentry->power;
	} // end if

      } // end entry loop


      // assign power data to idle entries
      for( vector<IpPowerData>::iterator ipentry = ip->idle.begin() ; ipentry != ip->idle.end(); ipentry++){
	
	// search action in MainData
	//..........................................
	vector<PowerData>::iterator mainentry = mainip->entry.begin();
	while( (mainentry != mainip->entry.end()) && (mainentry->action != ipentry->action) ){ mainentry++; }
	//..........................................

	// print warning if action is missing
	// ........................................
	if( mainentry == mainip->entry.end() ){
	  // set default value
	  ipentry->power = 0;
	  // print warning if needed
	  missing = PM::missingAction.begin();
	  while( ( missing != missingAction.end() ) && ( (*missing) != (ip->name)+(ipentry->action) ) ){ missing++; }
	  if ( missing == PM::missingAction.end() ){
	    PM::missingAction.push_back( (ip->name)+(ipentry->action) );
	    v::warn << "no power data for IP " << ip->name << ", action : " << ipentry->action << ", setting default value 0" << endl;
	  }
	//........................................

	}else{
	  ipentry->power = mainentry->power;
	} // end if

      } // end idle_entry loop

    } // end if
  } // end ip loop
} // end addpower 
//------------------------------------------------


// comparing function to sort IpPowerEntries
//------------------------------------------------
bool PM::sortIpEntry(IpPowerData d1, IpPowerData d2){

  if (d1.id != d2.id){
    return(d1.id < d2.id);
  }else{
    // sort by timestamp if equal 'end' is lower
    if (d1.timestamp != d2.timestamp){
      return(d1.timestamp < d2.timestamp);
    }else{
      return(!d1.start);
    }
  }
}
//------------------------------------------------


// sort temporary sums
//------------------------------------------------
bool PM::sortTempSum(tempSum s1, tempSum s2){
  // sort by timestamp if equal 'end' is lower
  if (s1.timestamp != s2.timestamp){
    return(s1.timestamp < s2.timestamp);
  }else{
    return(s2.start);
  }
}
//------------------------------------------------


// print action vector
//------------------------------------------------
void PM::printActionVector ( string &ip, vector<tempActions>::iterator action){

  fstream debug;
  debug.open("logfiles/debug.dat" , ios::out|ios::app );

  debug << "***** action vector *****" << endl;
  debug << "IP : " << ip << ", action : " << action->action << endl; 
  debug << "vector length : " << action->entry.size() << endl;
  for( vector<tempData>::iterator actentry = action->entry.begin(); actentry != action->entry.end(); actentry++){ // entries
    debug << "start : " << actentry->start << " , timestamp : " << actentry->timestamp << " , ID : " << actentry->id << endl;
  }
  debug << "*************************" << endl;
  debug << endl;

}
//------------------------------------------------


// check if action vector entries are correct
//------------------------------------------------
void PM::checkActionVector (IpPowerEntry &ip, vector<tempActions>::iterator const action) {

  fstream debug;
  unsigned int i = action->entry.size()-1;   // last element

  if ( PM::debug >= 2 ){
    debug.open("logfiles/debug.dat" , ios::out|ios::app );
  }
  
  // remove 'start' entries at the end of vector
  while ( (action->entry.size() > 0) && (action->entry[i].start == 1) ){
    if ( PM::debug >= 2 ){
      debug << ip.sc_name << " >> " << action->action << " : " << "start entry without end removed at : " << action->entry[i].timestamp << endl;
    }else{
      v::warn << "Power Monitor" << ip.sc_name << " >> " << action->action << " : " << "start entry without end removed at : " << action->entry[i].timestamp << endl;
    }
    action->entry.erase( action->entry.begin()+i );
    i--;
  }
  
  // check rest
  while ( (action->entry.size() > 1) && (i>0) ) {
    
    if ( action->entry[i].start != action->entry[i-1].start) {
      i--;
    }else{
      
      // 2 end entries
      if ( action->entry[i].start == 0 ) {
	if ( PM::debug >= 2 ){
	  debug << ip.sc_name << " >> " << action->action << " : " << "end entry without start removed at : " << action->entry[i].timestamp << endl; 
	}else{
	  v::warn << "Power Monitor" << ip.sc_name << " >> " << action->action << " : " << "end entry without start removed at : " << action->entry[i].timestamp << endl; 
	}
	action->entry.erase( action->entry.begin() + i-1 );
	i--;
      }
      
      // 2 start entries
      if ( action->entry[i].start == 1 ){
	if ( PM::debug >= 2 ){
	  debug << ip.sc_name << " >> " << action->action << " : " << "start entry without end removed at : " << action->entry[i].timestamp << endl;
	}else{
	  v::warn << "Power Monitor" << ip.sc_name << " >> " << action->action << " : " << "start entry without end removed at : " << action->entry[i].timestamp << endl;
	}
	action->entry.erase( action->entry.begin() + i );
	i--;
      }
      
    } // end correct check if
  } // end while
  
  // remove 'end' entries at start of vector
  if ( action->entry[0].start == 0 ){
    if ( PM::debug >= 2 ){
      debug << ip.sc_name << " >> " << action->action << " : " << "end entry without start removed at : " << action->entry[0].timestamp << endl;
    }else{
      v::warn << "Power Monitor" << ip.sc_name << " >> " << action->action << " : " << "end entry without start removed at : " << action->entry[0].timestamp << endl;
    }    
    action->entry.erase( action->entry.begin() );
  }
  
  // check if valid entries remain
  if ( action->entry.size() <= 1 ) {
    if ( PM::debug >= 2 ){
      debug << ip.sc_name << " >> " << action->action << " : " << "no valid entries remain for : " << action->action << endl;
    }else{
      v::warn << "Power Monitor" << ip.sc_name << " >> " << action->action << " : " << "no valid entries remain for : " << action->action << endl;
    }
    action->entry.clear();
  }

  if ( PM::debug >= 2 ){ debug.close(); }

}
//------------------------------------------------


// merge power sum vectors
//-----------------------------------------------
void PM::mergeSums ( vector<analyzedEntry>::iterator const parent, vector<analyzedEntry>::const_iterator const ip ){

  // declaration
  vector<powerSum> merged;
  vector<powerSum>::const_iterator p = parent->psum.begin();
  vector<powerSum>::const_iterator i = ip->psum.begin();
  powerSum e;

  // merge

  // skip entries at the beginning of parent
  //.........................
  if ( i != ip->psum.end() ){           // check if IP vector is empty
    while ( (p != parent->psum.end()) && (p->timestamp < i->timestamp) ) {
      e.timestamp = p->timestamp;
      e.power = p->power;
      merged.push_back(e);
      p++;
    }
  }
  //.........................
  
  // merge sums
  //.........................
  while ( (i != ip->psum.end()) && (p != parent->psum.end()) ) {

    // insert new value
    if ( i->timestamp != p->timestamp ) {
      
      e.timestamp = i->timestamp;
      if ( p != parent->psum.begin() ) {
	e.power = i->power + (p-1)->power;
      }else{
	e.power = i->power;
      }
      if ( e.power > parent->tpmax ) { parent->tpmax = e.power; }
      merged.push_back(e);
      
    // add power to current parent entry
    }else{
      e.timestamp = p->timestamp;
      e.power = p->power + i->power;
      if ( e.power > parent->tpmax ) { parent->tpmax = e.power; }
      merged.push_back(e);
      p++;
    }

    // internal skip
    while ( (p->timestamp < (i+1)->timestamp) && (p != parent->psum.end() ) ) {
      e.timestamp = p->timestamp;
      e.power = i->power + p->power;
      if ( e.power > parent->tpmax ) { parent->tpmax = e.power; }
      merged.push_back(e);
      p++;
    }

    // next ip entry
    i++;
   
  } // end while
  //.........................
  
  
  // if parent ends and ip entries remain
  //.........................
  if ( (p == parent->psum.end()) && (i != ip->psum.end()) ){
    while ( i != ip->psum.end() ) {
      e.timestamp = i->timestamp;
      e.power = i->power;
      merged.push_back(e);
      i++;
    }
  }
  //.........................
  

  // if ip ends and parent entries remain
  //.........................
  while ( p != parent->psum.end() ) {
    e.timestamp = p->timestamp;
    e.power = p->power;
    merged.push_back(e);
    p++;
  }
  //.........................

  parent->psum = merged;

}
//-----------------------------------------------


// propagate analyzed data in instance-tree
//------------------------------------------------
void PM::propagate(){

  string topname;
  string ipname;
  size_t pos;
  char str[255];

  vector<analyzedEntry>::iterator parent;

  if( PM::AnalyzedData.size() > 1 ){
    for(vector< vector<analyzedEntry> >::iterator level = PM::AnalyzedData.end()-1; level != PM::AnalyzedData.begin(); level--){
      for(vector<analyzedEntry>::const_iterator ip = level->begin(); ip != level->end(); ip++){   // IPs

	pos = ip->sc_name.find_last_of(".");
	topname = ip->sc_name.substr(0,pos);
	ipname = ip->sc_name.substr(pos+1);

	// search parent
	parent = (level-1)->begin();
	while( (parent != (level-1)->end()) && (parent->sc_name != topname) ){ parent++; }
	if( parent == (level-1)->end() && parent->sc_name != topname ){
	  cout << "!!! parent " << topname <<  "not found !!!" << endl;
	
	// propagate values
	}else{
	  parent->ptotal = parent->ptotal + ip->ptotal;
	  sprintf(str,"%10llu",ip->ptotal);
	  parent->subpower.push_back( ipname + " : " + str );
	  if( ip->ptotal > parent->subpmax ){ parent->subpmax = ip->ptotal; }
	}

	// propagate power sums
	PM::mergeSums( parent, ip );
	
      } // endfor IPs
    }  // endfor level
  }  // endif

}
//------------------------------------------------


// convert idle entries
//------------------------------------------------
void PM::merge_idle(IpPowerEntry &ip){

  IpPowerData tempdata;
  vector<IpPowerData> temp;

  if( ip.idle.size() > 0){
    
    vector<IpPowerData>::iterator i = ip.idle.begin();

    while( i != ip.idle.end()-1 ){
      temp.push_back( *i );
      
      // create end entry
      tempdata = *i;
      tempdata.start = 0;
      tempdata.timestamp = (i+1)->timestamp;
      temp.push_back( tempdata );

      // next entry
      i++;
    }

    // edit last entry
    temp.push_back( *i );
    
    tempdata = *i;
    tempdata.start = 0;
    tempdata.timestamp = PM::EndOfSimulation;
    temp.push_back( tempdata );

    // add idle entries to regular power entries
    ip.entry.insert( ip.entry.end(), temp.begin(), temp.end() );

  }

}
//------------------------------------------------


// analyze one IP
//------------------------------------------------
analyzedEntry PM::analyzeIP (IpPowerEntry ip) {

  // declaration
  analyzedEntry analyzedIP;
  vector<tempActions> actions;
  vector<tempActions>::iterator action;
  vector<tempData>::iterator actentry;
  tempData data;
  powerSum sum;
  vector<powerSum>::iterator sumit;
  analyzedData analyzed;
  char str[255];

  tempSum ts;
  vector<tempSum> tempSums;

  // initialization
  analyzedIP.sc_name = ip.sc_name;
  analyzedIP.level = ip.level;
  analyzedIP.pmax = 0;
  analyzedIP.subpmax = 0;
  analyzedIP.tpmax = 0;
  analyzedIP.tsmax = 0;
  analyzedIP.ptotal = 0;
  analyzedIP.entry.clear();
  analyzedIP.psum.clear();
  analyzedIP.subpower.clear();

  sum.timestamp = 0;
  sum.power = 0;


  // merge idle entries into regular power entries
  PM::merge_idle(ip);
 

  // check if analyzing necessary
  if ( ip.entry.size() > 0 ) {

    // sort entries
    sort( ip.entry.begin(), ip.entry.end(), PM::sortIpEntry );
    
    
    // create temporary vector 'actions'
    for(vector<IpPowerData>::const_iterator ipentry = ip.entry.begin(); ipentry != ip.entry.end(); ipentry++){
      
      // search correct position
      //............................................
      action = actions.begin();
      while( ( action != actions.end()) && (ipentry->action != action->action) ){ action++; }
      // if not yet exists create
      if( action == actions.end() ){
	
	tempActions newAction;
	newAction.action=ipentry->action;
	newAction.power=ipentry->power;
	
	actions.push_back(newAction);
	action=actions.end()-1;
	
      }
      //............................................

      // copy data
      //............................................
      data.start = ipentry->start;
      data.timestamp = ipentry->timestamp;
      data.id = ipentry->id;
      
      action->entry.push_back(data);
      //............................................
      
    }
    
    
    // analyze data
    //...........................................
    for( action = actions.begin(); action != actions.end(); action++ ){

      // print vector for debugging
      if ( PM::debug >= 2){ PM::printActionVector(analyzedIP.sc_name,action); }
      
      // check if entries are correct
      if ( PM::debug >= 1 ){ PM::checkActionVector(ip, action); }

      for( actentry = action->entry.begin(); actentry != action->entry.end(); actentry=actentry+2){ // entries
	
	// analyze entry
	//........................................
	analyzed.action=action->action;
	analyzed.start=actentry->timestamp;
	
	if( action->entry.size()>1 && (actentry+1)->start==0 && ( actentry->id == (actentry+1)->id ) ){
	  analyzed.end = (actentry+1)->timestamp;
	}else{
	  v::error << "Power Monitor" << "error in analyzing entry : " << analyzedIP.sc_name << " > " << action->action << endl;
	}
	
	analyzed.dur = analyzed.end - analyzed.start;
	analyzed.power = action->power;
	analyzed.totalpower = static_cast<uint64_t>(analyzed.power * analyzed.dur);
	analyzedIP.ptotal = analyzedIP.ptotal + analyzed.totalpower;
	//........................................
	
	// create temporary vector for computing power sum
	//........................................
	ts.power = analyzed.power;
	
	ts.timestamp = analyzed.start;
	ts.start = 1;
	tempSums.push_back(ts);
	
	ts.timestamp = analyzed.end;
	ts.start = 0;
	tempSums.push_back(ts);
	//........................................
	
	// maxvalues for plot
	if( analyzed.power > analyzedIP.pmax ){ analyzedIP.pmax = analyzed.power; }
	if( analyzed.end > analyzedIP.tsmax ){ analyzedIP.tsmax=analyzed.end; }
	
	analyzedIP.entry.push_back(analyzed);
      } // end entries


    } // end actions 
    //..............................................

    
    // compute power sum
    //..............................................
    sort(tempSums.begin(), tempSums.end(), PM::sortTempSum);

    for(vector<tempSum>::iterator sit = tempSums.begin(); sit != tempSums.end(); sit++){
      sum.timestamp = sit->timestamp;
	
	if( sit->start == 1 ){
	  sum.power = sum.power + sit->power;                                    // action starts -> add power
	  if( sum.power > analyzedIP.tpmax ){ analyzedIP.tpmax = sum.power; }    // log max total power
	}else{
	  sum.power = sum.power - sit->power;                                    // action ends -> subtract power
	}
	
	analyzedIP.psum.push_back(sum);
	sumit = analyzedIP.psum.end()-1;
	
	// correct entries at same timestamp
	if( (analyzedIP.psum.size() > 1 ) && (sumit->timestamp == (sumit-1)->timestamp) ){
	  analyzedIP.psum.erase( sumit-1 );
	}
    }
    //..............................................
    
  } // end if entry size > 0
  
  analyzedIP.subpmax = analyzedIP.ptotal;
  sprintf(str,"%10llu",analyzedIP.ptotal);
  analyzedIP.subpower.push_back( string("own : ") + str );

  return(analyzedIP);

}
//------------------------------------------------


// analyze gathered data
//------------------------------------------------
void PM::analyze(string const path, string const infile, string const outfile){
  
  // debug, remove later
  //PM::debug = 1;

  if (PM::LimitedRegion){ PM::EndOfSimulation = PM::end_log; }

  // create logfile for advanced debugging
  if (PM::debug >= 2){
    // output directory
    system("mkdir -p logfiles/");
    // output stream
    fstream debug;
    debug.open("logfiles/debug.dat" , ios::out );

    debug << "#########################" << endl;
    debug << "#   extended output     #" << endl;
    debug << "#########################" << endl;
    debug << endl;

    debug.close();
  }

  // add power data from file
  PM::addpower(path , infile);

  // declaration
  analyzedEntry analyzedIP;
  vector<analyzedEntry> empty;

  // expand AnalyzedData vector by required levels
  PM::AnalyzedData.resize( PM::maxLevel+1 , empty);

  // insert main
  analyzedIP.pmax=0;
  analyzedIP.subpmax=0;
  analyzedIP.tpmax=0;
  analyzedIP.tsmax=0;
  analyzedIP.ptotal=0;
  analyzedIP.subpower.push_back( string("own : 0") );
  analyzedIP.sc_name="main";
  PM::AnalyzedData[0].push_back( analyzedIP );
  
  // analyze IPs
  v::info << "Power Monitor" << "analyzing data, debuglevel is " << PM::debug << endl;
  for( vector<IpPowerEntry>::iterator ip = PM::IpData.begin(); ip != PM::IpData.end(); ip++ ){
    analyzedIP = PM::analyzeIP(*ip);
    PM::AnalyzedData[ ip->level ].push_back( analyzedIP );
  }

  // propagate values in instance-tree
  PM::propagate();

  // print analyzed data to file
  PM::analyzedlogprint(infile, outfile);

} // end analyze
//------------------------------------------------


// logprint
// prints received power data to logfile
//------------------------------------------------
void PM::raw_logprint(string const file){

  v::info << "Power Monitor" << "printing raw data to logfile" << endl;
  
  // create directory
  system("mkdir -p logfiles");

  // outputstream
  fstream log;
  log.open( ("logfiles/"+file).c_str() , ios::out );
  
  // print data
  for( vector<IpPowerEntry>::iterator ip=PM::IpData.begin(); ip!=PM::IpData.end(); ip++){
    log << "!IP!" << endl; 
    log << ip->sc_name << endl;
    log << ip-> name << endl;
    log << ip->level << endl;
    for(vector<IpPowerData>::iterator ipentry = ip->entry.begin(); ipentry != ip->entry.end(); ipentry++){
      log << ipentry->action << "\t" << ipentry->start << "\t" << ipentry->timestamp << endl;
    }
    log << "!ENDIP!" << endl;
    log << endl;
  }

  log.close();
} // end logprint
//------------------------------------------------


// read raw data from file
//------------------------------------------------
void PM::read_raw_data(string const infile){
  
  // declaration
  //..............................................
  string temp;
  IpPowerData pd;
  IpPowerEntry pe;

  fstream is;
  //..............................................

  // clear existing IpData
  PM::IpData.clear();

  v::info << "Power Monitor" << "reading raw data from file : " << infile << endl;

  // open input stream
  is.open( ("logfiles/"+infile).c_str() , ios::in );

  // read data from file
  //..............................................
  while( !is.eof() ){
    is >> temp;

    // new IP datablock
    if( temp=="!IP!" ){
      
      // IP name
      is >> temp;
      pe.sc_name = temp;
      is >> temp;
      pe.name = temp;
      is >> temp;
      pe.level = atoi( temp.c_str() );
      is >> temp;

      // IP power data
      while( temp!="!ENDIP!" ){
      
	pd.action=temp;
	is >> temp;
	pd.start = atoi( temp.c_str() );
	is >> temp;
	pd.timestamp = atoi( temp.c_str() );
	pd.power = 0;

	pe.entry.push_back(pd);
	is >> temp;
      }
      
    }

    // IP finished
    if(temp=="!ENDIP!"){
      PM::IpData.push_back(pe);
      pe.entry.clear();
    }

  }
  // end read data
  //..............................................

  is.close();
} // end read raw data
//------------------------------------------------


// analyze offline using raw data
//------------------------------------------------
void PM::analyze_offline(string const path, string const infile, string const outfile, string const data_path, string const data){

  // read raw data
  PM::read_raw_data( string(data_path + data) );

  // analyze
  PM::analyze(path,infile,outfile);

}
//------------------------------------------------


// print analyzed data to log and plot files
//------------------------------------------------
void PM::analyzedlogprint(string const &infile, string const &outfile){

  // declaration
  //..............................................

  // output streams
  fstream log;
  fstream plot;

  // scale
  uint64_t xmax;
  uint64_t tics;
  uint64_t ymax;

  // label
  double posy;
  double posx;

  // temporary vector
  char str1[255];
  char str2[255];
  string ts;
  vector<string> temp;
  size_t pos;

  //..............................................

  v::info << "Power Monitor" << "printing analyzed data to log and plotfiles" << endl;

  // create directories
  //............................................
  system( ("mkdir -p build/logfiles/"+outfile).c_str() );
  system( ("mkdir -p build/plotfiles/"+outfile).c_str() );
  system( ("mkdir -p build/graphs/"+outfile).c_str() );
  //............................................

  for(vector< vector<analyzedEntry> >::iterator level = PM::AnalyzedData.begin(); level != PM::AnalyzedData.end(); level++){
  for(vector<analyzedEntry>::iterator ip = level->begin(); ip != level->end(); ip++){   // IPs

    // open streams
    //............................................
    log.open( ("build/logfiles/"+outfile+"/"+ip->sc_name+".dat").c_str() , ios::out );
    plot.open( ("build/plotfiles/"+outfile+"/"+ip->sc_name+".gnu").c_str() , ios::out );
    //............................................

    // init
    //............................................

    // log
    log << "# analyzed power data" << endl;
    log << "# power data from file : " << infile << ".dat" << endl;
    log << endl;
    log << "# power composition : " << endl;
    log << "total power consumed by IP : " << ip->ptotal << endl;
    for(vector<string>::iterator sub = ip->subpower.begin(); sub != ip->subpower.end(); sub++){
      log << *sub << endl;
    }
    log << endl;
    log << "# own power statistic : " << endl;
    log << "# action \t start \t end \t duration \t power \t powertotal" << endl;
    log << endl;

    // plot
    plot << "# temporary file automatically generated by PowerMonitor" << endl;
    plot << endl;
    // settings
    plot << "# settings" << endl;
    plot << "set terminal postscript" << endl;
    plot << "set output \"" << ip->sc_name << "_graph.ps\"" << endl;
    plot << "set pointsize 0.3" << endl;
    // x-axis
    xmax = static_cast<uint64_t>( PM::EndOfSimulation + 1 + 0.1*(PM::EndOfSimulation-PM::start_log) );
    tics=(xmax-PM::start_log)/5;
    if( tics==0 ){ tics=1; }
    plot << "set xtics " << tics << endl;
    plot << "set xrange [" << PM::start_log << ":" << xmax << "]" << endl;
    plot << "set xlabel \"time \"" << endl;
    // y-axis
    ymax = static_cast<uint64_t>( ip->pmax + 1 + 0.1*ip->pmax );
    plot << "set yrange [0:" << ymax << ".]" << endl;
    plot << "set ylabel \"power \"" << endl;
    // arrows
    plot << "# linestyle and color can be changed here" << endl;
    plot << "set style arrow 1 heads size screen 0.008,90 #ls 1" << endl; // linestyle not available on server
    plot << endl;
    plot << "# data" << endl;
    plot << endl;
    //............................................

    // write data
    //............................................
    if(ip->entry.size()>0) {
    ts = ip->entry.begin()->action;
    for(vector<analyzedData>::iterator ipentry = ip->entry.begin(); ipentry != ip->entry.end(); ipentry++ ){
      // logdata
      if( ipentry->action != ts){
	log << "#=================================================================" << endl;
	// label
	posx=(ipentry->start + ipentry->end)/2;
	posy = ipentry->power + 0.02*ymax;
	plot << "set label ' "<< ipentry->action <<"' at " << posx << "," << posy << " center" << endl;
	ts = ipentry->action;
      }
      log << ipentry->action << "\t" << ipentry->start << "\t" << ipentry->end << "\t" << ipentry->dur << "\t" << ipentry->power << "\t" << ipentry->totalpower << endl;

      // plotdata
      // arrow
      plot << "set arrow from "<< ipentry->start << "," << ipentry->power << " to " << ipentry->end << "," << ipentry->power << " as 1 " << endl;
      plot << endl;
    }
    }
    plot << "plot [" << PM::start_log << ":" << xmax <<  "] 0 notitle" << endl;
    plot << endl;
    //............................................


    // init for sumplot
    //............................................
    plot << "set noarrow" << endl;
    plot << "set nolabel" << endl;

    // x-axis
    xmax = static_cast<uint64_t>( PM::EndOfSimulation + 1 + 0.1*(PM::EndOfSimulation-PM::start_log) ); 
    tics = static_cast<uint64_t>( (xmax-PM::start_log)/5 );
    if( tics==0 ){ tics=1; }
    plot << "set xtics " << tics << endl;
    plot << "set xrange ["<< PM::start_log << ":" << xmax << "]" << endl;
    plot << "set xlabel \"time \"" << endl;
 
    // y-axis
    ymax = static_cast<uint64_t>( ip->tpmax + 1 + 0.1*ip->tpmax );
    plot << "set yrange [0:" << ymax << "]" << endl;
    plot << "set ylabel \"total power \"" << endl;

    plot << "plot '"+ip->sc_name+".dat' using 1:2 with linespoints notitle" << endl;

    plot << endl;
    plot << "set xrange [-0.7:" << ip->subpower.size()-0.3 << "]" << endl;
    ymax = static_cast<uint64_t>( ip->subpmax + 1 + 0.1*(ip->subpmax) );
    plot << "set yrange [0:" << ymax << "]" << endl;
    plot << "set xlabel \"IP \"" << endl;
    plot << "set xtics(\"own\" 0";
    
    unsigned int i;
    vector<string>::iterator sub;

    if ( ip->subpower.size() > 1 ){
      i = 1;
      sub = ip->subpower.begin()+1;
      
      while( sub != ip->subpower.end() ){
	pos = sub->find_last_of(":");
	plot << ", \"" << sub->substr(0,pos-1) << "\" " << i;
	i++;
	sub++;
      }
    }

    plot << ")" << endl;

    plot << "set boxwidth 0.75" << endl; 
    plot << "set style fill solid 0.25 border" << endl;

    plot << "plot '"+ip->sc_name+".dat' using 0:4 with boxes notitle" << endl;
    //............................................

    log << endl;
    log << "# EOF" << endl;

    plot << endl;
    plot << "# EOF" << endl;

    // close streams
    //............................................
    log.close();
    plot.close();
    //............................................

    // write sum data
    //............................................
    plot.open( ("build/plotfiles/"+outfile+"/"+ip->sc_name+".dat").c_str() , ios::out );
    
    plot << "# timestamp \t totalpower" << endl;

    if ( ip->psum.size() > 0 ){
      sprintf(str1,"%10llu",ip->psum.begin()->timestamp);
      temp.push_back( string(str1) + "\t 0" );
      sprintf(str2,"%10llu",ip->psum.begin()->power);
      temp.push_back( string(str1) + "\t" + string(str2) );
    }

    if ( ip->psum.size() > 1 ){
      for(vector<powerSum>::iterator ipsum = ip->psum.begin()+1; ipsum != ip->psum.end(); ipsum++){
	sprintf(str1,"%10llu",ipsum->timestamp);
	sprintf(str2,"%10llu",(ipsum-1)->power);
	temp.push_back( string(str1) + "\t" + string(str2) );
	sprintf(str2,"%10llu",ipsum->power);
	temp.push_back( string(str1) + "\t" + string(str2) );
      }
    }

    // add subip data
    i=0;
    sub = ip->subpower.begin();
      
    while( sub != ip->subpower.end() && i<temp.size() ){
      pos = sub->find_last_of(":");
      temp[i] = temp[i] + "\t" + sub->substr(0,pos-1) + "\t" + sub->substr(pos+1);
      i++;
      sub++;
    }

    if( ip->subpower.size() > temp.size() ){
      while( sub != ip->subpower.end() ){
	pos = sub->find_last_of(":");
	temp.push_back( string("0 \t 0 \t ") + sub->substr(0,pos-1) + "\t" + sub->substr(pos+1) );
	sub++;
      }
    }

    for( vector<string>::iterator t = temp.begin(); t != temp.end(); t++ ){
      plot << *t << endl;
    }
    temp.clear();

    plot.close();
    //............................................
  }
  }

  // create plotscript
  //..............................................
  plot.open( "build/plotfiles/plot" , ios::out );
  
  plot << "#!/bin/bash" << endl;
  plot << "S=$1" << endl;
  plot << "S1=${S%.gnu}" << endl;
  plot << "DIR=${S1%/*}" << endl;
  plot <<" DAT=${S1#*/}" << endl;
  plot << "cd $DIR" << endl;
  plot << "if [ -f $DAT.gnu ]" << endl;
  plot << "then" << endl;
  plot << "gnuplot ${DAT}.gnu" << endl;
  plot << "ps2pdf ${DAT}_graph.ps" << endl; 
  plot << "rm ${DAT}_graph.ps" << endl;
  plot << "cd ../../" << endl;
  plot << "mkdir -p graphs/${DIR}" << endl;
  plot << "mv plotfiles/${DIR}/${DAT}_graph.pdf graphs/${DIR}/${DAT}.pdf" << endl;
  plot << "else" << endl;
  plot << "echo \"File ${DAT}.gnu does not exist!\" "<< endl;
  plot << "fi" << endl;
  plot << "exit 0" << endl;

  plot.close();
  system("chmod u+x build/plotfiles/plot");
  //..............................................

} // end analyzedlogprint
//------------------------------------------------
