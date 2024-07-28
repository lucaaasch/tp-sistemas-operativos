#ifndef SRC_UTILS_INCLUDE_TEST_H_
#define SRC_UTILS_INCLUDE_TEST_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <utils/include/estructuras.h>

typedef bool (*Test)(void);

typedef struct {
	Test *testSet;
	int setLength;
} UnitTest;

// Chequea la igualdad entre un valor esperado y un resultado de un test (Para int).
bool assertEqualsInt(int expectedResult, int result, char *testName);

// Chequea la igualdad entre un valor esperado y un resultado de un test. (Para uint32_t)
bool assertEqualsUint32(uint32_t expectedResult, uint32_t result, char *testName);

// Chequea la igualdad entre un valor esperado y un resultado de un test. (Para uint8_t)
bool assertEqualsUint8(uint8_t expectedResult, uint8_t result, char *testName);

// Chequea la igualdad entre un valor esperado y un resultado de un test. (Para string)
bool assertEqualsString(char *expectedResult, char *result, char *testName);

// Chequea la igualdad entre un valor esperado y un resultado de un test. (Para char)
bool assertEqualsChar(char expectedResult, char result, char *testName);

// Inicializa el UnitTest.
UnitTest* initUnitTest();

// Agrega un test al UnitTest.
void addTest(UnitTest *unitTest, Test test);

// Corre todos los tests del UnitTest.
void runUnitTest(UnitTest unitTest);

// Libera la memoria por el UnitTest.
void freeUnitTest(UnitTest *unitTest);

#endif
