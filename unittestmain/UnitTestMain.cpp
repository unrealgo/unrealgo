
#include "platform/SgSystem.h"

#include "GoInit.h"
#include "SgInit.h"

namespace {

void Init()
{
    SgInit();
    GoInit();
}

void Fini()
{
  GoFinish();
    SgFini();
}

} // namespace

#include <cstdlib>
#define BOOST_TEST_DYN_LINK

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
    #include <boost/test/unit_test.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif


bool init_unit_test()
{
    try
    {
        Init();
    }
    catch (const std::exception& e)
    {
        return false;
    }
    if (std::atexit(Fini) != 0)
        return false;
    return true;
}

int main(int argc, char* argv[])
{
    return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}
