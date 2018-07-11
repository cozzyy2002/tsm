
// StateMachineManualTestDlg.h : header file
//

#pragma once

#include "StateControl.h"
#include <memory>

class CStateControl;

// CStateMachineManualTestDlg dialog
class CStateMachineManualTestDlg : public CDialogEx
{
protected:
	std::unique_ptr<CStateControl> m_stateControl;

// Construction
public:
	CStateMachineManualTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STATEMACHINEMANUALTEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileStatecontrol();
	afx_msg void OnDestroy();
};
