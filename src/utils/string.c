#include "string.h"
#include <stdlib.h>
#include <string.h>

int32_t str_first_instance(const char* str, uint32_t size, char search){
    for(uint32_t i = 0; i < size; i++){
        if(str[i] == search)
            return i;
    }
    return -1;
}

uint8_t char_to_uint8(const char c){
    char buffer[] = {c, '\0'};
    return atoi(buffer);
}
