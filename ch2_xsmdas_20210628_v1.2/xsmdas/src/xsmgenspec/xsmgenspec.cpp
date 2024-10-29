#include <xsmgenspec.h>


xsmgenspec::xsmgenspec(){
	scDataVec.clear();
	spec2DataVec.clear();
	arf2DataVec.clear();
	spectype=2;

    //Initialize for random number
    Rand_ix=5;
    Rand_iy=11;
    Rand_iz=17;

}

xsmgenspec::~xsmgenspec(){}

int xsmgenspec::read(int argc,char **argv)
{
    int status=0;


    runHistory.clear();
    runHistory.push_back("************** XSMGENSPEC **************");

    if(PIL_OK!=(status=PILInit(argc,argv))){
        cout<<"***Error Initializing PIL***\n";
        cout<<"*** $PFILES not set or parameter file missing ***\n";
        return status;
    }

    if(PIL_OK!=(status=PILGetFname("l1file",l1file))){
        cout<<"***Error reading input level-1 file***\n";
        return status;
    }

    if(!FileExists(l1file)){
        cout<<"***ERROR: This file does not exist: "<<l1file<<"***\n";
        return EXIT_FAILURE;
    }

    if(PIL_OK!=(status=PILGetFname("specfile",specfile))){
        cout<<"***Error reading output Spectrum file***\n";
        return status;
    }

    runHistory.push_back("l1file = "+(string)basename(l1file));
    runHistory.push_back("specfile = "+(string)basename(specfile));

/*
	vector <string> inStrings;

    inStrings.clear();
    inStrings.push_back("time-resolved");
    inStrings.push_back("time-integrated");
    do {
        if (PIL_OK != (status = PILGetString("spectype", spectype_str))) {
            cout << "***Error reading output spectrum type***\n";
            return status;
        }
    } while (isStringValid((string) spectype_str, inStrings));

*/

    if (PIL_OK != (status = PILGetString("spectype", spectype_str))) {
        cout << "***Error reading output spectrum type***\n";
        return status;
    }

    if (strcasecmp(spectype_str, "time-integrated") == 0) {
        spectype = 1;
        runHistory.push_back("spectype = time-integrated");
    } else if (strcasecmp(spectype_str, "time-resolved") == 0) {
        spectype = 2;
        runHistory.push_back("spectype = time-resolved");
	} else{
        cout<<"***ERROR: Invalid input for spectype. Valid inputs are time-integrated or time-resolved\n";
        return EXIT_FAILURE;
    }

/*    
    inStrings.clear();
    inStrings.push_back("PHA");
    inStrings.push_back("PI");

    do {
        if (PIL_OK != (status = PILGetString("chantype", chantype_str))) {
            cout << "***Error reading output channel type***\n";
            return status;
		}
   	} while (isStringValid((string) chantype_str, inStrings));
*/

    if (PIL_OK != (status = PILGetString("chantype", chantype_str))) {
        cout << "***Error reading output channel type***\n";
        return status;
    }
    
    if (strcasecmp(chantype_str, "PHA") == 0) {
        chantype = 1;
        runHistory.push_back("chantype = PHA");
    } else if (strcasecmp(chantype_str, "PI") == 0) {
        chantype = 2;
        runHistory.push_back("chantype = PI");
    } else {
        cout<<"***ERROR: Invalid input for chantype. Valid inputs are PHA or PI\n";
        return EXIT_FAILURE;
    }

    // if chantype==PI, find whether source is Sun or cal to decide gain correction method
    if(chantype==2)
    {
        /*
        inStrings.clear();
        inStrings.push_back("cal");
        inStrings.push_back("sun");
        do {
            if (PIL_OK != (status = PILGetString("sourcetype", sourcetype_str))) {
                cout << "***Error reading source type***\n";
                return status;
            }
        } while (isStringValid((string) sourcetype_str, inStrings));

        */
        
        if (PIL_OK != (status = PILGetString("sourcetype", sourcetype_str))) {
            cout << "***Error reading source type***\n";
            return status;
        }
        
        if (strcasecmp(sourcetype_str, "cal") == 0) {
            sourcetype = 1;
            runHistory.push_back("sourcetype = cal");
        } else if (strcasecmp(sourcetype_str, "sun") == 0) {
            sourcetype = 2;
            runHistory.push_back("sourcetype = sun");
        } else{
            cout<<"***ERROR: Invalid input for sourcetype. Valid inputs are cal or sun\n";
            return EXIT_FAILURE;
        }
    }

    // Whether to generate ARF corresponding to not areascaled spectrum with PI channel only

    if (chantype==2)
    {
        if (PIL_OK != (status = PILGetBool("genarf", &genarf))) {
            cout << "***Error Reading genarf***\n";
            return status;
        }
    }
    else
        genarf=0;
        
    if(genarf) runHistory.push_back("genarf = yes");
    else runHistory.push_back("genarf = no");

    if(genarf)
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
    
    if(genarf==0)
    {
        // Whether to apply area scaling to spectrum
        if (PIL_OK != (status = PILGetBool("areascal", &areascal))) {
            cout << "***Error Reading areascal***\n";
            return status;
        }
    }

    if(areascal) runHistory.push_back("areascal = yes");
    else runHistory.push_back("areascal = no");
 
    char tempStr[100];

    if (PIL_OK != (status = PILGetReal("tstart", &tstart))) {
        cout << "***Error reading Start time***\n";
        return status;
    }

    if (PIL_OK != (status = PILGetReal("tstop", &tstop))) {
        cout << "***Error reading Stop time***\n";
        return status;
    }

    if(tstop<=tstart && tstart!=0){
        cout<<"***Error: Invalid tstart and tstop values. tstop should be > tstart\n";
        return EXIT_FAILURE;
    }

    sprintf(tempStr,"Tstart = %lf",tstart);
    runHistory.push_back((string)tempStr);
    sprintf(tempStr,"Tstop = %lf",tstop);
    runHistory.push_back((string)tempStr);

    if(spectype==2)
    {
        if (PIL_OK != (status = PILGetInt("tbinsize", &tbinsize))) {
            cout << "***Error reading Stop time***\n";
            return status;
        }

        if(tbinsize<1){
            cout<< "***Error: Invalid time bin size. tbinsize should be >=1\n";
            return EXIT_FAILURE;
        }

        sprintf(tempStr,"Tbinsize = %d",tbinsize);
        runHistory.push_back((string)tempStr);
    }


    if (PIL_OK != (status = PILGetBool("addsyserror", &addsyserror))) {
        cout << "***Error Reading addsyserror parmeter ***\n";
        return status;
    }

    if(addsyserror) runHistory.push_back("addsyserror = yes");
    else runHistory.push_back("addsyserror = no");

    if(PIL_OK!=(status=PILGetFname("hkfile",hkfile))){
        cout<<"***Error reading HK file***\n";
        return status;
    }

    if(!FileExists(hkfile)){
        cout<<"***ERROR: This file does not exist: "<<hkfile<<"***\n";
        return EXIT_FAILURE;
    }

    runHistory.push_back("hkfile = "+(string)basename(hkfile));

    //Read safile if (sourcetype==sun and chantype==PI or areascal==yes or genarf==yes)
    if((chantype==2&&sourcetype==2)||areascal||genarf){

        if(PIL_OK!=(status=PILGetFname("safile",safile))){
            cout<<"***Error reading Sun angle file***\n";
            return status;
        }
        if(!FileExists(safile)){
            cout<<"***ERROR: This file does not exist: "<<safile<<"***\n";
            return EXIT_FAILURE;
        }

        runHistory.push_back("safile = "+(string)basename(safile));

    }

    if(PIL_OK!=(status=PILGetFname("gtifile",gtifile))){
        cout<<"***Error reading GTI file***\n";
        return status;
    }

    if(!FileExists(gtifile)){
        cout<<"***ERROR: This file does not exist: "<<gtifile<<"***\n";
        return EXIT_FAILURE;
    }

    runHistory.push_back("gtifile = "+(string)basename(gtifile));

    // If spectype==PI read ebounds and gain
	if(chantype==2)
	{
        if (PIL_OK != (status = PILGetFname("eboundsfile", eboundsfile))) {
            cout << "***Error reading eboundsfile name***\n";
            return status;
        }

        if(strcasecmp(eboundsfile, "CALDB") != 0 && (!FileExists(eboundsfile))){
            cout<<"***ERROR: This file does not exist: "<<eboundsfile<<"***\n";
            return EXIT_FAILURE;
        }

        if (PIL_OK != (status = PILGetFname("gainfile", gainfile))) {
            cout << "***Error reading gain file name***\n";
            return status;
        }

        if(strcasecmp(gainfile, "CALDB") != 0 && !FileExists(gainfile)){
            cout<<"***ERROR: This file does not exist: "<<gainfile<<"***\n";
            return EXIT_FAILURE;
        }

        if (PIL_OK != (status = PILGetReal("gaincorfactor", &gaincorfactor))) {
            cout << "***Error reading Gain Correction factor***\n";
            return status;
        }

        if (PIL_OK != (status = PILGetReal("offsetcorfactor", &offsetcorfactor))) {
            cout << "***Error reading Offset Correction factor***\n";
            return status;
        }

        runHistory.push_back("eboundsfile = "+(string)(eboundsfile));
        runHistory.push_back("gainfile = "+(string)(gainfile));

        sprintf(tempStr,"gaincorfactor = %lf",gaincorfactor);
        runHistory.push_back((string)tempStr);

        sprintf(tempStr,"offsetcorfactor = %lf",offsetcorfactor);
        runHistory.push_back((string)tempStr);

	}

    //If genarf||areascal==yes read abscoeffile and effareaparfile 
    if(genarf||areascal){

        if (PIL_OK != (status = PILGetFname("abscoeffile", abscoeffile))) {
            cout << "***Error reading area file name***\n";
            return status;
        }

        if(strcasecmp(abscoeffile, "CALDB") != 0 && !FileExists(abscoeffile)){
            cout<<"***ERROR: This file does not exist: "<<abscoeffile<<"***\n";
            return EXIT_FAILURE;
        }

        runHistory.push_back("abscoeffile = "+(string)(abscoeffile));

        if (PIL_OK != (status = PILGetFname("effareaparfile", effareaparfile))) {
            cout << "***Error reading area file name***\n";
            return status;
        }

        if(strcasecmp(effareaparfile, "CALDB") != 0 && !FileExists(effareaparfile)){
            cout<<"***ERROR: This file does not exist: "<<effareaparfile<<"***\n";
            return EXIT_FAILURE;
        }

        runHistory.push_back("effareaparfile = "+(string)(effareaparfile));

    }       

    if(addsyserror && chantype==2){

        if (PIL_OK != (status = PILGetFname("syserrfile", syserrfile))) {
            cout << "***Error reading syserr file name***\n";
            return status;
        }

        if(strcasecmp(syserrfile, "CALDB") != 0 && !FileExists(syserrfile)){
            cout<<"***ERROR: This file does not exist: "<<syserrfile<<"***\n";
            return EXIT_FAILURE;
        }

    }

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

int  xsmgenspec::display()
{
    return EXIT_SUCCESS;
}

int xsmgenspec::xsmgenspecProcess()
{
    int status=0;
	long nrows,i,j;
	double dataTstart, dataTstop;
    long nrowsSa,nrowsHk,nrowsSc;
    string xsmbasePath = getenv("xsmdas");

	scDataVec.clear();
    status=scfh.read_l1science_file((string)l1file,scDataVec);

    if(status)
    {
        cout<<"Error in reading level-1 file\n";
        return EXIT_FAILURE;
    }
  
	nrows=scDataVec.size();

	dataTstart=scDataVec[0].time;
	dataTstop=scDataVec[nrows-1].time+1.0;
    
	if(tstart==0&&tstop==0) 
	{
		tstart=dataTstart-0.0001;
		tstop=dataTstop-0.0001;
    }
    else if (tstop<=tstart)
    {
        cout<<"*** Error: Provided tstart and tstop invalid. Tstop <= Tstart\n";
        return EXIT_FAILURE;
    }
	else if(tstart<dataTstart||tstop>dataTstop)
	{
		cout<<"***Error: Level-1 file doesn't span from given tstart to tstop\n";
		cout<<"***Error: Exiting without generating spectrum\n";
		return EXIT_FAILURE;
	}

    vector <gtiData> gtiFileDataVec, userGTI;
    
    //set User GTI selection (tstart & tstop)
    userGTI.clear();
    gtiData gtiRecord;
    gtiRecord.tstart=tstart;
    gtiRecord.tstop=tstop;
    userGTI.push_back(gtiRecord);
    
	gtiFileDataVec.clear();
	status=gtifh.read_gtifile((string)gtifile,gtiFileDataVec);

    if(status)
    {
        cout<<"***Error in reading GTI file\n";
        return EXIT_FAILURE;
    }

    if(gtiFileDataVec.size()==0)
    {
        cout<<"***Error: Provided GTI file has zero records\n";
        return EXIT_FAILURE;
    }

    gtiDataVec.clear();
    status=find_intersecting_range(gtiFileDataVec, userGTI, gtiDataVec);

    if(status||gtiDataVec.size()==0)
    {
        cout<<"***Error: No GTI for the duration specified by the user\n";
        return EXIT_FAILURE;
    }

    status=gtiFilterScienceData(scDataVec,gtiDataVec);

    if(status)
    {
        cout<<"***Error in filtering L1 file with GTI\n";
        return EXIT_FAILURE;
    }

	// Read HK file and filter with GTI  

	hkDataVec.clear();
	status=hkfh.read_hkfile((string)hkfile,hkDataVec);

   	if(status)
    {
    	cout<<"***Error in reading HK file\n";
    	return EXIT_FAILURE;
    }

	status=gtiFilterHkData(hkDataVec,gtiDataVec);

  	if(status)
    {
    	cout<<"Error in filtering HK data with GTI\n";
    	return EXIT_FAILURE;
    }


	nrowsHk=hkDataVec.size();
	nrowsSc=scDataVec.size();

	if(nrowsHk!=nrowsSc)
	{
		cout<<"ERROR: Unequal entries in science data and HK data\n";
		cout<<"ERROR: Unable to proceed further, quitting... \n";
		return EXIT_FAILURE;
	}
    
	
    // Read Sun angle if spectype==PI (2) and sourcetype==Sun(2) for gain correction
    // or if areascal==y or if genarf==y 
    // Then filter sunAngDataVec with GTI

    if((chantype==2&&sourcetype==2)||areascal==1||genarf==1) 
    {
        sunAngDataVec.clear();
        status=safh.read_safile((string)safile,sunAngDataVec);
    
        if(status){
            cout<<"***Error in reading SunAngle file\n";
            return EXIT_FAILURE;
        }      

        status=gtiFiltersunAngData(sunAngDataVec,gtiDataVec);

        if(status)
        {
            cout<<"Error in filtering SunAng data with GTI\n";
            return EXIT_FAILURE;
        }
        
        nrowsSa=sunAngDataVec.size();
        nrowsSc=scDataVec.size();

        if(nrowsSa!=nrowsSc)
        {
            cout<<"ERROR: Unequal entries in science data and SunAng data\n";
            cout<<"ERROR: Unable to proceed further, quitting... \n";
            return EXIT_FAILURE;
        }

    }
    
	// If bin type is PI, read Ebounds for binning information
    // Open the caldb ebounds file and read PI channel information
	
	if(chantype==2)
	{

        if(strcasecmp(eboundsfile, "CALDB")==0)
        {
            string caldbeboundsfile;
            status=queryCaldb("EBOUNDS",tstart,caldbeboundsfile);
            if(status) return(status);
            sprintf(eboundsfile,"%s",caldbeboundsfile.c_str());
            cout<<"MESSAGE: Ebounds CALDB file used is: "<<eboundsfile<<"\n";
        }

	    fitsfile *febounds;

	    fits_open_file(&febounds,eboundsfile,READONLY,&status);
    	if(status) { 
            cout<<"***Error in opening eboundsfile "<<eboundsfile<<"\n";
            cout<<"***Incorrect format file\n";
            fits_report_error(stderr,status);  
            return (EXIT_FAILURE); 
        }

	    fits_movnam_hdu(febounds, BINARY_TBL, "EBOUNDS", 0, &status);
        if(status) { 
            cout<<"***Error in getting EBOUNDS extenstion in "<<eboundsfile<<"\n";
            cout<<"***Incorrect format file\n";
            fits_report_error(stderr,status);  
            return (EXIT_FAILURE); 
        }

		int naxis2;

	    fits_read_key(febounds,TINT,"NAXIS2",&naxis2,NULL, &status);
    	n_PIbin=naxis2;
        
		int *PIchannel;
		float *PImin,*PImax;
	    PIchannel=(int *)malloc(sizeof(int)*n_PIbin);
    	PImin=(float *)malloc(sizeof(float)*n_PIbin);
	    PImax=(float *)malloc(sizeof(float)*n_PIbin);

    	fits_read_col(febounds,TINT,1,1,1,n_PIbin,NULL,PIchannel,NULL,&status);
	    fits_read_col(febounds,TFLOAT,2,1,1,n_PIbin,NULL,PImin,NULL,&status);
    	fits_read_col(febounds,TFLOAT,3,1,1,n_PIbin,NULL,PImax,NULL,&status);

    	if(status)
    	{
        	cout<<"***Error in reading ebounds file "<<eboundsfile<<"\n";
        	return EXIT_FAILURE;
    	}

        PIbinsize=PImax[0]-PImin[0];

	    fits_close_file(febounds,&status);
    	if(status) { fits_report_error(stderr,status);  return (EXIT_FAILURE); }

		PIEmin=PImin[0];
		PIEmax=PImax[n_PIbin-1];

        free(PIchannel);
        free(PImin);
        free(PImax);

	} 
	else 
	{
		n_PIbin=0;
		PIEmin=0;
		PIEmax=0;
		PIbinsize=0;
	}


	// If bin type is PI, compute gain and offset for each second

    if(chantype==2)
	{
		gainarr=(double *)malloc(sizeof(double)*scDataVec.size());
    	offsetarr=(double *)malloc(sizeof(double)*scDataVec.size());

		for(i=0;i<scDataVec.size();i++)
		{
			gainarr[i]=0.;
			offsetarr[i]=0.;
		}

		status=computeGain();

        if(status)
        {
            cout<<"ERROR: Error in computing gain and offset\n";
            return EXIT_FAILURE;
        }
		
	}

    // Compute effective area for each second for genarf or arescal (relative)

	int relFlag,eneType;
    //eneType: 1-PI, 2-PHA, 3-RSP

    if(genarf)
    {
        nchan_area=3000; // response energy bins
		eneType=3;
		relFlag=0;

        areaArr=(double **)malloc(sizeof(double*)*nrowsSc);
        for(i=0;i<nrowsSc;i++)
        {
            areaArr[i]=(double*)malloc(sizeof(double)*nchan_area);
            for(j=0;j<nchan_area;j++) areaArr[i][j]=0.0;
        }

	    status=get_effectivearea(sunAngDataVec,hkDataVec,areaArr,nchan_area,eneType,relFlag,abscoeffile,effareaparfile,tstart);

        if(status)
        {
            cout<<"ERROR: Error in computing area as function of time\n";
            return EXIT_FAILURE;
        }
        
    }    
    else if(areascal)
    {

		relFlag=1;

        if(chantype==1) {
            nchan_area=SPEC_SIZE;     // No. of PHA bins
            eneType=2;                // PHA   
        }
        else if(chantype==2){
            nchan_area=n_PIbin;  // No. of PI bins
            eneType=1;           // PI
        }

        areaArr=(double **)malloc(sizeof(double*)*nrowsSc);
        for(i=0;i<nrowsSc;i++)
        {
            areaArr[i]=(double*)malloc(sizeof(double)*nchan_area);
            for(j=0;j<nchan_area;j++) areaArr[i][j]=1.0;
        }

        status=get_effectivearea(sunAngDataVec,hkDataVec,areaArr,nchan_area,eneType,relFlag,abscoeffile,effareaparfile,tstart);

        if(status)
        {
            cout<<"ERROR: Error in computing area as function of time\n";
            return EXIT_FAILURE;
        }        
    }

    // Get systematic errors
	int nchan_sys;
    if(chantype==1) nchan_sys=SPEC_SIZE;     // 1024
    else if(chantype==2) nchan_sys=n_PIbin;  // 512 (9-bit PHA)

    phasyserrfrac=(float *)malloc(sizeof(float)*nchan_sys);

    for (i=0;i<nchan_sys;i++) phasyserrfrac[i]=0.0;

    if(addsyserror && chantype==2){

        /*
        FILE *fsys;
        string syserrfile = xsmbasePath+"/caldb/systematic_error_XSM_PHA_512.txt";
        fsys=fopen(syserrfile.c_str(),"r");
        cout<<"MESSAGE: Syserror CALDB file used is"<<syserrfile<<"\n";
        for (i=0;i<512;i++) fscanf(fsys,"%f\n",&phasyserrfrac[i]);
        fclose(fsys);
        */

        if(strcasecmp(syserrfile, "CALDB")==0)
        {
            string caldbsyserrfile;
            status=queryCaldb("SYSERR",tstart,caldbsyserrfile);
            if(status) return(status);
            sprintf(syserrfile,"%s",caldbsyserrfile.c_str());
            cout<<"MESSAGE: Syserror CALDB file used is: "<<syserrfile<<"\n";
        }

        fitsfile *fsyserr;

        fits_open_file(&fsyserr,syserrfile,READONLY,&status);
        if(status) {
            cout<<"***Error in opening syserrfile "<<syserrfile<<"\n";
            cout<<"***Incorrect format file\n";
            fits_report_error(stderr,status);
            return (EXIT_FAILURE);
        }

        fits_movnam_hdu(fsyserr, BINARY_TBL, "SYSERR", 0, &status);
        if(status) {
            cout<<"***Error in getting SYSERR extenstion in "<<syserrfile<<"\n";
            cout<<"***Incorrect format file\n";
            fits_report_error(stderr,status);
            return (EXIT_FAILURE);
        }

        fits_read_col(fsyserr,TFLOAT,2,1,1,512,NULL,phasyserrfrac,NULL,&status);

        if(status)
        {
            cout<<"***Error in reading syserr file "<<syserrfile<<"\n";
            return EXIT_FAILURE;
        }

        fits_close_file(fsyserr,&status);
        if(status) { fits_report_error(stderr,status);  return (EXIT_FAILURE); }

    }


	// Generate spectrum 

	if(spectype==1)
		status=generateSpec1();
	else
		status=generateSpec2();

	if(status)
	{
		cout<<"Error in generating spectrum\n";
		return EXIT_FAILURE;
	}


    // Copy header keywords from Science file to SPEC and ARF
    specfh.date_obs=scfh.date_obs;
    specfh.date_end=scfh.date_end;

    // Set ANCRFILE AND RESPFILE IN CASE OF GENARF
    if(genarf)
    {
	    specfh.arffile=(string)basename(arffile);
        string caldbrmffile;
        status=queryCaldb("RMF",tstart,caldbrmffile);
        if(status) return(status);
        //specfh.respfile=(string)basename((char *)caldbrmffile.c_str());
        specfh.respfile=(string)((char *)caldbrmffile.c_str());
    }
    else
    {
        specfh.arffile="";
        specfh.respfile="";
    }

    // Set RESPFILE IN NO ARF CASE
    if(chantype==2 and genarf==0 and areascal!=0)
    {
        string caldbrspfile;
        status=queryCaldb("RSP_OPEN",tstart,caldbrspfile);
        if(status) return(status);
        specfh.respfile=(string)basename((char *)caldbrspfile.c_str());
    }

	arffh.date_obs=scfh.date_obs;
	arffh.date_end=scfh.date_end;
    
	// Write Spectrum to file

    status=doClobber(specfile,clobber);
    if(status) return (status);

	specfh.spectype=spectype;	// Defines type-I or II
	specfh.chantype=chantype;	// Defines PHA or PI

    if(chantype==1) specfh.nchan=SPEC_SIZE;		// No. of PHA bins
    else if(chantype==2) specfh.nchan=n_PIbin;  // No. of PI bins

	status=specfh.create_specfile((string)specfile);

    if(status)
    {
        cout<<"Error in creating spectrum file "<<specfile<<"\n";
        return EXIT_FAILURE;
    }

	if(spectype==1)
    {
		status=specfh.write_type1_specfile((string)specfile,spec1Data,runHistory);
        spec1Data.makeFree();
    }
	else
    {
		status=specfh.write_type2_specfile((string)specfile,spec2DataVec,runHistory);
        for (i=0;i<spec2DataVec.size();i++) spec2DataVec[i].makeFree();
    }

    if(status)
    {
        cout<<"Error in writing to spectrum file "<<specfile<<"\n";
        return EXIT_FAILURE;
    }

    // Write ARF to file

    if(genarf)
    {

    	status=doClobber(arffile,clobber);
    	if(status) return (status);

    	arffh.arftype=spectype;   // Defines type-I or II

    	status=arffh.create_arffile((string)arffile);

	    if(status)
    	{
        	cout<<"Error in creating arf file "<<arffile<<"\n";
        	return EXIT_FAILURE;
    	}

	    if(spectype==1)
    	{
            status=arffh.write_type1_arffile((string)arffile,arf1Data,runHistory);
        }
        else
        {
            status=arffh.write_type2_arffile((string)arffile,arf2DataVec,runHistory);
        }

	    if(status)
    	{
        	cout<<"Error in writing to arf file "<<arffile<<"\n";
        	return EXIT_FAILURE;
    	}

    }


    free(phasyserrfrac);

    if(chantype==2){

        free(gainarr);
        free(offsetarr);
    }

    if(areascal||genarf) {
        for (i=0;i<nrowsSc;i++) free(areaArr[i]);
        free(areaArr);
    }

    cout<<"\nMESSAGE: XSMGENSPEC completed successully\n";
    cout<<"MESSAGE: Output file = "<<specfile<<"\n";
	if(genarf) cout<<"MESSAGE: Output ARF = "<<arffile<<"\n";
	cout<<"\n";
	
	
    return EXIT_SUCCESS;
}

int xsmgenspec::generateSpec1()
{
	frame F;
	long i,nrows;
	double fexp;

	double gain=0.0163;
	double offset=0.0;

	nrows=scDataVec.size();
	
	spec1Data.initialize(chantype,n_PIbin);
	if (genarf) arf1Data.initialize();

	for(i=0;i<nrows;i++)
	{
		if(scDataVec[i].time >= tstart && scDataVec[i].time < tstop)
		{
			F.decode_frame(scDataVec[i].dataArray,0,0,1);
			fexp=1.0;

			if (chantype==1) 
            {
                if(areascal){
                    spec1Data.addSpecArea(F.spec.spectrum,phasyserrfrac,fexp,areaArr[i]);
                }
                else
                    spec1Data.addSpec(F.spec.spectrum,phasyserrfrac,fexp);
            }
			else 
            {
				gain=gainarr[i];
				offset=offsetarr[i];
				long PIspec[n_PIbin];
				float PIspec_syserr[n_PIbin];
				rebinPHA2PI(F.spec.spectrum,PIspec,PIspec_syserr,gain,offset);
                if(areascal){
                    spec1Data.addSpecArea(PIspec,PIspec_syserr,fexp,areaArr[i]);
                }
                else
                {
                    spec1Data.addSpec(PIspec,PIspec_syserr,fexp);
				    if(genarf)
                        arf1Data.addArea(areaArr[i],fexp);
                }
			}
		}
		else if(scDataVec[i].time >=tstop)
		{
			break;
		}
	}

    int ngti=gtiDataVec.size();

	spec1Data.setTimes(gtiDataVec[0].tstart,gtiDataVec[ngti-1].tstop);
    if(genarf) arf1Data.setTimes(gtiDataVec[0].tstart,gtiDataVec[ngti-1].tstop);

	return EXIT_SUCCESS;
}

int xsmgenspec::generateSpec2()
{

    frame F;
    long i,nrows;
    double fexp;

	double specTstart;
	double specTstop;

    double gain=0.0163;
    double offset=0.;
    double area=1.0;

	//NOTE: This code used to give memory leak if singspecData is defined in this function
	// Hence it is defined in the class definition. Do not know why.
    // Fixed the problem with proper freeing of memory in specData class (20/10/18)	

    specData singspecData;
    arfData singarfData;

    long ntbins=(long)((tstop-tstart)/(double)tbinsize)+1;
	long current_tbin=0,prev_tbin=-1;

    nrows=scDataVec.size();

	spec2DataVec.clear();
    arf2DataVec.clear();

    for(i=0;i<nrows;i++)
    {
        if(scDataVec[i].time >= tstart && scDataVec[i].time < tstop)
        {
			
			current_tbin=(long)((scDataVec[i].time-tstart)/(double)tbinsize);

            F.decode_frame(scDataVec[i].dataArray,0,0,1);
            fexp=1.0;

			if(current_tbin==prev_tbin)
			{
	            if (chantype==1)
                {
                    if(areascal){
                        singspecData.addSpecArea(F.spec.spectrum,phasyserrfrac,fexp,areaArr[i]);
                    }
                    else
                        singspecData.addSpec(F.spec.spectrum,phasyserrfrac,fexp);
                }
    	        else 
                {
	                gain=gainarr[i];
    	            offset=offsetarr[i];			
        	        long PIspec[n_PIbin];
	                float PIspec_syserr[n_PIbin];
            	    rebinPHA2PI(F.spec.spectrum,PIspec,PIspec_syserr,gain,offset);
                    if(areascal){
                    	singspecData.addSpecArea(PIspec,PIspec_syserr,fexp,areaArr[i]);
                    }
                    else
                    {
                        singspecData.addSpec(PIspec,PIspec_syserr,fexp);
	                    if(genarf)
    	                    singarfData.addArea(areaArr[i],fexp);
                    }
            	}
			}
			else 
			{
				if(prev_tbin!=-1)
                {
					spec2DataVec.push_back(singspecData);
                    if(genarf) arf2DataVec.push_back(singarfData);
                }
                
				singspecData.initialize(chantype,n_PIbin);
				singspecData.setTimes(tstart+current_tbin*tbinsize,tstart+(current_tbin+1)*tbinsize);

                // Temporarily setting filter status as 0 always
                singspecData.setFilter(0);
			
                if(genarf)
                {
                    singarfData.initialize();
                    singarfData.setTimes(tstart+current_tbin*tbinsize,tstart+(current_tbin+1)*tbinsize);
                }
                
                if (chantype==1) 
                {
                    if(areascal){
                        singspecData.addSpecArea(F.spec.spectrum,phasyserrfrac,fexp,areaArr[i]);
                    }
                    else
                        singspecData.addSpec(F.spec.spectrum,phasyserrfrac,fexp);                    
                }
                else 
                {
                    long PIspec[n_PIbin];
					float PIspec_syserr[n_PIbin];
                    rebinPHA2PI(F.spec.spectrum,PIspec,PIspec_syserr,gain,offset);
                    if(areascal){
                        singspecData.addSpecArea(PIspec,PIspec_syserr,fexp,areaArr[i]);
                    }
                    else
                    {
                        singspecData.addSpec(PIspec,PIspec_syserr,fexp);                

                        if(genarf)
                            singarfData.addArea(areaArr[i],fexp);
                    }
                        
                }
	
				prev_tbin=current_tbin;
			}

        }
        else if(scDataVec[i].time >=tstop)
        {
            break;
        }
    }

	if(current_tbin==prev_tbin) spec2DataVec.push_back(singspecData); // to add the last spectrum 
    if(current_tbin==prev_tbin && genarf) arf2DataVec.push_back(singarfData); // to add the last spectrum 


    return EXIT_SUCCESS;
}

int xsmgenspec::rebinPHA2PI(long *PHASpec, long *PISpec,float *PIspec_syserr,double gain,double offset)
{
	int i,j;
	double ene=0;
	float pha=0;

	for (i=0;i<n_PIbin;i++){
        PISpec[i]=0;
        PIspec_syserr[i]=0;
    }

	
	for (i=0;i<SPEC_SIZE;i++)
	{
		if(PHASpec[i]>0)
		{
			for (j=0;j<PHASpec[i];j++){
				pha=2.0*floor(i/2)+2.0*urand();  // This ignores the LSB of 10-bit PHA value (previously pha=i+urand();) 
				ene=pha*gain+offset;
				if(ene >= PIEmin && ene< PIEmax) 
                {
                    PISpec[(int)((ene-PIEmin)/PIbinsize)]+=1;	
                    PIspec_syserr[(int)((ene-PIEmin)/PIbinsize)]+=phasyserrfrac[i/2];
                }
			}
		}
	}
	

	return EXIT_SUCCESS;
}

int xsmgenspec::computeGain()
{
    int status=0;
	long Ntec, Nrad;
	float tecBinsize;
	float radBinsize;
	long i,j;

    long naxis2;

    if(strcasecmp(gainfile, "CALDB")==0)
    {
        string caldbgainfile;
        status=queryCaldb("GAIN",tstart,caldbgainfile);
        if(status) return(status);
        sprintf(gainfile,"%s",caldbgainfile.c_str());
        cout<<"MESSAGE: Gain CALDB file used is: "<<gainfile<<"\n";
    }

    fitsfile *fin;

    fits_open_file(&fin,gainfile,READONLY,&status);
    if(status) { 
        cout<<"***Error in opening gainfile "<<gainfile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);  
        return (EXIT_FAILURE); 
    }

    // If sourcetype==sun use GAIN extension of the gain file
    if(sourcetype==2)
    {

        fits_movnam_hdu(fin, BINARY_TBL, "GAIN", 0, &status);
        if(status) {
            cout<<"***Error in finding GAIN extn in file "<<gainfile<<"\n";
            cout<<"***Incorrect format file\n";
            fits_report_error(stderr,status);
            return (EXIT_FAILURE);
        }

    }
    else if(sourcetype==1) // if sourcetype==cal use CALGAIN extension of the gain file
    {
        fits_movnam_hdu(fin, BINARY_TBL, "CALGAIN", 0, &status);
        if(status) {
            cout<<"***Error in finding CALGAIN extn in file "<<gainfile<<"\n";
            cout<<"***Incorrect format file\n";
            fits_report_error(stderr,status);
            return (EXIT_FAILURE);
        }

        cout <<"MESSAGE: Using gain for CAL source from gainfile\n";
    }

    fits_read_key(fin,TLONG,"NAXIS2",&naxis2,NULL, &status);
    fits_read_key(fin,TLONG,"NTEC",&Ntec,NULL, &status);
    fits_read_key(fin,TLONG,"NRAD",&Nrad,NULL, &status);
    fits_read_key(fin,TFLOAT,"TECBINSIZE",&tecBinsize,NULL, &status);
    fits_read_key(fin,TFLOAT,"RADBINSIZE",&radBinsize,NULL, &status);

    if(status) {
        cout<<"***Error in reading header keywords from file "<<gainfile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    if((Ntec*Nrad)!=naxis2){
        cout<<"***ERROR: Unexpected number of entries in the gainfile\n";
        return EXIT_FAILURE;
    }

    double **gainmatrix;
    double **offsetmatrix;

	gainmatrix=(double**)malloc(sizeof(double*)*Ntec);
	offsetmatrix=(double**)malloc(sizeof(double*)*Ntec);

	for(i=0;i<Ntec;i++){
		gainmatrix[i]=(double*)malloc(sizeof(double)*Nrad);
		offsetmatrix[i]=(double*)malloc(sizeof(double)*Nrad);
		for(j=0;j<Nrad;j++){
			gainmatrix[i][j]=0.0;
			offsetmatrix[i][j]=0.0;
		}
	}

    for (i=0;i<Ntec;i++)
    {
        for(j=0;j<Nrad;j++)
        {
            long row=i*Nrad+j;

            fits_read_col(fin,TDOUBLE,3,row+1,1,1,NULL,&gainmatrix[i][j],NULL,&status);
            fits_read_col(fin,TDOUBLE,4,row+1,1,1,NULL,&offsetmatrix[i][j],NULL,&status);

            if(status)
            {
                cout<<"Error in reading Gain file "<<gainfile<<"\n";
                return EXIT_FAILURE;
            }
        }
    }


    fits_close_file(fin,&status);
    if(status) { 
        cout<<"***Error in closing file "<<gainfile<<"\n"; 
        fits_report_error(stderr,status);  
        return (EXIT_FAILURE); 
    }

	long nrowsHk;
	nrowsHk=hkDataVec.size();

    if(gaincorfactor !=1){
        cout<<"MESSAGE: USING ADDITIONAL GAIN CORRECTION FACTOR "<<gaincorfactor<<"\n";
    }

    if(offsetcorfactor !=0){
        cout<<"MESSAGE: USING ADDITIONAL OFFSET CORRECTION FACTOR "<<offsetcorfactor<<"\n";
    }

	for(i=0;i<nrowsHk;i++)
	{
		int tecbin;
		int radbin=0;

		tecbin=(int)(hkDataVec[i].tecCurent/tecBinsize);

		if(tecbin>=Ntec)
		{
			cout<<"ERROR: Gain values not available for TEC current in HK\n";
			return EXIT_FAILURE;
		}

        // If sourcetype==sun use Detector position for gain
        if(sourcetype==2) 
        {

            radbin=(int)(sunAngDataVec[i].detPosR/radBinsize);

            if(radbin>=Nrad)
            {
                cout<<"ERROR: Gain values not available for DetPos in sunAng file\n";
                return EXIT_FAILURE;
            }            
        }
        else{
            radbin=0;   // Use gain at detector center
        }

		gainarr[i]=gainmatrix[tecbin][radbin]*gaincorfactor;
        offsetarr[i]=offsetmatrix[tecbin][radbin]+offsetcorfactor-0.01;     // ADDITIONAL OFFSET -10eV added for now
		
		//printf("%ld\t%d\t%d\t%lf\t%lf\n",i,tecbin,radbin,gainarr[i],offsetarr[i]);
	}

    for(i=0;i<Ntec;i++){
        free(gainmatrix[i]);
        free(offsetmatrix[i]);
    }

	free(gainmatrix);
	free(offsetmatrix);

	return EXIT_SUCCESS;
}

/*
int xsmgenspec::computeRelArea()
{
    int status=0;
    long i,Ntot,j;

    float binsize_ph;
    float binsize_th;
    int nbins_ph;
    int nbins_th;
    
    float garea;    
    
    if(strcasecmp(areafile, "CALDB")==0)
    {
        string caldbareafile;
        status=queryCaldb("EFFAREA",tstart,caldbareafile);
        if(status) return(status);
        sprintf(areafile,"%s",caldbareafile.c_str());
        cout<<"MESSAGE: EffArea CALDB file used is: "<<areafile<<"\n";
    }

    fitsfile *fin;

    fits_open_file(&fin,areafile,READONLY,&status);
    if(status) {
        cout<<"***Error in opening areafile "<<areafile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    fits_movnam_hdu(fin, BINARY_TBL, "AREA", 0, &status);
    if(status) {
        cout<<"***Error in finding EFFAREA extn in file "<<areafile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    fits_read_key(fin,TLONG,"NAXIS2",&Ntot,NULL, &status);
    fits_read_key(fin,TFLOAT,"THBINSIZ",&binsize_th,NULL, &status);
    fits_read_key(fin,TFLOAT,"PHBINSIZ",&binsize_ph,NULL, &status);
    fits_read_key(fin,TFLOAT, "GEOMAREA",&garea, NULL, &status);
    fits_read_key(fin,TINT, "NTHBIN",&nbins_th, NULL, &status);
    fits_read_key(fin,TINT, "NPHBIN",&nbins_ph, NULL, &status);

    float areamatrix[Ntot];

    fits_read_col(fin,TFLOAT,3,1,1,Ntot,NULL,areamatrix,NULL,&status);

    if(status) {
        cout<<"***Error in reading from file "<<areafile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }
   

    fits_movnam_hdu(fin, BINARY_TBL, "EFFICIENCY", 0, &status);
    if(status) {
        cout<<"***Error in finding EFFICIENCY extn in file "<<areafile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    long Nrows2;
    fits_read_key(fin,TLONG,"NAXIS2",&Nrows2,NULL, &status);

    float effmatrix[nbins_th][nchan_area];
    int eff_col;

    if(chantype==1) // PHA read EFF_PHA column
        fits_get_colnum(fin,CASEINSEN,"EFF_PHA",&eff_col,&status);
    else if (chantype==2) // PI read EFF_PI column           
        fits_get_colnum(fin,CASEINSEN,"EFF_PI",&eff_col,&status);
     
    if(status) {
        cout<<"***Error in getting column number of EFF_PHA/EFF_PI in file "<<areafile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }
                
    for (i=0;i<Nrows2;i++)
    {
        fits_read_col(fin,TFLOAT,eff_col,i+1,1,nchan_area,NULL,effmatrix[i],NULL,&status);
        if(status) {
            cout<<"***Error in reading from file "<<areafile<<"\n";
            fits_report_error(stderr,status);
            return (EXIT_FAILURE);
        }
    }


    fits_close_file(fin,&status);
    if(status) {
        cout<<"***Error in closing file "<<areafile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }


     // Compute area as function of time
     long nrowsSa;
     nrowsSa=sunAngDataVec.size();

     for (i=0;i<nrowsSa;i++)
     {
         int phbin,thbin;
         phbin=(int)((sunAngDataVec[i].phi+180)/binsize_ph);
         thbin=(int)(sunAngDataVec[i].theta/binsize_th);

         if(phbin>=nbins_ph||thbin>=nbins_th)
         {
             cout<<"ERROR: Area values not available for Angles in SunAng file\n";
             return EXIT_FAILURE;
         }

         long irow=phbin*nbins_th+thbin;

        //Compute relative area with respect to on-axis 
         for (j=0;j<nchan_area;j++)
         {
             if(effmatrix[0][j] >0.01 && effmatrix[thbin][j] >0.01)
                 //areaArr[i][j]=((double)areamatrix[irow]/garea)*effmatrix[thbin][j]/effmatrix[0][j];
                 areaArr[i][j]=effmatrix[thbin][j]/effmatrix[0][j];
             else areaArr[i][j]=1.0;
         }
    
     }

     return EXIT_SUCCESS;
}
*/
// Pseudo random number generator AS 183

double xsmgenspec::urand()
{
      double dummy;
      int genurandt_a,genurandt_b,genurandt_c;

      genurandt_a=30269;
      genurandt_b=30307;
      genurandt_c=30319;

      Rand_ix = (171*Rand_ix)%genurandt_a;
      Rand_iy = (172*Rand_iy)%genurandt_b;
      Rand_iz = (170*Rand_iz)%genurandt_c;

      dummy = (1.0*Rand_ix/genurandt_a+1.0*Rand_iy/genurandt_b+1.0*Rand_iz/genurandt_c);

      double rand= fmod(dummy,1.0);

      return rand;
}


/* Function written by Mithun and Rakesh Khanna (13/12/15) 
   based on the earlier version of the function for CZTI pipeline  
   modified sutiably for xsmgtigen
 * */
int find_intersecting_range(vector<gtiData> gti1, vector<gtiData> gti2, vector<gtiData> &mergedVec)
{
    long nsize1=0,nsize2=0;

    nsize1=gti1.size();
    nsize2=gti2.size();

    bool gtiValidityFlag=true;

    gtiData intersec;
    int i,j,k,l,i1,i2;
    double index[4],subgti[4];

    if(nsize1>0){
        for(i1=0; i1<nsize1; i1++){
            if(gti1[i1].tstart >gti1[i1].tstop){
                gtiValidityFlag=false;
                break;
            }
        }
        if(gtiValidityFlag==false){
            cout<<"***ERROR: GTI1 tstart should be less than tstop";
            return(EXIT_FAILURE);
        }
    }
    if(nsize2>0){
        for(i2=0; i2<nsize2; i2++){
            if(gti2[i2].tstart >gti2[i2].tstop){
                gtiValidityFlag=false;
                break;
            }
        }
        if(gtiValidityFlag==false){
            cout<<"***ERROR: GTI2 tstart should be less than tstop";
            return(EXIT_FAILURE);
        }
    }


    if(nsize1==0 && nsize2==0){
        cout<<"***ERROR: Both GTIs are empty\n";
        cout<<"***Exiting without processing\n";
        return(EXIT_FAILURE);
    }
    else if(nsize1==0)
    {
        cout<<"***ERROR: GTI1 is empty - NO DATA PRESENT\n";
        return(EXIT_FAILURE);
    } else if(nsize2==0)
    {
        cout<<"***ERROR: No GTI for the requried Parameter range\n";
        return(EXIT_FAILURE);
    }

    for(i=0;i<gti1.size();i++)
       {
            for(j=0;j<gti2.size();j++)
            {
                subgti[0]=gti1[i].tstart;
                subgti[1]=gti1[i].tstop;
                subgti[2]=gti2[j].tstart;
                subgti[3]=gti2[j].tstop;

                index[0]=0;
                index[1]=1;
                index[2]=0;
                index[3]=1;

                for(k=0;k<4;k++)
                {
                    double gtiTemp=0;
                    int indTemp=0;

                    for(l=k+1;l<4;l++)
                    {
                        if(subgti[k]>subgti[l])
                        {
                        gtiTemp=subgti[k];
                        indTemp=index[k];
                        subgti[k]=subgti[l];
                        index[k]=index[l];
                        subgti[l]=gtiTemp;
                        index[l]=indTemp;
                        }
                    }
                }

                    if(index[1]==0 && index[2]==1)
                    {
                        intersec.tstart=subgti[1];
                        intersec.tstop=subgti[2];
                        mergedVec.push_back(intersec);
                    }

            }

       }

    //FIX to Remove the reduntant entries with tstart==tstop
       for(i=0;i<mergedVec.size();i++)
       {
           if(mergedVec[i].tstart==mergedVec[i].tstop)
           {
               mergedVec.erase(mergedVec.begin()+i);
               i-=1;
           }
       }

    if(mergedVec.size()==0)
    {
        cout<<"***ERROR: No Common GTI available\n";
        return EXIT_FAILURE;
    }

    return(EXIT_SUCCESS);
}

