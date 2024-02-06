#include "InnoactivePortalCommands.h"

#define LOCTEXT_NAMESPACE "FInnoactivePortalModule"

void FInnoactivePortalCommands::RegisterCommands()
{
	UI_COMMAND(PluginCommand, "InnoactivePortal", "Start Innoactive Portal Widget", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE