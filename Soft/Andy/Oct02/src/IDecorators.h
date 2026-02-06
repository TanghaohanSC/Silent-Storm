#ifndef __A5_INTERFACEDECOR_H__
#define __A5_INTERFACEDECOR_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4250)
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CBaseDecorator
class CBaseDecorator: public IWindow
{
	OBJECT_NOCOPY_METHODS(CBaseDecorator)
private:
	ZDATA
	CObj<IWindow> pHolder; /// keep valid
	CMObj<IWindow> pThis;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&pHolder); f.Add(3,&pThis); return 0; }

public:
	CBaseDecorator() {}
	CBaseDecorator( IWindow *pWindow );
	virtual ~CBaseDecorator();

	bool IsSame( const IWindow *pWindow ) const;

	IInterface* GetInterface() const;
	const string& GetWindowID() const;

	IWindow* GetParent() const;
	void SetParent( IWindow *pParent );

	void AddChild( IWindow *pWindow );
	void RemoveChild( IWindow *pWindow );
	void DecorateChild( IWindow *pWindow, IWindow *pDecorator );
	IWindow* GetChildByID( const string &szID );
	void GetChildrenList( list<CPtr<IWindow> > *pList );

	bool GetStyle( int nStyle ) const;
	void SetStyle( int nStyle, bool bOn );

	const SPoint& GetSize() const;
	void SetSize( const SPoint &_sSize );

	const SPoint& GetPosition() const;
	void SetPosition( const SPoint &_sPosition );

	bool IsActive() const;
	void ShowWindow( int nCmdShow );

	bool HitTest( int nX, int nY ) const;
	bool ClientToScreen( SPoint *pPosition, SRect *pWindow, bool bSelf = true ) const;
	void ScreenToClient( const SPoint &sScreenPos, SPoint *pPosition ) const;
	void VirtualToScreen( const SScene &sScene, SPoint *pPosition, SRect *pRes ) const;

	IWindow* GetToolTip() const;
	void SetToolTip( IWindow *pWindow );

	bool SendMessage( IWindow *pTarget, const SEvent &sEvent );
	bool ProcessMessage( const SEvent &sEvent );

	void Update( const SScene &sScene );
	void Draw( const SScene &sScene );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CBaseDecorator
template<class Type>
class CDecorator: public CBaseDecorator
{
private:
	ZDATA_(CBaseDecorator)
	CPtr<Type> pWindow;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CBaseDecorator*)this); f.Add(2,&pWindow); return 0; }

protected:
	Type* GetThis() { ASSERT( pWindow ); return pWindow; }

public:
	typedef CDecorator<Type> TBaseClass;

public:
	CDecorator() {}
	CDecorator( Type *_pWindow ): CBaseDecorator( _pWindow ), pWindow( _pWindow ) {}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
