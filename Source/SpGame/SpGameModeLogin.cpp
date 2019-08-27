// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameModeLogin.h"
#include "GUI/DlgLogin.h"

void ASpGameModeLogin::BeginPlay()
{
	Super::BeginPlay();
}

void ASpGameModeLogin::OnInited()
{
	Super::OnInited();

	UDlgLogin::GetInstance()->Show();
}