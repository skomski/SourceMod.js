#pragma once
#include "SMJS_Module.h"
#include "SMJS_GameRules.h"
#include "filesystem.h"

class MFileSystem : public SMJS_Module {
public:
	MFileSystem();

	void Init();

	FUNCTION_DECL(readFileSync);

	WRAPPED_CLS(MFileSystem, SMJS_Module) {
		temp->SetClassName(v8::String::NewSymbol("FSModule"));
		
		WRAPPED_FUNC(readFileSync);
	}
};
