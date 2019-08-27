// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TableViewBase.h"
#include "ListBox.generated.h"

struct FListBoxItem {
	FString Text;
};

/**
 * 
 */
UCLASS()
class SVGAME_API UListBox : public UTableViewBase
{
	GENERATED_UCLASS_BODY()

public:

	/** The height of each widget */
	UPROPERTY(EditAnywhere, Category = Content)
	float ItemHeight;

	TArray<TSharedPtr<FString>> Items;

	/** The selection method for the list */
	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<ESelectionMode::Type> SelectionMode;

	typedef TSharedPtr<FString> ItemType;

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(UWidget*, FOnGenerateRowEvent, FString, Item);
	FOnGenerateRowEvent OnGenerateRowEvent;

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSelectionChangedEvent, FString, item, ESelectInfo::Type, selType);
	FOnSelectionChangedEvent OnSelectionChangedEvent;

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDoubleClickEvent, FString, item);
	FOnDoubleClickEvent OnDoubleClickEvent;

	void RebuildList()
	{
		if (MyListView.IsValid()) {
			MyListView->RebuildList();
		}
		else {
			TakeWidget();
		}
	}
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	typedef SListView<TSharedPtr<FString>> SListViewType;
protected:
	TSharedPtr<SListViewType> MyListView;

	TSharedRef<ITableRow> HandleOnGenerateRow(TSharedPtr<FString> Item, const TSharedRef< STableViewBase >& OwnerTable) const;
	void HandleSelectionChanged(TSharedPtr<FString> item, ESelectInfo::Type selType);
	void HandleDoubleClick(TSharedPtr<FString> item);

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface	
	
};
