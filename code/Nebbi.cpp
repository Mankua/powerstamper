/*===========================================================================*\
 | 
 |  FILE:	Nebbi.cpp
 |			PowerStamper
 |			Texture Builder by Bitmap Projection for 3d studio max
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Diego A. Castaño
 |			Mankua
 |			Copyright(c) Mankua 2001
 |
 |  HIST:	Started : March 1 2001
 | 
\*===========================================================================*/

/* AppData Structure:
Slot -1:	Demo version Mapping Channel. - The demo version reads the mapping channel only here
Slot 0 :	Nebbi Version ( 1000 )
Slot 1 :	Input Data
Slot 2 :	Input BMP Name
Slot 3 :	Output numgroups
Slot 4 :	Output Group 0 Data
Slot 5 :	Output Group 0 BMP Name
Slot 6 :	Output Group 1 Data
Slot 7 :	Output Group 1 BMP Name
*/

#if ( MAX_RELEASE >= 9000 )
	#define MAX_MALLOC MAX_malloc
#else
	#define MAX_MALLOC malloc
#endif

#define APP_DATA_NEBBI_VERSION		0
#define APP_DATA_INPUT_DATA			1
#define APP_DATA_INPUT_NAME			2
#define APP_DATA_OUTPUT_NUMGROUPS	3
#define APP_DATA_OUTGROUP_DATA		4
#define APP_DATA_OUTGROUP_NAME		5

#include "Nebbi.h"
#include <process.h>

// Warning messages
#define WARN_MULTIPLE_SEL	1
#define WARN_NONE_SEL		2
#define WARN_NO_INDATA		3
#define WARN_NO_INNAME		4
#define WARN_NO_INIMAGE		5
#define WARN_INVALID_OBJ	6
#define WARN_NOTHING_REND	7

Bitmap *	NebbiUtility::inBM = NULL;

/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class SkeletonUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return &theNebbiUtility; }
	const TCHAR *	ClassName()					{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()				{ return UTILITY_CLASS_ID; }
	Class_ID 		ClassID()					{ return NEBBI_CLASSID; }
	const TCHAR* 	Category()					{ return GetString(IDS_CATEGORY);  }
	void ResetClassParams (BOOL fileReset);
};

static SkeletonUtilClassDesc SkeletonUtilCD;
ClassDesc* GetSkeletonUtilDesc() {return &SkeletonUtilCD;}

// Reset all the utility values on File/Reset
void SkeletonUtilClassDesc::ResetClassParams (BOOL fileReset) 
{
}

static HIMAGELIST h_about_image = NULL;
static HIMAGELIST h_help_image = NULL;

static void LoadImages()
{
	if (!h_about_image) 
	{
		HBITMAP hBitmap, hMask;
		h_about_image = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 3, 0);
		hBitmap       = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_ABOUT));
		hMask         = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_ABOUT_MASK));
		ImageList_Add(h_about_image,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}
	if (!h_help_image) 
	{
		HBITMAP hBitmap, hMask;
		h_help_image  = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 3, 0);
		hBitmap       = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_HELP));
		hMask         = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_HELP_MASK));
		ImageList_Add(h_help_image,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}
}	

#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK AboutDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	switch (msg) {
		case WM_INITDIALOG:
			{
			CenterWindow(hWnd,GetParent(hWnd));
			break;
			}

		case WM_CLOSE:
			EndDialog(hWnd,1);
			break;


		default:
			return FALSE;
		}
	return TRUE;
	}

#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK LogDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK LogDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	TCHAR * info;
	switch (msg) {
		case WM_INITDIALOG: {
			info = (TCHAR*)lParam;
			CenterWindow(hWnd,GetParent(hWnd));

			SetWindowText(GetDlgItem(hWnd, IDC_LOG_TEXT), info );
//			SendMessage(GetDlgItem(hWnd, IDC_LOG_TEXT), EM_FINDTEXT ,1, 0);
			break;
			}

		case WM_CLOSE:
			EndDialog(hWnd,1);
			break;


		default:
			return FALSE;
		}
	return TRUE;
	}

/*===========================================================================*\
 |	Dialog Handler for Utility
\*===========================================================================*/

