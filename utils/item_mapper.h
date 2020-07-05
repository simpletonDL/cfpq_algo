#pragma once

#include <stdint.h>
#include "../grammar/conf.h"

#define ITEM_NOT_EXIST 0
#define ITEM_EXIST 1

typedef struct {
	char **arr;
} ItemMapper;

void ItemMapper_Init(ItemMapper *dict);

MapperIndex ItemMapper_GetPlaceIndex(ItemMapper *dict, const char *token);
MapperIndex ItemMapper_Insert(ItemMapper *dict, const char *token);
int ItemMapper_Find(ItemMapper *dict, const char *token);
const char * ItemMapper_Map(ItemMapper *dict, MapperIndex mapperIdex);
