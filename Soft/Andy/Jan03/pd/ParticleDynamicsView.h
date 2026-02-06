// ParticleDynamicsView.h : interface of the CParticleDynamicsView class
//


#pragma once


class CParticleDynamicsView : public CView
{
protected: // create from serialization only
	CParticleDynamicsView();
	DECLARE_DYNCREATE(CParticleDynamicsView)

// Attributes
public:
	CParticleDynamicsDoc* GetDocument() const;

// Operations
public:
	float fRot;

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CParticleDynamicsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg BOOL OnEraseBkgnd(CDC*) { return TRUE; }
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ParticleDynamicsView.cpp
inline CParticleDynamicsDoc* CParticleDynamicsView::GetDocument() const
   { return reinterpret_cast<CParticleDynamicsDoc*>(m_pDocument); }
#endif

