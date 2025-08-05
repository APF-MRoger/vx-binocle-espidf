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

// Expander print status
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
    if (printExpStatus_args.exp_id->count != 1)
    {
        printf("Invalid number of arguments.\n");
        return 1;
    }

    if (printExpStatus_args.exp_id->ival[0] < 0)
    {
        printf("Invalid expander ID.\n");
        return 1;
    }

    if (expanders[printExpStatus_args.exp_id->ival[0]] == nullptr)
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

// Basic set single IO output
static struct
{
    struct arg_int *level;
    struct arg_end *end;
} setActHL_args;

static int set_ignition(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 0;
    const char *nickname = "Ignition";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_hi_beams(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 1;
    const char *nickname = "Hi beams";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_alternator(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 2;
    const char *nickname = "Alternator";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_brake(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 3;
    const char *nickname = "Low brake level";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_parking_brake(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 4;
    const char *nickname = "Parking brake";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_oil_low(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 5;
    const char *nickname = "Oil pressure low";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_airbag(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 6;
    const char *nickname = "Airbag";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_CEL(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 7;
    const char *nickname = "CEL";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[0]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_right_turn(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 0;
    const char *nickname = "Right Turn";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_left_turn(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 1;
    const char *nickname = "Left Turn";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_ABS(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 2;
    const char *nickname = "ABS";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_door(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 3;
    const char *nickname = "Door";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_coolant_low(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 4;
    const char *nickname = "Coolant Low";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_Button(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 5;
    const char *nickname = "Button";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_B07(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 6;
    const char *nickname = "B07";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static int set_backlight(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 7;
    const char *nickname = "Backlight";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[1]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[1]->digitalWrite(PIN, setActHL_args.level->ival[0]) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, setActHL_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    }
    else
    {
        if (expanders[1]->digitalWrite(PIN, internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
        internalST = !internalST;
    }
    return 0;
}

static void register_set_shortcuts(void)
{
    setActHL_args.level = arg_int1(NULL, NULL, "<level>", "High (1) or Low (0)");
    setActHL_args.end = arg_end(3);

    esp_console_cmd_t cmds[16];

    cmds[0] = {
        .command = "set_ignition",
        .help = "Set the ignition Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_ignition,
        .argtable = &setActHL_args};

    cmds[1] = {
        .command = "set_hi_beams",
        .help = "Set the hi beams Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_hi_beams,
        .argtable = &setActHL_args};

    cmds[2] = {
        .command = "set_alternator",
        .help = "Set the alternator Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_alternator,
        .argtable = &setActHL_args};

    cmds[3] = {
        .command = "set_brake",
        .help = "Set the brake level low Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_brake,
        .argtable = &setActHL_args};

    cmds[4] = {
        .command = "set_parking_brake",
        .help = "Set the parking brake Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_parking_brake,
        .argtable = &setActHL_args};

    cmds[5] = {
        .command = "set_oil_low",
        .help = "Set the oil pressure low Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_oil_low,
        .argtable = &setActHL_args};

    cmds[6] = {
        .command = "set_airbag",
        .help = "Set the airbag Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_airbag,
        .argtable = &setActHL_args};

    cmds[7] = {
        .command = "set_CEL",
        .help = "Set the CEL Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_CEL,
        .argtable = &setActHL_args};

    cmds[8] = {
        .command = "set_right_turn",
        .help = "Set the right_turn Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_right_turn,
        .argtable = &setActHL_args};

    cmds[9] = {
        .command = "set_left_turn",
        .help = "Set the left turn Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_left_turn,
        .argtable = &setActHL_args};

    cmds[10] = {
        .command = "set_ABS",
        .help = "Set the ABS Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_ABS,
        .argtable = &setActHL_args};

    cmds[11] = {
        .command = "set_door",
        .help = "Set the Door Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_door,
        .argtable = &setActHL_args};

    cmds[12] = {
        .command = "set_coolant_low",
        .help = "Set the coolant low Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_coolant_low,
        .argtable = &setActHL_args};

    cmds[13] = {
        .command = "set_button",
        .help = "Set the button Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_Button,
        .argtable = &setActHL_args};

    cmds[14] = {
        .command = "set_B07",
        .help = "Set the B07 Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_B07,
        .argtable = &setActHL_args};

    cmds[15] = {
        .command = "set_backlight",
        .help = "Set the backlight Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_backlight,
        .argtable = &setActHL_args};


    for (uint8_t i = 0; i < 16; i++)
    {
        ESP_ERROR_CHECK(esp_console_cmd_register(&cmds[i]));
    }
}