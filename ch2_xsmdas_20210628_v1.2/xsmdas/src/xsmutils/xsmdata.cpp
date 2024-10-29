/* XSMDATA.CPP
 *
 * Mithun NPS 
 * */
#include "xsmdata.h"

// class frame

// Decoding as per PE data format V6 (02/12/2016) 
// THIS FORMAT IS NOW FROZEN
int frame::decode_frame(unsigned char *data,bool decode_hdr,bool decode_lc,bool decode_spec)
{
	int i;
/*
	//Initialize spec structure
	
    for(i=0;i<SPEC_SIZE;i++)
	 	spec.spectrum[i]=0;
*/
    long frame_no,fpga_time;
    long slow_counter,slow_detected;

    fpga_time=word(data[4],data[5]);
    fpga_time=word(fpga_time,data[6]);
    fpga_time=word(fpga_time,data[7]);
    fpga_time=word(fpga_time,data[8]);
    fpga_time=word(fpga_time,data[9]);

    slow_counter=word(data[14],data[15]);
    slow_counter=word(slow_counter,data[16]);
    slow_counter=word(slow_counter,data[17]);

    slow_detected=word(data[18],data[19]);
    slow_detected=word(slow_detected,data[20]);
    slow_detected=word(slow_detected,data[21]);

	if(decode_hdr){

        hdr.sync_UW=(long)word(data[0],data[1]);
        hdr.sync_LW=(long)word(data[2],data[3]);

        frame_no=word(data[10],data[11]);
        frame_no=word(frame_no,data[12]);
        frame_no=word(frame_no,data[13]);


		hdr.frame_no=frame_no;	
	
		hdr.fpga_time=fpga_time;

		hdr.event_counter=slow_counter;
		hdr.event_detected=slow_detected;

		hdr.ramp_counter=word(data[22],data[23]);

        hdr.hv_monitor=word(data[24],data[25]);
        hdr.temperature=word(data[26],data[27]);
        hdr.tec_current=word(data[28],data[29]);
        hdr.lv1_monitor=word(data[30],data[31]);
        hdr.lv2_monitor=word(data[32],data[33]);

		hdr.ch1_start=word(data[34],data[35]);
        hdr.ch1_stop=word(data[36],data[37]);		
		hdr.ch2_start=word(data[38],data[39]);
        hdr.ch2_stop=word(data[40],data[41]); 		
        hdr.ch3_start=word(data[42],data[43]);
        hdr.ch3_stop=word(data[44],data[45]); 
	
		hdr.lld_voltage=word(data[46],data[47]);
		hdr.tecref_voltage=word(data[48],data[49]);

		hdr.window_lower_thresh=(long)data[51]*100;
		hdr.window_upper_thresh=(long)data[50]*1000;

		//Based on flags in byte 52,53,54,55
		
		int manualm,autom,forcestepm;
		manualm=(int)((data[53]&0x01));
		autom=(int)((data[53]&0x02)>>1);
		forcestepm=(int)((data[52]&0x04)>>2);

		if(autom==1&&forcestepm==0&&manualm==0)
			hdr.motor_control_mode=0;
		else if(manualm==1&&forcestepm==0&&autom==0)
			hdr.motor_control_mode=1;
		else if(forcestepm==1)
			 hdr.motor_control_mode=2;
		else
			 hdr.motor_control_mode=5;

		int irmode,countermode,stepmode;
		irmode=(int)((data[52]&0x80)>>7);
		countermode=(int)((data[55]&0x01));
		stepmode=(int)((data[55]&0x02)>>1);

		if(irmode==1&&countermode==0&&stepmode==0)
			hdr.motor_operation_mode=0; //ir
		else if (countermode==1&&stepmode==0&&irmode==0)
			hdr.motor_operation_mode=1; //counter
		else if (stepmode==1&&countermode==0&&irmode==0)
			hdr.motor_operation_mode=2;	//step
		else
			hdr.motor_operation_mode=5;
        
		if(((data[53]&0x80)>>7)==0)
			hdr.motor_automovement_time=100;
		else 
            hdr.motor_automovement_time=1000;

		if(((data[53]&0x40)>>6)==0)
			hdr.pileup_time=5;
		else
			hdr.pileup_time=10;

		hdr.pileup_rejection_mode=((data[53]&0x20)>>5);
		hdr.motor_stepmode_direction=((data[52]&0x08)>>3);
		hdr.ir_power_status=((data[52]&0x02)>>1);
		hdr.frame_discard=((data[52]&0x01));

		hdr.guardbits=word(data[54],(data[55]>>2));

		int setopen,setbe,setcal;

		setopen=(int)((data[53]&0x10)>>4);
		setbe=(int)((data[53]&0x08)>>3);
		setcal=(int)((data[53]&0x04)>>2);

		if(setopen==1&&setcal==0&&setbe==0)
			hdr.motor_manual_set_position=0;
		else if(setcal==1&&setbe==0&&setopen==0)
            hdr.motor_manual_set_position=1;		
		else if(setbe==1&&setopen==0&&setcal==0)
            hdr.motor_manual_set_position=2;
		else 
		    hdr.motor_manual_set_position=5;

		int openir,beir,calir;

		openir=(int)((data[52]&0x40)>>6);
		beir=(int)((data[52]&0x20)>>5);
		calir=(int)((data[52]&0x10)>>4);
		
		if(openir==1&&calir==0&&beir==0)
			hdr.motor_ir_position=0;
		else if(calir==1&&openir==0&&beir==0)
            hdr.motor_ir_position=1;
		else if(beir==1&&calir==0&&openir==0)
            hdr.motor_ir_position=2;
		else
            hdr.motor_ir_position=5;

		// Decode Coarse channel LC and get total

		hdr.coarse_channel_events=0;

      	for(i=0;i<10;i++)
        {
            hdr.coarse_channel_events+=(long)(word(data[6*i+LCSTART_BYTE],data[6*i+1+LCSTART_BYTE]));
            hdr.coarse_channel_events+=(long)(word(data[6*i+2+LCSTART_BYTE],data[6*i+3+LCSTART_BYTE]));
            hdr.coarse_channel_events+=(long)(word(data[6*i+4+LCSTART_BYTE],data[6*i+5+LCSTART_BYTE]));
        }

		//Decode spectrum for total and uld
		hdr.spectrum_events=0;
        for(i=0;i<963;i++)
            hdr.spectrum_events+=(long)(word(data[2*i+HDR_SIZE],data[2*i+1+HDR_SIZE]));
		hdr.uld_events=(long)(word(data[2*962+HDR_SIZE],data[2*962+1+HDR_SIZE]));

	}

	// In this version LCSTART_BYTE=56
	
	if(decode_lc){

		for(i=0;i<10;i++)
		{
			lc.ch1lc[i]=word(data[6*i+LCSTART_BYTE],data[6*i+1+LCSTART_BYTE]);
			lc.ch2lc[i]=word(data[6*i+2+LCSTART_BYTE],data[6*i+3+LCSTART_BYTE]);
			lc.ch3lc[i]=word(data[6*i+4+LCSTART_BYTE],data[6*i+5+LCSTART_BYTE]);
		}
	
	}

    //In this version HDR_SIZE=
   	if(decode_spec){

		for(i=0;i<963;i++)
			spec.spectrum[i]=word(data[2*i+HDR_SIZE],data[2*i+1+HDR_SIZE]);
		for(i=963;i<SPEC_SIZE;i++)
			spec.spectrum[i]=0;		
        
        spec.event_counter=slow_counter;
        spec.event_detected=slow_detected;
    
	}
    
	return EXIT_SUCCESS;
}

