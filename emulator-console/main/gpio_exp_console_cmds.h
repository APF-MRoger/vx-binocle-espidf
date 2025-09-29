#pragma once
#include "gpio_exp_helper.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "GPIO_EXP_CMDS"

#pragma region setExpIO
// Basic set single IO output
static struct
{
    struct arg_int *exp_id;
    struct arg_int *gpio_id;
    struct arg_int *level;
    struct arg_end *end;
} setExpIO_args;

/// @brief Sets a specific expander output pin high or low
/// @param argc Takes an expander ID, GPIO id and a target level
/// @param argv
/// @return 1 in case of error, 0 otherwise
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
    // Check this expander exists
    if (expanders[setExpIO_args.exp_id->ival[0]] == nullptr)
    {
        printf("Not a valid expander ID\n");
        return 1;
    }
    // Force GPIO output mode
    printf("Forcing GPIO in output mode.\n");
    if (expanders[setExpIO_args.exp_id->ival[0]]->pinMode(setExpIO_args.gpio_id->ival[0], OUTPUT) == false)
        return 1;

    // Switch pin to output state
    if (expanders[setExpIO_args.exp_id->ival[0]]->digitalWrite(setExpIO_args.gpio_id->ival[0], setExpIO_args.level->ival[0]) == false)
        return 1;
    // Feedback
    printf("GPIO %u on expander %u set to %s \n", setExpIO_args.gpio_id->ival[0], setExpIO_args.exp_id->ival[0], setExpIO_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    return 0;
}

/// @brief Register setExpIO
/// @param
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

#pragma endregion

#pragma region printExpStatus
// Expander print status
static struct
{
    struct arg_int *exp_id;
    struct arg_end *end;
} printExpStatus_args;

