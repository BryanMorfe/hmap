#ifndef HMAP_H
#define HMAP_H

#include <stdio.h>
#include <stdint.h>

#define MAX_IP_LENGTH (49)
#define MAX_HOSTNAME_LENGTH (253)
#define MAX_PATH_LENGTH (255)

#define MAX_MAP_STR_LENGTH (1024)

#define MAX_NUM_ALIASES (10)  // HMAP-specific limit

#define SEVERITY_NONE "[.] "
#define SEVERITY_WARN "\033[33m[!]\033[0m "
#define SEVERITY_NORMAL "\033[34m[*]\033[0m "
#define SEVERITY_ERROR "\033[31m[!]\033[0m "
#define SEVERITY_SUCCESS "\033[32m[*]\033[0m "

#define printf_none(fmt, ...) fprintf(stdout, SEVERITY_NONE fmt, ##__VA_ARGS__)
#define printf_warning(fmt, ...) fprintf(stdout, SEVERITY_WARN fmt, ##__VA_ARGS__)
#define printf_normal(fmt, ...) fprintf(stdout, SEVERITY_NORMAL fmt, ##__VA_ARGS__)
#define printf_error(fmt, ...) fprintf(stderr, SEVERITY_ERROR fmt, ##__VA_ARGS__)
#define printf_success(fmt, ...) fprintf(stdout, SEVERITY_SUCCESS fmt, ##__VA_ARGS__)

struct host_map
{
    char    ip[MAX_IP_LENGTH];
    char    hosts[MAX_NUM_ALIASES+1][MAX_HOSTNAME_LENGTH];
    uint8_t num_hosts;
};

struct hmap
{
    struct host_map* maps;
    uint32_t         num_maps;
};

enum
{
    ADDMAP,
    RMHOST,
    SHOWMAPS,
    VALIDATE,

    HMAX_OP
};

struct hmap_opts
{
    char    hosts_fpath[MAX_PATH_LENGTH];
    char    map_string[MAX_MAP_STR_LENGTH];
    char    spec_host[MAX_HOSTNAME_LENGTH];
    uint8_t op;
};

#endif /* HMAP_H */