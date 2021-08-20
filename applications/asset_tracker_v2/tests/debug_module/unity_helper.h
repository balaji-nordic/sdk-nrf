/*
 * Note: Tail end of macro name must match your type's capitalization!
 */
#include "memfault/core/event_storage_implementation.h"
#include "memfault/core/event_storage.h"

void AssertEqualsMemfaultEventStorageImpl(sMemfaultEventStorageImpl  expected,
                           sMemfaultEventStorageImpl  actual,
                           unsigned short line);


#define UNITY_TEST_ASSERT_EQUAL_sMemfaultEventStorageImpl(expected, actual, line, message)  AssertEqualsMemfaultEventStorageImpl(expected, actual, line);

#define TEST_ASSERT_EQUAL_EXAMPLE_sMemfaultEventStorageImp(expected, actual) UNITY_TEST_ASSERT_EQUAL_sMemfaultEventStorageImpl(expected, actual, __LINE__, NULL);
