#include "KFBBEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

IMPLEMENT_GAME_MODULE(FKFBBEditorModule, KFBBEditor)

DEFINE_LOG_CATEGORY(KFBBEditor)

#define LOCTEXT_NAMESPACE "KFBBEditor"

void FKFBBEditorModule::StartupModule()
{
	UE_LOG(KFBBEditor, Warning, TEXT("KFBBEditor: Log Started"));
}

void FKFBBEditorModule::ShutdownModule()
{
	UE_LOG(KFBBEditor, Warning, TEXT("MyGameEditor: Log Ended"));
}

#undef LOCTEXT_NAMESPACE