/*
// Decoding as per PE data format V5 (02/12/2016)
int frame::decode_frame_v5(unsigned char *data,bool decode_hdr,bool decode_lc,bool decode_spec)
{

	int i;
    long frame_no,fpga_time;
    long slow_counter,slow_detected;

    fpga_time=word(data[4],data[5]);
    fpga_time=word(fpga_time,data[6]);
    fpga_time=word(fpga_time,data[7]);
    fpga_time=word(fpga_time,data[8]);
    fpga_time=word(fpga_time,data[9]);

    slow_counter=word(data[14],data[15]);
    slow_counter=word(slow_counter,data[16]);
    slow_counter=word(slow_counter,data[17]);

    slow_detected=word(data[18],data[19]);
    slow_detected=word(slow_detected,data[20]);
    slow_detected=word(slow_detected,data[21]);

	if(decode_hdr){

        hdr.sync_UW=(long)word(data[0],data[1]);
        hdr.sync_LW=(long)word(data[2],data[3]);

        frame_no=word(data[10],data[11]);
        frame_no=word(frame_no,data[12]);
		
		hdr.frame_no=frame_no;	
	
		hdr.fpga_time=fpga_time;

		hdr.slow_counter=slow_counter;
		hdr.slow_detected=slow_detected;

		hdr.ramp_counter=word(data[22],data[23]);

        hdr.hv_monitor=word(data[24],data[25]);
        hdr.temperature=word(data[26],data[27]);
        hdr.tec_current=word(data[28],data[29]);
        hdr.lv1_monitor=word(data[30],data[31]);
        hdr.lv2_monitor=word(data[32],data[33]);
        hdr.rtd1_fpga=word(data[34],data[35]);
        hdr.rtd2_sensor=word(data[36],data[37]);
        hdr.rtd3_relay=word(data[38],data[39]);


		hdr.ch1_start=word(data[40],data[41]);
        hdr.ch1_stop=word(data[42],data[43]);		
		hdr.ch2_start=word(data[44],data[45]);
        hdr.ch2_stop=word(data[46],data[47]); 		
        hdr.ch3_start=word(data[48],data[49]);
        hdr.ch3_stop=word(data[50],data[51]); 
	
		hdr.lld_voltage=word(data[52],data[53]);
		hdr.tecref_voltage=word(data[54],data[55]);

		hdr.window_lower_thresh=(int)data[56];
		hdr.window_upper_thresh=(int)data[57];

		//Include flags in byte 58,59,60,61

	}

	if(decode_lc){

		for(i=0;i<10;i++)
		{
			lc.ch1lc[i]=word(data[6*i+LCSTART_BYTE],data[6*i+1+LCSTART_BYTE]);
			lc.ch2lc[i]=word(data[6*i+2+LCSTART_BYTE],data[6*i+3+LCSTART_BYTE]);
			lc.ch3lc[i]=word(data[6*i+4+LCSTART_BYTE],data[6*i+5+LCSTART_BYTE]);
		}
	
	}

    if(decode_spec){

		for(i=0;i<963;i++)
			spec.spectrum[i]=word(data[2*i+HDR_SIZE],data[2*i+1+HDR_SIZE]);
		for(i=963;i<SPEC_SIZE;i++)
			spec.spectrum[i]=0;		
        
        spec.slow_counter=slow_counter;
        spec.slow_detected=slow_detected;
    
	}
    
	return EXIT_SUCCESS;
}



// Frame decoding as per PE data format V4(For data before 30/11/2016)
int frame::decode_frame_v4(unsigned char *data,bool decode_hdr,bool decode_lc,bool decode_spec)
{


//#define LCSTART_BYTE 54
//#define HDR_SIZE 128 

	int i;
    long frame_no,fpga_time;
    long fast_counter,slow_counter,slow_detected;

    fpga_time=wordL(data[8],data[9]);
    fpga_time=wordL(data[7],fpga_time);
    fpga_time=wordL(data[6],fpga_time);
    fpga_time=wordL(data[5],fpga_time);
    fpga_time=wordL(data[4],fpga_time);

    fast_counter=wordL(data[14],data[15]);
    fast_counter=wordL(data[13],fast_counter);

    slow_counter=wordL(data[17],data[18]);
    slow_counter=wordL(data[16],slow_counter);

    slow_detected=wordL(data[20],data[21]);
    slow_detected=wordL(data[19],slow_detected);

	if(decode_hdr){

        hdr.sync_LW=(long)wordL(data[0],data[1]);
        hdr.sync_UW=(long)wordL(data[2],data[3]);

        frame_no=wordL(data[11],data[12]);
        frame_no=wordL(data[10],frame_no);
		
		hdr.frame_no=frame_no;	
	
		hdr.fpga_time=fpga_time;

		hdr.fast_counter=fast_counter;	
		hdr.slow_counter=slow_counter;
		hdr.slow_detected=slow_detected;

		hdr.ramp_counter=wordL(data[22],data[23]);

        hdr.hv_monitor=wordL(data[24],data[25]);
        hdr.temperature=wordL(data[26],data[27]);
        hdr.tec_current=wordL(data[28],data[29]);
        hdr.lv1_monitor=wordL(data[30],data[31]);
        hdr.lv2_monitor=wordL(data[32],data[33]);
	
		hdr.ch1_start=wordL(data[34],data[35]);
        hdr.ch1_stop=wordL(data[36],data[37]);		
		hdr.ch2_start=wordL(data[38],data[39]);
        hdr.ch2_stop=wordL(data[40],data[41]); 		
        hdr.ch3_start=wordL(data[42],data[43]);
        hdr.ch3_stop=wordL(data[44],data[45]); 
	
		hdr.lld_voltage=wordL(data[46],data[47]);
		hdr.tecref_voltage=wordL(data[48],data[49]);

		hdr.window_lower_thresh=(int)data[50];
		hdr.window_upper_thresh=(int)data[51];

	}

	if(decode_lc){

		for(i=0;i<10;i++)
		{
			lc.ch1lc[i]=wordL(data[6*i+LCSTART_BYTE],data[6*i+1+LCSTART_BYTE]);
			lc.ch2lc[i]=wordL(data[6*i+2+LCSTART_BYTE],data[6*i+3+LCSTART_BYTE]);
			lc.ch3lc[i]=wordL(data[6*i+4+LCSTART_BYTE],data[6*i+5+LCSTART_BYTE]);
		}
	
	}

    if(decode_spec){

		for(i=0;i<960;i++)
			spec.spectrum[i]=wordL(data[2*i+HDR_SIZE],data[2*i+1+HDR_SIZE]);
		for(i=961;i<SPEC_SIZE;i++)
			spec.spectrum[i]=0;		
        
		spec.fast_counter=fast_counter;
        spec.slow_counter=slow_counter;
        spec.slow_detected=slow_detected;
    
	}
    
	return EXIT_SUCCESS;
}
*/

