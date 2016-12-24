#include "../platform.h"
#include "../ustring.h"

int32_t ValidateUTF8(char unsigned *buff, int32_t len);
void dump_unicode(char unsigned *buff, int32_t len);
//uint32_t *ToUTF32(const unsigned char *utf8, int32_t len);
uint32_t *ToUTF32(const unsigned char *utf8);
unsigned char *ToUTF8(const uint32_t *unicode);
void print_char(int32_t pos, int32_t len, uint32_t ch);
void dump_unicode_string(uint32_t *str);
int32_t UTF8Len(unsigned char ch);
int32_t UTF32Len(const uint32_t* ustr);
unsigned char *raw2utf8(unsigned ch);
unsigned char *ToUTF8_2(const uint32_t *unicode);