#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK InSizeDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK InSizeDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	static float *os;
	ISpinnerControl *spinW;
	ISpinnerControl *spinH; 
	ISpinnerControl *spinA; 
	switch (msg) {
		case WM_INITDIALOG: {
			os = (float*)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			
			spinW = SetupIntSpinner(hWnd,IDC_IBS_WIDTH_SPIN,IDC_IBS_WIDTH,0,8000,(int)os[1]);
			spinH = SetupIntSpinner(hWnd,IDC_IBS_HEIGHT_SPIN,IDC_IBS_HEIGHT,0,8000,(int)os[2]);
			spinA = SetupFloatSpinner(hWnd,IDC_IBS_ASPECT_SPIN,IDC_IBS_ASPECT,0.001f,1000.0f,os[3],0.001f);

			if ( (int)os[0] ) {
				CheckRadioButton(hWnd, IDC_SIZE_VIEWPORT, IDC_SIZE_CUSTOM, IDC_SIZE_CUSTOM);
				spinW->Enable(TRUE);
				spinH->Enable(TRUE);
				spinA->Enable(TRUE);
				}
			else {
				CheckRadioButton(hWnd, IDC_SIZE_VIEWPORT, IDC_SIZE_CUSTOM, IDC_SIZE_VIEWPORT);
				spinW->Enable(FALSE);
				spinH->Enable(FALSE);
				spinA->Enable(FALSE);
				}

			ReleaseISpinner(spinW);
			ReleaseISpinner(spinH);
			ReleaseISpinner(spinA);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					os[0] = (float)IsDlgButtonChecked(hWnd, IDC_SIZE_CUSTOM);

					spinW = GetISpinner(GetDlgItem(hWnd,IDC_IBS_WIDTH_SPIN));
					os[1] = (float)spinW->GetIVal();
					spinH = GetISpinner(GetDlgItem(hWnd,IDC_IBS_HEIGHT_SPIN));
					os[2] = (float)spinH->GetIVal();
					spinA = GetISpinner(GetDlgItem(hWnd,IDC_IBS_ASPECT_SPIN));
					os[3] = spinA->GetFVal();

					ReleaseISpinner(spinW);
					ReleaseISpinner(spinH);
					ReleaseISpinner(spinA);

					EndDialog(hWnd,1);
					break;

				case IDC_SIZE_VIEWPORT:
				case IDC_SIZE_CUSTOM:
					spinW = GetISpinner(GetDlgItem(hWnd,IDC_IBS_WIDTH_SPIN));
					spinH = GetISpinner(GetDlgItem(hWnd,IDC_IBS_HEIGHT_SPIN));
					spinA = GetISpinner(GetDlgItem(hWnd,IDC_IBS_ASPECT_SPIN));

					if ( IsDlgButtonChecked(hWnd, IDC_SIZE_CUSTOM) ) {
						spinW->Enable(TRUE);
						spinH->Enable(TRUE);
						spinA->Enable(TRUE);
						}
					else {
						spinW->Enable(FALSE);
						spinH->Enable(FALSE);
						spinA->Enable(FALSE);
						}

					ReleaseISpinner(spinW);
					ReleaseISpinner(spinH);
					ReleaseISpinner(spinA);
					break;

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK OutSizeDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK OutSizeDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	static int *os;
	switch (msg) {
		case WM_INITDIALOG: {
			os = (int*)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			
			ISpinnerControl *spin; 
			spin = SetupIntSpinner(hWnd,IDC_OBS_WIDTH_SPIN,IDC_OBS_WIDTH,0,8000,os[0]);
			spin = SetupIntSpinner(hWnd,IDC_OBS_HEIGHT_SPIN,IDC_OBS_HEIGHT,0,8000,os[1]);
			ReleaseISpinner(spin);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					ISpinnerControl *spin; 
					spin = GetISpinner(GetDlgItem(hWnd,IDC_OBS_WIDTH_SPIN));
					os[0] = spin->GetIVal();
					spin = GetISpinner(GetDlgItem(hWnd,IDC_OBS_HEIGHT_SPIN));
					os[1] = spin->GetIVal();
					ReleaseISpinner(spin);

					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

#define ITEMS_IN_LIST 5

#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK NumGroupsDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK NumGroupsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	switch (msg) {
		case WM_INITDIALOG: {
			CenterWindow(hWnd,GetParent(hWnd));
			
			ISpinnerControl *spin; 
			spin = SetupIntSpinner(hWnd,IDC_NUMGROUPS_SPIN,IDC_NUMGROUPS_EDIT,0,9999,(int)lParam);
			ReleaseISpinner(spin);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					ISpinnerControl *spin; 
					spin = GetISpinner(GetDlgItem(hWnd,IDC_NUMGROUPS_SPIN));
					EndDialog(hWnd,spin->GetIVal());
					ReleaseISpinner(spin);
					break;

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK OutputsDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK OutputsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	static OutputGroupsInfo *ogi;

	static short int menuGroup;
	static short int firstInList;
	BOOL display_buttons = FALSE;
	BOOL display_number = FALSE;

	switch (msg) {
		case WM_INITDIALOG: {
			ogi = (OutputGroupsInfo*)lParam;

			display_buttons = TRUE;
			display_number = TRUE;

			firstInList = 0;
			CenterWindow(hWnd,GetParent(hWnd));
			break;
			}

		case WM_VSCROLL:
			switch ( LOWORD(wParam) ) {
				case SB_LINEUP: 	firstInList--;					break;
				case SB_LINEDOWN:	firstInList++;					break;
				case SB_PAGEUP:		firstInList -= ITEMS_IN_LIST;	break;
				case SB_PAGEDOWN:	firstInList += ITEMS_IN_LIST;	break;
				
				case SB_THUMBPOSITION: 
				case SB_THUMBTRACK:
					firstInList = (short int)HIWORD(wParam);
					break;
				}

			if ( firstInList >= ogi->num_groups - ITEMS_IN_LIST ) 
				firstInList = ogi->num_groups - ITEMS_IN_LIST;
			if ( firstInList < 0 ) 
				firstInList = 0;

			SetScrollRange( GetDlgItem(hWnd,IDC_OUTDLG_SCROLL), SB_CTL, 0, ogi->num_groups - ITEMS_IN_LIST, FALSE );
			SetScrollPos( GetDlgItem(hWnd,IDC_OUTDLG_SCROLL), SB_CTL, firstInList, TRUE );

			display_buttons = TRUE;
			break;

		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_MATID_SPIN_0:
				case IDC_MATID_SPIN_1:
				case IDC_MATID_SPIN_2:
				case IDC_MATID_SPIN_3:
				case IDC_MATID_SPIN_4: {
					int group = firstInList + (LOWORD(wParam) - IDC_MATID_SPIN_0);
					ISpinnerControl *iSpin;
					iSpin = GetISpinner(GetDlgItem(hWnd,LOWORD(wParam)));
					ogi->mat_ids[group] = iSpin->GetIVal() - 1;
					ReleaseISpinner(iSpin);
					display_buttons = TRUE;
					}
					break;

				case IDC_UVW_SPIN_0:
				case IDC_UVW_SPIN_1:
				case IDC_UVW_SPIN_2:
				case IDC_UVW_SPIN_3:
				case IDC_UVW_SPIN_4: {
					int group = firstInList + (LOWORD(wParam) - IDC_UVW_SPIN_0);
					ISpinnerControl *iSpin;
					iSpin = GetISpinner(GetDlgItem(hWnd,LOWORD(wParam)));
					ogi->uvw[group] = iSpin->GetIVal();
					ReleaseISpinner(iSpin);
					display_buttons = TRUE;
					}
					break;
				}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDM_DEL_OUTFILE:
					if ( menuGroup < ogi->num_groups ) 
					{
						if ( DoesFileExist( ogi->bmp_names[ menuGroup ].c_str() ) )
						{
							TSTR question;
							question.printf( GetString(IDS_ASK_DELETEFILE) , ogi->bmp_names[ menuGroup ].c_str() );

							if ( MessageBox( hWnd , question , "Are you sure?", MB_YESNO) == IDYES ) 
							{
								DeleteFile( ogi->bmp_names[ menuGroup ].c_str() );
								display_buttons = TRUE;
							}
						}
					}
					break;

				case IDC_GROUP_DELETE_0:
				case IDC_GROUP_DELETE_1:
				case IDC_GROUP_DELETE_2:
				case IDC_GROUP_DELETE_3:
				case IDC_GROUP_DELETE_4: {
					int group = firstInList + (LOWORD(wParam) - IDC_GROUP_DELETE_0);
					TSTR question;
					question.printf( GetString(IDS_ASK_DELETEGROUP) , ogi->bmp_names[ group ] , ogi->mat_ids[ group ]+1 , ogi->uvw[ group ] );
					if ( MessageBox( hWnd , question , "Are you sure?", MB_YESNO) == IDYES ) {
						ogi->DeleteGroup( group );

						if ( firstInList == group )	firstInList--;
						if ( firstInList < 0 )		firstInList = 0;

						display_buttons = TRUE;
						display_number = TRUE;
						}
					}
					break;

				case IDC_SET_OUTBMP_0:
				case IDC_SET_OUTBMP_1:
				case IDC_SET_OUTBMP_2:
				case IDC_SET_OUTBMP_3:
				case IDC_SET_OUTBMP_4:
					switch (HIWORD(wParam)) {
						case BN_RIGHTCLICK: {
							menuGroup = firstInList + ( LOWORD(wParam) - IDC_SET_OUTBMP_0);
							HMENU mainMenu = GetSubMenu(LoadMenu(hInstance,MAKEINTRESOURCE(IDR_OUTBMPS_MENU)),0);

							// Show our popup
							POINT lpPt; GetCursorPos(&lpPt);
							TrackPopupMenu(mainMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
								lpPt.x, lpPt.y, 0, hWnd, NULL);	
							break;
							}
						default: {
							int group = firstInList + (LOWORD(wParam) - IDC_SET_OUTBMP_0);
							BitmapInfo bi;
							BOOL ok = TheManager->SelectFileOutput(	&bi, 
																	GetCOREInterface()->GetMAXHWnd(),
																	GetString(IDS_SET_OUTPUT_BMP));
							if (ok)
							{
								ogi->bmp_names[group] = bi.Name();
								display_buttons = TRUE;

								if ( !DoesFileExist( ogi->bmp_names[group].c_str() ) )
								{
									int outputSize[2];
							
									outputSize[0] = 400;
									outputSize[1] = 300;

									int res = DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_OUTSIZE), hWnd, OutSizeDlgProc, (LPARAM)outputSize );

									if ( res == 1 ) 
									{
										ogi->out_widths[group] = outputSize[0];
										ogi->out_heights[group] = outputSize[1];
										display_buttons = TRUE;
										}
									}
								}
							}
						}
					break;

				case IDC_SET_SIZE_0:
				case IDC_SET_SIZE_1:
				case IDC_SET_SIZE_2:
				case IDC_SET_SIZE_3:
				case IDC_SET_SIZE_4: {
					int group = firstInList + (LOWORD(wParam) - IDC_SET_SIZE_0);
					int outputSize[2];
			
					outputSize[0] = ogi->out_widths[group];
					outputSize[1] = ogi->out_heights[group];

					int res = DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_OUTSIZE), hWnd, OutSizeDlgProc, (LPARAM)outputSize );

					if ( res == 1 ) {
						ogi->out_widths[group] = outputSize[0];
						ogi->out_heights[group] = outputSize[1];
						display_buttons = TRUE;
						}
					}
					break;

				case IDC_GROUP_USE_0:
				case IDC_GROUP_USE_1:
				case IDC_GROUP_USE_2:
				case IDC_GROUP_USE_3:
				case IDC_GROUP_USE_4: {
					int group = firstInList + (LOWORD(wParam) - IDC_GROUP_USE_0);
					ogi->use[group] = IsDlgButtonChecked( hWnd , LOWORD(wParam) );
					}
					break;

				case IDC_SET_NUMGROUPS: 
					{
					int res = DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_NUM_GROUPS), hWnd, NumGroupsDlgProc, (LPARAM)ogi->num_groups );
					if (res>=0) {
						firstInList = 0;
						ogi->SetNumGroups( res );
						display_buttons = TRUE;
						display_number = TRUE;
						}
					}
					break;

				case IDOK:
					EndDialog(hWnd,1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}

	if ( display_buttons ) {
		for ( int i=0; i<ITEMS_IN_LIST; i++ ) {

			int group = firstInList + i;

			ISpinnerControl *iMatIDSpin;
			ISpinnerControl *iUVWSpin;
			ICustButton *iBmpBut;
			ICustButton *iSizeBut;

			iMatIDSpin = SetupIntSpinner(hWnd , IDC_MATID_SPIN_0 + i ,IDC_MATID_EDIT_0 + i , 1 , 9999 , 0 );
			iUVWSpin = SetupIntSpinner(hWnd , IDC_UVW_SPIN_0 + i ,IDC_UVW_EDIT_0 + i , 0 , 100 , 1 );
			iBmpBut = GetICustButton(GetDlgItem( hWnd , IDC_SET_OUTBMP_0 + i ));
			iSizeBut = GetICustButton(GetDlgItem( hWnd , IDC_SET_SIZE_0 + i ));

			if ( group < ogi->num_groups ) {
				ShowWindow( GetDlgItem(hWnd,IDC_GROUP_USE_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_MATID_EDIT_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_MATID_SPIN_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_UVW_SPIN_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_UVW_EDIT_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_SET_OUTBMP_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_SET_SIZE_0+i) , SW_SHOW );
				ShowWindow( GetDlgItem(hWnd,IDC_GROUP_DELETE_0+i) , SW_SHOW );

				CheckDlgButton( hWnd, IDC_GROUP_USE_0+i, ogi->use[group] );

				iMatIDSpin->Enable(TRUE);
				iMatIDSpin->SetValue(ogi->mat_ids[group]+1,FALSE);
				iUVWSpin->Enable(TRUE);
				iUVWSpin->SetValue(ogi->uvw[group],FALSE);
				iBmpBut->Enable(TRUE);

				if ( ogi->bmp_names[group].length() == 0 ) 
				{
					iBmpBut->SetRightClickNotify(FALSE);
					iBmpBut->SetText( GetString(IDS_SET_OUTPUT_BMP) );
					iBmpBut->SetTooltip( FALSE , _T("- not defined -") );
					iSizeBut->Enable(FALSE);
				}
				else 
				{
					TSTR fileName( ogi->bmp_names[group].c_str() );

					int pto = fileName.last('\\') + 1;
					fileName.remove(0,pto);

					iBmpBut->SetText( fileName );
					iBmpBut->SetTooltip( TRUE , TSTR( ogi->bmp_names[group].c_str() ) );

					if ( DoesFileExist( ogi->bmp_names[group].c_str() ) ) {
						iBmpBut->SetRightClickNotify(TRUE);
						iSizeBut->Enable(FALSE);
						}
					else {
						iSizeBut->Enable(TRUE);
						iBmpBut->SetRightClickNotify(FALSE);
						TSTR sizeBuf;
						sizeBuf.printf(_T("%d × %d") , ogi->out_widths[group] , ogi->out_heights[group] );
						iSizeBut->SetTooltip( TRUE , sizeBuf );
						}
					}
				}
			else {
				ShowWindow( GetDlgItem(hWnd,IDC_GROUP_USE_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_MATID_EDIT_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_MATID_SPIN_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_UVW_EDIT_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_UVW_SPIN_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_SET_OUTBMP_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_SET_SIZE_0+i) , SW_HIDE );
				ShowWindow( GetDlgItem(hWnd,IDC_GROUP_DELETE_0+i) , SW_HIDE );
				}
			
			ReleaseISpinner(iMatIDSpin);
			ReleaseISpinner(iUVWSpin);
			ReleaseICustButton(iBmpBut);
			ReleaseICustButton(iSizeBut);
			}
		}

	if ( display_number ) {
		int num = ogi->num_groups;
		TSTR buf;
		buf.printf( _T("%d") , num );
		SetWindowText(GetDlgItem(hWnd, IDC_TXT_NUMGROUPS), buf);
		}

	return TRUE;
	}


