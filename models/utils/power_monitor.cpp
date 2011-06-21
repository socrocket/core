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

int PM::maxLevel;
//------------------------------------------------

//================================================


// register IP
//------------------------------------------------
void PM::registerIP(sc_module* ip, string name){
  
  IpPowerEntry temp;
  string sc_name = ip->name(); 
  // add testbench to name
  sc_name = "main."+sc_name;
  v::info << "power_monitor" << "IP " << sc_name << " registered" << endl;
  
  temp.ip=ip;
  temp.sc_name=sc_name;
  temp.name=name;

  int level = 0;
  
  // split name
  size_t pos = sc_name.find_last_of(".");
  while( pos != -1 ){
    level++;
    sc_name = sc_name.substr(0,pos);
    pos = sc_name.find_last_of(".");
  }


  temp.level = level;                     // instance level
  if ( level > PM::maxLevel ){ PM::maxLevel = level; } // global maximum level

  // add new IP to datavector
  PM::IpData.push_back(temp);

} // end registerIP
//------------------------------------------------


// send data
// called by IP 
//------------------------------------------------
void PM::send(sc_module* ip, IpPowerData data){

  // search IP in IpData
  vector<IpPowerEntry>::iterator i=PM::IpData.begin();
  while( (i !=PM::IpData.end()) && (ip!=i->ip) ){ i++; }

  // check if IP registered and insert data
  if( i==PM::IpData.end() ){
    v::warn << "Power Monitor" << "sending IP not registered" << endl;
  }else{
    data.power=0;
    i->entry.push_back(data);
  }
} // send 
//------------------------------------------------