//class specData

specData::specData() {}

specData::~specData(){
}

void specData::initialize(int chantype,int n_PIbin)
{
	int i;
	this->chantype=chantype;
	this->n_PIbin=n_PIbin;

	if(chantype==1) this->nchan=SPEC_SIZE;
    else if(chantype==2) this->nchan=n_PIbin;	

	this->spectrum=(float *)malloc(sizeof(float)*nchan);
	for(i=0;i<nchan;i++) this->spectrum[i]=0.0;

    this->syserror=(float *)malloc(sizeof(float)*nchan);
    for(i=0;i<nchan;i++) this->syserror[i]=0.0;

	
	exposure=0.0;
}

void specData::makeFree()
{
    free(spectrum);
    free(syserror);
}

void specData::addSpec(long *fspec,float *fsyserr,double fexp)
{
	int j;

	for(j=0;j<nchan;j++) 
    {
        this->spectrum[j]+=(float)(fspec[j]);
        this->syserror[j]+=fsyserr[j];
    }
    exposure+=fexp;

}

void specData::addSpecArea(long *fspec,float *fsyserr,double fexp,double *area)
{
    int j;

    for(j=0;j<nchan;j++) 
    {
        this->spectrum[j]+=(float)((float)fspec[j]/area[j]);
        this->syserror[j]+=fsyserr[j]/area[j];
    }

    exposure+=fexp;

}