#if ( MAX_RELEASE >= 9000 )
static INT_PTR CALLBACK DefaultDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
#else
static BOOL CALLBACK DefaultDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
#endif
{
	switch (msg) {
		case WM_INITDIALOG:
			theNebbiUtility.Init(hWnd);
			break;

		case WM_DESTROY:
			theNebbiUtility.Destroy(hWnd);
			break;

		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_NEB_SAMP_SPIN:
					theNebbiUtility.SetInputData();
					break;
				case IDC_NEB_PREF_SPIN:
					theNebbiUtility.SetInputData();
					break;
				case IDC_NEB_ANGS_SPIN: {
					ISpinnerControl * as_spin = GetISpinner(GetDlgItem(hWnd,IDC_NEB_ANGS_SPIN));
					ISpinnerControl * ae_spin = GetISpinner(GetDlgItem(hWnd,IDC_NEB_ANGE_SPIN));

					float a_s = as_spin->GetFVal();
					float a_e = ae_spin->GetFVal();

					if ( a_s > a_e ) {
						ae_spin->SetValue( a_s , FALSE );
						}
					ReleaseISpinner(as_spin);
					ReleaseISpinner(ae_spin);
					}
					theNebbiUtility.SetInputData();
					break;
				case IDC_NEB_ANGE_SPIN: {
					ISpinnerControl * as_spin = GetISpinner(GetDlgItem(hWnd,IDC_NEB_ANGS_SPIN));
					ISpinnerControl * ae_spin = GetISpinner(GetDlgItem(hWnd,IDC_NEB_ANGE_SPIN));

					float a_s = as_spin->GetFVal();
					float a_e = ae_spin->GetFVal();

					if ( a_e < a_s ) {
						as_spin->SetValue( a_e , FALSE );
						}
					ReleaseISpinner(as_spin);
					ReleaseISpinner(ae_spin);
					}
					theNebbiUtility.SetInputData();
					break;
				}
			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLOSE:
					theNebbiUtility.iu->CloseUtility();
					break;

				case IDC_NEBBI_ABOUT: 
					DialogBoxParam(	hInstance, MAKEINTRESOURCE(IDD_ABOUT), theNebbiUtility.ip->GetMAXHWnd(), AboutDlgProc, 0);
				break;

				case IDC_NEBBI_HELP: 
					ShellExecute(NULL, "open", "http://www.mankua.com/powerstamper.php", NULL, NULL, SW_SHOWNORMAL);
				break;

				case IDC_LOG:
//					MessageBox(hWnd,theNebbiUtility.info_message,_T("PowerStamper Log Info"),MB_OK);
					DialogBoxParam(	hInstance,
									MAKEINTRESOURCE(IDD_LOG_DLG),
									theNebbiUtility.ip->GetMAXHWnd(),
									LogDlgProc,
									(LPARAM)theNebbiUtility.info_message.data());
					break;

				case IDC_PRE_ALPHA:
					theNebbiUtility.SetInputData();
					break;

				case IDC_SET_INBMP:
					theNebbiUtility.SetInputBMP();
					break;
				
				case IDC_SIZE_IN:
					theNebbiUtility.SetSizeIn();
					break;

				case IDC_RENDER_IN:
					theNebbiUtility.RenderInputBMP();
					break;

				case IDC_SET_OUTBMP:
					theNebbiUtility.SetOutBmpsByMatID();
					break;

				case IDC_PAINT_OUT:
					theNebbiUtility.PaintOutputBMP();
					break;

				case IDC_BANNER:
					ShellExecute(NULL, "open", "http://www.crackart.org", NULL, NULL, SW_SHOWNORMAL);
					break;
			}
			break;


		default:
			return FALSE;
	}
	return TRUE;
}



/*===========================================================================*\
 |  Utility implimentations
\*===========================================================================*/

NebbiUtility::NebbiUtility()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;

	info_message = _T("Nothing Stamped Yet");
}

NebbiUtility::~NebbiUtility() 
{
}

void NebbiUtility::BeginEditParams(Interface *ip,IUtil *iu) {
	this->ip = ip;
	this->iu = iu;

	hPanel = ip->AddRollupPage( hInstance, MAKEINTRESOURCE(IDD_NEBBIUTIL), DefaultDlgProc, GetString(IDS_PARAMETERS), 0 );
}
	
void NebbiUtility::EndEditParams(Interface *ip,IUtil *iu) 
{
	if ( hPanel )
		ip->DeleteRollupPage(hPanel);
	hPanel = NULL;

	this->iu = NULL;
	this->ip = NULL;
	}

void NebbiUtility::Init(HWND hWnd)
{
	hPanel = hWnd;

	LoadImages();
	// About Button
	ICustButton *iTmp;

	iTmp = GetICustButton(GetDlgItem(hPanel,IDC_NEBBI_ABOUT));
	iTmp->SetImage(h_about_image, 0, 0, 0, 0, 16, 16);
	iTmp->SetTooltip(TRUE,_T("About Power Stamper"));
	ReleaseICustButton(iTmp);

	iTmp = GetICustButton(GetDlgItem(hPanel,IDC_NEBBI_HELP));
	iTmp->SetImage(h_help_image, 0, 0, 0, 0, 16, 16);
	iTmp->SetTooltip(TRUE,_T("Help"));
	ReleaseICustButton(iTmp);

	SelectionSetChanged(ip,iu);

	ShowInputData();
	ShowInputBMPName();
	}

void NebbiUtility::Destroy(HWND hWnd) {
	hPanel = NULL;
	}

void NebbiUtility::SelectionSetChanged(Interface *ip,IUtil *iu) {
	if ( !hPanel ) return;

	if (ip->GetSelNodeCount()==1) {
		INode * node = ip->GetSelNode( 0 );
		ObjectState os = node->EvalWorldState(ip->GetTime());
		if ( IsGoodObjectToStamp( os ) )
			SetDlgItemText(hPanel,IDC_SEL_NAME,ip->GetSelNode(0)->GetName());
		else
			SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_NONESEL));
		}
	else if (ip->GetSelNodeCount())
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_MULTISEL));
	else
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_NONESEL));

	ShowInputData();
	ShowInputBMPName();
	}

BOOL NebbiUtility::IsGoodObjectToStamp( ObjectState &os ) 
{
	return os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID && os.obj->IsRenderable();
}

void NebbiUtility::PutInputData( InputData &in_data, std::string &in_name )
{
	int size = sizeof( InputData );

	int numnodes = ip->GetSelNodeCount();

	for ( int i_node=0; i_node<numnodes; i_node++ )
	{
		INode * node = ip->GetSelNode( i_node );
		ObjectState os = node->EvalWorldState(ip->GetTime());

		if ( IsGoodObjectToStamp(os) )
		{
			int * nebbi_version = (int*)MAX_MALLOC(sizeof(int));
			*nebbi_version = NEBBI_CUR_VERSION;

			node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_NEBBI_VERSION);
			node->AddAppDataChunk(		NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_NEBBI_VERSION, (DWORD)sizeof(int), nebbi_version );

			if ( in_data.input_width > 0 && in_data.input_height ) 
			{
				InputData * app_data = (InputData*)MAX_MALLOC( sizeof(InputData) );

				app_data->custom_size  = in_data.custom_size;
				app_data->input_width  = in_data.input_width;
				app_data->input_height = in_data.input_height;
				app_data->input_aspect = in_data.input_aspect;
				app_data->angle_start  = in_data.angle_start;
				app_data->angle_end    = in_data.angle_end;
				app_data->antialiasing = in_data.antialiasing;
				app_data->prefiltering = in_data.prefiltering;
				app_data->pre_alpha    = in_data.pre_alpha;

				node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_INPUT_DATA);
				node->AddAppDataChunk(		NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_INPUT_DATA, (DWORD)size, app_data );
			}

			if ( in_name.length() ) 
			{
				int len = in_name.length() + 1;
				TCHAR * app_name = (TCHAR*)MAX_MALLOC(len);
				_tcscpy( app_name, in_name.c_str() );

				node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_INPUT_NAME);
				node->AddAppDataChunk(		NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_INPUT_NAME, (DWORD)len, app_name );
			}
		}
	}
}

void NebbiUtility::GetInputData( InputData &in_data ) 
{
	InputData * app_data;

	int numnodes = ip->GetSelNodeCount();

	for ( int i_node=0; i_node<numnodes; i_node++ ) 
	{
		INode * node = ip->GetSelNode( i_node );
		ObjectState os = node->EvalWorldState(ip->GetTime());

		if ( IsGoodObjectToStamp( os ) )
		{
			AppDataChunk *ad;

			ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,APP_DATA_INPUT_DATA);

			if ( ad != NULL ) 
			{
				app_data = (InputData*)ad->data;
				i_node = numnodes + 1;

				in_data.custom_size   = app_data->custom_size ;
				in_data.input_width   = app_data->input_width ;
				in_data.input_height  = app_data->input_height;
				in_data.input_aspect  = app_data->input_aspect;
				in_data.angle_start   = app_data->angle_start ;
				in_data.angle_end     = app_data->angle_end   ;
				in_data.dist_start    = app_data->dist_start  ;
				in_data.dist_end      = app_data->dist_end    ;
				in_data.antialiasing  = app_data->antialiasing;
				in_data.prefiltering  = app_data->prefiltering;
				in_data.pre_alpha     = app_data->pre_alpha   ;
			}
		}
	}
}

void NebbiUtility::GetInputName( std::string &in_name ) 
{
	int numnodes = ip->GetSelNodeCount();

	for ( int i_node = 0; i_node < numnodes; i_node++ )
	{
		INode * node = ip->GetSelNode( i_node );
		ObjectState os = node->EvalWorldState(ip->GetTime());

		if ( IsGoodObjectToStamp( os ) )
		{
			AppDataChunk *ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,APP_DATA_INPUT_NAME);

			if ( ad != NULL )
			{
				in_name = (TCHAR*)ad->data;

				return;
			}
		}
	}
}

