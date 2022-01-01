/*
 * file2hex4c
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct HexDumpParam {
	int is_raw;
	int is_writable;
	int is_has_size;
	const char *file;
	const char *vname;
} HexDumpParam;

#define BUFFER_SIZE (0x4000)

uint8_t working_buffer[BUFFER_SIZE];

const char *find_item(int argc, char *argv[], const char *name){

	for(int i=0;i<argc;i++){
		if(strstr(argv[i], name) != NULL){
			return (char *)(strstr(argv[i], name) + strlen(name));
		}
	}

	return NULL;
}

int parse_args(int argc, char *argv[], HexDumpParam *param){

	const char *content;
	HexDumpParam loc_param;

	memset(&loc_param, 0, sizeof(loc_param));

	content = find_item(argc, argv, "-file=");
	if(content == NULL){
		return 1;
	}

	loc_param.file = content;

	int is_raw = 0;

	content = find_item(argc, argv, "-raw=");

	loc_param.is_raw = (content == NULL || strncmp(content, "false", 5) != 0);

	content = find_item(argc, argv, "-vname=");
	if(loc_param.is_raw == 0 && content == NULL){
		return 1;
	}

	loc_param.vname = content;

	content = find_item(argc, argv, "-writable=");

	loc_param.is_writable = (content != NULL && strncmp(content, "true", 5) == 0);

	content = find_item(argc, argv, "-has_size=");
	if(content != NULL && strncmp(content, "macro", 5) == 0){
		loc_param.is_has_size = 2;
	}else if(content != NULL && strncmp(content, "true", 5) == 0){
		loc_param.is_has_size = 1;
	}else{
		loc_param.is_has_size = 0;
	}

	if(loc_param.is_has_size != 0 && loc_param.is_raw != 0){
		return 1;
	}

	memcpy(param, &loc_param, sizeof(*param));

	return 0;
}

/*
 * file     : The target file
 * raw      : The dump as RAW. (0x00, 0x01, 0x02 ...)
 * vname    : The output variable name
 * writable : Output is const or not
 * has_size : Include size variable
 */
int main(int argc, char *argv[]){

	if(argc <= 1 || argv == NULL){
		return 1;
	}

	int res;
	FILE *fd;
	HexDumpParam dump_param;

	res = parse_args(argc, argv, &dump_param);
	if(res != 0){
		return 1;
	}

	fd = fopen(dump_param.file, "rb");
	if(fd == NULL){
		return 1;
	}

	fseek(fd, 0, SEEK_END);

	long length = ftell(fd);
	if(length == 0){
		goto error;
	}

	long org_length = length;

	fseek(fd, 0, SEEK_SET);

	length -= 1;

	if(dump_param.is_raw == 0){
		if(dump_param.is_writable == 0){
			printf("const ");
		}

		printf("char %s[0x%lX] = {\n", dump_param.vname, org_length);
	}

	while(length >= BUFFER_SIZE){

		if(fread(working_buffer, BUFFER_SIZE, 1, fd) != 1){
			goto error;
		}

		if(dump_param.is_raw == 0 && org_length == (length + 1))
			printf("\t");

		for(int i=0;i<BUFFER_SIZE;i++){

			printf("0x%02X", working_buffer[i]);

			if((i & 0xF) == 0xF){
				printf(",\n");
				if(dump_param.is_raw == 0)
					printf("\t");
			}else{
				printf(", ");
			}
		}

		length -= BUFFER_SIZE;
	}

	if(length != 0){
		if(fread(working_buffer, length, 1, fd) != 1){
			goto error;
		}

		if(dump_param.is_raw == 0 && org_length == (length + 1))
			printf("\t");

		for(int i=0;i<length;i++){

			printf("0x%02X", working_buffer[i]);

			if((i & 0xF) == 0xF){
				printf(",\n");
				if(dump_param.is_raw == 0)
					printf("\t");
			}else{
				printf(", ");
			}
		}
	}

	if(fread(working_buffer, 1, 1, fd) != 1){
		goto error;
	}

	printf("0x%02X\n", working_buffer[0]);

	if(dump_param.is_raw == 0){
		printf("};\n");
	}

	if(dump_param.is_has_size == 1){
		printf("\n");
		if(dump_param.is_writable == 0){
			printf("const ");
		}

		printf("unsigned int %s_len = 0x%lX;\n\n", dump_param.vname, org_length);
	}else if(dump_param.is_has_size == 2){
		printf("\n#define %s_len (0x%lX)\n\n", dump_param.vname, org_length);
	}

	fclose(fd);

	return 0;

error:
	fclose(fd);

	return 1;
}
