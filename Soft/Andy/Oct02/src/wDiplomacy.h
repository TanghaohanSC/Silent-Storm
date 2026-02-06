#ifndef __WDIPLOMACY_H_
#define __WDIPLOMACY_H_
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
	typedef unsigned int uint;
	OBJECT_BASIC_METHODS( CDiplomacy );
	ZDATA
	uint nDiplomacy;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&nDiplomacy); return 0; }
	//
	CDiplomacy( unsigned int _nDiplomacy = 0 );
	//
	virtual NDb::EDiplomacyState GetDiplomacyState( int nPlayer );
	virtual void SetDiplomacyState( int nPlayer, NDb::EDiplomacyState state );
	virtual unsigned int GetDiplomacy() { return nDiplomacy; }
	virtual void SetDiplomacy( unsigned int _nDiplomacy ) { nDiplomacy = _nDiplomacy; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalDiplomacy 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalDiplomacy: public CObjectBase
{
	OBJECT_BASIC_METHODS( CGlobalDiplomacy );
	ZDATA
	vector< CPtr<CDiplomacy> > diplomacy;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&diplomacy); return 0; }
	//
	void LoadDiplomacy( int nZoneID );
public:
	//
	CGlobalDiplomacy( int nZoneID = 0 );
	//
	virtual NDb::EDiplomacyState GetDiplomacyState( int nPlayer1, int nPlayer2 );
	virtual void SetDiplomacyState( int nPlayer1, int nPlayer2, NDb::EDiplomacyState state );
	virtual unsigned int GetPlayerDiplomacy( int nPlayer );
	virtual void SetPlayerDiplomacy( int nPlayer, unsigned int nDiplomacy );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __WDIPLOMACY_H_