void specData::setTimes(double startT,double stopT)
{
	tstart=startT;
	tstop=stopT;
}

void specData::setFilter(unsigned char filterStatus)
{
    this->filterStatus=filterStatus;
}

//class arfData

arfData::arfData() {}

arfData::~arfData(){
}

void arfData::initialize()
{
    int i;
    
    nchan=N_RESPENEBINS;
    for(i=0;i<nchan;i++) this->area[i]=0.0;
    exposure=0.0;
}

void arfData::addArea(double *areaperframe,double fexp)
{
    int j;

    for(j=0;j<nchan;j++)
        this->area[j]+=(float)(areaperframe[j]);

    this->exposure+=fexp;
}


void arfData::setTimes(double startT,double stopT)
{
    tstart=startT;
    tstop=stopT;
}


// gtiData structure: Independant functions

double computeExposure(vector<gtiData> gtiDataVec)
{
	long nrows=gtiDataVec.size();
	long i;

	double exposure=0.0;

	for (i=0;i<nrows;i++)
		exposure+=(double)(gtiDataVec[i].tstop-gtiDataVec[i].tstart);

	return exposure;
}

int gtiFilterScienceData(vector <scienceData> &scDataVec,vector<gtiData> gtiDataVec)
{
	long nrowsSc,nrowsGti,i,j;

	nrowsSc=scDataVec.size();
	nrowsGti= gtiDataVec.size();

	vector <scienceData> filtScDataVec;
	filtScDataVec.clear();

	if(nrowsGti==0)
	{
		printf("No valid rows in GTI. Quitting selection by GTI\n");
		return EXIT_FAILURE;
	}

	if(nrowsSc==0)
    {
        printf("No valid rows in Science Data. Quitting selection by GTI\n");
        return EXIT_FAILURE;
    }


	j=0;

	for(i=0;i<nrowsSc;i++)
	{
		while(scDataVec[i].time > gtiDataVec[j].tstop && j<nrowsGti-1) j++;
		
		if(j==nrowsGti) break;

		if(scDataVec[i].time > gtiDataVec[j].tstart && scDataVec[i].time < gtiDataVec[j].tstop)
		{
			filtScDataVec.push_back(scDataVec[i]);
		}
	}

	if(filtScDataVec.size()==0)
	{
		printf("No frames available after filtering. No output to be generated\n");
		return EXIT_FAILURE;
	}

	scDataVec.clear();
	scDataVec=filtScDataVec;

	return EXIT_SUCCESS;

}


