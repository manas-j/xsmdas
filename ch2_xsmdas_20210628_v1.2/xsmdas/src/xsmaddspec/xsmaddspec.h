#ifndef XSMADDSPEC_H
#define XSMADDSPEC_H

#include "xsmfilehandler.h"
#include "xsmdata.h"
#include <xsmutils.h>

class xsmaddspec {

    private:
        char listfile[PIL_LINESIZE];
        char specfile[PIL_LINESIZE];
        char arffile[PIL_LINESIZE];

        int addarf;
        int clobber;
 
        int nfiles;
        int nchan;
        int chantype;
        int nrespchan;

        double tstart,tstop;

		vector <string> speclist;
        vector <string> arflist;
        vector <string> resplist;
        vector <string> date_obs_vec;
        vector <string> date_end_vec;
		vector <specData> specDataVec;
        vector <arfData> arfDataVec;

        vector <string> runHistory;
        
        specData outSpecData;
        arfData outArfData;

        specFileHandler specfh;
        arfFileHandler arffh;
                
    public:

        xsmaddspec();
        ~xsmaddspec();
        int xsmaddspecProcess();
        int read(int argc,char **argv);
        int display();

};

#endif

