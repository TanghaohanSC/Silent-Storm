#ifndef __IGLOBAL_COMMONUI_H_
#define __IGLOBAL_COMMONUI_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\Misc\2DArray.h"
namespace NDb
{
	class CSequence;
	enum ECameraType;
}
namespace NGScene
{
	class IGameView;
	class CCFBTransform;
}
namespace NRender
{
	class IRenderGame;
	class IShowUnit;
	class IShowUnitHead;
}
namespace NGame
{
	class IMission;
}
namespace NWorld
{
	class CUnit;
	class CPlayerItem;
}
namespace NRPG
{
	class CUnit;
	class IInventory;
	class IInventoryItem;
}
#include "iActionDecorator.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CLineBar
////////////////////////////////////////////////////////////////////////////////////////////////////
class CLineBar: public CImage
{
	OBJECT_BASIC_METHODS(CLineBar)
private:
	ZDATA_(CImage)
	int nBarWidth;
	CPtr<CImage> pBar;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CImage*)this); f.Add(2,&nBarWidth); f.Add(3,&pBar); return 0; }

public:
	CLineBar() {}
	CLineBar( const SWindowInfo &sInfo );

	void Set( float fBar );

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CImageNumber
////////////////////////////////////////////////////////////////////////////////////////////////////
class CImageNumber: public CWindow
{
	OBJECT_BASIC_METHODS(CImageNumber)
public:
	enum EType
	{
		TYPE_UNITINFOPANEL
	};

private:
	ZDATA_(CWindow)
	int nValue;
	SPoint sRealSize;
	vector<int> textureIDs;
	NGfx::SPixel8888 sColor;
	list<CObj<CImage> > imagesList;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&nValue); f.Add(3,&sRealSize); f.Add(4,&textureIDs); f.Add(5,&sColor); f.Add(6,&imagesList); return 0; }

public:
	CImageNumber() {}
	CImageNumber( const SWindowInfo &sInfo, EType eType );

	void Set( int nValue );
	void SetColor( const NGfx::SPixel8888 &_sColor );

	const SPoint& GetRealSize() const;

	bool ProcessMessage( const SEvent &sEvent );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CHoverButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CHoverButton: public CButton
{
	OBJECT_BASIC_METHODS(CHoverButton)
public:
	enum
	{
		STATE_NORMAL,
		STATE_HOVER,
		STATE_DISABLED
	};

private:
	ZDATA_(CButton)
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); return 0; }

public:
	CHoverButton() {}
	CHoverButton( const SWindowInfo &sInfo );

	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CFlashButton
////////////////////////////////////////////////////////////////////////////////////////////////////
class CFlashButton: public CButton
{
	OBJECT_BASIC_METHODS(CFlashButton)
public:
	enum
	{
		STATE_NORMAL,
		STATE_HOVER,
		STATE_DISABLED
	};

private:
	ZDATA_(CButton)
	STime sFlashTime;
	CPtr<CImage> pLight;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CButton*)this); f.Add(2,&sFlashTime); f.Add(3,&pLight); return 0; }

public:
	CFlashButton() {}
	CFlashButton( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
	void Update( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitHead
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitHead: public CWindow
{
	OBJECT_BASIC_METHODS(CUnitHead);
protected:
	ZDATA_(CWindow)
	float fScale;
	CObj<NGScene::IGameView> p3DView;
	CPtr<NRender::IRenderGame> pRenderGame;
	CObj<NGScene::CCFBTransform> pTransform;
	CObj<NRender::IShowUnitHead> pHead;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&fScale); f.Add(3,&p3DView); f.Add(4,&pRenderGame); f.Add(5,&pTransform); f.Add(6,&pHead); return 0; }

public:
	CUnitHead() {}
	CUnitHead( const SWindowInfo &sInfo, NRender::IRenderGame *pRender, float fScale = 1.0f );

	void SetUnit( NWorld::CUnit *pUnit );
	void SetSequence( NDb::CSequence *pSequence );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CUnitView
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUnitView: public CWindow
{
	OBJECT_BASIC_METHODS(CUnitView);
protected:
	ZDATA_(CWindow)
	float fYaw;
	float fPitch;
	float fDistance;
	CVec3 vAnchor;
	CTimeCounter sTimer;
	CObj<NGScene::IGameView> p3DView;
	CPtr<NRender::IShowUnit> pInventoryUnit;
	CPtr<NRender::IRenderGame> pRenderGame;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&fYaw); f.Add(3,&fPitch); f.Add(4,&fDistance); f.Add(5,&vAnchor); f.Add(6,&sTimer); f.Add(7,&p3DView); f.Add(8,&pInventoryUnit); f.Add(9,&pRenderGame); return 0; }

