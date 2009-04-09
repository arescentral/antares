#ifndef ANTARES_STUB_REGISTRATION_TOOL_H_
#define ANTARES_STUB_REGISTRATION_TOOL_H_

STUB2(RT_Open, OSErr(bool, int codes), false);
STUB1(RT_GetLicenseeName, void(unsigned char* user_name));
STUB0(RT_Close, void());
STUB1(RT_DisplayNotice, void(bool));

enum {
    VERSION_2_CODES = 6000,
};

#endif // ANTARES_STUB_REGISTRATION_TOOL_H_
