
// StateMachineManualTestDlg.h : header file
//

#pragma once
#include "MyContext.h"
#include "ReportView.h"
#include "EventProperties.h"

#include "afxcmn.h"
#include "afxpropertygridctrl.h"

// CStateMachineManualTestDlg dialog
class CStateMachineManualTestDlg : public CDialogEx
{
protected:
	MyContext context;

	CReportView m_statesView;
	CReportView m_logView;
	int m_logNo;

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
protected:
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);
public:
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	CListCtrl m_listLog;
	CListCtrl m_listStates;
	CEventProperties m_eventProperties;
public:
	afx_msg void OnClickedButtonSetup();
	afx_msg void OnClickedButtonShutdown();
	afx_msg void OnClickedButtonTriggerEvent();
protected:
	afx_msg LRESULT OnStateChanged(WPARAM wParam, LPARAM lParam);
};
