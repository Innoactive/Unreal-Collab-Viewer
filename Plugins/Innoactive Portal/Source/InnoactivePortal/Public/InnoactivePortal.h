#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FInnoactivePortalModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void AddWidget();

	TSharedPtr<class FUICommandList> PluginCommands;
};
