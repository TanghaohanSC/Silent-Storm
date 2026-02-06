#ifndef __GLightmap_H_
#define __GLightmap_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GCombiner.h"
#include "Cache.h"
#include "DG.h"

namespace NGfx
{
	class CTexture;
}
namespace NGScene
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLightmapTextureCache;
class CLMRegion : 
	public NCache::CGatherElementBase<NCache::CShortPtrAllocator,NCache::CQuadTreeElement, CLMRegion>
{
	typedef NCache::CGatherElementBase<NCache::CShortPtrAllocator,NCache::CQuadTreeElement, CLMRegion> CParent;
	OBJECT_NOCOPY_METHODS(CLMRegion);
	ZDATA
	ZPARENT(CParent)
public:
	CTRect<int> lmRegion;
	int nLOD; // higher is better
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CParent*)this); f.Add(3,&lmRegion); return 0; }
	int nLastFrame;
	CLMRegion() : nLastFrame(0), nLOD(0) {}
	CLightmapTextureCache* GetCache();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class ILightmappedElement : public CObjectBase
{
public:
	virtual void AddForLightmapRender( CSceneFragments *pRes ) = 0;
	virtual CVersioningBase* GetOnChangeNode() const = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLightmapTextureCache :
	public NCache::CGatheringCache<NCache::CShortPtrAllocator, NCache::CQuadTreeElement, CLMRegion>
{
	OBJECT_NOCOPY_METHODS( CLightmapTextureCache );
	typedef NCache::CGatheringCache<NCache::CShortPtrAllocator, NCache::CQuadTreeElement, CLMRegion> CParent;
	CObj<NGfx::CTexture> pTexture;

	void InitCache();
public:
	CLightmapTextureCache() {}
	CLightmapTextureCache( int ) { InitCache(); RefreshTexture(); }
	void RefreshTexture();
	NGfx::CTexture* GetTexture() const { return pTexture; }
	int operator&( CStructureSaver &f );
	friend class CLMRegion;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif