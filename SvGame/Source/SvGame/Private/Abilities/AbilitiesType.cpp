// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AbilitiesType.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

FAttribs& FAttribs::operator += (const FAttribs &rhs)
{
	for (int i = 0; i < EAttribType::Count; ++i) {
		Attribs[i] += rhs.Attribs[i];
	}

	return *this;
}

FAttribs& FAttribs::operator -= (const FAttribs &rhs)
{
	for (int i = 0; i < EAttribType::Count; ++i) {
		Attribs[i] -= rhs.Attribs[i];
	}

	return *this;
}

FAttribs& FAttribs::operator * (float n)
{
	for (int i = 0; i < EAttribType::Count; ++i) {
		Attribs[i] *= n;
	}

	return *this;
}
