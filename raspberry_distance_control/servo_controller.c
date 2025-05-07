#include "servo_controller.h"
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define PWM_PERIOD_US 20000  // 20ms = 50Hz
#define MIN_PULSE_US 1000    // 1ms
#define MAX_PULSE_US 2000    // 2ms

static struct gpiod_chip *chip;
static struct gpiod_line *line;
static int running = 0;

// Fonction utilitaire pour le délai en microsecondes
static void delay_us(unsigned int us) {
    usleep(us);
}

int servo_init(void) {
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        perror("gpiod_chip_open");
        return -1;
    }

    line = gpiod_chip_get_line(chip, SERVO_PIN);
    if (!line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return -1;
    }

    if (gpiod_line_request_output(line, "servo", 0) < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(chip);
        return -1;
    }

    running = 1;
    return 0;
}

int servo_set_angle(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Convert angle to pulse width
    int pulse_width = MIN_PULSE_US + (angle * (MAX_PULSE_US - MIN_PULSE_US) / 180);

    // Generate PWM signal
    gpiod_line_set_value(line, 1);
    delay_us(pulse_width);
    gpiod_line_set_value(line, 0);
    delay_us(PWM_PERIOD_US - pulse_width);

    return 0;
}

void servo_cleanup(void) {
    running = 0;
    gpiod_line_release(line);
    gpiod_chip_close(chip);
}