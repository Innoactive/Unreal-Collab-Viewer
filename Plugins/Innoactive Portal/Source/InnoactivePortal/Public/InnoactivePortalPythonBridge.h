#pragma once

#include "Engine.h"
#include "InnoactivePortalBlueprint.h"
#include "InnoactivePortalPythonBridge.generated.h"

UCLASS(Blueprintable)
class UInnoactivePortalPythonBridge : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = Python)
        static UInnoactivePortalPythonBridge* Get();

    UFUNCTION(BlueprintImplementableEvent, Category = Python)
        void Authenticate(const FPostAuthentication& PostAuthentication) const;

    UFUNCTION(BlueprintImplementableEvent, Category = Python)
        void ValidateCurrentToken(const FString& AccessToken, const FString& RefreshToken, const int32 OrganizationId, const FPostAuthentication& PostAuthentication) const;
};
