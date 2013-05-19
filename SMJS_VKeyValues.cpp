#include "SMJS_VKeyValues.h"
#include "SMJS_Plugin.h"

WRAPPED_CLS_CPP(SMJS_VKeyValues, SMJS_BaseWrapped);

SMJS_VKeyValues::~SMJS_VKeyValues(){
	for(auto it = restoreValues.begin(); it != restoreValues.end(); ++it){
		delete it->second;
	}
}

void SMJS_VKeyValues::Restore(){
	for(auto it = restoreValues.begin(); it != restoreValues.end(); ++it){
		switch(it->second->m_iDataType){
			case KeyValues::TYPE_STRING:
				kv->SetString2(it->first.c_str(), it->second->asString);
			break;
			case KeyValues::TYPE_INT:
				kv->SetInt(it->first.c_str(), it->second->asInt);
			break;
			case KeyValues::TYPE_FLOAT:
				kv->SetInt(it->first.c_str(), it->second->asFloat);
			break;

		}
	}
}

v8::Handle<v8::Value> SMJS_VKeyValues::GetKeyValue(v8::Local<v8::String> prop, const v8::AccessorInfo &info){
	SMJS_VKeyValues *self = dynamic_cast<SMJS_VKeyValues*>((SMJS_BaseWrapped*)Handle<External>::Cast(info.This()->GetInternalField(0))->Value());
	if(self->kv == NULL) THROW("Invalid keyvalue object");

	v8::String::Utf8Value str(prop);
	switch(self->kv->GetDataType(*str)){
		case KeyValues::TYPE_NONE: return v8::Undefined();
		case KeyValues::TYPE_STRING: return v8::String::New(self->kv->GetString(*str));
		case KeyValues::TYPE_INT: return v8::Int32::New(self->kv->GetInt(*str));
		case KeyValues::TYPE_FLOAT: return v8::Number::New(self->kv->GetFloat(*str));
		default: THROW_VERB("Unknown data type %d", self->kv->GetDataType(*str));
	}
}

v8::Handle<v8::Value> SMJS_VKeyValues::SetKeyValue(v8::Local<v8::String> prop, v8::Local<v8::Value> value, const v8::AccessorInfo &info){
	SMJS_VKeyValues *self = dynamic_cast<SMJS_VKeyValues*>((SMJS_BaseWrapped*)Handle<External>::Cast(info.This()->GetInternalField(0))->Value());
	if(self->kv == NULL) THROW("Invalid keyvalue object");
	
	v8::String::Utf8Value str(prop);
	std::string propStdName(*str);

	switch(self->kv->GetDataType(*str)){
		case KeyValues::TYPE_STRING:
			{
				auto it = self->restoreValues.find(propStdName);
				if(it == self->restoreValues.end()){
					auto r = new VKeyValuesRestore();
					r->asString = self->kv->GetString(*str);
					r->m_iDataType = KeyValues::TYPE_STRING;
					self->restoreValues.insert(std::make_pair(propStdName, r));
				}

				v8::String::Utf8Value vstr(value);
				self->kv->SetString2(*str, *vstr);
			}
		break;
		case KeyValues::TYPE_INT:
			{
				auto it = self->restoreValues.find(propStdName);
					if(it == self->restoreValues.end()){
						auto r = new VKeyValuesRestore();
						r->asInt = self->kv->GetInt(*str);
						r->m_iDataType = KeyValues::TYPE_INT;
						self->restoreValues.insert(std::make_pair(propStdName, r));
					}

				self->kv->SetInt(*str, (int) value->NumberValue());
			}
		break;
		case KeyValues::TYPE_FLOAT:
			{
				auto it = self->restoreValues.find(propStdName);
				if(it == self->restoreValues.end()){
						auto r = new VKeyValuesRestore();
						r->asFloat = self->kv->GetFloat(*str);
						r->m_iDataType = KeyValues::TYPE_FLOAT;
						self->restoreValues.insert(std::make_pair(propStdName, r));
					}

				self->kv->SetFloat(*str, value->NumberValue());
			}
		break;
		default: THROW_VERB("Unknown data type %d", self->kv->GetDataType(*str));
	}

	return value;
}