#include <napi.h>
#include "shared-mem-rw.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return SharedMemory::Init(env, exports);
}

NODE_API_MODULE(shared_mem_rw, InitAll)