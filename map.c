#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "map.h"

int load_map(char* kb_layout, keyboard_map_t* map) {
	FILE* file;
	char* line = NULL;
	size_t len = 0;
	ssize_t nread;
	int line_count = 0;
	int i = 0;
	char* token = NULL;

	file = fopen(kb_layout, "r");
	if (file == NULL) {
		perror("fopen for layout");
		return 1;
	}

	/* Count the number of lines in the file and rewind */
	while (getline(&line, &len, file) != -1) {
		line_count++;
	}
	if (line_count != NUM_HID_KEYS) {
		fprintf(stderr, "Incorrect number of entries for keyboard layout file\n");
		return 1;
	}
	fseek(file, 0, SEEK_SET);

	while ((nread = getline(&line, &len, file)) != -1) {
		token = strchr(line, ' ');
		if (token == NULL) {
			fprintf(stderr, "No keyname found\n");
			return 1;
		}
		if (line[0] == ' ') token++; /* handle space case */
		*token = '\0'; token++;
		strcpy(map->keys[i].name, line);

		map->keys[i].mod_val = strtol(token, &token, 16); token++;
		map->keys[i].res_val = strtol(token, &token, 16); token++;
		map->keys[i].key_val = strtol(token, &token, 16); token++;
		/*printf("%s %02x %02x %02x\n", 
			map->keys[i].name,
			map->keys[i].mod_val & 0xFF,
			map->keys[i].res_val & 0xFF,
			map->keys[i].key_val & 0xFF);*/
		i++;
	}
	free(line);
	fclose(file);
	return 0;
}

short get_bytes_by_char(char key, keyboard_map_t* map) {
	int i;
	for (i = 0; i < NUM_HID_KEYS; i++) {
		/* We're only interested in single chars */
		if (strlen(map->keys[i].name) > 1) continue;
		if (key == map->keys[i].name[0]) {
			return map->keys[i].mod_val << 8 | map->keys[i].key_val;
		}
	}
	return 0;
}

char* str_space(char* str) {
	while (*str != '\0' && !isspace(*str)) {
		str++;
	}
	return str;
}

char* str_notspace(char* str) {
	while (*str != '\0' && isspace(*str)) {
		str++;
	}
	return str;
}

short get_bytes_by_keynames(char* names, keyboard_map_t* map) {
	int i;
	short bytes = 0;
	int val_len = strlen(names);
	char* first_name_end = str_space(names);
	char* second_name = NULL;

	*first_name_end = '\0';
	if ((first_name_end - names) < val_len) {
		second_name = str_notspace(first_name_end+1);
	}
	//printf("first name is %s\n", names);
	//printf("second name is %s\n", second_name);

	for (i = 0; i < NUM_HID_KEYS; i++) {
		if (strcmp(names, map->keys[i].name) == 0) {
			if (map->keys[i].key_val == 0 && second_name == NULL) {
				bytes = map->keys[i].mod_val;
			}
			else {
				bytes = map->keys[i].mod_val << 8 | map->keys[i].key_val;
			}
		}
	}
	if (second_name != NULL) {
		for (i = 0; i < NUM_HID_KEYS; i++) {
			if (strcmp(second_name, map->keys[i].name) == 0) {
				bytes |= map->keys[i].key_val;
			}
		}
	}
	return bytes;
}

