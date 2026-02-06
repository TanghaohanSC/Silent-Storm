// ParticleDynamicsDoc.cpp : implementation of the CParticleDynamicsDoc class
//

#include "stdafx.h"
#include "ParticleDynamics.h"

#include "ParticleDynamicsDoc.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CParticleDynamicsDoc

IMPLEMENT_DYNCREATE(CParticleDynamicsDoc, CDocument)

BEGIN_MESSAGE_MAP(CParticleDynamicsDoc, CDocument)
END_MESSAGE_MAP()

BOOL CParticleDynamicsDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	StartNew();

	return TRUE;
}

#pragma warning( disable: 4267 )

static void AddGroup( vector< vector<int> > *pRes, const vector<SLink> &links, int nStart, int nSkip )
{
	vector<int> r;
	r.push_back( nStart );
	for(;;)
	{
		int nS = r.size();
		for ( int k = 0; k < links.size(); ++k )
		{
			const SLink &l = links[k];
			if ( l.n1 == nSkip || l.n2 == nSkip )
				continue;
			bool b1 = IsInSet( r, l.n1 ), b2 = IsInSet( r, l.n2 );
			if ( b1 && !b2 )
				r.push_back( l.n2 );
			if ( !b1 && b2 )
				r.push_back( l.n1 );
		}
		if ( nS == r.size() )
			break;
	}
	sort( r.begin(), r.end() );
	if ( IsInSet( *pRes, r ) )
		return;
	pRes->push_back( r );
}

static void GenerateDOFGroups( vector<SDOF> *pRes, const vector<SLink> &links, int nPoints )
{
	pRes->clear();
	for ( int k = 0; k < nPoints; ++k )
	{
		vector< vector<int> > groups;
		// select start points and create groups
		for ( int n = 0; n < links.size(); ++n )
		{
			const SLink &l = links[n];
			if ( l.n1 == k )
				AddGroup( &groups, links, l.n2, k );
			if ( l.n2 == k )
				AddGroup( &groups, links, l.n1, k );
		}
		for ( int i = 1; i < groups.size(); ++i )
		{
			SDOF dof;
			dof.left = groups[0];
			dof.right = groups[i];
			dof.nCenter = k;
			pRes->push_back( dof );
		}
	}
}

/*static double sqr( double x ) { return x * x; }

int __forceinline Float2Int( const float fpVar )
{
	int nRet;
	__asm 
	{
		fld dword ptr fpVar
			fistp nRet
	}
	return nRet;
}
*/
static int Distance( const CPos &a, const CPos &b )
{
	double nDif = sqr( (double)(a.x - b.x) ) + sqr( (double)(a.y - b.y) ) + sqr( (double)(a.z - b.z) );
	return Float2Int( sqrt( nDif ) );
}

void CParticleDynamicsDoc::AddLink( int n1, int n2 )
{
	links.push_back( SLink( n1, n2, Distance( cur.pos[n1], cur.pos[n2] ) ) );
}

void CParticleDynamicsDoc::AddPoint( int x, int y, int z, int r )
{
	cur.pos.push_back( CPos( x * 1024, y * 1024, z * 1024 ) );
	//cur.speed.push_back( CPos( sx, sy ) );
	cur.potential.push_back( CPos( 0, 0, 0 ) );
	radiuses.push_back( r * 1024 );
}

const float fKeepEnergy = 1;//0.5f;

