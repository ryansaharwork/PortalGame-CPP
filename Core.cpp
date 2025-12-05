#include <cstring>
#include <cstdlib>

// Force heap allocations on Mac to also memset to 0xcd
// So that uninitialized variables behave the same on Mac/PC
#if (defined(__APPLE__) || defined(__GNUC__)) && !defined(NDEBUG)
void* operator new(size_t size)
{
	void* ptr = std::malloc(size);
	std::memset(ptr, 0xcd, size);
	return ptr;
}

void* operator new[](size_t size)
{
	void* ptr = std::malloc(size);
	std::memset(ptr, 0xcd, size);
	return ptr;
}

void operator delete(void* ptr, size_t size) noexcept
{
	std::memset(ptr, 0xdd, size);
	std::free(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept
{
	std::memset(ptr, 0xdd, size);
	std::free(ptr);
}
#endif
