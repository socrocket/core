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
// Title:      power_monitor.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Header file for power monitor class
//             
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


#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <iostream>
#include<fstream>
#include<stdio.h>

#include <stdlib.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

#include<string>
#include<vector>

#include<systemc>
#include "verbose.h"

using namespace std;

//================================================

// structs for power monitor LUT
//------------------------------------------------
struct PowerData{
  string action;
  unsigned int power;
};

struct PowerEntry{
  string name;
  vector<PowerData> entry;
};
//------------------------------------------------


// structs for data sent from IPs
//------------------------------------------------
struct IpPowerData{
  string action;
  bool start;
  unsigned long int timestamp;
  unsigned int power;
  unsigned int id;
};

struct IpPowerEntry{
  sc_module* ip;
  string sc_name;
  string name;
  unsigned int level;
  vector<IpPowerData> entry;
  vector<IpPowerData> idle;
};
//------------------------------------------------


// structs for analysis
//------------------------------------------------

//
struct tempData{
  bool start;
  unsigned long int timestamp;
};

//
struct tempSum{
  unsigned long int timestamp;
  bool start;
  unsigned long int power;
};

// 
struct tempActions{
  string action;
  unsigned int power;
  vector<tempData> entry;
};

// entry for integrated plot
struct powerSum{
  unsigned long int timestamp;
  unsigned long int power;
};

// evaluated ip entry
struct analyzedData{
  string action;                  // name of performed action
  unsigned long int start;        // timestamp of action start
  unsigned long int end;          // timestamp of action end
  unsigned long int dur;          // duration of action
  unsigned int power;             // basic power consumption
  unsigned long int totalpower;   // total power consumed by action
};

// evaluatd ip
struct analyzedEntry{
  // evaluation
  string sc_name;                 // systemc hierarchy name
  unsigned int level;             // level in instance tree
  unsigned long int ptotal;       // total power consumed
  vector<analyzedData> entry;     // evaluated data for actions
  vector<powerSum> psum;          // integrated total power consumed
  vector<string> subpower;        // names of subips
  // for scaling plots
  unsigned int pmax;              // highest ip power value
  unsigned int subpmax;           // highest subip power value
  unsigned int tpmax;             // highest power sum power value
  unsigned long int tsmax;        // highest ip timestamp
};
//------------------------------------------------

//================================================

class PM {


 private :
    // data
    //--------------------------------------------
    static vector<PowerEntry> MainData;
    static vector<IpPowerEntry> IpData;
    static vector< vector<analyzedEntry> > AnalyzedData;
    static unsigned int maxLevel;
    static unsigned long int EndOfSimulation;
    static vector<string> missingIp;
    static vector<string> missingAction;
    //--------------------------------------------

    // methods
    //--------------------------------------------
    
    // I/O
    static void readdata(string const &path, string const &infile);
    static void addpower(string const &path, string const &infile);
    static void analyzedlogprint(string const &infile, string const &outfile);
    static void read_raw_data(string const infile);

    // analyzing
    static bool sortIpEntry(IpPowerData d1, IpPowerData d2);
    static bool sortTempSum(tempSum s1, tempSum s2);
    static void printActionVector ( string &ip, vector<tempActions>::iterator action);
    static void checkActionVector ( IpPowerEntry &ip, vector<tempActions>::iterator const action);

    static void merge_idle(IpPowerEntry &ip);
    static void mergeSums( vector<analyzedEntry>::iterator const parent, vector<analyzedEntry>::const_iterator const ip );
    static void propagate();

    static analyzedEntry analyzeIP (IpPowerEntry ip);

    // for debugging, remove later
    static void print ( vector<powerSum> &v );

    //--------------------------------------------


 public :
  
  // constructors + destructors
  //----------------------------------------------
  
  // standard-constructor
  PM(void);

  // destructor
  ~PM(void);
  //----------------------------------------------

  // variables
  //---------------------------------------------
  static unsigned int debug;
  //---------------------------------------------


  // methods
  //----------------------------------------------
  static void registerIP(sc_module* ip, string name, bool active);
  static void send(sc_module* ip, string action, bool start, sc_time timestamp, unsigned int id, bool active);
  static void send_idle(sc_module* ip, string action, sc_time timestamp, bool active);
  static void analyze(string const path, string const infile, string const outfile);

  static void raw_logprint(string const file);
  static void analyze_offline(string const path, string const infile, string const outfile, string const data_path, string const data);
  //----------------------------------------------

};

#endif // POWER_MONITOR_H
