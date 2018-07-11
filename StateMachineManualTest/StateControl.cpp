// StateControl.cpp : implementation file
//

#include "stdafx.h"
#include "StateMachineManualTest.h"
#include "StateControl.h"
#include "afxdialogex.h"


// CStateControl dialog

IMPLEMENT_DYNAMIC(CStateControl, CDialogEx)

CStateControl::CStateControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_EVENT, pParent)
{
	auto ret = Create(IDD_DIALOG_EVENT, pParent);
}

CStateControl::~CStateControl()
{
}

void CStateControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStateControl, CDialogEx)
END_MESSAGE_MAP()


// CStateControl message handlers