public:
	CUnitView() {}
	CUnitView( const SWindowInfo &sInfo, NRender::IRenderGame *pRender = 0 );

	void SetUnit( NRPG::CUnit *pUnit );
	void SetUnit( NWorld::CUnit *pUnit );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CItemModel
////////////////////////////////////////////////////////////////////////////////////////////////////
class CItemModel: public CActionDecorator<CModel>
{
	OBJECT_BASIC_METHODS(CItemModel)
private:
	ZDATA_(TBaseClass)
	CPtr<NRPG::IInventoryItem> pItem;
	////
	CObj<CToolTip> pToolTip;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pItem); f.Add(3,&pToolTip); return 0; }

public:
	CItemModel() {}
	CItemModel( const SWindowInfo &sInfo, NGame::IMission *pMission );

	bool CanHandleState( NGame::IState *pState ) const;
	CObjectBase* GetTarget() const;

	void Set( NRPG::IInventoryItem* pItem, NDb::ECameraType eCameraType );

	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CSlot
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSlot: public CWindow
{
protected:
	struct SItem
	{
		ZDATA
		CTPoint<int> sPos;
		CObj<CWindow> pModel;
		CPtr<NRPG::IInventoryItem> pItem;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&sPos); f.Add(3,&pModel); f.Add(4,&pItem); return 0; }

		SItem(): sPos( 0, 0 ) {}
	};
	struct SHilight
	{
		ZDATA
		int nID;
		CObj<CImage> pImage;
		ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nID); f.Add(3,&pImage); return 0; }
	};

private:
	ZDATA_(CWindow)
	CPtr<NGame::IMission> pMission;
	////
	int nWidth;
	int nHeight;
	bool bTrackMouse;
	bool bAlwaysHilight;
	SPoint sMousePoint;
	CPtr<CWindow> pSlotView;
	CPtr<CWindow> pHilight;
	vector<SItem> itemsSet;
	NDb::ECameraType eCameraType;
	CArray2D<SHilight> hilights;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&pMission); f.Add(3,&nWidth); f.Add(4,&nHeight); f.Add(5,&bTrackMouse); f.Add(6,&bAlwaysHilight); f.Add(7,&sMousePoint); f.Add(8,&pSlotView); f.Add(9,&pHilight); f.Add(10,&itemsSet); f.Add(11,&eCameraType); f.Add(12,&hilights); return 0; }

protected:
	void GetInSlotPos( int nX, int nY, SPoint *pCoords );
	void GetItemInSlotPos( int nX, int nY, const SPoint &sItemSize, SPoint *pCoords );
	NGame::IMission* GetGame();
	bool GetDragItem( NWorld::IPlayer::SItemInfo *pInfo );

	virtual void GetItemsList( vector<SItem> *pItemsSet ) = 0;
	virtual bool CanTransferFrom( NWorld::CUnit *pUnit ) = 0;

public:
	CSlot() {}
	CSlot( const SWindowInfo &sInfo, NGame::IMission *pMission, int nWidth, int nHeight, NDb::ECameraType eCameraType, bool bAlwaysHilight );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
