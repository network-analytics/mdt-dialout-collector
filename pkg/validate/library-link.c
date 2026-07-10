/*
 * Validate the mdt-dialout-collector-lib package the way pmacct consumes
 * it: include the bridge header via the pkg-config Cflags, link with the
 * pkg-config Libs, and resolve libgrpc_collector.so.0 through the loader
 * cache at runtime (no LD_LIBRARY_PATH).
 *
 * Compile (mirrors pmacct's configure/make):
 *   cc pkg/validate/library-link.c \
 *      $(pkg-config --cflags grpc-collector) \
 *      $(pkg-config --libs grpc-collector) -o /tmp/lib_link_validate
 * Run:
 *   /tmp/lib_link_validate
 */

#include <stdio.h>
#include <grpc_collector_bridge/grpc_collector_bridge.h>

int main(void)
{
    /* Idempotent no-op when the collector was never started. */
    int rc = stop_grpc_dialout_collector();
    printf("LIBRARY LINK VALIDATION OK (stop_grpc_dialout_collector=%d)\n",
        rc);
    return 0;
}
