#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

static const uint8_t MPU_ADDR     = 0x68;
static const uint8_t REG_WHO_AM_I = 0x75;
static const uint8_t REG_PWR_MGMT = 0x6B;
static const uint8_t REG_ACCEL_X  = 0x3B;
static const uint8_t REG_TEMP     = 0x41;
static const uint8_t REG_GYRO_X   = 0x43;

static const float ACCEL_SCALE = 16384.0f; // ±2g
static const float GYRO_SCALE  = 131.0f;   // ±250°/s

static int open_bus(const char* dev) {
    int fd = open(dev, O_RDWR);
    if (fd < 0) { perror("open i2c bus"); return -1; }
    if (ioctl(fd, I2C_SLAVE, MPU_ADDR) < 0) { perror("ioctl I2C_SLAVE"); close(fd); return -1; }
    return fd;
}

static int reg_read(int fd, uint8_t reg, uint8_t* buf, int len) {
    if (write(fd, &reg, 1) != 1) return -1;
    if (read(fd, buf, len) != len) return -1;
    return 0;
}

static int reg_write(int fd, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    if (write(fd, buf, 2) != 2) return -1;
    return 0;
}

static int16_t to16(uint8_t hi, uint8_t lo) {
    return (int16_t)((hi << 8) | lo);
}

int main(int argc, char** argv) {
    const char* dev = (argc > 1) ? argv[1] : "/dev/i2c-1";
    if (argc > 2) {
        unsigned long addr = strtoul(argv[2], nullptr, 0);
        if (addr == 0 || addr > 0x7F) {
            fprintf(stderr, "error: invalid address %s\n", argv[2]);
            return 1;
        }
    }

    int fd = open_bus(dev);
    if (fd < 0) return 1;

    uint8_t who = 0;
    if (reg_read(fd, REG_WHO_AM_I, &who, 1) < 0) {
        fprintf(stderr, "error: cannot read WHO_AM_I\n");
        close(fd); return 1;
    }
    if (who != 0x68) {
        fprintf(stderr, "error: wrong WHO_AM_I=0x%02X (expected 0x68)\n", who);
        close(fd); return 1;
    }
    printf("MPU-6050 found (WHO_AM_I=0x%02X)\n", who);

    if (reg_write(fd, REG_PWR_MGMT, 0x00) < 0) {
        fprintf(stderr, "error: cannot wake up MPU-6050\n");
        close(fd); return 1;
    }

    uint8_t buf[14];
    for (int i = 0; i < 20; i++) {
        if (reg_read(fd, REG_ACCEL_X, buf, 14) < 0) {
            fprintf(stderr, "error: read failed\n");
            close(fd); return 1;
        }

        float ax = to16(buf[0],  buf[1])  / ACCEL_SCALE;
        float ay = to16(buf[2],  buf[3])  / ACCEL_SCALE;
        float az = to16(buf[4],  buf[5])  / ACCEL_SCALE;
        float t  = to16(buf[6],  buf[7])  / 340.0f + 36.53f;
        float gx = to16(buf[8],  buf[9])  / GYRO_SCALE;
        float gy = to16(buf[10], buf[11]) / GYRO_SCALE;
        float gz = to16(buf[12], buf[13]) / GYRO_SCALE;

        printf("Accel: %6.3fg %6.3fg %6.3fg  Gyro: %7.2f %7.2f %7.2f deg/s  Temp: %.1f C\n",
               ax, ay, az, gx, gy, gz, t);
        usleep(200000);
    }

    close(fd);
    return 0;
}