int NebbiUtility::GetNumOutGroups() {
	int * num_groups = NULL;
	INode * node = ip->GetSelNode( 0 );

	ObjectState os = node->EvalWorldState(ip->GetTime());
	if ( IsGoodObjectToStamp( os ) ) {
		AppDataChunk *ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,APP_DATA_OUTPUT_NUMGROUPS);
		if ( ad != NULL ) {
			num_groups = (int*)ad->data;
			}
		}

	if ( num_groups )	{
		return *num_groups;
		}
	else				return 0;
	}

void NebbiUtility::PutNumOutGroups( int n_g )
{
	int * num_groups = (int*)MAX_MALLOC(sizeof(int));
	*num_groups = n_g;
	INode * node = ip->GetSelNode( 0 );

	int old_num_groups = 0;

	ObjectState os = node->EvalWorldState(ip->GetTime());

	if ( IsGoodObjectToStamp( os ) )
	{
		int * entero;
		AppDataChunk *ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,APP_DATA_OUTPUT_NUMGROUPS);

		if ( ad != NULL ) 
		{
			entero = (int*)ad->data;
			old_num_groups = *entero;
		}

		node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_OUTPUT_NUMGROUPS);
		node->AddAppDataChunk(		NEBBI_CLASSID, UTILITY_CLASS_ID, APP_DATA_OUTPUT_NUMGROUPS, (DWORD)sizeof(int), num_groups);
	
		for ( int i=0; i<old_num_groups; i++ ) 
		{
			int data_slot = APP_DATA_OUTGROUP_DATA + i*2;
			int name_slot = APP_DATA_OUTGROUP_NAME + i*2;

			node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, data_slot);
			node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, name_slot);
		}
	}
}

void NebbiUtility::PutOutputData( int group_id, OutputData &out_data, std::string &out_name ) 
{
	int size = sizeof( OutputData );
	int data_slot = APP_DATA_OUTGROUP_DATA + group_id*2;
	int name_slot = APP_DATA_OUTGROUP_NAME + group_id*2;

	INode * node = ip->GetSelNode( 0 );
	ObjectState os = node->EvalWorldState(ip->GetTime());

	if ( IsGoodObjectToStamp( os ) )
	{
		OutputData * app_data = (OutputData*)MAX_MALLOC( sizeof(OutputData) );

		app_data->output_width  = out_data.output_width;
		app_data->output_height = out_data.output_height;
		app_data->output_uvw    = out_data.output_uvw;
		app_data->output_matid  = out_data.output_matid;
		app_data->output_use    = out_data.output_use;

		node->RemoveAppDataChunk(	NEBBI_CLASSID, UTILITY_CLASS_ID, data_slot);
		node->AddAppDataChunk(		NEBBI_CLASSID, UTILITY_CLASS_ID, data_slot, (DWORD)size, app_data );

		if ( out_name.length() )
		{
			int len = out_name.length() + 1;
			TCHAR * app_name = (TCHAR*)MAX_MALLOC(len);
			_tcscpy( app_name, out_name.c_str() );

			node->RemoveAppDataChunk( NEBBI_CLASSID, UTILITY_CLASS_ID, name_slot );
			node->AddAppDataChunk(	  NEBBI_CLASSID, UTILITY_CLASS_ID, name_slot, (DWORD)len, app_name );
		}
	}
}

void NebbiUtility::GetOutputName( int group_id, std::string &out_name )
{
	int name_slot = APP_DATA_OUTGROUP_NAME + group_id*2;

	INode * node = ip->GetSelNode( 0 );
	ObjectState os = node->EvalWorldState(ip->GetTime());

	if ( IsGoodObjectToStamp( os ) )
	{
		AppDataChunk *ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,name_slot);

		if ( ad != NULL )
		{
			out_name = (TCHAR*)ad->data;
		}
	}
}

void NebbiUtility::GetOutputData( int group_id, OutputData &data )
{
	OutputData * out_data = NULL;

	int data_slot = APP_DATA_OUTGROUP_DATA + group_id*2;

	INode * node = ip->GetSelNode( 0 );
	ObjectState os = node->EvalWorldState(ip->GetTime());

	if ( IsGoodObjectToStamp( os ) )
	{
		AppDataChunk *ad;

		ad = node->GetAppDataChunk( NEBBI_CLASSID,UTILITY_CLASS_ID, data_slot );

		if ( ad != NULL )
		{
			OutputData * app_data = (OutputData*)ad->data;

			data.output_use    = app_data->output_use;
			data.output_matid  = app_data->output_matid;
			data.output_uvw    = app_data->output_uvw;
			data.output_width  = app_data->output_width;
			data.output_height = app_data->output_height;
		}
	}
}

int NebbiUtility::GetNumOutGroups( INode * node ) 
{
	int * num_groups = NULL;

	ObjectState os = node->EvalWorldState(ip->GetTime());
	if ( IsGoodObjectToStamp( os ) ) {
		AppDataChunk *ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,APP_DATA_OUTPUT_NUMGROUPS);
		if ( ad != NULL ) {
			num_groups = (int*)ad->data;
			}
		}

	if ( num_groups )	{
		return *num_groups;
		}
	else				return 0;
	}

void NebbiUtility::GetOutputName( INode * node, int group_id, std::string &out_name ) 
{
	int name_slot = APP_DATA_OUTGROUP_NAME + group_id*2;

	ObjectState os = node->EvalWorldState(ip->GetTime());
	
	if ( IsGoodObjectToStamp( os ) ) 
	{
		AppDataChunk *ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,name_slot);
	
		if ( ad != NULL )
		{
			out_name = (TCHAR*)ad->data;
		}
	}
}

void NebbiUtility::GetOutputData( INode * node, int group_id, OutputData &out_data ) 
{
	int data_slot = APP_DATA_OUTGROUP_DATA + group_id * 2;

	ObjectState os = node->EvalWorldState(ip->GetTime());
	
	if ( IsGoodObjectToStamp( os ) ) 
	{
		AppDataChunk *ad;

		ad = node->GetAppDataChunk(NEBBI_CLASSID,UTILITY_CLASS_ID,data_slot);
		
		if ( ad != NULL )
		{
			OutputData * app_data = (OutputData*)ad->data;
			
			out_data.output_width  = app_data->output_width; 
			out_data.output_height = app_data->output_height; 
			out_data.output_use    = app_data->output_use; 
			out_data.output_uvw    = app_data->output_uvw; 
			out_data.output_matid  = app_data->output_matid; 
		}
	}
}

void NebbiUtility::ShowInputData() 
{
	InputData in_data;
	GetInputData( in_data );

	ISpinnerControl *spin = NULL;

	spin = SetupIntSpinner(hPanel,IDC_NEB_SAMP_SPIN,IDC_NEB_SAMP_EDIT,1,8,in_data.antialiasing);
	ReleaseISpinner(spin);
	
	spin = SetupIntSpinner(hPanel,IDC_NEB_PREF_SPIN,IDC_NEB_PREF_EDIT,0,25,in_data.prefiltering);
	ReleaseISpinner(spin);

	spin = SetupFloatSpinner(hPanel,IDC_NEB_ANGS_SPIN,IDC_NEB_ANGS_EDIT,0,180.0f,in_data.angle_start,1.0f);
	ReleaseISpinner(spin);
	
	spin = SetupFloatSpinner(hPanel,IDC_NEB_ANGE_SPIN,IDC_NEB_ANGE_EDIT,0,180.0f,in_data.angle_end,1.0f);
	ReleaseISpinner(spin);

	BOOL pre_alpha = in_data.pre_alpha;
	CheckDlgButton( hPanel, IDC_PRE_ALPHA, pre_alpha );
}

void NebbiUtility::ShowInputBMPName()
{
	std::string in_name;
	InputData   in_data;

	GetInputName( in_name );
	GetInputData( in_data );

	if ( in_name.length() )
	{
		BitmapInfo bi;
		bi.SetName( in_name.c_str() );
		TSTR fileName = TSTR( bi.Filename() );
		TSTR pathName = TSTR( bi.Name() );

		ICustButton *iBut;
		iBut = GetICustButton(GetDlgItem(hPanel,IDC_SET_INBMP));
		iBut->SetText(fileName);
		iBut->SetTooltip(TRUE,pathName);

		ReleaseICustButton(iBut);
	}
	else 
	{
		ICustButton *iBut;
		iBut = GetICustButton(GetDlgItem(hPanel,IDC_SET_INBMP));
		iBut->SetText(_T("Stamping Image"));
		iBut->SetTooltip(TRUE,_T("Stamping Image"));
		ReleaseICustButton(iBut);
	}
}

void NebbiUtility::DlgInputSize( float * inputSize)
{
	int res = DialogBoxParam( hInstance, MAKEINTRESOURCE(IDD_INSIZE), ip->GetMAXHWnd(), InSizeDlgProc, (LPARAM)inputSize);
}

void NebbiUtility::SetInputBMP() 
{
	BitmapInfo inBI;
	BOOL ok = TheManager->SelectFileOutput(	&inBI, GetCOREInterface()->GetMAXHWnd(), GetString(IDS_SET_INPUT_BMP));

	if ( !ok )
	{
		return;
	}

	int in_custom_size;
	int in_width;
	int in_height;
	float in_aspect;

	if ( !DoesFileExist(inBI.Name()) ) 
	{
		float inputSize[4];
		inputSize[0] = 1.0f;
		inputSize[1] = 400.0f;
		inputSize[2] = 300.0f;
		inputSize[3] = 1.0f;

		DlgInputSize(inputSize);

		in_custom_size	= (int)inputSize[0];
		in_width		= (int)inputSize[1];
		in_height		= (int)inputSize[2];
		in_aspect		= inputSize[3];
	}
	else 
	{
		TheManager->GetImageInfo(&inBI, inBI.Name());
		in_custom_size = 1;
		in_width = inBI.Width();
		in_height = inBI.Height();
		in_aspect = inBI.Aspect();
	}

	InputData in_data;

	in_data.custom_size = in_custom_size;
	in_data.input_width = in_width;
	in_data.input_height = in_height;
	in_data.input_aspect = in_aspect;

	GetUIData( in_data );

	std::string in_name = inBI.Name();

	PutInputData( in_data, in_name );
	
	ShowInputBMPName();
}

