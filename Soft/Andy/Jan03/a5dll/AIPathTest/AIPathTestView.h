// AIPathTestView.h : interface of the CAIPathTestView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_AIPATHTESTVIEW_H__103E893A_9055_4E45_84A0_35FA774E3709__INCLUDED_)
#define AFX_AIPATHTESTVIEW_H__103E893A_9055_4E45_84A0_35FA774E3709__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CAIPathTestView : public CScrollView
{
protected: // create from serialization only
	CAIPathTestView();
	DECLARE_DYNCREATE(CAIPathTestView)

// Attributes
public:
	CAIPathTestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAIPathTestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAIPathTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAIPathTestView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in AIPathTestView.cpp
inline CAIPathTestDoc* CAIPathTestView::GetDocument()
   { return (CAIPathTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AIPATHTESTVIEW_H__103E893A_9055_4E45_84A0_35FA774E3709__INCLUDED_)
