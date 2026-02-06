#include "StdAfx.h"
#include "Physics.h"
#include "..\misc\2DArray.h"

extern vector<SSphere> sphereParticles; // test from RWGame.cpp
namespace NPhysics
{

CPos operator+( const CPos &a, const CPos &b )
{
	return CPos( a.x + b.x, a.y + b.y, a.z + b.z );
}

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

void CAnimator::AddLink( int n1, int n2 )
{
	links.push_back( SLink( n1, n2, Distance( cur.pos[n1], cur.pos[n2] ) ) );
}

int CAnimator::AddPoint( int x, int y, int z, float fRadius )
{
	x &= ~255;
	y &= ~255;
	z &= ~255;
	cur.pos.push_back( CPos( x, y, z ) );
	radiuses.push_back( fRadius );
//	radiuses.push_back( r * 1024 );
	return cur.pos.size() - 1;
}

void CAnimator::StartNew()
{
	ASSERT( cur.pos.size() > 2 );

	cur.speed.push_back( 0 ); // movement
	cur.speed.push_back( 0 ); // movement
	cur.speed.push_back( 0 ); // movement
	cur.speed.push_back( 0 ); // rotational
	cur.speed.push_back( 0 ); // rotational
	cur.speed.push_back( 0 ); // rotational
	GenerateDOFGroups( &dofGroups, links, cur.pos.size() );
	for ( int k = 0; k < dofGroups.size(); ++k )
	{
		cur.speed.push_back( 0 );
		cur.speed.push_back( 0 );
		cur.speed.push_back( 0 );
	}
	prev = cur;

	bPhrozen = false;
	originalPos = cur;
}

CVec3 SState::CalcMassCenter()
{
	CVec3 r( 0, 0, 0 );
	for ( int i = 0; i < pos.size(); ++i )
	{
		r.x += pos[i].x;
		r.y += pos[i].y;
		r.z += pos[i].z;
	}
	r /= pos.size();
	return r;
}

static float Dot( const CPos &p, const CVec3 &vDir )
{
	float f = p.x * vDir.x + p.y * vDir.y + p.z * vDir.z;
	return f;	
}

static void Shift( CPos *pRes, const CVec3 &vDir, int nDelta )
{
	pRes->x += Float2Int( vDir.x * nDelta );
	pRes->y += Float2Int( vDir.y * nDelta );
	pRes->z += Float2Int( vDir.z * nDelta );
}

void SState::ShiftSinglePoint( CPos *pRes, const CPos &_delta, ICollider *pCollider, int nPoint, int nContactOffset, float fRadius )
{
	CPos delta(_delta);

	for ( int i = 0; i < potential.size(); ++i )
	{
		SContactInfo &r = potential[i];
		if ( r.nPoint == nPoint )
		{
			SLimitPlane *pPlane = &r.plane;
			Shift( &delta, pPlane->vDir, r.nPotential );
			// out of this contact finally
			potential.erase( potential.begin() + i );
			--i;

			/*int nDelta = Float2Int( Dot( delta, pPlane->vDir ) );
			r.nPotential += nDelta;
			Shift( &delta, pPlane->vDir, -nDelta );
			if ( r.nPotential > nContactOffset )
			{
				//DebugTrace( "lost contact on %d point\n", nPoint );
				Shift( &delta, pPlane->vDir, r.nPotential );
				// out of this contact finally
				potential.erase( potential.begin() + i );
				--i;
			}*/
		}
	}

	CVec4 vPlane;
	for(;;)
	{
		if ( pCollider->Collide( *pRes, fRadius, delta, &vPlane ) )
		{
			CPos test;
			test = *pRes + delta;
			SLimitPlane plane( CVec3(vPlane.x, vPlane.y, vPlane.z), vPlane.w );
			bool bFound = false;
			for ( int i = 0; i < potential.size(); ++i )
			{
				SContactInfo &r = potential[i];
				if ( r.nPoint == nPoint && r.plane == plane )
					bFound = true;
			}
			if ( !bFound )
			{
				float f = Dot( test, plane.vDir ) + plane.fDist;
				if ( f < 0 )
				{
					SContactInfo &r = *potential.insert( potential.end(), SContactInfo() );
					r.plane = plane;
					r.nPoint = nPoint;
					r.nPotential = Float2Int( f );
					r.fFriction = 0.4f; // default friction
					Shift( &delta, plane.vDir, -r.nPotential );
					//DebugTrace( "new contact on %d point, %g %g %g\n", nPoint, plane.vDir.x, plane.vDir.y, plane.vDir.z );
				}
				else
				{
					ASSERT(0);
					break;
				}
			}
			else
				break;
		}
		else
			break;
	}
	*pRes = *pRes + delta;
	
	/*
	for ( int k = 0; k < planes.size(); ++k )
	{
		const SLimitPlane *pPlane = &planes[k];
		bool bFound = false;
		CPos test;
		test = *pRes + delta;
		for ( int i = 0; i < potential.size(); ++i )
		{
			SContactInfo &r = potential[i];
			if ( r.nPoint == nPoint && r.plane == *pPlane )
			{
				int nDelta = Float2Int( Dot( delta, pPlane->vDir ) );
				r.nPotential += nDelta;
				Shift( &delta, pPlane->vDir, -nDelta );
				if ( r.nPotential > nContactOffset )
				{
					Shift( &delta, pPlane->vDir, r.nPotential );
					// out of this contact finally
					potential.erase( potential.begin() + i );
				}
				bFound = true;
			}
		}
		if ( bFound )
			continue;
		float f = Dot( test, pPlane->vDir ) + pPlane->fDist;
		if ( f < 0 )
		{
			SContactInfo &r = *potential.insert( potential.end(), SContactInfo() );
			r.plane = *pPlane;
			r.nPoint = nPoint;
			r.nPotential = Float2Int( f );
			Shift( &delta, pPlane->vDir, -r.nPotential );
		}
	}
	*pRes = *pRes + delta;*/
}

void SState::ApplyDelta( const vector<CPos> &delta, ICollider *pCollider, int nContactOffset, const vector<float> &radiuses )
{
	for ( int i = 0; i < pos.size(); ++i )
		ShiftSinglePoint( &pos[i], delta[i], pCollider, i, nContactOffset, radiuses[i] );
}

void SState::ApplyStringsReaction( vector<CPos> *pSpeed, const vector<SLink> &links )
{
	vector<CPos> &speed = *pSpeed;
	float fRelaxAlpha = 1.0f / ( pSpeed->size() * 2 + 1 );
	vector<float> dx( speed.size(), 0.0f ), dy( speed.size(), 0.0f ), dz( speed.size(), 0.0f );
	for ( int i = 0; i < links.size(); ++i )
	{
		const SLink &l = links[i];
		int nDist = Distance( pos[l.n1], pos[l.n2] );
		double alpha = ( 1 - ((double)l.nSize) / nDist ) * 0.5f;
		//ASSERT( fabs( alpha ) < 0.4f );
		alpha = Clamp( alpha, (double)-1, (double)1 );
		alpha *= fRelaxAlpha;
		float fDeltaX = ( pos[l.n2].x - pos[l.n1].x ) * alpha;
		float fDeltaY = ( pos[l.n2].y - pos[l.n1].y ) * alpha;
		float fDeltaZ = ( pos[l.n2].z - pos[l.n1].z ) * alpha;
		dx[l.n1] += fDeltaX;
		dy[l.n1] += fDeltaY;
		dz[l.n1] += fDeltaZ;
		dx[l.n2] -= fDeltaX;
		dy[l.n2] -= fDeltaY;
		dz[l.n2] -= fDeltaZ;
	}
	for ( int k = 0; k < speed.size(); ++k )
	{
		speed[k].x += Float2Int( dx[k] );
		speed[k].y += Float2Int( dy[k] );
		speed[k].z += Float2Int( dz[k] );
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
	float fCheck = CalcNorm2( res );
	Turn( &res, p );
	for ( int k = 0; k < pRes->size(); ++k )
	{
		float f = Sub( &res, (*pRes)[k] );
		f = f;
		//char szBuf[1000];
		//sprintf( szBuf, "%g\n", f );
		//OutputDebugString( szBuf );
	}
	float f = CalcNorm2( res );
	if ( f > fCheck * 1e-3f )
	{
		Normalize( &res );
		pRes->push_back( res );
	}
	else
	{
		ASSERT(0);
		for ( int i = 0; i < res.size(); ++i )
			res[i] = CVec3(0,0,0);
		pRes->push_back( res );
	}
	//OutputDebugString( "----" );
}

void SState::GenerateDOFs( const vector<SDOF> &dofs, vector<Speed> *pRes ) const
{
	pRes->clear();
	Speed res, src, srcRot;
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

	srcRot = src;
	for ( int n = 0; n < pRes->size(); ++n )
	{
		float f = Sub( &srcRot, (*pRes)[n] );
	}
	// rotation
	AddRotation( srcRot, pRes, TurnXY );
	AddRotation( srcRot, pRes, TurnYZ );
	AddRotation( srcRot, pRes, TurnXZ );

	for ( int k = 0; k < dofs.size(); ++k )
	{
		const SDOF &dof = dofs[k];
		for ( int i = 0; i < res.size(); ++i )
			res[i] = CVec3(0,0,0);
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
		for ( int i = 0; i < dof.right.size(); ++i )
		{
			int n = dof.right[i];
			res[ n ] = -res[ n ];
		}
		AddRotation( res, pRes, TurnXY );
		AddRotation( res, pRes, TurnYZ );
		AddRotation( res, pRes, TurnXZ );
	}
	//OutputDebugString( "---\n" );
}

void SState::GenerateSpeed( const vector<SDOF> &dofGroups, const vector<float> &speeds, vector<CPos> *pRes )
{
	vector<Speed> dofs;
	GenerateDOFs( dofGroups, &dofs );
	pRes->resize( pos.size() );
	for ( int i = 0; i < pRes->size(); ++i )
	{
		CVec3 vRes( 0, 0, 0 );
		for ( int k = 0; k < dofs.size(); ++k )
			vRes += dofs[k][i] * speeds[k];
		(*pRes)[i].x = Float2Int( vRes.x );
		(*pRes)[i].y = Float2Int( vRes.y );
		(*pRes)[i].z = Float2Int( vRes.z );
	}
}

static void CalcGeneralizedPointSpeed( vector<float> *pRes, const vector<Speed> &dofs, int nPoint, const CVec3 &vSpeed )
{
	vector<float> &delta = *pRes;
	delta.resize( dofs.size(), 0 );
	for ( int i = 0; i < delta.size(); ++i )
		delta[i] = dofs[i][nPoint] * vSpeed;
	// normalize contact plane
	float fMul = 1 / CalcNorm2( delta );
	for ( int i = 0; i < delta.size(); ++i )
		delta[i] *= fMul;
}

static void GetGenerilizedContacts( const SState &state, const vector<SDOF> &dofGroups, vector<SContact> *pRes )
{
	vector<Speed> dofs;
	state.GenerateDOFs( dofGroups, &dofs );
	ASSERT( dofs.size() == state.speed.size() );

	pRes->resize( 0 );
	for ( int k = 0; k < state.potential.size(); ++k )
	{
		const SContactInfo &c = state.potential[k];
		int nContact = c.nPoint;
		CVec3 vPlaneX, vPlaneY;
		vPlaneX = CVec3(0,0,1) ^ c.plane.vDir;
		if ( fabs2( vPlaneX ) < 0.1f )
			vPlaneX = CVec3(1,0,0) ^ c.plane.vDir;
		Normalize( &vPlaneX );
		vPlaneY = c.plane.vDir ^ vPlaneX;

		SContact &rr = *pRes->insert( pRes->end(), SContact() );
		CalcGeneralizedPointSpeed( &rr.speed, dofs, nContact, c.plane.vDir );
		CalcGeneralizedPointSpeed( &rr.speedX, dofs, nContact, vPlaneX );
		CalcGeneralizedPointSpeed( &rr.speedY, dofs, nContact, vPlaneY );
		rr.fFriction = c.fFriction;
	}
}

static void FindSomeLinearSolution( const CArray2D<float> &_m, vector<float> &proj, vector<float> *pRes  )
{
	int nSize = proj.size();
	ASSERT( _m.GetXSize() == nSize && _m.GetYSize() == nSize );
	pRes->resize( proj.size() );
	CArray2D<float> left(_m);
	CArray2D<float> right;
	right.SetSizes( nSize, nSize );
	for ( int y = 0; y < nSize; ++y )
	{
		for ( int x = 0; x < nSize; ++x )
			right[y][x] = x == y;
	}

	const float about0 = 1e-3f;
	for ( int y = 0; y < nSize; y++ )
	{
		float fDiag = left[y][y], fMax = fabs( fDiag );
		int nBestRow = y;
		for ( int k = y + 1; k < nSize; ++k )
		{
			float fTest = fabs( left[k][y] );
			if ( fTest > 2 * fMax )
			{
				fMax = fTest;
				nBestRow = k;
			}
		}
		if ( nBestRow != y )
		{
			float f = fMax * fDiag > 0 ? 1 : -1;
			for ( int x = 0; x < nSize; ++x )
			{
				left[y][x] += f * left[ nBestRow ][x];
				right[y][x] += f * right[ nBestRow ][x];
			}
		}
		fDiag = left[y][y];
		if ( fabs( fDiag ) < about0 )
		{
			int h = y;
			while( ( h < nSize - 1 ) && ( fabs(fDiag) < about0 ) )
			{
				h++;
				if( fabs(left[h][y]) > about0 )
				{
					for( int u = 0; u < nSize; u++ )
					{
						left[y][u] += left[h][u];
						right[y][u] += right[h][u];
					}
					fDiag = left[y][y];
				}
			}
			if ( fabs(fDiag) < about0 )
			{
				// not needed contact
				/*for ( int u = 0; u < nSize; ++u )
				{
					left[y][u] = 0; left[u][y] = 0;
					right[y][u] = 0; right[u][y] = 0;
				}*/
				continue;
			}
		}
		float fDiag1 = 1 / fDiag;
		for ( int x = 0; x < nSize; ++x )
		{
			left[y][x] *= fDiag1;
			right[y][x] *= fDiag1;
		}
		for ( int k = 0; k < nSize; ++k )
		{
			if ( k == y )
				continue;
			float fK = left[k][y];
			for ( int x = 0; x < nSize; ++x )
			{
				left[k][x] -= left[y][x] * fK;
				right[k][x] -= right[y][x] * fK;
			}
		}
	}
	for ( int y = 0; y < nSize; ++y )
	{
		float fRes = 0;
		for ( int x = 0; x < nSize; ++x )
			fRes += right[y][x] * proj[x];
		(*pRes)[y] = fRes;
	}
}

static void AddToLinearSpace( vector<vector<float> > *pRes, const vector<float> &_vec )
{
	vector<float> v( _vec );
	for ( int i = 0; i < pRes->size(); ++i )
		Sub( &v, (*pRes)[i] );
	float f = Dot( v, v );
	if ( f > 1e-4f )
	{
		for ( int z = 0; z < v.size(); ++z )
			v[z] *= 1 / sqrt(f);
		pRes->push_back( v );
	}
}

static void BuildLinearSpace( vector<vector<float> > *pRes, const vector<SContact> &contacts, const vector<int> &offending )
{
	pRes->resize( 0 );
	for ( int k = 0; k < offending.size(); ++k )
		AddToLinearSpace( pRes, contacts[ offending[ k ] ].speed );
}

static void Sub( vector<float> *pRes, const vector<vector<float> > &linearSpace )
{
	for ( int k = 0; k < linearSpace.size(); ++k )
		Sub( pRes, linearSpace[k] );
}

static bool FindOffending( const vector<float> &speed, const vector<SContact> &contacts, vector<int> *pOffend )
{
	vector<int> &offending = *pOffend;
	offending.resize( 0 );
	float fOffendingEnergy = 0;
	for ( int k = 0; k < contacts.size(); ++k )
	{
		float f = Dot( speed, contacts[k].speed );
		if ( f < 0 )
		{
			offending.push_back( k );
			fOffendingEnergy += sqr( f );
		}
	}
	return !offending.empty();
}

static void FindContactStrength( const vector<float> &speed, const vector<SContact> &contacts, const vector<int> &offending, vector<float> *pContactStrength )
{
	int nSize = offending.size();
	vector<float> &contactStrength = *pContactStrength, src( nSize );
	contactStrength.resize( nSize );
	CArray2D<float> m;
	m.SetSizes( nSize, nSize );
	for ( int y = 0; y < nSize; ++y )
	{
		for ( int x = y; x < nSize; ++x )
		{
			float f = Dot( contacts[ offending[y] ].speed, contacts[ offending[x] ].speed );
			m[y][x] = f;
			m[x][y] = f;
		}
		src[y] = Dot( contacts[ offending[y] ].speed, speed );
	}
	// distribute speed by contacts
	FindSomeLinearSolution( m, src, &contactStrength );
}

static void SubtractContacts( vector<float> *pRes, const vector<SContact> &contacts, const vector<int> &offending, const vector<float> &contactStrength, float fRestitution )
{
	for ( int k = 0; k < contactStrength.size(); ++k )
	{
		float fVal = contactStrength[k];
		//if ( fVal >= 1e-5 ) // for some reason this check is wrong
		//	continue;
		for ( int x = 0; x < pRes->size(); ++x )
			(*pRes)[x] -= ( 1 + fRestitution ) * fVal * contacts[ offending[k] ].speed[x];
	}
}

static bool ReflectSpeeds( vector<float> *pRes, const vector<SContact> &contacts, float fRestitution )
{
	vector<int> offending;
	if ( !FindOffending( *pRes, contacts, &offending ) )
		return false;
	vector<float> contactStrength;
	FindContactStrength( *pRes, contacts, offending, &contactStrength );
	SubtractContacts( pRes, contacts, offending, contactStrength, fRestitution );
	return true;
}

/*static void PerformFriction( vector<float> *pRes, const vector<SContact> &contacts, const vector<int> offending, const vector<float> &contactStrength )
{
	//vector<vector<float> > contactSpace;
	//BuildLinearSpace( &contactSpace, contacts );
	for ( int k = 0; k < contactStrength.size(); ++k )
	{
		float fStrength = contactStrength[k];
		const SContact &c = contacts[ offending[k] ];
		vector<float> speedX( c.speedX ), speedY( c.speedY );

		float f = Dot( speedX, speedY ), fD1 = 1 / ( 1 - f * f ), fX, fY;
		float fX1 = Dot( speedX, *pRes );
		float fY1 = Dot( speedY, *pRes );
		fX = fD1 * (  fX1 - f * fY1 );
		fY = fD1 * ( -f * fX1 + fY1 );

		//for ( int i = 0; i < contactSpace.size(); ++i )
		//{
		//Sub( &speedX, contactSpace[i] );
		//Sub( &speedY, contactSpace[i] );
		//}

		float fMaxCancel = fabs( fStrength * c.fFriction );
		float fNorm = sqrt(sqr(fX) + sqr(fY) );
		//DebugTrace( "max cancel = %g, movement = %g\n", fMaxCancel, fNorm );
		if ( fNorm <= fMaxCancel )
		{
			for ( int i = 0; i < pRes->size(); ++i )
			{
				(*pRes)[i] -= fX * speedX[i] + fY * speedY[i];
			}
		}
		else
		{
			float fK = fMaxCancel / fNorm;
			ASSERT( fK >= 0 && fK < 1 );
			for ( int i = 0; i < pRes->size(); ++i )
			{
				(*pRes)[i] -= fK * ( fX * speedX[i] + fY * speedY[i] );
			}
		}
	}
}*/

/*static void SimulateFriction( vector<float> *pRes, const vector<SContact> &contacts, const vector<float> &forces )
{
	vector<int> offending;
	vector<float> contactStrength;
	if ( FindOffending( *pRes, contacts, &offending ) )
	{
		FindContactStrength( *pRes, contacts, offending, &contactStrength );
		PerformFriction( pRes, contacts, offending, contactStrength );
	}
	if ( FindOffending( forces, contacts, &offending ) )
	{
		FindContactStrength( forces, contacts, offending, &contactStrength );
		PerformFriction( pRes, contacts, offending, contactStrength );
	}
}*/

// simulate sub time step oscillations convergence
static void SimulateSubStepConvergence( vector<float> *pRes, const vector<SContact> &contacts, const vector<float> &forces )
{
	vector<int> cancel;
	for ( int k = 0; k < contacts.size(); ++k )
	{
		const SContact &c = contacts[k];
		float fSpeed = Dot( *pRes, c.speed );
		float fForce = Dot( forces, c.speed );
		if ( fForce < 0 && fabs(fSpeed) < 1.1f * fabs(fForce) )
			cancel.push_back( k );
	}
	vector<float> contactStrength;
	FindContactStrength( *pRes, contacts, cancel, &contactStrength );
	SubtractContacts( pRes, contacts, cancel, contactStrength, 0 );

/*	float fSpeedFraction = Dot( forces, *pRes ), fForce = Dot( forces, forces );
	if ( fabs( fSpeedFraction ) < 1.1f * fForce )
	{
		float f = fSpeedFraction / fForce;
		for ( int k = 0; k < forces.size(); ++k )
			(*pRes)[k] -= forces[k] * f;
		vector<float> reflectedForces( forces );
		ReflectSpeeds( &reflectedForces, contacts, 0 );
		for ( int k = 0; k < reflectedForces.size(); ++k )
			(*pRes)[k] += reflectedForces[k] * f;
	}*/
}

static void CalcReflection( const vector<float> &delta, const vector<SContact> &contacts, float fRestitution, vector<float> *pContactReaction, vector<float> *pReaction )
{
	vector<int> offending;
	vector<float> contactStrength;
	if ( FindOffending( delta, contacts, &offending ) )
	{
		FindContactStrength( delta, contacts, offending, &contactStrength );
		for ( int k = 0; k < contactStrength.size(); ++k )
			(*pContactReaction)[ offending[k] ] += contactStrength[k];

		SubtractContacts( pReaction, contacts, offending, contactStrength, fRestitution );
	}
}

//const float fFrictionKoef = 0.4f;//45f;
static void PerformSpeedIntegration( vector<float> *pRes, const vector<SContact> &contacts, float fRestitution, const vector<float> &forces )
{
	//if ( contacts.empty() )
	//	return;
	int nTry;
	for ( nTry = 0; nTry < 50; ++nTry )
	{
		vector<float> contactReaction( contacts.size(), 0.0f );
		vector<float> reaction( pRes->size(), 0.0f );

		if ( nTry == 0 )
		{
			for ( int k = 0; k < reaction.size(); ++k )
				(*pRes)[k] += forces[k] * 0.5f;
		}
		
		/*vector<int> crap;
		if ( contacts.size() > 1 )
			crap.clear();*/

		/*if ( nTry == 0 )
		{
			// add forces
			for ( int k = 0; k < reaction.size(); ++k )
				reaction[k] += forces[k];
			// subtract reaction forces
			CalcReflection( forces, contacts, 0, &contactReaction, &reaction );
		}*/
		// add reaction forces for speed reflection
		CalcReflection( *pRes, contacts, fRestitution, &contactReaction, &reaction );

		// apply to speed
		for ( int k = 0; k < pRes->size(); ++k )
			(*pRes)[k] += reaction[k];

		if ( nTry == 0 )
		{
			for ( int k = 0; k < reaction.size(); ++k )
				(*pRes)[k] += forces[k] * 0.5f;
		}

		SimulateSubStepConvergence( pRes, contacts, forces );

		float fTotalSpeed = sqrt( Dot( *pRes, *pRes ) );
		// friction
		vector<vector<float> > frictionSpace;
		vector<vector<float> > contactSpace;
		vector<int> activeContacts;
		for ( int k = 0; k < contactReaction.size(); ++k )
		{
			if ( contactReaction[k] != 0 )
				activeContacts.push_back( k );
		}
		BuildLinearSpace( &contactSpace, contacts, activeContacts );
		{
			vector<char> takenReaction( contactReaction.size(), (char)0 );
			float fPrevBest;
			for (;;)
			{
				float fBest = 0;
				int nBest;
				for ( int k = 0; k < contactReaction.size(); ++k )
				{
					float f = fabs( contactReaction[k] );
					if ( f > fBest && !takenReaction[k] )
					{
						fBest = f;
						nBest = k;
					}
				}
				// slow down
				if ( !frictionSpace.empty() )
				{
					float fDelta = fPrevBest - fBest;
					vector<float> p( frictionSpace.size() );
					for ( int k = 0; k < p.size(); ++k )
						p[k] = Dot( *pRes, frictionSpace[k] );

					float fMaxCancel = fabs( fDelta * 0.4f ); // friction is 0.4
					float fNorm = CalcNorm2( p );
					//DebugTrace( "max cancel = %g, movement = %g\n", fMaxCancel, fNorm );
					if ( fNorm <= fMaxCancel )
					{
						for ( int k = 0; k < frictionSpace.size(); ++k )
						{
							for ( int i = 0; i < pRes->size(); ++i )
								(*pRes)[i] -= p[k] * frictionSpace[k][i];
						}
					}
					else
					{
						float fK = fMaxCancel / fNorm;
						ASSERT( fK >= 0 && fK < 1 );
						for ( int k = 0; k < frictionSpace.size(); ++k )
						{
							for ( int i = 0; i < pRes->size(); ++i )
								(*pRes)[i] -= fK * p[k] * frictionSpace[k][i];
						}
					}
				}
				fPrevBest = fBest;

				if ( fBest == 0 )
					break;
				takenReaction[nBest] = 1;
				const SContact &c = contacts[ nBest ];
				vector<float> vec( c.speedX );
				Sub( &vec, contactSpace );
				AddToLinearSpace( &frictionSpace, vec );
				vec = c.speedY;
				Sub( &vec, contactSpace );
				AddToLinearSpace( &frictionSpace, vec );
			}
		}
		/*const int N_FRICTION_STEPS = 10;
		for ( int nStep = 0; nStep < N_FRICTION_STEPS; ++nStep )
		{
			for ( int k = 0; k < contactReaction.size(); ++k )
			{
				float fStrength = contactReaction[k] / N_FRICTION_STEPS;
				if ( fStrength == 0 )
					continue;
				const SContact &c = contacts[ k ];
				vector<float> speedX( c.speedX ), speedY( c.speedY );
				//float fStrength = Dot( reaction, c.speed );

				float f = Dot( speedX, speedY ), fD1 = 1 / ( 1 - f * f ), fX, fY;
				float fX1 = Dot( speedX, *pRes );
				float fY1 = Dot( speedY, *pRes );
				fX = fD1 * (  fX1 - f * fY1 );
				fY = fD1 * ( -f * fX1 + fY1 );

				//for ( int i = 0; i < contactSpace.size(); ++i )
				//{
				//Sub( &speedX, contactSpace[i] );
				//Sub( &speedY, contactSpace[i] );
				//}

				float fMaxCancel = fabs( fStrength * c.fFriction );
				float fNorm = sqrt(sqr(fX) + sqr(fY) );
				//DebugTrace( "max cancel = %g, movement = %g\n", fMaxCancel, fNorm );
				if ( 1 )//fNorm <= fMaxCancel )
				{
					for ( int i = 0; i < pRes->size(); ++i )
					{
						(*pRes)[i] -= fX * speedX[i] + fY * speedY[i];
					}
				}
				else
				{
					float fK = fMaxCancel / fNorm;
					ASSERT( fK >= 0 && fK < 1 );
					for ( int i = 0; i < pRes->size(); ++i )
					{
						(*pRes)[i] -= fK * ( fX * speedX[i] + fY * speedY[i] );
					}
				}
			}
		}*/

		float fCheck = sqrt( Dot( *pRes, *pRes ) );
		ASSERT( fCheck <= fTotalSpeed * 1.001f );

/*
		// friction
		SimulateFriction( pRes, contacts, forces );
		if ( !ReflectSpeeds( pRes, contacts, fRestitution ) )
			break;

		float fCheck = sqrt( Dot( *pRes, *pRes ) );
		ASSERT( fCheck <= fTotalSpeed * 1.001f );
		*/

		SimulateSubStepConvergence( pRes, contacts, forces );

		float fReactionNorm = CalcNorm2( reaction );
		float fResNorm = CalcNorm2( *pRes );
		if ( fReactionNorm < fResNorm * 1e-6f || fResNorm < 0.1f )
			break;
	}
	ASSERT( nTry < 50 );
	// check
	for ( int k = 0; k < contacts.size(); ++k )
	{
		float f = Dot( *pRes, contacts[k].speed );
		ASSERT( f > -1e-3f );
	}
}

void CAnimator::Step( ICollider *pCollider )
{
	if ( !bPhrozen )
	{
		//static double fEnergy, fWasEnergy = 1e30f;
		//fEnergy = CalcEnergy();
		//ASSERT( fEnergy <= fWasEnergy );
		//fWasEnergy = fEnergy;
		DebugTrace( "%g speed, %d contacts\n", sqrt(Dot(cur.speed, cur.speed)), cur.potential.size() );
		//DebugTrace( "speed: %g %g %g %g %g %g\n", cur.speed[0], cur.speed[1], cur.speed[2], cur.speed[3], cur.speed[4], cur.speed[5] );
		//DebugTrace( "before positions: %d %d %d %d %d\n", cur.pos[0].z, cur.pos[1].z, cur.pos[2].z, cur.pos[3].z, cur.pos[4].z );
		// cheat for convergence - increase friction with time
		//for ( int i = 0; i < cur.potential.size(); ++i )
		//	cur.potential[i].fFriction *= 1.5f;
		int nContactOffset = 0;//nGravity;// * 5;
		const float fRestitution = 0.6f;//1;//

		prev = cur;

		vector<SContact> contacts;
		GetGenerilizedContacts( cur, dofGroups, &contacts );

		vector<CPos> speeds;
		cur.GenerateSpeed( dofGroups, cur.speed, &speeds );
		cur.ApplyDelta( speeds, pCollider, nContactOffset, radiuses );

		//DebugTrace( "shifted positions: %d %d %d %d %d\n", cur.pos[0].z, cur.pos[1].z, cur.pos[2].z, cur.pos[3].z, cur.pos[4].z );

		// relaxation using potential
		for ( int k = 0; k < 100; ++k )
		{
			for ( int i = 0; i < speeds.size(); ++i )
				speeds[i] = CPos(0,0,0);
			cur.ApplyStringsReaction( &speeds, links );
			cur.ApplyDelta( speeds, pCollider, nContactOffset, radiuses );
		}

		//DebugTrace( "relaxed positions: %d %d %d %d %d\n", cur.pos[0].z, cur.pos[1].z, cur.pos[2].z, cur.pos[3].z, cur.pos[4].z );
		vector<float> forces( cur.speed.size(), 0.0f );
		// apply gravity
		forces[2] -= nGravity * sqrt( (float)cur.pos.size() );

		//for ( int k = 0; k < cur.speed.size(); ++k )
		//	cur.speed[k] += forces[k] * 0.5f;

		vector<float> beforeSpeed( cur.speed );
		PerformSpeedIntegration( &cur.speed, contacts, fRestitution, forces );

		float fDif = 0;
		for ( int k = 0; k < beforeSpeed.size(); ++k )
			fDif += sqr( beforeSpeed[k] - cur.speed[k] );
		if ( sqrt( fDif ) < CalcNorm2( forces ) * 0.5f )
		{
			DebugTrace( "almost frozen\n" );
			if ( cur.pos == prev.pos )
				bPhrozen = true;
		}
		/*vector<float> resForces( forces );
		ReflectSpeeds( &resForces, contacts, 0 );
		// sometimes freezes a little bit earlier then true freeze takes place
		if ( Dot( resForces, resForces ) < 0.1f && Dot( cur.speed, cur.speed ) < cur.speed.size() * 1.1f )
		{
			DebugTrace( "almost frozen\n" );
			if ( cur.pos == prev.pos )
				bPhrozen = true;
		}*/

		/*if ( !bPhrozen )
		{
			for ( int k = 0; k < cur.speed.size(); ++k )
				cur.speed[k] += forces[k] * 0.5f;

			LimitSpeeds( &cur.speed, contacts, 1, forces );
			//SimulateSubStepConvergence( &cur.speed, contacts, forces );
		}*/
	}
	{
		sphereParticles.clear();
		for ( int i = 0; i < cur.pos.size(); ++i )
			sphereParticles.push_back( SSphere( cur.pos[i].GetVec3() / 65536, 0.1f ) );
	}
}

double CAnimator::CalcEnergy()
{
	double fR = 0;
	for ( int i = 0; i < cur.speed.size(); ++i )
	{
		fR += sqr( prev.speed[i] ) * 0.5f;
	}
	for ( int i = 0; i < cur.pos.size(); ++i )
	{
		fR += nGravity * ( cur.pos[i].z + prev.pos[i].z ) * 0.5f;
	}
	for ( int i = 0; i < cur.potential.size(); ++i )
	{
		fR += nGravity * ( ( cur.potential[i].nPotential ) * cur.potential[i].plane.vDir.z ) * 0.5f;
	}
	for ( int i = 0; i < prev.potential.size(); ++i )
	{
		fR += nGravity * ( ( prev.potential[i].nPotential ) * prev.potential[i].plane.vDir.z ) * 0.5f;
	}
	return fR;
}

CVec3 CAnimator::CalcMassCenter()
{
	return cur.CalcMassCenter();
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

void CAnimator::SetPosition( const CVec3 &vPos, const CQuat &q )
{
	for ( int k = 0; k < originalPos.pos.size(); ++k )
	{
		CVec3 v;
		q.Rotate( &v, originalPos.pos[k].GetVec3() );
		v += vPos;
		cur.pos[k] = CPos( Float2Int( v.x ), Float2Int( v.y ), Float2Int( v.z ) );
	}
	for ( int k = 0; k < cur.speed.size(); ++k )
		cur.speed[k] = 0;
	cur.potential.clear();
}

static bool IsAlmostEqual( CVec3 &a, CVec3 &b, float fEps )
{
	return fabs2( a - b ) < sqr( fEps ) * fabs2( a );
}

static float CalcAngle( const CVec3 &a, const CVec3 &b, const CVec3 &vAxis )
{
	CVec3 ap = a - vAxis * ( a * vAxis );
	CVec3 ar = vAxis ^ ap;
	float fCos = b * ap, fSin = b * ar;
	return atan2( fSin, fCos );
}

void CAnimator::GetPosition( CVec3 *pRes, CQuat *pQ )
{
	CVec3 vCurCenter = cur.CalcMassCenter();
	CVec3 vOriginalCenter = originalPos.CalcMassCenter();
	*pRes = vCurCenter - vOriginalCenter;
	int nCount = 0;
	CVec3 vA[2], vB[2], vDir[2];
	for ( int k = 0; k < cur.pos.size(); ++k )
	{
		CVec3 vStart( originalPos.pos[k].GetVec3() - vOriginalCenter );
		CVec3 vFinish( cur.pos[k].GetVec3() - vCurCenter );
		if ( IsAlmostEqual( vStart, vFinish, 1e-3f ) )
			continue;
		vA[nCount] = vStart; 
		vB[nCount] = vFinish;
		vDir[nCount] = vB[nCount] - vA[nCount];
		if ( nCount == 0 )
			nCount = 1;
		else if ( nCount == 1 && !IsAlmostEqual( vDir[0], vDir[1], 0.1f ) )
			++nCount;
		if ( nCount == 2 )
			break;
	}
	if ( nCount == 2 )
	{
		CVec3 vAxis = vDir[0] ^ vDir[1];
		Normalize( &vAxis );
		// angle?
		float fAngle1 = CalcAngle( vA[0], vB[0], vAxis );
		float fAngle2 = CalcAngle( vA[1], vB[1], vAxis );
		//ASSERT( fAngle1 == fAngle2 );
		pQ->FromAngleAxis( fAngle1, vAxis );
	}
	else if ( nCount == 1 )
	{
		pQ->FromAngleAxis( FP_PI, vA[0] + vB[0], true );
	}
	else
		*pQ = QNULL;
}

void CAnimator::SetVelocity( const CVec3 &vMove, const CVec3 &vRot )
{
	cur.speed[0] = vMove.x;
	cur.speed[1] = vMove.y;
	cur.speed[2] = vMove.z;
	cur.speed[3] = vRot.x;
	cur.speed[4] = vRot.y;
	cur.speed[5] = vRot.z;
}
}