void NebbiUtility::GetUIData( InputData &in_data )
{
	ISpinnerControl * spin = NULL;

	spin = GetISpinner(GetDlgItem(hPanel,IDC_NEB_ANGS_SPIN));
	in_data.angle_start = spin->GetFVal();

	spin = GetISpinner(GetDlgItem(hPanel,IDC_NEB_ANGE_SPIN));
	in_data.angle_end = spin->GetFVal();

	spin = GetISpinner(GetDlgItem(hPanel,IDC_NEB_SAMP_SPIN));
	in_data.antialiasing = spin->GetIVal();

	spin = GetISpinner(GetDlgItem(hPanel,IDC_NEB_PREF_SPIN));
	in_data.prefiltering = spin->GetIVal();

	BOOL pre_alpha = IsDlgButtonChecked(hPanel, IDC_PRE_ALPHA);
	in_data.pre_alpha = pre_alpha;

	ReleaseISpinner(spin);
}

void NebbiUtility::SetInputData()
{
	std::string in_name;
	InputData   in_data;

	GetInputName( in_name );
	GetInputData( in_data );

	InputData new_in_data;

	if ( in_data.input_width != 0 && in_data.input_height != 0 )
	{
		new_in_data.custom_size  = in_data.custom_size;
		new_in_data.input_width  = in_data.input_width;
		new_in_data.input_height = in_data.input_height;
		new_in_data.input_aspect = in_data.input_aspect;
	}
	else 
	{
		new_in_data.custom_size  = 1;
		new_in_data.input_width  = 400;
		new_in_data.input_height = 300;
		new_in_data.input_aspect = 1.0f;
	}

	GetUIData( new_in_data );

	PutInputData( new_in_data, in_name );
}

void NebbiUtility::SetSizeIn() 
{
	InputData   in_data;
	std::string in_name;

	GetInputData( in_data );
	GetInputName( in_name );

	if ( in_name.length() == 0 ) 
	{
		SetInputBMP();
	}

	float inputSize[4];

	inputSize[0] = (float)in_data.custom_size;
	inputSize[1] = (float)in_data.input_width;
	inputSize[2] = (float)in_data.input_height;
	inputSize[3] =        in_data.input_aspect;

	DlgInputSize( inputSize );

	GetUIData( in_data );

	PutInputData( in_data, in_name );
}	

void NebbiUtility::RenderInputBMP()
{
	std::string in_name;
	InputData   in_data;

	GetInputName( in_name );
	GetInputData( in_data );

	if ( in_data.input_height == 0 || in_data.input_width == 0 )
	{
		SetInputBMP();

		GetInputName( in_name );
		GetInputData( in_data );
	}

	int w,h;
	float aspect;

	ViewExp *view = ip->GetActiveViewport();

	if ( !in_data.custom_size )
	{
		GraphicsWindow * gw = view->getGW();
		w = gw->getWinSizeX();
		h = gw->getWinSizeY();
		aspect = 1.0f;
	}
	else 
	{
		w = in_data.input_width;
		h = in_data.input_height;
		aspect = in_data.input_aspect;
	}

	if ( aspect < 0.001f )
	{
		aspect = 0.001f;
	}

	if (inBM) 
	{
		inBM->UnDisplay();
		inBM->DeleteThis();
		inBM = NULL;
	}
	
	BitmapInfo bi;
	bi.SetName( in_name.c_str() );
	bi.SetWidth(w);
	bi.SetHeight(h);
	bi.SetType(BMM_TRUE_64);
	bi.SetFlags(MAP_NOFLAGS);
	bi.SetAspect(aspect);
	inBM = TheManager->Create(&bi);
	inBM->Display(_T("Stamping Image Render "));

	ip->OpenCurRenderer(NULL,view);
	ip->CurRendererRenderFrame(ip->GetTime(),inBM);
	ip->CloseCurRenderer();	

	inBM->OpenOutput(&bi);
	inBM->Write(&bi);
	inBM->Close(&bi);

	ip->ReleaseViewport(view);

	// Let's put this same info to all our objects!

	PutInputData( in_data , in_name );
}

static int __cdecl CompareMatIDs(const int *k1, const int *k2)
{
	if ( *k1 < *k2 ) return -1;
	if ( *k1 > *k2 ) return 1;
	return 0;
}

void NebbiUtility::SetOutBmpsByMatID() {
	if ( ip->GetSelNodeCount() > 1 ) {
		WarningMessage( WARN_MULTIPLE_SEL );
		return;
		}
	else if ( ip->GetSelNodeCount() == 0 ) { 
		WarningMessage( WARN_NONE_SEL );
		return;
		}

	TimeValue t = ip->GetTime();

	INode * node = ip->GetSelNode( 0 );
	int num_groups = GetNumOutGroups();

	OutputGroupsInfo outInfo(0);

	if ( num_groups == 0 ) {
		ObjectState os = node->EvalWorldState(ip->GetTime());
		if ( !IsGoodObjectToStamp( os ) ) {
			WarningMessage( WARN_INVALID_OBJ );
			return;
			}

		BOOL needDel;
		NullView nullView;
		Mesh *mesh = ((GeomObject*)os.obj)->GetRenderMesh(t,ip->GetSelNode(0),nullView,needDel);
		
		Tab <int> mat_ids;
		mat_ids.SetCount(0);
		for ( int nf=0; nf<mesh->numFaces; nf++ ) {
			int mat_id = mesh->faces[nf].getMatID();
			int exists = 0;
			for ( int nm=0; nm<mat_ids.Count(); nm++ ) {
				if ( mat_id == mat_ids[nm] ) {
					exists = 1;
					nm = mat_ids.Count() + 1;
					}
				}
			if ( !exists ) 
				mat_ids.Append(1,&mat_id,3);
			}

		mat_ids.Shrink();
		mat_ids.Sort((CompareFnc)CompareMatIDs); 

		outInfo.SetNumGroups( mat_ids.Count() );
		for ( int n_m=0; n_m<mat_ids.Count(); n_m++ ) {
			outInfo.mat_ids[n_m] = mat_ids[n_m];
			}

		if (needDel) delete mesh;
	}
	else 
	{
		outInfo.SetNumGroups( num_groups );

		for ( int n_g=0; n_g<num_groups; n_g++ )
		{
			std::string out_name;
			OutputData out_data;

			GetOutputName( n_g, out_name );
			GetOutputData( n_g, out_data );
			
			if ( out_name.length() )
			{
				outInfo.bmp_names[n_g] = out_name;
			}

			if ( out_data.output_width != 0 && out_data.output_height != 0 )
			{
				outInfo.use[n_g]         = out_data.output_use;
				outInfo.mat_ids[n_g]     = out_data.output_matid;
				outInfo.uvw[n_g]         = out_data.output_uvw;
				outInfo.out_widths[n_g]  = out_data.output_width;
				outInfo.out_heights[n_g] = out_data.output_height;
			}
		}
	}

	int res = DialogBoxParam(	hInstance, MAKEINTRESOURCE(IDD_OUTBMP_BYMATID), ip->GetMAXHWnd(), OutputsDlgProc, (LPARAM)&outInfo);

	if ( res == 1 )
	{
		PutNumOutGroups( outInfo.num_groups );

		for ( int n_g=0; n_g<outInfo.num_groups; n_g++ )
		{
			std::string out_name = outInfo.bmp_names[n_g];

			OutputData out_data;

			out_data.output_use    = outInfo.use[n_g];
			out_data.output_matid  = outInfo.mat_ids[n_g];
			out_data.output_uvw    = outInfo.uvw[n_g];
			out_data.output_width  = outInfo.out_widths[n_g];
			out_data.output_height = outInfo.out_heights[n_g];

			PutOutputData( n_g, out_data, out_name );
		}
	}
}

class ThreadData {
	public:
		NebbiUtility* util;
		NebbiRendContext *rgc;
		TheBmpCache * bmC;
		Bitmap * inputBM;

		int hStart,hEnd;
	};
void RenderThread();
void RenderThread(void *data){
	((ThreadData*)(data))->util->PaintZone(	((ThreadData*)(data))->rgc,
											((ThreadData*)(data))->bmC,
											((ThreadData*)(data))->hStart,
											((ThreadData*)(data))->hEnd,
											((ThreadData*)(data))->inputBM );
	}

void NebbiUtility::PaintZone(NebbiRendContext *rgc, TheBmpCache * bmC, int hStart, int hEnd, Bitmap * inputBM) {
	int outw = rgc->width;
	int outh = rgc->height;
	int aa = rgc->antialiasing;

	NebbiShadeContext gsc( rgc );

	int px,py,aax,aay,fct;
	for (py=hStart; py<hEnd; py++) {
		for (px=0; px<outw; px++) {
			int zx = px/rgc->azw;
			int zy = py/rgc->azh;
			int zona = zy * rgc->nzw + zx;

			int numhits = 0;
			AColor pixCol(0,0,0,0);

			for (aax=0; aax<aa; aax++)
				for (aay=0; aay<aa; aay++) {

					// Primera evaluacion. Centro del cuadrado
					float step = 1/float(aa);
					float uf =		  (float(px)+step/2.0f+step*aax) / float(outw);
					float vf = 1.0f - (float(py)+step/2.0f+step*aay) / float(outh);

					for (fct=0;fct<rgc->zones[zona]->faces.Count();fct++) {
						int fFace = rgc->zones[zona]->faces[fct];
						int face = rgc->fFaces[fFace].index;
						Point3 bc(0,0,0);
						if ( rgc->fFaces[fFace].HitTest( Point2( uf,vf ), bc ) ) {
							gsc.SetSC(bc,fFace,face);
							AColor col = EvalColor(gsc,inputBM);
							pixCol += col;
							numhits++;
							}
						} // for(fct 0->faces.Count
					} // for(aay=0->aa
			if (numhits) {
				pixCol /= float(numhits);
				pixCol.ClampMinMax();
				bmC->SetPixel(pixCol.r,pixCol.g,pixCol.b,pixCol.a,px,py);
				}
			} // for (px
		} // for (py
	}

