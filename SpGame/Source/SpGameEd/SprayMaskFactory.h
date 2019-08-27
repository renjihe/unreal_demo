#pragma once

#include "SprayMask.h"
#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
#include "SprayMaskFactory.generated.h"


class FAssetTypeActions_SprayMask : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override  {
		return FText::FromString("SprayMask");
	}

	virtual FColor GetTypeColor() const override  {
		return FColor(100, 200, 100);
	}

	virtual UClass* GetSupportedClass() const override {
		return USprayMask::StaticClass();
	}

	virtual uint32 GetCategories() override {
		return EAssetTypeCategories::Gameplay;
	}

	virtual bool IsImportedAsset() const override {
		return true;
	}

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override {

	}
};


UCLASS()
class USprayMaskFactory : public UFactory
{
	GENERATED_BODY()
	
public:
    USprayMaskFactory();
    virtual ~USprayMaskFactory();
    
    // Begin UFactory Interface
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName
		, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
    
    /** Returns whether or not the given class is supported by this factory. */
    virtual bool DoesSupportClass(UClass* Class) override;
    
    /** Returns true if this factory can deal with the file sent in. */
    virtual bool FactoryCanImport(const FString& Filename) override;
    
    /** Returns the name of the factory for menus */
    virtual FText GetDisplayName() const override;    
};
