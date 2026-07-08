#include "include/fs.h"
#include "include/ata.h"
#include "include/string.h"

#define OJFS_MAGIC 0x4F4A4653u

#define DIR_LBA_START 1
#define DIR_SECTORS 2
#define DATA_LBA_START 3
#define MAX_FILES 24

typedef struct {
    uint32_t magic;
    uint32_t file_count;
    uint32_t next_free_lba;
    uint8_t reserved[500];
} __attribute__((packed)) ojfs_superblock_t;

typedef struct {
    char name[OJFS_MAX_NAME];
    uint32_t size;
    uint32_t start_lba;
    uint8_t used;
    uint8_t reserved[3];
} __attribute__((packed)) ojfs_entry_t;

static ojfs_superblock_t sb;
static ojfs_entry_t entries[MAX_FILES];

static void load_metadata(void) {
    ata_read_sector(0, (uint8_t*)&sb);

    uint8_t buf[1024];
    ata_read_sector(DIR_LBA_START, buf);
    ata_read_sector(DIR_LBA_START + 1, buf + 512);
    k_memcpy(entries, buf, sizeof(ojfs_entry_t) * MAX_FILES);
}

static void save_metadata(void) {
    ata_write_sector(0, (uint8_t*)&sb);

    uint8_t buf[1024];
    k_memset(buf, 0, sizeof(buf));
    k_memcpy(buf, entries, sizeof(ojfs_entry_t) * MAX_FILES);
    ata_write_sector(DIR_LBA_START, buf);
    ata_write_sector(DIR_LBA_START + 1, buf + 512);
}

void fs_init(void) {
    load_metadata();

    if (sb.magic != OJFS_MAGIC) {
        sb.magic = OJFS_MAGIC;
        sb.file_count = 0;
        sb.next_free_lba = DATA_LBA_START;
        k_memset(entries, 0, sizeof(entries));
        save_metadata();
    }
}

int fs_list(char names[][OJFS_MAX_NAME], uint32_t* sizes, int max_count) {
    int found = 0;
    for (int i = 0; i < MAX_FILES && found < max_count; i++) {
        if (entries[i].used) {
            k_strcpy(names[found], entries[i].name);
            sizes[found] = entries[i].size;
            found++;
        }
    }
    return found;
}

static int find_entry(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && k_strcmp(entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int fs_write(const char* name, const uint8_t* data, uint32_t size) {
    int idx = find_entry(name);

    if (idx < 0) {
        for (int i = 0; i < MAX_FILES; i++) {
            if (!entries[i].used) {
                idx = i;
                break;
            }
        }
        if (idx < 0) return -1;
        sb.file_count++;
    }

    uint32_t sectors_needed = (size + 511) / 512;
    if (sectors_needed == 0) sectors_needed = 1;
    uint32_t start_lba = sb.next_free_lba;

    uint8_t sector_buf[512];
    for (uint32_t s = 0; s < sectors_needed; s++) {
        uint32_t offset = s * 512;
        uint32_t remaining = (offset < size) ? (size - offset) : 0;
        uint32_t chunk = remaining > 512 ? 512 : remaining;

        k_memset(sector_buf, 0, 512);
        if (chunk > 0) {
            k_memcpy(sector_buf, data + offset, chunk);
        }
        ata_write_sector(start_lba + s, sector_buf);
    }

    k_strcpy(entries[idx].name, name);
    entries[idx].size = size;
    entries[idx].start_lba = start_lba;
    entries[idx].used = 1;

    sb.next_free_lba += sectors_needed;

    save_metadata();
    return 0;
}

int fs_read(const char* name, uint8_t* buffer, uint32_t max_size) {
    int idx = find_entry(name);
    if (idx < 0) return -1;

    uint32_t size = entries[idx].size;
    if (size > max_size) size = max_size;

    uint32_t sectors_needed = (entries[idx].size + 511) / 512;
    uint8_t sector_buf[512];
    uint32_t copied = 0;

    for (uint32_t s = 0; s < sectors_needed && copied < size; s++) {
        ata_read_sector(entries[idx].start_lba + s, sector_buf);
        uint32_t chunk = size - copied;
        if (chunk > 512) chunk = 512;
        k_memcpy(buffer + copied, sector_buf, chunk);
        copied += chunk;
    }

    return (int)size;
}
