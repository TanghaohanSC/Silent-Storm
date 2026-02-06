#ifndef __IMAIN_H_
#define __IMAIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Time.h"
namespace NInput
{
	struct SEvent;
}
namespace NMainLoop
{
	int GetInterfaceStackDepth();
	bool StepApp( bool bActive, bool bSetGamma ); // return false on exit state
	void DoneInterface();
	void ShowLogo();
	//
	class IInterfaceObject: public CObjectBase
	{
	protected:
		const STime GetTime();

	public:
		virtual void Step() = 0;
		virtual bool ProcessEvent( const NInput::SEvent &eEvent ) = 0;
	};
	//
	class IInterfaceBase: public IInterfaceObject
	{
	protected:
		bool CanRender();

	public:
		virtual void OnGetFocus() = 0;
		friend class CInterfaceCommand;
	};
	//
	class CInterfaceCommand: public CObjectBase
	{
		int operator&( CStructureSaver &f ) { ASSERT(0); return 0; }
		protected:
			void ResetStack();
			void SetInterface( IInterfaceBase *pNewInterface );
			void PushInterface( IInterfaceBase *pNewInterface );
			void PopInterface();
      IInterfaceBase* GetInterface() const;
		public:
			virtual void Exec() = 0;
	};
	//
	void Command( CInterfaceCommand *pCmd );
	//
	class CICLoad: public CInterfaceCommand
	{
		OBJECT_BASIC_METHODS(CICLoad);
			string szFileName;
		public:
			CICLoad() {}
			CICLoad( const char *pszFileName ): szFileName(pszFileName) {}
			virtual void Exec();
	};
	//
	class CICSave: public CInterfaceCommand
	{
		OBJECT_BASIC_METHODS(CICSave);
			string szFileName;
		public:
			CICSave() {}
			CICSave( const char *pszFileName ): szFileName(pszFileName) {}
			virtual void Exec();
	};
	//
	class CICExitModal: public CInterfaceCommand
	{
		OBJECT_BASIC_METHODS(CICExitModal);
	public:
		virtual void Exec();
	};
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif