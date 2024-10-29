
#ifndef XSMGTIGEN_H
#define XSMGTIGEN_H


#include "xsmutils.h"
#include "xsmfilehandler.h"
#include "xsmdata.h"

#define totParams 37
#define maxRanges 5

class xsmgtigen {

	private:
		char hkfile[PIL_LINESIZE];
		char safile[PIL_LINESIZE];
        char filterfile[PIL_LINESIZE];
		char gtifile[PIL_LINESIZE];
		char usergtifile[PIL_LINESIZE];
        int usesunang;
	    int clobber;
    	int history;
		int usergtistatus;

        hkFileHandler hkfh;
        gtiFileHandler gtifh;
	    saFileHandler safh;
		vector <hkData> hkDataVec;
		vector <sunAngData> sunAngDataVec;
		vector <gtiData> gtiDataVec;
        vector <gtiData> userGTI;


        string allPars[totParams];
        int paramSelFlag[totParams];
        string paramLimits[totParams][maxRanges*2];
        
        vector <string> runHistory;

	public:
        
        xsmgtigen();
        ~xsmgtigen();
		int xsmgtigenProcess();
		int computeGTI();
		int computehkGTI(vector <gtiData> &hkGTI);
        int computemandatoryGTI(vector <gtiData> &mandGTI);
        int readFilterFile();
		int readUserGtiFile();
        int validateRecord(long row);
        template <class T>
        bool isInRange(T pval,int nrange,string ranges[maxRanges*2]);

		int read(int argc,char **argv);
		int read(char *hkfile,char *safile,char *gtifile, int clobber, int history);
		int display();


};

int find_intersecting_range(vector<gtiData> gti1, vector<gtiData> gti2, vector<gtiData> &gtiout);
#endif
