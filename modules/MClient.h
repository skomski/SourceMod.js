#pragma once
#include "SMJS_Module.h"
#include <vector>
#include <map>
#include "SMJS_Client.h"

#define MAXCLIENTS 24

extern SMJS_Client *clients[MAXCLIENTS];



class MClient : public SMJS_Module {
public:
	MClient();

	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);

	WRAPPED_CLS(MClient, SMJS_Module) {
		temp->SetClassName(v8::String::NewSymbol("ClientModule"));
		
	}

	static void RunAuthChecks();
	static void OnGameFrame(bool simulating);
};
