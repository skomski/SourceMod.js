#pragma once
#include "SMJS_Module.h"
#include "SMJS_GameRules.h"
#include "filesystem.h"

class MEngine : public SMJS_Module {
public:
	MEngine();

	void Init();

	WRAPPED_CLS(MEngine, SMJS_Module) {
		temp->SetClassName(v8::String::NewSymbol("EngineModule"));
		
	}
};
