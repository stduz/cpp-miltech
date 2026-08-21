#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_timer.h"

#define I2C_PORT     I2C_NUM_0
#define I2C_SDA      21
#define I2C_SCL      22
#define I2C_FREQ     400000

#define MPU_ADDR     0x68
#define REG_PWR_MGMT 0x6B
#define REG_ACCEL_X  0x3B

#define UART_PORT    UART_NUM_0
#define UART_BUF     256
#define ACCEL_SCALE  16384.0f

static volatile bool     g_measure   = false;
static volatile uint32_t g_period_ms = 500;
static char              g_mode[16]  = "normal";

static void timer_cb(void*) {
    g_measure = true;
}

static esp_timer_handle_t g_timer;

static void restart_timer(uint32_t ms) {
    esp_timer_stop(g_timer);
    esp_timer_start_periodic(g_timer, (uint64_t)ms * 1000);
}

static int mpu_write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buf, 2, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

static int mpu_read(uint8_t reg, uint8_t* data, int len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

static void parse_cmd(const char* line) {
    char cmd[32];
    int  val;
    if (sscanf(line, "%31s %d", cmd, &val) == 2) {
        if (strcmp(cmd, "p") == 0 && val >= 50 && val <= 10000) {
            g_period_ms = (uint32_t)val;
            restart_timer(g_period_ms);
            if (val <= 200)      snprintf(g_mode, sizeof g_mode, "fast");
            else if (val >= 800) snprintf(g_mode, sizeof g_mode, "slow");
            else                 snprintf(g_mode, sizeof g_mode, "normal");
            printf("ok period=%ums mode=%s\n", (unsigned)g_period_ms, g_mode);
            return;
        }
    }
    if (strncmp(line, "reset", 5) == 0) {
        g_period_ms = 500;
        snprintf(g_mode, sizeof g_mode, "normal");
        restart_timer(g_period_ms);
        printf("ok reset period=500ms mode=normal\n");
        return;
    }
    printf("err unknown command\n");
}

extern "C" void app_main(void) {
    // I2C init
    i2c_config_t i2c_cfg = {};
    i2c_cfg.mode             = I2C_MODE_MASTER;
    i2c_cfg.sda_io_num       = I2C_SDA;
    i2c_cfg.scl_io_num       = I2C_SCL;
    i2c_cfg.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_cfg.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_cfg.master.clk_speed = I2C_FREQ;
    i2c_param_config(I2C_PORT, &i2c_cfg);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(100));
    mpu_write(REG_PWR_MGMT, 0x00); // wake MPU-6050

    // UART0 already initialized by IDF; install driver for rx
    uart_driver_install(UART_PORT, UART_BUF * 2, 0, 0, NULL, 0);

    // Periodic timer
    esp_timer_create_args_t ta = {};
    ta.callback = timer_cb;
    ta.name     = "mpu_timer";
    esp_timer_create(&ta, &g_timer);
    esp_timer_start_periodic(g_timer, (uint64_t)g_period_ms * 1000);

    printf("hw18 ready. Commands: p <ms>  reset\n");

    static char rxbuf[UART_BUF];
    int rxpos = 0;

    while (true) {
        // Measure
        if (g_measure) {
            g_measure = false;
            uint8_t raw[6];
            if (mpu_read(REG_ACCEL_X, raw, 6) == ESP_OK) {
                int16_t ax16 = (int16_t)((raw[0] << 8) | raw[1]);
                int16_t ay16 = (int16_t)((raw[2] << 8) | raw[3]);
                int16_t az16 = (int16_t)((raw[4] << 8) | raw[5]);
                float ax = ax16 / ACCEL_SCALE;
                float ay = ay16 / ACCEL_SCALE;
                float az = az16 / ACCEL_SCALE;
                uint32_t t_ms = (uint32_t)(esp_timer_get_time() / 1000);
                printf("t=%u ms  ax=%.2f ay=%.2f az=%.2f  mode=%s\n",
                       (unsigned)t_ms, ax, ay, az, g_mode);
            }
        }

        // UART command
        uint8_t ch;
        while (uart_read_bytes(UART_PORT, &ch, 1, 0) == 1) {
            if (ch == '\r' || ch == '\n') {
                if (rxpos > 0) {
                    rxbuf[rxpos] = '\0';
                    parse_cmd(rxbuf);
                    rxpos = 0;
                }
            } else if (rxpos < UART_BUF - 1) {
                rxbuf[rxpos++] = (char)ch;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
