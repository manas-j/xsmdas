/* XSMDATA.H
 *
 * Mithun NPS 
 * */

#ifndef XSMDATA_H
#define XSMDATA_H

#include <xsmutils.h>

// Structure and class definitions for frame data

struct framehdr {	
	double time;
	char timestr[30];
    long sync_UW, sync_LW;
	long frame_no,fpga_time;
    long event_counter,event_detected;
    int ramp_counter,hv_monitor,temperature,tec_current,lv1_monitor,lv2_monitor;
	int ch1_start,ch1_stop,ch2_start,ch2_stop,ch3_start,ch3_stop;
	int lld_voltage,tecref_voltage;

    unsigned char pileup_rejection_mode;
    unsigned char pileup_time;

	long coarse_channel_events;
	long spectrum_events;
	long uld_events;

	int guardbits;

	// motor mechanism related	
	long window_lower_thresh,window_upper_thresh;
	unsigned char motor_control_mode,motor_operation_mode;
	unsigned char motor_manual_set_position;
	unsigned char motor_ir_position;
	unsigned char motor_automovement_time;
	unsigned char ir_power_status;
	unsigned char motor_stepmode_direction;
	unsigned char frame_discard;

};

struct framespec {
	double time;
	long spectrum[SPEC_SIZE];
	long event_counter,event_detected;
};

struct framelc {
	double time;
	long ch1lc[COARSECH_LCBIN];
    long ch2lc[COARSECH_LCBIN];
    long ch3lc[COARSECH_LCBIN];	
};

class frame {
	public:
		framehdr hdr;
		framespec spec;
		framelc lc;

		int decode_frame(unsigned char *frame_data,bool decode_hdr,bool decode_lc,bool decode_spec);
		//int decode_frame_v4(unsigned char *frame_data,bool decode_hdr,bool decode_lc,bool decode_spec);
        //int decode_frame_v5(unsigned char *frame_data,bool decode_hdr,bool decode_lc,bool decode_spec);

			
};

//sciencedata structure

struct scienceData{
	double time;
	char timestr[30];
	long frameNumber;
	double bdhTime;
	double xsmTime;
	int decodingStatusFlag;
	unsigned char dataArray[FRAME_SIZE];
};


//specData class

class specData{
	public:
		double tstart;
		double tstop;
		double exposure;
    	float *spectrum;
        float *syserror;
		string rspfile;
		int chantype;
		int n_PIbin;
		int nchan;
        unsigned char filterStatus;
	
		specData();
		~specData();	
		void initialize(int chantype,int n_PIbin);
        void makeFree();
		void addSpecArea(long *fspec,float *fsyserr,double fexp, double *area);
        void addSpec(long *fspec,float *fsyserr,double fexp);
		void setTimes(double startT,double stopT);
        void setFilter(unsigned char filterStatus);
};

//arfData class

class arfData{
    public:
        double tstart;
        double tstop;
        float area[N_RESPENEBINS];
        double exposure;
        int nchan;

        arfData();
        ~arfData();
        void initialize();
        void addArea(double *area,double fexp);
        void setTimes(double startT,double stopT);
};


// lcData 

struct lcData{

	double *lctime;
	float *lcrate;
	float *lcerror;
	float *fracexp;
    float *area;
    int chstart;
    int chstop;
	double tstart;
	double tstop;
	double tbinsize;
	long ntbins;

};

//hkData structure
struct hkData{
	double time;
    char timestr[30];
	long syncUW, syncLW;
	double frameNo,fpgaTime;
	long eventCounter,eventDetected;
	int rampCounter;
	float hvMonitor,detTemperature,tecCurent,lv1Monitor,lv2Monitor;
	float lldRefVoltage,tecRefVoltage;

    int ch1_start,ch1_stop,ch2_start,ch2_stop,ch3_start,ch3_stop;

    unsigned char pileup_rejection_mode;
    unsigned char pileup_time;

    long coarse_channel_events;
    long spectrum_events;
    long uld_events;

    int guardbits;

    // motor mechanism related  
    long window_lower_thresh,window_upper_thresh;
    unsigned char motor_control_mode,motor_operation_mode;
    unsigned char motor_manual_set_position;
    unsigned char motor_ir_position;
    unsigned char motor_automovement_time;
    unsigned char ir_power_status;
    unsigned char motor_stepmode_direction;
    unsigned char frame_discard;

};

//sunangData structure

struct sunAngData{
	double time;
	float theta;
	float phi;
	float detPosR;
    float ramAngle;
    float ra;
    float dec;
    float angRate;
    unsigned char fovFlag;
	unsigned char occultFlag;
};


//gtiData class

struct gtiData{
	double tstart;
	double tstop;
};

double computeExposure(vector<gtiData> gtiDataVec);

int gtiFilterScienceData(vector <scienceData> &scDataVec,vector<gtiData> gtiDataVec);
int gtiFilterHkData(vector <hkData> &hkDataVec,vector<gtiData> gtiDataVec);
int gtiFiltersunAngData(vector <sunAngData> &sunAngDataVec,vector<gtiData> gtiDataVec);

int checkL0TimeSanity(vector <scienceData> &scDataVec);


#endif
