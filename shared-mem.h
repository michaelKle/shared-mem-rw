#include <string>

#if defined(_WIN32) || defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class SharedMem final
{
public:
    enum class Permission
    {
        CREATE_RW,
        CREATE_R,
        OPEN_RW,
        OPEN_R,
    };

public:
    SharedMem(const std::string &name, Permission permission, size_t size);
    ~SharedMem();

    bool IsValid() const;

    void *GetMemory() const;
    size_t Size() const;

private:
    bool m_writeable = false;
    std::string m_memoryKey = "";
    size_t m_memorySize = 0;
    void* m_sharedData = nullptr;

#if _WIN32
    HANDLE m_mapHandle = nullptr;
#else
    int m_descriptor = -1;
#endif
};