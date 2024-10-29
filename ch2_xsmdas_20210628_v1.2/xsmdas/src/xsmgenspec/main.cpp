#include "xsmgenspec.h"

int main(int argc, char** argv)
{
	int status=0;

    print_banner("XSMGENSPEC");

	xsmgenspec xgen;

    if(xgen.read(argc,argv)){
        cout<<"***Error reading parameters***\n";
        cout<<"*** Check if the parameter file is corrupted or if the input file exists\n";        
        return (EXIT_FAILURE);
    }

    xgen.display();

    status=xgen.xsmgenspecProcess();

	if(status){
		cout<<"***Error in xsmgenspecProcess\n";
		return(status);
	}

	return EXIT_SUCCESS;
}

