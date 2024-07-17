#ifndef SHARED_MEM_RW_H
#define SHARED_MEM_RW_H

#define NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED
// Electron does not allow passing on external memory
// to ArrayBuffers
// The above flag hides the ArrayBuffer::New method
// which takes an external memory pointer

#include <napi.h>
#if defined(_WIN32) || defined(WIN32)
#else
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>
typedef key_t HANDLE;
#endif

#include "shared-mem.h"

class SharedMemory : public Napi::ObjectWrap<SharedMemory>
{
public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	SharedMemory(const Napi::CallbackInfo &info);

private:
	static Napi::FunctionReference constructor;
	Napi::Value Close(const Napi::CallbackInfo &info);
	Napi::Value Memory(const Napi::CallbackInfo &info);
	Napi::Value SharedMemory::Set(const Napi::CallbackInfo& info);

	std::unique_ptr<SharedMem> m_mem;
};

#endif