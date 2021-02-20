// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "GLTFFactory.generated.h"


UCLASS()
class GLTFRUNTIMEEDITOR_API UGLTFFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UGLTFFactory();

	UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, 
		const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	void CleanUp() override;
};
