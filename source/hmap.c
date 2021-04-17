#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmap.h"
#include "hmap_op.h"

#define HOSTS_FPATH "/etc/hosts"

#if !defined(HMAP_VERSION)
#   define HMAP_VERSION "0.0.1"
#endif

static struct hmap_opts opts = {
    .hosts_fpath = {0},
    .map_string  = {0},
    .spec_host   = {0},
    .op          = SHOWMAPS
};

static void print_host_map(struct host_map* _host_map)
{
    printf("%s <- ", _host_map->ip);

    for (uint8_t i = 0; i < _host_map->num_hosts; ++i)
    {
        printf("%s,", _host_map->hosts[i]);
    }

    printf("\n");
}

static void print_hmap(struct hmap* _hmap)
{
    for (uint32_t i = 0; i < _hmap->num_maps; ++i)
    {
        print_host_map(&_hmap->maps[i]);
    }
}

static void print_validation_res(struct validate_context* v_context)
{
    if (v_context->vcode == VCODE_NONE)
    {
        printf_success("File validated successfully!\n");
    }
    else if (v_context->vcode == VCODE_WARNING)
    {
        printf_warning("Warning: ");
        fprintf(stderr,
                "%s:%lu:%u: \033[1;35mwarning: %s\n",
                opts.hosts_fpath,
                v_context->line_num,
                v_context->char_num,
                ERR_MSGS[v_context->err_code]);
        fprintf(stderr, "%s\n", v_context->line);

        for (uint16_t i = 0; i < v_context->char_num; ++i)
            fprintf(stderr, " ");

        fprintf(stderr, "\033[1;35m^\033[0m\n");
    }
    else
    {
        printf_error("Error: ");
        fprintf(stderr,
                "%s:%lu:%u: \033[1;31merror:\033[0m %s\n",
                opts.hosts_fpath,
                v_context->line_num,
                v_context->char_num,
                ERR_MSGS[v_context->err_code]);
        
        fprintf(stderr, "%s\n", v_context->line);

        for (uint16_t i = 0; i < v_context->char_num; ++i)
            fprintf(stderr, " ");

        fprintf(stderr, "\033[1;31m^\033[0m\n");
    }
}

static void print_help()
{
    printf("HMAP v%s\n", HMAP_VERSION);
    printf("Usage: hmap [OPTIONS [ARGUMENTS]]\n\n");

    printf("Options:\n");
    printf("  --file-path or -f <file path>        Specify what hosts file to use (default is /etc/hosts).\n");
    printf("  --help or -h <file path>             Display this menu.\n");
    printf("  --map or -m <map>                    Host(s)-IP Map to add to the hosts file. See --map-format.\n");
    printf("  --map-format                         Display format of maps and shows examples.\n");
    printf("  --operation or -o <hmap operation>   Set the operation (default is showmaps, see --show-operations).\n");
    printf("  --show-operations                    Lists all available operations.\n");
    printf("  --spec-host or -s <host>             Host to remove. Required only if operation is rmhost.\n");
    printf("  --version or -v                      Displays the version of hmap.\n");
    exit(EXIT_SUCCESS);
}

static void print_version()
{
    printf("HMAP v%s\n", HMAP_VERSION);
    exit(EXIT_SUCCESS);
}

static void show_operations()
{
    printf("HMAP v%s\n\n", HMAP_VERSION);

    printf("Operations    |    Description");
    printf("------------------------------------------------------------------------------");
    printf("addmap        |    Adds a host to IP map (using a map string, --map).\n");
    printf("rmhost        |    Removes a host from the hosts file. Requires --spec-host.\n");
    printf("showmaps      |    Displays the maps of the hosts file.\n");
    printf("validate      |    Validates a specified hosts file.\n");

    exit(EXIT_SUCCESS);
}

static void show_map_format()
{
    printf("HMAP v%s\n\n", HMAP_VERSION);

    printf("A map allows you to specify hosts that point to an IP address (an host to IP map).\n");
    printf("The format is as follows: '/ip/<-/host[,host...]/'. The single or double quotes must"
           "be used, or a \\ to escape characters that are not allowed (such as '<').\n"
           "Spaces are ONLY allowed if using double or single quotes.\n\n");

    printf("Examples:\n");
    printf("'192.168.1.1 <- myrouter'              # maps 'myrouter' to 192.168.1.1, single quotes\n");
    printf("\"192.168.1.1<-   myrouter\"             # maps 'myrouter' to 192.168.1.1, double quotes\n");
    printf("192.168.1.1\\<-myrouter                 # maps 'myrouter' to 192.168.1.1, escaping '<', notice no spaces.\n");
    printf("'::1 <- localhost-v6'                  # maps 'localhost-v6' to ::1\n");
    printf("'100.56.32.54 <- host1, host2, host3'  # maps 'host1', 'host2', and 'host3' to 100.56.32.54\n");

    exit(EXIT_SUCCESS);
}