void CParticleDynamicsDoc::StartNew()
{
	//AddPoint( 400, 400, 10, 0, 100 );
	//AddPoint( 500, 400, 10, 0, 100 );
	//AddPoint( 600, 400, 10, 0, 100 );
	//AddPoint( 600, 500, 10, 0, 100 );
	//AddPoint( 400, 750, 10, 0, 0 );
	//AddPoint( 400, 400, 10, 0, 100 );
	//AddPoint( 400, 300, 10, 0, 100 );
	//AddPoint( 400, 200, 10, 0, 100 );
	//AddPoint( 400, 100, 10, 0, 100 );
	//AddPoint( 400, 90, 10, 0, 100 );
	//AddPoint( 400, 80, 10, 0, 100 );
	//AddPoint( 400, 70, 10, 0, 100 );
	//AddPoint( 400, 60, 10, 0, 100 );
	//AddLink( 0, 1 );//1000 );//
	//AddLink( 1, 2 );
	//AddLink( 2, 3 );//0 );//
	//AddLink( 3, 4, 100 );
	//AddLink( 4, 5, -1000 );
	//AddLink( 5, 6 );
	//AddLink( 6, 7 );
	
//	AddPoint( 0, 0, 340, 10 );
	AddPoint( 0, 0, 400, 10 );
	AddPoint( 0, 0, 500, 10 );
	AddPoint( 100, 0, 400, 10 );
	//AddPoint( 400, 370, 10, 0, -1000 );

//	AddLink( 0, 1 );
	AddLink( 0, 1 );
	AddLink( 0, 2 );
	AddLink( 1, 2 );
	//AddLink( 3, 4 );
	
	cur.speed.push_back( 0 ); // movement
	cur.speed.push_back( 0 ); // movement
	cur.speed.push_back( 0 ); // movement
	cur.speed.push_back( -1000 ); // rotational
	cur.speed.push_back( 1000 ); // rotational
	cur.speed.push_back( 1000 ); // rotational
	//GenerateDOFGroups( &dofGroups, links, cur.pos.size() );
	//for ( int k = 0; k < dofGroups.size(); ++k )
	//	cur.speed.push_back( 0 );
}

void SState::ApplyDelta( const vector<CPos> &delta )
{
	for ( int i = 0; i < pos.size(); ++i )
	{
		pos[i].x += delta[i].x;
		pos[i].y += delta[i].y;
		// handle collision
		if ( pos[i].z == 0 )
		{
			int nDelta = delta[i].z;
			if ( nDelta > 0 )
			{
				potential[i].z += nDelta;
				if ( potential[i].z > 0 )
				{
					pos[i].z += potential[i].z;
					potential[i].z = 0;
				}
			}
			else
				potential[i].z += nDelta;
		}
		else 
		{
			pos[i].z += delta[i].z;
			if ( pos[i].z < 0 )
			{
				potential[i].z = pos[i].z;
				pos[i].z = 0;
			}
		}
	}
}

//static int Sign( int n ) { if ( n > 0 ) return 1; if ( n < 0 ) return -1; return 0; }

void SState::ApplyGravity()
{
	speed[2] -= 8;
}

void SState::ApplyStringsReaction( vector<CPos> *pSpeed, const vector<SLink> &links )
{
	vector<CPos> &speed = *pSpeed;
	float fRelaxAlpha = 0.25f;
	for ( int i = 0; i < links.size(); ++i )
	{
		const SLink &l = links[i];
		int nDist = Distance( pos[l.n1], pos[l.n2] );
		double alpha = ( 1 - ((double)l.nSize) / nDist ) * 0.5f;
		ASSERT( fabs( alpha ) < 0.4f );
		alpha *= fRelaxAlpha;
		int nDeltaX = Float2Int( ( pos[l.n2].x - pos[l.n1].x ) * alpha );
		int nDeltaY = Float2Int( ( pos[l.n2].y - pos[l.n1].y ) * alpha );
		int nDeltaZ = Float2Int( ( pos[l.n2].z - pos[l.n1].z ) * alpha );
		nDeltaX &= ~1;
		nDeltaY &= ~1;
		nDeltaZ &= ~1;
		speed[l.n1].x += nDeltaX;
		speed[l.n1].y += nDeltaY;
		speed[l.n1].z += nDeltaZ;
		speed[l.n2].x -= nDeltaX;
		speed[l.n2].y -= nDeltaY;
		speed[l.n2].z -= nDeltaZ;
	}
}

