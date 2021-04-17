#ifndef HMAP_OP_H
#define HMAP_OP_H

#include "hmap.h"

#include <stdbool.h>

#define MAX_LINE_LENGTH (4096)

enum
{
    VCODE_NONE,
    VCODE_WARNING,
    VCODE_ERROR,
};

enum
{
    ERR_NONE,
    ERR_FILE,
    ERR_WHITESPACE_BEFORE_IP,
    ERR_MIXED_WHITESPACE,
    ERR_INVAL_IP,
    ERR_INVAL_HOST,
    ERR_UNEXPECTED_CHAR,
    ERR_NO_HOST,
};

extern const char* ERR_MSGS[];

struct validate_context
{
    uint8_t  vcode;
    int32_t  err_code;
    size_t   line_num;
    uint16_t char_num;
    char     line[MAX_LINE_LENGTH + 1];
};

int  read_hosts(struct hmap* _hmap, char* fpath);
int  add_host(char* fpath, struct host_map* _host_map);
int  host_str2host_map(const char* str, struct host_map* _host_map);
void clear_hosts(struct hmap* _hmap);

bool is_host_in_use(struct hmap* _hmap, char* _host);
bool is_valid_ip(const char* ip);
bool is_valid_host(const char* host);
bool is_valid_hosts_file(const char* fpath, struct validate_context* v_context);

#endif /* HMAP_OP_H */