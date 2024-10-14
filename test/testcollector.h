/* BASED ON
 * http://qtcreator.blogspot.de/2009/10/running-multiple-unit-tests.html
 */
#ifndef TESTCOLLECTOR_H
#define TESTCOLLECTOR_H

#include <QtTest>
#include <memory>
#include <map>
#include <string>
#include <iostream>

namespace TestCollector{
typedef std::map<std::string, std::shared_ptr<QObject> > TestList;
inline TestList& GetTestList()
{
   static TestList list;
   return list;
}

inline int RunAllTests(int argc, char **argv) {
    int result = 0;
    QList<std::string> failedTests;
    QMap<std::string, std::string> failedTestsWithExceptions;
    for (const auto&i:GetTestList()) {
        try{
        int res = QTest::qExec(i.second.get(), argc, argv);
        result += res;
        printf("\n");
        if(res)
            failedTests.append(i.first);
        }
        catch(std::exception& e)
        {
            failedTestsWithExceptions.insert(i.first, e.what());
            failedTests.append(i.first);
        }
    }
    if(!failedTests.isEmpty())
        std::cout<<"Failed:\n";
    for(std::string s:failedTests)
    {
        if(!failedTestsWithExceptions.contains(s))
            std::cout<<s<<std::endl;
        else
            std::cout<<s<< " throwed exception: " << failedTestsWithExceptions.value(s) << std::endl;
    }
    return result;
}

template <class T>
class UnitTestClass {
public:
    UnitTestClass(const std::string& pTestName) {
        auto& testList = TestCollector::GetTestList();
        if (0==testList.count(pTestName)) {
            testList.insert(std::make_pair(pTestName, std::make_shared<T>()));
        }
    }
};
}

//helper to check where is the problem...
inline void compareBase64(QString a,QString b)
{
    QByteArray ab= QByteArray::fromBase64(a.toLatin1());
    QByteArray bb= QByteArray::fromBase64(b.toLatin1());
    if(ab.length()!=bb.length()){
        qDebug() << "Wrong length "<<ab.length() <<" != " << bb.length();
    }
    qDebug() << ab.toHex();
    qDebug() << bb.toHex();

    for(int i=0;i<qMin(ab.length(),bb.length());i++)
    {
        if(ab[i]!=bb[i]){
            qDebug() << "Error at byte "<< i;
            qDebug() << ab.right(ab.length()-i).toHex();
            qDebug() << bb.right(bb.length()-i).toHex();
            break;
        }
    }
}
#define WAIT_FOR(EXP)\
	{\
    int wait=1;\
    while(EXP){\
       QTest::qWait(10);\
       qApp->processEvents();\
       wait++;\
       if(wait>300)\
           break;\
    }\
	}



#define ADD_TEST(className) static TestCollector::UnitTestClass<className> \
    test(#className);

#endif // TESTCOLLECTOR_H
