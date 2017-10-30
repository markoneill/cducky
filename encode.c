#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "encode.h"
#include "map.h"

char REM[] = "REM";
char STRING[] = "STRING";
char DELAY[] = "DELAY";
char DEFAULT_DELAY[] = "DEFAULT_DELAY";
char DEFAULTDELAY[] = "DEFAULTDELAY";
char REPEAT[] = "REPEAT";
char STRING_DELAY[] =  "STRING_DELAY";
char KEYNAMES[] = "KEYNAMES";

typedef struct instruction {
	char* command;
	union {
		char* value;
		unsigned int num_value;
	};
} instruction_t;

typedef struct encoder_ctx {
	int instruction_count;
	instruction_t* instructions;
	keyboard_map_t map;
} encoder_ctx_t;

void write_byte(FILE* file, char byte) {
	fwrite(&byte, sizeof(char), 1, file);
	return;
}

void write_short(FILE* file, short bytes) {
	fwrite(&bytes, sizeof(char), 2, file);
	return;
}

void write_delay(FILE* file, unsigned int val) {
	while (val > 0) {
		write_byte(file, DELAY_BYTE);
		if (val > 255) { 
			write_byte(file, 255);
			val -= 255;
		}
		else {
			write_byte(file, val);
			val = 0;
		}
	}
	return;
}

char* next_token(char* str) {
	while (*str != '\0' && !isspace(*str)) {
		str++;
	}
	while (*str != '\0' && isspace(*str)) {
		str++;
	}
	return str;
}

int write_instruction(FILE* file, instruction_t* instruction, instruction_t* last_inst, unsigned int* delay, keyboard_map_t* map) {
	int i;
	short bytes;
	unsigned int val;
	char* ptr;
	if (instruction->command == STRING) {
		for (i = 0; i < strlen(instruction->value); i++) {
			bytes = get_bytes_by_char(instruction->value[i], map);
			write_short(file, bytes);
		}
	}
	else if (instruction->command == DELAY) {
		val = instruction->num_value;
		write_delay(file, val);
	}
	else if (instruction->command == DEFAULT_DELAY) {
		if (last_inst != NULL) return 1; /* If used, DEFAULT_DELAY must be first */
		*delay = instruction->num_value;
		return 0; /* return now so we don't double delay */
	}
	else if (instruction->command == STRING_DELAY) {
		val = strtol(instruction->value, NULL, 10);
		ptr = next_token(instruction->value);
		//printf("instval = %s, val=%u, remaining=%s\n", instruction->value, val, ptr);
		for (i = 0; i < strlen(ptr); i++) {
			bytes = get_bytes_by_char(ptr[i], map);
			write_short(file, bytes);
			write_delay(file, val);
		}
	}
	else if (instruction->command == REPEAT) {
		if (last_inst == NULL || last_inst->command == REPEAT) return 1; /* Can't REPEAT a REPEAT */
		val = instruction->num_value;
		for (i = 0; i < val; i++) {
			if (write_instruction(file, last_inst, NULL, delay, map)) {
				return 1;
			}
		}
	}
	else if (instruction->command == KEYNAMES) {
		bytes = get_bytes_by_keynames(instruction->value, map);
		if (bytes == 0) {
			return 1;
		}
		write_short(file, bytes);
	}
	else if (instruction->command == REM) {
		return 0; /* Do nothing, comment */
	}
	if (*delay > 0) {
		write_delay(file, *delay);
	}
	return 0;
}

int write_outfile(char* outfile_path, encoder_ctx_t* ctx) {
	int i;
	unsigned int delay = 0;
	FILE* file;
	instruction_t* last_inst = NULL;
	instruction_t* cur_inst = NULL;
	file = fopen(outfile_path, "wb");
	for (i = 0; i < ctx->instruction_count; i++) {
		cur_inst = &ctx->instructions[i];
		if (write_instruction(file, cur_inst, last_inst, &delay, &ctx->map)) {
			fprintf(stderr, "Invalid instruction on line %d (%s)\n", i, cur_inst->command);
			return 1;
		}
		if (cur_inst->command != REM) last_inst = cur_inst;
	}

	fclose(file);
	return 0;
}

void free_instruction(instruction_t* inst) {
	if (inst == NULL) return;
	//if (inst->command) free(inst->command);
	if (inst->command == STRING || inst->command == KEYNAMES
	    || inst->command == STRING_DELAY || inst->command == REM) {
		if (inst->value) free(inst->value);
	}
	return;
}

void free_encoder_ctx(encoder_ctx_t* ctx) {
	int i;
	if (ctx->instructions == NULL) return;
	for (i = 0; i < ctx->instruction_count; i++) {
		free_instruction(&ctx->instructions[i]);
	}
	free(ctx->instructions);
	ctx->instructions = NULL;
	ctx->instruction_count = 0;
	return;
}