static void Normalize( Speed *pRes )
{
	float f = 0;
	for ( int i = 0; i < pRes->size(); ++i )
		f += fabs2( (*pRes)[i] );
	f = 1 / sqrt( f );
	for ( int i = 0; i < pRes->size(); ++i )
		(*pRes)[i] *= f;
}

template<class T>
static float Sub( vector<T> *pRes, const vector<T> &b )
{
	float f = 0;
	for ( int i = 0; i < pRes->size(); ++i )
		f += (*pRes)[i] * b[i];
	for ( int i = 0; i < pRes->size(); ++i )
		(*pRes)[i] -= b[i] * f;
	return f;
}

template<class T>
static float Dot( const vector<T> &a, const vector<T> &b )
{
	float f = 0;
	for ( int i = 0; i < a.size(); ++i )
		f += a[i] * b[i];
	return f;
}

static void mad( Speed *pRes, const Speed &a, float f )
{
	for ( int k = 0; k < pRes->size(); ++k )
		(*pRes)[k] += a[k] * f;
}

static CVec3 TurnXY( const CVec3 &v ) { return CVec3( v.y, -v.x, 0 ); }
static CVec3 TurnYZ( const CVec3 &v ) { return CVec3( 0, v.z, -v.y ); }
static CVec3 TurnXZ( const CVec3 &v ) { return CVec3( v.z, 0, -v.x ); }

template<class T>
static void Turn( Speed *pRes, T p )
{
	Speed &res = *pRes;
	for ( int k = 0; k < res.size(); ++k )
		res[k] = p( res[k] );
}

template<class T>
static float CalcNorm2( const vector<T> &a )
{
	float f = 0;
	for ( int i = 0; i < a.size(); ++i )
		f += fabs2( a[i] );
	return sqrt( f );
}

template <class T> 
static void AddRotation( const Speed &src, vector<Speed> *pRes, T p )
{
	Speed res;
	res = src;
	for ( int k = 0; k < pRes->size(); ++k )
	{
    float f = Sub( &res, (*pRes)[k] );
		//char szBuf[1000];
		//sprintf( szBuf, "%g\n", f );
		//OutputDebugString( szBuf );
	}
	float fCheck = CalcNorm2( res );
	Turn( &res, p );
	Normalize( &res );
	pRes->push_back( res );
	//OutputDebugString( "----" );
}

void SState::GenerateDOFs( const vector<SDOF> &dofs, vector<Speed> *pRes )
{
	pRes->clear();
	Speed res, src;
	src.resize( pos.size() );
	for ( int k = 0; k < src.size(); ++k )
		src[k] = CVec3( pos[k].x, pos[k].y, pos[k].z );

	// linear movement
	res.resize( pos.size() );
	for ( int k = 0; k < res.size(); ++k )
		res[k] = CVec3(1,0,0);
	Normalize( &res );
	pRes->push_back( res );
	for ( int k = 0; k < res.size(); ++k )
		res[k] = CVec3(0,1,0);
	Normalize( &res );
	pRes->push_back( res );
	for ( int k = 0; k < res.size(); ++k )
		res[k] = CVec3(0,0,1);
	Normalize( &res );
	pRes->push_back( res );

	// rotation
	AddRotation( src, pRes, TurnXY );
	AddRotation( src, pRes, TurnYZ );
	AddRotation( src, pRes, TurnXZ );

/*	for ( int k = 0; k < dofs.size(); ++k )
	{
		const SDOF &dof = dofs[k];
		for ( int i = 0; i < res.size(); ++i )
			res[i] = CVec2(0,0);
		for ( int i = 0; i < dof.left.size(); ++i )
		{
			int n = dof.left[i];
			res[ n ] = src[ n ] - src[ dof.nCenter ];
		}
		for ( int i = 0; i < dof.right.size(); ++i )
		{
			int n = dof.right[i];
			res[ n ] = src[ n ] - src[ dof.nCenter ];
		}
		Turn( &res );
		for ( int i = 0; i < dof.right.size(); ++i )
		{
			int n = dof.right[i];
			res[ n ] = -res[ n ];
		}

		for ( int n = 0; n < pRes->size(); ++n )
		{
			float f = Sub( &res, (*pRes)[n] );
			//char szBuf[1000];
			//sprintf( szBuf, "%g\n", f );
			//OutputDebugString( szBuf );
		}
		float f = CalcNorm2( res );
		ASSERT( f > fCheck * 1e-3f );
		Normalize( &res );
		pRes->push_back( res );
	}*/
	//OutputDebugString( "---\n" );
}