static void parse_args(int argc, char* argv[])
{
    int c;
    int opt_idx;
    static int map_format = 0;
    static int show_ops = 0;

    static const struct option long_opts[] = {
        {"file-path", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
        {"map", required_argument, 0, 'm'},
        {"map-format", no_argument, &map_format, 1},
        {"operation", required_argument, 0, 'o'},
        {"show-operations", no_argument, &show_ops, 1},
        {"spec-host", required_argument, 0, 's'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0},
    };

    while (1)
    {
        c = getopt_long(argc, argv, "f:hm:o:s:v", long_opts, &opt_idx);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            if (map_format)
                show_map_format();

            if (show_ops)
                show_operations();
            break;
        case 'f':
            strcpy(opts.hosts_fpath, optarg);
            break;
        case 'h':
            print_help();
            break;
        case 'm':
            strcpy(opts.map_string, optarg);
            break;
        case 'o':
            if (strcasecmp(optarg, "addmap") == 0)
                opts.op = ADDMAP;
            else if (strcasecmp(optarg, "rmhost") == 0 || strcasecmp(optarg, "jpg") == 0)
                opts.op = RMHOST;
            else if (strcasecmp(optarg, "showmaps") == 0)
                opts.op = SHOWMAPS;
            else if (strcasecmp(optarg, "validate") == 0)
                opts.op = VALIDATE;
            else
            {
                fprintf(stderr, "Unrecognized operation: %s. Aborting.\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            strcpy(opts.spec_host, optarg);
            break;
        case 'v':
            print_version();
            break;
        case '?':  // fallthrough
        default:
            exit(EXIT_FAILURE);
        }
    }
}

static void validate_args()
{
    if (opts.op == RMHOST && strlen(opts.spec_host) == 0)
    {
        fprintf(stderr, "Host to remove is missing. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    if (opts.op == ADDMAP && strlen(opts.map_string) == 0)
    {
        fprintf(stderr, "Map to is missing. Aborting.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    struct hmap             hmap;
    struct host_map         host_map;
    struct validate_context v_context;
    int                     ext_code = 0;

    parse_args(argc, argv);
    validate_args();

    if (strlen(opts.hosts_fpath) == 0)
    {
        strcpy(opts.hosts_fpath, HOSTS_FPATH);
    }

    if (opts.op == ADDMAP)
    {
        if (host_str2host_map(opts.map_string, &host_map) < 0)
        {
            printf_error("Invalid map string. See --help and --map-format for more information.\n");
            exit(EXIT_FAILURE);
        }

        if (read_hosts(&hmap, opts.hosts_fpath) < 0)
        {
            printf_error("Failed to parse hosts file.\n");
            exit(EXIT_FAILURE);
        }

        if (!is_valid_ip(host_map.ip))
        {
            printf_error("'%s' is not a valid IP address.\n", host_map.ip);
            exit(EXIT_FAILURE);
        }

        for (uint8_t i = 0; i < host_map.num_hosts; ++i)
        {
            if (is_host_in_use(&hmap, host_map.hosts[i]))
            {
                printf_error("'%s' is already pointing to an IP address.\n", host_map.hosts[i]);
                exit(EXIT_FAILURE);
            }

            if (!is_valid_host(host_map.hosts[i]))
            {
                printf_error("'%s' is not a valid hostname/domain name.\n", host_map.hosts[i]);
                exit(EXIT_FAILURE);
            }
        }

        printf_normal("Adding the following map:\n");
        print_host_map(&host_map);

        if (add_host(opts.hosts_fpath, &host_map) < 0)
        {
            printf_error("Failed to add new host. Make sure you have enough permission and have a valid hosts file. See --help for more information.\n");
            exit(EXIT_FAILURE);
        }

        printf_success("Map added successfully.\n");
    }
    else if (opts.op == RMHOST)
    {
        printf_error("Operation not available. Wait for updates.\n");
    }
    else if (opts.op == SHOWMAPS)
    {
        if (read_hosts(&hmap, opts.hosts_fpath) < 0)
        {
            printf_error("Failed to parse hosts file.\n");
            exit(EXIT_FAILURE);
        }

        printf_normal("Maps for %s:\n", opts.hosts_fpath);
        print_hmap(&hmap);
    }
    else if (opts.op == VALIDATE)
    {
        if (!is_valid_hosts_file(opts.hosts_fpath, &v_context))
        {
            ext_code = EXIT_FAILURE;
        }

        print_validation_res(&v_context);
    }

    return ext_code;
}
