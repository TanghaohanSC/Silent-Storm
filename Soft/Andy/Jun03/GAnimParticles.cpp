#include "StdAfx.h"
#include "Transform.h"
#include "GAnimFormat.h"
#include "GAnimParticles.h"
#include "aiMap.h"
#include "phCollider.h"
#include "..\misc\RandomGen.h"
#include "..\misc\LogStream.h"
#include "..\misc\StrProc.h"
#include "..\Misc\Commands.h"
#include "wTSFlags.h"
#include "GSceneUtils.h"
#include "aiStability.h"
#include "Bound.h"
#include "rpgItemInfo.h"
#include "wCheckGlassGrenade.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
extern vector<SSphere> sphereParticles; // test from RWGame.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAnimation
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CAParticle
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAParticle::InitSticks( const int *pPairs, int nPairs )
{
	sticks.resize( nPairs );
	for ( int nS = 0; nS < nPairs; ++nS )
	{
		sticks[nS].nP1 = pPairs[ nS * 2 ];
		sticks[nS].nP2 = pPairs[ nS * 2 + 1 ];
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAParticle::IterateSticks()
{
	for ( int nS = 0; nS < sticks.size(); ++nS )
	{
		CVec3 &pos1 = particles[ sticks[nS].nP1 ];
		CVec3 &pos2 = particles[ sticks[nS].nP2 ];
		CVec3 delta = pos2 - pos1;
		float fDelta = fabs(delta);
		if ( fDelta < 1e-6f )
		{
			fDelta = 1.f;
			delta = VNULL3;
		}
		float fDiff = (fDelta - sticks[nS].fRest)/fDelta/2;
		pos1 += fDiff * delta;
		pos2 -= fDiff * delta;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAParticle::RestoreAll( vector<CVec3> parts, SSkeletonPose *pPose )
{
	ASSERT( pPose->size() == restore.size() );
	for ( int nBone = 0; nBone < pPose->size(); ++nBone )
	{
		SRestoreBone &rest = restore[nBone];
		if ( rest.nP1 == -1 || rest.nP2 == -1 )
			continue;
		if ( rest.nP3 == -1 )
			RestoreLine( parts, rest.nP1, rest.nP2, nBone, pPose );
		else
		{
			// CRAP{ -- needed to fix later
			if ( rest.nP2 == 0xFF )
			{
				CVec3 t1, t2;
				t1 = parts[3];
				t2 = parts[4];
				parts[3] = (t1 + t2) / 2;
				RestoreTriangle( parts, rest.nP1, 3, rest.nP3, nBone, pPose );
				parts[3] = t1;
			}
			else
			// CRAP}
				RestoreTriangle( parts, rest.nP1, rest.nP2, rest.nP3, nBone, pPose );
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAParticle::RestoreTriangle( vector<CVec3> parts, int nP1, int nP2, int nP3, int nTargetBone, SSkeletonPose *pPose )
{
	CVec3 axis;
	float fAngle;

	int nBone1 = nParticleBones[nP1];
	int nBone2 = nParticleBones[nP2];
	int nBone3 = nParticleBones[nP3];

	SBonePose local1 = (*pPose)[nBone1];
	SBonePose local2 = (*pPose)[nBone2];
	SBonePose local3 = (*pPose)[nBone3];
	int nTargetParent = (*pPose)[nTargetBone].nParent;

	(*pPose)[nBone1].MakeGlobal( *pPose );
	(*pPose)[nBone2].MakeGlobal( *pPose );
	(*pPose)[nBone3].MakeGlobal( *pPose );
	(*pPose)[nTargetBone].MakeGlobal( *pPose );

	CVec3 old1, old2, old3;
	old1 = (*pPose)[nBone1].pos;
	old2 = (*pPose)[nBone2].pos;
	old3 = (*pPose)[nBone3].pos;
	CVec3 new1, new2, new3;
	new1 = parts[nP1];
	new2 = parts[nP2];
	new3 = parts[nP3];

	CVec3 shift = old1 - (*pPose)[nTargetBone].pos;
	CVec3 move = new1 - old1;
	old1 += move;
	old2 += move;
	old3 += move;
	CVec3 n1 = (old2 - old1) ^ (old3 - old1);
	CVec3 n2 = (new2 - new1) ^ (new3 - new1);
	Normalize(&n1);
	Normalize(&n2);
	axis = n1 ^ n2;
	fAngle = acos( n1 * n2 );
	CQuat q1( fAngle, axis, true );
	if ( 1 - n1 * n2 < 1e-6 )
		q1 = QNULL;
	old2 = old1 + q1.Rotate(old2 - old1);
	old3 = old1 + q1.Rotate(old3 - old1);
	n1 = old2 - old1;
	n2 = new2 - new1;
	Normalize(&n1);
	Normalize(&n2);
	axis = n1 ^ n2;
	fAngle = acos( n1 * n2 );
	CQuat q2( fAngle, axis, true );
	if ( 1 - n1 * n2 < 1e-6 )
		q2 = QNULL;

	CQuat q = q2 * q1;
	(*pPose)[nTargetBone].rot = q * (*pPose)[nTargetBone].rot;
	(*pPose)[nTargetBone].pos += shift;
	(*pPose)[nTargetBone].pos += move;
	(*pPose)[nTargetBone].pos -= q.Rotate(shift);

	if ( nTargetBone != nBone1 )
		(*pPose)[nBone1] = local1;
	if ( nTargetBone != nBone2 )
		(*pPose)[nBone2] = local2;
	if ( nTargetBone != nBone3 )
		(*pPose)[nBone3] = local3;
	(*pPose)[nTargetBone].MakeLocal( *pPose, nTargetParent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAParticle::RestoreLine( vector<CVec3> parts, int nP1, int nP2, int nTargetBone, SSkeletonPose *pPose )
{
	int nBone1 = nParticleBones[nP1];
	int nBone2 = nParticleBones[nP2];

	SBonePose local1 = (*pPose)[nBone1];
	SBonePose local2 = (*pPose)[nBone2];
	int nTargetParent = (*pPose)[nTargetBone].nParent;

	(*pPose)[nBone1].MakeGlobal( *pPose );
	(*pPose)[nBone2].MakeGlobal( *pPose );
	(*pPose)[nTargetBone].MakeGlobal( *pPose );

	CVec3 old1, old2;
	old1 = (*pPose)[nBone1].pos;
	old2 = (*pPose)[nBone2].pos;
	CVec3 new1, new2;
	new1 = parts[nP1];
	new2 = parts[nP2];
	CVec3 n1 = old2 - old1;
	CVec3 n2 = new2 - new1;
	Normalize(&n1);
	Normalize(&n2);
	CVec3 axis = n1 ^ n2;
	float fAngle = acos( n1 * n2 );
	CQuat q( fAngle, axis, true );
	if ( 1 - n1 * n2 < 1e-6 )
		q = QNULL;
	(*pPose)[nTargetBone].pos = new1;
	(*pPose)[nTargetBone].rot = q * (*pPose)[nTargetBone].rot;
/*
	// CRAP
	if ( nP1 == 5 )
	{
		int n = 14;
		int nParent = (*pPose)[n].nParent;
		(*pPose)[n].MakeGlobal( *pPose );
		(*pPose)[n].rot = q * (*pPose)[n].rot;
		(*pPose)[n].MakeLocal( *pPose, nParent );
	}
	if ( nP1 == 8 )
	{
		int n = 16;
		int nParent = (*pPose)[n].nParent;
		(*pPose)[n].MakeGlobal( *pPose );
		(*pPose)[n].rot = q * (*pPose)[n].rot;
		(*pPose)[n].MakeLocal( *pPose, nParent );
	}
*/
	if ( nTargetBone != nBone1 )
		(*pPose)[nBone1] = local1;
	if ( nTargetBone != nBone2 )
		(*pPose)[nBone2] = local2;
	(*pPose)[nTargetBone].MakeLocal( *pPose, nTargetParent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CParticleSkeleton
////////////////////////////////////////////////////////////////////////////////////////////////////
const int NUM_PARTICLES = 19;
const char *pszParticleBoneNames[] = {
	"Head",
	"L_UArm",
	"R_UArm",
	"Spine",
	"Spine",

	"L_Thigh",
	"R_Thigh",
	"L_Leg",
	"R_Leg",
	"L_Foot",

	"R_Foot",
	"L_LArm",
	"R_LArm",
	"L_Hand",
	"R_Hand",

	"L_Thigh",
	"R_Thigh",
	"L_Leg",
	"R_Leg",

};
////////////////////////////////////////////////////////////////////////////////////////////////////
const float fInitInvMasses[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const float fRadiuses[] = {
	0.16f, 0.15f, 0.15f, 0.13f, 0.13f,
	0.13f, 0.13f, 0.13f, 0.13f, 0.13f,
	0.13f, 0.12f, 0.12f, 0.11f, 0.11f,
	0.13f, 0.13f, 0.13f, 0.13f,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const CVec3 shifts[] = {
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0.1f,0,0),
	CVec3(-0.1f,0,0),

	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),

	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),

	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
	CVec3(0,0,0),
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const int NUM_STICKS = 31;
const int nPairs[] = {
0, 1,
1, 3,
3, 4,
4, 2,
2, 0,
0, 3,
3, 2,
2, 1,
1, 4,
4, 0,
3, 5,
4, 6,
3, 6,
4, 5,
5, 6,
13, 11,
11, 1,
14, 12,
12, 2,

5, 7,
7, 9,
6, 8,
8, 10,

5, 15,
15, 7,
6, 16,
16, 8,
7, 17,
17, 9,
8, 18,
18, 10,

};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SAngle
{
	int nP1;
	int nP2;
	int nP3;
	float fAngle;
	int nMax;
};
int NUM_ANGLES = 14;
SAngle angles[18] = {
	{ 3, 0, 5, FP_PI2 * 1.6f, 0 }, // spine/chest
	{ 4, 0, 6, FP_PI2 * 1.6f, 0 },
	{ 2, 12, 1, FP_PI2 * 1.3f, 0 }, // shoulders
	{ 1, 11, 2, FP_PI2 * 1.3f, 0 },
	{ 5, 6, 7, FP_PI2, 0 }, // thighs
	{ 6, 5, 8, FP_PI2, 0 },
	{ 5, 6, 7, FP_PI2 * 1.3f, 1 },
	{ 6, 5, 8, FP_PI2 * 1.3f, 1 },
	{ 5, 3, 7, FP_PI2 * 1.2f, 0 },
	{ 6, 4, 8, FP_PI2 * 1.2f, 0 },
	{ 7, 5, 9, FP_PI2 * 0.5f, 0 }, // knees
	{ 8, 6, 10, FP_PI2 * 0.5f, 0 },
	{ 11, 13, 1, FP_PI2 * 0.4f, 0 }, // elbows
	{ 12, 14, 2, FP_PI2 * 0.4f, 0 },

	{ 7, 15, 17, FP_PI2 * 0.5f, 0 }, // knees
	{ 8, 16, 18, FP_PI2 * 0.5f, 0 },
	{ 7, 5, 9, FP_PI2 * 1.8f, 1 }, // knees
	{ 8, 6, 10, FP_PI2 * 1.8f, 1 },
/*
	{ 15, 5, 7, FP_PI2 * 1.99f, 0 },
	{ 16, 6, 8, FP_PI2 * 1.99f, 0 },
	{ 17, 7, 9, FP_PI2 * 1.99f, 0 },
	{ 18, 8, 10, FP_PI2 * 1.99f, 0 },
*/
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCPlane
{
	int nP1;
	int nP2;
	int nP3;
	int nP4; // project 4 to (1,2,3) plane if distance < 0
};
const int NUM_PLANES = 4;
SCPlane planes[NUM_PLANES] = {
	{ 8, 6, 5, 10 },
	{ 5, 7, 6, 9 },
	{ 5, 3, 6, 7 },
	{ 4, 6, 5, 8 },
//	{ 8, 6, 5, 18 },
//	{ 7, 6, 5, 17 },
//	{ 3, 6, 5, 15 },
//	{ 4, 6, 5, 16 },
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SCLine
{
	int nP1;
	int nP2;
	int nP3;
};
const int NUM_LINES = 4;
SCLine lines[NUM_LINES] = {
	{ 5, 7, 15 },
	{ 7, 9, 17 },
	{ 6, 8, 14 },
	{ 8, 10, 18 },
//	{ 8, 6, 5, 18 },
//	{ 7, 6, 5, 17 },
//	{ 3, 6, 5, 15 },
//	{ 4, 6, 5, 16 },
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const int NUM_ADD_SOFTS = 2;
CAParticle::SStick addSofts[NUM_ADD_SOFTS] = {
//	{ 7, 8, 0.25f, 0 },
	{ 13, 0, 0.2f, 0 },
//	{ 13, 3, 0.2f, 0 },
//	{ 13, 5, 0.2f, 0 },
	{ 14, 0, 0.2f, 0 },
//	{ 14, 4, 0.2f, 0 },
//	{ 14, 6, 0.2f, 0 },
};
////////////////////////////////////////////////////////////////////////////////////////////////////
bool VIS_ON = false;
int NUM_ITERATIONS = 5;
int NUM_CYCLES = 5;
float FRICTION = 0.5f;
float DELTA_T = 0.05f;
DWORD DELTA_T_DWORD = (DWORD)(DELTA_T*1000);
float STIFFNESS = 0.9f; 
// stopping rule
float F_STOP_DELTA_INCREMENT = 0.15f;
int N_INCREMENT_STEP = 0;

const float F_STOP_DELTA = 0;//0.003f;

const float Z_LEVEL = -3;
const float GRAVITY = 10.f;
////////////////////////////////////////////////////////////////////////////////////////////////////
CParticleSkeleton::CParticleSkeleton( NGScene::CCInt *_pFloor, CPtrFuncBase<CFileSkeletonInfo> *_pSkeleton, 
	NWorld::CUnitServer *_pUnit )
 : pFloor(_pFloor), bStartPhys(false), bStopped(false), pUnit(_pUnit)
{
	pSkeleton = _pSkeleton;

	CFileSkeletonInfo *pSkel = pSkeleton->GetValue();
	for ( int nP = 0; nP < NUM_PARTICLES; ++nP )
	{
		int nBone = pSkel->GetBoneIndex( pszParticleBoneNames[nP] );
		nParticleBones.push_back( nBone );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::CalcParticles( vector<CVec3> *pParticles, SSkeletonPose &pose )
{
	vector<CVec3> &particles = *pParticles;
	for ( int nP = 0; nP < NUM_PARTICLES; ++nP )
	{
		int nBone = nParticleBones[nP];
		SBonePose bone = pose[nBone];
		bone.pos += shifts[nP];
		// CRAP{
		//if ( nP == 3 )
		//	bone.pos.x += 0.1f;
		//if ( nP == 4 )
		//	bone.pos.x -= 0.1f;
		// CRAP}
		bone.MakeGlobal( pose );
		particles[nP] = bone.pos;
		if ( nP > 14 )
			particles[nP] = (particles[nP-10] + particles[nP-8]) / 2;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::Init( STime t, SSkeletonPose &pose, SSkeletonPose &nextPose, const CVec3 &impact )
{
	particles.resize(NUM_PARTICLES);
	last.resize(NUM_PARTICLES);
	nStep = 0;
	
	pSkeleton.Refresh();

/*
	history.resize( NUM_PARTICLES * N_HISTORY_DEPTH );
	for ( int i = 0; i < history.size(); ++i )
		history[i] = CVec3( 1e10f, 0, 0 );
	nHistoryFrame = 0;
*/
	CVec3 imp = impact;
	Normalize(&imp);
	imp.z = 1.0f;
	Normalize(&imp);
	imp *= random.GetFloat( 3, 7 );

	CFileSkeletonInfo *pSkel = pSkeleton->GetValue();
	defaultPose.resize( pSkel->bones.size() );
	for ( int i = 0; i < defaultPose.size(); ++i )
	{
		defaultPose[i].nParent = pSkel->bones[i].nParent;
		defaultPose[i].pos = pSkel->bones[i].pos;
		defaultPose[i].rot = pSkel->bones[i].rot;
	}
	CalcParticles( &particles, defaultPose );

	fInvMasses.resize(NUM_PARTICLES);
	for ( int nP = 0; nP < NUM_PARTICLES; ++nP )
		fInvMasses[nP] = fInitInvMasses[nP];
	if ( IsValid( pEffector ) )
	{
		SBonePose boneSpine = defaultPose[nParticleBones[3]];
		boneSpine.MakeGlobal( defaultPose );
		boneSpine.rot.UnitInverse();
		for ( int i = 0; i < 5; ++i )
		{
			CVec3 p = boneSpine.rot.Rotate( particles[i] - boneSpine.pos );
			CQuat r1( 0, CVec3(0,0,1) );
			spine.push_back( r1.Rotate(p)/* + CVec3(0,0.3f,0)*/ );
			fInvMasses[i] = 0;
		}
	}

	sticks.resize( NUM_STICKS );
	for ( int nS = 0; nS < NUM_STICKS; ++nS )
	{
		SStick &stick = sticks[nS];
		stick.nP1 = nPairs[ nS * 2 ];
		stick.nP2 = nPairs[ nS * 2 + 1 ];
		stick.fRest = fabs(particles[ sticks[nS].nP2 ] - particles[ sticks[nS].nP1 ]);
		stick.fSumInvMasses = fInvMasses[ stick.nP1 ] + fInvMasses[ stick.nP2 ];
	}
	softs.resize( NUM_ANGLES + NUM_ADD_SOFTS );
	for ( int nS = 0; nS < NUM_ANGLES; ++nS )
	{
		softs[nS].nP1 = angles[nS].nP2;
		softs[nS].nP2 = angles[nS].nP3;
		float a = fabs( particles[ angles[nS].nP2 ] - particles[ angles[nS].nP1 ] );
		float b = fabs( particles[ angles[nS].nP3 ] - particles[ angles[nS].nP1 ] );
		float c = a * a + b * b - 2 * a * b * cos( angles[nS].fAngle );
		if ( c < 0 )
			c = 0;
		softs[nS].fRest = sqrt(c);
		softs[nS].nMax = angles[nS].nMax;
	}
	for ( int nS = 0; nS < NUM_ADD_SOFTS; ++nS )
	{
		softs[nS + NUM_ANGLES].nP1 = addSofts[nS].nP1;
		softs[nS + NUM_ANGLES].nP2 = addSofts[nS].nP2;
		softs[nS + NUM_ANGLES].fRest = addSofts[nS].fRest;
		softs[nS + NUM_ANGLES].nMax = addSofts[nS].nMax;
	}
	CalcParticles( &particles, nextPose );
	CalcParticles( &last, pose );

	for ( int i = 0; i < pSkeleton->GetValue()->bones.size(); ++i )
	{
		string pszBoneName = pSkeleton->GetValue()->bones[i].szName;
		if ( pszBoneName == "Hip" )
			restore.push_back( SRestoreBone(5, 0xFF, 6) );
		else if ( pszBoneName == "Spine" )
			restore.push_back( SRestoreBone(1, 0xFF, 2) );
		else if ( pszBoneName == "L_UArm" )
			restore.push_back( SRestoreBone(1, 11) );
		else if ( pszBoneName == "L_LArm" )
			restore.push_back( SRestoreBone(11, 13) );
		else if ( pszBoneName == "R_UArm" )
			restore.push_back( SRestoreBone(2, 12) );
		else if ( pszBoneName == "R_LArm" )
			restore.push_back( SRestoreBone(12, 14) );
		else if ( pszBoneName == "L_Thigh" )
			restore.push_back( SRestoreBone(5, 7) );
		else if ( pszBoneName == "L_Leg" )
			restore.push_back( SRestoreBone(7, 9) );
		else if ( pszBoneName == "R_Thigh" )
			restore.push_back( SRestoreBone(6, 8) );
		else if ( pszBoneName == "R_Leg" )
			restore.push_back( SRestoreBone(8, 10) );
		else
			restore.push_back( SRestoreBone() );
	}
	tCurrent = t;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::GetInputFrame( STime t, SSkeletonPose *pPose )
{
	pInput->GetFrame( t, pPose );
	/*if ( addRandom.empty() )
	{
		addRandom.resize( pPose->size(), QNULL );
		SRand rnd;
		for ( int nBoneNum = 0; nBoneNum < 3; ++nBoneNum )
		{
			int nBone = nParticleBones[nBoneNum];
			CVec3 axis( rnd.GetFloat(-1,1), rnd.GetFloat(-1,1), rnd.GetFloat(-1,1) );
			addRandom[ nBone ] = CQuat( random.GetFloat(-0.7f, 0.7f), axis, true );
		}
	}
	for ( int i = 0; i < addRandom.size(); ++i )
	{
		SBonePose &pose = (*pPose)[i];
		pose.rot *= addRandom[i];
	}*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::CheckIfCollided( STime t, SSkeletonPose *pPose, SSkeletonPose *pNextPose )
{
	ASSERT( tActive <= tMax );
	SSkeletonPose &pose = *pPose;
	SSkeletonPose &nextPose = *pNextPose;
	pose.resize( pPose->size() );
	GetInputFrame( t, &pose );
	if ( t < tActive )
		return;
	nextPose.resize( pPose->size() );
	STime tNextStep = t + DELTA_T_DWORD;
	if ( tNextStep > tMax )
		tNextStep = t;
	GetInputFrame( tNextStep, &nextPose );
	if ( t > tMax - 2*DELTA_T_DWORD )
	{
		bStartPhys = true;
		return;
	}
	vector<CVec3> centers, vels;
	centers.resize( NUM_PARTICLES );
	vels.resize( NUM_PARTICLES );
	CalcParticles( &centers, pose );
	CalcParticles( &vels, nextPose );
	vector<SSphere> spheres;
	spheres.resize( NUM_PARTICLES );
	float fLen = 0;
	for ( int i = 0; i < NUM_PARTICLES; ++i )
	{
		spheres[i].ptCenter = centers[i];
    spheres[i].fRadius = fRadiuses[i];
		vels[i] -= centers[i];
		fLen = Max( fLen, fabs( vels[i] ) );
	}
	if ( VIS_ON ) // test visualization
	{
		sphereParticles.clear();
		for ( int nP = 0; nP < spheres.size(); ++nP )
			sphereParticles.push_back( spheres[nP] );
	}
	ASSERT( fLen > 0.0f && "Художники бажат, анимация зависшего трупа" );
	if ( fLen < 0.01f * DELTA_T )
	{
		bStartPhys = true;
		return;
	}
	vector<NAI::SCollisionPoint> res;
	NAI::PhysCollideInfo( pMap, spheres, vels, &res, NWorld::TS_ITEM_BLOCKER );
	for ( int i = 0; i < NUM_PARTICLES; ++i )
	{
		if ( i == 9 || i == 10 )
			continue; // feet do not collide :)
		if ( res[i].fDist != NAI::FP_NO_COLLISION )
			bStartPhys = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::GetFrame( STime t, SSkeletonPose *pPose )
{
	SSkeletonPose nextPose;
	if ( !bStartPhys )
		CheckIfCollided( t, pPose, &nextPose );
	if ( !bStartPhys )				
		return;
	else if ( last.empty() )
		Init( t + DELTA_T_DWORD, *pPose, nextPose, impact );

	while ( t >= tCurrent )
	{
		tCurrent += DELTA_T_DWORD;
		Step();
	}

	pSkeleton.Refresh();

	if ( bStopped )
	{
		for ( int i = 0; i < pPose->size(); ++i )
		{
			(*pPose)[i].nParent = pSkeleton->GetValue()->bones[i].nParent;
			(*pPose)[i].pos = pSkeleton->GetValue()->bones[i].pos;
			(*pPose)[i].rot = pSkeleton->GetValue()->bones[i].rot;
		}
		RestoreAll( particles, pPose );
		return;
	}

	SSkeletonPose poseLast, poseCurrent;
	poseCurrent.resize( pSkeleton->GetValue()->bones.size() );
	for ( int i = 0; i < poseCurrent.size(); ++i )
	{
		poseCurrent[i].nParent = pSkeleton->GetValue()->bones[i].nParent;
		poseCurrent[i].pos = pSkeleton->GetValue()->bones[i].pos;
		poseCurrent[i].rot = pSkeleton->GetValue()->bones[i].rot;
	}
	poseLast = poseCurrent;
	RestoreAll( last, &poseLast );
	RestoreAll( particles, &poseCurrent );
	float val = 1 - float(tCurrent - t) / DELTA_T_DWORD;
	for ( int i = 0; i < pPose->size(); ++i )
		(*pPose)[i].Interpolate( *pPose, val, poseLast[i], poseCurrent[i] );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::Step()
{
	++nStep;
	if ( bStopped )
		return;
	CVec3 gravity(0,0,-GRAVITY);

	int nParticles = nParticleBones.size();

	for ( int nP = 0; nP < nParticles; ++nP )
	{
		CVec3 speed = particles[nP] - last[nP];
		if ( IsValid( pEffector ) )//&& ( nP == 9 || nP == 10 ) )
		{
			float fSpeed = fabs( speed );
			if ( fSpeed > 0.1f )
			{
				//DebugTrace( "Speed downsized from %f\n", fSpeed * 10 );
				speed *= ( 0.1f / fSpeed );
			}
		}
		CVec3 future = particles[nP] + speed + gravity * DELTA_T * DELTA_T;
		last[nP] = particles[nP];
		particles[nP] = future;
		if ( particles[nP].z < Z_LEVEL )
		{
			particles[nP] -= FRICTION * (particles[nP] - last[nP]);
			particles[nP].z = Z_LEVEL;
		}
	}

	// effector
	SSkeletonPose effPose;
	if ( IsValid( pEffector ) )
	{
		effPose.resize(1);
		pEffector->GetFrame( tCurrent, &effPose );
		for ( int i = 0; i < 5; ++i )
			particles[i] = effPose[0].rot.Rotate( spine[i] ) + effPose[0].pos;
	}

	vector<SSphere> spheres;
	vector<CVec3> velocities;
	vector<CVec3> results;

	spheres.resize( nParticles );
	velocities.resize( nParticles );
	results.clear();


	vector<CVec3> forCollisions;
	forCollisions.resize( nParticles );
	for ( int nP = 0; nP < nParticles; ++nP )
		forCollisions[nP] = last[nP];

	for ( int nC = 0; nC < NUM_CYCLES; ++nC )
	{

	// collision
	for ( int nP = 0; nP < nParticles; ++nP )
	{
		spheres[nP].fRadius = fRadiuses[nP];
		spheres[nP].ptCenter = forCollisions[nP];
		velocities[nP] = particles[nP] - forCollisions[nP];
	}
	results.clear();
	int nResFloor;
	NAI::PhysCollideSliding( pMap, spheres, velocities, &results, NWorld::TS_ITEM_BLOCKER, &nResFloor );
	if ( nResFloor < 100 )
		pFloor->Set( nResFloor );
	for ( int nP = 0; nP < nParticles; ++nP )
		particles[nP] = forCollisions[nP] = results[nP];

	// constraints
	for ( int nI = 0; nI < NUM_ITERATIONS; ++nI )
	{
		if ( IsValid( pEffector ) )
		{
			effPose.resize(1);
			pEffector->GetFrame( tCurrent, &effPose );
			for ( int i = 0; i < 5; ++i )
				particles[i] = effPose[0].rot.Rotate( spine[i] ) + effPose[0].pos;
		}

		// for case
		for ( int nP = 0; nP < nParticleBones.size(); ++nP )
		{
			if ( particles[nP].z < Z_LEVEL )
				particles[nP].z = Z_LEVEL;
		}

		{
			CVec3 &p1 = particles[8], &p2 = particles[6], &p3 = particles[5], &p4 = particles[10];
			CVec3 n1 = p2 - p1;
			Normalize(&n1);
			CVec3 n2 = p3 - p2;
			n2 = n2 - (n2 * n1) * n1;
			Normalize(&n2);
			SPlane plane;
			plane.Set( n2, 0 );
			plane.RecalcDist( p2 );
			float d = STIFFNESS * plane.GetDistanceToPoint( p4 ) * 0.25f;
			p1 += d * plane.n;
			p2 += d * plane.n;
			p3 += d * plane.n;
			p4 -= (3 * d) * plane.n;
		}
		{
			CVec3 &p1 = particles[7], &p2 = particles[5], &p3 = particles[6], &p4 = particles[9];
			CVec3 n1 = p2 - p1;
			Normalize(&n1);
			CVec3 n2 = p3 - p2;
			n2 = n2 - (n2 * n1) * n1;
			Normalize(&n2);
			SPlane plane;
			plane.Set( n2, 0 );
			plane.RecalcDist( p2 );
			float d = STIFFNESS * plane.GetDistanceToPoint( p4 ) * 0.25f;
			p1 += d * plane.n;
			p2 += d * plane.n;
			p3 += d * plane.n;
			p4 -= (3 * d) * plane.n;
		}
		
		// planes
		for ( int nS = 0; nS < NUM_PLANES; ++nS )
		{
			CVec3 &p1 = particles[ planes[nS].nP1 ], &p2 = particles[ planes[nS].nP2 ],
			      &p3 = particles[ planes[nS].nP3 ], &p4 = particles[ planes[nS].nP4 ];
			SPlane plane;
			plane.Set( p1, p2, p3 );
			float d = plane.GetDistanceToPoint( p4 );
			if ( d < 0 )
			{
				d *= STIFFNESS * 0.25f;
				p1 += d * plane.n;
				p2 += d * plane.n;
				p3 += d * plane.n;
				p4 -= (3 * d) * plane.n;
			}
		}
/*
		// lines
		for ( int nS = 0; nS < NUM_LINES; ++nS )
		{
			CVec3 &p1 = particles[ lines[nS].nP1 ], &p2 = particles[ lines[nS].nP2 ],
			      &p3 = particles[ lines[nS].nP3 ];
			CVec3 n = p2 - p1;
			if ( Normalize(&n) )
			{
				float k = (p3 - p1) * n;
				CVec3 p = p1 + k * n;
				
				CVec3 d = 0.333f * STIFFNESS * (p3 - p);
				p1 += d;
				p2 += d;
				p3 -= 2 * d;
				
				//CVec3 d = STIFFNESS * (p3 - p);
				//p3 -= d;
			}
		}
*/
		// angles
		for ( int nS = 0; nS < softs.size(); ++nS )
		{
			CVec3 &pos1 = particles[ softs[nS].nP1 ];
			CVec3 &pos2 = particles[ softs[nS].nP2 ];
			CVec3 delta = pos2 - pos1;
			float fDelta = fabs(delta);
			if ( fDelta < 1e-6f )
			{
				fDelta = 1e-6f;
				delta = CVec3(fDelta,0,0);
			}
			if ( (softs[nS].nMax == 0 && fDelta < softs[nS].fRest) || (softs[nS].nMax == 1 && fDelta > softs[nS].fRest) )
			{
				float fDiff = STIFFNESS * (fDelta - softs[nS].fRest) / fDelta / (fInvMasses[softs[nS].nP1] + fInvMasses[softs[nS].nP2]);
				pos1 += fInvMasses[softs[nS].nP1] * fDiff * delta;
				pos2 -= fInvMasses[softs[nS].nP2] * fDiff * delta;
			}
		}
			
		// sticks
		for ( vector<SStick>::iterator it = sticks.begin(); it != sticks.end(); ++it )
		{
			if ( it->fSumInvMasses == 0 )
				continue;
			CVec3 &pos1 = particles[ it->nP1 ];
			CVec3 &pos2 = particles[ it->nP2 ];
			CVec3 delta = pos2 - pos1;
			float fDelta = fabs(delta);
			/*if ( fDelta < 1e-6f )
			{
				fDelta = 1e-6f;
				delta = CVec3(fDelta,0,0);
			}*/
			float fDiff = STIFFNESS * ( fDelta - it->fRest ) / fDelta / it->fSumInvMasses;
			pos1 += fInvMasses[ it->nP1 ] * fDiff * delta;
			pos2 -= fInvMasses[ it->nP2 ] * fDiff * delta;
		}

	} // NUM_ITERATIONS

	} // NUM_CYCLES
	
	if ( !IsValid( pEffector ) )
	{
		/*
		for ( int nH = 0; nH < N_HISTORY_DEPTH; ++nH )
		{
			bStopped = true;
			for ( int nP = 0; nP < nParticles; ++nP )
			{
				if ( fabs2( particles[nP] - history[nH * nParticles + nP] ) > sqr(F_STOP_DELTA) )
					bStopped = false;
			}
			if ( bStopped )
				break;
		}
		//
		for ( int nP = 0; nP < nParticles; ++nP )
		{
			bStopped = false;
			for ( int nH = 0; nH < N_HISTORY_DEPTH; ++nH )
			{
				if ( fabs2( particles[nP] - history[nH * nParticles + nP] ) < sqr(F_STOP_DELTA) )
				{
					bStopped = true;
					break;
				}
			}
			if ( !bStopped )
				break;
		}
*/

		if ( nStep > 9.5f / DELTA_T )
			bStopped = true;
		else
		{
			float fStopDelta = F_STOP_DELTA;
			if ( nStep > N_INCREMENT_STEP * DELTA_T )
				fStopDelta = F_STOP_DELTA + (nStep / DELTA_T - N_INCREMENT_STEP) * F_STOP_DELTA_INCREMENT / 10000.0f;
			bStopped = true;
			for ( int nP = 0; nP < nParticles; ++nP )
			{
				if ( fabs2(particles[nP] - last[nP]) > sqr(fStopDelta) )
					bStopped = false;
			}
		}
		if ( bStopped )
			BeStopped();
		/*
		else
		{
			for ( int nP = 0; nP < nParticles; ++nP )
				history[nHistoryFrame * nParticles + nP] = particles[nP];
			nHistoryFrame = (nHistoryFrame + 1) % N_HISTORY_DEPTH;
		}
		*/
	}

	if ( VIS_ON ) // test visualization
	{
		sphereParticles.clear();
		for ( int nP = 0; nP < nParticles; ++nP )
			sphereParticles.push_back( SSphere( particles[nP], fRadiuses[nP] ) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CParticleSkeleton::BeStopped()
{
	SBoundCalcer bc;
	for ( int nP = 0; nP < nParticleBones.size(); ++nP )
	{
		last[nP] = particles[nP];
		bc.Add( last[nP], fRadiuses[nP] );
	}
	SBound b;
	bc.Make( &b );
	pMap->GetStabilityTrackers()->AddCorpse( pUnit, b );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CParticleSkeleton::NeedUpdate( STime t )
{
	return !bStopped;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
string CParticleSkeleton::GetDebugInfo() const 
{ 
	string ret = "Particle skeleton (dead body)";
	if ( IsValid( pEffector ) )
	{
		ret += "\n  Is carried, carrier: ";
		ret += pEffector->GetDebugInfo();
	}
	if ( IsValid( pInput ) )
	{
		char buf[50];
		sprintf( buf, "\n  Has input animator until time %d: ", tMax );
		ret += buf;
		ret += pInput->GetDebugInfo();
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// CASphereSet
////////////////////////////////////////////////////////////////////////////////////////////////////
const int N_PHYSICS_SCALE = 65536;
const int N_PHYSICS_STEP = 20;
CASphereSet::CASphereSet( const vector<SMassSphere> &_spheres, const CVec3 &boundCenter, 
	const CVec3 &_boundSize, CObjectBase *_pIgnore, int _nFloor )	: nFloor(_nFloor) 
	//massCenter( boundCenter ), boundSize(_boundSize), pIgnore(_pIgnore)
{
	//bCollided = false;
	//spheres = _spheres;
	if ( _spheres.empty() )
	{
		// default
		float fDefaultSize = 0.1f;
		AddSphere( SSphere( CVec3( fDefaultSize,0, fDefaultSize), fDefaultSize), 1 );
		AddSphere( SSphere( CVec3( fDefaultSize,0,-fDefaultSize), fDefaultSize), 1 );
		AddSphere( SSphere( CVec3(-fDefaultSize,0, fDefaultSize), fDefaultSize), 1 );
		//AddSphere( SSphere( CVec3(-fDefaultSize,0,-fDefaultSize), fDefaultSize), 1 );
	}
	else
	{
		for ( int k = 0; k < _spheres.size(); ++k )
			AddSphere( SSphere( _spheres[k].ptCenter, _spheres[k].fRadius ), _spheres[k].fMass );
	}
	animator.StartNew();
	animator.SetGravity( Float2Int( 9.8f * N_PHYSICS_SCALE / sqr( 1024.0f / N_PHYSICS_STEP ) ) );
	massCenter = animator.CalcMassCenter() / N_PHYSICS_SCALE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::AddSphere( const SSphere &sphere, float fMass )
{
	int nPoint = animator.AddPoint( 
		sphere.ptCenter.x * N_PHYSICS_SCALE, 
		sphere.ptCenter.y * N_PHYSICS_SCALE,
		sphere.ptCenter.z * N_PHYSICS_SCALE,
		sphere.fRadius );
	for ( int k = 0; k < nPoint; ++k )
		animator.AddLink( k, nPoint );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::Init( STime t, const CVec3 &position, const CQuat &rotation, 
	const CVec3 &velocity, bool bMassCenter, EPhysCase physCase, NRPG::IInventoryItem *pItem )
{
	tCurrent = t;
	animator.SetPosition( position * N_PHYSICS_SCALE, rotation );
	animator.SetVelocity( velocity * N_PHYSICS_SCALE / 1024 * N_PHYSICS_STEP, CVec3(0,0,0) );

	// calc center of mass
/*	fMass = 0;
	massCenter = VNULL3;
	for ( int i=0; i<spheres.size(); ++i )
	{
		massCenter += spheres[i].ptCenter * spheres[i].fMass;
		fMass += spheres[i].fMass;
	}
	massCenter /= fMass;
	// move center of mass to zero
	for ( int i=0; i<spheres.size(); ++i )
		spheres[i].ptCenter -= massCenter;

	fBoundSize = 0;
	for ( int i=0; i<spheres.size(); ++i )
		fBoundSize = Max( fBoundSize, fabs(spheres[i].ptCenter) + spheres[i].fRadius );

	SHMatrix inertia;

	float fSizeLimit = 20.f;
	float xs = boundSize.x, ys = boundSize.y, zs = boundSize.z;
	float xy = xs / ys;
	if ( xy < 1 )
		xy = 1 / xy;
	float yz = ys / zs;
	if ( yz < 1 )
		yz = 1 / yz;
	float zx = zs / xs;
	if ( zx < 1 )
		zx = 1 / zx;
	bool bBrick = false;
	if ( bMassCenter || xy > fSizeLimit || yz > fSizeLimit || zx > fSizeLimit )
		bBrick = true;
	else
	{
		memset( &inertia, 0, sizeof(SHMatrix) );
		// treat spheres as particles
		for ( int i=0; i<spheres.size(); ++i )
		{
			CVec3 r = spheres[i].ptCenter;
			float m = spheres[i].fMass;

			inertia._11 += m * (r.y * r.y + r.z * r.z);
			float t = m * r.x * r.y;
			inertia._12 -= t;
			inertia._21 -= t;

			inertia._22 += m * (r.x * r.x + r.z * r.z);
			t = m * r.x * r.z;
			inertia._13 -= t;
			inertia._31 -= t;

			inertia._33 += m * (r.x * r.x + r.y * r.y);
			t = m * r.y * r.z;
			inertia._23 -= t;
			inertia._32 -= t;
		}
	}
	
	if ( bBrick || !inertiaInvBody.HomogeneousInverse( inertia ) )
	{
		// inertia matrix as if it is a brick
		memset( &inertia, 0, sizeof(SHMatrix) );
		inertia._11 = fMass * (ys * ys + zs * zs) / 12.f;
		inertia._22 = fMass * (xs * xs + zs * zs) / 12.f;
		inertia._33 = fMass * (ys * ys + xs * xs) / 12.f;
		inertiaInvBody.HomogeneousInverse( inertia );
	}
	lastRot = rot = rotation;
	if ( bMassCenter ) // for grenades
		lastPos = pos = position;
	else
		lastPos = pos = position + rot.Rotate( massCenter );
	p = fMass * velocity;
	l.x = random.GetFloat(-2.0f,2.0f);
	l.y = random.GetFloat(-2.0f,2.0f);
	l.z = random.GetFloat(-2.0f,2.0f);
	//l = VNULL3;
	//l.y = 0.5f;
	inertia.RotateVector( &l, l );
	CalcVelocities();
	tCurrent = t;
	tLast = tCurrent + 15000;
	bStopped = false;

	bool bGrenade = false, bWeapon = false; 
	if ( CDynamicCast<NRPG::IGrenadeItemInfo>( pItem ) )
		bGrenade = true;
	else if ( CDynamicCast<NRPG::IWeaponItemInfo>( pItem ) )
		bWeapon = true;

	const char *name = "Default";
	switch ( physCase )
	{
		case PH_GRENADE_ATTACK: 
			name = "GrenadeAttack"; break;
		case PH_FALL_FROM_BODY: 
			if ( bGrenade )
				name = "GrenadeFallFromBody"; 
			else if ( bWeapon )
				name = "WeaponFallFromBody"; 
			else
				name = "ItemFallFromBody"; 
			break;
		case PH_THROW_OUT:
			bool bLong = false;
			if ( boundSize.z > 4*boundSize.x && boundSize.z > 4*boundSize.y )
				bLong = true;
			if ( boundSize.y > 4*boundSize.x && boundSize.y > 4*boundSize.z )
				bLong = true;
			if ( boundSize.x > 4*boundSize.y && boundSize.x > 4*boundSize.z )
				bLong = true;

			if ( bLong )
				name = "LongWeaponThrowOut"; 
			else if ( bGrenade )
				name = "GrenadeThrowOut"; 
			else if ( bWeapon )
				name = "WeaponThrowOut"; 
			else
				name = "ItemThrowOut"; 
			break;
	}

	csSystem << "Starting physics case " << name << endl;
	pPhys = NDb::GetDBPhysParams( name );
	*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::Calc( STime t )
{
	while ( t >= tCurrent )//&& !bStopped )
	{
		tCurrent += N_PHYSICS_STEP;//pPhys->DELTA_T_DWORD;
		Step();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CASphereSet::HasStopped()
{
	return animator.HasStopped();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::GetFrame( STime t, SSkeletonPose *pPose )
{
	Calc( t );
	//
	CVec3 pos;
	CQuat rot;
	animator.GetPosition( &pos, &rot );
	pos /= N_PHYSICS_SCALE;
	(*pPose)[0].nParent = -1;
	(*pPose)[0].pos = pos + massCenter - rot.Rotate( massCenter );
	(*pPose)[0].rot = rot;
/*	if ( bStopped )
	{
		(*pPose)[0].nParent = -1;
		(*pPose)[0].pos = pos - rot.Rotate( massCenter );
		(*pPose)[0].rot = rot;
		return;
	}
	CVec3 posI;
	CQuat rotI;
	float fI = float(tCurrent - t) / pPhys->DELTA_T_DWORD;
	posI.Interpolate( pos, lastPos, fI );
	rotI.Interpolate( rot, lastRot, fI );
	(*pPose)[0].nParent = -1;
	(*pPose)[0].pos = posI - rotI.Rotate( massCenter );
	(*pPose)[0].rot = rotI;*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*struct SCollision
{
	NAI::SCollisionPoint res;
	CVec3 vel;
	float fPart;
	CVec3 center;
};
struct SCollisionFunctional
{
	bool operator()( const SCollision &a, const SCollision &b ) const 
	{ 
		return ( a.fPart > b.fPart ); 
	}
};*/
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CASphereSet::BeStopped()
{
	bStopped = true;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
class CASphereCollider : public NPhysics::ICollider
{
	NAI::CPhysCollider collider;
public:
	CASphereCollider( NAI::IAIMap *_pAIMap, const CVec3 &vMin, const CVec3 &vMax, int nFlags )
	{
		SBound bound;
		bound.BoxInit( vMin, vMax );
		_pAIMap->PrepareCollider( &collider, bound, 0.2f, nFlags );
	}
	bool Collide( const NPhysics::CPos &pos, float fRadius, const NPhysics::CPos &delta, CVec4 *pPlane )
	{
		NAI::SCollisionPoint res;
		collider.CollideInfo( SSphere( pos.GetVec3() / N_PHYSICS_SCALE, fRadius ), delta.GetVec3() / N_PHYSICS_SCALE, &res );
		if ( res.fDist == NCollider::F_NO_COLLISION )
			return false;
		pPlane->x = res.plane.n.x;
		pPlane->y = res.plane.n.y;
		pPlane->z = res.plane.n.z;
		pPlane->w = ( res.plane.d - fRadius ) * N_PHYSICS_SCALE;
		return true;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::Step()
{
	if ( HasStopped() )
		return;
	int nFlags = NWorld::TS_ITEM_BLOCKER;
	//if ( IsValid( pIgnore ) )
//		nFlags |= NWorld::TS_UNITS;

	CVec3 vCenter = animator.CalcMassCenter() / N_PHYSICS_SCALE;
	float f = 1;
	CASphereCollider collider( pMap, vCenter - CVec3(f,f,f), vCenter + CVec3(f,f,f), nFlags );
	animator.Step( &collider );
/*	int nCurFloor = 100;
	//DebugTrace( "STEP ******************************************************\n" );
	lastPos = pos;
	lastRot = rot;

	int nCycle = 0;
	float fTimeLeft = pPhys->DELTA_T;
	float fMinTimeStep = pPhys->DELTA_T * 0.05f;

	float fTimeStep = min( fTimeLeft, max( GetEnergy(), fMinTimeStep ) );

	CVec3 predict = PredictPosition( fTimeLeft, true );
	SSphere check( (pos + predict) / 2.f, fBoundSize + fabs(pos - predict) / 2.f );
	int nFlags = NWorld::TS_ITEM_BLOCKER;
	if ( IsValid( pIgnore ) )
		nFlags |= NWorld::TS_UNITS;
	if ( !pMap->CalcIntersection( check.ptCenter, check.fRadius, nFlags, pIgnore ) )
	{
		AdvanceIntegrator( fTimeLeft, true );
		return;
	}

	while ( fTimeLeft > 1e-4f && nCycle++ < 32 )
	{
		//float fEnergy = GetEnergy();
		//DebugTrace( "Timeleft: %f  Step: %f  Energy: %f\n", fTimeLeft, fTimeStep, fEnergy );
		vector<SSphere> poss;
		poss.resize( spheres.size() );
		vector<CVec3> vels;
		vels.resize( spheres.size() );
		vector<NAI::SCollisionPoint> ress;
		float fLong = 1.f;
		bool bShort = true;
		for ( int i=0; i<spheres.size(); ++i )
		{
			CVec3 global = rot.Rotate( spheres[i].ptCenter );
			poss[i].ptCenter = pos + global;
			poss[i].fRadius = spheres[i].fRadius;
			vels[i] = fTimeStep * (posVel + (rotVel ^ global));
			float fSpeed = fabs(vels[i]);
			fLong = Max( fLong, fSpeed * 0.5f / poss[i].fRadius );
			if ( fSpeed > 0.05f * fTimeStep )
				bShort = false;
		}
		if ( fLong > 1.01f )
		{
			fTimeStep /= fLong;
			continue;
		}
		if ( bShort )
		{
			BeStopped();
			break;
		}
		if ( VIS_ON ) // test visualization
		{
			//sphereParticles.clear();
			//for ( int i = 0; i < poss.size(); ++i )
			//	sphereParticles.push_back( poss[i] );
		}
		NAI::PhysCollideInfo( pMap, poss, vels, &ress, nFlags );
		vector<SCollision> colls;
		for ( int i=0; i<ress.size(); ++i )
		{
			float fVel = fabs(vels[i]);
			if ( fVel > 1e-3f && ress[i].fDist != NAI::FP_NO_COLLISION )
			{
				if ( pIgnore && ress[i].pSrc && ress[i].pSrc->pUserData == pIgnore && !bCollided )
					continue;
				SCollision c;
				c.res = ress[i];
				c.vel = vels[i];
				c.center = poss[i].ptCenter;
				c.res.fDist = Clamp( ress[i].fDist, 0.0f, fVel );
				c.fPart = c.res.fDist / fVel;
				colls.push_back(c);
				nCurFloor = Min( nCurFloor, c.res.pSrc->nFloor );
			}
			//DebugTrace( "--- Vel: %f\n", fVel / fTimeStep );
		}

		bool bCollision = false;
		sort( colls.begin(), colls.end(), SCollisionFunctional() );
		int nContactPoints = colls.size();
		//DebugTrace( "Collisions: %d\n", colls.size() );
		while ( !bCollision && !colls.empty() ) //!bCollision &&
		{
			SCollision c = colls.back();
			// collision response
			CVec3 back = - c.vel;
			Normalize(&back);
			back *= c.res.fDist;
			CVec3 n = c.center - c.res.pt - back; // normal direction
			Normalize(&n);

			//AdvanceIntegrator( c.fPart * fTimeStep );
			//fTimeLeft -= c.fPart * fTimeStep;
			
			CVec3 ptColl = c.res.pt + back - pos;//c.center - pos;
			CVec3 ptCollVel = posVel + (rotVel ^ ptColl);
			float fScal = n * ptCollVel;			

			colls.pop_back();
			//for ( int i = 0; i < colls.size(); ++i )
			//	colls[i].fPart -= c.fPart;
							
			if ( fScal < -1e-3f ) // check opposite direction
			{
				CVec3 velChange;
				if ( fabs2(ptCollVel) < sqr(0.1f) )
				{
					// microcollision
					velChange = - ptCollVel;
				}
				else
				{
					// normal
					float fNormVelChange = - (1 + pPhys->fResistance) * fScal;
					velChange = fNormVelChange * n;
					// tangent
					CVec3 tangVel = ptCollVel - fScal * n;
					velChange -= pPhys->fFriction * tangVel;
				}
				//velChange = - ptCollVel;
				if ( NWorld::CheckItemsBreakGlass( c.res.pSrc ) )
				{
					ApplyCollision( ptColl, velChange );
					bCollision = true;
					bCollided = true;
				}
			}
		}
		if ( !bCollision )
		{
			AdvanceIntegrator( fTimeStep );
			fTimeLeft -= fTimeStep;
		}
		float fEnergy = GetEnergy();
		if ( fEnergy < 1e-4f || ( fEnergy < 1e-3f && nContactPoints >= 3 ) )
		{
			BeStopped();
			break;
		}
		fTimeStep = min( fTimeLeft, max( fEnergy, fMinTimeStep ) );
	}
	if ( nCurFloor < 100 )
		nFloor = nCurFloor;
		*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*float CASphereSet::GetEnergy()
{
	return ((posVel * p) + (rotVel * l)) / fMass;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CASphereSet::ApplyCollision( CVec3 ptColl, CVec3 vel )
{
	SHMatrix mxR, mxTemp;
	SHMatrix mx, mxInv;
	CVec3 &r = ptColl;
	memset( &mxR, 0, sizeof(SHMatrix) );
	mxR._12 = -r.z;
	mxR._13 = r.y;
	mxR._23 = -r.x;
	mxR._21 = r.z;
	mxR._31 = -r.y;
	mxR._32 = r.x;
	Multiply( &mxTemp, inertiaInv, mxR );
	Multiply( &mx, mxR, mxTemp );
	float fInvMass = 1 / fMass;
	mx._11 -= fInvMass;
	mx._22 -= fInvMass;
	mx._33 -= fInvMass;
	mxInv.HomogeneousInverse( mx );
	CVec3 impulse;
	mxInv.RotateVector( &impulse, vel );
	p -= impulse;
	l -= r ^ impulse;
	CalcVelocities();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::CalcVelocities()
{
	CalcPosVel();
	CalcRotVel();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::CalcPosVel()
{
	posVel = p / fMass;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::CalcRotVel()
{
	SHMatrix m, temp;
	memset( &m, 0, sizeof(SHMatrix) );
	rot.DecompEulerMatrix( &m );
	Multiply( &temp, m, inertiaInvBody );
	Transpose( &m, m );
	Multiply( &inertiaInv, temp, m ); // current inverted inertia matrix
	inertiaInv.RotateVector( &rotVel, l ); // angular velocity

	float fRotSpeed = fabs(rotVel);
	if ( fRotSpeed > 20.f )
	{
		rotVel *= 20.f / fRotSpeed;
		l *= 20.f / fRotSpeed;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CVec3 CASphereSet::PredictPosition( float fTime, bool bMidPoint )
{
	if ( fTime < 1e-4f )
		return pos;
	CVec3 gravity(0,0,-GRAVITY);
	if ( bMidPoint )
		return pos + fTime * (posVel + gravity * fTime * 0.5f);
	else
		return pos + fTime * posVel;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CASphereSet::AdvanceIntegrator( float fTime, bool bMidPoint )
{
	if ( fTime < 1e-4f )
		return;
	CVec3 gravity(0,0,-GRAVITY);
	if ( bMidPoint )
		pos = pos + fTime * (posVel + gravity * fTime * 0.5f);
	else
		pos = pos + fTime * posVel;
	p = p + fMass * gravity * fTime;
	CalcPosVel();

	CVec3 dRotVel;
	float fRotVel;
	CQuat oldRot = rot;
	dRotVel = rotVel * (fTime * 0.5f);
	fRotVel = fabs(dRotVel);
	if ( fRotVel > 1e-6f )
	{
		CQuat dq( fRotVel, dRotVel, true );
		rot = dq * rot;
	}
	CalcRotVel();
	dRotVel = rotVel * fTime;
	fRotVel = fabs(dRotVel);
	if ( fRotVel > 1e-6f )
	{
		CQuat dq( fRotVel, dRotVel, true );
		rot = dq * oldRot;
	}
	else
		rot = oldRot;
	Normalize( &rot );
	CalcRotVel();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CASphereSet::HasStopped()
{
	if ( bStopped )
		return true;
	if ( pos.z < Z_LEVEL )
		BeStopped();
	STime t = pTime->GetValue();
	if (t > tLast)
		BeStopped();
	return bStopped;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// CATrailPath
////////////////////////////////////////////////////////////////////////////////////////////////////
void CATrailPath::Init( STime _sCast, const vector<STrailPoint> &_trail )
{
	sCast = _sCast;
	trailpointsSet = _trail;
	nTrailCount = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CATrailPath::GetFrame( STime sFrameTime, SSkeletonPose *pPose )
{
	if ( sFrameTime >= sCast )
	{
		for ( int nTemp = nTrailCount + 1; nTemp < trailpointsSet.size(); nTemp++ )
		{
			if ( trailpointsSet[nTemp].sPassTime > sFrameTime )
				break;

			nTrailCount = nTemp;
		}

		const STrailPoint &sCurrent = trailpointsSet[nTrailCount];

		SHMatrix sMatrix;
		CVec3 vDir( sCurrent.vDir );
		Normalize( &vDir );
		MakeMatrix( &sMatrix, CVec3( 0, 0, 0 ), vDir );
		CQuat sRot;
		sRot.FromEulerMatrix( sMatrix );

		(*pPose)[0].nParent = -1;
		(*pPose)[0].pos = sCurrent.vPosition + sCurrent.vDir * ( sFrameTime - sCurrent.sPassTime ) / 1000.0f;
		(*pPose)[0].rot = sRot * CQuat( ToRadian( 90.0f ), V3_AXIS_Z );

		return;
	}

	(*pPose)[0].nParent = -1;
	(*pPose)[0].pos = trailpointsSet.front().vPosition;
	(*pPose)[0].rot = QNULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TypePhysicsParameters( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	csSystem << "Friction = " << NAnimation::FRICTION << endl;
	csSystem << "Time step = " << NAnimation::DELTA_T << endl;
	csSystem << "Iterations = " << NAnimation::NUM_ITERATIONS << endl;
	csSystem << "Cycles = " << NAnimation::NUM_CYCLES << endl;
	csSystem << "Stiffness = " << NAnimation::STIFFNESS << endl;
	csSystem << " Physics additional restrictions are now ";
	if ( NAnimation::NUM_ANGLES == 18 )
		csSystem << "on";
	else
		csSystem << "off";
	csSystem << " Stopping rule for corpses: after " << NAnimation::N_INCREMENT_STEP << 
		" seconds increase stop level by " << NAnimation::F_STOP_DELTA_INCREMENT << " per second " << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void PhysicsVisualize( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	NAnimation::VIS_ON = !NAnimation::VIS_ON;
	csSystem << " Physics visualization is now ";
	if ( NAnimation::VIS_ON )
		csSystem << "on";
	else
		csSystem << "off";
	csSystem << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetTimestep( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
		csSystem << "Please give one number (time step) to this command" << endl;
	NAnimation::DELTA_T = atof( NStr::ToAscii( szParams[0] ).data() );
	NAnimation::DELTA_T_DWORD = (DWORD)(NAnimation::DELTA_T*1000);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetFriction( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
		csSystem << "Please give one number (friction) to this command" << endl;
	NAnimation::FRICTION = atof( NStr::ToAscii( szParams[0] ).data() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetStopRule( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() < 2 )
		csSystem << "Please give one integer and one float" << endl;
	NAnimation::N_INCREMENT_STEP = atoi( NStr::ToAscii( szParams[0] ).data() );
	NAnimation::F_STOP_DELTA_INCREMENT = atof( NStr::ToAscii( szParams[1] ).data() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetStiffness( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( szParams.size() == 0 )
		csSystem << "Please give one number (stiffness) to this command" << endl;
	NAnimation::STIFFNESS = atof( NStr::ToAscii( szParams[0] ).data() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void PhysicsAdditRestrictions( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	if ( NAnimation::NUM_ANGLES == 18 )
		NAnimation::NUM_ANGLES = 14;
	else
		NAnimation::NUM_ANGLES = 18;
	csSystem << " Physics additional restrictions are now ";
	if ( NAnimation::NUM_ANGLES == 18 )
		csSystem << "on";
	else
		csSystem << "off";
	csSystem << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void RestoreDefaults( const string &szID, const vector<wstring> &szParams, void *pContext )
{
	NAnimation::VIS_ON = false;
	NAnimation::NUM_ITERATIONS = 5;
	NAnimation::NUM_CYCLES = 5;
	NAnimation::FRICTION = 0.5f;
	NAnimation::DELTA_T = 0.05f;
	NAnimation::DELTA_T_DWORD = (DWORD)(NAnimation::DELTA_T*1000);
	NAnimation::STIFFNESS = 0.9f;
	NAnimation::NUM_ANGLES = 14;
	NAnimation::N_INCREMENT_STEP = 0;
	NAnimation::F_STOP_DELTA_INCREMENT = 0.15f;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
START_REGISTER(GAnimParticles)
	REGISTER_CMD( "phoutput", TypePhysicsParameters )
	REGISTER_CMD( "phvis", PhysicsVisualize )
	REGISTER_CMD( "phtimestep", SetTimestep )
	REGISTER_CMD( "phfriction", SetFriction )
	REGISTER_CMD( "phstiffness", SetStiffness )
	REGISTER_CMD( "phaddrestrict", PhysicsAdditRestrictions )
	REGISTER_CMD( "phstoprule", SetStopRule )
	REGISTER_CMD( "phdefault", RestoreDefaults )
FINISH_REGISTER
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NAnimation;
REGISTER_SAVELOAD_CLASS( 0x10441191, CParticleSkeleton );
REGISTER_SAVELOAD_CLASS( 0x11081181, CASphereSet );
REGISTER_SAVELOAD_CLASS( 0xB1081181, CATrailPath );