int gtiFilterHkData(vector <hkData> &hkDataVec,vector<gtiData> gtiDataVec)
{
    long nrowsHk,nrowsGti,i,j;

    nrowsHk=hkDataVec.size();
    nrowsGti= gtiDataVec.size();

    vector <hkData> filtHkDataVec;
    filtHkDataVec.clear();

    if(nrowsGti==0)
    {
        printf("No valid rows in GTI. Quitting selection by GTI\n");
        return EXIT_FAILURE;
    }

    if(nrowsHk==0)
    {
        printf("No valid rows in HK Data. Quitting selection by GTI\n");
        return EXIT_FAILURE;
    }

    j=0;

    for(i=0;i<nrowsHk;i++)
    {
        while(hkDataVec[i].time > gtiDataVec[j].tstop && j<nrowsGti-1) j++;

        if(j==nrowsGti) break;

        if(hkDataVec[i].time > gtiDataVec[j].tstart && hkDataVec[i].time < gtiDataVec[j].tstop)
        {
            filtHkDataVec.push_back(hkDataVec[i]);
        }
    }

    if(filtHkDataVec.size()==0)
    {
        printf("No HK data available after filtering. No output to be generated\n");
        return EXIT_FAILURE;
    }

    hkDataVec.clear();
    hkDataVec=filtHkDataVec;

    return EXIT_SUCCESS;

}

