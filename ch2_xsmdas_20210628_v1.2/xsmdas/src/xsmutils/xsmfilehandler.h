/* XSMFILEHANDLER.H
 *
 * Mithun NPS 
 * */
#ifndef XSMFILEHANDLER_H
#define XSMFILEHANDLER_H

#include <xsmutils.h>
#include <xsmdata.h>

class scienceFileHandler {
    public:
        string date_obs,date_end;
        double tstart,tstop,telapse;
        int create_l1science_file(string scienceFileName);
        int write_l1science_file(string scienceFileName,vector <scienceData> scDataVec,vector <string> runHistory);
        int read_l1science_file(string scienceFileName,vector <scienceData> &scDataVec);
};

class hkFileHandler {
	public:
        string date_obs,date_end;
        double tstart,tstop,telapse;        
		int create_hkfile(string hkFileName);
		int write_hkfile(string hkFileName,vector <framehdr> hdrDataVec,vector <string> runHistory);
		int read_hkfile(string hkfile,vector <hkData> &hkDataVec);
};

class saFileHandler {
    public:
        string date_obs,date_end;
        double tstart,tstop,telapse;
        int create_safile(string saFileName);
        int write_safile(string saFileName,vector <sunAngData> sunAngDataVec,vector <string> runHistory);
        int read_safile(string safile,vector <sunAngData> &sunAngDataVec);
};


class gtiFileHandler {
	public:
        string date_obs,date_end;
        double tstart,tstop,telapse;
        double tot_exposure;
		int create_gtifile(string gtiFileName);
		int write_gtifile(string gtiFileName,vector <gtiData> gtiDataVec,vector <string> runHistory);
		int read_gtifile(string gtiFile,vector <gtiData> &gtiDataVec);
};

class specFileHandler{
	public:
        string date_obs,date_end;
        string arffile;
        string respfile;
		int spectype;
		int chantype;
        int nchan;		
		int create_specfile(string specFileName);	
		int write_type1_specfile(string specFile,specData spec1Data,vector <string> runHistory);
		int write_type2_specfile(string specFile,vector <specData> spec2Data,vector <string> runHistory);
        int read_type1_specfile(string specFile,specData &spec1Data);
};

class arfFileHandler{
    public:
        string date_obs,date_end;
        int arftype;
        int create_arffile(string arfFileName);
        int write_type1_arffile(string arfFile,arfData arf1Data,vector <string> runHistory);
        int write_type2_arffile(string arfFile,vector <arfData> arf2DataVec,vector <string> runHistory);
        int read_type1_arffile(string arfFile,arfData &arf1Data);
    
};


class lcFileHandler{
    public:
        string date_obs,date_end;
        int lctype;
        int create_lcfile(string lcFileName);
        int write_type1_lcfile(string lcFile,lcData lc1Data,vector <string> runHistory);
        int write_type2_lcfile(string lcFile,vector <lcData> lc2Data,vector <string> runHistory);
};


class respFileHandler{
    public:
        int create_respfile(string respFileName,int nPIbin);
        int write_respfile(string respFile,float *Elo,float *Ehi,int *ngrp,int *fchan,int *nchan,float **rspmatrix,int n_Ebin,int n_PIbin,float *PImin,float *PImax,int *PIchannel,int  includearea);
};


#endif