char* trim_line(char* line, size_t len) {
	char* cur_char;
	cur_char = line;

	if (len == 0 || line == NULL) return line;

	/* Move past preceding white space */
	while (isspace(*cur_char) && *cur_char != '\0') {
		cur_char++;
	}

	/* Detect blank lines */
	if (*cur_char == '\0') {
		return cur_char;
	}

	/* Remove \r from lines ending with \r\n, if present */
	if (len >= 2 && line[len-1] == '\n' && line[len-2] == '\r') {
		line[len-2] = '\0';
	}
	/* Remove \n from lines, if present */
	if (len >= 1 && line[len-1] == '\n') {
		line[len-1] = '\0';
	}

	return cur_char;
}

int parse_instruction(char* line, instruction_t* instruction) {
	int val_len;
	int val;
	if (strncmp(line, STRING_DELAY, sizeof(STRING_DELAY)-1) == 0) {
		line = strchr(line, ' ');
		if (line == NULL) return 1; /* No string specified */
		line++;
		instruction->command = STRING_DELAY;
		val_len = strlen(line);
		instruction->value = malloc(val_len+1);
		strcpy(instruction->value, line);
	}
	else if (strncmp(line, STRING, sizeof(STRING)-1) == 0) {
		line = strchr(line, ' ');
		if (line == NULL) return 1; /* No string specified */
		line++;
		instruction->command = STRING;
		val_len = strlen(line);
		instruction->value = malloc(val_len+1);
		strcpy(instruction->value, line);
	}
	else if (strncmp(line, DELAY, sizeof(DELAY)-1) == 0) {
		line = next_token(line);
		instruction->command = DELAY;
		sscanf(line, "%u", &instruction->num_value);
	}
	else if (strncmp(line, DEFAULT_DELAY, sizeof(DEFAULT_DELAY)-1) == 0 ||
		 strncmp(line, DEFAULTDELAY, sizeof(DEFAULTDELAY)-1) == 0) {
		line = next_token(line);
		instruction->command = DEFAULT_DELAY;
		sscanf(line, "%u", &instruction->num_value);
	}
	else if (strncmp(line, REPEAT, sizeof(REPEAT)-1) == 0) {
		line = next_token(line);
		instruction->command = REPEAT;
		sscanf(line, "%u", &instruction->num_value);
	}
	else if (strncmp(line, REM, sizeof(REM)-1) == 0) {
		line = next_token(line);
		instruction->command = REM;
		val_len = strlen(line);
		instruction->value = malloc(val_len+1);
		strcpy(instruction->value, line);
	}
	else {
		instruction->command = KEYNAMES;
		val_len = strlen(line);
		instruction->value = malloc(val_len+1);
		strcpy(instruction->value, line);
		//printf("%s\n", instruction->value);
	}
	return 0;
}

int parse_infile(char* infile_path, encoder_ctx_t* ctx) {
	FILE* file;
	char* line = NULL;
	char* trimmed_line = NULL;
	size_t len = 0;
	ssize_t nread;
	int line_count = 0;

	file = fopen(infile_path, "r");
	if (file == NULL) {
		perror("fopen");
		return 1;
	}

	/* Count the number of lines in the file and rewind */
	while (getline(&line, &len, file) != -1) {
		line_count++;
	}
	fseek(file, 0, SEEK_SET);

	ctx->instruction_count = 0;
	ctx->instructions = malloc(sizeof(instruction_t) * line_count);
	if (ctx->instructions == NULL) {
		perror("malloc");
		return 1;
	}

	/* Read instructions */
	while ((nread = getline(&line, &len, file)) != -1) {

		/* Clean up whitespace issues */
		trimmed_line = trim_line(line, nread);

		/* Blank line, continue */
		if (trimmed_line[0] == '\0') {
			continue;
		}

		/* Comment, ignore line */
		/*if (strncmp(trimmed_line, REM, sizeof(REM)-1) == 0) {
			continue;
		}*/

		//printf("%s\n", trimmed_line);

		parse_instruction(trimmed_line, &ctx->instructions[ctx->instruction_count]);
		ctx->instruction_count++;
	}

	free(line);
	fclose(file);
	return 0;
}

void encode(char* infile_path, char* outfile_path, char* keyboard_layout) {
	encoder_ctx_t ctx;
	printf("Loading mappings...\n");
	if (load_map(keyboard_layout, &ctx.map)) {
		exit(EXIT_FAILURE);
	}

	printf("Parsing script...\n");
	if (parse_infile(infile_path, &ctx)) {
		exit(EXIT_FAILURE);
	}

	printf("Writing binary...\n");
	if (write_outfile(outfile_path, &ctx)) {
		exit(EXIT_FAILURE);
	}
	free_encoder_ctx(&ctx);
	printf("Success!\n");
	return;
}
