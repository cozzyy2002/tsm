#pragma once

#include <mutex>

namespace tsm
{
using lock_object_t = std::recursive_mutex;

class lock_t : public std::lock_guard<lock_object_t>
{
public:
	lock_t(lock_object_t& obj) : std::lock_guard<lock_object_t>(obj) {}
};
}
