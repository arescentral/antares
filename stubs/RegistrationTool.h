#ifndef ANTARES_STUB_REGISTRATION_TOOL_H_
#define ANTARES_STUB_REGISTRATION_TOOL_H_

OSErr RT_Open(bool, int codes);
void RT_GetLicenseeName(unsigned char* user_name);
void RT_Close();
void RT_DisplayNotice(bool);

enum {
    VERSION_2_CODES = 6000,
};

#endif // ANTARES_STUB_REGISTRATION_TOOL_H_
