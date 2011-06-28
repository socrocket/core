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
  int power;
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
  int power;
};

struct IpPowerEntry{
  sc_module* ip;
  string sc_name;
  string name;
  int level;
  vector<IpPowerData> entry;
};
//------------------------------------------------


// structs for analysis
//------------------------------------------------
struct tempData{
  bool start;
  unsigned long int timestamp;
};

struct tempActions{
  string action;
  int power;
  vector<tempData> entry;
};

struct powerSum{
  unsigned long int timestamp;
  unsigned int power;
  unsigned int oldpower;
};

struct tempIp{
  unsigned int tpmax;
  vector<tempActions> act;
  vector<powerSum> sums;
};

struct analyzedData{
  string action;
  unsigned long int start;
  unsigned long int end;
  unsigned long int dur;
  unsigned int power;
  unsigned long int totalpower;
};

struct analyzedEntry{
  string sc_name;
  unsigned int pmax;
  unsigned int subpmax;
  unsigned int tpmax;
  unsigned long int tsmax;
  long int ptotal;
  vector<analyzedData> entry;
  vector<powerSum> psum;
  vector<string> subpower;
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
    static vector<string> missingIp;
    static vector<string> missingAction;
    //--------------------------------------------

    // methods
    //--------------------------------------------
    static void readdata(string &infile);
    static void addpower(string &infile);
    static tempIp extractIp(IpPowerEntry ip);
    static bool sortIpEntry(IpPowerData d1, IpPowerData d2);
    static void analyzedlogprint(string infile, string outfile);
    //--------------------------------------------


 public :
  
  // constructors + destructors
  //----------------------------------------------
  
  // standard-constructor
  PM(void);

  // destructor
  ~PM(void);
  //----------------------------------------------


  // methods
  //----------------------------------------------
  static void registerIP(sc_module* ip, string name);
  static void send(sc_module* ip, string action, bool start, unsigned long int timestamp);
  static void analyze(string infile, string outfile);

  static void raw_logprint(string file);
  static void read_raw_data(string infile);
  //----------------------------------------------

};

#endif // POWER_MONITOR_H
