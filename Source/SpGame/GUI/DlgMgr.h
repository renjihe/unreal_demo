// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <list>

enum EInputEvent;
struct FKey;
class UDlgBase;
typedef std::list<UDlgBase*> DlgList;

/**
 * 
 */
class SPGAME_API FDlgMgr
{
	friend class UDlgBase;
public:
	//GameMode关闭时清除所有的窗口
	void	DestroyAllDlg();
private:
	//窗口create时就调用AddDlg
	void	AddDlg(UDlgBase* dlg);
	void	DelDlg(UDlgBase* dlg);

public:	
	static FDlgMgr* GetInstance();
	bool	PreprocessedInputKey(const FKey Key, const EInputEvent Event);
private:
	static FDlgMgr* Instance;
	FDlgMgr();

private:
	DlgList DlgLt;
};

 #define DlgMgr FDlgMgr::GetInstance()