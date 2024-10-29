#include "xsmgenlc.h"

int main(int argc, char** argv)
{
	int status=0;

    print_banner("XSMGENLC");

	xsmgenlc xgen;

    if(xgen.read(argc,argv)){
        cout<<"***Error reading parameters***\n";
        cout<<"*** Check if the parameter file is corrupted or if the input file exists\n";        
        return (EXIT_FAILURE);
    }

    xgen.display();

    status=xgen.xsmgenlcProcess();

	if(status){
		cout<<"***Error in xsmgenlcProcess\n";
		return(status);
	}

	return EXIT_SUCCESS;
}

