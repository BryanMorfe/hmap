#include "hmap_op.h"

#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LENGTH (4096)

#define _public
#define _private static

_private ssize_t read_line(int fd, uint8_t* _line)
{
    static const char LINE_END_CHAR = '\n';
    int               c;
    ssize_t           line_size = 0;
    ssize_t           bytes_read;

    while ((bytes_read = read(fd, (void*)&c, 1)) == 1 && c != LINE_END_CHAR)
    {
        _line[line_size] = c;
        line_size++;
    }

    if (bytes_read == 0)
    {
        _line[line_size] = '\0';
    }
    else
    {
        _line[line_size] = '\n';
        line_size++;
    }
    

    return line_size;
}

_private bool file_exists(const char* path)
{
    if (access(path, F_OK) != -1)
        return true;

    return false;
}

_private size_t num_whitespace(uint8_t* line, size_t line_size, size_t i)
{
    size_t count = 0;
    while(i < line_size && isspace(line[i]))
    {
        ++i;
        count++;
    }
        
    return count;
}

_private void parse_line(uint8_t* line, size_t line_size, struct hmap* _hmap)
{
    static const char COMMENT_CHAR   = '#';

    uint8_t status_idx = 0;  // 0 is ip, 1..n are hosts
    uint8_t j = 0;  // str index
    size_t  i = 0;

    _hmap->maps[_hmap->num_maps].num_hosts = 0;

    i = num_whitespace(line, line_size, i);

    for (; i < line_size; ++i)
    {
        if (line[i] == COMMENT_CHAR)
        {
            break;
        }
        else if (isspace(line[i]))
        {
            i += num_whitespace(line, line_size, i) - 1;  // always 1 or greater bc of above condition

            if (status_idx == 0)
            {
                _hmap->maps[_hmap->num_maps].ip[j] = '\0';
            }
            else
            {
                _hmap->maps[_hmap->num_maps].hosts[status_idx - 1][j] = '\0';
                _hmap->maps[_hmap->num_maps].num_hosts++;
            }

            status_idx++;
            j = 0;
        }
        else
        {
            if (status_idx == 0)
            {
                _hmap->maps[_hmap->num_maps].ip[j] = line[i];
            }
            else
            {
                _hmap->maps[_hmap->num_maps].hosts[status_idx - 1][j] = line[i];
            }

            ++j;
        }
    }

    if (status_idx > 0)
    {
        _hmap->maps[_hmap->num_maps].hosts[status_idx - 1][j] = '\0';
        _hmap->maps[_hmap->num_maps].num_hosts++;
        _hmap->num_maps++;
    }
}

_private char* host_map2str(struct host_map* _host_map, char* line_str)
{
    strcpy(line_str, _host_map->ip);

    for (uint8_t i = 0; i < _host_map->num_hosts; ++i)
    {
        strcat(line_str, "\t");
        strcat(line_str, _host_map->hosts[i]);
    }

    return line_str;
}

_public int read_hosts(struct hmap* _hmap, char* fpath)
{
    assert(_hmap != NULL);
    int      fd;
    uint8_t  line[MAX_LINE_LENGTH];
    ssize_t  line_size;
    uint32_t current_capacity = 32;

    if (!file_exists(fpath))
        return -EEXIST;
    
    fd = open(fpath, O_RDONLY);

    if (fd < 0)
        return -EACCES;

    _hmap->maps     = calloc(current_capacity, sizeof(struct host_map));
    _hmap->num_maps = 0;
    
    line_size = read_line(fd, line);

    while (line_size > 0)
    {
        line[line_size] = '\0';
        line_size--;

        if (line_size > 0)
            parse_line(line, (size_t)line_size, _hmap);

        line_size = read_line(fd, line);

        if (_hmap->num_maps == current_capacity)
        {
            current_capacity = current_capacity * 3 / 2;
            _hmap->maps = realloc(_hmap->maps, current_capacity * sizeof(struct host_map));
        }
    }

    _hmap->maps = realloc(_hmap->maps, _hmap->num_maps * sizeof(struct host_map));

    close(fd);

    return 0;
}

_public bool is_host_in_use(struct hmap* _hmap, char* _host)
{
    assert(_hmap != NULL);
    bool found = false;

    for (uint32_t i = 0; i < _hmap->num_maps && !found; ++i)
    {
        for (uint8_t j = 0; j < _hmap->maps[i].num_hosts && !found; ++j)
        {
            if (strcmp(_hmap->maps[i].hosts[j], _host) == 0)
            {
                found = true;
            }
        }
    }

    return found;
}

_public int add_host(char* fpath, struct host_map* _host_map)
{
    assert(_host_map != NULL);

    int                fd;
    char*              line_str;
    size_t             line_size;
    int                ret = 0;
    static const char* COMMENT = "# The host below was added by HMAP";
    static const char  NEW_LINE[] = {'\n'};

    if (!file_exists(fpath))
        return -EEXIST;
    
    fd = open(fpath, O_WRONLY | O_APPEND);

    if (fd < 0)
        return fd;

    line_str = malloc(MAX_LINE_LENGTH + 2);
    host_map2str(_host_map, line_str);
    line_size = strlen(line_str);

    if (write(fd, COMMENT, strlen(COMMENT)) != strlen(COMMENT)
        || write(fd, NEW_LINE, sizeof(NEW_LINE) / sizeof(NEW_LINE[0])) != sizeof(NEW_LINE) / sizeof(NEW_LINE[0])
        || write(fd, (void*)line_str, line_size) != line_size
        || write(fd, NEW_LINE, sizeof(NEW_LINE) / sizeof(NEW_LINE[0])) != sizeof(NEW_LINE) / sizeof(NEW_LINE[0]))
    {
        ret = -EIO;
    }

    free(line_str);
    close(fd);

    return ret;
}

_public int str2host_map(const char* str, struct host_map* _host_map)
{
    static const char MAP_SEPARATOR[] = {'<', '-'};
    static const char HOST_SEPARATOR  = ','; 
    size_t            len             = strlen(str);
    uint8_t           j               = 0;  // item idx
    uint16_t          k               = 0;  // char idx

    _host_map->num_hosts = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (!isspace(str[i]))
        {
            if (str[i] == MAP_SEPARATOR[0])
            {
                if (j > 0)
                {
                    return -EINVAL;
                }

                _host_map->ip[k] = '\0';

                i++;
                if (str[i] != MAP_SEPARATOR[1])
                    return -EINVAL;
                
                j++;
                k = 0;
            }
            else if (str[i] == HOST_SEPARATOR)
            {
                _host_map->hosts[j - 1][k] = '\0';
                _host_map->num_hosts++;
                j++;
                k = 0;
            }
            else
            {
                if (j == 0)  // parsing IP
                {
                    _host_map->ip[k] = str[i];
                    k++;
                }
                else         // Parsing hosts
                {
                    _host_map->hosts[j - 1][k] = str[i];
                    k++;
                }
            }
        }
    }

    if (j == 0)
        return -EINVAL;
    else
    {
        _host_map->num_hosts++;
        _host_map->hosts[j - 1][k] = '\0';
    }

    return 0;
}

_public void clear_hosts(struct hmap* _hmap)
{
    assert(_hmap != NULL);
    free(_hmap->maps);
}
