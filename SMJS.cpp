#include "SMJS.h"
#include "extension.h"
#include "modules/MSocket.h"

v8::Isolate *mainIsolate;


time_t lastPing;
bool isPaused = false;
SourceMod::IMutex *pingMutex;

//TODO: Read this from sourcemod.js/dota.js
const char *scriptDotaStr = NULL;
v8::ScriptData *scriptDotaData;

static void SMJS_OnPausedTick();
static void OnMessage(Handle<Message> message, Handle<Value> error);
static char* FileToString(const char *file, const char* dir);

class SMJS_CheckerThread : public SourceMod::IThread{
	void RunThread(IThreadHandle *pHandle);
	void OnTerminate(IThreadHandle *pHandle, bool cancel){}
};

void SMJS_Init(){
	mainIsolate = v8::Isolate::GetCurrent();
	HandleScope handle_scope(mainIsolate);

	char smjsPath[512];
	smutils->BuildPath(Path_SM, smjsPath, sizeof(smjsPath), "sourcemod.js");

	scriptDotaStr = FileToString("dota.js", smjsPath);
	if(scriptDotaStr == NULL){
		printf("File \"dota.js\" missing\n");
		getchar();
	}
	scriptDotaData = v8::ScriptData::PreCompile(v8::String::New(scriptDotaStr));

	v8::V8::AddMessageListener(OnMessage);

	
	pingMutex = threader->MakeMutex();
	SMJS_Ping();
	threader->MakeThread(new SMJS_CheckerThread());
}

void SMJS_CheckerThread::RunThread(IThreadHandle *pHandle){
	while(1){
		pingMutex->Lock();

		int now = time(NULL);
		if(!isPaused && now - lastPing > 30){
			v8::V8::TerminateExecution(mainIsolate);
			printf("SERVER STOPPED THINKING, QUITTING\n");
			threader->ThreadSleep(3000);

			// In production, just let the server die so it can be restarted
			// Otherwise we'd have to close the dialog that Windows creates before
			// the server is restarted again
#ifdef DEBUG
			abort();
#else
			exit(3);
#endif
		}

		pingMutex->Unlock();
		threader->ThreadSleep(1000);
	}
}


void OnMessage(Handle<Message> message, Handle<Value> error){
	HandleScope handle_scope(mainIsolate);

	fprintf(stderr, "----------------------------- JAVASCRIPT EXCEPTION -----------------------------");


	v8::String::Utf8Value tmp(message->Get());
	const char* exception_string = *tmp;

	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", exception_string);
	} else {
		// Print (filename):(line number): (message).
		String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = *filename;
		int linenum = message->GetLineNumber();
		fprintf(stderr, "%s:%d: %s\n", filename_string, linenum, exception_string);
		// Print line of source code.
		String::Utf8Value sourceline(message->GetSourceLine());
		const char* sourceline_string = *sourceline;
		fprintf(stderr, "%s\n", sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
			fprintf(stderr, " ");
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
			fprintf(stderr, "^");
		}
		fprintf(stderr, "\n");

		auto stackTrace = message->GetStackTrace();
		if(!stackTrace.IsEmpty()){
			int len = stackTrace->GetFrameCount();

			for(int i = 0; i < len; ++i){
				auto frame = stackTrace->GetFrame(i);
				v8::String::Utf8Value scriptName(frame->GetScriptNameOrSourceURL());
				v8::String::Utf8Value funName(frame->GetFunctionName());
			
				fprintf(stderr, "%s - %s @ line %d\n", *scriptName, *funName, frame->GetLineNumber());
			}
		}
	}

	fprintf(stderr, "--------------------------------------------------------------------------------");
}

void SMJS_Ping(){
	pingMutex->Lock();
	lastPing = time(NULL);
	pingMutex->Unlock();
}

void SMJS_Pause(){
	printf("Entering in deep sleep\n");

	pingMutex->Lock();
	if(isPaused){
		pingMutex->Unlock();
		return;
	}

	isPaused = true;
	pingMutex->Unlock();

	while(1){
		pingMutex->Lock();
		if(!isPaused){
			pingMutex->Unlock();
			return;
		}

		pingMutex->Unlock();

		SMJS_OnPausedTick();

		threader->ThreadSleep(100);
	}
}

void SMJS_Resume(){
	pingMutex->Lock();
	if(!isPaused){
		pingMutex->Unlock();
		return;
	}

	lastPing = time(NULL);
	isPaused = false;
	pingMutex->Unlock();
}

void SMJS_OnPausedTick(){
	MSocket::Process();
}

char* FileToString(const char* file, const char* dir){
	char path[512];
	if(dir != NULL){
#if WIN32
		snprintf(path, sizeof(path), "%s\\%s", dir, file);
#else
		snprintf(path, sizeof(path), "%s/%s", dir, file);
#endif
	}else{
		strcpy(path, file);
	}

	FILE* fileHandle = fopen(path, "rb");
	if(fileHandle == NULL) return false;

	fseek(fileHandle, 0, SEEK_END);
	size_t size = ftell(fileHandle);
	rewind(fileHandle);
	
	char* source = new char[size + 1];
	size_t i = 0;
	while(i < size) {
		i += fread(&source[i], 1, size - i, fileHandle);
		if(ferror(fileHandle)){
			printf("Error reading file %s\n", path);
			perror(NULL);
			fclose(fileHandle);
			delete[] source;
			return false;
		}
	}
	source[size] = '\0';
	fclose(fileHandle);

	return source;
}