#include "include/ata.h"
#include "include/io.h"

#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECCOUNT    0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

#define ATA_CMD_READ    0x20
#define ATA_CMD_WRITE   0x30
#define ATA_CMD_FLUSH   0xE7

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

static int ata_wait_ready(void) {
    int timeout = 100000;
    while ((inb(ATA_STATUS) & ATA_STATUS_BSY) && timeout--) {
        io_wait();
    }
    return timeout > 0 ? 0 : -1;
}

static int ata_wait_drq(void) {
    int timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(ATA_STATUS);
        if (status & ATA_STATUS_ERR) return -1;
        if (status & ATA_STATUS_DRQ) return 0;
        io_wait();
    }
    return -1;
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    if (ata_wait_ready() != 0) return -1;

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_READ);

    if (ata_wait_ready() != 0) return -1;
    if (ata_wait_drq() != 0) return -1;

    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_DATA);
        buffer[i * 2]     = (uint8_t)(data & 0xFF);
        buffer[i * 2 + 1] = (uint8_t)((data >> 8) & 0xFF);
    }

    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    if (ata_wait_ready() != 0) return -1;

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_WRITE);

    if (ata_wait_ready() != 0) return -1;
    if (ata_wait_drq() != 0) return -1;

    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | ((uint16_t)buffer[i * 2 + 1] << 8);
        outw(ATA_DATA, data);
    }

    outb(ATA_COMMAND, ATA_CMD_FLUSH);
    ata_wait_ready();

    return 0;
}
