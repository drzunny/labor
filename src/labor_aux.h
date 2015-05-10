#ifndef __LABOR_VM_AUX_H__
#define __LABOR_VM_AUX_H__

#include <string>

namespace labor
{
    enum EServiceType
    {
        SV_PUBLISH,
        SV_PUSH,
        SV_PULL
    };

    // Json library
    std::string ext_json_encode(void * jsonptr);
    void ext_json_decode(const char *str, void * jsonptr);

    // Push service
    void * ext_service_init_push(const char * addr);
    void ext_service_push(void * hnd, const char * message);
    void ext_service_publish(const char * message);

    // logger library
    void ext_logger_debug(const char * message);
    void ext_logger_info(const char * message);
    void ext_logger_warning(const char * message);
    void ext_logger_error(const char* message);

    /* HTTP client library is comming soon ... */
}

#endif