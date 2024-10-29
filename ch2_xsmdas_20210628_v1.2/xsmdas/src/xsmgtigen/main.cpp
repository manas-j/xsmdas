#include "xsmgtigen.h"

int main(int argc, char** argv)
{
	int status=0;

    print_banner("XSMGTIGEN");

	xsmgtigen xgti;

    if(xgti.read(argc,argv)){
        cout<<"***Error reading parameters***\n";
        cout<<"*** Check if the parameter file is corrupted or if the input file exists\n";        
        return (EXIT_FAILURE);
    }

    xgti.display();

    status=xgti.xsmgtigenProcess();

	if(status){
		cout<<"***ERROR in xsmgtigenProcess***\n";
		return(status);
	}

	return EXIT_SUCCESS;
}

