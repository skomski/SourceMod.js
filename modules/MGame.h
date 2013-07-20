#pragma once
#include "SMJS_Module.h"
#include "SMJS_GameRules.h"
#include "filesystem.h"

class MGame :
	public SMJS_Module,
	public IGameEventListener2
{
public:
	SMJS_GameRules rules;

	MGame();

	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);
	void Init();

	static CBaseEntity* FindEntityByClassname(int startIndex, char *searchname);
	static CBaseEntity* NativeFindEntityByClassname(int startIndex, char *searchname);

	// IGameEventListener2
	void FireGameEvent(IGameEvent *pEvent){}
	int GetEventDebugID(){
		return EVENT_DEBUG_ID_INIT;
	}

	FUNCTION_DECL(hook);
	FUNCTION_DECL(getTeamClientCount);
	FUNCTION_DECL(precacheModel);
	FUNCTION_DECL(findEntityByClassname);
	FUNCTION_DECL(findEntitiesByClassname);
	FUNCTION_DECL(findEntityByTargetname);
	FUNCTION_DECL(getTime);
	FUNCTION_DECL(hookEvent);
	FUNCTION_DECL(createEntity);
	FUNCTION_DECL(getPropOffset);
	FUNCTION_DECL(getEntityByIndex);
	FUNCTION_DECL(getEHandleIndex);

	FUNCTION_DECL(pause);
	FUNCTION_DECL(resume);

	WRAPPED_CLS(MGame, SMJS_Module) {
		temp->SetClassName(v8::String::NewSymbol("GameModule"));
		
		proto->Set("rules", v8::Null());

		WRAPPED_FUNC(hook);
		WRAPPED_FUNC(getTeamClientCount);
		WRAPPED_FUNC(precacheModel);
		WRAPPED_FUNC(findEntityByClassname);
		WRAPPED_FUNC(findEntitiesByClassname);
		WRAPPED_FUNC(findEntityByTargetname);
		WRAPPED_FUNC(getTime);
		WRAPPED_FUNC(hookEvent);
		WRAPPED_FUNC(createEntity);
		WRAPPED_FUNC(getPropOffset);
		WRAPPED_FUNC(getEntityByIndex);

		WRAPPED_FUNC(pause);
		WRAPPED_FUNC(resume);
	}

private:
	void InitTeamNatives();
	void OnServerActivate();
	void OnPreServerActivate();
	void OnThink(bool finalTick);
	static void LevelShutdown();
	static bool OnFireEvent(IGameEvent *pEvent, bool bDontBroadcast);
	static bool OnFireEvent_Post(IGameEvent *pEvent, bool bDontBroadcast);

	static FileHandle_t FSOpen(const char *pFileName, const char *pOptions, const char *pathID = NULL);
	static bool FSReadFile(const char *pFileName, const char *pPath, CUtlBuffer &buf, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL);
	static int FSReadFileEx(const char *pFileName, const char *pPath, void **ppBuf, bool bNullTerminate = false, bool bOptimalAlloc = false, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL);
	static FileHandle_t FSOpenEx(const char *pFileName, const char *pOptions, unsigned flags = 0, const char *pathID = 0, char **ppszResolvedFilename = NULL);


	static void OnGameFrame(bool simulating);
};
