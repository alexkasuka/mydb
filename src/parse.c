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

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
	if (NULL == dbhdr) return STATUS_ERROR;
	if (NULL == employees) return STATUS_ERROR;
	if (NULL == *employees) return STATUS_ERROR;
	if (NULL == addstring) return STATUS_ERROR;


	char *name = strtok(addstring, ",");
	if (name == NULL) return STATUS_ERROR;
	char *address = strtok(NULL, ",");
	if (address == NULL) return STATUS_ERROR;
	char *hours = strtok(NULL, ",");
	if (hours == NULL) return STATUS_ERROR;

	dbhdr->count++;
	struct employee_t *new_employees = realloc(*employees, dbhdr->count * sizeof(struct employee_t));
	if (new_employees == NULL) return STATUS_ERROR;
	*employees = new_employees;
	struct employee_t *e = *employees;

	strncpy(e[dbhdr->count-1].name, name, sizeof(e[dbhdr->count-1].name)-1);
	e[dbhdr->count-1].name[sizeof(e[dbhdr->count-1].name)-1] = '\0';
	strncpy(e[dbhdr->count-1].address, address, sizeof(e[dbhdr->count-1].address)-1);
	e[dbhdr->count-1].address[sizeof(e[dbhdr->count-1].address)-1] = '\0';
	e[dbhdr->count-1].hours = atoi(hours);

	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
	if (fd < 1) {
		printf("Error: got a bad file descriptor from user.\n");
		return STATUS_ERROR;
	}
	int count = dbhdr->count;
	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if (employees == NULL) {
		printf("Malloc failed.\n");
		return  STATUS_ERROR;
	}

	read(fd, employees, count * sizeof(struct employee_t));

	int i = 0;
	for (; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	(void) employees;
	if (fd < 0) {
		printf("Error: got a bad file descriptor from user.\n");
		return STATUS_ERROR;
	}

	int realcount = dbhdr->count;

	dbhdr->version = htons(dbhdr->version);
	dbhdr->count = htons(dbhdr->count);
	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

	lseek(fd, 0, SEEK_SET);
	write(fd, dbhdr, sizeof(struct dbheader_t));

	for (int i = 0; i < realcount; i++) {
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

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
