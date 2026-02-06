#ifndef __IMAIN_H_
#define __IMAIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////////////
namespace NInput
{
	struct SMessage;
}
namespace NMainLoop
{
	bool StepApp(); // return false on exit state
	//
	class CInterfaceBase: public CFundament
	{
		protected:
			bool CanRender();
		public:
			virtual void Step() = 0;
	};
	//
	class CInterfaceCommand: public CFundament
	{
		protected:
			void SetInterface( CInterfaceBase *pNewInterface );
		public:
			virtual void Exec() = 0;
	};
	//
	void Command( CInterfaceCommand *pCmd );
	void ProcessStandardMsgs( NInput::SMessage &msg );
	void BindStandardMsgs(); // CRAP, should be realized via classes
	//
	class CICLoad: public CInterfaceCommand
	{
			string szFileName;
		public:
			CICLoad( const char *pszFileName ): szFileName(pszFileName) {}
			virtual void Exec();
	};
	//
	class CICSave: public CInterfaceCommand
	{
			string szFileName;
		public:
			CICSave( const char *pszFileName ): szFileName(pszFileName) {}
			virtual void Exec();
	};
};
/////////////////////////////////////////////////////////////////////////////////////
void RegisterInterfaceClasses( int nBase );
/////////////////////////////////////////////////////////////////////////////////////
#endif