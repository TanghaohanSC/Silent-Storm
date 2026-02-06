#ifndef __GVIEW_H_
#define __GVIEW_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "DG.h"
#include "GSkeleton.h"
#include "Time.h"
#include "GRenderModes.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRandomSeed;
class CTransformStack;
class CMemObject;
class CTerrainPart;
namespace NBuilding
{
	class CBuildingGrid;
	class CBuildingInfoHold;
}
namespace NDb
{
	class CTexture;
	class CModel;
	class CTemplVariant;
	class CEffect;
	class CAmbientLight;
	class CComplexHead;
	class CAIGeometry;
	class CSkeleton;
};
namespace NLSHead
{
	struct SHeadFrame;
};
struct STerrainInfo;
struct SMapBuilding;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
class CRenderNode;
class CSelectionNode;
class CBuilding;
class CObjectInfo;
class CPolyline;
class ILight;
class CParticles;
class CParticleEffect;
struct SRenderStats;
class CGrassTracker;
class CExplosionInfo;
class IGScene;
class CLightGroup;
class CGrassAnimator;
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRoomInfo
{
	ZDATA
	CPtr<CLightGroup> pGroup;
	short nFloor;
	bool bShadowCast, bTreeCrown;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pGroup); f.Add(3,&nFloor); f.Add(4,&bShadowCast); f.Add(5,&bTreeCrown); return 0; }
	//
	SRoomInfo(): nFloor(-100), bShadowCast(true), bTreeCrown(false) {}
	SRoomInfo( int _nFloor ): nFloor(_nFloor), bShadowCast(true), bTreeCrown(false) {}
	SRoomInfo( CLightGroup *_p, int _nFloor = -100 ): pGroup(_p), nFloor(_nFloor), bShadowCast(true), bTreeCrown(false) {}
	bool operator==( const SRoomInfo &a ) const { return a.pGroup == pGroup && a.nFloor == nFloor && a.bShadowCast == bShadowCast && a.bTreeCrown == bTreeCrown; }
};
//////////////////////////////////////////////////////////////////////////////////////
class IGameView: public CObjectBase
{
public:
	enum ELightMode
	{
		LT_ZONE,
		LT_INVENTORY
	};
	struct SDrawInfo
	{
		CTransformStack *pTS; // pTS на весь экран
		CVec2 vOrigin, vSize; // in [0,1] diapason
		CVec3 vClearColor;
		bool bOverlay, bUseDefaultClearColor;

		SDrawInfo() : pTS(0), vOrigin(0,0), vSize(1,1), vClearColor(0,0,0), bOverlay(false), bUseDefaultClearColor(true) {}
	};
	virtual IGScene* GetGScene() = 0;
	virtual CLightGroup* CreateLightGroup() = 0;
	virtual CRenderNode* CreateMesh( NDb::CModel *pModel, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateMesh( CMemObject *pModel, const CVec4 &color, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateMesh( NDb::CModel *pModel, const SFBTransform &placement, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &placement, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateSkin( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateOccluder( NDb::CAIGeometry *pGeom, const SFBTransform &placement ) = 0;
	virtual CRenderNode* CreateOccluder( NDb::CAIGeometry *pGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation ) = 0;
	virtual CRenderNode* CreateTerrainRegion( CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion, const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, NGScene::CGrassTracker *pGrass, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateTerrainWall( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture ) = 0;
	virtual CObjectBase* CreateGrassSector( CGrassAnimator *pEffect, NDb::CTexture *pTexture, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CSelectionNode* CreateSelection( const vector<CObjectBase*> &target, const CVec4 &vColor ) = 0;
	virtual CObjectBase* CreateParticles( NDb::CEffect *pEffect, STime stBeginTime, CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CBuilding* CreateBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI ) = 0;
	virtual CPolyline* CreatePolyline( const vector<CVec3> &points, const CVec3 &color ) = 0;
	virtual CObjectBase* CreateExplosion( CFuncBase<STime> *pTime, NDb::CEffect *pEffect, CFuncBase<CExplosionInfo> *pExplosion, const CVec3 &pos, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual CRenderNode* CreateLSHead( NDb::CComplexHead *pHead, CFuncBase<NLSHead::SHeadFrame> *pAnimation, CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, bool bInterface, bool bHasCap = false, const SRoomInfo &_g = SRoomInfo() ) = 0;
	virtual void Draw( const SDrawInfo &drawInfo ) = 0;
	virtual CVec2 GetScreenRect() = 0;
	virtual int  GetCutFloor() = 0;
	virtual void SetCutFloor( int nFloor ) = 0;
	virtual bool GetParticleShow() = 0;
	virtual void SetParticleShow( bool bNewState ) = 0;
	virtual void SetAmbient( const CVec3 &ambientColor ) = 0;
	virtual ILight* AddDirectionalLight( const CVec3 &ptColor, const CVec3 &ptLight, const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly = false ) = 0;
	virtual ILight* AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly = false ) = 0;
	virtual ILight* AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, NDb::CTexture *pMask, bool bLightmapOnly = false ) = 0;
	virtual void SetAmbient( NDb::CAmbientLight *pLight, ELightMode lm = LT_ZONE ) = 0;
	virtual ESceneRenderMode GetRenderMode() const = 0;
	virtual void SetRenderMode( ESceneRenderMode mode ) = 0;
	virtual EFogMode GetFogMode() const = 0;
	virtual void SetFogMode( EFogMode mode ) = 0;
	virtual void SetHSRMode( EHSRMode m ) = 0;
	virtual EHSRMode GetHSRMode() const = 0;
	virtual bool TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor ) = 0;
	void SetNextFogMode();
	void SetNextShadowsMode();
	void SetNextHSRMode();
	void SetFastMode();
};
IGameView* CreateNewView();
IGameView* CreateNewFastInterfaceView();
// wrappers to Gfx to exclude interface on Gfx dependency, realisation in GScene.cpp
bool Is3DActive();
void SetWireframe( bool bWire );
void MakeScreenShot();
void Flip();
void GetRenderStats( SRenderStats *pStats );
float GetFrameTime();
int CalcTouchedTextureSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
