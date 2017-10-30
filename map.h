#ifndef MAP_H
#define MAP_H

#define DELAY_BYTE	0x00
#define NUM_HID_KEYS	151

typedef struct hid_value {
	char name[32];
	unsigned char mod_val;
	unsigned char res_val;
	unsigned char key_val;
} hid_value_t;

typedef struct keyboard_map {
	hid_value_t keys[NUM_HID_KEYS];
} keyboard_map_t;

short get_bytes_by_char(char key, keyboard_map_t* map);
short get_bytes_by_keynames(char* names, keyboard_map_t* map);
int load_map(char* kb_layout, keyboard_map_t* map);

#endif
