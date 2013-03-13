# include "instance.hpp"
# include <cppunit/extensions/HelperMacros.h>
# include <vector>

using namespace std;
using namespace mlr;

class UnitInstanceTest : public CPPUNIT_NS::TestFixture
{
   // Test Start
   CPPUNIT_TEST_SUITE( UnitInstanceTest );
   CPPUNIT_TEST( testlabelcheck );
   CPPUNIT_TEST( testqidcheck );
   CPPUNIT_TEST( testdocidcheck );
   CPPUNIT_TEST( testvectorcheck );
   CPPUNIT_TEST( testpredict );
   // Test End
   CPPUNIT_TEST_SUITE_END();

   protected:
      instance* testInstance;

   public:
      void setUp();
      void tearDown();

   protected:
      void testlabelcheck();
      void testqidcheck();
      void testdocidcheck();
      void testvectorcheck();
      void testpredict();
};

CPPUNIT_TEST_SUITE_REGISTRATION( UnitInstanceTest );

/**
 * OHSUMED/QueryLevelNorm
 */
void UnitInstanceTest::setUp()
{
   try{
      docline str("0 qid:1 1:1.000000 2:1.000000 3:0.833333 4:0.871264 5:0 6:0 7:0 8:0.941842 9:1.000000 10:1.000000 11:1.000000 12:1.000000 13:1.000000 14:1.000000 15:1.000000 16:1.000000 17:1.000000 18:0.719697 19:0.729351 20:0 21:0 22:0 23:0.811565 24:1.000000 25:0.972730 26:1.000000 27:1.000000 28:0.922374 29:0.946654 30:0.938888 31:1.000000 32:1.000000 33:0.711276 34:0.722202 35:0 36:0 37:0 38:0.798002 39:1.000000 40:1.000000 41:1.000000 42:1.000000 43:0.959134 44:0.963919 45:0.971425 #docid = 244338");

      this->testInstance = new instance;
      this->testInstance->set(str);
   }
   catch (const char *ex)
   {
      cerr << ex << endl;
      throw ex;
   }
   catch (...)
   {
   }
}

void UnitInstanceTest::tearDown()
{
   delete this->testInstance;
}

void UnitInstanceTest::testlabelcheck()
{
   double label = 0;
   CPPUNIT_ASSERT_EQUAL(label, this->testInstance->getlabel());
}

void UnitInstanceTest::testqidcheck()
{
   long qid = 1;
   CPPUNIT_ASSERT_EQUAL(qid, this->testInstance->getqid());
}

void UnitInstanceTest::testdocidcheck()
{
   long docid = 244338;
   CPPUNIT_ASSERT_EQUAL(docid,this->testInstance->getdocid());
}

void UnitInstanceTest::testvectorcheck()
{
   double secondfeatures = 1.0;
   const _fvector& fv = this->testInstance->getvect();
   CPPUNIT_ASSERT_EQUAL(secondfeatures,fv[2].val);
}

void UnitInstanceTest::testpredict()
{
   double predict = 1.0;
   this->testInstance->setpredict(predict);
   CPPUNIT_ASSERT_EQUAL(predict, this->testInstance->getpredict());
}
