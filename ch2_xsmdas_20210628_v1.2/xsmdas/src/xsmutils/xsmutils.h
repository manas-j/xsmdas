/* XSMUTILS.H 
 *
 * Mithun NPS 
 * */
#ifndef XSMUTILS_H
#define XSMUTILS_H

#define XSMDAS_VERSION 1.2
#define MINIMUM_VERSION 1.1

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <string>
#include <fitsio.h>
#include <libgen.h>
#include<pil.h>

//Macro definitions
#define YES 1
#define NO 0


#define ISTINPUT 1  // THIS DEFINES THAT TIME IN DATA IS IN IST

// CONSTANTS 
#define FRAME_SIZE 2048
#define SPEC_SIZE 1024
//#define BDH_FRAME_SIZE 2107 (For data before 10th June 2017)
#define BDH_FRAME_SIZE 2059
#define L0_HEADER_SIZE 60
#define L0_FRAME_SIZE 2120 
#define COARSECH_LCBIN 10
#define HDR_SIZE 122 
#define LCSTART_BYTE 56
#define REFERENCE_DAC_GAIN 1.22070 //5000 mV 12 bit (using only LSB 10bits)
#define MONITOR_ADC_GAIN 0.00122070   //5 V 12 bit

#define N_RESPENEBINS 3000
#define RESP_EMIN 0.1
#define RESP_EBIN 0.01

#define CH2O_SPICE_ID -152

#define word(x,y) (x*256+y)  // This assumes big endian data

#define wordL(x,y) (x+y*256) //This assumes little endian data

#define GET_VARIABLE_NAME(Variable) (#Variable)

//template file names
#define SCIENCETEMPLATE "l1scienceTemplate"
#define HKTEMPLATE "hkfileTemplate"
#define SATEMPLATE "sunangfileTemplate"
#define GTITEMPLATE "gtiTemplate"
#define SPEC1TEMPLATE512 "spec1Template_512"
#define SPEC2TEMPLATE512 "spec2Template_512"
#define SPEC1TEMPLATE "spec1Template"
#define SPEC2TEMPLATE "spec2Template"
#define LC1TEMPLATE "lc1Template"
#define LC2TEMPLATE "lc2Template"
#define RESPTEMPLATE "respTemplate"
#define RESPTEMPLATE512 "respTemplate_512"
#define ARF1TEMPLATE "arf1Template"
#define ARF2TEMPLATE "arf2Template"

// Function definitions

int create_empty_fitsfile(string outFilename, string outTemplate);
int writeHistory2FITS(fitsfile *fout,vector <string> runHistory);
int getCaldbVer(string &caldbver);
int queryCaldb(string codename,double tstart,string &caldb_filename);
void print_banner(char *module_name);
bool FileExists(char *filename);
int isStringValid(string inString, vector<string> sampleStrings);
bool getMatchingString(string inString, vector<string> sampleStrings,int &istring);
bool checkRepetingString(vector<string> sampleStrings);
int doClobber(char *outfile,int clobber);
int getCurrentTime(string &timeString);
int validateDate(int UTCYear,int UTCMonth,int UTCDate);
double computeMET(long UTCYear,long UTCMonth,long UTCDate,long UTCHour,long UTCMin,long UTCSec,float UTCms);

#endif 
