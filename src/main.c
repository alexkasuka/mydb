#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <filepath>\n", argv[0]);
    printf("\t -n - create a new database file.\n");
    printf("\t -f - (required) database filepath.\n");
}

int main(int argc, char *argv[]) {
    char *filepath = NULL;
    int c = 0;
    bool newFile = false;

    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;

    while ((c = getopt(argc, argv, "nf:")) != -1) {
        switch (c) {
            case 'f':
                filepath = optarg;
                break;
            case 'n':
                newFile = true;
                break;
            case  '?':
                printf("Error: unknown option.\n");
                break;
            default:
                return -1;
        }
    }
    if (filepath == NULL) {
        printf("Error: <filepath> is a required argument.\n");
        print_usage(argv);
        return 0;
    }
    if (newFile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Error: create database failed.");
            return -1;
        }
        if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
            printf("Error: create database header failed.");
            return -1;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Error: open database failed.");
            return -1;
        }

        if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
            printf("Error: validate database header failed.");
            return -1;
        }
    }

    output_file(dbfd, dbhdr, NULL);

    printf("newfile: %s\n", newFile ? "true" : "false");
    printf("filepath: %s\n", filepath);

    return 0;
}
