#include <cPQuery.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
#ifdef DEBUG
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
#endif

  PQuery pqueryMaster = PQuery();

  try {
    if (!pqueryMaster.parseCliOptions(argc, argv)) {
      return EXIT_FAILURE;
    }

    return pqueryMaster.run();
  } catch (const std::exception &e) {
    std::cerr << "=> Fatal error: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "=> Fatal error: unknown exception" << '\n';
    return EXIT_FAILURE;
  }
}
