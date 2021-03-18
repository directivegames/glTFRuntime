// Copyright 2020, Roberto De Ioris.

#include "glTFRuntimeEditor.h"

#if 1 // WITH_DIRECTIVE
#include "glTFRuntimeSettings.h"
#include "ISettingsModule.h"
#endif

#define LOCTEXT_NAMESPACE "FglTFRuntimeEditorModule"

void FglTFRuntimeEditorModule::StartupModule()
{
#if 1 // WITH_DIRECTIVE
	if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "glTF Runtime",
			LOCTEXT("glTFRuntimeSettingsName", "glTF Runtime"),
			LOCTEXT("glTFRuntimeSettingsDescription", "Configure the glTF Runtime plug-in."),
			GetMutableDefault<UglTFRuntimeSettings>()
		);
	}
#endif
}

void FglTFRuntimeEditorModule::ShutdownModule()
{
#if 1 // WITH_DIRECTIVE
	if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "glTF Runtime");
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FglTFRuntimeEditorModule, glTFRuntimeEditor)
