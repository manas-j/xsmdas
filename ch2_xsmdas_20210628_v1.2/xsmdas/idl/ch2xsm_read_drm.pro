pro ch2xsm_read_drm,DRM_STR , FILE=file ;, SFILE=sfile, ERR_CODE=err_code, ERR_MSG=err_msg

;; This program reads Chandrayaan-2 Solar X-ray Monitor (XSM) response matrix 
;; for OSPEX. Called by ch2xsm_read_data when invoked from OSPEX.
;; 
;; Based on OSPEX doc to read 'any' data file - https://hesperia.gsfc.nasa.gov/ssw/packages/spex/doc/ospex_any_specfile.htm
;;      
;; Mithun N P S (18/08/20)
;;

    err_code = 0
    err_msg = ''

	d1=mrdfits(file,1,/unsigned,/dscale)
	d2=mrdfits(file,2,/unsigned,/dscale)
	
	edges_out=transpose([[d1.E_MIN],[d1.E_MAX]])
	edges_in=transpose([[d2.ENERG_LO],[d2.ENERG_HI]])

	area=1.0
	drm=d2.matrix
	sepdets=0
	data_name='CH-2 XSM'
	atten_state=-1
	detused='XSM SDD'

	;; Select above 1 keV only

	ind_out=where(edges_out(0,*) ge 1.0 and edges_out(0,*) lt 15.0)
	ind_in=where(edges_in(0,*) ge 1.0)

    edges_out=edges_out(*,ind_out)
    edges_in=edges_in(*,ind_in)

	drm=drm[*,ind_in]
	drm=drm[ind_out,*]

	;; Trying different filter state eff area for time dependance
	;drm_filt=fltarr(n_elements(ind_out),n_elements(ind_in),20)
	;atten_state=findgen(20)
	;for i=0,19 do drm_filt(*,*,i)=drm
	;drm=drm_filt

	drm_str = { $
	EDGES_OUT: edges_out, $ ;[2,nedges_out] count energy edges in keV
	;PH_EDGES: ph_edges, $ ;[2,nedges_in] photon energy edges in keV
	EDGES_IN: edges_in, $ ;[2,nedges_in] photon energy edges in keV
    AREA: area, $ ;detector area in cm^2
	DRM: drm, $ ;[nedges_out,nedges_in,nfilter] DRM matrix for each filter state
	SEPDETS: sepdets, $ ;1 if separate detectors
	DATA_NAME: data_name, $ ;string name of data, e.g. 'RHESSI'
	FILTER: atten_state, $ ;[nfilter] value of filter states, or scalar -1 if doesn't apply
	DETUSED: detused } ;string of detectors used, e.g. '1F 3F 4F 5F 6F 8F 9F'

end
