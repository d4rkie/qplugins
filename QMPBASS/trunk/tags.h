#if !defined(_TAGS_H)
#define _TAGS_H

#pragma once

#include "qcdhelper.h"

bool read_tags ( reader *r, file_info *info );

void insert_tag_field ( file_info *info, const char *field, const char *value );

#endif