#include "ValleyGodEditor.h"
#include "ValleyContentFactory.h"

IMPLEMENT_MODULE(FValleyGodEditorModule, ValleyGodEditor);

void FValleyGodEditorModule::StartupModule()
{
	FValleyContentFactory::EnsureContent();
}

void FValleyGodEditorModule::ShutdownModule()
{
}
