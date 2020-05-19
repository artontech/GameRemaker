#pragma once

struct Char3 {
	char chr1;
	char chr2;
	char chr3;
};

struct Int24 {
	uint32_t value : 24;
	uint32_t placeholder : 8;
};

union Parser16 {
	uint16_t uint16;
	char chr[3];
};

union Parser24 {
	Int24 uint24;
	char chr[4];
};

union Parser32 {
	uint32_t uint32;
	char chr[5];
};