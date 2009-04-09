#ifndef ANTARES_STUB_COPY_PROTECTION_H_
#define ANTARES_STUB_COPY_PROTECTION_H_

enum {
    kDigitNumber = 8,
};

typedef struct {
    unsigned char* name;
    char number[kDigitNumber];
} serialNumberType;

#endif // ANTARES_STUB_COPY_PROTECTION_H_
