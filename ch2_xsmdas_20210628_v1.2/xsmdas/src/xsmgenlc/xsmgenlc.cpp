#include <xsmgenlc.h>


xsmgenlc::xsmgenlc(){
	scDataVec.clear();
    lc2Data.resize(3);
	dataTstart=0.;
	dataTstop=0.;


    //Initialize for random number
    Rand_ix=5;
    Rand_iy=11;
    Rand_iz=17;

    //Initialize the variables
    enelow=1.0;
    enehigh=15.0;

    chstart=0;
    chstop=960;
}

xsmgenlc::~xsmgenlc(){}

int xsmgenlc::read(int argc,char **argv)
{
    int status=0;

    runHistory.clear();
    runHistory.push_back("************** XSMGENLC **************");

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

    if(PIL_OK!=(status=PILGetFname("lcfile",lcfile))){
        cout<<"***Error reading output Lightcurve file***\n";
        return status;
    }

    runHistory.push_back("l1file = "+(string)basename(l1file));
    runHistory.push_back("lcfile = "+(string)basename(lcfile));

	vector <string> inStrings;

    if (PIL_OK != (status = PILGetString("lctype", lctype_str))) {
        cout << "***Error reading output LC type***\n";
        return status;
    }

    if (strcasecmp(lctype_str, "standard") == 0) {
        lctype = 1;
        runHistory.push_back("lctype = standard");
    } else if (strcasecmp(lctype_str, "high-res") == 0) {
        lctype = 2;
        runHistory.push_back("lctype = high-res");        
	} else{
        cout<<"***ERROR: Invalid input for lcctype. Valid inputs are standard or high-res\n";
        return EXIT_FAILURE;        
    }

    char tempStr[100];

	if(lctype==1){
	    // Whether to apply area scaling to lightcurve
    	if (PIL_OK != (status = PILGetBool("areascal", &areascal))) {
        	cout << "***Error Reading areascal***";
        	return status;
    	}

    	if(areascal) runHistory.push_back("areascal = yes");
    	else runHistory.push_back("areascal = no");
	}
    else
    {
        cout<<"No areascaling possible for high time resolution light curves\n";
        areascal=0;
    }


    if(lctype==1){

	    // Whether to use energy selection for light curve instead of PHA
    	if (PIL_OK != (status = PILGetBool("enesel", &enesel))) {
        	cout << "***Error Reading enesel***";
        	return status;
    	}

	    if(enesel) runHistory.push_back("enesel = yes");
    	else runHistory.push_back("enesel = no");

		if(enesel)
        {

            if (PIL_OK != (status = PILGetReal("enelow", &enelow))) {
                cout << "***Error reading Lower Energy***\n";
                return status;
            }

            if (PIL_OK != (status = PILGetReal("enehigh", &enehigh))) {
                cout << "***Error reading Higher Energy***\n";
                return status;
            }

            if(enelow>enehigh||enelow<0.5||enehigh<0.5||enelow>=17.396)
            {
                cout<<"ERROR: Bad values for enelow and/or enehigh\n";
                return EXIT_FAILURE;
            }

            sprintf(tempStr,"enelow = %d",enelow);
            runHistory.push_back((string)tempStr);

            sprintf(tempStr,"enehigh = %d",enehigh);
            runHistory.push_back((string)tempStr);

			// Get ebounds and gain file names
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
 
            runHistory.push_back("eboundsfile = "+(string)(eboundsfile));
            runHistory.push_back("gainfile = "+(string)(gainfile));

            
        }
        else
        {
        
            if (PIL_OK != (status = PILGetInt("chstart", &chstart))) {
                cout << "***Error reading Start channel***\n";
                return status;
            }

            if (PIL_OK != (status = PILGetInt("chstop", &chstop))) {
                cout << "***Error reading Stop channel***\n";
                return status;
            }

            if(chstart>chstop||chstart<0||chstop<0||chstart>=SPEC_SIZE||chstop>=SPEC_SIZE)
            {
                cout<<"ERROR: Bad values for chstart and/or chstop\n";
                return EXIT_FAILURE;
            }

            sprintf(tempStr,"chstart = %d",chstart);
            runHistory.push_back((string)tempStr);

            sprintf(tempStr,"chstop = %d",chstop);
            runHistory.push_back((string)tempStr);

        }

        if (PIL_OK != (status = PILGetInt("tbinsizesec", &tbinsizesec))) {
            cout << "***Error reading time bin size***\n";
            return status;
        }

        if(tbinsizesec<1){
            cout<< "***Error: Invalid time bin size. tbinsizesec should be >=1\n";
            return EXIT_FAILURE;
        }

        sprintf(tempStr,"tbinsizesec = %d",tbinsizesec);
        runHistory.push_back((string)tempStr);

    }
    else if(lctype==2)
    {
	
		enesel=0;

        if (PIL_OK != (status = PILGetInt("tbinsizems", &tbinsizems))) {
        cout << "***Error reading time bin size***\n";
        return status;
        }

        if(tbinsizems<100||tbinsizems%100!=0){
            cout<<"ERROR:Invalid time binsize. It has to be multiple of 100ms and >=100ms\n";
            return EXIT_FAILURE;
        }

        sprintf(tempStr,"tbinsizems = %d",tbinsizems);
        runHistory.push_back((string)tempStr);

    }


    if(PIL_OK!=(status=PILGetFname("hkfile",hkfile))){
        cout<<"***Error reading HK file***\n";
        return status;
    }

    if(!FileExists(hkfile)){
        cout<<"***ERROR: This file does not exist: "<<hkfile<<"***\n";
        return EXIT_FAILURE;
    }

    runHistory.push_back("hkfile = "+(string)basename(hkfile));        


    if(areascal||enesel){

        if(PIL_OK!=(status=PILGetFname("safile",safile))){
            cout<<"***Error reading Sun Angle file***\n";
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

    if(areascal){

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

        
    if (PIL_OK != (status = PILGetBool("clobber", &clobber))) {
        cout << "***Error Reading clobber***\n";
        return status;
    }

    if(clobber) runHistory.push_back("clobber = yes");
    else runHistory.push_back("clobber = no");

    runHistory.push_back("**************************************");

    PILClose(status);

    cout<<"------------------------------------------------------------------------\n";

    return (EXIT_SUCCESS);

}

int  xsmgenlc::display()
{
    return EXIT_SUCCESS;
}

int xsmgenlc::xsmgenlcProcess()
{
    int status=0;
	long nrows,i,j;
    long nrowsSa,nrowsSc,nrowsHk;
    

	scDataVec.clear();
    status=scfh.read_l1science_file((string)l1file,scDataVec);

    if(status)
    {
        cout<<"***Error in reading level-1 file\n";
        return EXIT_FAILURE;
    }
  
	nrows=scDataVec.size();

	gtiDataVec.clear();
	status=gtifh.read_gtifile((string)gtifile,gtiDataVec);

    if(status)
    {
        cout<<"***Error in reading GTI file\n";
        return EXIT_FAILURE;
    }

    status=gtiFilterScienceData(scDataVec,gtiDataVec);

    if(status)
    {
        cout<<"***Error in filtering L1 file with GTI\n";
        return EXIT_FAILURE;
    }

    dataTstart=scDataVec[0].time-0.001;
    dataTstop=scDataVec[nrows-1].time+0.999;

    // Read HK file and GTI filter
	hkDataVec.clear();
    status=hkfh.read_hkfile((string)hkfile,hkDataVec);

    if(status)
    {
        cout<<"Error in reading HK file\n";
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

    if(lctype==2)
    {
        int chrangestatus=0;

        lc2Data[0].chstart=hkDataVec[0].ch1_start; lc2Data[0].chstop=hkDataVec[0].ch1_stop;
        lc2Data[1].chstart=hkDataVec[0].ch2_start; lc2Data[1].chstop=hkDataVec[0].ch2_stop;
        lc2Data[2].chstart=hkDataVec[0].ch3_start; lc2Data[2].chstop=hkDataVec[0].ch3_stop;
            
        for(i=0;i<nrowsHk;i++){

            if(hkDataVec[i].ch1_start!=lc2Data[0].chstart || hkDataVec[i].ch1_stop!=lc2Data[0].chstop||
               hkDataVec[i].ch2_start!=lc2Data[1].chstart || hkDataVec[i].ch2_stop!=lc2Data[1].chstop||
               hkDataVec[i].ch3_start!=lc2Data[2].chstart || hkDataVec[i].ch3_stop!=lc2Data[2].chstop)
            {
				chrangestatus=1;

                //cout<<"***ERROR: Channel ranges for coarse channel LC not constant within the data\n";
                //cout<<"***Use xsmgtigen to select times with only constant coarse channel ranges\n";
                //cout<<"***EXITING WITHOUT GENERATING OUTPUT\n";
                //return EXIT_FAILURE; 
            }

        }

		if(chrangestatus){
                cout<<"***WARNING: Channel ranges for coarse channel LC not constant within the data\n";
				cout<<"***Be careful in interpreting the high-res light curve\n";
                cout<<"***Better to use xsmgtigen to select times with only constant coarse channel ranges\n";			
		}
    }


    // In case of areascal==yes or enesel==yes Read SA file and Filter sunAngDataVec with GTI

    if(areascal||enesel)
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

    // In case of ene range selection, read ebounds file
    // Compute gain as function of time

	if(enesel)
    {
        if(strcasecmp(eboundsfile, "CALDB")==0)
        {
            string caldbeboundsfile;
            status=queryCaldb("EBOUNDS",dataTstart,caldbeboundsfile);
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

		// Get the start and stop PI channels from enelow/enehigh

        if(enelow >= PIEmin && enelow< PIEmax)
            PIstart=(int)((enelow-PIEmin)/PIbinsize);
        else
        {
            cout<<"ERROR: enelow value not in range\n";
            return EXIT_FAILURE;
        }

        if(enehigh >= PIEmin && enehigh< PIEmax)
            PIstop=(int)((enehigh-PIEmin)/PIbinsize);
        else
        {
            cout<<"ERROR: enehigh value not in range\n";
            return EXIT_FAILURE;
        }

	}

   // if areascal==yes, Compute area as function of PI/PHA energies based in enesel

	if(areascal)
    {
        int eneType,relFlag;
        relFlag=1;

        if(enesel)
        {
            eneType=1; //PI
            
        	areaArr=(double **)malloc(sizeof(double*)*nrowsSc);
        	for(i=0;i<nrowsSc;i++) 
        	{
           		areaArr[i]=(double*)malloc(sizeof(double)*n_PIbin);
            	for(j=0;j<n_PIbin;j++) areaArr[i][j]=1.0;
        	}

            status=get_effectivearea(sunAngDataVec,hkDataVec,areaArr,n_PIbin,eneType,relFlag,abscoeffile,effareaparfile,dataTstart);
            
        }
        else
        {
            eneType=2; // PHA

            
        	areaArr=(double **)malloc(sizeof(double*)*nrowsSc);
        	for(i=0;i<nrowsSc;i++)
        	{
            	areaArr[i]=(double*)malloc(sizeof(double)*SPEC_SIZE);
            	for(j=0;j<SPEC_SIZE;j++) areaArr[i][j]=1.0;
        	}

            status=get_effectivearea(sunAngDataVec,hkDataVec,areaArr,SPEC_SIZE,eneType,relFlag,abscoeffile,effareaparfile,dataTstart);

        }

        if(status)
        {
            cout<<"ERROR: Error in computing area as function of time\n";
            return EXIT_FAILURE;
        }
       
    }


	if(lctype==1)
	{
		ntbins=(long)((dataTstop-dataTstart)/tbinsizesec)+1;		
		lcDataRecord.lctime=(double *)malloc(sizeof(double)*ntbins);
        lcDataRecord.lcrate=(float *)malloc(sizeof(float)*ntbins);
        lcDataRecord.lcerror=(float *)malloc(sizeof(float)*ntbins);
        lcDataRecord.fracexp=(float *)malloc(sizeof(float)*ntbins);
        lcDataRecord.area=(float *)malloc(sizeof(float)*ntbins);

		for(i=0;i<ntbins;i++)
		{
			lcDataRecord.lctime[i]=dataTstart+(i)*(double)tbinsizesec;
			lcDataRecord.lcrate[i]=0.;
			lcDataRecord.lcerror[i]=0.;
			lcDataRecord.fracexp[i]=0.;
            lcDataRecord.area[i]=0.;
		}

		lcDataRecord.chstart=chstart;
		lcDataRecord.chstop=chstop;
		lcDataRecord.tstart=dataTstart;
		lcDataRecord.tstop=dataTstop;
		lcDataRecord.tbinsize=(double)tbinsizesec;
		lcDataRecord.ntbins=ntbins;

		status=generatelc1();

	    if(status)
    	{
        	cout<<"***ERROR: Error in computing light curve \n";
        	return (status);
    	}

	}
	else if(lctype==2)
	{
        ntbins=(long)((dataTstop-dataTstart)/(tbinsizems*0.001))+1;
        
        for(i=0;i<3;i++){
            lc2Data[i].lctime=(double *)malloc(sizeof(double)*ntbins);
            lc2Data[i].lcrate=(float *)malloc(sizeof(float)*ntbins);
            lc2Data[i].lcerror=(float *)malloc(sizeof(float)*ntbins);
            lc2Data[i].fracexp=(float *)malloc(sizeof(float)*ntbins);
            lc2Data[i].area=(float *)malloc(sizeof(float)*ntbins);

            for(j=0;j<ntbins;j++){
                lc2Data[i].lctime[j]=dataTstart+(j)*(double)tbinsizems*0.001;
                lc2Data[i].lcrate[j]=0.;
                lc2Data[i].lcerror[j]=0.;
                lc2Data[i].fracexp[j]=0.;
                lc2Data[i].area[j]=0.;
            }
    
            lc2Data[i].tstart=dataTstart;
            lc2Data[i].tstop=dataTstop;
            lc2Data[i].tbinsize=(double)tbinsizems*0.001;
            lc2Data[i].ntbins=ntbins;

        }

        status=generatelc2();

        if(status)
        {
            cout<<"***ERROR: Error in computing light curve \n";
            return (status);
        }
        
	}

    // Copy header keywords from Science file to LC
    lcfh.date_obs=scfh.date_obs;
    lcfh.date_end=scfh.date_end;
    
    // Write LC file

    status=doClobber(lcfile,clobber);
    if(status) return (status);
    
	lcfh.lctype=lctype;

	status=lcfh.create_lcfile((string)lcfile);

    if(status)
    {
        cout<<"Error in creating light curve file "<<lcfile<<"\n";
        return EXIT_FAILURE;
	}

    if(lctype==1)
        status=lcfh.write_type1_lcfile((string)lcfile,lcDataRecord,runHistory);
    else if(lctype==2)
        status=lcfh.write_type2_lcfile((string)lcfile,lc2Data,runHistory);
    

    if(status)
    {
        cout<<"Error in writing to lc file "<<lcfile<<"\n";
        return EXIT_FAILURE;
    }

    if(areascal) {
        for (i=0;i<nrowsSc;i++) free(areaArr[i]);
        free(areaArr);
    }

    if(lctype==1){
        free(lcDataRecord.lctime) ;
        free(lcDataRecord.lcrate) ;
        free(lcDataRecord.lcerror);
        free(lcDataRecord.fracexp);
        free(lcDataRecord.area)   ;
    }
    else if(lctype==2){

        for(i=0;i<3;i++){
            free(lc2Data[i].lctime)   ;
            free(lc2Data[i].lcrate)  ;
            free(lc2Data[i].lcerror)  ;
            free(lc2Data[i].fracexp)  ;
            free(lc2Data[i].area)     ;
        }
    }


    cout<<"\nMESSAGE: XSMGENLC completed successully\n";
    cout<<"MESSAGE: Output file = "<<lcfile<<"\n\n";
	
    return EXIT_SUCCESS;
}

int xsmgenlc::generatelc1()
{
    
	frame F;
    long i,nrows,j;
    long current_tbin=0;
    
	nrows=scDataVec.size();

	double tstart=lcDataRecord.tstart;

 	for(i=0;i<nrows;i++)
    {
        current_tbin=(long)((scDataVec[i].time-tstart)/(double)tbinsizesec);

		if(current_tbin>=ntbins)
		{
			cout<<"ERROR: Invalid time bin. Error in lc generation\n";
			return EXIT_FAILURE;
		}

        F.decode_frame(scDataVec[i].dataArray,0,0,1);

        if(enesel)
		{
            double gain=gainarr[i];
            double offset=offsetarr[i];
            long PIspec[n_PIbin];
            rebinPHA2PI(F.spec.spectrum,PIspec,gain,offset);

	        if(areascal){
    	        for(j=PIstart;j<=PIstop;j++)
        	        lcDataRecord.lcrate[current_tbin]+=(PIspec[j]/areaArr[i][j]);
        	}
        	else{
            	for(j=PIstart;j<=PIstop;j++)
                	lcDataRecord.lcrate[current_tbin]+=(PIspec[j]);
        	}

            
        }
		else
        {

            if(areascal){
		        for(j=chstart;j<=chstop;j++)
			        lcDataRecord.lcrate[current_tbin]+=(F.spec.spectrum[j]/areaArr[i][j]);
            }
            else{
                for(j=chstart;j<=chstop;j++)
                    lcDataRecord.lcrate[current_tbin]+=(F.spec.spectrum[j]);
            }

        }

		lcDataRecord.fracexp[current_tbin]+=1.0;
	}

	for(i=0;i<ntbins;i++) 
	{
		if(lcDataRecord.fracexp[i]>0)
		{
            lcDataRecord.fracexp[i]/=(float)tbinsizesec;
            
			lcDataRecord.lcerror[i]=sqrt(lcDataRecord.lcrate[i])/((float)tbinsizesec*lcDataRecord.fracexp[i]);
			lcDataRecord.lcrate[i]/=((float)tbinsizesec*lcDataRecord.fracexp[i]);
		}
	}

	return EXIT_SUCCESS;
}

int xsmgenlc::generatelc2()
{

    frame F;
    long i,nrows,j;
    int k;
    long current_tbin=0;

    nrows=scDataVec.size();
    double tstart=lc2Data[0].tstart;

    for(i=0;i<nrows;i++)
    {
        F.decode_frame(scDataVec[i].dataArray,0,1,0);

        for(j=0;j<COARSECH_LCBIN;j++){

            current_tbin=(long)((scDataVec[i].time+j*0.1-tstart)/((double)tbinsizems*0.001));

            if(current_tbin>=ntbins)
            {
                cout<<"ERROR: Invalid time bin. Error in high-res lc generation\n";
                return EXIT_FAILURE;
            }
           
            lc2Data[0].lcrate[current_tbin]+=F.lc.ch1lc[j];
            lc2Data[1].lcrate[current_tbin]+=F.lc.ch2lc[j];
            lc2Data[2].lcrate[current_tbin]+=F.lc.ch3lc[j];            

            for (k=0;k<3;k++) lc2Data[k].fracexp[current_tbin]+=1.0;
        }

    }

    for(i=0;i<ntbins;i++)
    {
        if(lc2Data[0].fracexp[i]>0)
        {

            for(k=0;k<3;k++)
            {
                lc2Data[k].fracexp[i]/=(((float)tbinsizems*0.001)/0.1);
                lc2Data[k].lcerror[i]=sqrt(lc2Data[k].lcrate[i])/((float)tbinsizems*0.001*lc2Data[k].fracexp[i]);
                lc2Data[k].lcrate[i]/=((float)tbinsizems*0.001*lc2Data[k].fracexp[i]);
            }
            
        }
    }
    

    return EXIT_SUCCESS;
}

int xsmgenlc::rebinPHA2PI(long *PHASpec, long *PISpec,double gain,double offset)
{
    int i,j;
    double ene=0;
    float pha=0;

    for (i=0;i<n_PIbin;i++){
        PISpec[i]=0;
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
                }
            }
        }
    }


    return EXIT_SUCCESS;
}


int xsmgenlc::computeGain()
{

	int sourcetype=2;
    double gaincorfactor=1.0;

    int status=0;
    long Ntec, Nrad;
    float tecBinsize;
    float radBinsize;
    long i,j;

    long naxis2;

    if(strcasecmp(gainfile, "CALDB")==0)
    {
        string caldbgainfile;
        status=queryCaldb("GAIN",dataTstart,caldbgainfile);
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
        offsetarr[i]=offsetmatrix[tecbin][radbin];

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

double xsmgenlc::urand()
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



/*

int xsmgenlc::computeRelArea()
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
        status=queryCaldb("EFFAREA",dataTstart,caldbareafile);
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
        cout<<"***Error in finding AREA extn in file "<<areafile<<"\n";
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

    float effmatrix[nbins_th][SPEC_SIZE];

    for (i=0;i<Nrows2;i++)
    {
        fits_read_col(fin,TFLOAT,3,i+1,1,SPEC_SIZE,NULL,effmatrix[i],NULL,&status);
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
         for (j=0;j<SPEC_SIZE;j++)
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
/*
 * PREVIOUS FUNCTION FOR THETA DEP AREA AND XY ANG AREA CASES
int xsmgenlc::computeRelArea()
{

    int status=0;
    long Nang;
    float angBinsize;
    long i;

    if(strcasecmp(areafile, "CALDB")==0)
    {
        string caldbareafile;
        status=queryCaldb("EFFAREA",dataTstart,caldbareafile);
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

    fits_movnam_hdu(fin, BINARY_TBL, "EFFAREA", 0, &status);
    if(status) {
        cout<<"***Error in finding EFFAREA extn in file "<<areafile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

*/
/*
    long Ntotbin;

    fits_read_key(fin,TLONG,"NAXIS2",&Ntotbin,NULL, &status);

    float areamatrix[Ntotbin];

    fits_read_col(fin,TFLOAT,3,1,1,Ntotbin,NULL,areamatrix,NULL,&status);

    if(status) {
        cout<<"***Error in reading from file "<<areafile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    fits_close_file(fin,&status);
    if(status) {
        cout<<"***Error in closing file "<<areafile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    float xangBinsize=0.5;
    float yangBinsize=0.5;

     // Compute area as function of time
     long nrowsSa;
     nrowsSa=sunAngDataVec.size();

     for (i=0;i<nrowsSa;i++)
     {
         float xval=sunAngDataVec[i].theta*cos(sunAngDataVec[i].phi*3.1415/180.0);
         float yval=sunAngDataVec[i].theta*sin(sunAngDataVec[i].phi*3.1415/180.0);
         
         int xangbin,yangbin;
         xangbin=(int) ((xval+50.25)/xangBinsize);
         yangbin=(int) ((yval+50.25)/yangBinsize);

         int totbin=xangbin*201+yangbin;

         if(totbin>=Ntotbin)
         {
             cout<<"ERROR: Area values not available for Angles in SunAng file\n";
             return EXIT_FAILURE;
         }

         areaArr[i]=areamatrix[totbin]/areamatrix[20200];
     }


*/

/*    
    fits_read_key(fin,TLONG,"NAXIS2",&Nang,NULL, &status);
    fits_read_key(fin,TFLOAT,"ANGBINSIZE",&angBinsize,NULL, &status);

    float areamatrix[Nang];

    fits_read_col(fin,TFLOAT,2,1,1,Nang,NULL,areamatrix,NULL,&status);

    if(status) {
        cout<<"***Error in reading from file "<<areafile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
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
         int angbin;
         angbin=(int)(sunAngDataVec[i].theta/angBinsize);

         if(angbin>=Nang)
         {
             cout<<"ERROR: Area values not available for Angles in SunAng file\n";
             return EXIT_FAILURE;
         }

         areaArr[i]=areamatrix[angbin]/areamatrix[0];
     }

     return EXIT_SUCCESS;
}
*/
