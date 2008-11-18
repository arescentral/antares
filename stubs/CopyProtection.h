#ifndef ANTARES_STUB_COPY_PROTECTION_H_
#define ANTARES_STUB_COPY_PROTECTION_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
    kDigitNumber = 8,
};

typedef struct {
    unsigned char* name;
    char number[kDigitNumber];
} serialNumberType;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_COPY_PROTECTION_H_
