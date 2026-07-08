#ifndef FS_H
#define FS_H

#include <stdint.h>

#define OJFS_MAX_NAME 28

void fs_init(void);
int fs_list(char names[][OJFS_MAX_NAME], uint32_t* sizes, int max_count);
int fs_write(const char* name, const uint8_t* data, uint32_t size);
int fs_read(const char* name, uint8_t* buffer, uint32_t max_size);

#endif
