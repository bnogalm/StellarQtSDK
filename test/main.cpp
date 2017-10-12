#include "testcollector.h"
#include <iostream>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc,argv);

    auto nFailedTests = TestCollector::RunAllTests(argc, argv);
    std::cout<<"Total number of failed tests: "<<nFailedTests<<std::endl;

    return nFailedTests;
}
