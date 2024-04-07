#include <stddef.h>
#include <stdlib.h>

char* build_disconnect(size_t* size) {
	char fixed_header_type = 0b11100000;
	char remaining_length = 0b00000000;

	char* built_disconnect = (char*)malloc(3 * sizeof(char));
	*size = 3;

	built_disconnect[0] = fixed_header_type;
	built_disconnect[1] = remaining_length;

	return built_disconnect;
}