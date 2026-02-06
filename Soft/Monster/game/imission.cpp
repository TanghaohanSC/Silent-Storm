#include "StdAfx.h"
#include "iInput.h"
#include "iMain.h"
#include "wInterface.h"
#include "GScene.h"
#include "Transform.h"
#include "Camera.h"
#include "iMission.h"
#include "RPGGlobal.h"
#include "RPGMission.h"
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class CMoveControl
{
	float fpAngle, fpDistance;
	NInput::CSlider fwd, back, left, right;
public:
	CMoveControl( float _fpAngle );
	void Update();
	float GetAngle() const { return fpAngle; }
	CVec3 GetPos() const;
	friend void Serialize( CStructureSaver *pFile, CMoveControl *pObj );
};
/////////////////////////////////////////////////////////////////////////////////////
inline void Serialize( CStructureSaver *pFile, CMoveControl *pObj ) 
{
	pFile->AddData( 1, &pObj->fpAngle ); 
	pFile->AddData( 2, &pObj->fpDistance );
}
/////////////////////////////////////////////////////////////////////////////////////
CMoveControl::CMoveControl( float _fpAngle ): fpAngle(_fpAngle), fpDistance(0),
	fwd("moveforward"), back("movebackward"), left("moveleft"), right("moveright")
{
}
/////////////////////////////////////////////////////////////////////////////////////
void CMoveControl::Update()
{
	fpAngle += left.GetDelta() - right.GetDelta();
	fpDistance += fwd.GetDelta() - back.GetDelta();
}
/////////////////////////////////////////////////////////////////////////////////////
CVec3 CMoveControl::GetPos() const
{
	return CVec3( cos( fpAngle ) * fpDistance, sin( fpAngle ) * fpDistance, 0 );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
struct SViewObject
{
	CPtr<NWorld::CObject> pSrc;
	CObj<NGScene::CRenderNode> pRender;
	//
	friend void Serialize( CStructureSaver *pFile, SViewObject *pObj );
};
/////////////////////////////////////////////////////////////////////////////////////
inline void Serialize( CStructureSaver *pFile, SViewObject *pObj ) 
{
	pFile->AddObject( 1, &pObj->pSrc );
	pFile->AddObject( 2, &pObj->pRender );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
struct SViewUnit
{
	CPtr<NWorld::CUnit> pSrc;
	CObj<NGScene::CRenderNode> pRender;
	CObj<NDG::CBaseNode> pSelected;
	//
	friend void Serialize( CStructureSaver *pFile, SViewUnit *pObj );
};
/////////////////////////////////////////////////////////////////////////////////////
inline void Serialize( CStructureSaver *pFile, SViewUnit *pObj ) 
{
	pFile->AddObject( 1, &pObj->pSrc );
	pFile->AddObject( 2, &pObj->pRender );
	pFile->AddObject( 3, &pObj->pSelected );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class CInterfaceMission;
class CMoveState: public CFundament
{
	FUNDAMENT_BASIC_METHODS(CMoveState);
	//
	CMoveControl move;
	CPtr<NWorld::CUnit> pSrc;
	CVec3 ptStartPos;
	CObj<NGScene::CRenderNode> pRender;
	CObj<NGScene::CCMSR> pMoveDest;
	CObj<NDG::CBaseNode> pSelected;
	//
public:
	CMoveState(): move(0) {} // for loading
	CMoveState( CInterfaceMission *m, SViewUnit *pUnit );
	CVec3 GetDst() const { return ptStartPos + move.GetPos(); }
	void Update();
	NWorld::CUnit* GetSrc() const { return pSrc; }
	virtual void Serialize( CStructureSaver *pFile );
};
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class CInterfaceMission: public NMainLoop::CInterfaceBase
{
	FUNDAMENT_BASIC_METHODS(CInterfaceMission);
	//
	enum ECmd
	{
		CMD_EXIT = 1,
		CMD_NEXT = 2,
		CMD_MOVE = 3,
		CMD_EXEC = 4,
		CMD_PAUSE = 5,
	};
	enum EState
	{
		ST_NORMAL,
		ST_CMDMOVE,
		ST_ACTION
	};
	//
	struct SObjTrack
	{
		CInterfaceMission &m;
		SObjTrack( CInterfaceMission &_m ): m(_m) {}
		void Add( SViewObject &dst, NWorld::CObject *pSrc ) const { m.AddNewObject( dst, pSrc ); }
		void Delete( SViewObject &dst ) const {}
	};
	struct SUnitTrack
	{
		CInterfaceMission &m;
		SUnitTrack( CInterfaceMission &_m ): m(_m) {}
		void Add( SViewUnit &dst, NWorld::CUnit *pSrc ) const { m.AddNewUnit( dst, pSrc ); }
		void Delete( SViewUnit &dst ) const {}
	};
	//
	list< SViewObject > objects;
	list< SViewUnit > units;
	CPtr<NWorld::CWorldBase> pWorld;
	CPtr<NGScene::CGSceneBase> pScene;
	CPtr<NRPG::CGlobalGame> pGame;
	CTransformStack ts;
	CCamera camera;
	CPtr<CMoveState> pMove;
	NInput::STime prevTime;
	//
	EState state;
	bool bPause;
	//
	void AddNewObject( SViewObject &o, NWorld::CObject *pSrc );
	void AddNewUnit( SViewUnit &o, NWorld::CUnit *pSrc );
	void UpdateViewWorld();
	void RenderFrame();
	//
	SViewUnit* GetSelected();
	void SelectNext();
public:
	CInterfaceMission();
	void Init( NRPG::CGlobalGame *_pGame );
	virtual void Step();
	virtual void Serialize( CStructureSaver *pFile );
	//
	friend struct CInterfaceMission::SObjTrack;
	friend struct CInterfaceMission::SUnitTrack;
	friend class CMoveState;
	friend void RegisterMissionInterfaceClasses( int nBase ); // CRAP due to event binds
};
/////////////////////////////////////////////////////////////////////////////////////
// CMoveState
/////////////////////////////////////////////////////////////////////////////////////
CMoveState::CMoveState( CInterfaceMission *m, SViewUnit *pUnit ): move( 0 )
{
	pSrc = pUnit->pSrc;
	CDGPtr<NDG::CFuncBase<SHMatrix> > pPos(pSrc->pPosition);
	pPos.Refresh();  // CRAP, CUnit should have info on current unit position
	ptStartPos = pPos->GetValue().GetTranslation();
	pMoveDest = new NGScene::CCMSR;
	Update();
	pRender = m->pScene->CreateModel( pSrc->nModelID, pMoveDest );
	pSelected = m->pScene->AddHilight( pRender, CVec3(0.7f,0.7f,0.7f) );
}
/////////////////////////////////////////////////////////////////////////////////////
void CMoveState::Update()
{
	move.Update();
	SHMatrix s;
	MakeMatrix( &s, 0, move.GetAngle(), GetDst() );
	pMoveDest->Set( s );
}
/////////////////////////////////////////////////////////////////////////////////////
void CMoveState::Serialize( CStructureSaver *pFile )
{
	pFile->AddObject( 1, &move );
	pFile->AddObject( 2, &pSrc );
	pFile->AddData( 3, &ptStartPos );
	pFile->AddObject( 4, &pRender );
	pFile->AddObject( 5, &pMoveDest );
	pFile->AddObject( 6, &pSelected );
}
/////////////////////////////////////////////////////////////////////////////////////
// CInterfaceMission
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::AddNewObject( SViewObject &o, NWorld::CObject *pSrc )
{
	NGScene::CRenderNode *pR = pScene->CreateModel( pSrc->nModelID, pSrc->pPosition );
	o.pSrc = pSrc;
	o.pRender = pR;
}
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::AddNewUnit( SViewUnit &o, NWorld::CUnit *pSrc )
{
	NGScene::CRenderNode *pR = pScene->CreateModel( pSrc->nModelID, pSrc->pPosition );
	o.pSrc = pSrc;
	o.pRender = pR;
	if ( units.size() == 1 )
	{
		o.pSelected = pScene->AddHilight( pR, CVec3(1,1,1) );
	}
}
/////////////////////////////////////////////////////////////////////////////////////
template< class TDst, class TSrc, class TFunc >
void SyncSets( const TSrc &src, TDst *pDst, const TFunc &f )
{
	typedef TSrc::value_type::CDestType CElement;
	TDst::iterator k = pDst->begin();
	TSrc::const_iterator i = src.begin();
	//
	for(;;)
	{
		if ( i == src.end() )
		{
			while ( k != pDst->end() )
			{
				TDst::iterator kn( k );
				f.Delete( *kn );
				++k;
				pDst->erase( kn );
			}
			break;
		}
		if ( k == pDst->end() )
		{
			for ( ; i != src.end(); ++i )
			{
				pDst->push_back();
				f.Add( pDst->back(), *i );
			}
			break;
		}
		CElement *pSrcElem = *i, *pDstElem = k->pSrc;
		if ( pSrcElem == pDstElem )
		{
			++i;
			++k;
			continue;
		}
		if ( pSrcElem < pDstElem )
		{
			f.Add( *pDst->insert( k ), *i );
			continue;
		}
		if ( 1 )
		{
			TDst::iterator kn( k );
			f.Delete( *kn );
			++k;
			pDst->erase( kn );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::UpdateViewWorld()
{
	SyncSets( pWorld->GetActiveObjects(), &objects, SObjTrack( *this ) );
	SyncSets( pWorld->GetActiveUnits(), &units, SUnitTrack( *this ) );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::SelectNext()
{
	for ( list< SViewUnit >::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( i->pSelected->IsValid() )
		{
			i->pSelected = 0;
			++i;
			if ( i == units.end() )
				i = units.begin();
			i->pSelected = pScene->AddHilight( i->pRender, CVec3(1,1,1) );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
SViewUnit* CInterfaceMission::GetSelected()
{
	for ( list< SViewUnit >::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( i->pSelected->IsValid() )
			return &(*i);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::RenderFrame()
{
	ts.SetCamera( camera.GetPos() );
	pScene->Draw( &ts );
}
/////////////////////////////////////////////////////////////////////////////////////
CInterfaceMission::CInterfaceMission()
{
	ts.MakeProjective();
	state = ST_NORMAL;
	prevTime = 0;
	bPause = false;
}
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::Init( NRPG::CGlobalGame *_pGame )
{
	pGame = _pGame;
	//
	pScene = NGScene::CreateNewScene();
	pScene->AddLight( CVec3(5,5,0), CVec3( 0,0,-1), 10, 20 );
	//
	pWorld = NWorld::CreateWorld( pScene->GetTime() );
	vector<CPtr<NRPG::CUnitMissionBase> > mercs;
	for ( int i = 0; i < pGame->mercs.size(); ++i )
		mercs.push_back( NRPG::CreateUnit( pGame->mercs[i] ) );
	pWorld->CreateRandom( mercs );
}
/////////////////////////////////////////////////////////////////////////////////////
void CInterfaceMission::Step()
{
	NInput::SMessage msg;
	while ( NInput::GetMessage( &msg ) )
	{
		NMainLoop::ProcessStandardMsgs( msg );
		switch ( msg.nEventID )
		{
			case CMD_EXIT: 
				NMainLoop::Command( 0 ); 
				break;
			case CMD_PAUSE:
				bPause = !bPause;
				break;
		}
		switch ( state )
		{
			case ST_NORMAL:
				{
					switch ( msg.nEventID )
					{
						case CMD_NEXT:
							SelectNext();
							break;
						case CMD_MOVE:
							{
								SViewUnit *pUnit = GetSelected();
								if ( pUnit )
								{
									state = ST_CMDMOVE;
									pMove = new CMoveState( this, pUnit );
								}
							}
							break;
					}
				}
				break;
			case ST_CMDMOVE:
				{
					switch ( msg.nEventID )
					{
						case CMD_EXEC:
							pWorld->ExecMove( pMove->GetSrc(), pMove->GetDst() );
							pMove = 0;
							state = ST_ACTION;
							break;
					}
				}
				break;
		}
		//if ( msg.msg == SWindowsMsg::KEY_DOWN && ( msg.nKey == 'w' || msg.nKey == 'W' ) )
			//NGfx::SetWireframe( bWire = !bWire );
		//if ( msg.msg == SWindowsMsg::KEY_DOWN && ( msg.nKey == ' ' ) )
			//bPause = !bPause;
	}
	// update Time, here msg.time - current time
	bool bCanRender = CanRender();
	if ( !bPause && bCanRender )
	{
		if ( prevTime == 0 )
			prevTime = msg.time;
		NInput::STime deltaT = msg.time - prevTime;
		pScene->GetTime()->Set( pScene->GetTime()->GetValue() + deltaT );
		pScene->UseUpdated();
	}
	prevTime = msg.time;
	// 
	switch ( state )
	{
		case ST_CMDMOVE:
			ASSERT( pMove->IsValid() );
			if ( pMove->IsValid() )
			{
				pMove->Update();
			}
			break;
		case ST_ACTION:
			if ( !pWorld->IsExecuting() )
			{
				state = ST_NORMAL;
			}
			break;
	}
	if ( bCanRender )
	{
		camera.Update();
		pWorld->StepWorld();
		UpdateViewWorld();
		RenderFrame();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
struct SSrcGreater
{
	template <class T>
	bool operator()( const T &a, const T &b ) const{ return a.pSrc < b.pSrc; }
};
void CInterfaceMission::Serialize( CStructureSaver *pFile )
{
	pFile->AddContainer( 1, &objects );
	pFile->AddContainer( 2, &units );
	pFile->AddObject( 3, &pWorld );
	pFile->AddObject( 4, &pScene );
	pFile->AddData( 5, &ts );
	pFile->AddObject( 6, &camera );
	pFile->AddObject( 7, &pMove );
	pFile->AddData( 9, &state );
	pFile->AddData( 10, &bPause );
	pFile->AddObject( 11, &pGame );
	//
	objects.sort( SSrcGreater() );
	units.sort( SSrcGreater() );
}
/////////////////////////////////////////////////////////////////////////////////////
// CICMission
/////////////////////////////////////////////////////////////////////////////////////
CICMission::CICMission( NRPG::CGlobalGame *_pGame ): pGame(_pGame) 
{
}
/////////////////////////////////////////////////////////////////////////////////////
void CICMission::Exec()
{
	CInterfaceMission *pRes = new CInterfaceMission();
	pRes->Init( pGame );
	SetInterface( pRes );
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void RegisterMissionInterfaceClasses( int nBase )
{
	REGISTER_SAVELOAD_CLASS( nBase + 0, CInterfaceMission );
	REGISTER_SAVELOAD_CLASS( nBase + 1, CMoveState );
	// CRAP{
	NInput::RegisterEvent( CInterfaceMission::CMD_EXIT, "exit" );
	NInput::RegisterEvent( CInterfaceMission::CMD_NEXT, "next" );
	NInput::RegisterEvent( CInterfaceMission::CMD_MOVE, "move" );
	NInput::RegisterEvent( CInterfaceMission::CMD_EXEC, "exec" );
	NInput::RegisterEvent( CInterfaceMission::CMD_PAUSE, "pause" );
	NInput::BindSlider( "forward", 'A' );
	NInput::BindSlider( "backward", 'Z' );
	NInput::BindSlider( "left", VK_LEFT );
	NInput::BindSlider( "right", VK_RIGHT );
	NInput::BindSlider( "up", VK_UP );
	NInput::BindSlider( "down", VK_DOWN );
	NInput::BindSlider( "moveleft", 'S' );
	NInput::BindSlider( "moveright", 'F' );
	NInput::BindSlider( "moveforward", 'E' );
	NInput::BindSlider( "movebackward", 'D' );
	NInput::Bind( "exit", VK_ESCAPE );
	NInput::Bind( "next", VK_TAB );
	NInput::Bind( "move", 'M' );
	NInput::Bind( "exec", VK_RETURN );
	NInput::Bind( "pause", ' ' );
	// CRAP}
}
/////////////////////////////////////////////////////////////////////////////////////
