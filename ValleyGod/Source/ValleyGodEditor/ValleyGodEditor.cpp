#include "ValleyGodEditor.h"
#include "ValleyContentFactory.h"
#include "Misc/CoreDelegates.h"

IMPLEMENT_MODULE(FValleyGodEditorModule, ValleyGodEditor);

void FValleyGodEditorModule::StartupModule()
{
	// Type Editor + Default phase so -run=ValleyPrep can FindObject the commandlet
	// before PostEngineInit. Bake itself waits until editor asset tools exist.
	PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddLambda([]()
	{
		FValleyContentFactory::EnsureContent();
	});
}

void FValleyGodEditorModule::ShutdownModule()
{
	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}
}
