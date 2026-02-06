// AIPathTestDoc.h : interface of the CAIPathTestDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_AIPATHTESTDOC_H__1EB72F0F_BB1C_47AA_B13D_C6A5464BE871__INCLUDED_)
#define AFX_AIPATHTESTDOC_H__1EB72F0F_BB1C_47AA_B13D_C6A5464BE871__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTest.h"

class CAIPathTestDoc : public CDocument
{
protected: // create from serialization only
	CAIPathTestDoc();
	DECLARE_DYNCREATE(CAIPathTestDoc)

// Attributes
public:
	CWorld world;
	CVec2 src, dst;
	vector<CVec2> path;

// Operations
public:
	void UpdatePath();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAIPathTestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAIPathTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAIPathTestDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AIPATHTESTDOC_H__1EB72F0F_BB1C_47AA_B13D_C6A5464BE871__INCLUDED_)
