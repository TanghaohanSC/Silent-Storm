#ifndef __A5_INTERFACEDRAGANDDROP_H__
#define __A5_INTERFACEDRAGANDDROP_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4250)
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDragItemBase
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDragItemBase: public CDecorator<IWindow>
{
private:
	ZDATA_(TBaseClass)
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); return 0; }

public:
	CDragItemBase() {}
	CDragItemBase( IWindow *pWindow ): TBaseClass( pWindow ) {}

	bool ProcessMessage( const SEvent &sEvent )
	{
		switch( sEvent.nEvent )
		{
			case EVENT_MOUSEMOVE:
			{
				const SPoint &sSize = GetSize();
//				SetPosition( SPoint( sEvent.nX - sSize.x / 2, sEvent.nY - sSize.y / 2 ) );
				break;
			}
			case EVENT_MOUSEUPDATECURSOR:
			{
				GetInterface()->SetCursor( SCursorInfo() );
				return true;
			}
		}

		return TBaseClass::ProcessMessage( sEvent );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDragItem
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TUser>
class CDragItem: public CDragItemBase
{
	OBJECT_NOCOPY_METHODS(CDragItem)
private:
	ZDATA_(CDragItemBase)
	CPtr<TUser> pObject;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CDragItemBase*)this); f.Add(2,&pObject); return 0; }

public:
	CDragItem() {}
	CDragItem( IWindow *pWindow, TUser *_pObject ): CDragItemBase( pWindow ), pObject( _pObject ) {}

	TUser* GetObject() { return pObject; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// CDropPad
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class TUser, class TWindow = IWindow>
class CDropPad: public CDecorator<TWindow>
{
private:
	ZDATA_(TBaseClass)
	CPtr<CDragItem<TUser> > pObject;
public:
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(TBaseClass*)this); f.Add(2,&pObject); return 0; }

public:
	CDropPad() {}
	CDropPad( IWindow *pWindow ): TBaseClass( pWindow ) {}

	TUser* GetDragItem() const
	{
		if ( IsValid( pObject ) && IsValid( pObject->GetObject() ) )
			return pObject->GetObject();

		return 0;
	}
	void ResetDragItem()
	{
		pObject = 0;
//		GetInterface()->SetDragItem( 0 );
		return;
	}

	bool ProcessMessage( const SEvent &sEvent )
	{
		switch( sEvent.nEvent )
		{
			/*
			case EVENT_MOUSEENTER:
			{
				CDragItemBase *pItemBase = GetInterface()->GetDragItem();
				if ( CDynamicCast< CDragItem<TUser> > pItem( pItemBase ) )
				{
					SendMessage( this, SEvent( EVENT_DRAGANDDROPBEGIN ) );
					pObject = pItem;
				}

				break;
			}
			*/
			case EVENT_MOUSEEXIT:
			{
				if ( IsValid( pObject ) )
					SendMessage( this, SEvent( EVENT_DRAGANDDROPEND ) );

				pObject = 0;
				break;
			}
			case EVENT_MOUSEMOVE:
			{
				CDragItemBase *pItemBase = GetInterface()->GetDragItem();
				if ( CDynamicCast< CDragItem<TUser> > pItem( pItemBase ) )
				{
					if ( pItem != pObject )
					{
						SendMessage( this, SEvent( EVENT_DRAGANDDROPBEGIN ) );
						pObject = pItem;
					}
				}

			}
		}

		return TBaseClass::ProcessMessage( sEvent );
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
