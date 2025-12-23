#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
	(void) dbhdr;
	(void) employees;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
	(void) dbhdr;
	(void) employees;
	(void) addstring;
	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
	(void) fd;
	(void) dbhdr;
	(void) employeesOut;
	return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	(void) employees;
	if (fd < 0) {
		printf("Error: got a bad file descriptor from user.\n");
		return STATUS_ERROR;
	}

	dbhdr->version = htons(dbhdr->version);
	dbhdr->count = htons(dbhdr->count);
	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(dbhdr->filesize);

	lseek(fd, 0, SEEK_SET);
	write(fd, dbhdr, sizeof(struct dbheader_t));

	return STATUS_SUCCESS;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
	if (fd < 0) {
		printf("Error: got a bad file descriptor from user.\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Error: malloc failed to crate db header.\n");
		return STATUS_ERROR;
	}
	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("header");
		free(header);
		return STATUS_ERROR;
	}
	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);
	if (header->magic != HEADER_MAGIC) {
		printf("Error: improper header magic.\n");
		free(header);
		return STATUS_ERROR;
	}
	if (header->version != 1) {
		printf("Error: unsupported database version.\n");
		free(header);
		return STATUS_ERROR;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if (dbstat.st_size != header->filesize) {
		printf("Error: corrupted database file.\n");
		free(header);
		return STATUS_ERROR;
	}
	*headerOut = header;
	return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Error: malloc failed to crate db header.\n");
		return STATUS_ERROR;
	}
	header->magic = HEADER_MAGIC;
	header->version = 1;
	header->count = 0;
	header->filesize = sizeof(struct dbheader_t);
	*headerOut = header;

	return STATUS_SUCCESS;
}
