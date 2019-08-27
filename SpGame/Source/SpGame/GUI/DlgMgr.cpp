// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgMgr.h"
#include "GUI/DlgBase.h"
#include "Input/Events.h"
#include "Engine/EngineBaseTypes.h"

FDlgMgr *FDlgMgr::Instance = nullptr;

FDlgMgr::FDlgMgr(): DlgLt()
{
}

FDlgMgr* FDlgMgr::GetInstance()
{
	if (nullptr == Instance) {
		Instance = new FDlgMgr();
	}
	return Instance;
}

void FDlgMgr::DestroyAllDlg()
{
	for (std::list<UDlgBase*>::reverse_iterator beg = DlgLt.rbegin(); beg != DlgLt.rend(); ++beg) {
		(*beg)->SafeDestroy();
	}
	DlgLt.clear();
}

void FDlgMgr::AddDlg(UDlgBase* dlg)
{
	DlgLt.push_back(dlg);
}

void FDlgMgr::DelDlg(UDlgBase* dlg)
{
	for (std::list<UDlgBase*>::iterator i = DlgLt.begin(); i != DlgLt.end(); ++i)
	{
		if (*i == dlg)
		{
			DlgLt.erase(i);
			return;
		}
	}
}

bool FDlgMgr::PreprocessedInputKey(const FKey Key, const EInputEvent Event)
{
	bool bResult = false;
	for (std::list<UDlgBase*>::reverse_iterator beg = DlgLt.rbegin(); !bResult && beg!=DlgLt.rend(); ++beg)
	{
		bResult = (*beg) -> PreprocessedInputKey(Key, Event);
	}
	return bResult;
}