void NebbiUtility::PaintOutputBMP() {
	SYSTEM_INFO info;
	GetSystemInfo( &info );  // address of system information 
    int numProcessors = info.dwNumberOfProcessors; 
//	numProcessors = 1; // PILAS... UN SOLO PROCESADOR

	info_message = _T("");

	if (ip->GetSelNodeCount()==0)
		return;

	TimeValue t = ip->GetTime();

	std::string in_name;
	InputData   in_data;

	GetInputName( in_name );
	GetInputData( in_data );

	if ( in_name.length() == 0 )
	{
		WarningMessage( WARN_NO_INNAME );
		return;
	}

	if ( in_data.input_width == 0 || in_data.input_height == 0 )
	{
		WarningMessage( WARN_NO_INDATA );
		return;
	}

	int custom_size = in_data.custom_size;
	int in_width = in_data.input_width;
	int in_height = in_data.input_height;

	float a_s = in_data.angle_start;
	float a_e = in_data.angle_end;

	int aa = in_data.antialiasing;
	int p_f = in_data.prefiltering;

	BOOL p_a = in_data.pre_alpha;

	BitmapInfo inBI;
	Bitmap *inputBM = NULL;

	if ( ! DoesFileExist( in_name.c_str() ) )
	{
		WarningMessage( WARN_NO_INIMAGE );
		return;
	}

	inBI.SetName( in_name.c_str() );
		
	inputBM = TheManager->Load(&inBI);
	inputBM->SetFilter(BMM_FILTER_PYRAMID);

	int rendered = 0;

	int num_nodes = ip->GetSelNodeCount();
	for ( int i_node=0; i_node<num_nodes; i_node++ ) {

		INode * node = ip->GetSelNode( i_node );

		info_message.printf( _T("%sStamping %s\n") , info_message , node->GetName() );

		ObjectState os = node->EvalWorldState(ip->GetTime());
		
		if( IsGoodObjectToStamp( os ) ) 
		{
			BOOL needDel;
			NullView nullView;
			Mesh *mesh = ((GeomObject*)os.obj)->GetRenderMesh(t,node,nullView,needDel);
			mesh->buildRenderNormals();
			
			if ( mesh ) 
			{

				// Here we start multi-bitmap for this node
				int num_groups = GetNumOutGroups( node );
				
				for ( int i_group=0; i_group<num_groups; i_group++ ) 
				{
					// Here we start the output stuff
						
					BitmapInfo outBI;

					std::string out_name;
					OutputData out_data;
					
					GetOutputName( node, i_group, out_name );
					GetOutputData( node, i_group, out_data );

					info_message.printf( _T("%s Material ID : %d\n") , info_message , out_data.output_matid );
					info_message.printf( _T("%s UVW Channel : %d\n") , info_message , out_data.output_uvw );

					if ( out_data.output_width == 0 || out_data.output_height == 0 || out_name.length() == 0 || !out_data.output_use )
					{
						if ( out_data.output_width == 0 || out_data.output_height == 0 )
						{
							info_message.printf( _T("%s - Output info is not set for this object.\n") , info_message );
						}
						if ( out_name.length() == 0 )
						{
							info_message.printf( _T("%s - No Bitmap Name set for this object.\n") , info_message );
						}
						if ( !out_data.output_use )
						{
							info_message.printf( _T("%s - Deactivated Group\n") , info_message );
						}
					}
					else 
					{
						if ( DoesFileExist( out_name.c_str() ) )
						{
							TheManager->GetImageInfo( &outBI, out_name.c_str() );
						}
						else
						{
							outBI.SetName( out_name.c_str() );
							outBI.SetWidth( out_data.output_width );
							outBI.SetHeight( out_data.output_height );
							outBI.SetType(BMM_TRUE_64);
							outBI.SetFlags(MAP_HAS_ALPHA);
							outBI.SetAspect(1.0f);
						}

						int outw = outBI.Width();
						int outh = outBI.Height();

						int uvw_ch = out_data.output_uvw;

						if ( mesh->mapSupport( uvw_ch ) ) 
						{
							SetPercent(0);
							
							TheBmpCache * bmCache = new TheBmpCache(outw,outh);
							NebbiRendContext *rgc  = new NebbiRendContext(mesh,uvw_ch,custom_size,in_width,in_height,outw,outh,aa,p_f,a_s,a_e,p_a,out_data.output_matid,t);

							ApplyCamMap(rgc,node);

							if ( BuildZones(rgc,mesh) ) {

								rendered++;

								ThreadData TData[8];
								int ct =0,numLines;
								numLines = outh/numProcessors;
								static HANDLE ThreadHandles[8];

								int numLinesPerScan = 4;
								int numScans = outh / numLinesPerScan;
								if ( outh % numLinesPerScan != 0 ) numScans++;

								BOOL canceled = FALSE;

								for (int nS=0; nS<numScans;  ) {
									for (int nTh=0; nTh<numProcessors; nTh++) {
										if (nS < numScans) {
											if (rgc) {
												TData[nTh].util = this;
												TData[nTh].rgc = rgc;
												TData[nTh].bmC = bmCache;
												TData[nTh].inputBM = inputBM;

												TData[nTh].hStart = nS * 4;
												int end = ( (nS + 1) * 4 );// - 1;
												if (end >= outh) end = outh - 1;
												TData[nTh].hEnd = end;

												BOOL cancel = GetAsyncKeyState(VK_ESCAPE)<0;
												if (cancel) {
													if ( MessageBox(hPanel , GetString(IDS_CANCEL_PAINT), "Cancel Painting", MB_YESNO) == IDYES ) {
														nS = numScans + 1;
														nTh = numProcessors + 1;
														canceled = TRUE;
														}
													}
												else {
													ThreadHandles[nTh] = (HANDLE) _beginthread(RenderThread,0,(void *) &TData[nTh]);
													nS++;
													}
												}
											}
										}
									int ThredErr = WaitForMultipleObjects(numProcessors,ThreadHandles,TRUE,INFINITE);
									int percent = 20 + int ( 60.0f * float(nS)/float(numScans) );
									SetPercent(percent);
									} // for ( nS = 0 -> numScans

								if (!canceled) {
									SaveRenderedMap(rgc,bmCache,&outBI,node );
									} // !canceled

								delete rgc;

								delete bmCache;
								bmCache = NULL;
								} // Build Zone
							} // UVW Support t
						else {
							info_message.printf( _T("%s - No UVW channel in this object.\n") , info_message );
							}
						} // else de Name or Use group
					// Here we finish the Bitmap Output Painting
					} // for ( the_group<num_groups
				} // if ( mesh ) 

			if (needDel) 
				delete mesh;

			} // if ( GoodObjectToStamp
		else {
			info_message.printf( _T("%s - Can't stamp this object\n") , info_message );
			}
		} // for ( i_node<num_nodes

	if (inputBM) inputBM->DeleteThis();

	if ( rendered == 0 )
		WarningMessage( WARN_NOTHING_REND );
	else
		ip->ForceCompleteRedraw();
	}

