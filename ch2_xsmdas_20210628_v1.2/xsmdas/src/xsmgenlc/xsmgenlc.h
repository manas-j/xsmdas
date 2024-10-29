
#ifndef XSMGENLC_H
#define XSMGENLC_H

#include "xsmfilehandler.h"
#include "xsmdata.h"
#include <xsmutils.h>
#include "xsmarea.h"

class xsmgenlc {

    private:
        char l1file[PIL_LINESIZE];
        char lcfile[PIL_LINESIZE];
		char gtifile[PIL_LINESIZE];
        char safile[PIL_LINESIZE];
        //char areafile[PIL_LINESIZE];
        char abscoeffile[PIL_LINESIZE];
        char effareaparfile[PIL_LINESIZE];        
        char hkfile[PIL_LINESIZE];        
		char lctype_str[PIL_LINESIZE];
        char lcunit_str[PIL_LINESIZE];
        char eboundsfile[PIL_LINESIZE];
        char gainfile[PIL_LINESIZE];

		int lctype;
		int areascal;
        int enesel;
		int chstart;
		int chstop;
        double enelow;
        double enehigh;
		int PIstart;
        int PIstop;
        double tbinsize; 
		int tbinsizesec;
		int tbinsizems;
        int clobber;
        int history;

		long ntbins;
        double dataTstart, dataTstop;

        scienceFileHandler scfh;
	    lcFileHandler lcfh;
		gtiFileHandler gtifh;	
        saFileHandler safh;
        hkFileHandler hkfh;    

		vector <scienceData> scDataVec;
		vector <gtiData> gtiDataVec;
        vector <sunAngData> sunAngDataVec;
        vector <hkData> hkDataVec;

		lcData lcDataRecord;
        vector <lcData> lc2Data;

        int n_PIbin;
        float PIbinsize;
        float PIEmin;
        float PIEmax;

        double **areaArr;
        double *gainarr;
        double *offsetarr;

        vector <string> runHistory;

        //For Random number generation
        int Rand_ix,Rand_iy,Rand_iz;

    public:
		
		xsmgenlc();
		~xsmgenlc();
        int xsmgenlcProcess();
        int read(int argc,char **argv);
        //int read(char *l1file,char *l1file, int clobber, int history);
        int display();
		int generatelc1();
        int generatelc2();       
        int rebinPHA2PI(long *PHASpec, long *PISpec,double gain,double offset);
        int computeGain();
        double urand();

        //int computeRelArea(); 

};



#endif