int gtiFiltersunAngData(vector <sunAngData> &sunAngDataVec,vector<gtiData> gtiDataVec)
{

    long nrowsSa,nrowsGti,i,j;

    nrowsSa=sunAngDataVec.size();
    nrowsGti= gtiDataVec.size();

    vector <sunAngData> filtSunAngDataVec;
    filtSunAngDataVec.clear();

    if(nrowsGti==0)
    {
        printf("No valid rows in GTI. Quitting selection by GTI\n");
        return EXIT_FAILURE;
    }

    if(nrowsSa==0)
    {
        printf("No valid rows in SunAng Data. Quitting selection by GTI\n");
        return EXIT_FAILURE;
    }

    j=0;

    for(i=0;i<nrowsSa;i++)
    {
        while(sunAngDataVec[i].time > gtiDataVec[j].tstop && j<nrowsGti-1) j++;

        if(j==nrowsGti) break;

        if(sunAngDataVec[i].time > gtiDataVec[j].tstart && sunAngDataVec[i].time < gtiDataVec[j].tstop)
        {
            filtSunAngDataVec.push_back(sunAngDataVec[i]);
        }
    }

    if(filtSunAngDataVec.size()==0)
    {
        printf("No SunAng data available after filtering. No output to be generated\n");
        return EXIT_FAILURE;
    }

    sunAngDataVec.clear();
    sunAngDataVec=filtSunAngDataVec;

    return EXIT_SUCCESS;
    
}


int checkL0TimeSanity(vector <scienceData> &scDataVec)
{
    long nrows,i;
    int xsmtdiff1,xsmtdiff2;
    int tdiff1,tdiff2;
    nrows=scDataVec.size();
    for(i=0;i<nrows-2;i++)
    {
        tdiff1=round(scDataVec[i+1].time-scDataVec[i].time);
        tdiff2=round(scDataVec[i+2].time-scDataVec[i+1].time);
        xsmtdiff1=round(scDataVec[i+1].xsmTime-scDataVec[i].xsmTime);
        xsmtdiff2=round(scDataVec[i+2].xsmTime-scDataVec[i+1].xsmTime);

        if(tdiff1!=xsmtdiff1&&xsmtdiff1>0&&xsmtdiff2>0&&scDataVec[i].xsmTime!=0)
        {
            if(tdiff2<xsmtdiff2)
            {
                /*printf("********LT*******\n");
                printf("%lf\t%lf\n",scDataVec[i].time,scDataVec[i].xsmTime);
                printf("%lf\t%lf\t%d\t%d\n",scDataVec[i+1].time,scDataVec[i+1].xsmTime,tdiff1,xsmtdiff1);
                printf("%lf\t%lf\t%d\t%d\n",scDataVec[i+2].time,scDataVec[i+2].xsmTime,tdiff2,xsmtdiff2);
				*/
                scDataVec[i+1].time=scDataVec[i+2].time-xsmtdiff2;
				
            }
            else if(tdiff2>xsmtdiff2)
            {
                
				/*printf("********GT*******\n");
				printf("%lf\t%lf\n",scDataVec[i].time,scDataVec[i].xsmTime);
                printf("%lf\t%lf\t%d\t%d\n",scDataVec[i+1].time,scDataVec[i+1].xsmTime,tdiff1,xsmtdiff1);
                printf("%lf\t%lf\t%d\t%d\n",scDataVec[i+2].time,scDataVec[i+2].xsmTime,tdiff2,xsmtdiff2);
				*/
                scDataVec[i+1].time=scDataVec[i+2].time-xsmtdiff2;

            }
            else if(tdiff2==xsmtdiff2&&tdiff1==0)
            {
				/*printf("********EQ*******\n");
                printf("%lf\t%lf\n",scDataVec[i-1].time,scDataVec[i-1].xsmTime);				
                printf("%lf\t%lf\n",scDataVec[i].time,scDataVec[i].xsmTime);
                printf("%lf\t%lf\t%d\t%d\n",scDataVec[i+1].time,scDataVec[i+1].xsmTime,tdiff1,xsmtdiff1);
                printf("%lf\t%lf\t%d\t%d\n",scDataVec[i+2].time,scDataVec[i+2].xsmTime,tdiff2,xsmtdiff2);
				*/
                scDataVec[i].time=scDataVec[i+1].time-xsmtdiff1;
            }
        }
    }

    return EXIT_SUCCESS;
}


