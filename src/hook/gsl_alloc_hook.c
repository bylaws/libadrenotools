#include "hook_impl.h"

__attribute__((visibility("default"))) int gsl_memory_alloc_pure(uint32_t size, uint32_t flags, void *memDesc) {
	return hook_gsl_memory_alloc_pure(size, flags, memDesc);
}

__attribute__((visibility("default"))) int gsl_memory_free_pure(void *memDesc) {
	return hook_gsl_memory_free_pure(memDesc);
}