int NebbiUtility::BuildZones(NebbiRendContext *rgc, Mesh *theMesh) {
	int percent;

	for (int i=0; i<rgc->zones.Count(); i++) {
		if ( rgc->zones[i] ) {
			delete rgc->zones[i];
			rgc->zones[i] = NULL;
			}
		}

	int w = rgc->width;
	int h = rgc->height;
	int aa = rgc->antialiasing;


	TVFace *outtvFace = theMesh->mapFaces(rgc->uvw);
	Point3 *outtVerts = theMesh->mapVerts(rgc->uvw);

	BOOL all_selected = theMesh->faceSel.NumberSet() == 0;

	if (!all_selected) {
		info_message.printf( _T("%s - Some faces are selected.\n") , info_message );
		}

	RVertex* rv0;
	rv0 = theMesh->getRVertPtr( 0 );
	rv0->rn.getNormal();

	BOOL hidden = FALSE;

	rgc->fFaces.Resize( theMesh->numFaces );
	for ( int nf=0; nf<theMesh->numFaces; nf++ ) {

		if ( theMesh->faces[nf].Hidden() ) {
			hidden = TRUE;
			continue;
			}

		IPoint3 face = rgc->tFaces[ nf ];

		Point3 vd,rn;
		BOOL good_face = FALSE;

		Face * f = &theMesh->faces[nf];
		DWORD smGroup = f->smGroup;

		unsigned int f_mat_id = theMesh->faces[nf].getMatID();

		if ( theMesh->faces[nf].getMatID() == rgc->mat_id && ( theMesh->faceSel[nf] || all_selected ) ) 
		{
			for ( int i=0; i<3; i++ )
			{
				RVertex* rv;

				int num_normals = 0;
				rv = theMesh->getRVertPtr( f->getVert(i) );

				if (rv->rFlags & SPECIFIED_NORMAL)
				{
					rn = rv->rn.getNormal();
				}
				else if (( num_normals = rv->rFlags & NORCT_MASK) && smGroup)
				{
					if (num_normals == 1)
					{
						rn = rv->rn.getNormal();
					}
					else
					{
						for (int j = 0; j < num_normals; j++)
						{
							if ( rv->ern[j].getSmGroup() & smGroup )
							{
								rn = rv->ern[j].getNormal();
							}
						}
					}
				}
				else
				{
					rn = theMesh->getFaceNormal(nf);
				}

				rn = rgc->objToWorld.VectorTransform( rn );
				rn = rgc->affineTM.VectorTransform( rn );

				// Metodo Viejo
				if ( rgc->perspective )
					vd = Normalize( rgc->camVerts[ face[i] ] );
				else
					vd = rgc->ray_dir;

				float ang = acos( DotProd( vd , rn ) );

				if ( ang > rgc->end_angle ) {
					good_face = TRUE;
					i = 3;
					}
				}
			}
		else
			good_face = FALSE;

		if ( good_face ) {
			int add_u0 = 0;
			int add_u1 = 0;

			Point3 tverts[3];
			for ( int tv=0; tv<3; tv++ ) {
				tverts[tv] = outtVerts[outtvFace[nf].getTVert(tv)];
				if ( tverts[tv].x < 0.0f ) add_u1 = 1;
				if ( tverts[tv].x > 1.0f ) add_u0 = 1;
				}

			FastFace fFace;
			fFace.SetFace(	nf , tverts[0] , tverts[1] , tverts[2] ,
							rgc->tVerts[ face.x ] , rgc->tVerts[ face.y ] , rgc->tVerts[ face.z ] ,
							w, h, aa );
			rgc->fFaces.Append(1,&fFace,10);

			if ( add_u0 && !add_u1 )
			{
				for ( int tv=0; tv<3; tv++ ) 
					tverts[tv].x = tverts[tv].x - 1.0f;

				FastFace fFace;
				fFace.SetFace(	nf , tverts[0] , tverts[1] , tverts[2] ,
								rgc->tVerts[ face.x ] , rgc->tVerts[ face.y ] , rgc->tVerts[ face.z ] ,
								w, h, aa );
				rgc->fFaces.Append(1,&fFace,10);
			}

			if ( add_u1 && !add_u0 ) {
				for ( int tv=0; tv<3; tv++ ) 
					tverts[tv].x = tverts[tv].x + 1.0f;

				FastFace fFace;
				fFace.SetFace(	nf , tverts[0] , tverts[1] , tverts[2] ,
								rgc->tVerts[ face.x ] , rgc->tVerts[ face.y ] , rgc->tVerts[ face.z ] ,
								w, h, aa );
				rgc->fFaces.Append(1,&fFace,10);
				}
			}
		percent = 10 + int ( 6.0f * float(nf+1) / float(theMesh->numFaces) );
		SetPercent(percent);
		}
	rgc->fFaces.Shrink();

	if ( rgc->fFaces.Count() == 0 ) {
		info_message.printf( _T("%s - The material ID doesn't exist in this mesh.\n") , info_message );
		return 0;
		}

	if ( hidden ) {
		info_message.printf( _T("%s - Some faces are hidden.\n") , info_message );
		}

	int nzw = 0; 
	int nzh = 0;

	float aspect = float(w)/float(h);

	int nz0 = theMesh->numFaces/5;

	nzh = int(aprox(sqrt(float(nz0)/aspect)));
	nzw = int(aprox(sqrt(float(nz0)/aspect) * aspect));

	int azw = int(aprox(float(w)/float(nzw)));
	int lzw = w - (nzw-1) * azw;

	int azh = int(aprox(float(h)/float(nzh)));
	int lzh = h - (nzh-1) * azh;

	rgc->azw = azw;		rgc->azh = azh;
	rgc->nzw = nzw;		rgc->nzh = nzh;

	rgc->SetNumZones(nzh * nzw);

	for ( int i=0; i<nzh*nzw; i++ )
	{ 
		Zone * zone;
		zone = new Zone(i);
		rgc->zones[i] = zone;
	}

	for (int fct=0; fct<rgc->fFaces.Count(); fct++) {
		int cix = 999999, cfx = -1;
		int ciy = 999999, cfy = -1;

		for ( int i=0; i<3; i++ ) {
			int px = int( rgc->fFaces[fct].GetTVert(i).x * float(w) );
			int py = int( (1.0f - rgc->fFaces[fct].GetTVert(i).y ) * float(h) );

			if ( px < 0 ) px = 0;	if ( px >= w ) px = w - 1;
			if ( py < 0 ) py = 0;	if ( py >= h ) py = h - 1;

			int cx = (int)floor( float(px) / float(azw) );
			int cy = (int)floor( float(py) / float(azh) );
			if (cx < cix) cix = cx;
			if (cx > cfx) cfx = cx;
			if (cy < ciy) ciy = cy;
			if (cy > cfy) cfy = cy;
			}

		for ( int cxct=cix; cxct<=cfx; cxct++ )
			for ( int cyct=ciy; cyct<=cfy; cyct++ ) {
				int zone = cyct * nzw + cxct;
				int numface = fct;
				rgc->zones[zone]->faces.Append(1,&numface);
				}

		percent = 16 + int ( 4.0f * float(fct+1) / float(rgc->fFaces.Count()) );
		SetPercent(percent);
	}

	return 1;
}

void NebbiUtility::SaveRenderedMap(NebbiRendContext *rgc, TheBmpCache * bmCache, BitmapInfo * outBI, INode *node) {
	PreFilterMap(rgc,bmCache);
	int w = bmCache->w;
	int h = bmCache->h;

	Bitmap * bm = NULL;
	if ( DoesFileExist(outBI->Name()) ) {
		bm = TheManager->Load(outBI);
		}
	else {
		bm = TheManager->Create(outBI);
		}

	BMM_Color_64 col64; // What we output to the bitmap in the end
	col64.r = 0.0;
	col64.g = 0.0;
	col64.b = 0.0;
	col64.a = 0.0;

	SetPercent(95);

	float r,g,b,a;
	for (int ch=0; ch<h; ch++)
		for (int cw=0; cw<w; cw++) {
			bmCache->GetPixel(r,g,b,a,cw,ch);
			AColor fgCol(r,g,b,a);

			BMM_Color_64 bgCol64;
			bm->GetPixels(cw,ch,1,&bgCol64);
			AColor bgCol( float(bgCol64.r)/65535.0 , float(bgCol64.g)/65535.0 , float(bgCol64.b)/65535.0 , float(bgCol64.a)/65535.0 );
			fgCol = CompOver(fgCol,bgCol);
			fgCol.ClampMinMax();

			col64.r = fgCol.r * 65535.0;
			col64.g = fgCol.g * 65535.0;
			col64.b = fgCol.b * 65535.0;
			col64.a = fgCol.a * 65535.0;

			bm->PutPixels(cw, ch, 1, &col64);
			}

	SetPercent(100);

	// Backup Renaming of files
	if ( DoesFileExist( outBI->Name() ) )
	{
		TSTR name = TSTR( outBI->Name() );
		TSTR ext = TSTR( outBI->Name() );

		int pto = name.last('.');
		ext.remove(0,pto);
		name.remove(pto);

		TSTR oldest;
		oldest.printf( _T("%s_02%s"),name,ext );
		TSTR older;
		older.printf( _T("%s_01%s"),name,ext );
		TSTR old;
		old.printf( _T("%s_00%s"),name,ext );

		if ( DoesFileExist( oldest.data() ) ) {
			DeleteFile( oldest.data() );
			}
		if ( DoesFileExist( older.data() ) ) {
			MoveFile( older.data() , oldest.data() );
			}
		if ( DoesFileExist( old.data() ) ) {
			MoveFile( old.data() , older.data() );
			}
		if ( DoesFileExist( outBI->Name() ) ) {
			MoveFile( outBI->Name() , old.data() );
			}
		}

	bm->OpenOutput(outBI);
	bm->Write(outBI);
	bm->Close(outBI);
	bm->DeleteThis();

	UpdateMaterial( node->GetMtl(), outBI, bm );
	SetPercent(0);
	}

void NebbiUtility::PreFilterMap(NebbiRendContext *rgc, TheBmpCache *bmC) 
{
	int percent;
	int prefilter = rgc->prefiltering;
	int w = bmC->w;
	int h = bmC->h;

	// Zona de Prefiltrado

	// Name Sum Check

	TSTR u_n = GetString(IDS_USER_NAME);
	int n_s = 0;

	for ( int i_c=0; i_c<40; i_c++ )
	{
		char ch = u_n.data()[i_c];
		n_s += int(ch);
		}


	{
		int px, py, i, j;
		for (int dp=0; dp<prefilter; dp++) {
			Tab <IPoint2> npix;
			npix.SetCount(0);
			Tab <AColor> ncols;
			ncols.SetCount(0);
			for (py=0; py<h; py++)
				for (px=0; px<w; px++) {
					if (!bmC->GetHit(px,py)) {

						float r,  g,  b,  a ;
						float rc = 0.0f;
						float gc = 0.0f;
						float bc = 0.0f;
						float ac = 0.0f;

						int vecinos = 0;
						
						for (i=0; i<3; i++) 
							for (j=0; j<3; j++) {
								int posW = px-1+i;
								int posH = py-1+j;
								if ( posW >= 0 && posW < w && posH >= 0 && posH < h ) {
									bmC->GetPixel( r, g, b, a, posW, posH);
									if (bmC->GetHit(posW,posH)) {
										vecinos++;
										rc = rc + r;
										gc = gc + g;
										bc = bc + b;
										ac = ac + a;
										}

									}
								}

						if (vecinos) {
							AColor allcol;
							allcol.r = rc / float(vecinos);
							allcol.g = gc / float(vecinos);
							allcol.b = bc / float(vecinos);
							allcol.a = ac / float(vecinos);

							IPoint2 pix(px,py);
							npix.Append(1,&pix);
							ncols.Append(1,&allcol);
							}
						}
					} // for (px py

			for (i=0; i<npix.Count(); i++) 
				bmC->SetPixel(ncols[i].r, ncols[i].g, ncols[i].b, ncols[i].a, npix[i].x, npix[i].y);

			percent = 80 + int (15.0f * float(dp+1) / float(prefilter) );
			SetPercent(percent);

		} 
	}

}

void NebbiUtility::SetPercent(int per) {
	SendMessage(GetDlgItem(hPanel , IDC_PERCENT), PBM_SETPOS, per, 0); 
	}

