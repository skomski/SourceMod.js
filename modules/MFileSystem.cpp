#include "MFileSystem.h"
#include "SMJS_Plugin.h"

WRAPPED_CLS_CPP(MFileSystem, SMJS_Module)

	
MFileSystem::MFileSystem(){
	identifier = "fs";
	sandboxed = false;
}

FUNCTION_M(MFileSystem::readFileSync)
	PSTR(path);

	//TODO: use libuv
	char *str = SMJS_FileToString(*path);
	auto ret = v8::String::New(str);
	delete str;
	RETURN_SCOPED(ret);
END