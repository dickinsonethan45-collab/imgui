#include "UnityHook.h"
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <elf.h>

#define TAG  "CheeseMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static uintptr_t FindModuleBase(const char* name)
{
    FILE* f = fopen("/proc/self/maps","r");
    if (!f) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line,sizeof(line),f)) {
        if (strstr(line,name)) {
            base = (uintptr_t)strtoull(line,nullptr,16);
            break;
        }
    }
    fclose(f);
    return base;
}

void UnityHook_PatchEGL(void* hookFn, void** origFn)
{
    uintptr_t base = FindModuleBase("libunity.so");
    if (!base) { LOGE("libunity.so not found"); return; }
    LOGI("libunity.so @ 0x%lx", (unsigned long)base);

    auto* ehdr = (Elf64_Ehdr*)base;
    if (ehdr->e_ident[0] != 0x7f) { LOGE("Bad ELF"); return; }

    auto* phdr = (Elf64_Phdr*)(base + ehdr->e_phoff);
    Elf64_Dyn* dyn = nullptr;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)(base + phdr[i].p_vaddr);
            break;
        }
    }
    if (!dyn) { LOGE("No DYNAMIC"); return; }

    Elf64_Rela* rela    = nullptr;
    size_t      relaSz  = 0;
    const char* strtab  = nullptr;
    Elf64_Sym*  symtab  = nullptr;

    for (Elf64_Dyn* d = dyn; d->d_tag != DT_NULL; d++) {
        switch(d->d_tag) {
            case DT_JMPREL:   rela   = (Elf64_Rela*)(base + d->d_un.d_ptr); break;
            case DT_PLTRELSZ: relaSz = d->d_un.d_val; break;
            case DT_STRTAB:   strtab = (const char*)(base + d->d_un.d_ptr); break;
            case DT_SYMTAB:   symtab = (Elf64_Sym*)(base + d->d_un.d_ptr); break;
        }
    }
    if (!rela || !strtab || !symtab) { LOGE("Missing ELF tables"); return; }

    size_t count = relaSz / sizeof(Elf64_Rela);
    for (size_t i = 0; i < count; i++) {
        uint32_t sym = ELF64_R_SYM(rela[i].r_info);
        const char* name = strtab + symtab[sym].st_name;
        if (strcmp(name,"eglSwapBuffers") == 0) {
            void** got = (void**)(base + rela[i].r_offset);
            *origFn = *got;
            uintptr_t page = (uintptr_t)got & ~(uintptr_t)(getpagesize()-1);
            mprotect((void*)page, getpagesize(), PROT_READ|PROT_WRITE);
            *got = hookFn;
            mprotect((void*)page, getpagesize(), PROT_READ);
            LOGI("Patched eglSwapBuffers GOT -> %p", hookFn);
            return;
        }
    }
    LOGE("eglSwapBuffers not in GOT");
}
