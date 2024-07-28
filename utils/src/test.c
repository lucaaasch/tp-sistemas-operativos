#include "../include/test.h"

void testSuccess(char *testName) {
	printf("El test \"%s\" funciono correctamente\n", testName);
}

void testFailedInt(char *testName, int expectedResult, int result) {
	printf("El test \"%s\" fallo:\nValor esperado: %d\nValor obtenido: %d\n", testName, expectedResult, result);
}

void testFailedString(char *testName, char *expectedResult, char *result) {
	printf("El test \"%s\" fallo:\nValor esperado: %s\nValor obtenido: %s\n", testName, expectedResult, result);
}

void testFailedChar(char *testName, char expectedResult, char result) {
	printf("El test \"%s\" fallo:\nValor esperado: %c\nValor obtenido: %c\n", testName, expectedResult, result);
}

bool assertEqualsInt(int expectedResult, int result, char *testName) {
	bool assert = expectedResult == result;

	if (assert) {
		testSuccess(testName);
	} else {
		testFailedInt(testName, expectedResult, result);
	}

	printf("\n");

	return assert;
}

bool assertEqualsUint32(uint32_t expectedResult, uint32_t result, char *testName) {
	bool assert = expectedResult == result;

	if (assert) {
		testSuccess(testName);
	} else {
		testFailedInt(testName, expectedResult, result);
	}

	printf("\n");

	return assert;
}

bool assertEqualsUint8(uint8_t expectedResult, uint8_t result, char *testName) {
	bool assert = expectedResult == result;

	if (assert) {
		testSuccess(testName);
	} else {
		testFailedInt(testName, expectedResult, result);
	}

	printf("\n");

	return assert;
}

bool assertEqualsString(char *expectedResult, char *result, char *testName) {
	bool assert = strcmp(expectedResult, result) == 0;

	if (assert) {
		testSuccess(testName);
	} else {
		testFailedString(testName, expectedResult, result);
	}

	printf("\n");

	return assert;
}

bool assertEqualsChar(char expectedResult, char result, char *testName) {
	bool assert = expectedResult == result;

	if (assert) {
		testSuccess(testName);
	} else {
		testFailedChar(testName, expectedResult, result);
	}

	printf("\n");

	return assert;
}

UnitTest* initUnitTest() {
	UnitTest *unitTest = malloc(sizeof(UnitTest));
	unitTest->testSet = NULL;
	unitTest->setLength = 0;
	return unitTest;
}

void addTest(UnitTest *unitTest, Test test) {
	unitTest->testSet = realloc(unitTest->testSet, (unitTest->setLength + 1) * sizeof(Test));
	unitTest->testSet[unitTest->setLength] = test;
	unitTest->setLength++;
}

void freeUnitTest(UnitTest *unitTest) {
	if (unitTest == NULL) {
		return;
	}
	free(unitTest->testSet);
	free(unitTest);
}

void runUnitTest(UnitTest unitTest) {
	bool huboFallos = false;
	printf("Corriendo tests...\n\n");
	for (int i = 0; i < (unitTest.setLength); ++i) {
		if (!unitTest.testSet[i]()) {
			huboFallos = true;
		}
	}
	if (huboFallos) {
		printf("Algun test fallo\n");
	} else {
		printf("Los tests corrieron con exito\n");
	}
}
