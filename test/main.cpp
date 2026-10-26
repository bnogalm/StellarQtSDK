#include "testcollector.h"
#include <iostream>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc,argv);

    auto nFailedTests = TestCollector::RunAllTests(argc, argv);
    std::cout<<"Total number of failed tests: "<<nFailedTests<<std::endl;

    // The count is printed above; the exit status only says pass or fail. As
    // a count it wrapped modulo 256, so 256 failures exited 0.
    return nFailedTests == 0 ? 0 : 1;
}
