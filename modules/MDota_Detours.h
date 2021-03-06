// Keep this file separated from MDota.cpp because it's really ugly

DETOUR_DECL_MEMBER3(ParseUnit, void*, void*, a2, KeyValues*, keyvalues, void*, a4){
	SMJS_Entity *entWrapper = GetEntityWrapper((CBaseEntity*) this);
	
	SMJS_VKeyValues *kv = NULL;

	int len = GetNumPlugins();
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnUnitParsed");

		if(hooks->size() == 0) continue;

		// The new way those functions are called makes it so it's too inefficient to support
		// old plugins
		if(pl->GetApiVersion() >= 4){
			if(kv == NULL){
				kv = new SMJS_VKeyValues((KeyValues2*) keyvalues);
			}

			v8::Handle<v8::Value> args[2];
			args[0] = entWrapper->GetWrapper(pl);
			args[1] = kv->GetWrapper(pl);
			for(auto it = hooks->begin(); it != hooks->end(); ++it){
				auto func = *it;
				func->Call(pl->GetContext()->Global(), 2, args);
			}
		}
	}

	void *ret = DETOUR_MEMBER_CALL(ParseUnit)(a2, kv != NULL ? kv->kv : keyvalues, a4);
	
	if(kv != NULL){
		kv->Restore();
		kv->kv = NULL;
		kv->Destroy();
	}

	return ret;
}


void CallGetAbilityValueHook(CBaseEntity *ability, AbilityData *data){
	auto clsname = gamehelpers->GetEntityClassname((CBaseEntity*) ability);
	auto entWrapper = GetEntityWrapper((CBaseEntity*) ability);

	
	int len = GetNumPlugins();
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnGetAbilityValue");

		if(hooks->size() == 0) continue;

		v8::Handle<v8::Value> args[4];
		args[0] = entWrapper->GetWrapper(pl);
		args[1] = v8::String::New(clsname);
		args[2] = v8::String::New(data->field);

		auto arr = v8::Array::New();
		int len = -1;

		for(int i=0;i<10;++i){
			++len;
			if(data->values[i].type == FT_Int){
				arr->Set(i, v8::Int32::New(data->values[i].value.asInt));
			}else if(data->values[i].type == FT_Float){
				arr->Set(i, v8::Number::New(data->values[i].value.asFloat));
			}else if(data->values[i].type == FT_Void){
				break;
			}else{
				printf("Unknown ability data type: %s.%s %d\n", clsname, data->field, data->values[i].type);
			}
		}
		

		args[3] = arr;

		for(auto it = hooks->begin(); it != hooks->end(); ++it){
			auto func = *it;
			auto ret = func->Call(pl->GetContext()->Global(), 4, args);
			if(!ret.IsEmpty() && !ret->IsUndefined()){
				if(!ret->IsArray()) continue;
				auto obj = ret->ToObject();
				if(obj->Get(v8::String::New("length"))->Int32Value() != len) continue;
				for(int i=0;i<len;++i){
					if(data->values[i].type == FT_Int){
						data->values[i].value.asInt = (int) obj->Get(i)->NumberValue();
					}else if(data->values[i].type == FT_Float){
						data->values[i].value.asFloat = (float) obj->Get(i)->NumberValue();
					}
				}

				return;
			}
		}
	}
}

DETOUR_DECL_STATIC1_STDCALL_NAKED(GetAbilityValue, uint8_t *, char*, a2){
	void *ability;
	AbilityData *data;
	int abilityLevel;

	__asm {
		push	ebp
		mov		ebp, esp
		sub		esp, 64

		mov		ability, eax

		push	a2
		mov		eax, ability
		call	GetAbilityValue_Actual
		mov		data, eax
	}

	__asm pushad
	__asm pushfd

	if(ability == NULL || data == NULL || strlen(a2) == 0) goto FEND;
	
	abilityLevel = *(int*)((intptr_t)ability + 984) - 1;
	
	CallGetAbilityValueHook((CBaseEntity*) ability, data);
	
	
FEND:
	__asm{
		popfd
		popad
		mov		eax, data
		leave
		ret		4
	}
}

