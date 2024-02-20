/** Representation of Portal Application Data */

#pragma once


#include "CoreMinimal.h"
#include "PortalApplication.generated.h"

USTRUCT(BlueprintType)
struct FPortalApplication
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Name"), Category="Innoactive")
	FString Name;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Id", MakeStructureDefaultValue="0"), Category = "Innoactive")
	int32 Id;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Identity"), Category = "Innoactive")
	FString Identity;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Version"), Category = "Innoactive")
	FString Version;
};
