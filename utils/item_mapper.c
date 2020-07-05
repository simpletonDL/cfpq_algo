#include <assert.h>
#include <stdio.h>

#include "item_mapper.h"
#include "string.h"
#include "arr.h"

MapperIndex ItemMapper_GetPlaceIndex(ItemMapper *dict, const char *token) {
    for (MapperIndex i = 0; i < array_len(dict->arr); i++) {
        if (strcmp(dict->arr[i], token) ==  0) {
//			printf("%s == %s -> %d\n", dict->arr[i], token, i);
			return i;
        } else {
//			printf("%s != %s\n", dict->arr[i], token);
		}
    }
    return array_len(dict->arr);
}

void ItemMapper_Init(ItemMapper *dict) {
    dict->arr = array_new(char*, 10);
}

MapperIndex ItemMapper_Insert(ItemMapper *dict, const char *token) {
	MapperIndex i = ItemMapper_GetPlaceIndex(dict, token);

	if (i < array_len(dict->arr)) {
        return i;
    } else {
		char* new_token = malloc(sizeof(char) * strlen(token));
		strcpy(new_token, token);
		dict->arr = array_append(dict->arr, new_token);
        return array_len(dict->arr)-1;
    }
}

int ItemMapper_Find(ItemMapper *dict, const char *token) {
    return ItemMapper_GetPlaceIndex(dict, token) == array_len(dict->arr) ? ITEM_NOT_EXIST : ITEM_EXIST;
}

const char * ItemMapper_Map(ItemMapper *dict, MapperIndex mapperIdex) {
    assert(mapperIdex < array_len(dict->arr));
    return dict->arr[mapperIdex];
}