const char* CallClientPickHero(CBaseEntity *client, CCommand *cmd, bool* block){
	int len = GetNumPlugins();

	auto clientWrapper = GetEntityWrapper(client);

	*block = false;

	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnHeroPicked");

		if(hooks->size() == 0) continue;
		
		if(pl->GetApiVersion() < 2){
			v8::Handle<v8::Value> args = v8::String::New(cmd->Arg(1));

			for(auto it = hooks->begin(); it != hooks->end(); ++it){
				auto func = *it;
				auto ret = func->Call(pl->GetContext()->Global(), 1, &args);
				if(!ret.IsEmpty() && ret->IsString()){
					v8::String::AsciiValue ascii(ret);
					return *ascii;
				}
			}
		}else{
			v8::Handle<v8::Value> args[2];
			args[0] = clientWrapper->GetWrapper(pl);
			args[1] = v8::String::New(cmd->Arg(1));

			for(auto it = hooks->begin(); it != hooks->end(); ++it){
				auto func = *it;
				auto ret = func->Call(pl->GetContext()->Global(), 2, args);
				if(!ret.IsEmpty() && !ret->IsUndefined()){
					if(ret->IsString()){
						v8::String::AsciiValue ascii(ret);
						char *str = new char[ascii.length() + 1];
						strcpy(str, *ascii);
						return str;
					}else{
						*block = true;
						return NULL;
					}
				}
			}
		}
	}
	return NULL;
}

DETOUR_DECL_STATIC1_STDCALL_NAKED(ClientPickHero, void, CCommand*, cmd){
	CBaseEntity *client;
	const char *newHero;
	bool block;
	CCommand *tmp;

	__asm {
		push	ebp
		mov		ebp, esp
		sub		esp, 64

		mov		client, eax
	}

	__asm pushad
	__asm pushfd
	
	block = false;
	newHero = CallClientPickHero(client, cmd, &block);

	if(!block){
		if(newHero == NULL){
			__asm {
				push	cmd
				mov		eax, client
				call	ClientPickHero_Actual
			}
		}else{
			// It has to be a pointer because I'm too lazy to calculate how much space
			// CCommand would use in the stack
			tmp = new CCommand(*cmd);

			tmp->ArgV()[1] = newHero;

			__asm {
				push	tmp
				mov		eax, client
				call	ClientPickHero_Actual
			}

			delete newHero;
			delete tmp;
		}
	}

	__asm{
		popfd
		popad
		leave
		ret		4
	}
}

DETOUR_DECL_STATIC4_STDCALL(HeroBuyItem, signed int, CBaseEntity*, unit, char*, item, int, playerID, signed int, a4){
	auto unitWrapper = GetEntityWrapper(unit);

	int len = GetNumPlugins();
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnBuyItem");

		if(hooks->size() == 0) continue;

		v8::Handle<v8::Value> args[4];
		args[0] = unitWrapper->GetWrapper(pl);
		args[1] = v8::String::New(item);
		args[2] = v8::Int32::New(playerID);
		args[3] = v8::Int32::New(a4);

		for(auto it = hooks->begin(); it != hooks->end(); ++it){
			auto func = *it;
			auto res = func->Call(pl->GetContext()->Global(), 4, args);
			if(!res.IsEmpty() && !res->IsUndefined() && res->IsFalse()){
				return -1;
			}
		}
	}

	return DETOUR_STATIC_CALL(HeroBuyItem)(unit, item, playerID, a4);
}

DETOUR_DECL_MEMBER0(UnitThink, void){
	CBaseEntity *ent = (CBaseEntity*) this;

	int len = GetNumPlugins();
	auto entWrapper = GetEntityWrapper(ent);

	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnUnitPreThink");

		if(hooks->size() == 0) continue;
		
		v8::Handle<v8::Value> args[1];
		args[0] = entWrapper->GetWrapper(pl);

		for(auto it = hooks->begin(); it != hooks->end(); ++it){
			auto func = *it;
			func->Call(pl->GetContext()->Global(),1, args);
		}
	}

	DETOUR_MEMBER_CALL(UnitThink)();

	canSetState = true;
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnUnitThink");

		if(hooks->size() == 0) continue;
		
		v8::Handle<v8::Value> args[1];
		args[0] = entWrapper->GetWrapper(pl);

		for(auto it = hooks->begin(); it != hooks->end(); ++it){
			auto func = *it;
			func->Call(pl->GetContext()->Global(), 1, args);
		}
	}
	canSetState = false;
}

DETOUR_DECL_MEMBER1(HeroSpawn, void, int, something){
	CBaseEntity *ent = (CBaseEntity*) this;

	DETOUR_MEMBER_CALL(HeroSpawn)(something);

	int len = GetNumPlugins();
	auto entWrapper = GetEntityWrapper(ent);
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;
		
		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto hooks = pl->GetHooks("Dota_OnHeroSpawn");

		if(hooks->size() == 0) continue;
		
		v8::Handle<v8::Value> args[1];
		args[0] = entWrapper->GetWrapper(pl);

		for(auto it = hooks->begin(); it != hooks->end(); ++it){
			auto func = *it;
			func->Call(pl->GetContext()->Global(), 1, args);
		}
	}
}