#pragma once
#include "gpio_exp_helper.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "GPIO_EXP_CMDS"

// Basic set single IO output
static struct
{
    struct arg_int *exp_id;
    struct arg_int *gpio_id;
    struct arg_int *level;
    struct arg_end *end;
} setExpIO_args;

static int setExpIO(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&setExpIO_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setExpIO_args.end, argv[0]);
        return 1;
    }
    assert(setExpIO_args.exp_id->count == 1);
    assert(setExpIO_args.gpio_id->count == 1);
    assert(setExpIO_args.level->count == 1);
    assert(setExpIO_args.gpio_id->ival[0] > -1);
    assert(setExpIO_args.exp_id->ival[0] > -1);
    assert(setExpIO_args.level->ival[0] < 2 && setExpIO_args.level->ival[0] > -1);

    assert(expanders[setExpIO_args.exp_id->ival[0]] != nullptr);

    printf("Forcing GPIO in output mode.\n");
    if (expanders[setExpIO_args.exp_id->ival[0]]->pinMode(setExpIO_args.gpio_id->ival[0], OUTPUT) == false)
        return 1;
    if (expanders[setExpIO_args.exp_id->ival[0]]->digitalWrite(setExpIO_args.gpio_id->ival[0], setExpIO_args.level->ival[0]) == false)
        return 1;
    printf("GPIO %u on expander %u set to %s \n", setExpIO_args.gpio_id->ival[0], setExpIO_args.exp_id->ival[0], setExpIO_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    return 0;
}

static void register_setExpIO(void)
{
    setExpIO_args.exp_id = arg_int1(NULL, NULL, "<expander>", "Expander ID");
    setExpIO_args.gpio_id = arg_int1(NULL, NULL, "<gpio>", "0-indexed GPIO to set");
    setExpIO_args.level = arg_int1(NULL, NULL, "<1|0>", "Level to set (numerical)");
    setExpIO_args.end = arg_end(4);

    const esp_console_cmd_t cmd = {
        .command = "setExpIO",
        .help = "Set an IO at the target level on the desired expander",
        .hint = NULL,
        .func = &setExpIO,
        .argtable = &setExpIO_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

//Expander print status
static struct
{
    struct arg_int *exp_id;
    struct arg_end *end;
} printExpStatus_args;

static int printExpStatus(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&printExpStatus_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, printExpStatus_args.end, argv[0]);
        return 1;
    }
    if(printExpStatus_args.exp_id->count != 1)
    {
        printf("Invalid number of arguments.\n");
        return 1;
    }

    if(printExpStatus_args.exp_id->ival[0] <0)
    {
        printf("Invalid expander ID.\n");
        return 1;
    }

    if(expanders[printExpStatus_args.exp_id->ival[0]] == nullptr)
    {
        printf("Expander does not exist.\n");
        return 1;
    }

    expanders[printExpStatus_args.exp_id->ival[0]]->printStatus();

    return 0;
}

static void register_printExpStatus(void)
{
    printExpStatus_args.exp_id = arg_int1(NULL, NULL, "<expander>", "Expander ID");
    printExpStatus_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "printExpStatus",
        .help = "Print the pin status of a given expander",
        .hint = NULL,
        .func = &printExpStatus,
        .argtable = &printExpStatus_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}