/* XSMAREA.CPP
 *
 * Mithun NPS
 * */

#include "xsmarea.h"

int get_effectivearea(vector <sunAngData> sunAngDataVec,  vector <hkData> hkDataVec, double **areaArr,int nEbins, int eneType, int relFlag, char *abscoeffile, char *effareaparfile,double tstart)
{
//	cout <<"HERE\n";

	int status=0;
    int hdutype,nrows;
    long i;
    fitsfile *fabs,*fpar;
    double mu_be[nEbins],mu_si[nEbins],mu_sio2[nEbins];

    float dx,dy,xmin,ymin;
    int nxbins,nybins,nrowspar;

    // Get CALDB file names if required

    if(strcasecmp(abscoeffile, "CALDB")==0)
    {
        string caldbabscoeffile;
        status=queryCaldb("ABSCOEF",tstart,caldbabscoeffile);
        if(status) return(status);
        sprintf(abscoeffile,"%s",caldbabscoeffile.c_str());
        cout<<"MESSAGE: Abscoef CALDB file used is: "<<abscoeffile<<"\n";
    }

    if(strcasecmp(effareaparfile, "CALDB")==0)
    {
        string caldbeffareaparfile;
        status=queryCaldb("EFFAREAPAR",tstart,caldbeffareaparfile);
        if(status) return(status);
        sprintf(effareaparfile,"%s",caldbeffareaparfile.c_str());
        cout<<"MESSAGE: Effareapar CALDB file used is: "<<effareaparfile<<"\n";
    }

    // Read absorption coefficient data

    fits_open_file(&fabs,abscoeffile,READONLY,&status);
    if(status) {
        cout<<"***Error in opening abscoeffile "<<abscoeffile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    //eneType: 1-PI, 2-PHA, 3-RSP

   	fits_movabs_hdu(fabs, eneType+1, &hdutype, &status);
   	if (status) {
        cout <<"***Error in moving to PI/PHA/RSP HDU in file "<<abscoeffile<<"\n";
        fits_report_error(stderr, status);
        return (EXIT_FAILURE);
   	}
	
	fits_read_key(fabs,TINT,"NAXIS2",&nrows,NULL, &status);

	if(nrows!=nEbins)
    {
        cout<<"***Error: Mismatch in the number of energy bins in abscoef file\n";
        return (EXIT_FAILURE);
    }

    fits_read_col(fabs,TDOUBLE,2,1,1,nEbins,NULL,mu_be,NULL,&status);
    fits_read_col(fabs,TDOUBLE,3,1,1,nEbins,NULL,mu_si,NULL,&status);
    fits_read_col(fabs,TDOUBLE,4,1,1,nEbins,NULL,mu_sio2,NULL,&status);

    fits_close_file(fabs,&status);
    if(status) {
        cout<<"***Error in closing file "<<abscoeffile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    // Read effective area parameters

    fits_open_file(&fpar,effareaparfile,READONLY,&status);
    if(status) {
        cout<<"***Error in opening effareaparfile "<<effareaparfile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    fits_movabs_hdu(fpar, 2, &hdutype, &status);
    if(status) {
        cout<<"***Error in finding EFFAREAPAR extn in file "<<effareaparfile<<"\n";
        cout<<"***Incorrect format file\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }

    fits_read_key(fpar,TFLOAT,"DX",&dx,NULL, &status);
    fits_read_key(fpar,TFLOAT,"DY",&dy,NULL, &status);
    
    fits_read_key(fpar,TINT, "NXBINS",&nxbins, NULL, &status);
    fits_read_key(fpar,TINT, "NYBINS",&nybins, NULL, &status);

    fits_read_key(fpar,TFLOAT,"XMIN",&xmin,NULL, &status);
    fits_read_key(fpar,TFLOAT,"YMIN",&ymin,NULL, &status);

    fits_read_key(fpar,TINT,"NAXIS2",&nrowspar,NULL, &status);

    float alpha[nrowspar],be_thick[nrowspar],deadSi_thick[nrowspar],deadSiO2_thick[nrowspar],detSi_thick[nrowspar];

    fits_read_col(fpar,TFLOAT,3,1,1,nrowspar,NULL,alpha,NULL,&status);
    fits_read_col(fpar,TFLOAT,4,1,1,nrowspar,NULL,be_thick,NULL,&status);
    fits_read_col(fpar,TFLOAT,5,1,1,nrowspar,NULL,deadSi_thick,NULL,&status);
    fits_read_col(fpar,TFLOAT,6,1,1,nrowspar,NULL,deadSiO2_thick,NULL,&status);
    fits_read_col(fpar,TFLOAT,7,1,1,nrowspar,NULL,detSi_thick,NULL,&status);

    fits_close_file(fpar,&status);
    if(status) {
        cout<<"***Error in closing file "<<effareaparfile<<"\n";
        fits_report_error(stderr,status);
        return (EXIT_FAILURE);
    }


    // Compute area as function of time
    long nrowsSa;
    nrowsSa=sunAngDataVec.size();
	int xbin,ybin,rowpar,j;
	float theta,thrad;
	float detx,dety;
    float garea=0.00364999;
    float colarea,trans;
    float costh,be_t,si_t,sio2_t,detsi_t;

    float onaxis_area[nEbins];
    double threshold=1e-6;
	
//	cout<<"READ FILES  "<<nEbins<<  "\n";

/*
	for (j=0;j<nEbins;j++)
	{
        printf("%f\t%f\t%f\n",mu_be[j],mu_si[j],mu_sio2[j]);
	}
*/

    // If relative area is required, compute onaxis effective area
    if(relFlag==1)
    {
        xbin=(int)((0.0-xmin)/dx);
        ybin=(int)((0.0-ymin)/dy);

        if(xbin>nxbins) xbin=nxbins-1;
        if(ybin>nybins) ybin=nybins-1;

        rowpar=xbin*nybins+ybin;
                
        colarea=garea;
        be_t=be_thick[rowpar];
        si_t=deadSi_thick[rowpar];
        sio2_t=deadSiO2_thick[rowpar];
        detsi_t=detSi_thick[rowpar];

        for (j=0;j<nEbins;j++)
        {
            trans=exp(-mu_be[j]*be_t)*exp(-mu_si[j]*si_t)*exp(-mu_sio2[j]*sio2_t)*(1-exp(-mu_si[j]*detsi_t));
            onaxis_area[j]=colarea*trans;
        }
        
    }

    for (i=0;i<nrowsSa;i++)
    {
        detx=sunAngDataVec[i].detPosR*cos(sunAngDataVec[i].phi*3.1415/180.0);
        dety=sunAngDataVec[i].detPosR*sin(sunAngDataVec[i].phi*3.1415/180.0);

        xbin=(int)((detx-xmin)/dx);
        ybin=(int)((dety-ymin)/dy);

        if(xbin>nxbins) xbin=nxbins-1;
        if(ybin>nybins) ybin=nybins-1;

        /*
        float add_be=0.0;
        if(sunAngDataVec[i].phi<-90 && sunAngDataVec[i].theta > 20.26)
        {
            if(sunAngDataVec[i].theta > 26.0) add_be=(pow((sunAngDataVec[i].theta-20.),0.36)-0.6)*1e-4;
            else add_be=(pow((sunAngDataVec[i].theta-20.),0.31)-0.6)*1e-4;
            if(add_be<0.0) add_be=0.0;
        }

        if(sunAngDataVec[i].phi>-90 && sunAngDataVec[i].theta > 25.0) add_be=0.2e-4;
        */

        /*
        float testalpha=1.8;
        if(sunAngDataVec[i].phi>-90) testalpha=1.7;
        */
        
        //printf("XYVAL\t%d\t%d\n",xbin,ybin);

        rowpar=xbin*nybins+ybin;
        
        theta=sunAngDataVec[i].theta;
        thrad=theta*3.1415/180.0;
        costh=cos(thrad);

        //colarea=garea*pow(costh,testalpha)*erfc((theta-41.5)/2.5)/2.0;
        //be_t=(be_thick[rowpar]+add_be)/costh;
        
        colarea=garea*pow(costh,alpha[rowpar])*erfc((theta-41.5)/2.5)/2.0;
        be_t=(be_thick[rowpar])/costh;
        si_t=deadSi_thick[rowpar]/costh;
        sio2_t=deadSiO2_thick[rowpar]/costh;
        detsi_t=detSi_thick[rowpar]/costh;
        
        //printf("%f\t%f\n",add_be,be_t);

        for (j=0;j<nEbins;j++)
        {
            trans=exp(-mu_be[j]*be_t)*exp(-mu_si[j]*si_t)*exp(-mu_sio2[j]*sio2_t)*(1.0-exp(-mu_si[j]*detsi_t));
			areaArr[i][j]=colarea*trans;
		}

        //printf("%d\t%d\t%f\n",xbin,ybin,areaArr[i][24]);

        //printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\n",alpha[phbin],be_t,si_t,sio2_t,detsi_t,colarea,areaArr[i][90]);
       
    }
   

	// In case of relative effective area, divide the areaArr by the onaxis_area when it is above threshold

    if(relFlag==1)
    {
		for (j=0;j<nEbins;j++)
        {
            if(onaxis_area[j]>threshold)
            {
                for (i=0;i<nrowsSa;i++)
                    areaArr[i][j]=areaArr[i][j]/onaxis_area[j];
            }
            else
            {
                for (i=0;i<nrowsSa;i++)
                    areaArr[i][j]=1.0;                
            }
        }
    }

//	cout <<"END\n";

//	for (i=0;i<nrowsSa;i++) printf("%f\n",areaArr[i][16]);

    return (EXIT_SUCCESS);

}
