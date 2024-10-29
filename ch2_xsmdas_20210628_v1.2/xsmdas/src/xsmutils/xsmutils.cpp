/* XSMUTILS.CPP 
 *
 * Mithun NPS 
 * */
#include "xsmutils.h"

int create_empty_fitsfile(string outFilename, string outTemplate){

    int status=0; //status variable
    fitsfile *fptr;
    string templateFileName = ""; //full path to template file.
	int hdutype;

	string basePath = getenv("xsmdas");
	templateFileName=basePath+"/templates/"+outTemplate;

    if(templateFileName==""){
        cout<< "Not able to generate Output file template path.";
        cout<< "Probably Environment Variables are not declared properly.";
        return(EXIT_FAILURE);
    }

    fits_create_template(&fptr, (char*) outFilename.c_str(), (char*) templateFileName.c_str(), &status);
    if(status){
        cout << "Error in creating file : " << outFilename;
        fits_report_error(stderr, status);
        return(EXIT_FAILURE);
    }

    fits_close_file(fptr, &status);
    if (status) {
        cout << "***Error in closing file : " << outFilename << "***";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    // Write file name and creation date in header
    fits_open_file(&fptr, (char*) outFilename.c_str(), READWRITE, &status);
    if (status) {
        cout <<"Error in opening file "<<outFilename<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_update_key(fptr, TSTRING, "FILENAME",(char*) basename((char *)outFilename.c_str()), NULL, &status);

    string timeStr;
    getCurrentTime(timeStr);

    fits_update_key(fptr, TSTRING, "DATE",(char*) timeStr.c_str(), NULL, &status);

    char version[10];
    sprintf(version,"%.2lf",XSMDAS_VERSION);

    fits_update_key(fptr, TSTRING, "XSMDASVE", version, NULL, &status);

    fits_movabs_hdu(fptr, 2, &hdutype, &status);
    if (status) {
		cout <<"***Error in moving to bintable HDU in file "<<outFilename<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_update_key(fptr, TSTRING, "XSMDASVE", version, NULL, &status);
    fits_update_key(fptr, TSTRING, "FILENAME",(char*) basename((char *)outFilename.c_str()), NULL, &status);
    fits_update_key(fptr, TSTRING, "DATE",(char*) timeStr.c_str(), NULL, &status);

    fits_close_file(fptr, &status);
    if (status) {
        cout << "***Error in closing file : " << outFilename << "***";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }


    return (EXIT_SUCCESS);
}

int writeHistory2FITS(fitsfile *fout,vector <string> runHistory)
{
    int i,status=0;

    for(i=0;i<runHistory.size();i++)
    {
       fits_write_history(fout, (char *) runHistory[i].c_str(), &status);
       if (status) {
           cout << "Error writing History at writeHistory2FITS()\n";
           fits_report_error(stderr, status);
           return (EXIT_FAILURE);
       }
    }

    return EXIT_SUCCESS;
}

int getCaldbVer(string &caldbver)
{
    int status=0;
    fitsfile *fptr;
    string cifFile="";
    int hdutype=0;
    long nrows,i;

    string basePath = getenv("xsmdas");
    cifFile=basePath+"/caldb/xsmcaldb.indx";

    if(!FileExists((char *)cifFile.c_str())){
        cout<<"***ERROR: The CALDB index file does not exist: "<<cifFile<<"***\n";
        return EXIT_FAILURE;
    }

    //Open CIF file and move to 1st extension
    fits_open_file(&fptr,cifFile.c_str(),READONLY,&status);
    if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

    fits_movabs_hdu(fptr,2,&hdutype,&status);
    if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

	char caldb_version[100];

	fits_read_key(fptr, TSTRING, "CALDBVER", caldb_version, NULL, &status);
    if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

	caldbver=string(caldb_version);

    fits_close_file(fptr, &status);
    if (status) {
        cout << "***Error in closing file : " << cifFile << "***";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }


    return EXIT_SUCCESS;

}	

int queryCaldb(string codename,double tstart,string &caldb_filename)
{
    int status=0; 
    fitsfile *fptr;
    string cifFile=""; 
    int hdutype=0;
    long nrows,i;

    string basePath = getenv("xsmdas");
    cifFile=basePath+"/caldb/xsmcaldb.indx";

    if(!FileExists((char *)cifFile.c_str())){
        cout<<"***ERROR: The CALDB index file does not exist: "<<cifFile<<"***\n";
        return EXIT_FAILURE;
    }

    //Open CIF file and move to 1st extension
    fits_open_file(&fptr,cifFile.c_str(),READONLY,&status);
    if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

    fits_movabs_hdu(fptr,2,&hdutype,&status);
    if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

    fits_get_num_rows(fptr, &nrows, &status);
    if(status) { fits_report_error(stderr,status);  return (EXIT_FAILURE); }

    if(nrows==0)
    {
        cout<<"CALDB CIF file seems to be empty...\n";
        return(EXIT_FAILURE);
    }


    double reftime;
    int calflag;
    char *cname,*cfile,*cdesc;
    cname=(char*)malloc(sizeof(char )*20);
    cfile=(char*)malloc(sizeof(char )*40);
    cdesc=(char*)malloc(sizeof(char )*70);

    double dataMJD=57754.0+tstart/86400.0;

    int row_flag[nrows];

    for(i=0;i<nrows;i++)
    {
        row_flag[i]=0;
        fits_read_col_str(fptr,1,i+1,1,1,NULL,&cname,NULL,&status);
        fits_read_col_str(fptr,2,i+1,1,1,NULL,&cfile,NULL,&status);
        fits_read_col(fptr,TDOUBLE,3,i+1,1,1,NULL,&reftime,NULL,&status);
        fits_read_col(fptr,TINT,4,i+1,1,1,NULL,&calflag,NULL,&status);

        if(status) { fits_report_error(stderr,status);  return (EXIT_FAILURE); }

        if((!strcasecmp(cname,codename.c_str())) && calflag==0 && dataMJD>reftime)
                row_flag[i]=1;
    }

    double maxRefTime=-1;
    long maxRefTime_index=-1;

    //Select the latest of the data sets
    for(i=0;i<nrows;i++)
    {
         if(row_flag[i]==1)
         {

            fits_read_col(fptr,TDOUBLE,3,i+1,1,1,NULL,&reftime,NULL,&status);
            if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

            if(reftime>maxRefTime)
            {
                maxRefTime=reftime;
                maxRefTime_index=i;
            }
         }
    }


    if(maxRefTime_index==-1){
        cout<<"***Error: No valid calibration data set for "<<codename<<" for the given time\n";
        return EXIT_FAILURE;
    }
    else{
        
        fits_read_col_str(fptr,2,maxRefTime_index+1,1,1,NULL,&cfile,NULL,&status);
        if(status) {fits_report_error(stderr,status); return(EXIT_FAILURE);}

        caldb_filename=basePath+"/caldb/"+(string)cfile;

        if(!FileExists((char *)caldb_filename.c_str())){
            cout<<"***ERROR: This CALDB file for "<<codename<<" named "<<caldb_filename<<" does not exist***\n";
            return EXIT_FAILURE;
        }
    }

	free(cname);
	free(cfile);
	free(cdesc);

    fits_close_file(fptr, &status);
    if (status) {
        cout << "***Error in closing file : " << cifFile << "***";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

	

    return EXIT_SUCCESS;
}

void print_banner(char *module_name)
{
    cout<<"-------------------------------------------------------------------------\n";
    cout<<"  XSMDAS: Data Analysis Software for Chandrayaan-II Solar X-ray Monitor  \n";
    cout<<"                     XSMDAS Version: "<<XSMDAS_VERSION<<"              \n"; 
    cout<<"                     Module : "<< module_name<<"                        \n";
    cout<<"-------------------------------------------------------------------------\n";

}

bool FileExists(char *filename) {
    
	struct stat filestat;
    int filest;
    filestat.st_mode = -1;
    bool exist = false;

    filest = stat(filename, &filestat);

    if (S_ISREG(filestat.st_mode)) {
        exist = true;
    } else exist = false;

    return exist;
}

int isStringValid(string inString, vector<string> sampleStrings){
    int istring=0;
    int badFlag=TRUE;

    for(istring=0; istring<sampleStrings.size(); istring++){
        if(inString == sampleStrings[istring]){
            badFlag=FALSE;
            break;
        }
    }
    return badFlag;
}

bool getMatchingString(string inString, vector<string> sampleStrings,int &istring){
    int i;
    bool flag=false;

    for(i=0; i<sampleStrings.size(); i++){
        if(inString == sampleStrings[i]){
            flag=true;
            istring=i;
            break;
        }
    }
    
}

bool checkRepetingString(vector<string> sampleStrings){

	int i,j;
    bool flag=false;

    for(i=0; i<sampleStrings.size(); i++){
		for(j=0;j<sampleStrings.size();j++){
			if(j!=i){
				if(sampleStrings[i]==sampleStrings[j]){
					flag=true;
					return(flag);
				}
			}
		}
	}

	return(flag);
}

int doClobber(char *outfile,int clobber)
{
    if (clobber == YES && FileExists(outfile))
    {
        cout << outfile << "  :FileExists.. Replacing the old file\n";
        if (remove(outfile) != 0) {
            cout << "***Error in deleting " << outfile << "***\n";
            return (EXIT_FAILURE);
        }
    }
    else if (FileExists(outfile)){
            cout << "***Output file already exists***\n";
            cout << "Use clobber=yes for overwriting the file\n";
            return (EXIT_FAILURE);
    }	

	return EXIT_SUCCESS;
}

int getCurrentTime(string &timeString)
{
    time_t rawtime;
    struct tm * timeinfo;
    char timeStr[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(timeStr,sizeof(timeStr),"%Y-%m-%dT%H:%M:%S",timeinfo);

    timeString=(string)timeStr;
    
    return EXIT_SUCCESS;
}

int validateDate(int UTCYear,int UTCMonth,int UTCDate)
{

	if(UTCMonth < 1 || UTCMonth > 12)
	{
		cout<<"There are only 12 Months a year\n";
		return (EXIT_FAILURE);
	}

    double dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    if((UTCYear%4)==0) dom[1]=29;

	if(UTCDate>dom[UTCMonth-1]){
		cout<<"*** There are only "<<dom[UTCMonth-1]<<" days in the month "<<UTCMonth<<"-"<<UTCYear<<"\n";
		return (EXIT_FAILURE);
	}
	
	return (EXIT_SUCCESS);
}

// Compute Mission Elapse Time (Seconds since 2017/01/01 00:00:00 UTC)
double computeMET(long UTCYear,long UTCMonth,long UTCDate,long UTCHour,long UTCMin,long UTCSec,float UTCms)
{
	double UTCMET=0.0;
	int i;
    double msValue=UTCms/1000.0;

    long dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	if((UTCYear%4)==0) dom[1]=29;

	for(i=1;i<UTCMonth;i++) UTCMET+=((double)dom[i-1]*24.0*3600.0);

	UTCMET+=(((double)UTCDate-1.0)*24.0*3600.0)+(double)UTCHour*3600.0+(double)UTCMin*60.0+(double)UTCSec+msValue;

	for(i=2017;i<UTCYear;i++)
	{
		if(i%4==0) UTCMET+=(double)366.0*24.0*3600.0;
		else UTCMET+=(double)365.0*24.0*3600.0;
	}	

//  printf("%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\n",UTCYear, UTCMonth,UTCDate,UTCHour,UTCMin,UTCSec,UTCms);

	return (UTCMET);
}


/*
int headas_chat(int threshold, const char* fmt, ...){
    va_list ap;
    int status=0;

    if (headas_chatpar < 0){
    fprintf(stderr,"Error: headas_chat(): no chatter parameter present\n");
    status = 1;
    return status;
    }

    if (headas_chatpar >= threshold){
    va_start(ap, fmt);
    status = vfprintf(heaout, fmt, ap);
    va_end(ap);
    }

    return status;
}

*/



