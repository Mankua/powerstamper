#include "Nebbi.h"

// Calculate barycentric coordinates of a point inside a triangle.
Point3 CalcBaryCoords(Point2 p0, Point2 p1, Point2 p2, Point2 p)
{ 
	Point3 bary;

	float area_abc = ( p1.x - p0.x ) * ( p2.y - p0.y ) - ( p2.x - p0.x ) * ( p1.y - p0.y );
	area_abc = 1.0f/area_abc;

	float area_pbc = ( p1.x - p.x  ) * ( p2.y - p.y  ) - ( p2.x - p.x  ) * ( p1.y - p.y  );
	float area_apc = ( p.x  - p0.x ) * ( p2.y - p0.y ) - ( p2.x - p0.x ) * ( p.y  - p0.y );
	float area_abp = ( p1.x - p0.x ) * ( p.y  - p0.y ) - ( p.x  - p0.x ) * ( p1.y - p0.y );

	bary.x = area_pbc * area_abc;
	bary.y = area_apc * area_abc;
	bary.z = area_abp * area_abc;
	
	return bary;
}

Point3 CalcBaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p)
{ 
	Point3 bary;

	float area_abc = ( p1.x - p0.x ) * ( p2.y - p0.y ) - ( p2.x - p0.x ) * ( p1.y - p0.y );
	area_abc = 1.0f/area_abc;

	float area_pbc = ( p1.x - p.x  ) * ( p2.y - p.y  ) - ( p2.x - p.x  ) * ( p1.y - p.y  );
	float area_apc = ( p.x  - p0.x ) * ( p2.y - p0.y ) - ( p2.x - p0.x ) * ( p.y  - p0.y );
	float area_abp = ( p1.x - p0.x ) * ( p.y  - p0.y ) - ( p.x  - p0.x ) * ( p1.y - p0.y );

	bary.x = area_pbc * area_abc;
	bary.y = area_apc * area_abc;
	bary.z = area_abp * area_abc;
	
	return bary;
}

// Float to integer aproximation.
float aprox(float v) {
	if ((v - floor(v)) >= 0.05f)	v = ceil(v);
	else							v = floor(v);
	return v;
	}

NebbiRendContext::NebbiRendContext(Mesh *m, int oCh, int c_s, int in_w, int in_h, int w, int h, int aa, int pf, float a_s, float a_e, BOOL p_a, int mid, TimeValue tv) {
	mesh = m;

	custom_size = c_s;
	in_width = in_w;
	in_height = in_h;

	width = w;
	height = h;
	prefiltering = pf;
	antialiasing = aa;

	start_angle = ( 180.0f - a_s ) * PI / 180.0f;
	end_angle = ( 180.0f - a_e ) * PI / 180.0f;

	pre_alpha = p_a;

	mat_id = mid;
	uvw = oCh;

	t = tv;

	use_camera_range = 0;
	cam_near = 0.0f;
	cam_far = 0.0f;
	}

void NebbiRendContext::DeleteZones() {
	for (int i=0; i<zones.Count(); i++) {
		if ( zones[i] ) {
			delete zones[i];
			zones[i] = NULL;
			}
		}
	}

/*===============================*\
	NEBBI SHADE CONTEXT
\*===============================*/

NebbiShadeContext::NebbiShadeContext(NebbiRendContext* grc) {
	rc = grc;
	}

void NebbiShadeContext::SetSC(Point3 baryCoords, int fastFace, int faceNumber) {
	bc = baryCoords;
	fn = faceNumber;
	ff = fastFace;
	}

Point3 NebbiShadeContext::P() {
	return	bc.x * rc->camVerts[ rc->mesh->faces[fn].v[0] ] + 
			bc.y * rc->camVerts[ rc->mesh->faces[fn].v[1] ] + 
			bc.z * rc->camVerts[ rc->mesh->faces[fn].v[2] ];
	}

Point3 NebbiShadeContext::UVW() {
	IPoint3 face = rc->tFaces[fn];
	return	bc.x * rc->tVerts[face.x] + 
			bc.y * rc->tVerts[face.y] + 
			bc.z * rc->tVerts[face.z];
	}

Point3 NebbiShadeContext::DUVW() {
	return Point3( rc->fFaces[ff].duvw.x , rc->fFaces[ff].duvw.x , 0.0f );
	}
