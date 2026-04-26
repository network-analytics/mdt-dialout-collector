/*
 * Validate the mdt-dialout-collector-lib package: dlopen
 * libgrpc_collector.so and resolve every public C bridge symbol the
 * pmacct integration expects. Any missing symbol or unresolved
 * transitive dep on the runtime gRPC fails the test.
 *
 * Compile:
 *   cc pkg/validate/library.c -ldl -o /tmp/lib_validate
 * Run:
 *   /tmp/lib_validate
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *kSoPath =
    "/opt/mdt-dialout-collector/lib/libgrpc_collector.so";

static const char *kSymbols[] = {
    "InitOptions",
    "FreeOptions",
    "InitGrpcPayload",
    "free_grpc_payload",
    "start_grpc_dialout_collector",
    "stop_grpc_dialout_collector",
    "LoadOptions",
    "VendorThread",
    "LoadThreads",
};

int main(void)
{
    void *h = dlopen(kSoPath, RTLD_NOW | RTLD_GLOBAL);
    if (!h) {
        fprintf(stderr, "dlopen(%s) failed: %s\n", kSoPath, dlerror());
        return 1;
    }
    fprintf(stdout, "dlopen ok: %s\n", kSoPath);

    int missing = 0;
    for (size_t i = 0; i < sizeof(kSymbols)/sizeof(kSymbols[0]); ++i) {
        dlerror();   /* clear */
        void *sym = dlsym(h, kSymbols[i]);
        const char *err = dlerror();
        if (!sym || err) {
            fprintf(stderr, "  MISSING: %s (%s)\n",
                kSymbols[i], err ? err : "null");
            ++missing;
        } else {
            fprintf(stdout, "  ok: %s\n", kSymbols[i]);
        }
    }
    dlclose(h);

    if (missing) {
        fprintf(stderr, "%d symbol(s) missing from %s\n", missing, kSoPath);
        return 2;
    }
    fprintf(stdout, "LIBRARY VALIDATION OK (%zu symbols)\n",
        sizeof(kSymbols)/sizeof(kSymbols[0]));
    return 0;
}
