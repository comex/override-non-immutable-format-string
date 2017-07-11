#include <mach/mach.h>
#include <stdlib.h>
#include <execinfo.h>
#include <stdio.h>
#define DYLD_INTERPOSE(_replacment,_replacee) \
   __attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
            __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned         long)&_replacee };

kern_return_t my_vm_region_64(vm_map_t target_task, vm_address_t *address, vm_size_t *size, vm_region_flavor_t flavor, vm_region_info_t info, mach_msg_type_number_t *infoCnt, mach_port_t *object_name) {
    static void *printf_is_memory_read_only;
    void *ra = __builtin_return_address(0);
    if (!printf_is_memory_read_only) {
        void *callstack[2];
        // lol, this is slow; i would use substitute if i had any idea if it still worked
        int n = backtrace(callstack, sizeof(callstack)/sizeof(*callstack));
        if (n >= 2) {
            char **symbols = backtrace_symbols(callstack, n);
            if (strstr(symbols[1], "__printf_is_memory_read_only"))
                printf_is_memory_read_only = ra;
            free(symbols);
        }
    }
    if (ra == printf_is_memory_read_only) {
        if (flavor != VM_REGION_BASIC_INFO_64) {
            fprintf(stderr, "onifs: wat, wrong flavor\n");
            abort();
        }
        ((vm_region_basic_info_64_t)info)->protection = 0;
        return 0;
    }
    return vm_region_64(target_task, address, size, flavor, info, infoCnt, object_name);
}
DYLD_INTERPOSE(my_vm_region_64, vm_region_64);

__attribute__((constructor))
static void init() {
    char a[16];
    int b;
    sprintf(a, "%n", &b);
}
