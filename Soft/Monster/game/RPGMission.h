#ifndef __RPGMISSION_H_
#define __RPGMISSION_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
/////////////////////////////////////////////////////////////////////////////////////
// mission time RPG information handler
class CMerc;
struct SUnitInfo; // data about any unit that can be shown in interface
class CUnitMissionBase: public CObjectBase
{
public:
	virtual void TakeDamage( int nHP ) = 0;
	virtual const SUnitInfo& GetInfo() const = 0;
	virtual void UpdateMerc( CMerc *pDst ) = 0;   // update merc info on mission finish
};
/////////////////////////////////////////////////////////////////////////////////////
// create mission time RPG info from Merc info or dbms record
CUnitMissionBase* CreateUnit( CMerc *pSrc );
//CRPGUnitGlobal* CreateUnit( CRPGUnitInfo *pInfo ); // db info->rpgunit
/////////////////////////////////////////////////////////////////////////////////////
} // namespace
/////////////////////////////////////////////////////////////////////////////////////
#endif