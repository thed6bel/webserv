#include "header.hpp"

static int whatDirective(char *line)
{
    if (strncmp(line, "listen ", 7) == 0)
        return (7);
    if (strncmp(line, "server_name ", 12) == 0)
        return (12);
    if (strncmp(line, "root ", 5) == 0)
        return (5);
    if (strncmp(line, "index ", 6) == 0)
        return (6);
    if (strncmp(line, "error_page ", 11) == 0)
        return (11);
    if (strncmp(line, "client_max_body_size ", 21) == 0)
        return (21);
    return (0);
}

static int validValue(char *value, int type)
{
    char    *valueDup;

    if (value == NULL)
        return (-1);
    valueDup = strdup(value);
    if (valueDup == NULL)
        return (-1);
    *(strchr(valueDup, ';')) = '\0';
    if ((type == 11 && countWords(valueDup) == 2) || (type != 11 && countWords(valueDup) == 1))
    {
        free(valueDup);
        return (type);
    }
    free(valueDup);
    return (-1);
}

int isRegularLine(char *line)
{
    int i;
    char *semiColon;

    if (line == NULL)
        return (-1);
    semiColon = strchr(line, ';');
    if (strchr(line, ' ') == NULL || semiColon == NULL || *(semiColon + 1) != '\0')
        return (-1);
    else if ((i = whatDirective(line)) == 0)
        return (-1);
    return (validValue(line + i, i));
}

int isServerLine(char *line)
{
    if (line == NULL)
        return (-1);
    if (strcmp(line, "server {") == 0 || strcmp(line, "server{") == 0)
        return (1);
    return (0);
}

int isLocationLine(char *line)
{
    int i = 0;
    char    *curlyBrace;

    if (line == NULL)
        return (-1);
    trimEnd(line);
    while (std::isspace(line[i]))
        i++;
    curlyBrace = strchr(line, '{');
    if (strncmp(line, "location ", 9) == 0 && curlyBrace != NULL && *(curlyBrace + 1) == '\0' && countWords(line) == 3)
        return (1);
    return (0);
}

int serverTypeOfLine(char *line)
{
    int i;

    if (line == NULL)
        return (-1);
    i = 0;
    while (line[i] != '\0' && std::isspace(line[i]))
        i++;
    trimEnd(line);
    if (line[i] == '\0')
        return (0);
    else if (isServerLine(line + i) == 1)
        return (1);
    else if (strcmp(line + i, "}") == 0)
        return (2);
    else if (isLocationLine(line + i) == 1)
        return (3);
    return (isRegularLine(line + i));
}