// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "ListBox.h"


UListBox::UListBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = true;

	ItemHeight = 16.0f;
	SelectionMode = ESelectionMode::Single;
}

TSharedRef<SWidget> UListBox::RebuildWidget()
{
	MyListView = SNew(SListViewType)
		.SelectionMode(SelectionMode)
		.ListItemsSource(&Items)
		.ItemHeight(ItemHeight)
		.OnGenerateRow(BIND_UOBJECT_DELEGATE(SListViewType::FOnGenerateRow, HandleOnGenerateRow))
		.OnSelectionChanged(BIND_UOBJECT_DELEGATE(SListViewType::FOnSelectionChanged, HandleSelectionChanged))
		.OnMouseButtonDoubleClick(BIND_UOBJECT_DELEGATE(SListViewType::FOnMouseButtonDoubleClick, HandleDoubleClick))
		//.OnContextMenuOpening(this, &SSocketManager::OnContextMenuOpening)
		//.OnItemScrolledIntoView(this, &SSocketManager::OnItemScrolledIntoView)
		//	.HeaderRow
		//	(
		//		SNew(SHeaderRow)
		//		.Visibility(EVisibility::Collapsed)
		//		+ SHeaderRow::Column(TEXT("Socket"))
		//	);
		;

	return BuildDesignTimeWidget(MyListView.ToSharedRef());
}

void UListBox::HandleSelectionChanged(TSharedPtr<FString> item, ESelectInfo::Type selType)
{
	if (item.IsValid() && OnSelectionChangedEvent.IsBound()) {
		OnSelectionChangedEvent.Execute(*item, selType);
	}
}

TSharedRef<ITableRow> UListBox::HandleOnGenerateRow(TSharedPtr<FString> Item, const TSharedRef< STableViewBase >& OwnerTable) const
{
	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (OnGenerateRowEvent.IsBound())
	{
		UWidget* Widget = OnGenerateRowEvent.Execute(*Item);
		if (Widget != NULL)
		{
			return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
				[
					Widget->TakeWidget()
				];
		}
	}

	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromString(*Item))
		];
}

void UListBox::HandleDoubleClick(TSharedPtr<FString> item)
{
	if (item.IsValid() && OnDoubleClickEvent.IsBound()) {
		OnDoubleClickEvent.Execute(*item);
	}
}

#if WITH_EDITOR

const FText UListBox::GetPaletteCategory()
{
	return FText::FromString("Misc");
}

#endif

void UListBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyListView.Reset();
}
