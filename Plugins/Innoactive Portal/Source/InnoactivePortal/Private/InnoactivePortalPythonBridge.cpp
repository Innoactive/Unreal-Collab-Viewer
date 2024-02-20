#include "InnoactivePortalPythonBridge.h"

UInnoactivePortalPythonBridge* UInnoactivePortalPythonBridge::Get()
{
    TArray<UClass*> PythonBridgeClasses;
    GetDerivedClasses(UInnoactivePortalPythonBridge::StaticClass(), PythonBridgeClasses);
    int32 NumClasses = PythonBridgeClasses.Num();
    if (NumClasses > 0)
    {
        return Cast<UInnoactivePortalPythonBridge>(PythonBridgeClasses[NumClasses - 1]->GetDefaultObject());
    }
    return nullptr;
};
