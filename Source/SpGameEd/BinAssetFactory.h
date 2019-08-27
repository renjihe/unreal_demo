
#pragma once

#include "BinAsset.h"
#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "BinAssetFactory.generated.h"


class FAssetTypeActions_BinAsset : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override {
		return FText::FromString("BinAsset");
	}

	virtual FColor GetTypeColor() const override {
		return FColor(100, 100, 100);
	}

	virtual UClass* GetSupportedClass() const override {
		return UBinAsset::StaticClass();
	}

	virtual uint32 GetCategories() override {
		return EAssetTypeCategories::Gameplay;
	}

	virtual bool IsImportedAsset() const override	{
		return true;
	}

	static FString ResolveSourceFilePath(UObject *binAsset);
	virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
};

UCLASS()
class UBinAssetFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UBinAssetFactory();
	virtual ~UBinAssetFactory();

	// Begin UFactory Interface
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName
		, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;

	/** Returns whether or not the given class is supported by this factory. */
	virtual bool DoesSupportClass(UClass* Class) override;

	/** Returns true if this factory can deal with the file sent in. */
	virtual bool FactoryCanImport(const FString& Filename) override;

	/** Returns the name of the factory for menus */
	virtual FText GetDisplayName() const override;


	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual const UObject* GetFactoryObject() const { 
		return this; 
	}
	//~ End FReimportHandler Interface
};