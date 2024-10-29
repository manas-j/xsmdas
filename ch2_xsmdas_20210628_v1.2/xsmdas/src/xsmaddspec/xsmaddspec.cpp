#include <xsmaddspec.h>


xsmaddspec::xsmaddspec(){

	speclist.clear();
    arflist.clear();
    resplist.clear();
    date_obs_vec.clear();
    date_end_vec.clear();

	specDataVec.clear();
	arfDataVec.clear();

}

xsmaddspec::~xsmaddspec(){}

int xsmaddspec::read(int argc,char **argv)
{
    int status=0;


    runHistory.clear();
    runHistory.push_back("************** XSMADDSPEC **************");

    if(PIL_OK!=(status=PILInit(argc,argv))){
        cout<<"***Error Initializing PIL***\n";
        cout<<"*** $PFILES not set or parameter file missing ***\n";
        return status;
    }

    if(PIL_OK!=(status=PILGetFname("listfile",listfile))){
        cout<<"***Error reading input listfile***\n";
        return status;
    }

    if(!FileExists(listfile)){
        cout<<"***ERROR: This file does not exist: "<<listfile<<"***\n";
        return EXIT_FAILURE;
    }

    runHistory.push_back("listfile = "+(string)basename(listfile));


    if(PIL_OK!=(status=PILGetFname("specfile",specfile))){
        cout<<"***Error reading output Spectrum file***\n";
        return status;
    }

    runHistory.push_back("specfile = "+(string)basename(specfile));


    if (PIL_OK != (status = PILGetBool("addarf", &addarf))) {
        cout << "***Error Reading addarf ***\n";
        return status;
    }

    if(addarf) runHistory.push_back("addarf = yes");
    else runHistory.push_back("addarf = no");

    if(addarf)
    {
        if(PIL_OK!=(status=PILGetFname("arffile",arffile))){
            cout<<"***Error reading output ARF file***\n";
            return status;
        }

        if (strcasecmp(arffile, "default") == 0) {
            string phaname=string(specfile);
            string arfname=phaname.substr(0,phaname.size()-4)+".arf";
            sprintf(arffile,"%s",arfname.c_str());
        }
    }
    else
        sprintf(arffile,"%s","none");

    runHistory.push_back("arffile = "+(string)basename(arffile));


    if (PIL_OK != (status = PILGetBool("clobber", &clobber))) {
        cout << "***Error Reading clobber ***\n";
        return status;
    }

    if(clobber) runHistory.push_back("clobber = yes");
    else runHistory.push_back("clobber = no");

    runHistory.push_back("**************************************");

    PILClose(status);

    cout<<"------------------------------------------------------------------------\n";

    return (EXIT_SUCCESS);

}

int  xsmaddspec::display()
{
    return EXIT_SUCCESS;
}

