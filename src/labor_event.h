#ifndef __LABOR_EVENT_H__
#define __LABOR_EVENT_H__

#include <memory>
#include <string>

namespace labor
{
    class _event_impl;

    class Event
    {
    public:
        Event();
        ~Event();

        void setEventHandler(const std::string & name, const std::string & resolve);
        void emit(const std::string & name);

        void run();

    private:
        std::unique_ptr<_event_impl> event_;
    };

}

#endif