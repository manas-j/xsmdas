/* XSMFILEHANDLER.CPP
 *
 * Mithun NPS 
 * */

#include <xsmfilehandler.h>

//scienceFileHandler class

int scienceFileHandler::create_l1science_file(string scienceFileName)
{
    int status=0;

	status=create_empty_fitsfile(scienceFileName,SCIENCETEMPLATE);

    return EXIT_SUCCESS;
}

int scienceFileHandler::write_l1science_file(string scienceFile,vector <scienceData> scDataVec,vector <string> runHistory)
{
	int status=0;
	int time_col,frameNumber_col,dataArray_col,decodingStatusFlag_col;
	int XSMTime_col,BDHTime_col,UTCString_col;
	long nrows=0,i;	
	int hdutype;

    fitsfile *fout;
	
    fits_open_file(&fout, scienceFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }
	

    fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
    fits_get_colnum(fout,CASEINSEN,"UTCString",&UTCString_col,&status);
	fits_get_colnum(fout,CASEINSEN,"FRAMENUMBER",&frameNumber_col,&status);
    fits_get_colnum(fout,CASEINSEN,"BDHTime",&BDHTime_col,&status);
    fits_get_colnum(fout,CASEINSEN,"XSMTime",&XSMTime_col,&status);
    fits_get_colnum(fout,CASEINSEN,"DataArray",&dataArray_col,&status);
    fits_get_colnum(fout,CASEINSEN,"DecodingStatusFlag",&decodingStatusFlag_col,&status);

    if (status) {
        cout <<"***Error in getting column numbers for file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

	nrows=scDataVec.size();	

	for (i=0;i<nrows;i++)
	{
		fits_write_col(fout, TDOUBLE, time_col, i+1,1, 1, &scDataVec[i].time, &status);
        //fits_write_col(fout, TSTRING, UTCString_col, i+1,1, 1, scDataVec[i].timestr, &status);
		char *UTCstr;
		UTCstr=(char*)malloc(sizeof(char *)*30);
		strcpy(UTCstr,scDataVec[i].timestr);
		fits_write_col_str(fout, UTCString_col,  i+1,1, 1,&UTCstr, &status);
   		fits_write_col(fout, TLONG, frameNumber_col, i+1,1, 1, &scDataVec[i].frameNumber, &status);
        fits_write_col(fout, TDOUBLE, BDHTime_col, i+1,1, 1, &scDataVec[i].bdhTime, &status);
        fits_write_col(fout, TDOUBLE, XSMTime_col, i+1,1, 1, &scDataVec[i].xsmTime, &status);
        fits_write_col(fout, TBYTE, dataArray_col, i+1,1,FRAME_SIZE, scDataVec[i].dataArray, &status);
        fits_write_col(fout, TINT, decodingStatusFlag_col, i+1,1, 1, &scDataVec[i].decodingStatusFlag, &status);
	    free(UTCstr);
        if(status)
        {
            cout<<"***Error in writing to file "<<scienceFile<<"\n";
            return (status);
        }

    }

    // Write keywords in header

    /*
    double tstarti,tstartf,tstopi,tstopf;
    tstarti=floor(scDataVec[0].time);
    tstartf=scDataVec[0].time-floor(scDataVec[0].time);
    tstopi=floor(scDataVec[nrows-1].time);
    tstopf=scDataVec[nrows-1].time-floor(scDataVec[nrows-1].time);
    */
    double telapse;
    telapse=scDataVec[nrows-1].time-scDataVec[0].time;
    
    fits_update_key(fout, TSTRING, "DATE-OBS",scDataVec[0].timestr, NULL, &status);
    fits_update_key(fout, TSTRING, "DATE-END",scDataVec[nrows-1].timestr, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTART",&scDataVec[0].time, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTOP",&scDataVec[nrows-1].time, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TELAPSE",&telapse, NULL, &status);
    /*
    fits_update_key(fout, TDOUBLE, "TSTARTI",&tstarti, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTARTF",&tstartf, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTOPI",&tstopi, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTOPF",&tstopf, NULL, &status);
    */

    if (status) {
        cout <<"***Error in writing to header of file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }
    
    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }


	return EXIT_SUCCESS;
}

int scienceFileHandler::read_l1science_file(string scienceFile,vector <scienceData> &scDataVec)
{

    int status=0;
    int time_col,frameNumber_col,dataArray_col,decodingStatusFlag_col;
    int XSMTime_col,BDHTime_col,UTCString_col;

    long nrows=0,i;
    int hdutype;

    fitsfile *fout;

    fits_open_file(&fout, scienceFile.c_str(), READONLY, &status);
    if (status) {
        cout <<"***Error in opening file "<<scienceFile<<"\n";
        cout <<"***Error: Check the file format\n";        
		fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in moving to data HDU in file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

    
    float input_version;
    fits_read_key(fout, TFLOAT, "XSMDASVE", &input_version, NULL, &status);
    if(status) input_version=1.0;
       
    if(status || input_version < (MINIMUM_VERSION-0.005))
    {
        cout<<"***Error: Input data generated with incompatible version of XSMDAS\n";
        cout<<"***Data Version : "<<input_version<<"\n";
        cout<<"***Minimum req. version: "<<MINIMUM_VERSION<<"\n";
        cout<<"***Exiting without proceeding further\n";
        return (EXIT_FAILURE);
    }
        
    fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
    fits_get_colnum(fout,CASEINSEN,"UTCString",&UTCString_col,&status);	
	fits_get_colnum(fout,CASEINSEN,"FRAMENUMBER",&frameNumber_col,&status);
    fits_get_colnum(fout,CASEINSEN,"DataArray",&dataArray_col,&status);
    fits_get_colnum(fout,CASEINSEN,"DecodingStatusFlag",&decodingStatusFlag_col,&status);
    fits_get_colnum(fout,CASEINSEN,"BDHTime",&BDHTime_col,&status);
    fits_get_colnum(fout,CASEINSEN,"XSMTime",&XSMTime_col,&status);


	fits_get_num_rows(fout, &nrows, &status);

    if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in getting Expected column numbers for file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

	if(nrows==0){
		cout<<"***Error: No rows in input Level-1 sciencefile "<<scienceFile<<"\n";
		return EXIT_FAILURE;
	}
    
    scDataVec.resize(nrows);

    for (i=0;i<nrows;i++)
    {
        fits_read_col(fout, TDOUBLE, time_col, i+1,1, 1,NULL,&scDataVec[i].time,NULL,&status);
        char *UTCstr;
	    UTCstr=(char*)malloc(sizeof(char *)*30);
		fits_read_col_str(fout, UTCString_col, i+1,1, 1,NULL,&UTCstr,NULL,&status);
		strcpy(scDataVec[i].timestr,UTCstr);
		fits_read_col(fout, TDOUBLE, BDHTime_col, i+1,1, 1,NULL,&scDataVec[i].bdhTime,NULL,&status);
        fits_read_col(fout, TDOUBLE, XSMTime_col, i+1,1, 1,NULL,&scDataVec[i].xsmTime,NULL,&status);		
		fits_read_col(fout, TLONG, frameNumber_col, i+1,1, 1,NULL, &scDataVec[i].frameNumber,NULL, &status);
        fits_read_col(fout, TBYTE, dataArray_col, i+1,1,FRAME_SIZE,NULL, scDataVec[i].dataArray,NULL, &status);
        fits_read_col(fout, TINT, decodingStatusFlag_col, i+1,1, 1,NULL, &scDataVec[i].decodingStatusFlag,NULL, &status);
        free(UTCstr);
    }

    // Read Header keywords
    
    char date_obs1[80],date_end1[80];
    fits_read_key(fout, TSTRING, "DATE-OBS", date_obs1, NULL, &status);
    fits_read_key(fout, TSTRING, "DATE-END", date_end1, NULL, &status);
    this->date_obs=(string)date_obs1;
    this->date_end=(string)date_end1;
    fits_read_key(fout,TDOUBLE,"TSTART",&this->tstart,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TSTOP",&this->tstop,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TELAPSE",&this->telapse,NULL, &status);

    if (status) {
        cout <<"***Error in reading Header keywords from file "<<scienceFile<<"\n";
		cout <<"***Error: Mandatory keywords missing\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }
    

    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<scienceFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }


    return EXIT_SUCCESS;
}
/*********************************************************************************************************************************************/
//hkFileHandler class

int hkFileHandler::create_hkfile(string hkFileName)
{
    int status=0;

    status=create_empty_fitsfile(hkFileName,HKTEMPLATE);

    return EXIT_SUCCESS;
}

int hkFileHandler::write_hkfile(string hkFile,vector <framehdr> hdrData,vector <string> runHistory)
{

    int status=0;
    long nrows=0,i;
    int hdutype;
	int time_col,fpgaTime_col,frameNo_col,syncLW_col,syncUW_col;
	int eventCounter_col,eventDetected_col,rampCounter_col;
	int hvMonitor_col,detTemperature_col,tecCurent_col,lv1Monitor_col,lv2Monitor_col;
	int lldRefVoltage_col, tecRefVoltage_col;

	int UTCString_col;

	int MotorControlMode_col,MotorOperationMode_col;
	int MotorSetPos_col,MotorIRPos_col;
	int IRPowerStatus_col,FrameDiscardFlag_col,MotorAutoTime_col;
	int StepModeDir_col,WindowLowerThresh_col,WindowUpperThresh_col;
	int PileupRejMode_col,PileupRejTime_col,GuardBits_col;
	int Ch1Start_col,Ch1Stop_col,Ch2Start_col,Ch3Start_col,Ch2Stop_col,Ch3Stop_col;
	int CoarseChEvents_col,SpecEvents_col,ULDEvents_col;

    fitsfile *fout;

    fits_open_file(&fout, hkFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

    fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
    fits_get_colnum(fout,CASEINSEN,"UTCString",&UTCString_col,&status);	
    fits_get_colnum(fout,CASEINSEN,"FPGATIME",&fpgaTime_col,&status);
    fits_get_colnum(fout,CASEINSEN,"FRAMENo",&frameNo_col,&status);
    fits_get_colnum(fout,CASEINSEN,"SYNCLW",&syncLW_col,&status);
    fits_get_colnum(fout,CASEINSEN,"SYNCUW",&syncUW_col,&status);
    fits_get_colnum(fout,CASEINSEN,"EventCounter",&eventCounter_col,&status);
    fits_get_colnum(fout,CASEINSEN,"EventDetected",&eventDetected_col,&status);
    fits_get_colnum(fout,CASEINSEN,"RampCounter",&rampCounter_col,&status);
    fits_get_colnum(fout,CASEINSEN,"HVMonitor",&hvMonitor_col,&status);
    fits_get_colnum(fout,CASEINSEN,"DetTemperature",&detTemperature_col,&status);
    fits_get_colnum(fout,CASEINSEN,"TECCurrent",&tecCurent_col,&status);
    fits_get_colnum(fout,CASEINSEN,"LV1Monitor",&lv1Monitor_col,&status);
    fits_get_colnum(fout,CASEINSEN,"LV2Monitor",&lv2Monitor_col,&status);
    fits_get_colnum(fout,CASEINSEN,"LLDRefVoltage",&lldRefVoltage_col,&status);
    fits_get_colnum(fout,CASEINSEN,"TECRefVoltage",&tecRefVoltage_col,&status);
	fits_get_colnum(fout,CASEINSEN,"MotorControlMode",&MotorControlMode_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorOperationMode",&MotorOperationMode_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorSetPos",&MotorSetPos_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorIRPos",&MotorIRPos_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"IRPowerStatus",&IRPowerStatus_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"FrameDiscardFlag",&FrameDiscardFlag_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorAutoTime",&MotorAutoTime_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"StepModeDir",&StepModeDir_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"WindowLowerThresh",&WindowLowerThresh_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"WindowUpperThresh",&WindowUpperThresh_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"PileupRejMode",&PileupRejMode_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"PileupRejTime",&PileupRejTime_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"GuardBits",&GuardBits_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch1Start",&Ch1Start_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch1Stop",&Ch1Stop_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch2Start",&Ch2Start_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch2Stop",&Ch2Stop_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch3Start",&Ch3Start_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch3Stop",&Ch3Stop_col ,&status);
	fits_get_colnum(fout,CASEINSEN,"CoarseChEvents",&CoarseChEvents_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"SpecEvents",&SpecEvents_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"ULDEvents",&ULDEvents_col ,&status);

    if (status) {
        cout <<"***Error in getting expected column numbers for file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

  	nrows=hdrData.size();

    char *UTCstr;

    for (i=0;i<nrows;i++)
    {
		
		double fpgaTime=(double)hdrData[i].fpga_time*1.0e-6;

        fits_write_col(fout, TDOUBLE, time_col, i+1,1, 1, &hdrData[i].time, &status);
        
        UTCstr=(char*)malloc(sizeof(char *)*30);
        strcpy(UTCstr,hdrData[i].timestr);
        fits_write_col_str(fout, UTCString_col,  i+1,1, 1,&UTCstr, &status);
	    free(UTCstr);
            
		fits_write_col(fout, TDOUBLE, fpgaTime_col, i+1,1, 1, &fpgaTime, &status);
        fits_write_col(fout, TLONG, frameNo_col, i+1,1, 1, &hdrData[i].frame_no, &status);


        fits_write_col(fout, TLONG,syncLW_col, i+1,1, 1, &hdrData[i].sync_LW, &status);
        fits_write_col(fout, TLONG,syncUW_col, i+1,1, 1, &hdrData[i].sync_UW, &status);

        //fits_write_col(fout, TLONG, fastCounter_col, i+1,1, 1, &hdrData[i].fast_counter, &status);
        
		fits_write_col(fout, TLONG, eventCounter_col, i+1,1, 1, &hdrData[i].event_counter, &status);
        fits_write_col(fout, TLONG, eventDetected_col, i+1,1, 1, &hdrData[i].event_detected, &status);
        
		fits_write_col(fout, TINT, rampCounter_col, i+1,1, 1, &hdrData[i].ramp_counter, &status);

		float hvMonitor,detTemperature,tecCurent,lv1Monitor,lv2Monitor,lldRefVoltage,tecRefVoltage;

		hvMonitor=hdrData[i].hv_monitor*MONITOR_ADC_GAIN;
		detTemperature=hdrData[i].temperature*MONITOR_ADC_GAIN;
		tecCurent=hdrData[i].tec_current*MONITOR_ADC_GAIN;
		lv1Monitor=hdrData[i].lv1_monitor*MONITOR_ADC_GAIN;
        lv2Monitor=hdrData[i].lv2_monitor*MONITOR_ADC_GAIN;
		
		lldRefVoltage=hdrData[i].lld_voltage*REFERENCE_DAC_GAIN;
		tecRefVoltage=hdrData[i].tecref_voltage*REFERENCE_DAC_GAIN;	

        fits_write_col(fout, TFLOAT, hvMonitor_col, i+1,1, 1, &hvMonitor, &status);
        fits_write_col(fout, TFLOAT, detTemperature_col, i+1,1, 1, &detTemperature, &status);
        fits_write_col(fout, TFLOAT, tecCurent_col, i+1,1, 1, &tecCurent, &status);
        fits_write_col(fout, TFLOAT, lv1Monitor_col, i+1,1, 1, &lv1Monitor, &status);
        fits_write_col(fout, TFLOAT, lv2Monitor_col, i+1,1, 1, &lv2Monitor, &status);
        fits_write_col(fout, TFLOAT, lldRefVoltage_col, i+1,1, 1, &lldRefVoltage, &status);
        fits_write_col(fout, TFLOAT, tecRefVoltage_col, i+1,1, 1, &tecRefVoltage, &status);

		fits_write_col(fout, TBYTE, MotorControlMode_col,i+1,1,1,&hdrData[i].motor_control_mode , &status);
		fits_write_col(fout, TBYTE, MotorOperationMode_col,i+1,1,1,&hdrData[i].motor_operation_mode , &status);
        fits_write_col(fout, TBYTE, MotorSetPos_col,i+1,1,1,&hdrData[i].motor_manual_set_position , &status);
        fits_write_col(fout, TBYTE, MotorIRPos_col,i+1,1,1,&hdrData[i].motor_ir_position , &status);
        fits_write_col(fout, TBYTE, IRPowerStatus_col,i+1,1,1,&hdrData[i].ir_power_status , &status);
        fits_write_col(fout, TBYTE, FrameDiscardFlag_col,i+1,1,1,&hdrData[i].frame_discard , &status);
        fits_write_col(fout, TBYTE, MotorAutoTime_col,i+1,1,1,&hdrData[i].motor_automovement_time , &status);
        fits_write_col(fout, TBYTE, StepModeDir_col,i+1,1,1,&hdrData[i].motor_stepmode_direction , &status);
        fits_write_col(fout, TLONG, WindowLowerThresh_col,i+1,1,1,&hdrData[i].window_lower_thresh , &status);
        fits_write_col(fout, TLONG, WindowUpperThresh_col,i+1,1,1,&hdrData[i].window_upper_thresh , &status);
        fits_write_col(fout, TBYTE, PileupRejMode_col,i+1,1,1,&hdrData[i].pileup_rejection_mode , &status);
        fits_write_col(fout, TBYTE, PileupRejTime_col,i+1,1,1,&hdrData[i].pileup_time , &status);
        fits_write_col(fout, TINT, GuardBits_col,i+1,1,1,&hdrData[i].guardbits , &status);
        fits_write_col(fout, TINT, Ch1Start_col,i+1,1,1,&hdrData[i].ch1_start , &status);
        fits_write_col(fout, TINT, Ch1Stop_col,i+1,1,1,&hdrData[i].ch1_stop , &status);
        fits_write_col(fout, TINT, Ch2Start_col,i+1,1,1,&hdrData[i].ch2_start , &status);
        fits_write_col(fout, TINT, Ch2Stop_col,i+1,1,1,&hdrData[i].ch2_stop , &status);
        fits_write_col(fout, TINT, Ch3Start_col,i+1,1,1,&hdrData[i].ch3_start , &status);
        fits_write_col(fout, TINT, Ch3Stop_col,i+1,1,1,&hdrData[i].ch3_stop , &status);
		fits_write_col(fout, TLONG, CoarseChEvents_col,i+1,1,1,&hdrData[i].coarse_channel_events , &status);
        fits_write_col(fout, TLONG, SpecEvents_col,i+1,1,1,&hdrData[i].spectrum_events , &status);
        fits_write_col(fout, TLONG, ULDEvents_col,i+1,1,1,&hdrData[i].uld_events , &status);

	    if (status) {
    	    cout <<"***Error in writing to the file: "<<hkFile<<"\n";
        	fits_report_error(stderr, status);
        	return (EXIT_FAILURE);
    	}

    }

    // Write keywords to header

    fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
    fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTART",&this->tstart, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTOP",&this->tstop, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TELAPSE",&this->telapse, NULL, &status);

    if (status) {
        cout <<"***Error in writing Header keywords to file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }
	
    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

	return EXIT_SUCCESS;
}

int hkFileHandler::read_hkfile(string hkFile,vector <hkData> &hkDataVec)
{
    int status=0;
    long nrows=0,i;
    int hdutype;
    int time_col,fpgaTime_col,frameNo_col,syncLW_col,syncUW_col;
    int eventCounter_col,eventDetected_col,rampCounter_col;
    int hvMonitor_col,detTemperature_col,tecCurent_col,lv1Monitor_col,lv2Monitor_col;
    int lldRefVoltage_col, tecRefVoltage_col;
    
	int UTCString_col;

    int MotorControlMode_col,MotorOperationMode_col;
    int MotorSetPos_col,MotorIRPos_col;
    int IRPowerStatus_col,FrameDiscardFlag_col,MotorAutoTime_col;
    int StepModeDir_col,WindowLowerThresh_col,WindowUpperThresh_col;
    int PileupRejMode_col,PileupRejTime_col,GuardBits_col;
    int Ch1Start_col,Ch1Stop_col,Ch2Start_col,Ch3Start_col,Ch2Stop_col,Ch3Stop_col;
    int CoarseChEvents_col,SpecEvents_col,ULDEvents_col;


    fitsfile *fout;

    fits_open_file(&fout, hkFile.c_str(), READONLY, &status);
    if (status) {
        cout <<"***Error in opening file "<<hkFile<<"\n";
        cout <<"***Error: Check the file format\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in moving to data HDU in file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

    float input_version;
    fits_read_key(fout, TFLOAT, "XSMDASVE", &input_version, NULL, &status);
    if(status) input_version=1.0;

    if(status || input_version < (MINIMUM_VERSION-0.005))
    {
        cout<<"***Error: Input data generated with incompatible version of XSMDAS\n";
        cout<<"***Data Version : "<<input_version<<"\n";
        cout<<"***Minimum req. version: "<<MINIMUM_VERSION<<"\n";
        cout<<"***Exiting without proceeding further\n";
        return (EXIT_FAILURE);
    }

    fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
    fits_get_colnum(fout,CASEINSEN,"UTCString",&UTCString_col,&status);	
	fits_get_colnum(fout,CASEINSEN,"FPGATIME",&fpgaTime_col,&status);
    fits_get_colnum(fout,CASEINSEN,"FRAMENo",&frameNo_col,&status);
    fits_get_colnum(fout,CASEINSEN,"SYNCLW",&syncLW_col,&status);
    fits_get_colnum(fout,CASEINSEN,"SYNCUW",&syncUW_col,&status);
//    fits_get_colnum(fout,CASEINSEN,"FastCounter",&fastCounter_col,&status);
    fits_get_colnum(fout,CASEINSEN,"EventCounter",&eventCounter_col,&status);
    fits_get_colnum(fout,CASEINSEN,"EventDetected",&eventDetected_col,&status);
    fits_get_colnum(fout,CASEINSEN,"RampCounter",&rampCounter_col,&status);
    fits_get_colnum(fout,CASEINSEN,"HVMonitor",&hvMonitor_col,&status);
    fits_get_colnum(fout,CASEINSEN,"DetTemperature",&detTemperature_col,&status);
    fits_get_colnum(fout,CASEINSEN,"TECCurrent",&tecCurent_col,&status);
    fits_get_colnum(fout,CASEINSEN,"LV1Monitor",&lv1Monitor_col,&status);
    fits_get_colnum(fout,CASEINSEN,"LV2Monitor",&lv2Monitor_col,&status);
    fits_get_colnum(fout,CASEINSEN,"LLDRefVoltage",&lldRefVoltage_col,&status);
    fits_get_colnum(fout,CASEINSEN,"TECRefVoltage",&tecRefVoltage_col,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorControlMode",&MotorControlMode_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorOperationMode",&MotorOperationMode_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorSetPos",&MotorSetPos_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorIRPos",&MotorIRPos_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"IRPowerStatus",&IRPowerStatus_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"FrameDiscardFlag",&FrameDiscardFlag_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"MotorAutoTime",&MotorAutoTime_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"StepModeDir",&StepModeDir_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"WindowLowerThresh",&WindowLowerThresh_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"WindowUpperThresh",&WindowUpperThresh_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"PileupRejMode",&PileupRejMode_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"PileupRejTime",&PileupRejTime_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"GuardBits",&GuardBits_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch1Start",&Ch1Start_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch1Stop",&Ch1Stop_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch2Start",&Ch2Start_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch2Stop",&Ch2Stop_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch3Start",&Ch3Start_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"Ch3Stop",&Ch3Stop_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"CoarseChEvents",&CoarseChEvents_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"SpecEvents",&SpecEvents_col ,&status);
    fits_get_colnum(fout,CASEINSEN,"ULDEvents",&ULDEvents_col ,&status);

    fits_get_num_rows(fout, &nrows, &status);

    if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in getting Expected column numbers for file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    if(nrows==0){
        cout<<"***Error: No rows in input HK file "<<hkFile<<"\n";
        return EXIT_FAILURE;
    }

	hkData hkDataRow;
	hkDataVec.clear();

    for (i=0;i<nrows;i++)
    {

        fits_read_col(fout, TDOUBLE, time_col, i+1,1,1,NULL, &hkDataRow.time, NULL,&status);
        fits_read_col(fout, TDOUBLE, fpgaTime_col, i+1,1,1,NULL, &hkDataRow.fpgaTime, NULL,&status);
        fits_read_col(fout, TLONG, frameNo_col, i+1,1,1,NULL, &hkDataRow.frameNo, NULL,&status);

        fits_read_col(fout, TLONG,syncLW_col, i+1,1,1,NULL, &hkDataRow.syncLW, NULL,&status);
        fits_read_col(fout, TLONG,syncUW_col, i+1,1,1,NULL, &hkDataRow.syncUW, NULL,&status);

//        fits_read_col(fout, TLONG, fastCounter_col, i+1,1,1,NULL, &hkDataRow.fastCounter, NULL,&status);
        fits_read_col(fout, TLONG, eventCounter_col, i+1,1,1,NULL, &hkDataRow.eventCounter, NULL,&status);
        fits_read_col(fout, TLONG, eventDetected_col, i+1,1,1,NULL, &hkDataRow.eventDetected, NULL,&status);
        fits_read_col(fout, TINT, rampCounter_col, i+1,1,1,NULL, &hkDataRow.rampCounter, NULL,&status);

        fits_read_col(fout, TFLOAT, hvMonitor_col, i+1,1,1,NULL, &hkDataRow.hvMonitor, NULL,&status);
        fits_read_col(fout, TFLOAT, detTemperature_col, i+1,1,1,NULL, &hkDataRow.detTemperature, NULL,&status);
        fits_read_col(fout, TFLOAT, tecCurent_col, i+1,1,1,NULL, &hkDataRow.tecCurent, NULL,&status);
        fits_read_col(fout, TFLOAT, lv1Monitor_col, i+1,1,1,NULL, &hkDataRow.lv1Monitor, NULL,&status);
        fits_read_col(fout, TFLOAT, lv2Monitor_col, i+1,1,1,NULL, &hkDataRow.lv2Monitor, NULL,&status);
        fits_read_col(fout, TFLOAT, lldRefVoltage_col, i+1,1,1,NULL, &hkDataRow.lldRefVoltage, NULL,&status);
        fits_read_col(fout, TFLOAT, tecRefVoltage_col, i+1,1,1,NULL, &hkDataRow.tecRefVoltage, NULL,&status);

        fits_read_col(fout, TBYTE, MotorControlMode_col,i+1,1,1,NULL,&hkDataRow.motor_control_mode , NULL,&status);
        fits_read_col(fout, TBYTE, MotorOperationMode_col,i+1,1,1,NULL,&hkDataRow.motor_operation_mode , NULL,&status);
        fits_read_col(fout, TBYTE, MotorSetPos_col,i+1,1,1,NULL,&hkDataRow.motor_manual_set_position , NULL,&status);
        fits_read_col(fout, TBYTE, MotorIRPos_col,i+1,1,1,NULL,&hkDataRow.motor_ir_position , NULL,&status);
        fits_read_col(fout, TBYTE, IRPowerStatus_col,i+1,1,1,NULL,&hkDataRow.ir_power_status , NULL,&status);
        fits_read_col(fout, TBYTE, FrameDiscardFlag_col,i+1,1,1,NULL,&hkDataRow.frame_discard , NULL,&status);
        fits_read_col(fout, TBYTE, MotorAutoTime_col,i+1,1,1,NULL,&hkDataRow.motor_automovement_time , NULL,&status);
        fits_read_col(fout, TBYTE, StepModeDir_col,i+1,1,1,NULL,&hkDataRow.motor_stepmode_direction , NULL,&status);
        fits_read_col(fout, TLONG, WindowLowerThresh_col,i+1,1,1,NULL,&hkDataRow.window_lower_thresh , NULL,&status);
        fits_read_col(fout, TLONG, WindowUpperThresh_col,i+1,1,1,NULL,&hkDataRow.window_upper_thresh , NULL,&status);
        fits_read_col(fout, TBYTE, PileupRejMode_col,i+1,1,1,NULL,&hkDataRow.pileup_rejection_mode , NULL,&status);
        fits_read_col(fout, TBYTE, PileupRejTime_col,i+1,1,1,NULL,&hkDataRow.pileup_time , NULL,&status);
        fits_read_col(fout, TINT, GuardBits_col,i+1,1,1,NULL,&hkDataRow.guardbits , NULL,&status);
        fits_read_col(fout, TINT, Ch1Start_col,i+1,1,1,NULL,&hkDataRow.ch1_start , NULL,&status);
        fits_read_col(fout, TINT, Ch1Stop_col,i+1,1,1,NULL,&hkDataRow.ch1_stop , NULL,&status);
        fits_read_col(fout, TINT, Ch2Start_col,i+1,1,1,NULL,&hkDataRow.ch2_start , NULL,&status);
        fits_read_col(fout, TINT, Ch2Stop_col,i+1,1,1,NULL,&hkDataRow.ch2_stop , NULL,&status);
        fits_read_col(fout, TINT, Ch3Start_col,i+1,1,1,NULL,&hkDataRow.ch3_start , NULL,&status);
        fits_read_col(fout, TINT, Ch3Stop_col,i+1,1,1,NULL,&hkDataRow.ch3_stop , NULL,&status);
        fits_read_col(fout, TLONG, CoarseChEvents_col,i+1,1,1,NULL,&hkDataRow.coarse_channel_events , NULL,&status);
        fits_read_col(fout, TLONG, SpecEvents_col,i+1,1,1,NULL,&hkDataRow.spectrum_events , NULL,&status);
        fits_read_col(fout, TLONG, ULDEvents_col,i+1,1,1,NULL,&hkDataRow.uld_events , NULL,&status);		


        if (status) {
            cout <<"***Error in reading from the file: "<<hkFile<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }

        hkDataVec.push_back(hkDataRow);
		
	}

    // Read Header keywords

    char date_obs1[80],date_end1[80];
    fits_read_key(fout, TSTRING, "DATE-OBS", date_obs1, NULL, &status);
    fits_read_key(fout, TSTRING, "DATE-END", date_end1, NULL, &status);
    this->date_obs=(string)date_obs1;
    this->date_end=(string)date_end1;
    fits_read_key(fout,TDOUBLE,"TSTART",&this->tstart,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TSTOP",&this->tstop,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TELAPSE",&this->telapse,NULL, &status);

    if (status) {
        cout <<"***Error in reading Header keywords from file "<<hkFile<<"\n";
		cout <<"***Error: Mandatory Keywords missing from file\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }


    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<hkFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

	return EXIT_SUCCESS;

}


/*********************************************************************************************************************************************/

//specFileHandler

int specFileHandler::create_specfile(string specFileName)
{
    int status=0,hdutype;

	if(spectype==1)	
    {
        if(nchan==1024)
           status=create_empty_fitsfile(specFileName,SPEC1TEMPLATE);
        else if (nchan==512)
           status=create_empty_fitsfile(specFileName,SPEC1TEMPLATE512); 
        else
        {
        cout<<"Template file not available for this number of channels "<<nchan<<"\n";
        return EXIT_FAILURE;        
        }
    }
	else if(spectype==2) 
	{
        if(nchan==1024)
    		status=create_empty_fitsfile(specFileName,SPEC2TEMPLATE);
        else if (nchan==512)
            status=create_empty_fitsfile(specFileName,SPEC2TEMPLATE512);
        else
        {
        cout<<"Template file not available for this number of channels "<<nchan<<"\n";
        return EXIT_FAILURE;
        }
	}
	else
	{
		cout<<"Unknown spectrum type";
		return EXIT_FAILURE;
	}
	
   	return (status);

}

int specFileHandler::read_type1_specfile(string specFile,specData &spec1Data)
{
	this->spectype=1;

    int status=0;
    int nrows=0;
    long i;
    int hdutype;
    int channel_col,counts_col,stat_err_col,sys_err_col;
    fitsfile *fout;

    fits_open_file(&fout, specFile.c_str(), READONLY, &status);
    if (status) {
        cout <<"***Error in opening file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"CHANNEL",&channel_col,&status);
   fits_get_colnum(fout,CASEINSEN,"COUNTS",&counts_col,&status);
   fits_get_colnum(fout,CASEINSEN,"STAT_ERR",&stat_err_col,&status);
   fits_get_colnum(fout,CASEINSEN,"SYS_ERR",&sys_err_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   fits_read_key(fout,TINT,"NAXIS2",&nrows,NULL, &status);
   this->nchan=nrows;
   spec1Data.nchan=nrows;
	
   char chantype_str[10];
   fits_read_key(fout, TSTRING, "CHANTYPE",chantype_str, NULL, &status);

   if (strcasecmp(chantype_str, "PHA") == 0) {
       spec1Data.chantype = 1;
	   this->chantype = 1;	
   } else if (strcasecmp(chantype_str, "PI") == 0) {
       spec1Data.chantype = 2;
       this->chantype = 2;
   }

   // Initialize the spectrum data file
   spec1Data.initialize(chantype,nchan);
    

   char arffile_char[1000],respfile_char[1000],date_obs_char[1000],date_end_char[1000];	
	
   fits_read_key(fout, TSTRING, "ANCRFILE",arffile_char, NULL, &status);
   fits_read_key(fout, TSTRING, "RESPFILE",respfile_char, NULL, &status);
   
   this->arffile=string(arffile_char);
   this->respfile=string(respfile_char);		
 
   fits_read_key(fout, TSTRING, "DATE-OBS",date_obs_char, NULL, &status);
   fits_read_key(fout, TSTRING, "DATE-END",date_end_char, NULL, &status);

   this->date_obs=string(date_obs_char);
   this->date_end=string(date_end_char);

   fits_read_key(fout, TDOUBLE, "TSTART",&spec1Data.tstart, NULL, &status);
   fits_read_key(fout, TDOUBLE, "TSTOP",&spec1Data.tstop, NULL, &status);
   fits_read_key(fout, TDOUBLE, "EXPOSURE",&spec1Data.exposure, NULL, &status);

 
   if (status) {
       cout <<"***Error in getting header keys for file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   long channel[nchan];
   float spec[nchan],stat_err[nchan],sys_err[nchan];
   	
   fits_read_col(fout,TLONG,channel_col,1,1,nchan,NULL,channel,NULL,&status);   	
   fits_read_col(fout,TFLOAT,counts_col,1,1,nchan,NULL,spec,NULL,&status);
   fits_read_col(fout,TFLOAT,stat_err_col,1,1,nchan,NULL,stat_err,NULL,&status);
   fits_read_col(fout,TFLOAT,sys_err_col,1,1,nchan,NULL,sys_err,NULL,&status);

   if (status) {
       cout <<"***Error in reading the file: "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }
   
   for (i=0;i<nchan;i++)
   {
       spec1Data.spectrum[i]=spec[i];
       if(chantype==2) spec1Data.syserror[i]=spec[i]*sys_err[i];
   }

   return EXIT_SUCCESS;
   
}

int specFileHandler::write_type1_specfile(string specFile,specData spec1Data,vector <string> runHistory)
{

    int status=0;
    int nrows=0;
	long i;
    int hdutype;
    int channel_col,counts_col,stat_err_col,sys_err_col;
    fitsfile *fout;

    fits_open_file(&fout, specFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"CHANNEL",&channel_col,&status);
   fits_get_colnum(fout,CASEINSEN,"COUNTS",&counts_col,&status);
   fits_get_colnum(fout,CASEINSEN,"STAT_ERR",&stat_err_col,&status);
   fits_get_colnum(fout,CASEINSEN,"SYS_ERR",&sys_err_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   nrows=this->nchan;  // This takes care of whether it is PHA or PI (this is set at xsmgenspec.cpp) 

   float syserror_frac;

   for (i=0;i<nrows;i++)
   {
		fits_write_col(fout, TLONG, channel_col, i+1,1, 1, &i, &status);
		fits_write_col(fout, TFLOAT, counts_col, i+1,1, 1, &spec1Data.spectrum[i], &status);
		float error=sqrt(spec1Data.spectrum[i]);
        if (spec1Data.spectrum[i]!=0)
            syserror_frac=spec1Data.syserror[i]/spec1Data.spectrum[i];
        else
            syserror_frac=0.0;
        fits_write_col(fout, TFLOAT, stat_err_col, i+1,1, 1, &error, &status);
        fits_write_col(fout, TFLOAT, sys_err_col, i+1,1, 1, &syserror_frac, &status);
   }

   if (status) {
       cout <<"***Error in writing to the file: "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   //fits_update_key(fout, TLONG, "NAXIS2",&nrows, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTART",&spec1Data.tstart, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTOP",&spec1Data.tstop, NULL, &status);
   fits_update_key(fout, TDOUBLE, "EXPOSURE",&spec1Data.exposure, NULL, &status);

   char chantype_str[10];   
   if(chantype==1) sprintf(chantype_str,"%s","PHA");
   if(chantype==2) sprintf(chantype_str,"%s","PI");

   fits_update_key(fout, TSTRING, "CHANTYPE",chantype_str, NULL, &status);

    // Write keywords to header
    
    double telapse=spec1Data.tstop-spec1Data.tstart;

    fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
    fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);
    fits_update_key(fout, TDOUBLE, "TELAPSE",&telapse, NULL, &status);

    fits_update_key(fout, TSTRING, "ANCRFILE",(char *)this->arffile.c_str(), NULL, &status);
//    fits_update_key(fout, TSTRING, "RESPFILE",(char *)this->respfile.c_str(), NULL, &status);
    fits_update_key_longstr(fout, "RESPFILE",(char *)this->respfile.c_str(), NULL, &status);


	string caldbver;
	status=getCaldbVer(caldbver);
	fits_update_key_longstr(fout, "CALDBVER",(char *)caldbver.c_str(), NULL, &status);	

   if (status) {
       cout <<"***Error in writing to header of file: "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}

int specFileHandler::write_type2_specfile(string specFile,vector <specData> spec2DataVec, vector <string> runHistory)
{

    int status=0;
    long nrows=0,i,j;
    int hdutype;
    int spec_num_col,channel_col,counts_col,stat_err_col,respfile_col,sys_err_col;
	int exposure_col,tstart_col,tstop_col,filtstatus_col;
    fitsfile *fout;

/*
 *
 *  This is now taken care of in creating the pha file where based on number of channels 
 *  in spectrum (nchan) template file is chosen to be of 1024 or 512. For any other value,
 *  Exception is returned
 *
	if(nchan!=SPEC_SIZE)
	{
		cout<<"Number of channels in spectrum is different from Type II file template\n";
		cout<<"Unable to write to type II PHA file\n";
		cout<<"Exiting without writing output file\n";
		return EXIT_FAILURE;
	}
*/
    fits_open_file(&fout, specFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"SPEC_NUM",&spec_num_col,&status);
   fits_get_colnum(fout,CASEINSEN,"CHANNEL",&channel_col,&status);
   fits_get_colnum(fout,CASEINSEN,"COUNTS",&counts_col,&status);
   fits_get_colnum(fout,CASEINSEN,"STAT_ERR",&stat_err_col,&status);
   fits_get_colnum(fout,CASEINSEN,"SYS_ERR",&sys_err_col,&status);   
   fits_get_colnum(fout,CASEINSEN,"EXPOSURE",&exposure_col,&status);
   //fits_get_colnum(fout,CASEINSEN,"RESPFILE",&respfile_col,&status);
   fits_get_colnum(fout,CASEINSEN,"TSTART",&tstart_col,&status);
   fits_get_colnum(fout,CASEINSEN,"TSTOP",&tstop_col,&status);
   fits_get_colnum(fout,CASEINSEN,"FILT_STATUS",&filtstatus_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   long channel[nchan];
   for(i=0;i<nchan;i++) channel[i]=i;		

   nrows=spec2DataVec.size();

//   printf("NROWS %ld\n",nrows);

   for (i=0;i<nrows;i++)
   {
		int num=i;
        
		fits_write_col(fout, TINT, spec_num_col, i+1,1, 1, &num, &status);
        fits_write_col(fout, TLONG, channel_col, i+1,1, nchan, channel, &status);

		float spec[nchan];
		for(j=0;j<nchan;j++) spec[j]=spec2DataVec[i].spectrum[j];

        fits_write_col(fout, TFLOAT, counts_col, i+1,1, nchan, spec, &status);
     
	 	
		float spec_error[nchan];

//		char *respfile_dummy;
//		respfile_dummy=(char*)malloc(sizeof(char )*10);
//		strcpy(respfile_dummy,"none");

        float syserror_frac[nchan];

		for(j=0;j<nchan;j++)
        {
            spec_error[j]=sqrt(spec2DataVec[i].spectrum[j]);

            if (spec2DataVec[i].spectrum[j]!=0)
                syserror_frac[j]=spec2DataVec[i].syserror[j]/spec2DataVec[i].spectrum[j];
            else
                syserror_frac[j]=0.0;
        }
        
        fits_write_col(fout, TFLOAT, stat_err_col, i+1,1, nchan, spec_error, &status);

        fits_write_col(fout, TFLOAT, sys_err_col, i+1,1, nchan, syserror_frac, &status);
        	
//	    fits_write_col_str(fout,respfile_col,i+1,1,1,&respfile_dummy,&status);

        fits_write_col(fout, TDOUBLE, exposure_col, i+1,1, 1, &spec2DataVec[i].exposure, &status);
        fits_write_col(fout, TDOUBLE, tstart_col, i+1,1, 1, &spec2DataVec[i].tstart, &status);
        fits_write_col(fout, TDOUBLE, tstop_col, i+1,1, 1, &spec2DataVec[i].tstop, &status);

        fits_write_col(fout, TBYTE, filtstatus_col, i+1,1, 1, &spec2DataVec[i].filterStatus, &status);


//        free(respfile_dummy);

   if (status) {
       cout <<"***Error in writing data row to the file: "<<specFile<<"\n";
	   cout <<"ROW IS "<<i<<"\n";	
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }
				
   }

   if (status) {
       cout <<"***Error in writing to the file: "<<specFile <<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   char chantype_str[10];
   if(chantype==1) sprintf(chantype_str,"%s","PHA");
   if(chantype==2) sprintf(chantype_str,"%s","PI");

   fits_update_key(fout, TSTRING, "CHANTYPE",chantype_str, NULL, &status);

   double telapse=spec2DataVec[nrows-1].tstop-spec2DataVec[0].tstart;

   fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
   fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);
   fits_update_key(fout, TDOUBLE, "TELAPSE",&telapse, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTART",&spec2DataVec[0].tstart, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTOP",&spec2DataVec[nrows-1].tstop, NULL, &status);

   fits_update_key(fout, TSTRING, "ANCRFILE",(char *)this->arffile.c_str(), NULL, &status);
   //fits_update_key(fout, TSTRING, "RESPFILE",(char *)this->respfile.c_str(), NULL, &status);
   fits_update_key_longstr(fout, "RESPFILE",(char *)this->respfile.c_str(), NULL, &status);

    string caldbver;
    status=getCaldbVer(caldbver);
    fits_update_key_longstr(fout, "CALDBVER",(char *)caldbver.c_str(), NULL, &status);
   
   if (status) {
       cout <<"***Error in writing to header of file: "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<specFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<specFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }


	return EXIT_SUCCESS;
}

/*********************************************************************************************************************************************/

// gtiFileHandler

int gtiFileHandler::create_gtifile(string gtiFileName)
{
    int status=0;

    status=create_empty_fitsfile(gtiFileName,GTITEMPLATE);

    return EXIT_SUCCESS;
}

int gtiFileHandler::write_gtifile(string gtiFile,vector <gtiData> gtiDataVec,vector <string> runHistory)
{

	int status=0;
    long nrows=0,i;
    int hdutype;
    int start_col,stop_col;
    fitsfile *fout;

    fits_open_file(&fout, gtiFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<gtiFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<gtiFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"START",&start_col,&status);
   fits_get_colnum(fout,CASEINSEN,"STOP",&stop_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<gtiFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   nrows=gtiDataVec.size();
   double exposure=0;

   for (i=0;i<nrows;i++)
   {
        fits_write_col(fout, TDOUBLE, start_col, i+1,1, 1, &gtiDataVec[i].tstart, &status);
        fits_write_col(fout, TDOUBLE, stop_col, i+1,1, 1, &gtiDataVec[i].tstop, &status);
		exposure+=(gtiDataVec[i].tstop-gtiDataVec[i].tstart);
   }

   double telapse=gtiDataVec[nrows-1].tstop-gtiDataVec[0].tstart;

   fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
   fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);
   fits_update_key(fout, TDOUBLE, "TELAPSE",&telapse, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTART",&gtiDataVec[0].tstart, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTOP",&gtiDataVec[nrows-1].tstop, NULL, &status);
   fits_update_key(fout, TDOUBLE, "EXPOSURE",&exposure, NULL, &status);
   fits_update_key(fout, TDOUBLE, "OBSEXP",&tot_exposure, NULL, &status);

   if (status) {
       cout <<"***Error in writing to header of file: "<<gtiFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }


   if (status) {
       cout <<"***Error in writing to the file: "<<gtiFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<gtiFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<gtiFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

	return EXIT_SUCCESS;
}

int gtiFileHandler::read_gtifile(string gtiFile,vector <gtiData> &gtiDataVec)
{
    int status=0;
    long nrows=0,i;
    int hdutype;
    int start_col,stop_col;
    fitsfile *fout;
	gtiData singlegtiData;

    fits_open_file(&fout, gtiFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<gtiFile<<"\n";
        cout <<"***Error: Check the file format\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in moving to data HDU in file "<<gtiFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

    float input_version;
    fits_read_key(fout, TFLOAT, "XSMDASVE", &input_version, NULL, &status);
    if(status) input_version=1.0;
       
    if(status || input_version < (MINIMUM_VERSION-0.005))
    {
        cout<<"***Error: Input data generated with incompatible version of XSMDAS\n";
        cout<<"***Data Version : "<<input_version<<"\n";
        cout<<"***Minimum req. version: "<<MINIMUM_VERSION<<"\n";
        cout<<"***Exiting without proceeding further\n";
        return (EXIT_FAILURE);
    }

   fits_get_colnum(fout,CASEINSEN,"START",&start_col,&status);
   fits_get_colnum(fout,CASEINSEN,"STOP",&stop_col,&status);

   fits_get_num_rows(fout, &nrows, &status);

   if (status) {
        cout <<"***Error: Check the file format\n";
       cout <<"***Error in getting expected column numbers for file "<<gtiFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    if(nrows==0){
        cout<<"***Error: No rows in input GTI file "<<gtiFile<<"\n";
        return EXIT_FAILURE;
    }

   gtiDataVec.clear();

   for (i=0;i<nrows;i++)
   {
        fits_read_col(fout, TDOUBLE, start_col, i+1,1,1,NULL, &singlegtiData.tstart, NULL,&status);
        fits_read_col(fout, TDOUBLE, stop_col, i+1,1,1,NULL, &singlegtiData.tstop, NULL,&status);

		gtiDataVec.push_back(singlegtiData);		

	    if (status) {
    	   cout <<"***Error in reading from the file: "<<gtiFile<<"\n";
       	fits_report_error(stderr, status);
       	return (EXIT_FAILURE);
   		}

   }

    // Read Header keywords

    char date_obs1[80],date_end1[80];
    fits_read_key(fout, TSTRING, "DATE-OBS", date_obs1, NULL, &status);
    fits_read_key(fout, TSTRING, "DATE-END", date_end1, NULL, &status);
    this->date_obs=(string)date_obs1;
    this->date_end=(string)date_end1;
    fits_read_key(fout,TDOUBLE,"TSTART",&this->tstart,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TSTOP",&this->tstop,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TELAPSE",&this->telapse,NULL, &status);

    if (status) {
        cout <<"***Error in reading Header keywords from file "<<gtiFile<<"\n";
		cout <<"***Error: Mandatory keywords missing from file\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<gtiFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

	return EXIT_SUCCESS;
}

/*********************************************************************************************************************************************/
// saFileHandler

int saFileHandler::create_safile(string saFileName)
{
    int status=0;

    status=create_empty_fitsfile(saFileName,SATEMPLATE);

    return EXIT_SUCCESS;
}

int saFileHandler::write_safile(string saFile,vector <sunAngData> sunAngDataVec,vector <string> runHistory)
{
    int status=0;
    long nrows=0,i;
    int hdutype;
    int time_col,theta_col,phi_col,fovFlag_col,occultFlag_col,detPosR_col;
    int ramAngle_col, ra_col,dec_col,angRate_col;

    fitsfile *fout;
	
    fits_open_file(&fout, saFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
   fits_get_colnum(fout,CASEINSEN,"THETA",&theta_col,&status);
   fits_get_colnum(fout,CASEINSEN,"PHI",&phi_col,&status);
   fits_get_colnum(fout,CASEINSEN,"FOVFLAG",&fovFlag_col,&status);
   fits_get_colnum(fout,CASEINSEN,"OCCULTFLAG",&occultFlag_col,&status);
   fits_get_colnum(fout,CASEINSEN,"DETPOSR",&detPosR_col,&status);
   fits_get_colnum(fout,CASEINSEN,"RAMANGLE",&ramAngle_col,&status);
   fits_get_colnum(fout,CASEINSEN,"RA",&ra_col,&status);
   fits_get_colnum(fout,CASEINSEN,"DEC",&dec_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ANGRATE",&angRate_col,&status);


    if (status) {
        cout <<"***Error in getting column numbers for file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    nrows=sunAngDataVec.size();

    for (i=0;i<nrows;i++)
    {

        fits_write_col(fout, TDOUBLE, time_col, i+1,1, 1, &sunAngDataVec[i].time, &status);
        fits_write_col(fout, TFLOAT, theta_col, i+1,1, 1, &sunAngDataVec[i].theta, &status);
        fits_write_col(fout, TFLOAT, phi_col, i+1,1, 1, &sunAngDataVec[i].phi, &status);
        fits_write_col(fout, TBYTE, fovFlag_col, i+1,1, 1, &sunAngDataVec[i].fovFlag, &status);
        fits_write_col(fout, TBYTE, occultFlag_col, i+1,1, 1, &sunAngDataVec[i].occultFlag, &status);
        fits_write_col(fout, TFLOAT, detPosR_col, i+1,1, 1, &sunAngDataVec[i].detPosR, &status);

        fits_write_col(fout, TFLOAT, ramAngle_col, i+1,1, 1, &sunAngDataVec[i].ramAngle, &status);
        fits_write_col(fout, TFLOAT, ra_col, i+1,1, 1, &sunAngDataVec[i].ra, &status);
        fits_write_col(fout, TFLOAT, dec_col, i+1,1, 1, &sunAngDataVec[i].dec, &status);
        fits_write_col(fout, TFLOAT, angRate_col, i+1,1, 1, &sunAngDataVec[i].angRate, &status);

	
	    if (status) {
    	    cout << "***Error in writing to sun angle file.\n";
        	fits_report_error(stderr, status);
        	return (EXIT_FAILURE);
    	}
	
	}

    // Write keywords to header

    fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
    fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTART",&this->tstart, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TSTOP",&this->tstop, NULL, &status);
    fits_update_key(fout, TDOUBLE, "TELAPSE",&this->telapse, NULL, &status);

    if (status) {
        cout <<"***Error in writing Header keywords to file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }
	

	return EXIT_SUCCESS;
}

int saFileHandler::read_safile(string saFile,vector <sunAngData> &sunAngDataVec)
{
    int status=0;
    long nrows=0,i;
    int hdutype;
    int time_col,theta_col,phi_col,fovFlag_col,occultFlag_col,detPosR_col;
    int ramAngle_col, ra_col,dec_col,angRate_col;

    fitsfile *fout;

    fits_open_file(&fout, saFile.c_str(), READONLY, &status);
    if (status) {
        cout <<"***Error in opening file "<<saFile<<"\n";
        cout <<"***Error: Check the file format\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in moving to data HDU in file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   float input_version;
   fits_read_key(fout, TFLOAT, "XSMDASVE", &input_version, NULL, &status);
   if(status) input_version=1.0;

   if(status || input_version < (MINIMUM_VERSION-0.005))
   {
       cout<<"***Error: Input data generated with incompatible version of XSMDAS\n";
       cout<<"***Data Version : "<<input_version<<"\n";
       cout<<"***Minimum req. version: "<<MINIMUM_VERSION<<"\n";
       cout<<"***Exiting without proceeding further\n";
       return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
   fits_get_colnum(fout,CASEINSEN,"THETA",&theta_col,&status);
   fits_get_colnum(fout,CASEINSEN,"PHI",&phi_col,&status);
   fits_get_colnum(fout,CASEINSEN,"FOVFLAG",&fovFlag_col,&status);
   fits_get_colnum(fout,CASEINSEN,"OCCULTFLAG",&occultFlag_col,&status);
   fits_get_colnum(fout,CASEINSEN,"DETPOSR",&detPosR_col,&status);

   fits_get_colnum(fout,CASEINSEN,"RAMANGLE",&ramAngle_col,&status);
   fits_get_colnum(fout,CASEINSEN,"RA",&ra_col,&status);
   fits_get_colnum(fout,CASEINSEN,"DEC",&dec_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ANGRATE",&angRate_col,&status);

    if (status) {
        cout <<"***Error: Check the file format\n";
        cout <<"***Error in getting column numbers for file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_get_num_rows(fout, &nrows, &status);

    if (status) {
        cout <<"**Error in getting row numbers for file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    if(nrows==0){
        cout<<"***Error: No rows in input Sun angle file "<<saFile<<"\n";
        return EXIT_FAILURE;
    }

    sunAngData sunAngDataRow;
    sunAngDataVec.clear();

    for (i=0;i<nrows;i++)
    {

        fits_read_col(fout, TDOUBLE, time_col, i+1,1, 1, NULL,&sunAngDataRow.time,NULL, &status);
        fits_read_col(fout, TFLOAT, theta_col, i+1,1, 1, NULL,&sunAngDataRow.theta,NULL, &status);
        fits_read_col(fout, TFLOAT, phi_col, i+1,1, 1, NULL,&sunAngDataRow.phi,NULL, &status);
        fits_read_col(fout, TBYTE, fovFlag_col, i+1,1, 1, NULL,&sunAngDataRow.fovFlag,NULL, &status);
        fits_read_col(fout, TBYTE, occultFlag_col, i+1,1, 1, NULL,&sunAngDataRow.occultFlag,NULL, &status);
        fits_read_col(fout, TFLOAT, detPosR_col, i+1,1, 1, NULL,&sunAngDataRow.detPosR,NULL, &status);

        fits_read_col(fout, TFLOAT, ramAngle_col, i+1,1, 1, NULL,&sunAngDataRow.ramAngle,NULL, &status);
        fits_read_col(fout, TFLOAT, ra_col, i+1,1, 1, NULL,&sunAngDataRow.ra,NULL, &status);
        fits_read_col(fout, TFLOAT, dec_col, i+1,1, 1, NULL,&sunAngDataRow.dec,NULL, &status);
        fits_read_col(fout, TFLOAT, angRate_col, i+1,1, 1, NULL,&sunAngDataRow.angRate,NULL, &status);


        if (status) {
            cout <<"***Error in reading from the file: "<<saFile<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }

        /*
         * Removed in version 1.06 on 18 Aug 2020, as it is fixed in calsa
        // TEMPORARY FIX FOR WRONG PHI VALUES; NEED TO BE REMOVED WHEN CALSA IS CORRECTED AND RE-RUN
        //if(sunAngDataRow.phi>0) sunAngDataRow.phi-=180.0;
        */

        sunAngDataVec.push_back(sunAngDataRow);
	}


    // Read Header keywords

    char date_obs1[80],date_end1[80];
    fits_read_key(fout, TSTRING, "DATE-OBS", date_obs1, NULL, &status);
    fits_read_key(fout, TSTRING, "DATE-END", date_end1, NULL, &status);
    this->date_obs=(string)date_obs1;
    this->date_end=(string)date_end1;
    fits_read_key(fout,TDOUBLE,"TSTART",&this->tstart,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TSTOP",&this->tstop,NULL, &status);
    fits_read_key(fout,TDOUBLE,"TELAPSE",&this->telapse,NULL, &status);

    if (status) {
        cout <<"***Error in reading Header keywords from file "<<saFile<<"\n";
		cout <<"***Error: Mandatory keywords missing\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<saFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }



	return EXIT_SUCCESS;
}
/*********************************************************************************************************************************************/

//respFileHandler


int respFileHandler::create_respfile(string respFileName,int nPIbin)
{
    int status=0;

	if(nPIbin==1024)
	    status=create_empty_fitsfile(respFileName,RESPTEMPLATE);
	else if (nPIbin==512)
        status=create_empty_fitsfile(respFileName,RESPTEMPLATE512);
	else 
	{
        cout <<"***Error in finding response file template with DETCHANS== "<<nPIbin<<"\n";
        return (EXIT_FAILURE);
	}

    return EXIT_SUCCESS;
}

int respFileHandler::write_respfile(string respFileName,float *Elo,float *Ehi,int *ngrp,int *fchan,int *nchan,float **rspmatrix,int n_Ebin,int n_PIbin,float *PImin,float *PImax,int *PIchannel,int  includearea)
{

	int status=0,i;

    fitsfile *frsp;

    fits_open_file(&frsp, respFileName.c_str(), READWRITE, &status);
    if (status) {
        cout << "***Error in opening response file.\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_movnam_hdu(frsp, BINARY_TBL, "EBOUNDS", 0, &status);

    fits_write_col(frsp,TINT,1,1,1,n_PIbin,PIchannel,&status);
    fits_write_col(frsp,TFLOAT,2,1,1,n_PIbin,PImin,&status);
    fits_write_col(frsp,TFLOAT,3,1,1,n_PIbin,PImax,&status);

    if (status) {
        cout << "***Error in writing to response file EBOUNDS extension.\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    fits_movnam_hdu(frsp, BINARY_TBL, "MATRIX", 0, &status);

    for(i=0;i<n_Ebin;i++)
    {
    fits_write_col(frsp, TFLOAT, 1, i+1,1, 1,&Elo[i], &status);
    fits_write_col(frsp, TFLOAT, 2, i+1,1, 1,&Ehi[i], &status);
    fits_write_col(frsp, TINT, 3, i+1,1, 1,&ngrp[i], &status);
    fits_write_col(frsp, TINT, 4, i+1,1,1,&fchan[i], &status);
    fits_write_col(frsp, TINT, 5, i+1,1,1,&nchan[i], &status);
    fits_write_col(frsp, TFLOAT, 6, i+1,1, n_PIbin,rspmatrix[i], &status);
    }

    if (status) {
        cout << "***Error in writing to response file MATRIX extension.\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    char hduclas3_str[10];

    if(includearea)
        sprintf(hduclas3_str,"%s","FULL");
    else
        sprintf(hduclas3_str,"%s","REDIST");
            
    fits_update_key(frsp, TSTRING, "HDUCLAS3",hduclas3_str, NULL, &status);
        

    // Close rsp file 

    fits_close_file(frsp,&status);
    if (status) {
        cout << "***Error in closing response file.\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

/*********************************************************************************************************************************************/

//lcFileHandler

int lcFileHandler::create_lcfile(string lcFileName)
{
    int status=0;

    if(lctype==1) 
		status=create_empty_fitsfile(lcFileName,LC1TEMPLATE);
	else if(lctype==2)
        status=create_empty_fitsfile(lcFileName,LC2TEMPLATE);

    return (status);
}


int lcFileHandler::write_type1_lcfile(string lcFile,lcData lc1Data,vector <string> runHistory)
{

    int status=0;
    long i;
    int hdutype;
    int time_col,rate_col,error_col,fracexp_col;
    fitsfile *fout;

    fits_open_file(&fout, lcFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<lcFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<lcFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
   fits_get_colnum(fout,CASEINSEN,"RATE",&rate_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ERROR",&error_col,&status);
   fits_get_colnum(fout,CASEINSEN,"FRACEXP",&fracexp_col,&status);


   if (status) {
       cout <<"***Error in getting column numbers for file "<<lcFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   long nrows=lc1Data.ntbins;

   long nl=0;

   for(i=0;i<nrows;i++)
   {
	   if(lc1Data.fracexp[i]>0)
	   {
	   	nl+=1;
   		fits_write_col(fout, TDOUBLE, time_col, nl,1, 1, &lc1Data.lctime[i], &status);
   		fits_write_col(fout, TFLOAT, rate_col, nl,1, 1, &lc1Data.lcrate[i], &status);
   		fits_write_col(fout, TFLOAT, error_col, nl,1, 1, &lc1Data.lcerror[i], &status);
   		fits_write_col(fout, TFLOAT, fracexp_col, nl,1, 1, &lc1Data.fracexp[i], &status);
	   }
   }

   if (status) {
       cout <<"***Error in writing to the file: "<<lcFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   fits_update_key(fout, TDOUBLE, "TSTART",&lc1Data.tstart, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTOP",&lc1Data.tstop, NULL, &status);

   fits_update_key(fout, TDOUBLE, "TIMEDEL",&lc1Data.tbinsize, NULL, &status);

   double telapse=lc1Data.tstop-lc1Data.tstart;

   fits_update_key(fout, TDOUBLE, "TELAPSE",&telapse, NULL, &status);


   fits_update_key(fout, TINT, "CHSTART",&lc1Data.chstart, NULL, &status);
   fits_update_key(fout, TINT, "CHSTOP",&lc1Data.chstop, NULL, &status);

   fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
   fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);

   string caldbver;
   status=getCaldbVer(caldbver);
   fits_update_key_longstr(fout, "CALDBVER",(char *)caldbver.c_str(), NULL, &status);

   if (status) {
       cout <<"***Error in writing to header of file: "<<lcFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<lcFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<lcFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   return EXIT_SUCCESS;

}

int lcFileHandler::write_type2_lcfile(string lcFile,vector <lcData> lc2Data,vector <string> runHistory)
{

    int status=0;
    long i;
    int band;
    int hdutype;
    int time_col,rate_col,error_col,fracexp_col;
    fitsfile *fout;

    fits_open_file(&fout, lcFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<lcFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    for(band=0;band<3;band++)
    {
        fits_movabs_hdu(fout, 2+band, &hdutype, &status);
        if (status) {
            cout <<"***Error in moving to data HDU "<<band+2<<" in file "<<lcFile<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }

        fits_get_colnum(fout,CASEINSEN,"TIME",&time_col,&status);
        fits_get_colnum(fout,CASEINSEN,"RATE",&rate_col,&status);
        fits_get_colnum(fout,CASEINSEN,"ERROR",&error_col,&status);
        fits_get_colnum(fout,CASEINSEN,"FRACEXP",&fracexp_col,&status);


        if (status) {
            cout <<"***Error in getting column numbers for file "<<lcFile<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }

        long nrows=lc2Data[band].ntbins;

        long nl=0;

        for(i=0;i<nrows;i++)
        {
            if(lc2Data[band].fracexp[i]>0)
            {
                nl+=1;
                fits_write_col(fout, TDOUBLE, time_col, nl,1, 1, &lc2Data[band].lctime[i], &status);
                fits_write_col(fout, TFLOAT, rate_col, nl,1, 1, &lc2Data[band].lcrate[i], &status);
                fits_write_col(fout, TFLOAT, error_col, nl,1, 1, &lc2Data[band].lcerror[i], &status);
                fits_write_col(fout, TFLOAT, fracexp_col, nl,1, 1, &lc2Data[band].fracexp[i], &status);
            }
        }


        if (status) {
            cout <<"***Error in writing to the file: "<<lcFile<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }

        fits_update_key(fout, TDOUBLE, "TSTART",&lc2Data[band].tstart, NULL, &status);
        fits_update_key(fout, TDOUBLE, "TSTOP",&lc2Data[band].tstop, NULL, &status);

        fits_update_key(fout, TDOUBLE, "TIMEDEL",&lc2Data[band].tbinsize, NULL, &status);

        double telapse=lc2Data[band].tstop-lc2Data[band].tstart;

        fits_update_key(fout, TDOUBLE, "TELAPSE",&telapse, NULL, &status);


        fits_update_key(fout, TINT, "CHSTART",&lc2Data[band].chstart, NULL, &status);
        fits_update_key(fout, TINT, "CHSTOP",&lc2Data[band].chstop, NULL, &status);

        fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
        fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);

	    string caldbver;
    	status=getCaldbVer(caldbver);
    	fits_update_key_longstr(fout, "CALDBVER",(char *)caldbver.c_str(), NULL, &status);

        if (status) {
            cout <<"***Error in writing to header of file: "<<lcFile<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }

    }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<lcFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }
    
    fits_close_file(fout,&status);
    if (status) {
        cout <<"***Error in closing file "<<lcFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

/*********************************************************************************************************************************************/

//arfFileHandler

int arfFileHandler::create_arffile(string arfFileName)
{

    int status=0;

    if(arftype==1)
        status=create_empty_fitsfile(arfFileName,ARF1TEMPLATE);
    else if(arftype==2)
        status=create_empty_fitsfile(arfFileName,ARF2TEMPLATE);

    return (status);

}

int arfFileHandler::read_type1_arffile(string arfFile,arfData &arf1Data)
{
    int status=0;
    int nrows=0;
    long i;
    int hdutype;
    int energlo_col,energhi_col,specresp_col;
    fitsfile *fout;

    fits_open_file(&fout, arfFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"ENERG_LO",&energlo_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ENERG_HI",&energhi_col,&status);
   fits_get_colnum(fout,CASEINSEN,"SPECRESP",&specresp_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   fits_read_key(fout,TINT,"NAXIS2",&nrows,NULL, &status);
   arf1Data.nchan=nrows;
   arf1Data.initialize(); 

   fits_read_col(fout,TFLOAT,specresp_col,1,1,nrows,NULL,arf1Data.area,NULL,&status);

   if (status) {
       cout <<"***Error in reading the file: "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   return EXIT_SUCCESS;

}


int arfFileHandler::write_type1_arffile(string arfFile,arfData arf1Data,vector <string> runHistory)
{

    int status=0;
    int nrows=0;
    long i;
    int hdutype;
    int energlo_col,energhi_col,specresp_col;
    fitsfile *fout;

    fits_open_file(&fout, arfFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"ENERG_LO",&energlo_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ENERG_HI",&energhi_col,&status);
   fits_get_colnum(fout,CASEINSEN,"SPECRESP",&specresp_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   nrows=arf1Data.nchan;
   float Emin=RESP_EMIN;
   float Ebin=RESP_EBIN; 
   float elo,ehi;

   for (i=0;i<nrows;i++)
   {
        elo=Emin+i*Ebin;
        ehi=elo+Ebin;
        fits_write_col(fout, TFLOAT, energlo_col, i+1,1, 1, &elo, &status);
        fits_write_col(fout, TFLOAT, energhi_col, i+1,1, 1, &ehi, &status);

        float area=arf1Data.area[i]/arf1Data.exposure;

        fits_write_col(fout, TFLOAT, specresp_col, i+1,1, 1, &area, &status);
   }

   if (status) {
       cout <<"***Error in writing to the file: "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   //fits_update_key(fout, TLONG, "NAXIS2",&nrows, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTART",&arf1Data.tstart, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTOP",&arf1Data.tstop, NULL, &status);

    // Write keywords to header

    fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
    fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);

    string caldbver;
    status=getCaldbVer(caldbver);
    fits_update_key_longstr(fout, "CALDBVER",(char *)caldbver.c_str(), NULL, &status);

   if (status) {
       cout <<"***Error in writing to header of file: "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    status=writeHistory2FITS(fout,runHistory);

    if (status) {
        cout <<"***Error in writing history to file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}

int arfFileHandler::write_type2_arffile(string arfFile,vector <arfData> arf2DataVec, vector <string> runHistory)
{

    int status=0;
    long nrows=0,i,j;
    int hdutype;
    int arf_num_col,energlo_col,energhi_col,specresp_col; 
    fitsfile *fout;

    fits_open_file(&fout, arfFile.c_str(), READWRITE, &status);
    if (status) {
        cout <<"***Error in opening file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_movabs_hdu(fout, 2, &hdutype, &status);
   if (status) {
        cout <<"***Error in moving to data HDU in file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   }

   fits_get_colnum(fout,CASEINSEN,"ARF_NUM",&arf_num_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ENERG_LO",&energlo_col,&status);
   fits_get_colnum(fout,CASEINSEN,"ENERG_HI",&energhi_col,&status);
   fits_get_colnum(fout,CASEINSEN,"SPECRESP",&specresp_col,&status);

   if (status) {
       cout <<"***Error in getting column numbers for file "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   nrows=arf2DataVec.size();

   int nchan=N_RESPENEBINS;
   float Emin=RESP_EMIN;
   float Ebin=RESP_EBIN;   
   float elo[nchan],ehi[nchan];

   for (i=0;i<nchan;i++)
   {
       elo[i]=Emin+i*Ebin;
       ehi[i]=elo[i]+Ebin;
   }

   for (i=0;i<nrows;i++)
   {
        int num=i;

        fits_write_col(fout, TINT, arf_num_col, i+1,1, 1, &num, &status);
        fits_write_col(fout, TFLOAT, energlo_col, i+1,1, nchan, elo, &status);
        fits_write_col(fout, TFLOAT, energhi_col, i+1,1, nchan, ehi, &status);

        float area[nchan];
        for (j=0;j<nchan;j++) area[j]=arf2DataVec[i].area[j]/arf2DataVec[i].exposure;

        fits_write_col(fout, TFLOAT, specresp_col, i+1,1, nchan, area, &status);

        if (status) {
            cout <<"***Error in writing data row to the file: "<<arfFile<<"\n";
            cout <<"ROW IS "<<i<<"\n";
            fits_report_error(stderr, status);
            return (EXIT_FAILURE);
        }
   
   }

   if (status) {
       cout <<"***Error in writing to the file: "<<arfFile <<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

   fits_update_key(fout, TSTRING, "DATE-OBS",(char *)this->date_obs.c_str(), NULL, &status);
   fits_update_key(fout, TSTRING, "DATE-END",(char *)this->date_end.c_str(), NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTART",&arf2DataVec[0].tstart, NULL, &status);
   fits_update_key(fout, TDOUBLE, "TSTOP",&arf2DataVec[nrows-1].tstop, NULL, &status);

    string caldbver;
    status=getCaldbVer(caldbver);
    fits_update_key_longstr(fout, "CALDBVER",(char *)caldbver.c_str(), NULL, &status);

   if (status) {
       cout <<"***Error in writing to header of file: "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }

    status=writeHistory2FITS(fout,runHistory);
    if (status) {
        cout <<"***Error in writing history to file "<<arfFile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
    }

   fits_close_file(fout,&status);
   if (status) {
       cout <<"***Error in closing file "<<arfFile<<"\n";
       fits_report_error(stderr, status);
       return (EXIT_FAILURE);
   }


    return EXIT_SUCCESS;
}

/*********************************************************************************************************************************************/

