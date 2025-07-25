#pragma once
#include "pwm_gen_helpers.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "PWM_GEN_CMDS"

static struct {
    struct arg_int *channel;
    struct arg_int *duty;
    struct arg_int *frequency;
    struct arg_end *end;
} channel_args;

static int set_channel_duty_freq(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void**)&channel_args);
    if(nerrors !=0)
    {
        arg_print_errors(stderr,channel_args.end,argv[0]);
        return 1;
    }
    assert(channel_args.channel->count == 1);
    assert(channel_args.duty->count == 1);
    assert(channel_args.frequency->count == 1);

    const ledc_channel_t target_channel = (ledc_channel_t) (channel_args.channel->ival[0]);
    const uint8_t target_duty = (uint8_t) (channel_args.duty->ival[0]);
    const uint32_t target_frequency = (uint32_t) (channel_args.frequency->ival[0]);

    if(change_duty_cycle(target_channel,target_duty) != ESP_OK)
    {
        printf("Cannot change duty cycle to %u\n",target_duty);
        return 1;
    }
    if(change_frequency(target_channel,target_frequency) != ESP_OK)
    {
        printf("Cannot change frequency to %lu",target_frequency);
        return 1;
    }
    return 0;
}

static void register_set_channel_duty_freq(void)
{
    channel_args.channel = arg_int1(NULL,"channel","<chan>","LEDC Channel number");
    channel_args.duty = arg_int1(NULL,"duty","<d>","Duty cycle, percentile");
    channel_args.frequency = arg_int1(NULL,"frequency","<f>","Frequency, Hz");
    channel_args.end = arg_end(3);
    
    const esp_console_cmd_t cmd = {
        .command = "set_channel_duty_freq",
        .help = "Set the duty and frequency for a specific channel",
        .hint = NULL,
        .func = &set_channel_duty_freq,
        .argtable = &channel_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}