#ifndef __RPGDIPLOMACY_H_
#define __RPGDIPLOMACY_H_
//
namespace NWorld
{
	class IWorld;
	class CWorld;
}
//
namespace NDb
{
	enum EDiplomacyState;
}
//
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDiplomacy
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDiplomacy: public CObjectBase
{
	OBJECT_BASIC_METHODS( CDiplomacy );
	ZDATA
	DWORD nDiplomacy;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nDiplomacy); return 0; }
	//
	CDiplomacy( DWORD _nDiplomacy = 0 );
	//
	virtual NDb::EDiplomacyState GetDiplomacyState( int nPlayer );
	virtual void SetDiplomacyState( int nPlayer, NDb::EDiplomacyState state );
	virtual DWORD GetDiplomacy() { return nDiplomacy; }
	virtual void SetDiplomacy( DWORD _nDiplomacy ) { nDiplomacy = _nDiplomacy; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalDiplomacy 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalDiplomacy: public CObjectBase
{
	OBJECT_BASIC_METHODS( CGlobalDiplomacy );
	ZDATA
	vector< CPtr<CDiplomacy> > diplomacy;
	CPtr<NWorld::CWorld> pWorld;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&diplomacy); f.Add(3,&pWorld); return 0; }
public:
	//
	CGlobalDiplomacy();
	//
	virtual NDb::EDiplomacyState GetDiplomacyState( int nPlayer1, int nPlayer2 );
	virtual void SetDiplomacyState( int nPlayer1, int nPlayer2, NDb::EDiplomacyState state );
	virtual DWORD GetPlayerDiplomacy( int nPlayer );
	virtual void SetPlayerDiplomacy( int nPlayer, DWORD nDiplomacy );
	//
	void LoadDiplomacy( int nZoneID );
	void SetWorld( NWorld::IWorld *_pWorld );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __RPGDIPLOMACY_H_