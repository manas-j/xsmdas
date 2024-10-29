#include <xsmgtigen.h>

xsmgtigen::xsmgtigen()
{
    int i,j;
    for (i=0;i<totParams;i++)
        paramSelFlag[i]=0;

	// Initialize allPars here
    // Ordering of the parameter names in allPars variable is IMPORTANT. MUST BE same as order in validateRecord
	
	string parNames[totParams]={"EventCounter","EventDetected","RampCounter","HVMonitor","DetTemperature","TECCurrent",
        "LV1Monitor","LV2Monitor","LLDRefVoltage","TECRefVoltage","MotorControlMode","MotorOperationMode","MotorSetPos",
        "MotorIRPos", "IRPowerStatus","FrameDiscardFlag","MotorAutoTime","StepModeDir","WindowLowerThresh","WindowUpperThresh",
        "PileupRejMode","PileupRejTime","Ch1Start","Ch1Stop","Ch2Start","Ch2Stop","Ch3Start","Ch3Stop","ULDEvents",
        "SunTheta","SunPhi","SunFovFlag","SunOccultFlag","RamAngle","RA","DEC","SunAngRate"};

	for(i=0;i<totParams;i++) allPars[i]=parNames[i];
}

xsmgtigen::~xsmgtigen(){}

int xsmgtigen::read(int argc,char **argv)
{
    int status=0;


    runHistory.clear();
    runHistory.push_back("************** XSMGTIGEN **************");


    if(PIL_OK!=(status=PILInit(argc,argv))){
        cout<<"***Error Initializing PIL***\n";
        cout<<"*** $PFILES not set or parameter file missing ***\n";         
        return status;
    }

    if(PIL_OK!=(status=PILGetFname("hkfile",hkfile))){
        cout<<"***Error reading input HK file***\n";
        return status;
    }

    if(!FileExists(hkfile)){
        cout<<"***ERROR: This file does not exist: "<<hkfile<<"***\n";
        return EXIT_FAILURE;
    }

    if(PIL_OK!=(status=PILGetFname("gtifile",gtifile))){
        cout<<"***Error reading output GTI file***\n";
        return status;
    }

    runHistory.push_back("hkfile = "+(string)basename(hkfile));
    runHistory.push_back("gtifile = "+(string)basename(gtifile));

    if (PIL_OK != (status = PILGetBool("usesunang", &usesunang))) {
        cout << "***Error Reading usesunang ***\n";
        return status;
    }
    
    
    if(usesunang){
        runHistory.push_back("usesunang = yes");
        if(PIL_OK!=(status=PILGetFname("safile",safile))){
            cout<<"***Error reading input SunAngle file***\n";
            return status;
        }
        if(!FileExists(safile)){
            cout<<"***ERROR: This file does not exist: "<<safile<<"***\n";
            return EXIT_FAILURE;
        }

        runHistory.push_back("safile = "+(string)basename(safile));
    }
    else
        runHistory.push_back("usesunang = no");

    if(PIL_OK!=(status=PILGetFname("filterfile",filterfile))){
        cout<<"***Error reading filter file***\n";
        return status;
    }

    if(!FileExists(filterfile)){
        cout<<"***ERROR: This file does not exist: "<<filterfile<<"***\n";
        return EXIT_FAILURE;
    }
    
    runHistory.push_back("filterfile = "+(string)basename(filterfile));

    if(PIL_OK!=(status=PILGetString("usergtifile",usergtifile))){
        cout<<"***Error reading usergti file***\n";
        return status;
    }

	if(strcasecmp(usergtifile, "-") == 0)
	{
		usergtistatus=0;
	}
	else
	{
		usergtistatus=1;
    	if(!FileExists(usergtifile)){
        	cout<<"***ERROR: This file does not exist: "<<usergtifile<<"***\n";
        	return EXIT_FAILURE;
    	}
	}

    runHistory.push_back("usergtifile = "+(string)basename(usergtifile));


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

int xsmgtigen::read(char *hkfile,char *safile,char *gtifile, int clobber, int history)
{
    strcpy(this->hkfile,hkfile);
    strcpy(this->safile,safile);	
    strcpy(this->gtifile,gtifile);
    this->clobber = clobber;
    this->history = history;
    return (EXIT_SUCCESS);
}

int  xsmgtigen::display()
{
	return EXIT_SUCCESS;
}

int xsmgtigen::xsmgtigenProcess()
{
	int status=0;
    long nrows_hk=0,nrows_sa=0,i;	

    // Check the existence of files etc

	status=hkfh.read_hkfile((string)hkfile,hkDataVec);
    if(status) {
        cout<<"***ERROR: Unable to read HK file. Check the format of file\n";
        return(status);
    }
	nrows_hk=hkDataVec.size();
	
    if(usesunang){
	    status=safh.read_safile((string)safile,sunAngDataVec);
        if(status) {
            cout<<"***ERROR: Unable to read SunAng file. Check the format of file\n";
            return(status);
        }    
	    nrows_sa=sunAngDataVec.size();

        if(nrows_sa!=nrows_hk) {
            cout<<"***ERROR: Nrows in HK and SunAng files do not match\n";
            cout<<"***EXITING WITHOUT GENERATING GTI\n";    
            return EXIT_FAILURE;
        }
    }

    status=readFilterFile();
    if(status) {
        cout<<"***ERROR: Unable to read Filter file. Check the format of file\n";
        return(status);
    }

	if(usergtistatus==1){
		status=readUserGtiFile();
	    if(status) {
    	    cout<<"***ERROR: Unable to read User Gti file. Check the format of file\n";
        	return(status);
    	}
		
	}

	status=computeGTI();    
    if(status) {
        cout<<"***ERROR in GTI computation\n";
        cout<<"***EXITING WITHOUT OUTPUT FILE\n";
        return(status);
    }

    // Copy header keywords from HK file to GTI
    gtifh.date_obs=hkfh.date_obs;
    gtifh.date_end=hkfh.date_end;

    // Also add total exposure based on lines of HK file
    gtifh.tot_exposure=(double)nrows_hk;

    // Write GTI file

    status=doClobber(gtifile,clobber);
    if(status) return (status);

    status=gtifh.create_gtifile((string)gtifile);

    if(status) {
        cout<<"***ERROR: Unable to create empty GTI file"<<gtifile<<"\n";
        return(status);
    }

	status=gtifh.write_gtifile((string)gtifile,gtiDataVec,runHistory);

    if(status) {
        cout<<"***ERROR in writing to GTI file"<<gtifile<<"\n";
        return(status);
    }

    cout<<"\nMESSAGE: XSMGTIGEN completed successully\n";
    cout<<"MESSAGE: Output file = "<<gtifile<<"\n\n";
	
	return EXIT_SUCCESS;
}

int xsmgtigen::computeGTI()
{
    
	int status=0;

	long nrows=0,nrows_sa=0,i;
    nrows=hkDataVec.size();
	
	double offset1=0.01;
	double offset2=1-offset1;	

	vector <gtiData> goodFrameGTI,hkGTI,mandGTI,interGTI;

	gtiData gtiRecord;

	gtiRecord.tstart=hkDataVec[0].time-offset1;

	for(i=1;i<nrows-1;i++)
	{	
		if((hkDataVec[i].time-hkDataVec[i-1].time) > 1.5)
		{
			gtiRecord.tstop=hkDataVec[i-1].time+offset2;
			goodFrameGTI.push_back(gtiRecord);
			gtiRecord.tstart=hkDataVec[i].time-offset1;
		}
	}

	gtiRecord.tstop=hkDataVec[nrows-1].time+offset2;	
	goodFrameGTI.push_back(gtiRecord);

    computemandatoryGTI(mandGTI);

    status=find_intersecting_range(goodFrameGTI, mandGTI, interGTI);

    if(status) return (status);

	computehkGTI(hkGTI);

	if(usergtistatus==1)
	{
		vector <gtiData> interGTI1;
		status=find_intersecting_range(interGTI, hkGTI, interGTI1);
		status=find_intersecting_range(userGTI, interGTI1, gtiDataVec);
	}
	else
		status=find_intersecting_range(interGTI, hkGTI, gtiDataVec);

	return (status);
}

int xsmgtigen::computemandatoryGTI(vector <gtiData> &mandGTI)
{

    double offset1=0.01;
    double offset2=1-offset1;   

    long nrows=0,i;
    nrows=hkDataVec.size();

    gtiData gtiRecord;

    // Compute GTI excluding the HV spike and frames with eventDetected!=spectrum_events
    gtiRecord.tstart=hkDataVec[0].time-offset1;

    for(i=1;i<nrows-1;i++)
    {
        if((hkDataVec[i].hvMonitor>1.0 && hkDataVec[i-1].hvMonitor<1.0) || (abs(hkDataVec[i].eventDetected-hkDataVec[i].spectrum_events)>=100))
        {
            gtiRecord.tstop=hkDataVec[i].time-offset1;
            mandGTI.push_back(gtiRecord);
            gtiRecord.tstart=hkDataVec[i+1].time-offset1;
        }
    }

    gtiRecord.tstop=hkDataVec[nrows-1].time+offset2;
    mandGTI.push_back(gtiRecord);
   
    return EXIT_SUCCESS; 
}

int xsmgtigen::computehkGTI(vector <gtiData> &hkGTI)
{
    long nrows=0,i;
    nrows=hkDataVec.size();

    double offset1=0.01;
    double offset2=1-offset1;

    gtiData gtiRecord;
   	int gtiFlag=0;	
 
    for(i=0;i<nrows;i++)
	{
        if(validateRecord(i))         
        {
			if(gtiFlag==0)
			{
				//begin GTI
            	gtiRecord.tstart=hkDataVec[i].time-offset1;
            	gtiFlag=1;
			}
        }
		else 
		{
			if(gtiFlag==1)
			{
			    //end GTI
                gtiRecord.tstop=hkDataVec[i-1].time+offset2;
                hkGTI.push_back(gtiRecord);
			    gtiFlag=0;
			}
		}
	}

	if(gtiFlag==1) 
	{
		gtiRecord.tstop=hkDataVec[nrows-1].time+offset2;
        hkGTI.push_back(gtiRecord);
        gtiFlag=0;		
	}

    return EXIT_SUCCESS;
		
}

int xsmgtigen::readUserGtiFile()
{
    FILE *f1;
    f1=fopen(usergtifile,"r");
	double tstart,tstop;

    while(fscanf(f1,"%lf %lf",&tstart,&tstop) != EOF)
	{
		gtiData gtiRecord;
		gtiRecord.tstart=tstart;
		gtiRecord.tstop=tstop;
		userGTI.push_back(gtiRecord);	
	}

    fclose(f1);

	return EXIT_SUCCESS;
}

int xsmgtigen::readFilterFile()
{
    FILE *f1;
    char pname[40];
    char rangestr[40];
    int nrange;
    int status=0,i;
    int iPar=0;
    bool flag=false;

    f1=fopen(filterfile,"r");

    while(fscanf(f1,"%s",pname) != EOF)
    {

        flag=false;
        for(i=0; i<totParams; i++){
            if((string)pname == allPars[i]){
                flag=true;
                iPar=i;
                break;
            }
        }

        if(flag && paramSelFlag[iPar]==0){

            if(fscanf(f1,"%d",&nrange) ==EOF){
                cout<<"***ERROR: Unexpected End of filter file. Incorrect format\n";
                status=EXIT_FAILURE;
                break;
            }
            else if(nrange > maxRanges){
                cout<<"***ERROR: Nrange for "<<pname<<" greater than maximum: "<<maxRanges<<"\n";
                status=EXIT_FAILURE;
                break;                
            }
            else if(nrange < 1){
                cout<<"***ERROR: Nrange for "<<pname<<" less than 1 which is not allowed\n";
                status=EXIT_FAILURE;
                break;
            }
            else{
                if(usesunang==0 && iPar>28) {
                    cout<<"WARNING: Found SunAngFile Parameter "<<pname<<" in filterfile\n";
                    cout<<"WARNING: Will be ignored as usesunang=no\n";
                }
                paramSelFlag[iPar]=nrange;
                for(i=0;i<nrange*2;i++){

                    if(fscanf(f1,"%s",rangestr)==EOF){
                        cout<<"***ERROR: Unexpected End of filter file. Incorrect format\n";
                        status=EXIT_FAILURE;
                        break;
                    }
                    else
                        paramLimits[iPar][i]=(string)rangestr;
                }

                if(iPar==3 && ((nrange!=1) || (atof((char*) paramLimits[iPar][0].c_str())!=1.0) || (paramLimits[iPar][1]!="-")))
                    cout<<"WARNING: Parameter ranges given for HV Monitor are not the recommended values\n";

                if(iPar==4 && ((nrange!=1) || (atof((char*) paramLimits[iPar][0].c_str())!=1.8) || (paramLimits[iPar][1]!="-")))
                    cout<<"WARNING: Parameter ranges given for Det Temperature are not the recommended values\n";
                    
                if(iPar==6 && ((nrange!=1) || (atof((char*) paramLimits[iPar][0].c_str())!=3.1) || (atof((char*) paramLimits[iPar][1].c_str())!=3.4)))
                    cout<<"WARNING: Parameter ranges given for LV1 Monitor are not the recommended values\n";

                if(iPar==7 && ((nrange!=1) || (atof((char*) paramLimits[iPar][0].c_str())!=1.4) || (atof((char*) paramLimits[iPar][0].c_str())!=1.6)))
                    cout<<"WARNING: Parameter ranges given for LV2 Monitor are not the recommended values\n";

                if(iPar==15 && ((nrange!=1) || (atof((char*) paramLimits[iPar][0].c_str())!=0) || (atof((char*) paramLimits[iPar][0].c_str())!=0)))
                    cout<<"WARNING: Parameter ranges given for FrameDiscardFlag are not the recommended values\n";
                       
            }

        }
        else if (!flag){

            cout<<"***ERROR: Unknown Parameter name in filter file. Incorrect format\n";
            status=EXIT_FAILURE;
            break;
        }
        else {
            cout<<"***ERROR: Repeting Parameter name "<<pname<<" in filter file. Incorrect format\n";
            status=EXIT_FAILURE;
            break;
        }

    }

    fclose(f1);

    //Push filterfile contents to runHistory
    runHistory.push_back("--------------  Filterfile  ---------------");
    char fileLine[300];
    f1=fopen(filterfile,"r");
    while (fgets(fileLine, sizeof(fileLine), f1)) 
        runHistory.push_back((string)fileLine); 
    fclose(f1);
    runHistory.push_back("-------------------------------------------");


    return (status);
}

// Verify whether the HK and Sun Angle Parameters of a row is within range specified by filter file
// Ordering in switch case IS SAME AS that of allPars defined in include file
int xsmgtigen::validateRecord(long row)
{
    int flag=1,iPar;

    for(iPar=0;iPar<totParams;iPar++){

        if((usesunang || iPar<=28) && paramSelFlag[iPar]>0){
            switch(iPar){
                
                case 0:
                    flag=isInRange(hkDataVec[row].eventCounter,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 1:
                    flag=isInRange(hkDataVec[row].eventDetected,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 2:
                    flag=isInRange(hkDataVec[row].rampCounter,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 3:
                    flag=isInRange(hkDataVec[row].hvMonitor,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 4:
                    flag=isInRange(hkDataVec[row].detTemperature,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 5:
                    flag=isInRange(hkDataVec[row].tecCurent,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 6:
                    flag=isInRange(hkDataVec[row].lv1Monitor,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 7:
                    flag=isInRange(hkDataVec[row].lv2Monitor,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 8:
                    flag=isInRange(hkDataVec[row].lldRefVoltage,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 9:
                    flag=isInRange(hkDataVec[row].tecRefVoltage,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 10:
                    flag=isInRange(hkDataVec[row].motor_control_mode,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 11:
                    flag=isInRange(hkDataVec[row].motor_operation_mode,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 12:
                    flag=isInRange(hkDataVec[row].motor_manual_set_position,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 13:
                    flag=isInRange(hkDataVec[row].motor_ir_position,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 14:
                    flag=isInRange(hkDataVec[row].ir_power_status,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 15:
                    flag=isInRange(hkDataVec[row].frame_discard,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 16:
                    flag=isInRange(hkDataVec[row].motor_automovement_time,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 17:
                    flag=isInRange(hkDataVec[row].motor_stepmode_direction,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 18:
                    flag=isInRange(hkDataVec[row].window_lower_thresh,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 19:
                    flag=isInRange(hkDataVec[row].window_upper_thresh,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 20:
                    flag=isInRange(hkDataVec[row].pileup_rejection_mode,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 21:
                    flag=isInRange(hkDataVec[row].pileup_time,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 22:
                    flag=isInRange(hkDataVec[row].ch1_start,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 23:
                    flag=isInRange(hkDataVec[row].ch1_stop,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 24:
                    flag=isInRange(hkDataVec[row].ch2_start,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 25:
                    flag=isInRange(hkDataVec[row].ch2_stop,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 26:
                    flag=isInRange(hkDataVec[row].ch3_start,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 27:
                    flag=isInRange(hkDataVec[row].ch3_stop,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 28:
                    flag=isInRange(hkDataVec[row].uld_events,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 29:
                    flag=isInRange(sunAngDataVec[row].theta,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 30:
                    flag=isInRange(sunAngDataVec[row].phi,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 31:
                    flag=isInRange(sunAngDataVec[row].fovFlag,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 32:
                    flag=isInRange(sunAngDataVec[row].occultFlag,paramSelFlag[iPar],paramLimits[iPar]);
                    break;                    
                case 33:
                    flag=isInRange(sunAngDataVec[row].ramAngle,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 34:
                    flag=isInRange(sunAngDataVec[row].ra,paramSelFlag[iPar],paramLimits[iPar]);
                    break;                        
                case 35:
                    flag=isInRange(sunAngDataVec[row].dec,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
                case 36:
                    flag=isInRange(sunAngDataVec[row].angRate,paramSelFlag[iPar],paramLimits[iPar]);
                    break;
            }
        
            if(!flag) return(flag);
        }

    }

    return(flag);
}

template <class T>
bool xsmgtigen::isInRange(T val,int nrange,string ranges[maxRanges*2])
{
    int i;
    string minValue;
    string maxValue;
    double minVal;
    double maxVal;
    bool x=false;

    for(i=0;i<nrange;i++){

        minValue=ranges[i*2];
        maxValue=ranges[i*2+1];
        if (minValue == "-" && maxValue != "-") {
            maxVal = atof((char*) maxValue.c_str());
            x = (val <= (T) maxVal) ? true : false;
        } 
        else if (minValue != "-" && maxValue == "-") {
            minVal = atof((char*) minValue.c_str());
            x = (val >= (T) minVal) ? true : false;
        } 
        else if (minValue != "-" && maxValue != "-") {
            maxVal = atof((char*) maxValue.c_str());
            minVal = atof((char*) minValue.c_str());
            x = (val >= (T) minVal && val <= (T) maxVal) ? true : false;
        } 
        else {
            x = true;
        }
    
        // Different ranges of a parameter are to be 'OR'd
        if(x) return(x);
    }

    return x;
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