AColor NebbiUtility::EvalColor(NebbiShadeContext &sc, Bitmap * bm) {
	BMM_Color_64 col;
	AColor theCol(0,0,0,0);
	float att = GetAttenuation(sc);
	if ( att == 0.0f ) return theCol;
//	att = 1.0f;

	Point3 t = sc.UVW();

	if ( t.x < 0.0f || t.x > 1.0f ) return theCol;
	if ( t.y < 0.0f || t.y > 1.0f ) return theCol;

	Point3 d = sc.DUVW();
	bm->GetFiltered( t.x, 1.0f-t.y, d.x, d.y, &col );
	if ( sc.rc->pre_alpha ) {
		theCol.r = (float)col.r / 65535.0f * (float)col.a / 65535.0f * att;
		theCol.g = (float)col.g / 65535.0f * (float)col.a / 65535.0f * att;
		theCol.b = (float)col.b / 65535.0f * (float)col.a / 65535.0f * att;
		theCol.a = (float)col.a / 65535.0f * att;
		}
	else {
		theCol.r = (float)col.r / 65535.0f * att;
		theCol.g = (float)col.g / 65535.0f * att;
		theCol.b = (float)col.b / 65535.0f * att;
		theCol.a = (float)col.a / 65535.0f * att;
		}
	return theCol;
	}

float NebbiUtility::GetAttenuation(NebbiShadeContext &sc) {
	float att = 1.0f;

	Point3 bc = sc.bc;
	int fn = sc.fn;

	Point3 vd;
	if ( sc.rc->perspective )
		vd = Normalize(sc.P());
	else
		vd = sc.rc->ray_dir;

	Mesh * theMesh = sc.rc->mesh;
	Face * f = &theMesh->faces[ sc.fn ];
	DWORD smGroup = f->smGroup;
	Point3 rvn[3];

	for ( int i=0; i<3; i++ ) {
		RVertex* rv;

		int num_normals = 0;
		rv = theMesh->getRVertPtr( f->getVert(i) );

		if (rv->rFlags & SPECIFIED_NORMAL)
			rvn[i] = rv->rn.getNormal();

		else if (( num_normals = rv->rFlags & NORCT_MASK) && smGroup)

			if (num_normals == 1)
				rvn[i] = rv->rn.getNormal();

			else
				for (int j = 0; j < num_normals; j++)
					if ( rv->ern[j].getSmGroup() & smGroup )
						rvn[i] = rv->ern[j].getNormal();


		else
			rvn[i] = theMesh->getFaceNormal( sc.fn );
		}

	Point3 rn =	bc.x * rvn[0] + 
				bc.y * rvn[1] + 
				bc.z * rvn[2];

	rn = sc.rc->objToWorld.VectorTransform( rn );
	rn = sc.rc->affineTM.VectorTransform( rn );

	float ang = acos( DotProd( vd , rn ) );

	float ang0 = sc.rc->end_angle;
	float ang1 = sc.rc->start_angle;

	if ( ang < ang0 ) att = 0.0f;
	else if ( ang > ang1 ) att = 1.0f;
	else if ( ang > ang0 && ang < ang1 ) {
		float u = ( ang - ang0 ) / ( ang1 - ang0 );
		att = u*u*(3.0f-2.0f*u);
		}

	if ( sc.rc->use_camera_range ) {
		float len = Length( sc.P() );
		float u = ( sc.rc->cam_far - len ) / ( sc.rc->cam_far - sc.rc->cam_near );
		float cam_att = u*u*(3.0f-2.0f*u);
		att = att * cam_att;
		}

	return att;
	}

void NebbiUtility::ApplyCamMap(NebbiRendContext *rgc, INode * object)
{
	TimeValue t = ip->GetTime();

	InputData in_data;
	std::string name;

	GetInputData( in_data );
	GetInputName( name );

	if ( in_data.input_width == 0 || in_data.input_height == 0 )
	{
		SetInputBMP();
		GetInputData( in_data );
	}

	int in_custom_size = in_data.custom_size;
	int in_width       = in_data.input_width;
	int in_height      = in_data.input_height;
	float in_aspect    = in_data.input_aspect;

	ViewExp *view = ip->GetActiveViewport();
	GraphicsWindow * gw = view->getGW();

	Matrix3 affineTM;
	view->GetAffineTM(affineTM);
	rgc->perspective = view->IsPerspView(); 

	if ( view->GetViewCamera() ) 
	{
		INode * camera = view->GetViewCamera();
		ObjectState camOState = camera->EvalWorldState(t);

		if ( ((CameraObject *)camOState.obj)->GetEnvDisplay() )
		{
			info_message.printf( _T("%s - Using Camera Ranges\n") , info_message );
			rgc->use_camera_range = TRUE;
			rgc->cam_near = ((CameraObject *)camOState.obj)->GetEnvRange( rgc->t , ENV_NEAR_RANGE );
			rgc->cam_far = ((CameraObject *)camOState.obj)->GetEnvRange( rgc->t , ENV_FAR_RANGE );
			}
		}

	rgc->affineTM = affineTM;

	if ( !rgc->perspective )
	{
		rgc->ray_dir = Point3(0,0,-1);
		Normalize( rgc->ray_dir );
	}

	int gww = gw->getWinSizeX();
	int gwh = gw->getWinSizeY();

	int w,h;

	if ( in_custom_size ) 
	{
		w = in_width;
		h = in_height;
	}
	else 
	{
		w = gww;
		h = gwh;
	}

	float dw2 = float(w)/2.0f;
	float dh2 = float(h)/2.0f;

	Matrix3 objToWorld = object->GetObjTMAfterWSM(t);

	rgc->objToWorld = objToWorld;

	float xscale,yscale;

	if ( rgc->perspective )
	{
		float fov = view->GetFOV();
		float fac =  -(float)(1.0 / tan(0.5*(double)fov));
		xscale =  fac * dw2;   // dw2 = float(devWidth)/2.0
		yscale =  xscale * in_aspect;
	}
	else 
	{
		int gwI = 0;
		int gwF = gww;

		if ( view->getSFDisplay() ) 
		{
			int vw = view->getGW()->getWinSizeX();
			int vh = view->getGW()->getWinSizeY();
			float va = float(vw)/float(vh);

			int rw = ip->GetRendWidth();
			int rh = ip->GetRendHeight();
			float ra = float(rw)/float(rh);

			if ( ra < va )
			{
				vw = vh * ra;

				int sfs = ( view->getGW()->getWinSizeX() - vw ) / 2;

				gwI = sfs;
				gwF = view->getGW()->getWinSizeX() - sfs;
			}
		}

		IPoint2 vp1(gwI,0),vp2(gwF,0);
		Point3 cp1 = view->GetPointOnCP(vp1);	// Proyeccion en el plano de construccion
		Point3 cp2 = view->GetPointOnCP(vp2);	// de cada uno de los puntos
		float vptWid = float(sqrt(pow((cp1.x - cp2.x),2) + pow((cp1.y - cp2.y),2) + pow((cp1.z - cp2.z),2)));

		xscale = (float)w/vptWid;
		yscale =  xscale * in_aspect;
	}

	// Apply Mapping

	Mesh * m = rgc->mesh;

	int nv = m->numVerts;
	int nf = m->numFaces;

	rgc->camVerts.SetCount(nv);
	rgc->tVerts.SetCount(nv);
	rgc->tFaces.SetCount(nf);

	int i;

	for (i=0; i<nv; i++)
	{
		rgc->camVerts[i] = m->verts[i] * objToWorld * affineTM;
	}

	SetPercent(2);

	for ( i=0; i<nf; i++)
	{
		rgc->tFaces[i] = IPoint3 ( m->faces[i].getVert(0) , m->faces[i].getVert(1) , m->faces[i].getVert(2) );
	}
	
	SetPercent(5);

	for ( i=0; i<nv; i++ ) 
	{
		Point3 vp = rgc->camVerts[i];

		Point2 s;
		
		if ( rgc->perspective)
		{
			s.x  = dw2 + xscale*vp.x/vp.z;
			s.y  = dh2 - yscale*vp.y/vp.z;		
		}
		else
		{
			s.x  = dw2 + xscale*vp.x;
			s.y  = dh2 - yscale*vp.y;
		}

		rgc->tVerts[i] = Point3(s.x/float(w),1.0f - s.y/float(h),0.0f);
	}

	SetPercent(10);

	ip->ReleaseViewport(view);
}

void NebbiUtility::UpdateTMaps(Texmap * map, BitmapInfo * bi, Bitmap * bm)
{
	if (!map) return;
	if ( map->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) 
	{
		BitmapTex *bmt = (BitmapTex*) map;

		if ( TSTR( bi->Name() ) == TSTR( bmt->GetMapName() ) )
		{
			bmt->ReloadBitmapAndUpdate();
		}
	}

	for (int i=0; i<map->NumSubTexmaps(); i++)
	{
		UpdateTMaps(map->GetSubTexmap(i),bi,bm);
	}
}

void NebbiUtility::UpdateMaterial(Mtl * mtl, BitmapInfo * bi, Bitmap * bm) {
	if (!mtl) return;
	for (int i=0; i<mtl->NumSubMtls(); i++) {
		UpdateMaterial(mtl->GetSubMtl(i),bi,bm);
		}
	for (int i=0; i<mtl->NumSubTexmaps(); i++) {
		UpdateTMaps(mtl->GetSubTexmap(i),bi,bm);
		}
	}

void NebbiUtility::WarningMessage( int message ) {
	TSTR message_text;
	switch ( message ) {
		case WARN_MULTIPLE_SEL:
			message_text = _T("Multiple Objects. Please select only one!");
			break;
		case WARN_NONE_SEL:
			message_text = _T("Please select an object!");
			break;
		case WARN_NO_INDATA:
			message_text = _T("Please set the stamping image size and options.");
			break;
		case WARN_NO_INNAME:
			message_text = _T("Please set an stamping image.");
			break;
		case WARN_NO_INIMAGE:
			message_text = _T("The stamping image doesn't exist. Nothing to do!");
			break;
		case WARN_INVALID_OBJ:
			message_text = _T("One of the selected objects cannot be painted!");
			break;
		case WARN_NOTHING_REND:
#ifndef NEBBI_DEMO
			message_text = _T("There was nothing to Render.\nPossible Causes:\n - Object doesn't have UVW Channel.\n - Materials ID are not properly configured.\n - Selected Faces are not visible.");
#else
			message_text = _T("This is not a valid object for the PowerStamper DEMO version.");
#endif
			break;
		default:
			message_text = _T("This is a Warning Message");
		}
	MessageBox( hPanel, message_text, _T("PowerStamper"), MB_OK);
	}

