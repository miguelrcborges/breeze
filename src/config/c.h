#ifndef CONFIG_C_H

#include "..\c.h"

static BreezePluginSetupFunction(SetupDefaultBar);
static BreezePluginCleanupFunction(BreezePluginCleanupStub);
static BreezePluginWorkspaceChangeCallback(BreezePluginWorkspaceChangeCallbackStub);

#define CONFIG_C_H
#endif
