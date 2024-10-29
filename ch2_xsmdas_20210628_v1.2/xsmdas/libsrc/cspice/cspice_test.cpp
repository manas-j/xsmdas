#include <stdio.h>
#include <stdlib.h>
#include "SpiceUsr.h"

int main()
{
    SpiceDouble a[3]={1,0,0};
    SpiceDouble b[3]={0,1,0};
    SpiceDouble dotp;

    dotp=vdot_c(a,b);

    return EXIT_SUCCESS;
}
