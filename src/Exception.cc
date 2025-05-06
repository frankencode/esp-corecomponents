#include <cc/Exception>

namespace cc {

const char* Exception::what() const throw()
{
    static thread_local String m = message();
    return m;
}

} // namespace cc
