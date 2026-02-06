#ifndef __DATADIFFICULTY_H_
#define __DATADIFFICULTY_H_
//
#include "..\ADOImport\BasicDB.h"
//
namespace NDb
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum ECanSave
{
	E_CS_CHAPTER = 0,
	E_CS_REALTIME,
	E_CS_ALWAYS,
	N_CAN_SAVE
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDBDifficulty
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBDifficulty: public CDBRecord
{
	OBJECT_BASIC_METHODS( CDBDifficulty );
public:
	ZDATA
	ZPARENT( CDBRecord );
	float fAPCoeff;
	float fVPCoeff;
	int nAIUnitsLevel;
	float fDeathCoeff;
	bool bNeedCarryOutUnconscious;
	ECanSave canSave;
	string szUserName;
	bool bHealOnLeaveZone;
	float fHealOnLeaveZoneCoeff;
	bool bBandageOnLeaveZone;
	float fBandageOnLeaveZoneCoeff;
	float fHealOnRestCoeff;
	int nREDifficulty;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,(CDBRecord *)this); f.Add(3,&fAPCoeff); f.Add(4,&fVPCoeff); f.Add(5,&nAIUnitsLevel); f.Add(6,&fDeathCoeff); f.Add(7,&bNeedCarryOutUnconscious); f.Add(8,&canSave); f.Add(9,&szUserName); f.Add(10,&bHealOnLeaveZone); f.Add(11,&fHealOnLeaveZoneCoeff); f.Add(12,&bBandageOnLeaveZone); f.Add(13,&fBandageOnLeaveZoneCoeff); f.Add(14,&fHealOnRestCoeff); f.Add(15,&nREDifficulty); return 0; }
	//
	virtual void Import();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
CDBDifficulty *GetDBDifficulty( int nID );
////////////////////////////////////////////////////////////////////////////////////////////////////
}
//
#endif __DATADIFFICULTY_H_