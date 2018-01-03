#pragma once

#include <ctype.h>

static inline char indextoLetter(char index)
{
	return index + 'A';
}

static inline unsigned short lettertoIndex(char letter)
{
	return toupper(letter) - 'A';
}