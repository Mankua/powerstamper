/*===========================================================================*\
 | 
 |  FILE:	Plugin.cpp
 |			Skeleton project and code for a Utility
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 9-2-99
 | 
\*===========================================================================*/

#include "Nebbi.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

BOOL InDate() {
	return TRUE;
	SYSTEMTIME date;
	double pvtime;
	GetSystemTime(&date);
	SystemTimeToVariantTime(&date,&pvtime);

	int time = int(pvtime);
	if ( time > 37050 ) {
		MessageBox(GetCOREInterface()->GetMAXHWnd(),_T("PowerStamper Beta period is over.\nThanks for your help!"),_T("PowerStamper"),MB_OK);
		return FALSE;
		}
	else {
		return TRUE;
		}
	}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
}



__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESC); }


__declspec( dllexport ) int LibNumberClasses() {
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) {
	switch(i) {
		case 0: return GetSkeletonUtilDesc();
		default: return 0;
		}
	}


__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }



TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}
