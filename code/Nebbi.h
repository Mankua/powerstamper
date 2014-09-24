/*===========================================================================*\
 | 
 |  FILE:	Nebbi.h
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

#ifndef __NEBBI__H
#define __NEBBI__H

#include <string>
#include <vector>
#include "max.h"
#include "iparamm2.h"
#include "utilapi.h"
#include "stdmat.h"

#include "resource.h"


// IMPORTANT:
// The ClassID must be changed whenever a new project
// is created using this skeleton
#define	NEBBI_CLASSID		Class_ID(0x2da74642, 0x5af779f6)

#define NEBBI_CUR_VERSION	1000

#define NAMESIZE 250
#define CONFIGNAME _T("powstamp.cod")

void GetCfgFilename( TCHAR *filename );
int GetAuthCode();

TCHAR *GetString(int id);
extern ClassDesc* GetSkeletonUtilDesc();

Point3		CalcBaryCoords(Point2 p0, Point2 p1, Point2 p2, Point2 p);
Point3		CalcBaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p);
float		aprox(float v);

/************************************************************************************\
|* TheBmpCache: Used for multithreading rendering											:::::|
\************************************************************************************/

class TheBmpCache
{
public:
	float * r;
	float * g;
	float * b;
	float * a;
	int	  * hit;
	int w;
	int h;
	int size;

	TheBmpCache(int wid, int hei) {	
		w = wid;
		h = hei;
		size = w * h;
		r = new float[size];
		g = new float[size];
		b = new float[size];
		a = new float[size];
		hit = new int[size];

		for (int i=0; i<size; i++) {
			r[i] = 0.0f; g[i] = 0.0f; b[i] = 0.0f; a[i] = 0.0f; hit[i] = 0;
			}
		}
	
	~TheBmpCache() {	
		delete [] r;
		delete [] g;
		delete [] b;
		delete [] a;
		delete [] hit;
		r = NULL;
		g = NULL;
		b = NULL;
		a = NULL;
		hit = NULL;
		}

	void SetPixel(float red, float green, float blue, float alpha, int wpos, int hpos) {
		int place = hpos * w + wpos;
		
		r[place] = red;
		g[place] = green;
		b[place] = blue;
		a[place] = alpha;
		hit[place] = 1;
		}

	void GetPixel(float &red, float &green, float &blue, float &alpha, int wpos, int hpos) {
		int place = hpos * w + wpos;

		red = r[place];
		green = g[place];
		blue = b[place];
		alpha = a[place];
		}

	int GetHit(int wpos, int hpos) {
		int place = hpos * w + wpos;
		return hit[place];
		}
	};

/************************************************************************************\
|* FastFaces: Faster to search faces											:::::|
\************************************************************************************/

class FastFace {
public:
	int index;
	float xmin, xmax, ymin, ymax;
	float sqRad;
	Point2 center;
	Point2 duvw;

	Point2 v0,v1,v2;

	Point3 c0,c1,c2;

	Point2 GetTVert(int i) {
		switch (i) {
			case 0:		return v0; 
			case 1:		return v1;
			case 2:		return v2;
			default:	return Point2(0.0f,0.0f);
			}
		}

	BOOL HitTest( Point2 hitPoint, Point3 &bc ) {
		if ( hitPoint.x < xmin )	return FALSE;
		if ( hitPoint.x > xmax )	return FALSE;
		if ( hitPoint.y < ymin )	return FALSE;
		if ( hitPoint.y > ymax )	return FALSE;

		Point2 radVec = center - hitPoint;
		float  hitRad = radVec.x * radVec.x + radVec.y * radVec.y;

		if ( hitRad > sqRad ) 		return FALSE;

		bc = CalcBaryCoords(v0, v1, v2, hitPoint);

		if ( bc.x < 0.0f || bc.x > 1.0f) return FALSE;
		if ( bc.y < 0.0f || bc.y > 1.0f) return FALSE;
		if ( bc.z < 0.0f || bc.z > 1.0f) return FALSE;

		return TRUE;
		}

