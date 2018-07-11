#pragma once


// CStateControl dialog

class CStateControl : public CDialogEx
{
	DECLARE_DYNAMIC(CStateControl)

public:
	CStateControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStateControl();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_EVENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
