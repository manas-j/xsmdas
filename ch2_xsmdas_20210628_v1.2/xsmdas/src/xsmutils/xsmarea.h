/* XSMAREA.H
 *
 * Mithun NPS 
 * */
#ifndef XSMAREA_H
#define XSMAREA_H

#include <xsmutils.h>
#include <xsmdata.h>

int get_effectivearea(vector <sunAngData> sunAngDataVec,  vector <hkData> hkDataVec, double **areaArr,int nEbins, int eneType, int relFlag, char *abscoeffile, char *effareaparfile,double tstart);

#endif