// read main data from file
//------------------------------------------------
void PM::readdata(string &infile){
  
  // declaration
  //..............................................
  string temp;
  PowerData pd;
  PowerEntry pe;

  fstream is;
  //..............................................

  // clear existing MainData
  PM::MainData.clear();

  v::info << "Power Monitor" << "reading power data from file : " << infile << ".dat" << endl;

  // open input stream
  is.open( (infile+".dat").c_str() , ios::in );

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
void PM::addpower(string &infile){

  // read MainData
  PM::readdata(infile);

  v::info << "Power Monitor" << "adding power data" << endl;

  // for every registered IP
  for( vector<IpPowerEntry>::iterator ip=PM::IpData.begin(); ip!=PM::IpData.end(); ip++){

    // search IP in MainData
    //............................................
    vector<PowerEntry>::iterator mainip = MainData.begin();
    while( (mainip != PM::MainData.end()) && (mainip->name != ip->name)  ){ mainip++; }
    //............................................

    if( mainip==PM::MainData.end() ){
      v::warn << "Power Monitor" << "no power data for registered IP " << ip->name << endl;
    }else{

      // assign power data to logged events
      for( vector<IpPowerData>::iterator ipentry = ip->entry.begin() ; ipentry != ip->entry.end(); ipentry++){
	
	// search action in MainData
	//..........................................
	vector<PowerData>::iterator mainentry = mainip->entry.begin();
	while( (mainentry != mainip->entry.end()) && (mainentry->action != ipentry->action) ){ mainentry++; }
	//..........................................

	if( mainentry == mainip->entry.end() ){
	  ipentry->power = 0;
	  v::warn << "no power data for IP " << ip->name << ", action : " << ipentry->action << ", setting default value 0" << endl;
	}else{
	  ipentry->power = mainentry->power;
	} // end if
      } // end entry loop
    } // end if
  } // end ip loop
} // end addpower 
//------------------------------------------------


// comparing function to sort IpPowerEntries
//------------------------------------------------
bool PM::sortIpEntry(IpPowerData d1, IpPowerData d2){
  return(d1.timestamp < d2.timestamp);
}
//------------------------------------------------


// extract gathered IP data
//------------------------------------------------
tempIp PM::extractIp(IpPowerEntry ip){

  // temporary structs
  tempIp tip;
  tempData td;
  powerSum temp;

  // sort entries
  sort( ip.entry.begin(), ip.entry.end(), PM::sortIpEntry );
  
  tip.act.clear();

  tip.tpmax=0;
  temp.power=0;
  temp.oldpower=0;

  for(vector<IpPowerData>::const_iterator ipentry = ip.entry.begin(); ipentry != ip.entry.end(); ipentry++){

    // ip power sum
    //............................................
    temp.timestamp = ipentry->timestamp;
    temp.oldpower = temp.power;

    if( ipentry->start == 1 ){
      temp.power = temp.power + ipentry->power;                  // action starts -> add power
      if( temp.power > tip.tpmax ){ tip.tpmax = temp.power; }    // log max total power
    }else{
      temp.power = temp.power - ipentry->power;                  // action ends -> subtract power
    }
    tip.sums.push_back(temp);
    //............................................


    // search correct position
    //............................................
    vector<tempActions>::iterator tipaction = tip.act.begin();
    while( ( tipaction != tip.act.end()) && (ipentry->action != tipaction->action) ){ tipaction++; }
    // if not yet exists create
    if( tipaction == tip.act.end() ){

      tempActions ta;
      ta.action=ipentry->action;
      ta.power=ipentry->power;

      tip.act.push_back(ta);
      tipaction=tip.act.end()-1;
    }
    //............................................

    // copy data
    //............................................
    td.start = ipentry->start;
    td.timestamp = ipentry->timestamp;

    tipaction->entry.push_back(td);
    //............................................
    
  }

  // correct parallel events at same timestamp
  // for powersum plot
  //............................................
  unsigned int i=0;
  if( tip.sums.size()>0 ){
    while( i < tip.sums.size()-1 ){
      if( tip.sums[i].timestamp == tip.sums[i+1].timestamp ){
	tip.sums[i].power = tip.sums[i].power + tip.sums[i+1].power;
	if( tip.sums[i].power > tip.tpmax){ tip.tpmax = tip.sums[i].power; }
	if( i < tip.sums.size()-2 ){ tip.sums[i+2].oldpower = tip.sums[i].power; }
	tip.sums.erase( tip.sums.begin()+i+1 );
      }
      i++;
    }
  }
  //............................................

  return(tip);
} // extractIp
//------------------------------------------------


// analyze gathered data
//------------------------------------------------
void PM::analyze(string infile, string outfile){

  // add power data from file
  PM::addpower(infile);

  // analyze registered IPs
  //..............................................
  tempIp tempip;
  analyzedData te;
  analyzedEntry ten;
  vector<analyzedEntry> empty;
  char str[255];

  PM::AnalyzedData.resize( PM::maxLevel+1 , empty);
  vector<analyzedEntry>::iterator analyzedIp;

  // insert main
  ten.pmax=0;
  ten.subpmax=0;
  ten.tpmax=0;
  ten.tsmax=0;
  ten.ptotal=0;
  ten.subpower.push_back( string("own : 0") );
  ten.sc_name="main";
  PM::AnalyzedData[0].push_back(ten);
  ten.subpower.clear();
  

  for( vector<IpPowerEntry>::iterator ip = PM::IpData.begin(); ip != PM::IpData.end(); ip++ ){  // IPs

    // initialize new analyzed entry
    ten.pmax=0;
    ten.subpmax=0;
    ten.tpmax=0;
    ten.tsmax=0;
    ten.entry.clear();
    ten.psum.clear();

    // extract one IP
    tempip=PM::extractIp(*ip);

    ten.sc_name=ip->sc_name;
    ten.psum=tempip.sums;
    ten.tpmax=tempip.tpmax;
    ten.ptotal=0;

    // add analyzed entry
    PM::AnalyzedData[ ip->level ].push_back(ten);
    analyzedIp = AnalyzedData[ ip->level ].end() - 1;

    // analyze data
    for( vector<tempActions>::iterator action = tempip.act.begin(); action != tempip.act.end(); action++ ){                    // actions
      for( vector<tempData>::iterator actentry = action->entry.begin(); actentry != action->entry.end(); actentry=actentry+2){ // entries

	// analyze entry
	//........................................
	te.action=action->action;
	te.start=actentry->timestamp;

	if( (actentry+1)->start==0 ){
	  te.end = (actentry+1)->timestamp;
	}else{
	  v::warn << "Power Monitor" << "error in analyzing entry" << endl;
	}

	te.dur = te.end - te.start;
	te.power = action->power;
	te.totalpower = static_cast<unsigned long int>(te.power * te.dur);
	analyzedIp->ptotal = analyzedIp->ptotal + te.totalpower;
	//........................................


	// maxvalues for plot
	if( te.power > analyzedIp->pmax ){ analyzedIp->pmax = te.power; }
	if( te.end > analyzedIp->tsmax ){ analyzedIp->tsmax=te.end; }

	analyzedIp->entry.push_back(te);
      }
    }
    analyzedIp->subpmax = analyzedIp->ptotal;
    sprintf(str,"%5d",analyzedIp->ptotal);
    analyzedIp->subpower.push_back( string("own : ") + str );
  }  // end for loop IPs
  //..............................................

  // propagate values in instance-tree
  //..............................................
  string topname;
  string ipname;
  size_t pos;

  vector<analyzedEntry>::iterator parent;

  if( PM::AnalyzedData.size() > 1 ){
    for(vector< vector<analyzedEntry> >::iterator level = PM::AnalyzedData.end()-1; level != PM::AnalyzedData.begin(); level--){
      for(vector<analyzedEntry>::iterator ip = level->begin(); ip != level->end(); ip++){   // IPs

	pos = ip->sc_name.find_last_of(".");
	topname = ip->sc_name.substr(0,pos);
	ipname = ip->sc_name.substr(pos+1);

	// search parent
	parent = (level-1)->begin();
	while( (parent != (level-1)->end()) && (parent->sc_name != topname) ){ parent++; }
	if( parent == (level-1)->end() && parent->sc_name != topname ){
	  v::warn << "Power Monitor" << "parent " << topname <<  "not found" << endl;
	}else{
	  parent->ptotal = parent->ptotal + ip->ptotal;
	  sprintf(str,"%5d",ip->ptotal);
	  parent->subpower.push_back( ipname + " : " + str );
	  if( ip->ptotal > parent->subpmax ){ parent->subpmax = ip->ptotal; }
	}

      } // endfor IPs
    }  // endfor level
  }  // endif
  //..............................................

  // print analyzed data to file
  PM::analyzedlogprint(infile, outfile);

} // end analyze
//------------------------------------------------


// logprint
// prints received power data to logfile
//------------------------------------------------
void PM::logprint(string file){

  fstream os;
  os.open( file.c_str() , ios::out|ios::app );
  
  for(vector<IpPowerEntry>::iterator ip = IpData.begin(); ip != PM::IpData.end(); ip++){
    os << "!IP!" << endl; 
    os << ip->name << endl;
    for(vector<IpPowerData>::iterator ipentry = ip->entry.begin(); ipentry != ip->entry.end(); ipentry++){
      os << ipentry->action << "\t" << ipentry->timestamp << endl;
    }
    os << "!ENDIP!" << endl;
    os << endl;
  }

  os.close();
} // end logprint
//------------------------------------------------


// print analyzed data to log
//------------------------------------------------
void PM::analyzedlogprint(string infile, string outfile){

  // declaration
  //..............................................

  // output streams
  fstream log;
  fstream plot;

  // scale
  unsigned long int xmax;
  unsigned long int tics;
  unsigned long int ymax;

  // label
  double posy;
  double posx;

  // temporary vector
  char str1[255];
  char str2[255];
  vector<string> temp;
  size_t pos;

  //..............................................

  v::info << "Power Monitor" << "printing analyzed data to log and plotfiles" << endl;

  // create directories
  //............................................
  system( ("mkdir -p logfiles/"+outfile).c_str() );
  system( ("mkdir -p plotfiles/"+outfile).c_str() );
  system( ("mkdir -p graphs/"+outfile).c_str() );
  //............................................

  for(vector< vector<analyzedEntry> >::iterator level = PM::AnalyzedData.begin(); level != PM::AnalyzedData.end(); level++){
  for(vector<analyzedEntry>::iterator ip = level->begin(); ip != level->end(); ip++){   // IPs

    // open streams
    //............................................
    log.open( ("logfiles/"+outfile+"/"+ip->sc_name+".dat").c_str() , ios::out );
    plot.open( ("plotfiles/"+outfile+"/"+ip->sc_name+".gnu").c_str() , ios::out );
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
    plot << "set pointsize 0.6" << endl;
    // x-axis
    xmax = static_cast<unsigned long int>( ip->tsmax + 1 + 0.1*ip->tsmax );
    tics=xmax/10;
    if( tics==0 ){ tics=1; }
    plot << "set xtics " << tics << endl;
    plot << "set xrange [0:" << xmax << "]" << endl;
    plot << "set xlabel \"time \"" << endl;
    // y-axis
    ymax = static_cast<unsigned long int>( ip->pmax + 1 + 0.1*ip->pmax );
    plot << "set yrange [0:" << ymax << "]" << endl;
    plot << "set ylabel \"power \"" << endl;
    // arrows
    plot << "# linestyle and color can be changed here" << endl;
    plot << "set style arrow 1 heads size screen 0.008,90 ls 1" << endl;
    plot << endl;
    plot << "# data" << endl;
    plot << endl;
    //............................................

    // write data
    //............................................
    for(vector<analyzedData>::iterator ipentry = ip->entry.begin(); ipentry != ip->entry.end(); ipentry++ ){
      // logdata
      log << ipentry->action << "\t" << ipentry->start << "\t" << ipentry->end << "\t" << ipentry->dur << "\t" << ipentry->power << "\t" << ipentry->totalpower << endl;

      // plotdata
      // arrow
      plot << "set arrow from "<< ipentry->start << "," << ipentry->power << " to " << ipentry->end << "," << ipentry->power << " as 1 " << endl;
      // label
      posx=(ipentry->start + ipentry->end)/2;
      posy = ipentry->power + 0.02*ymax;
      plot << "set label ' "<< ipentry->action <<"' at " << posx << "," << posy << " center" << endl;
      plot << endl;
    }
    //............................................

    // init for sumplot
    //............................................
    plot << "plot [0:" << xmax <<  "] 0 notitle" << endl;
    plot << endl;

    plot << "set noarrow" << endl;
    plot << "set nolabel" << endl;

    // x-axis
    tics=static_cast<unsigned long int>( xmax/10 );
    if( tics==0 ){ tics=1; }
    plot << "set xtics " << tics << endl;
    plot << "set xrange [0:" << xmax << "]" << endl;
    plot << "set xlabel \"time \"" << endl;
 
    // y-axis
    ymax = static_cast<unsigned long int>( ip->tpmax + 1 + 0.1*ip->pmax );
    plot << "set yrange [0:" << ymax << "]" << endl;
    plot << "set ylabel \"total power \"" << endl;

    plot << "plot '"+ip->sc_name+".dat' using 1:2 with linespoints notitle" << endl;

    plot << endl;
    plot << "set xrange [-0.7:" << ip->subpower.size()-0.3 << "]" << endl;
    ymax = static_cast<unsigned long int>( ip->subpmax + 1 + 0.1*(ip->subpmax) );
    plot << "set yrange [0:" << ymax << "]" << endl;
    plot << "set xlabel \"IP \"" << endl;
    plot << "set xtics(\"own\" 0";
    int i=1;
    vector<string>::iterator sub = ip->subpower.begin()+1;
      
    while( sub != ip->subpower.end() ){
      pos = sub->find_last_of(":");
      plot << ", \"" << sub->substr(0,pos-1) << "\" " << i;
      i++;
      sub++;
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
    plot.open( ("plotfiles/"+outfile+"/"+ip->sc_name+".dat").c_str() , ios::out );
    
    plot << "# timestamp \t totalpower" << endl;

    for(vector<powerSum>::iterator ipsum = ip->psum.begin(); ipsum != ip->psum.end(); ipsum++){
      sprintf(str1,"%5d",ipsum->timestamp);
      sprintf(str2,"%5d",ipsum->oldpower);
      temp.push_back( string(str1) + "\t" + string(str2) );
      sprintf(str2,"%5d",ipsum->power);
      temp.push_back( string(str1) + "\t" + string(str2) );
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

} // end analyzedlogprint
//------------------------------------------------
