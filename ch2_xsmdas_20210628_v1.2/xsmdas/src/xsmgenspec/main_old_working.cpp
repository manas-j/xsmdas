#include "xsmutils.h"
#include "xsmdata.h"

int main()
{

	FILE *fptr,*fnam;

	frame f;
    vector <framespec> allspec;
	vector <framehdr> allhdr;

	long tot_spec[1024];

    long nf=0,ifile,i,ch;

	for(i=0;i<1024;i++) tot_spec[i]=0;
	
    fnam=fopen("raw_names.dat","r");
	do{
		ch=fgetc(fnam);
		if(ch=='\n') nf++;
	} while(ch!=EOF);
	fclose(fnam);

    char names[nf][50];


	fnam=fopen("raw_names.dat","r");


    for (ifile=0;ifile<nf;ifile++)
    {

        fscanf(fnam,"%s",names[ifile]);

        fptr=fopen(names[ifile],"rb");

		
        unsigned char bdh_frame[BDH_FRAME_SIZE];

        fread(bdh_frame,BDH_FRAME_SIZE,1,fptr);

        unsigned char data[FRAME_SIZE];

        for(i=0;i<FRAME_SIZE;i++) data[i]=bdh_frame[i+53];

		f.decode_frame(data,1,0,1);
    

		//printf("%d\t%x\t%x\t%ld\t%lf\t%ld\t%ld\t%ld\t%d\n",ifile,f.hdr.sync_UW,f.hdr.sync_LW,f.hdr.frame_no,(double)f.hdr.fpga_time/1e6,f.hdr.fast_counter,f.hdr.slow_counter,f.hdr.slow_detected,f.hdr.ramp_counter);

		allhdr.push_back(f.hdr);		
		allspec.push_back(f.spec);

		for(i=0;i<1024;i++) tot_spec[i]+=f.spec.spectrum[i];

		printf("%lf\n",f.hdr.fpga_time);

		//printf("%d\t%d\t%d\t%d\t%d\t%d\n",f.hdr.temperature,f.hdr.tec_current,f.hdr.hv_monitor,f.hdr.ramp_counter,f.hdr.lv1_monitor,f.hdr.lv2_monitor);

		//for(i=0;i<1024;i++) printf("%ld\t",f.spec.spectrum[i]);
		//printf("\n");

        fclose(fptr);
	}

	//for(i=0;i<1024;i++) printf("%ld\n",tot_spec[i]);

    fclose(fnam);


	return EXIT_SUCCESS;
}
