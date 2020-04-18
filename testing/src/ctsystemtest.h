#ifndef CTSYSTEMTEST_H
#define CTSYSTEMTEST_H

#include <QtTest>

namespace CTL {
    class CTSystem;
}

class CTSystemTest : public QObject
{
    Q_OBJECT

public:
    CTSystemTest() = default;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void componentCount();
    void validSystem();
    void simpleSystem();
    void renameCheck();
    void testSystemBuilder();

private:
    CTL::CTSystem* _theTestSystem;
};

#endif // CTSYSTEMTEST_H