	void SetFace( int ind, Point3 v0, Point3 v1, Point3 v2, Point3 t0, Point3 t1, Point3 t2, int w, int h, int aa ) {
		index = ind;

		this->v0 = Point2(v0.x,v0.y);
		this->v1 = Point2(v1.x,v1.y);
		this->v2 = Point2(v2.x,v2.y);

		xmin = v0.x;
		if ( v1.x < xmin ) xmin = v1.x;
		if ( v2.x < xmin ) xmin = v2.x;

		ymin = v0.y;
		if ( v1.y < ymin ) ymin = v1.y;
		if ( v2.y < ymin ) ymin = v2.y;

		xmax = v0.x;
		if ( v1.x > xmax ) xmax = v1.x;
		if ( v2.x > xmax ) xmax = v2.x;

		ymax = v0.y;
		if ( v1.y > ymax ) ymax = v1.y;
		if ( v2.y > ymax ) ymax = v2.y;

		// Los vectores entre los puntos
		Point2 vec01 = v1 - v0;
		Point2 vec02 = v2 - v0;

		// Los puntos medios de esos vectores
		Point2 c01 = Point2(	v0.x + 0.5f * vec01.x ,
								v0.y + 0.5f * vec01.y	);
		Point2 c02 = Point2(	v0.x + 0.5f * vec02.x ,
								v0.y + 0.5f * vec02.y	);

		// Las normales a los vectores
		Point2 vn01( -vec01.y , vec01.x );
		Point2 vn02( -vec02.y , vec02.x );

		float det = - vn01.x * vn02.y + vn01.y * vn02.x;
		float u   = - vn02.y * ( c02.x - c01.x ) / det + vn02.x * ( c02.y - c01.y ) / det;

		center = c01 + u * vn01;
		Point2 vC = center - Point2(v0.x,v0.y);
		sqRad = vC.x*vC.x + vC.y*vC.y;

		// Calculo el duvw
		Point3 sw,tw;

		float txmin = t0.x;
		if ( t1.x < txmin ) txmin = t1.x;
		if ( t2.x < txmin ) txmin = t2.x;

		float tymin = t0.y;
		if ( t1.y < tymin ) tymin = t1.y;
		if ( t2.y < tymin ) tymin = t2.y;

		float txmax = t0.x;
		if ( t1.x > txmax ) txmax = t1.x;
		if ( t2.x > txmax ) txmax = t2.x;

		float tymax = t0.y;
		if ( t1.y > tymax ) tymax = t1.y;
		if ( t2.y > tymax ) tymax = t2.y;

		duvw.x = ( txmax - txmin ) / ( xmax * w * aa - xmin * w * aa );
		duvw.y = ( tymax - tymin ) / ( ymax * h * aa - ymin * h * aa );
		}
	};

/************************************************************************************\
|* Zone: Since we render 2D data, this class is used to store different zones of:::::|
|*       mesh to save rendering time while intersecting geometry:::::::::::::::::::::|
\************************************************************************************/

class Zone {
public:
	int w,h;
	Tab <int> faces;

	Zone(int i) { w = i; faces.SetCount(0);}
	};

/*===========================================================================*\
 |	Nebbi Render Context
\*===========================================================================*/

class NebbiRendContext {
public:
	Mesh * mesh;
	INode * node;
	Point3 camPos;
	TimeValue t;

	Matrix3 objToWorld;
	Matrix3 affineTM;

	int mat_id;
	int uvw;

	Box3 boundingObj;

	BOOL perspective;
	Point3 ray_dir;
	
	int custom_size;
	int in_width;
	int in_height;

	int width;
	int height;
	int prefiltering;
	int antialiasing;

	float start_angle;
	float end_angle;

	int use_camera_range;
	float cam_near;
	float cam_far;

	BOOL pre_alpha;

	int azw, azh, nzw, nzh;
	Tab <Zone*> zones;
	Tab <FastFace> fFaces;

	Tab <Point3> camVerts;

	Tab <IPoint3> tFaces;
	Tab <Point3> tVerts;

	NebbiRendContext(Mesh *m, int oCh, int c_s, int in_w, int in_h, int w, int h, int aa, int pf, float a_s, float a_e, BOOL p_a, int mid, TimeValue tv);
	
	void SetNumZones(int size)	{ zones.SetCount(size); }
	void DeleteZones();
	};

class NebbiShadeContext {
public:
	NebbiRendContext * rc;

	Point3 bc;
	int fn,ff;	// fn : The face number in the mesh's faces array
				// ff : The fast face id in the NebbiRendContext's Fast Faces array
	TimeValue t;
	int w,h;

	NebbiShadeContext(NebbiRendContext* grc);

	void SetSC(Point3 baryCoords, int fastFace, int faceNumber);
	Point3 P();
	Point3 UVW();
	Point3 DUVW();
	};

class NullView : public View {
	public:
		Point2 ViewToScreen(Point3 p)	{ return Point2(p.x,p.y); }
		NullView() {
			worldToView.IdentityMatrix();
			screenW=640.0f; screenH = 480.0f;
			}
	};

// Outputs Stuff
class OutputGroupsInfo
{
public:

	int num_groups;

	std::vector<BOOL> use;

	std::vector<int> uvw;

	std::vector<int> mat_ids;

	std::vector<std::string> bmp_names;

	std::vector<int> out_widths;
	std::vector<int> out_heights;

	OutputGroupsInfo( int n_g ) {
		num_groups = 0;
		SetNumGroups( 0 );
		}

