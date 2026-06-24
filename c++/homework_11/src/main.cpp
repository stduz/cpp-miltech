#include "drone_link.h"
#include "nav/Navigator.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <gpiod.h>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace dlink;

static volatile bool running = true;
static void sig_handler(int) { running = false; }

static int open_uart(const char* dev) {
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) { perror("open uart"); return -1; }
    termios tio{};
    tcgetattr(fd, &tio);
    cfmakeraw(&tio);
    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);
    tio.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &tio);
    return fd;
}

static void send_control(int fd, float accel, float turn) {
    Control c{accel, turn};
    uint8_t out[64];
    size_t m = encode(PKT_CONTROL, &c, sizeof c, out);
    write(fd, out, m);
}

static void pulse_drop(gpiod_line* line) {
    gpiod_line_set_value(line, 1);
    usleep(80000);
    gpiod_line_set_value(line, 0);
}

int main(int argc, char** argv) {
    const char* uart_dev  = "/tmp/ttyA";
    const char* chip_name = "gpiochip1";
    int start_line_n = 24;
    int drop_line_n  = 23;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "--uart")       && i+1 < argc) uart_dev    = argv[++i];
        else if (!strcmp(argv[i], "--gpiochip")   && i+1 < argc) chip_name   = argv[++i];
        else if (!strcmp(argv[i], "--start-line") && i+1 < argc) start_line_n = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--drop-line")  && i+1 < argc) drop_line_n  = atoi(argv[++i]);
    }

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    gpiod_chip* chip = gpiod_chip_open_by_name(chip_name);
    if (!chip) {
        fprintf(stderr, "error: cannot open %s\n", chip_name);
        return 1;
    }

    gpiod_line* gpio_start = gpiod_chip_get_line(chip, (unsigned)start_line_n);
    gpiod_line* gpio_drop  = gpiod_chip_get_line(chip, (unsigned)drop_line_n);
    if (!gpio_start || !gpio_drop) {
        fprintf(stderr, "error: cannot get gpio lines\n");
        gpiod_chip_close(chip);
        return 1;
    }
    if (gpiod_line_request_output(gpio_start, "drone", 0) < 0 ||
        gpiod_line_request_output(gpio_drop,  "drone", 0) < 0) {
        fprintf(stderr, "error: cannot request gpio lines as output\n");
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_line_set_value(gpio_start, 1);

    int fd = open_uart(uart_dev);
    if (fd < 0) {
        gpiod_chip_close(chip);
        return 1;
    }

    Navigator nav;
    Parser parser;
    bool dropped = false;
    uint8_t buf[256];
    uint8_t payload[260];
    uint8_t ptype, plen;

    while (running) {
        int n = read(fd, buf, sizeof buf);
        if (n <= 0) { usleep(1000); continue; }
        for (int i = 0; i < n; i++) {
            if (!parser.feed(buf[i], ptype, payload, plen)) continue;
            if (ptype == PKT_AMMO) {
                AmmoCfg a;
                memcpy(&a, payload, sizeof a);
                nav.setAmmo(a);
            } else if (ptype == PKT_TARGET) {
                TargetPos t;
                memcpy(&t, payload, sizeof t);
                nav.updateTarget(t);
            } else if (ptype == PKT_TELEMETRY) {
                Telemetry tel;
                memcpy(&tel, payload, sizeof tel);
                NavCmd cmd = nav.compute(tel);
                send_control(fd, cmd.accel, cmd.turnRate);
                if (cmd.drop && !dropped) {
                    dropped = true;
                    pulse_drop(gpio_drop);
                }
            }
        }
    }

    gpiod_line_set_value(gpio_start, 0);
    close(fd);
    gpiod_chip_close(chip);
    return 0;
}
