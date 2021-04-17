#include "hmap_op.h"

#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define __public
#define __private static

/* Private */
__private bool regex_matches_str(const char* _pattern, const char* _str, int _flags)
{
    regex_t reg;
    int     rv     = regcomp(&reg, _pattern, _flags);
    bool    result = rv == 0 && regexec(&reg, _str, 0, NULL, 0) == 0;

    regfree(&reg);

    return result;
}

__private bool is_valid_ipv4(const char* _ip)
{
    return true;
}

__private bool is_valid_ipv6(const char* _ip)
{
    return true;
}

__private size_t num_whitespace(uint8_t* line, size_t line_size, size_t i)
{
    size_t count = 0;
    while(i < line_size && isspace(line[i]))
    {
        ++i;
        count++;
    }
        
    return count;
}

__private uint16_t validate_line(char* line, size_t line_size, struct validate_context* v_context)
{
    static const char COMMENT_CHAR = '#';

    uint16_t cnum = 0;
    uint8_t  j    = 0;
    uint8_t  k    = 0;
    char     cword[MAX_HOSTNAME_LENGTH];

    while (cnum < line_size && v_context->vcode == VCODE_NONE)
    {
        if (isspace(line[cnum]))
        {
            if (cnum == 0)
            {
                v_context->vcode    = VCODE_ERROR;
                v_context->err_code = ERR_WHITESPACE_BEFORE_IP;
            }
            else
            {
                cword[k] = '\0';

                cnum += num_whitespace((uint8_t*)line, line_size, cnum);

                if (j == 0)
                {
                    if (!is_valid_ip(cword))
                    {
                        v_context->vcode    = VCODE_ERROR;
                        v_context->err_code = ERR_INVAL_IP;
                    }
                }
                else
                {
                    if (!is_valid_host(cword))
                    {
                        v_context->vcode    = VCODE_ERROR;
                        v_context->err_code = ERR_INVAL_HOST;
                    }
                }

                j++;
            }
        }
        else if (line[cnum] == COMMENT_CHAR)
        {
            break;
        }
        else if (isalnum(line[cnum]) || line[cnum] == '.' || line[cnum] == '-' || line[cnum] == ':' || line[cnum] == '%')
        {
            cword[k] = line[cnum];
            k++;
        }
        else
        {
            v_context->vcode    = VCODE_ERROR;
            v_context->err_code = ERR_UNEXPECTED_CHAR;
        }

        if (v_context->vcode == VCODE_NONE)
            ++cnum;
    }

    if (j == 0 && cnum > 0)
    {
        v_context->vcode    = VCODE_ERROR;
        v_context->err_code = ERR_NO_HOST;
    }
    else if (j > 0)
    {
        cword[k] = '\0';

        if (!is_valid_host(cword))
        {
            v_context->vcode    = VCODE_ERROR;
            v_context->err_code = ERR_INVAL_HOST;
        }
    }

    return cnum;
}

__private ssize_t read_line(int fd, uint8_t* _line)
{
    static const char LINE_END_CHAR = '\n';
    char              c;
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

__private bool file_exists(const char* path)
{
    if (access(path, F_OK) != -1)
        return true;

    return false;
}

__private void parse_line(uint8_t* line, size_t line_size, struct hmap* _hmap)
{
    static const char COMMENT_CHAR = '#';

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

__private char* host_map2formatted_str(struct host_map* _host_map, char* line_str)
{
    strcpy(line_str, _host_map->ip);

    for (uint8_t i = 0; i < _host_map->num_hosts; ++i)
    {
        strcat(line_str, "\t");
        strcat(line_str, _host_map->hosts[i]);
    }

    return line_str;
}

/* Public */
const char* ERR_MSGS[] = {
    [ERR_NONE]                 = "Success",
    [ERR_FILE]                 = "Unable to open/read hosts file",
    [ERR_WHITESPACE_BEFORE_IP] = "Whitespace before IP address is not allowed",
    [ERR_MIXED_WHITESPACE]     = "HMAP recommends against mixing tabs and spaces",
    [ERR_INVAL_IP]             = "Invalid IP address format",
    [ERR_INVAL_HOST]           = "Invalid host format",
    [ERR_UNEXPECTED_CHAR]      = "Unexpected character",
    [ERR_NO_HOST]              = "No host was provided",
};

__public int read_hosts(struct hmap* _hmap, char* fpath)
{
    assert(_hmap != NULL);
    int      fd;
    uint8_t  line[MAX_LINE_LENGTH + 1];
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

__public int add_host(char* fpath, struct host_map* _host_map)
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
    host_map2formatted_str(_host_map, line_str);
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

__public int host_str2host_map(const char* str, struct host_map* _host_map)
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

__public void clear_hosts(struct hmap* _hmap)
{
    assert(_hmap != NULL);
    free(_hmap->maps);
}

__public bool is_host_in_use(struct hmap* _hmap, char* _host)
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

__public bool is_valid_ip(const char* ip)
{
    return is_valid_ipv4(ip) || is_valid_ipv6(ip);
}

__public bool is_valid_host(const char* host)
{
    return regex_matches_str("\\([a-z]\\)\\([a-z0-9-]\\+[a-z0-9]\\)\\(\\.\\([a-z0-9-]\\+[a-z0-9]\\)\\)*", host, REG_ICASE);
}

__public bool is_valid_hosts_file(const char* fpath, struct validate_context* v_context)
{
    assert(v_context != NULL);
    int      fd;
    ssize_t  line_size;

    v_context->vcode    = VCODE_NONE;
    v_context->err_code = ERR_NONE;
    v_context->line_num = 0;

    if (!file_exists(fpath))
    {
        v_context->vcode    = VCODE_ERROR;
        v_context->err_code = ERR_FILE;
        return false;
    }
    
    fd = open(fpath, O_RDONLY);

    if (fd < 0)
    {
        v_context->vcode    = VCODE_ERROR;
        v_context->err_code = ERR_FILE;
        return false;
    }

    line_size           = read_line(fd, (uint8_t*)v_context->line);
    v_context->line_num++;

    while (v_context->vcode == VCODE_NONE && line_size > 0)
    {
        v_context->line[line_size] = '\0';
        line_size--;

        if (line_size > 0)
            v_context->char_num = validate_line(v_context->line, (size_t)line_size, v_context);


        if (v_context->vcode == VCODE_NONE)
        {
            line_size = read_line(fd, (uint8_t*)v_context->line);
            v_context->line_num++;
        }
    }

    close(fd);
        
    return true;
}