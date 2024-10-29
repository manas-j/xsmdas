pro ch2xsm_read_data,FILES=files, data_str=data_str, ERR_CODE=err_code, ERR_MSG=err_msg;, _REF_EXTRA=_ref_extra

;; This program reads Chandrayaan-2 Solar X-ray Monitor (XSM) data 
;; for OSPEX. Not meant for independent use outside OSPEX.
;;   	
;; Based on OSPEX doc to read 'any' data file - https://hesperia.gsfc.nasa.gov/ssw/packages/spex/doc/ospex_any_specfile.htm
;;    	
;; Mithun N P S 
;; 
;; Trial version (22/03/18)
;;
;; Release version (18/08/20)
;;

	err_code = 0
	err_msg = ''

	;; Check if input file array is empty
	if files[0] eq '' then begin
    	err_code=1
    	err_msg = 'No spectrum file selected.'
    	return
	endif

	nf=n_elements(files)

	xsm_data=[]

	for i=0,nf-1 do begin
    	xsm_d=mrdfits(files(i),1,/unsigned,/dscale)
    	xsm_data=[xsm_data,xsm_d]
	endfor
    
	ntime=n_elements(xsm_data)
    ut_edges=dblarr(2,ntime)

    ;mjdref_str={mjd: 57754.0, time: 0.0}
    ;refTime=anytim(anytim2utc(/ccsds,mjdref_str))

    ;ut_edges(0,*)=anytim(xsm_data.tstart+refTime)
    ;ut_edges(0,*)=anytim(xsm_data.tstop+refTime)    

	tstart_mjd=57754.0+xsm_data.tstart/86400.0
	tstop_mjd=57754.0+xsm_data.tstop/86400.0

	for i=0l,ntime-1 do begin
       
        tstart_mjd_str={ mjd: floor(tstart_mjd(i)), time: (tstart_mjd(i)-floor(tstart_mjd(i)))*8.64d7 }
        ut_edges(0,i)=anytim(anytim2utc(/ccsds,tstart_mjd_str))

        tstop_mjd_str={ mjd: floor(tstop_mjd(i)), time: (tstop_mjd(i)-floor(tstop_mjd(i)))*8.64d7 }
        ut_edges(1,i)=anytim(anytim2utc(/ccsds,tstop_mjd_str))

	endfor	

	start_time=ut_edges(0,0)
	end_time=ut_edges(1,ntime-1)

	rcounts=xsm_data.counts
	ercounts=sqrt(xsm_data.stat_err^2+(xsm_data.sys_err*rcounts)^2)
	
	units='counts'
	area=1.0
    
    ltime=fltarr(512,ntime)
	for i=0l,ntime-1 do ltime(*,i)=xsm_data(i).exposure

	e1=0.5+findgen(512)*0.033
	ct_edges=transpose([[e1],[e1+0.033]])

	data_name='CH-2 XSM'
	title='CH-2 XSM SPECTRUM'
	
	respfilename='$xsmdas/caldb/CH2xsmrspwitharea_open20191214v01.rsp'
    ch2xsm_read_drm,DRM_STR , FILE=respfilename
    respfile=drm_str

	detused='XSM SDD'
	atten_states=-1
	deconvolved=0
	pseudo_livetime=0
	xyoffset=[0,0]

	;; Select spectrum in 1-15 keV range alone (required for OSPEX, no spectrum below 1 keV)
	
    ind=where(e1 ge 1.0 and e1 lt 15.0)
    ct_edges=ct_edges(*,ind)
    rcounts=rcounts(ind,*)
    ercounts=ercounts(ind,*)

    ;; Trying different filter state eff area for time dependance
    ;atten_states=(findgen(864) mod 20)

	data_str = { $
	START_TIME: start_time, $ ;start time of file in anytim format
	END_TIME: end_time, $  ;end time of file in anytim format
	RCOUNTS: rcounts, $ ;[nenergy,ntime] count data
	ERCOUNTS: ercounts, $ ;[nenergy,ntime] error in count data
	UT_EDGES: ut_edges, $ ;[2,ntime] edges of time bins in anytim format
	UNITS: units, $ ;units of rcounts, usually 'counts'
	AREA: area, $ ;effective detector area in cm^2
	LTIME: ltime, $ ;[nenergy,ntime] livetimes
	CT_EDGES: ct_edges, $ ;[2,nenergy] edges of energy bins in keV
	DATA_NAME: data_name, $ ;string name of data, e.g. 'RHESSI'
	TITLE: title, $ ;string title of data, e.g. 'RHESSI SPECTRUM'
	RESPFILE: respfile, $ ;string name of corresponding SRM file, or blank string
	;  OR [nedges_out, nedges_in] numeric array of DRM values
	;  OR structure containing fields drm ([nedges_out, nedges_in]), edges_in, and edges_out
	DETUSED: detused, $ ;string of detector names
	ATTEN_STATES: atten_states, $ ;[ntime] array of filter states, or scalar -1 if doesn't apply
	DECONVOLVED: deconvolved, $ ;If 1, original data is already photons
	PSEUDO_LIVETIME: pseudo_livetime, $ ;if 1, livetime isn't a real livetime
	XYOFFSET: xyoffset } ;[2] x,y location of source on Sun

end
