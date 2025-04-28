#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "servo.h"

#define SERVO_PIN 12  // GPIO12

struct servo_ctx servo;
pthread_t pwm_thread;

void *pwm_task(void *arg)
{
    int angle = *((int *)arg);
    servo_set_angle(&servo, angle);
    return NULL;
}

int main(void)
{
    int angle = 0;

    if (servo_init(&servo, SERVO_PIN) < 0) {
        fprintf(stderr, "Failed to initialize servo\n");
        return EXIT_FAILURE;
    }

    printf("Moving servo to different angles. Press Ctrl+C to exit.\n");

    while (1) {
        // Test different angles
        int angles[] = {0, 90, 180};
        
        for (int i = 0; i < 3; i++) {
            angle = angles[i];
            printf("Moving to %d degrees\n", angle);
            
            // Create PWM thread
            if (pthread_create(&pwm_thread, NULL, pwm_task, &angle) != 0) {
                perror("pthread_create");
                break;
            }
            
            sleep(1);  // Hold position for 1 second
            
            // Stop PWM thread
            servo.running = 0;
            pthread_join(pwm_thread, NULL);
            servo.running = 1;
        }
    }

    servo_cleanup(&servo);
    return EXIT_SUCCESS;
}