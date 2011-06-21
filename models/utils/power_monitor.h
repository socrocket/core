/*##############################################*/
/*# FILE : power_monitor.h                      */
/*#                                             */
/*# description :                               */
/*# header file for PowerMonitor class   */
/*##############################################*/

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
    static int maxLevel;
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
  static void send(sc_module* ip, IpPowerData data);
  static void analyze(string infile, string outfile);

  static void logprint(string file);
  //----------------------------------------------

};

#endif // POWER_MONITOR_H