	~OutputGroupsInfo() {
		SetNumGroups(0);
		}

	void SetNumGroups( int n_g ) 
	{
		if ( n_g < num_groups )
		{
			for ( int i=n_g; i<num_groups; i++ ) 
			{
				bmp_names[i] = "";
			}
			
			num_groups = n_g;
			use.resize( num_groups );
			mat_ids.resize( num_groups );
			uvw.resize( num_groups );
			bmp_names.resize( num_groups );
			out_widths.resize( num_groups );
			out_heights.resize( num_groups );
		}
		if ( n_g > num_groups )
		{
			use.resize( n_g );
			mat_ids.resize( n_g );
			uvw.resize( n_g );
			bmp_names.resize( n_g );
			out_widths.resize( n_g );
			out_heights.resize( n_g );

			for ( int i=num_groups; i<n_g; i++ ) 
			{
				bmp_names[i] = "";
				use[i] = TRUE;
				mat_ids[i] = i;
				uvw[i] = 1;
				out_widths[i] = 400;
				out_heights[i] = 300;
			}
			num_groups = n_g;
		}
	}

	void DeleteGroup( int group )
	{
		use.erase(         use.begin()         + group );
		mat_ids.erase(     mat_ids.begin()     + group );
		uvw.erase(         uvw.begin()         + group );
		out_widths.erase(  out_widths.begin()  + group );
		out_heights.erase( out_heights.begin() + group );
		bmp_names.erase(   bmp_names.begin()   + group );

		num_groups = bmp_names.size();
	}
};

// TODO: Dejar espacio en cada una de estas clases para poner 2 float y 2 int

class InputData
{
public:
	int custom_size;
	int input_width;
	int input_height;
	float input_aspect;

	float angle_start;
	float angle_end;

	float dist_start;
	float dist_end;

	int antialiasing;
	int prefiltering;

	BOOL pre_alpha;

	InputData() : input_width(0), input_height(0), angle_start(45.0f), angle_end(90.0f), prefiltering(5), antialiasing(1) {}
};

class OutputData
{
public:
	BOOL output_use;
	int output_matid;
	int output_uvw;
	int output_width;
	int output_height;

	OutputData() : output_width(0), output_height(0) {}
};

/*===========================================================================*\
 |	NebbiUtility class defn
\*===========================================================================*/

class NebbiUtility : public UtilityObj {
	public:

		int serial;
		int user_name_sum;

		IUtil *iu;
		Interface *ip;

		static Bitmap * inBM;

		// Windows handle of our UI
		HWND hPanel;
		
		TSTR info_message;
		
		//Constructor/Destructor
		NebbiUtility();
		~NebbiUtility();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void SelectionSetChanged(Interface *ip,IUtil *iu);

		void PutInputData( InputData &in_data, std::string &in_name );
		void GetInputData( InputData &in_data );
		void GetInputName( std::string &in_name );
		void DlgInputSize( float * inputSize );

		void ShowInputData();
		void ShowInputBMPName();

		void SetInputData();
		void SetInputBMP();
		void SetSizeIn();
		void RenderInputBMP();

		void GetUIData( InputData &in_data );

		int GetNumOutGroups();
		void PutNumOutGroups( int n_g );

		void PutOutputData( int group_id, OutputData &out_data, std::string &out_name );
		void GetOutputData( int group_id, OutputData &out_data );
		void GetOutputName( int group_id, std::string &out_name );

		int GetNumOutGroups( INode * node );
		void GetOutputData(  INode * node, int group_id, OutputData &out_data );
		void GetOutputName(  INode * node, int group_id, std::string &out_name );

		void ShowOutputBMPName();
		void SetOutBmpsByMatID();

		void PaintOutputBMP();
		void PaintZone(NebbiRendContext *rgc, TheBmpCache * bmC, int hStart, int hEnd, Bitmap * inputBM);

		void UpdateTMaps(Texmap * map, BitmapInfo * bi, Bitmap * bm);
		void UpdateMaterial(Mtl * mtl, BitmapInfo * bi, Bitmap * bm);

		void SetPercent(int per);
		int BuildZones(NebbiRendContext *rgc, Mesh *theMesh);

		AColor EvalColor(NebbiShadeContext &sc, Bitmap *bm);
		float GetAttenuation(NebbiShadeContext &sc);

		void PreFilterMap(NebbiRendContext *rgc, TheBmpCache *bmC);
		void SaveRenderedMap(NebbiRendContext *rgc, TheBmpCache * bmCache, BitmapInfo * outBI, INode *node);

		void ApplyCamMap(NebbiRendContext *rgc, INode * object);

		BOOL IsGoodObjectToStamp( ObjectState &os );
		void WarningMessage( int message );
	};

static NebbiUtility theNebbiUtility;

#endif