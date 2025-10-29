#include "com.h"
#include "net.h"
#include <stdint.h>
#include <stddef.h>

#define GAME_GROUP_ID 1234


int32_t com_init(void) {
    if (net_init() != 0) {
        return -1;
    }
    return net_group_open(GAME_GROUP_ID);
}

int32_t com_deinit(void) {
    net_group_close();
    return net_deinit();
}

int32_t com_write(const void *buf, uint32_t size) {
    return net_send(NULL, buf, size, 0); 
}

int32_t com_read(void *buf, uint32_t size) {
    return net_recv(NULL, buf, size, 0);
}