/// @brief Print the expander pin status
/// @param argc Optionally takes the expander ID (0-indexed)
/// @param argv
/// @return 1 if error, 0 if OK
static int printExpStatus(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&printExpStatus_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, printExpStatus_args.end, argv[0]);
        return 1;
    }
    if (printExpStatus_args.exp_id->count > 1)
    {
        printf("Invalid number of arguments.\n");
        return 1;
    }
    if (printExpStatus_args.exp_id->count == 0)
    {
        uint8_t i = 0;
        while (expanders[i] != nullptr)
        {
            expanders[i]->printStatus();
            i++;
        }
        return 0;
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

/// @brief Register printExpStatus
/// @param
static void register_printExpStatus(void)
{
    printExpStatus_args.exp_id = arg_int0(NULL, NULL, "<expander>", "Expander ID");
    printExpStatus_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "printExpStatus",
        .help = "Print the pin status of a given expander",
        .hint = NULL,
        .func = &printExpStatus,
        .argtable = &printExpStatus_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

#pragma endregion

#pragma region ActiveHiLO set functions

// Basic set single IO output
static struct
{
    struct arg_int *level;
    struct arg_end *end;
} setActHL_args;

/// @brief Set ignition to the target level, or toggles it
/// @param argc
/// @param argv
/// @return
static int set_ignition(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 15;
    const char *nickname = "Ignition";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);

    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set high beams to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_hi_beams(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 14;
    const char *nickname = "Hi beams";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set alternator to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_alternator(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 13;
    const char *nickname = "Alternator";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set brake to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_brake(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 0;
    const char *nickname = "Low brake level";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set parking brake to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_parking_brake(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 1;
    const char *nickname = "Parking brake";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the low oil pressure to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_oil_low(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 2;
    const char *nickname = "Oil pressure low";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the airbag to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_airbag(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 3;
    const char *nickname = "Airbag";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the Check Engine Light to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_CEL(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 4;
    const char *nickname = "CEL";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the right turn indicator to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_right_turn(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 11;
    const char *nickname = "Right Turn";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the left turn indicator to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_left_turn(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 12;
    const char *nickname = "Left Turn";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the ABS turn indicator to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_ABS(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 10;
    const char *nickname = "ABS";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the door to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_door(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 9;
    const char *nickname = "Door";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the coolant low to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_coolant_low(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 8;
    const char *nickname = "Coolant Low";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the button to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_button(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 7;
    const char *nickname = "button";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set alarm to the target level or toggle (also can be the alarm/RPM shift)
/// @param argc
/// @param argv
/// @return
static int set_alarm(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 6;
    const char *nickname = "alarm";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Set the backlight/low beam to the target level, or toggle
/// @param argc
/// @param argv
/// @return
static int set_backlight(int argc, char **argv)
{
    static bool internalST = false;
    uint8_t PIN = 5;
    const char *nickname = "Backlight";
    int nerrors = arg_parse(argc, argv, (void **)&setActHL_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setActHL_args.end, argv[0]);
        return 1;
    }
    assert(setActHL_args.level->count < 2);
    if (setActHL_args.level->count > 0)
        assert(setActHL_args.level->ival[0] < 2 && setActHL_args.level->ival[0] > -1);

    if (expanders[0]->pinMode(PIN, OUTPUT) == false)
    {
        return 1;
    }

    if (setActHL_args.level->count > 0)
    {
        if (expanders[0]->digitalWrite(PIN, !(setActHL_args.level->ival[0])) == false)
        {
            return 1;
        }
        printf("%s set to %s \n", nickname, !(setActHL_args.level->ival[0]) ? "HIGH" : "LOW");
    }
    else
    {
        internalST = expanders[0]->digitalRead(PIN);
        if (expanders[0]->digitalWrite(PIN, !internalST) == false)
        {
            return 1;
        }
        printf("%s toggled to %s \n", nickname, internalST ? "HIGH" : "LOW");
    }
    return 0;
}

/// @brief Register all the direct access ActHiLo functions
/// @param
static void register_set_shortcuts(void)
{
    setActHL_args.level = arg_int0(NULL, NULL, "<level>", "High (1) or Low (0)");
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
        .func = &set_button,
        .argtable = &setActHL_args};

    cmds[14] = {
        .command = "set_alarm",
        .help = "Set the alarm Active Hi/Low or toggle it",
        .hint = NULL,
        .func = &set_alarm,
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

#pragma endregion

#pragma region setAllExpIO

// Basic set all IO output
static struct
{
    struct arg_int *exp_id;
    struct arg_int *level;
    struct arg_end *end;
} setAllExpIO_args;

/// @brief Sets a specific expander outputs all high or low
/// @param argc Takes an expander ID and a target level
/// @param argv
/// @return 1 in case of error, 0 otherwise
static int setAllExpIO(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&setAllExpIO_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setAllExpIO_args.end, argv[0]);
        return 1;
    }
    assert(setAllExpIO_args.exp_id->count == 1);
    assert(setAllExpIO_args.level->count == 1);
    assert(setAllExpIO_args.exp_id->ival[0] > -1);
    assert(setAllExpIO_args.level->ival[0] < 2 && setAllExpIO_args.level->ival[0] > -1);
    // Check this expander exists
    if (expanders[setAllExpIO_args.exp_id->ival[0]] == nullptr)
    {
        printf("Not a valid expander ID\n");
        return 1;
    }
    // Force GPIO output mode
    printf("Forcing all GPIO in output mode.\n");
    if (expanders[setAllExpIO_args.exp_id->ival[0]]->multiPinMode(0xFFFF, OUTPUT) == false)
        return 1;

    // Switch pins to output state
    if (expanders[setAllExpIO_args.exp_id->ival[0]]->multiDigitalWrite(0xFFFF, setAllExpIO_args.level->ival[0]) == false)
        return 1;
    // Feedback
    printf("GPIOs on expander %u set to %s \n", setAllExpIO_args.exp_id->ival[0], setAllExpIO_args.level->ival[0] == 1 ? "HIGH" : "LOW");
    expanders[setAllExpIO_args.exp_id->ival[0]]->printStatus();
    return 0;
}

/// @brief Register setAllExpIO
/// @param
static void register_setAllExpIO(void)
{
    setAllExpIO_args.exp_id = arg_int1(NULL, NULL, "<expander>", "Expander ID");
    setAllExpIO_args.level = arg_int1(NULL, NULL, "<1|0>", "Level to set (numerical)");
    setAllExpIO_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "setAllExpIO",
        .help = "Set all IO at the target level on the desired expander",
        .hint = NULL,
        .func = &setAllExpIO,
        .argtable = &setAllExpIO_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

#pragma endregion

#pragma region setLowResOut setHighResOut

// Set Resistor array on the low caliber output
static struct
{
    struct arg_int *divider;
    struct arg_end *end;
} setxxxResOut_args;

/// @brief Sets the resistor emulator on the low caliber (max 270 Ohm ) with a target divider
/// @param argc Specify the divider (0 to 8, 0 being OC)
/// @param argv
/// @return 1 in case of error, 0 otherwise
static int setLowResOut(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&setxxxResOut_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setxxxResOut_args.end, argv[0]);
        return 1;
    }
    assert(setxxxResOut_args.divider->count == 1);
    assert(setxxxResOut_args.divider->ival[0] < 9 && setxxxResOut_args.divider->ival[0] > -1);
    // Check expander exists
    if (expanders[1] == nullptr)
    {
        printf("Expander error\n");
        return 1;
    }
    // Calculate switch mask
    uint32_t outputMask = 0;
    for (int i = 0; i < setxxxResOut_args.divider->ival[0]; i++)
    {
        outputMask = (outputMask << 1) | 1;
    }
    printf("Mask : %lu\n", outputMask);

    // Force GPIO output mode
    printf("Forcing all GPIO in output mode.\n");
    if (expanders[1]->multiPinMode(0xFFFF, OUTPUT) == false)
        return 1;

    // Reset all pins to low
    if (expanders[1]->multiDigitalWrite(0xFFFF, LOW) == false)
        return 1;

    // Switch pins to output state (check logic)
    if (expanders[1]->multiDigitalWrite(0xFFFF & outputMask, HIGH) == false)
        return 1;
    // Feedback
    if (setxxxResOut_args.divider->ival[0] == 0)
    {
        printf("Resistor emulator set to OC condition (divider %u) \n", setxxxResOut_args.divider->ival[0]);
    }
    else
    {
        printf("Resistor emulator set to approx %.1f (270/%u) \n", (270.0 / ((float)setxxxResOut_args.divider->ival[0])), setxxxResOut_args.divider->ival[0]);
    }
    return 0;
}

/// @brief Sets the resistor emulator on the high caliber (max 2 kOhm ) with a target divider
/// @param argc Specify the divider (0 to 8, 0 being OC)
/// @param argv
/// @return 1 in case of error, 0 otherwise
static int setHighResOut(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&setxxxResOut_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, setxxxResOut_args.end, argv[0]);
        return 1;
    }
    assert(setxxxResOut_args.divider->count == 1);
    assert(setxxxResOut_args.divider->ival[0] < 9 && setxxxResOut_args.divider->ival[0] > -1);
    // Check expander exists
    if (expanders[1] == nullptr)
    {
        printf("Expander error\n");
        return 1;
    }
    // Calculate switch mask
    uint32_t outputMask = 0;
    for (int i = 0; i < setxxxResOut_args.divider->ival[0]; i++)
    {
        outputMask = (outputMask << 1) | 1;
    }

    outputMask = (outputMask << 8);
    printf("Mask : %lu\n", outputMask);

    // Force GPIO output mode
    printf("Forcing all GPIO in output mode.\n");
    if (expanders[1]->multiPinMode(0xFFFF, OUTPUT) == false)
        return 1;

    // Reset all pins to low
    if (expanders[1]->multiDigitalWrite(0xFFFF, LOW) == false)
        return 1;

    // Switch pins to output state (check logic)
    if (expanders[1]->multiDigitalWrite(0xFFFF & outputMask, HIGH) == false)
        return 1;
    // Feedback
    if (setxxxResOut_args.divider->ival[0] == 0)
    {
        printf("Resistor emulator set to OC condition (divider %u) \n", setxxxResOut_args.divider->ival[0]);
    }
    else
    {
        printf("Resistor emulator set to approx %.1f (2000/%u) \n", (2000.0 / ((float)setxxxResOut_args.divider->ival[0])), setxxxResOut_args.divider->ival[0]);
    }
    return 0;
}

/// @brief Register setLowResOut and setHighResOut
/// @param
static void register_setxxxResOut(void)
{
    setxxxResOut_args.divider = arg_int1(NULL, NULL, "<0..8>", "Divider, 0 is open circuit");
    setxxxResOut_args.end = arg_end(3);

    const esp_console_cmd_t cmd_low = {
        .command = "setLowResOut",
        .help = "Set the resistor emulator to a low output caliber with a certain divider",
        .hint = NULL,
        .func = &setLowResOut,
        .argtable = &setxxxResOut_args};

    const esp_console_cmd_t cmd_high = {
        .command = "setHighResOut",
        .help = "Set the resistor emulator to a high output caliber with a certain divider",
        .hint = NULL,
        .func = &setHighResOut,
        .argtable = &setxxxResOut_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_low));
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_high));
}

#pragma endregion

#pragma region getExpMask
// Basic set single IO output
static struct
{
    struct arg_int *exp_id;
    struct arg_end *end;
} getExpMask_args;

/// @brief Get a specific expander's current pin mask
/// @param argc Expander ID as input
/// @param argv
/// @return
static int getExpMask(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&getExpMask_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, getExpMask_args.end, argv[0]);
        return 1;
    }
    assert(getExpMask_args.exp_id->count == 1);
    assert(getExpMask_args.exp_id->ival[0] > -1);

    // Check this expander exists
    if (expanders[getExpMask_args.exp_id->ival[0]] == nullptr)
    {
        printf("Not a valid expander ID\n");
        return 1;
    }

    uint32_t expanderMask = (uint32_t)(expanders[getExpMask_args.exp_id->ival[0]]->multiDigitalRead(0xFFFF));

    // Feedback
    printf("Expander %u mask: %lx\n", getExpMask_args.exp_id->ival[0], expanderMask);
    return 0;
}

/// @brief Register the getExpMask function
/// @param
static void register_getExpMask(void)
{
    getExpMask_args.exp_id = arg_int1(NULL, NULL, "0..2", "Expander ID");
    getExpMask_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "getExpMask",
        .help = "Get the current pin mask of a given expander",
        .hint = NULL,
        .func = &getExpMask,
        .argtable = &getExpMask_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
#pragma endregion