
#ifndef USTRING_H
#define USTRING_H

#include "platform.h"

//#define USTR_DEBUG

class UStr
{
public:
	unsigned int m_length;	//length doesn't count null-terminator
	unsigned int* m_data;

	UStr();
	~UStr();
	UStr(const UStr& original);
	UStr(const char* cstr);
	UStr(unsigned int k);
	UStr(unsigned int* k);
	UStr& operator=(const UStr &original);
	UStr operator+(const UStr &other);
	UStr substr(int start, int len) const;
	int firstof(UStr find) const;
	std::string rawstr() const;
};

#endif
