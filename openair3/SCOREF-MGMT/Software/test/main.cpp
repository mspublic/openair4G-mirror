/*
 * main.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: demiray
 */

#include <boost/lexical_cast.hpp>
#include "gtest/gtest.h"

#include "test_util.hpp"

namespace ScorefTest {
	/*
	 * The fixture for SCOREF-MGMT testing
	 */
	class ScorefManagementTest: public ::testing::Test {
		protected:
			ScorefManagementTest() {
				// You can do set-up work for each test here.
			}

			virtual ~ScorefManagementTest() {
				// You can do clean-up work that doesn't throw exceptions here.
			}

			// If the constructor and destructor are not enough for setting up
			// and cleaning up each test, you can define the following methods:

			/**
			 * Initialization that'll be done before each test
			 *
			 * @param none
			 * @return none
			 */
			virtual void SetUp() {
				logger = new Logger("test.log", Logger::TRACE);
			}

			/**
			 * Finalization that'll be done after each test
			 *
			 * @param none
			 * @return none
			 */
			virtual void TearDown() {
				delete logger;
			}

			// Objects declared here can be used by all tests in the test case for Foo.
			Logger* logger;
	};

	/**
	 * Util class tests
	 */
	TEST_F(ScorefManagementTest, UtilTest) {
		/**
		 * Test Util class' methods
		 */
		testUtilResetBuffer(*logger);
		testUtilCopyBuffer(*logger);
		testUtilGetBinaryRepresentation(*logger);
		testUtilSetBit(*logger);
		testUtilUnsetBit(*logger);
		testUtilIsBitSet(*logger);
		testUtilParse8byteInteger(*logger);
		testUtilParse4byteInteger(*logger);
		testUtilParse2byteInteger(*logger);
		testUtilEncode8ByteInteger(*logger);
		testUtilEncode4ByteInteger(*logger);
		testUtilEncode2ByteInteger(*logger);
		testUtilEncodeBits(*logger);
		testUtilSplit(*logger);
		testUtilTrim(*logger);
		testUtilIsNumeric(*logger);
	}
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