void SState::ApplyReaction( const vector<SDOF> &dofGroups )
{
	vector<CPos> speeds;
	GenerateSpeed( &speeds, dofGroups );
	float fCheck = 0;
	int nContact;
	for ( int k = 0; k < pos.size(); ++k )
	{
		if ( pos[k].z == 0 && speeds[k].z < 0 )
		{
			fCheck += sqr( speeds[k].z );

			nContact = k;
		}
	}
	if ( fCheck == 0 )
		return;

	vector<Speed> dofs;
	GenerateDOFs( dofGroups, &dofs );
	vector<float> delta( speed.size(), 0 );

	for ( int i = 0; i < delta.size(); ++i )
		delta[i] = dofs[i][nContact].z;
	float fMul = 1 / CalcNorm2( delta );
	for ( int i = 0; i < delta.size(); ++i )
		delta[i] *= fMul;

	vector<float> newSpeed( speed );
	float fVal = Sub( &newSpeed, delta );
	for ( int k = 0; k < speed.size(); ++k )
		speed[k] -= 2.0f * fVal * delta[k];
}

void SState::GenerateSpeed( vector<CPos> *pRes, const vector<SDOF> &dofGroups )
{
	vector<Speed> dofs;
	GenerateDOFs( dofGroups, &dofs );
	pRes->resize( pos.size() );
	for ( int i = 0; i < pRes->size(); ++i )
	{
		CVec3 vRes( 0, 0, 0 );
		for ( int k = 0; k < dofs.size(); ++k )
			vRes += dofs[k][i] * speed[k];
		(*pRes)[i].x = Float2Int( vRes.x );
		(*pRes)[i].y = Float2Int( vRes.y );
		(*pRes)[i].z = Float2Int( vRes.z );
	}
}

void CParticleDynamicsDoc::Step()
{
	vector<CPos> speeds;
	cur.ApplyGravity();
	cur.ApplyReaction( dofGroups );
	cur.GenerateSpeed( &speeds, dofGroups );
	// apply delta using potential
	cur.ApplyDelta( speeds );
	// x = x + v * t + 0.5 * a * t * t !!!
	// relaxation using potential
	for ( int k = 0; k < 100; ++k )
	{
		for ( int i = 0; i < speeds.size(); ++i )
			speeds[i] = CPos(0,0,0);
		cur.ApplyStringsReaction( &speeds, links );
		cur.ApplyDelta( speeds );
	}

	UpdateAllViews( 0 );
}

double CParticleDynamicsDoc::CalcEnergy()
{
	double fR = 0;
	for ( int i = 0; i < cur.speed.size(); ++i )
	{
		fR += sqr( cur.speed[i] ) * 0.5f;
	}
	for ( int i = 0; i < cur.pos.size(); ++i )
	{
		fR += 8 * ( cur.pos[i].z + cur.potential[i].z );
	}
	return fR;
}

CVec3 CParticleDynamicsDoc::CalcMassCenter()
{
	CVec3 r( 0, 0, 0 );
	for ( int i = 0; i < cur.pos.size(); ++i )
	{
		r.x += cur.pos[i].x;
		r.y += cur.pos[i].y;
		r.z += cur.pos[i].z;
	}
	r /= cur.pos.size();
	return r;
}
// CParticleDynamicsDoc serialization

void CParticleDynamicsDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CParticleDynamicsDoc diagnostics

#ifdef _DEBUG
void CParticleDynamicsDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CParticleDynamicsDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CParticleDynamicsDoc commands
