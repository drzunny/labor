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

        void run();

    private:
        std::shared_ptr<_event_impl> event_;
    };

}

#endif
