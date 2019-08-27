// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgBase.h"
#include "Blueprint/UserWidget.h"
#include "DlgSingleton.generated.h"


#define DECLARE_DLGBASE_INSTANCE(cname)						\
public:														\
	static cname* GetInstance();							\
	virtual void SafeDestroy() override;					\
	static bool IsInstanced();								\
protected:													\
	static cname* Instance;

#define IMPLEMENT_DLGBASE_ISNTANCE(cname)					\
cname* cname::Instance = nullptr;							\
cname* cname::GetInstance() {								\
	if (nullptr == Instance) {								\
		Instance = NewObject<cname>(GetCurrWorld(),cname::StaticClass());						\
		Instance->Create();									\
	}														\
	return Instance;										\
}															\
void cname::SafeDestroy() 									\
{															\
	Super::SafeDestroy();									\
	Instance = nullptr;										\
}															\
bool cname::IsInstanced() {									\
	return Instance != NULL;								\
}


/**
 * 
 */
UCLASS()
class SPGAME_API UDlgSingleton : public UDlgBase
{
	GENERATED_BODY()
public:
	virtual void Destroy() override;
	virtual void SafeDestroy() override;
	virtual void Create() override;
	virtual void Init() override;
	virtual void UpdateFrame(float DeltaSeconds) {};
};
