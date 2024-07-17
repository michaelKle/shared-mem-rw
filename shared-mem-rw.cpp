#include "shared-mem-rw.h"

#include <cstdio>
#include <sstream>
#include <string>

SharedMemory::SharedMemory(const Napi::CallbackInfo &info) : Napi::ObjectWrap<SharedMemory>(info)
{
	Napi::Env env = info.Env();

	// Invoked as constructor: `new MyObject(...)`
	if (info.Length() < 3)
	{
		Napi::Error::New(info.Env(), "[SharedMemory] needs mapName, permission and mapSize").ThrowAsJavaScriptException();
		return;
	}

	if (!info[0].IsString())
	{
		Napi::Error::New(info.Env(), "[SharedMemory] argument 0 needs to be a valid mapName").ThrowAsJavaScriptException();
		return;
	}
	auto path = info[0].As<Napi::String>().Utf8Value();

	if (!info[1].IsString())
	{
		Napi::Error::New(info.Env(), "[SharedMemory] argument 1 needs to be a string").ThrowAsJavaScriptException();
		return;
	}
	auto permAsString = info[1].As<Napi::String>().Utf8Value();
	SharedMem::Permission perm = SharedMem::Permission::CREATE_RW;

	if (permAsString == "CREATE_RW")
		perm = SharedMem::Permission::CREATE_RW;
	else if (permAsString == "CREATE_R")
		perm = SharedMem::Permission::CREATE_R;
	else if (permAsString == "OPEN_RW")
		perm = SharedMem::Permission::OPEN_RW;
	else if (permAsString == "OPEN_R")
		perm = SharedMem::Permission::OPEN_R;
	else
	{
		std::stringstream ss;
		ss << "[SharedMemory] argument 1 needs to be either CREATE_RW, CREATE_R, OPEN_RW, OPEN_R but it is: " << permAsString;
		Napi::Error::New(info.Env(), ss.str()).ThrowAsJavaScriptException();
		return;
	}

	if (!info[2].IsNumber())
	{
		Napi::Error::New(info.Env(), "[SharedMemory] argument 2 needs to be a valid size").ThrowAsJavaScriptException();
		return;
	}
	auto len = (int64_t)info[2].As<Napi::Number>();

	m_mem = std::make_unique<SharedMem>(path, perm, len);
	if (!m_mem->IsValid())
	{
		m_mem.reset();
		Napi::Error::New(info.Env(), "[SharedMemory] Creating or accessing shared mem failed").ThrowAsJavaScriptException();
		return;
	}
		

	this->Value().Set("name", info[0]);
	this->Value().Set("perm", info[1]);
	this->Value().Set("length", info[2]);
}

Napi::Object SharedMemory::Init(Napi::Env env, Napi::Object exports)
{
	Napi::Function func = DefineClass(env, "SharedMemory", {
		InstanceMethod("close", &SharedMemory::Close),
		InstanceMethod("mem", &SharedMemory::Memory),
		InstanceMethod("set", &SharedMemory::Set)
	});

	Napi::FunctionReference *constructor = new Napi::FunctionReference();
	*constructor = Napi::Persistent(func);
	env.SetInstanceData(constructor);

	exports.Set("SharedMemory", func);
	return exports;
}

Napi::Value SharedMemory::Memory(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	auto buffer = Napi::ArrayBuffer::New(env, m_mem->Size());
	std::memcpy(buffer.Data(), m_mem->GetMemory(), m_mem->Size());

	return buffer;
}

Napi::Value SharedMemory::Close(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	m_mem.reset();

	this->Value().Delete("name");
	this->Value().Delete("perm");
	this->Value().Delete("length");

	return Napi::Value();
}

Napi::Value SharedMemory::Set(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	if (info.Length() < 2) 
	{
		Napi::Error::New(info.Env(), "[SharedMemory] Set needs offset and value").ThrowAsJavaScriptException();
		return Napi::Value();
	}
	if (!info[0].IsNumber())
	{
		Napi::Error::New(info.Env(), "[SharedMemory] argument 0 needs to be a valid mapOffset").ThrowAsJavaScriptException();
		return Napi::Value();
	}
	if (!info[1].IsNumber())
	{
		Napi::Error::New(info.Env(), "[SharedMemory] argument 1 needs to be a valid mapValue").ThrowAsJavaScriptException();
		return Napi::Value();
	}
	
	auto offset = static_cast<uint8_t>((int64_t)info[0].As<Napi::Number>());
	auto value = static_cast<uint8_t>((int64_t)info[1].As<Napi::Number>());


	if (offset >= m_mem->Size())
	{
		Napi::Error::New(info.Env(), "[SharedMemory] mapOffset is larger than memory").ThrowAsJavaScriptException();
		return Napi::Value();
	}

	auto u8ptr = static_cast<uint8_t*>(m_mem->GetMemory());
	u8ptr[offset] = value;

	return Napi::Value();
}