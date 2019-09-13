#ifndef __CFG_H__
#define __CFG_H__
#include "scopetypes.h"

typedef struct _config_t config_t;

typedef struct {
    char* name;
    char* value;
} custom_tag_t;

// Constructors Destructors
config_t*           cfgCreateDefault();
config_t*           cfgRead(const char* path);        // reads config from yaml file
void                cfgDestroy(config_t**);

// Accessors
cfg_out_format_t    cfgOutFormat(config_t*);
const char*         cfgOutStatsDPrefix(config_t*);
unsigned            cfgOutStatsDMaxLen(config_t*);
unsigned            cfgOutPeriod(config_t*);
const char*         cfgOutCmdPath(config_t*);
unsigned            cfgOutVerbosity(config_t*);
cfg_transport_t     cfgTransportType(config_t*, which_transport_t);
const char*         cfgTransportHost(config_t*, which_transport_t);
const char*         cfgTransportPort(config_t*, which_transport_t);
const char*         cfgTransportPath(config_t*, which_transport_t);
custom_tag_t**      cfgCustomTags(config_t*);
const char*         cfgCustomTagValue(config_t*, const char*);
cfg_log_level_t     cfgLogLevel(config_t*);


// Setters (modifies config_t, but does not persist modifications)
void                cfgOutFormatSet(config_t*, cfg_out_format_t);
void                cfgOutStatsDPrefixSet(config_t*, const char*);
void                cfgOutStatsDMaxLenSet(config_t*, unsigned);
void                cfgOutPeriodSet(config_t*, unsigned);
void                cfgOutCmdPathSet(config_t*, const char*);
void                cfgOutVerbositySet(config_t*, unsigned);
void                cfgTransportTypeSet(config_t*, which_transport_t, cfg_transport_t);
void                cfgTransportHostSet(config_t*, which_transport_t, const char*);
void                cfgTransportPortSet(config_t*, which_transport_t, const char*);
void                cfgTransportPathSet(config_t*, which_transport_t, const char*);
void                cfgCustomTagAdd(config_t*, const char*, const char*);
void                cfgLogLevelSet(config_t*, cfg_log_level_t);


// Setters (string-based)
void                cfgOutFormatSetFromStr(config_t*, const char*);
void                cfgOutStatsDPrefixSetFromStr(config_t*, const char*);
void                cfgOutStatsDMaxLenSetFromStr(config_t*, const char*);
void                cfgOutPeriodSetFromStr(config_t*, const char*);
void                cfgOutCmdPathSetFromStr(config_t*, const char*);
void                cfgOutVerbositySetFromStr(config_t*, const char*);
void                cfgTransportSetFromStr(config_t*, which_transport_t, const char*);
void                cfgCustomTagAddFromStr(config_t*, const char*, const char*);
void                cfgLogLevelSetFromStr(config_t*, const char*);

#endif // __CFG_H__
