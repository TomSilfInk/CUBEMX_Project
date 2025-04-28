#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "servo.h"

#define PWM_PERIOD_US 20000  // 20ms = 50Hz
#define MIN_PULSE_US 1000    // 1ms
#define MAX_PULSE_US 2000    // 2ms

// Fonction utilitaire pour le délai en microsecondes
static void delay_us(unsigned int us)
{
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

int servo_init(struct servo_ctx *ctx, unsigned int pin)
{
    ctx->chip = gpiod_chip_open("/dev/gpiochip0");
    if (!ctx->chip) {
        perror("gpiod_chip_open");
        return -1;
    }

    ctx->line = gpiod_chip_get_line(ctx->chip, pin);
    if (!ctx->line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(ctx->chip);
        return -1;
    }

    if (gpiod_line_request_output(ctx->line, "servo", 0) < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(ctx->chip);
        return -1;
    }

    ctx->running = 1;
    return 0;
}

void servo_set_angle(struct servo_ctx *ctx, int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Convert angle to pulse width
    int pulse_width = MIN_PULSE_US + (angle * (MAX_PULSE_US - MIN_PULSE_US) / 180);

    // Generate PWM signal
    while (ctx->running) {
        gpiod_line_set_value(ctx->line, 1);
        delay_us(pulse_width);
        gpiod_line_set_value(ctx->line, 0);
        delay_us(PWM_PERIOD_US - pulse_width);
    }
}

void servo_cleanup(struct servo_ctx *ctx)
{
    ctx->running = 0;
    gpiod_line_release(ctx->line);
    gpiod_chip_close(ctx->chip);
}