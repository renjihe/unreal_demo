// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameModeCampsite.h"
#include "GUI/DlgCampsite.h"

void ASpGameModeCampsite::BeginPlay()
{
	Super::BeginPlay();
}

void ASpGameModeCampsite::OnInited()
{
	Super::OnInited();

	UDlgCampsite::GetInstance()->Show();
}