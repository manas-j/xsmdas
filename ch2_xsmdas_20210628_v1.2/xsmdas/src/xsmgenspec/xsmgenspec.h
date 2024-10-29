
#ifndef XSMGENSPEC_H
#define XSMGENSPEC_H

#include "xsmfilehandler.h"
#include "xsmdata.h"
#include <xsmutils.h>
#include "xsmarea.h"

class xsmgenspec {

    private:
        char l1file[PIL_LINESIZE];
        char specfile[PIL_LINESIZE];
        char arffile[PIL_LINESIZE];
		char gtifile[PIL_LINESIZE];
        char hkfile[PIL_LINESIZE];
        char safile[PIL_LINESIZE];		
		char spectype_str[PIL_LINESIZE];
		char chantype_str[PIL_LINESIZE];
        char sourcetype_str[PIL_LINESIZE];
		char eboundsfile[PIL_LINESIZE];
        char gainfile[PIL_LINESIZE];
        //char areafile[PIL_LINESIZE];
        char abscoeffile[PIL_LINESIZE];
        char effareaparfile[PIL_LINESIZE]; 
        char syserrfile[PIL_LINESIZE];
    
		int spectype;
		int chantype;
        int sourcetype;
        int areascal;
        int genarf;
		double tstart;
		double tstop;
        double gaincorfactor;
        double offsetcorfactor;
		int tbinsize;
        int addsyserror; 
        int clobber;
        int history;

		int n_PIbin;
		float PIbinsize;
		float PIEmin;
		float PIEmax;

		double *gainarr;
		double *offsetarr;
        double **areaArr;
        float *phasyserrfrac;
        int nchan_area;

        scienceFileHandler scfh;
		hkFileHandler hkfh;
	    specFileHandler specfh;
		gtiFileHandler gtifh;	
        saFileHandler safh;    
        arfFileHandler arffh;

		vector <scienceData> scDataVec;
		vector <hkData> hkDataVec;
        vector <sunAngData> sunAngDataVec;        
		vector <gtiData> gtiDataVec;
		specData spec1Data;
		vector <specData> spec2DataVec;
        arfData arf1Data;
        vector <arfData> arf2DataVec;

		//For Random number generation
		int Rand_ix,Rand_iy,Rand_iz;
    
        vector <string> runHistory;

	public:
		
		xsmgenspec();
		~xsmgenspec();
        int xsmgenspecProcess();
        int read(int argc,char **argv);
        //int read(char *l1file,char *l1file, int clobber, int history);
        int display();
		int generateSpec1();
		int generateSpec2();
		int rebinPHA2PI(long *PHASpec, long *PISpec,float *PIspec_syserr,double gain,double offset);
		int computeGain();
        //int computeRelArea();
		double urand();

};

int find_intersecting_range(vector<gtiData> gti1, vector<gtiData> gti2, vector<gtiData> &gtiout);

#endif

