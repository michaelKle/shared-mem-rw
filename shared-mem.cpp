
#include "shared-mem.h"

void *SharedMem::GetMemory() const
{
    return m_sharedData;
}
size_t SharedMem::Size() const
{
    return m_memorySize;
}

#ifdef _WIN32
#include <stdexcept>
#include <string>

SharedMem::SharedMem(const std::string &name, Permission permission, size_t size)
    : m_writeable(permission == Permission::CREATE_RW || permission == Permission::OPEN_RW), m_memoryKey(std::move(name)), m_memorySize(size)
{
    if (permission == Permission::CREATE_RW)
    {
        m_mapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, m_memorySize, m_memoryKey.c_str());
    }
    else if (permission == Permission::CREATE_R)
    {
        m_mapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READONLY, 0, m_memorySize, m_memoryKey.c_str());
    }
    else
    {
        m_mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, m_memoryKey.c_str());
    }
    if (m_mapHandle == nullptr)
    {
        return;
    }

    if (m_writeable)
    {
        m_sharedData = static_cast<void *>(MapViewOfFile(m_mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, m_memorySize));
    }
    else
    {
        m_sharedData = static_cast<void *>(MapViewOfFile(m_mapHandle, FILE_MAP_READ, 0, 0, m_memorySize));
    }
    if (m_sharedData == nullptr)
    {
        return;
    }
}

SharedMem::~SharedMem()
{
    UnmapViewOfFile(m_sharedData);

    CloseHandle(m_mapHandle);
}
bool SharedMem::IsValid() const
{
    return m_mapHandle && m_sharedData;
}

#else
#include <fcntl.h>
#include <stdexcept>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

SharedMem::SharedMem(const std::string &name, Permission permission, size_t size)
    : m_writeable(permission == Permission::CREATE_RW || permission == Permission::OPEN_RW), m_memoryKey(std::move(name)), m_memorySize(size)
{
    if (permission == Permission::CREATE_RW)
    {
        m_descriptor = shm_open(m_memoryKey.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    }
    else if (permission == Permission::CREATE_R)
    {
        m_descriptor = shm_open(m_memoryKey.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    }
    else if (permission == Permission::OPEN_RW)
    {
        m_descriptor = shm_open(m_memoryKey.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    }
    else if (permission == Permission::OPEN_R)
    {
        m_descriptor = shm_open(m_memoryKey.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
    }

    if (m_descriptor == -1)
    {
        throw std::runtime_error("Can not open/create memory");
    }

    if (m_writeable)
    {
        ftruncate(m_descriptor, m_memorySize);
        m_sharedData = static_cast<void *>(mmap(NULL, m_memorySize, PROT_WRITE | PROT_READ, MAP_SHARED, m_descriptor, 0));
    }
    else
    {
        ftruncate(m_descriptor, m_memorySize);
        m_sharedData = static_cast<void *>(mmap(NULL, m_memorySize, PROT_READ, MAP_SHARED, m_descriptor, 0));
    }
    if (m_sharedData == nullptr)
    {
        throw std::runtime_error("Can not write to memory");
    }
}

SharedMem::~SharedMem()
{
    munmap(m_sharedData, m_memorySize);

    shm_unlink(m_memoryKey.c_str());
}

bool SharedMem::IsValid() const
{
    return m_descriptor && m_sharedData;
}

#endif
