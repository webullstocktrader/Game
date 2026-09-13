#include "SiltCountyEditor.h"
#include "SiltContentFactory.h"

IMPLEMENT_MODULE(FSiltCountyEditorModule, SiltCountyEditor);

void FSiltCountyEditorModule::StartupModule()
{
	FSiltContentFactory::EnsureContent();
}

void FSiltCountyEditorModule::ShutdownModule()
{
}
