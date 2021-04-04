#ifndef HMAP_OP_H
#define HMAP_OP_H

#include "hmap.h"

#include <stdbool.h>

int  read_hosts(struct hmap* _hmap, char* fpath);
bool is_host_in_use(struct hmap* _hmap, char* _host);
int  add_host(char* fpath, struct host_map* _host_map);
int  str2host_map(const char* str, struct host_map* _host_map);
void clear_hosts(struct hmap* _hmap);

#endif /* HMAP_OP_H */