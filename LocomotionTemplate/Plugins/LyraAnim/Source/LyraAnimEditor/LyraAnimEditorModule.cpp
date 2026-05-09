#include "LyraAnimEditorModule.h"

#include "Debug/SLAAnimDebuggerWindow.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

static const FName LyraAnimDebuggerTabName(TEXT("LyraAnimDebugger"));

#define LOCTEXT_NAMESPACE "FLyraAnimEditorModule"

void FLyraAnimEditorModule::StartupModule()
{
	PluginCommands = MakeShared<FUICommandList>();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		LyraAnimDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FLyraAnimEditorModule::SpawnDebuggerTab))
		.SetDisplayName(LOCTEXT("LyraAnimDebuggerTabTitle", "Lyra Anim Debugger"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLyraAnimEditorModule::RegisterMenus));
}

void FLyraAnimEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LyraAnimDebuggerTabName);

	PluginCommands.Reset();
}

void FLyraAnimEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("WindowLayout"));
	Section.AddMenuEntry(
		TEXT("LyraAnimDebugger"),
		LOCTEXT("LyraAnimDebuggerMenuLabel", "Lyra Anim Debugger"),
		LOCTEXT("LyraAnimDebuggerMenuTooltip", "Open the Lyra Anim Debugger window."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FLyraAnimEditorModule::OpenDebuggerTab)));
}

void FLyraAnimEditorModule::OpenDebuggerTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LyraAnimDebuggerTabName);
}

TSharedRef<SDockTab> FLyraAnimEditorModule::SpawnDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLAAnimDebuggerWindow)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLyraAnimEditorModule, LyraAnimEditor)
