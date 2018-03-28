#ifndef ANDROID_ADAPTER_H
#define ANDROID_ADAPTER_H

#include <string.h>
#include <syslog.h>

#define PROPERTY_VALUE_MAX 128

#define property_get(name, value_str, default_value) \
do { \
    strncpy(value_str, default_value, PROPERTY_VALUE_MAX); \
} while (0)

#define ALOGE(fmt, args...) syslog(LOG_ERR, fmt, ##args)
#define ALOGV(fmt, args...) syslog(LOG_INFO, fmt, ##args)
#define ALOGI(fmt, args...) syslog(LOG_INFO, fmt, ##args)
#define ALOGD(fmt, args...) syslog(LOG_DEBUG, fmt, ##args)

#endif

