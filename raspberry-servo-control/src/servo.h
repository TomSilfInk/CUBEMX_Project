#ifndef SERVO_H
#define SERVO_H

struct servo_ctx {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int running;
};

int servo_init(struct servo_ctx *ctx, unsigned int pin);
void servo_set_angle(struct servo_ctx *ctx, int angle);
void servo_cleanup(struct servo_ctx *ctx);

#endif