int xsmaddspec::xsmaddspecProcess()
{

    int status=0;
	FILE *f1;
    char filenam[PIL_LINESIZE];
	long i,j;
    specFileHandler inspecfh;
    arfFileHandler inarffh;

    runHistory.push_back("*******  Input spectrum files *******");

    f1=fopen(listfile,"r");
	while(fscanf(f1,"%s\n",filenam)!=EOF) {

	    if(!FileExists(filenam)){
    	    cout<<"***ERROR: This file does not exist: "<<filenam<<"***\n";
        	return EXIT_FAILURE;
    	}
        
        speclist.push_back(filenam);

		runHistory.push_back((string)basename(filenam));		
    }

    runHistory.push_back("**************************************");

    fclose(f1);

    nfiles=speclist.size();

    cout<<"MESSAGE: Number of input spectral files = "<<nfiles<<"\n";

	//Read all input spectrum files

    for(i=0;i<nfiles;i++)
    {
        specData spec1Data;
        inspecfh.read_type1_specfile(speclist[i],spec1Data);
        specDataVec.push_back(spec1Data);

		string basepath=(string)dirname((char *)speclist[i].c_str());
        arflist.push_back(basepath+"/"+inspecfh.arffile);
        resplist.push_back(inspecfh.respfile);
        date_obs_vec.push_back(inspecfh.date_obs);
        date_end_vec.push_back(inspecfh.date_end);
    } 

	// Read all input arf files if needed
    if(addarf){

        for(i=0;i<nfiles;i++)
        {

	        if(!FileExists((char *)arflist[i].c_str())){
    	        cout<<"***ERROR: This ARF file does not exist: "<<arflist[i].c_str()<<"***\n";
        	    return EXIT_FAILURE;
        	}

            arfData arf1Data;
            inarffh.read_type1_arffile(arflist[i],arf1Data);
            arfDataVec.push_back(arf1Data);
        }
    }
    
    // Check consistency and add spectra to get output spectra and arfs to get output arf (if needed)
    nchan=specDataVec[0].nchan;
    chantype=specDataVec[0].chantype;
    if(addarf) nrespchan=arfDataVec[0].nchan;

    tstart=specDataVec[0].tstart;
    tstop=specDataVec[nfiles-1].tstop;
    specfh.date_obs=date_obs_vec[0];
    specfh.date_end=date_end_vec[nfiles-1];

    outSpecData.initialize(chantype,nchan);
    if (addarf) outArfData.initialize();

    for(i=0;i<nfiles;i++)
    {
        if(specDataVec[i].nchan==nchan&&specDataVec[i].chantype==chantype)
        {
            for(j=0;j<nchan;j++)
            {
                outSpecData.spectrum[j]+=specDataVec[i].spectrum[j];
                if(chantype==2) outSpecData.syserror[j]+=specDataVec[i].syserror[j];
            }
        
            outSpecData.exposure+=specDataVec[i].exposure;

            if(specDataVec[i].tstart<tstart)
            {
                tstart=specDataVec[i].tstart;
                specfh.date_obs=date_obs_vec[i];
            }
            if(specDataVec[i].tstop>tstop)
            {
                tstop=specDataVec[i].tstop;
                specfh.date_end=date_end_vec[i];                
            }

            if(addarf)
            {
                if(arfDataVec[i].nchan==nrespchan){

                    for(j=0;j<nrespchan;j++)
                        outArfData.area[j]+=arfDataVec[i].area[j]*specDataVec[i].exposure;

                    outArfData.exposure+=specDataVec[i].exposure;
                }
                else{
                    cout<<"***ERROR: Inconsistent ARF channel numbers\n";
                    cout<<"***ERROR: Exiting without output\n";
                    return EXIT_FAILURE;
                }
            }

        }
        else
        {
            cout<<"***ERROR: Inconsistent chantype or number of channels. Can not add spectra\n";
            cout<<"***ERROR: Exiting without output\n";
            return EXIT_FAILURE;
        }
    }

    outSpecData.setTimes(tstart,tstop);
    if(addarf) outArfData.setTimes(tstart,tstop);

    arffh.date_obs=specfh.date_obs;
    arffh.date_end=specfh.date_end;    

    // Set ANCRFILE AND RESPFILE IN CASE OF ADDARF
    if(addarf)
    {
        specfh.arffile=(string)arffile;
        specfh.respfile=resplist[0];
    }
    else
    {
        specfh.arffile="";
        specfh.respfile="";
    }

    // Write output spectrum file

    status=doClobber(specfile,clobber);
    if(status) return (status);

    specfh.spectype=1;   		// Defines type-I or II
    specfh.chantype=chantype;   // Defines PHA or PI
	specfh.nchan=nchan;			// Number of channels

    status=specfh.create_specfile((string)specfile);

    if(status)
    {
        cout<<"Error in creating spectrum file "<<specfile<<"\n";
        return EXIT_FAILURE;
    }

    status=specfh.write_type1_specfile((string)specfile,outSpecData,runHistory);

    if(status)
    {
        cout<<"Error in writing to spectrum file "<<specfile<<"\n";
        return EXIT_FAILURE;
    }

    // Write ARF to file

    if(addarf)
    {

        status=doClobber(arffile,clobber);
        if(status) return (status);

        arffh.arftype=1;   // Defines type-I or II

        status=arffh.create_arffile((string)arffile);

        if(status){
            cout<<"Error in creating arf file "<<arffile<<"\n";
            return EXIT_FAILURE;
        }
        
        status=arffh.write_type1_arffile((string)arffile,outArfData,runHistory);

        if(status){
            cout<<"Error in writing to arf file "<<arffile<<"\n";
            return EXIT_FAILURE;
        }

	}

    for(i=0;i<nfiles;i++) specDataVec[i].makeFree();
    outSpecData.makeFree();

    cout<<"\nMESSAGE: XSMADDSPEC completed successully\n";
    cout<<"MESSAGE: Output file = "<<specfile<<"\n";
    if(addarf) cout<<"MESSAGE: Output ARF = "<<arffile<<"\n";
    cout<<"\n";


    return EXIT_SUCCESS;
	
}
