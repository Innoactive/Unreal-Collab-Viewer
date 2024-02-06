#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "InnoactivePortalStyle.h"

class FEditorStyle;

class FInnoactivePortalCommands : public TCommands<FInnoactivePortalCommands>
{
public:

	FInnoactivePortalCommands()
		: TCommands<FInnoactivePortalCommands>(TEXT("InnoactivePortal"), NSLOCTEXT("Contexts", "InnoactivePortal", "InnoactivePortal Plugin"), NAME_None, FInnoactivePortalStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginCommand;
};