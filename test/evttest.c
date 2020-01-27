#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "dbg.h"
#include "evt.h"

#include "test.h"

static void
evtCreateReturnsValidPtr(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtDestroy(&evt);
    assert_null(evt);
}

static void
evtDestroyNullOutDoesntCrash(void** state)
{
    evtDestroy(NULL);
    evt_t* evt = NULL;
    evtDestroy(&evt);
    // Implicitly shows that calling evtDestroy with NULL is harmless
}

static void
evtMetricHappyPath(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);

    event_t e = INT_EVENT("A", 1, DELTA, NULL);
    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-4",
                      .id = "host-evttest-cmd-4"};

    // when enabled, we should get a non-null json
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 1);
    cJSON* json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);

    // grab the time value from json to use in our expected output.
    // the specific value isn't of interest.
    cJSON* time = cJSON_GetObjectItem(json, "_time");

    char* expected = NULL;
    asprintf(&expected, "{\"sourcetype\":\"metric\","
                           "\"id\":\"host-evttest-cmd-4\","
                           "\"_time\":%ld,"
                           "\"source\":\"A\",\"data\":"
                           "{\"_metric\":\"A\","
                              "\"_metric_type\":\"counter\","
                              "\"_value\":1"
                           "},\"host\":\"host\","
                           "\"_channel\":\"12345\"}", (long)time->valuedouble);
    assert_non_null(expected);
    char* actual = cJSON_PrintUnformatted(json);
    assert_non_null(actual);
    assert_string_equal(expected, actual);
    cJSON_Delete(json);
    free(actual);
    free(expected);

    evtDestroy(&evt);
}

static void
evtMetricWithSourceDisabledReturnsNull(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);

    event_t e = INT_EVENT("A", 1, DELTA, NULL);
    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-4",
                      .id = "host-evttest-cmd-4"};

    // default is disabled
    cJSON* json = evtMetric(evt, &e, 12345, &proc);
    assert_null(json);

    // when enabled, we should get a non-null json
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 1);
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Set it back to disabled, just to be sure.
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 0);
    json = evtMetric(evt, &e, 12345, &proc);
    assert_null(json);
    cJSON_Delete(json);

    evtDestroy(&evt);
}

static void
evtMetricWithAndWithoutMatchingNameFilter(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 1);

    event_t e = INT_EVENT("A", 1, DELTA, NULL);
    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-4",
                      .id = "host-evttest-cmd-4"};
    cJSON* json;

    // Default name filter allows everything
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the name filter to "^B" shouldn't match.
    evtNameFilterSet(evt, CFG_SRC_METRIC, "^B");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_null(json);

    // Changing the name filter to "^A" should match.
    evtNameFilterSet(evt, CFG_SRC_METRIC, "^A");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    evtDestroy(&evt);
}

static void
evtMetricWithAndWithoutMatchingFieldFilter(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 1);

    event_field_t fields[] = {
        STRFIELD("proc",             "ps",                  3),
        NUMFIELD("pid",              2,                     3),
        FIELDEND
    };
    event_t e = INT_EVENT("A", 1, DELTA, fields);
    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-4",
                      .id = "host-evttest-cmd-4"};
    cJSON* json, *data;

    // Default field filter allows both fields
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    data = cJSON_GetObjectItem(json, "data");
    assert_non_null(data);
    assert_non_null(cJSON_GetObjectItem(data, "proc"));
    assert_non_null(cJSON_GetObjectItem(data, "pid"));
    cJSON_Delete(json);

    // Changing the field filter to ".*oc" should match proc but not pid
    evtFieldFilterSet(evt, CFG_SRC_METRIC, ".*oc");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    data = cJSON_GetObjectItem(json, "data");
    assert_non_null(data);
    assert_non_null(cJSON_GetObjectItem(data, "proc"));
    assert_null(cJSON_GetObjectItem(data, "pid"));
    cJSON_Delete(json);

    evtDestroy(&evt);
}

static void
evtMetricWithAndWithoutMatchingValueFilter(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 1);

    event_t e = INT_EVENT("A", 1, DELTA, NULL);
    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-4",
                      .id = "host-evttest-cmd-4"};
    cJSON* json;

    // Default value filter allows everything
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the value filter to "^2" shouldn't match.
    evtValueFilterSet(evt, CFG_SRC_METRIC, "^2");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_null(json);

    // Adding a field with value 2 should match.
    event_field_t fields[] = {
        STRFIELD("proc",             "ps",                  3),
        NUMFIELD("pid",              2,                     3),
        FIELDEND
    };
    e.fields = fields;
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the value filter to "^1" should match.
    evtValueFilterSet(evt, CFG_SRC_METRIC, "^1");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the value filter to "ps" should match too.
    evtValueFilterSet(evt, CFG_SRC_METRIC, "ps");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the value filter to "blah" should not match.
    evtValueFilterSet(evt, CFG_SRC_METRIC, "blah");
    json = evtMetric(evt, &e, 12345, &proc);
    assert_null(json);

    evtDestroy(&evt);
}

static void
evtMetricRateLimitReturnsNotice(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtSourceEnabledSet(evt, CFG_SRC_METRIC, 1);

    event_t e = INT_EVENT("Hey", 1, DELTA, NULL);
    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-4",
                      .id = "host-evttest-cmd-4"};
    cJSON* json, *data;

    time_t initial, current;
    time(&initial);

    int i;
    for (i=0; i<=MAXEVENTSPERSEC; i++) {
        json = evtMetric(evt, &e, 12345, &proc);
        assert_non_null(json);

        time(&current);
        if (initial != current) {
            // This test depends on running all iterations in the same second.
            // If we find this isn't true, start the loop over.
            initial = current;
            i=0;
            cJSON_Delete(json);
            continue;
        }

        //printf("i=%d %s\n", i, msg);
        data = cJSON_GetObjectItem(json, "data");
        assert_non_null(data);

        if (i<MAXEVENTSPERSEC) {
            // Verify that data contains _metric, and not "Truncated"
            assert_true(cJSON_HasObjectItem(data, "_metric"));
            assert_false(cJSON_IsString(data));
        } else {
            // Verify that data contains "Truncated", and not _metric
            assert_true(cJSON_IsString(data));
            assert_non_null(strstr(data->valuestring, "Truncated"));
            assert_false(cJSON_HasObjectItem(data, "_metric"));
        }
        cJSON_Delete(json);
    }

    evtDestroy(&evt);
}

static void
evtLogWithSourceDisabledReturnsNull(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);

    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-log",
                      .id = "host-evttest-cmd-4"};

    // default is disabled
    cJSON* json = evtLog(evt, "stdout", "hey", 4, 12345, &proc);
    assert_null(json);

    // when enabled, we should get a non-null msg
    evtSourceEnabledSet(evt, CFG_SRC_CONSOLE, 1);
    json = evtLog(evt, "stdout", "hey", 4, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Set it back to disabled, just to be sure.
    evtSourceEnabledSet(evt, CFG_SRC_CONSOLE, 0);
    json = evtLog(evt, "stdout", "hey", 4, 12345, &proc);
    assert_null(json);

    evtDestroy(&evt);
}

static void
evtLogWithAndWithoutMatchingNameFilter(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtSourceEnabledSet(evt, CFG_SRC_FILE, 1);

    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-log",
                      .id = "host-evttest-cmd-4"};

    // default name filter matches anything with log in the path
    cJSON* json = evtLog(evt, "/var/log/something.log", "hey", 4, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the name filter to ".*my[.]log" shouldn't match.
    evtNameFilterSet(evt, CFG_SRC_FILE, ".*my[.]log");
    json = evtLog(evt, "/var/log/something.log", "hey", 4, 12345, &proc);
    assert_null(json);

    // Changing the name filter to "^/var/log/.*[.]log$" should match.
    evtNameFilterSet(evt, CFG_SRC_FILE, "^/var/log/.*[.]log$");
    json = evtLog(evt, "/var/log/something.log", "hey", 4, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    evtDestroy(&evt);
}

static void
evtLogWithAndWithoutMatchingValueFilter(void** state)
{
    evt_t* evt = evtCreate();
    assert_non_null(evt);
    evtSourceEnabledSet(evt, CFG_SRC_FILE, 1);

    proc_id_t proc = {.pid = 4848,
                      .ppid = 4847,
                      .hostname = "host",
                      .procname = "evttest",
                      .cmd = "cmd-log",
                      .id = "host-evttest-cmd-4"};

    // default value filter matches anything
    cJSON* json = evtLog(evt, "/var/log/something.log", "hey", 4, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    // Changing the value filter to "blah" shouldn't match.
    evtValueFilterSet(evt, CFG_SRC_FILE, "blah");
    json = evtLog(evt, "/var/log/something.log", "hey", 4, 12345, &proc);
    assert_null(json);

    // Changing the value filter to "hey" should match.
    evtValueFilterSet(evt, CFG_SRC_FILE, "hey");
    json = evtLog(evt, "/var/log/something.log", "hey", 4, 12345, &proc);
    assert_non_null(json);
    cJSON_Delete(json);

    evtDestroy(&evt);
}

static void
evtValueFilterSetAndGet(void** state)
{
    evt_t* evt = evtCreate();

    /*
     * WARNING: This is hardcoded!! 
     * The default is ".*"
     * When the default changes this needs to change
    */
    regex_t* default_re = evtValueFilter(evt, CFG_SRC_FILE);
    assert_non_null(default_re);
    assert_int_equal(regexec(default_re, "anythingmatches", 0, NULL, 0), 0);

    // Make sure it can be changed
    evtValueFilterSet(evt, CFG_SRC_FILE, "myvalue.*");
    regex_t* new_re = evtValueFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "whatever", 0, NULL, 0), REG_NOMATCH);
    assert_int_equal(regexec(new_re, "myvalue.value", 0, NULL, 0), 0);

    // Make sure default is returned for null strings
    evtValueFilterSet(evt, CFG_SRC_FILE, "");
    new_re = evtValueFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "anythingmatches", 0, NULL, 0), 0);

    // Make sure default is returned for bad regex
    evtValueFilterSet(evt, CFG_SRC_FILE, "W![T^F?");
    new_re = evtValueFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "anything", 0, NULL, 0), 0);

    evtDestroy(&evt);

    // Get a default filter, even if evt is NULL
    default_re = evtValueFilter(evt, CFG_SRC_FILE);
    assert_non_null(default_re);
    assert_int_equal(regexec(default_re, "whatever", 0, NULL, 0), 0);
}

static void
evtFieldFilterSetAndGet(void** state)
{
    evt_t* evt = evtCreate();

    /*
     * WARNING: This is hardcoded!! 
     * The default is ".*host.*"
     * When the default changes this needs to change
    */
    regex_t* default_re = evtFieldFilter(evt, CFG_SRC_FILE);
    assert_non_null(default_re);
    assert_int_equal(regexec(default_re, "host:", 0, NULL, 0), 0);

    // Make sure it can be changed
    evtFieldFilterSet(evt, CFG_SRC_FILE, "myfield.*");
    regex_t* new_re = evtFieldFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "whatever", 0, NULL, 0), REG_NOMATCH);
    assert_int_equal(regexec(new_re, "myfield.value", 0, NULL, 0), 0);

    // Make sure default is returned for null strings
    evtFieldFilterSet(evt, CFG_SRC_FILE, "");
    new_re = evtFieldFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "host.myhost", 0, NULL, 0), 0);

    // Make sure default is returned for bad regex
    evtFieldFilterSet(evt, CFG_SRC_FILE, "W![T^F?");
    new_re = evtFieldFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "thishost", 0, NULL, 0), 0);

    evtDestroy(&evt);

    // Get a default filter, even if evt is NULL
    default_re = evtFieldFilter(evt, CFG_SRC_FILE);
    assert_non_null(default_re);
    assert_int_equal(regexec(default_re, "dohost", 0, NULL, 0), 0);
}

static void
evtNameFilterSetAndGet(void** state)
{
    evt_t* evt = evtCreate();

    /*
     * WARNING: This is hardcoded!! 
     * The default is ".*log.*"
     * When the default changes this needs to change
    */
    regex_t* default_re = evtNameFilter(evt, CFG_SRC_FILE);
    assert_non_null(default_re);
    assert_int_equal(regexec(default_re, "anythingwithlogmatches", 0, NULL, 0), 0);

    // Make sure it can be changed
    evtNameFilterSet(evt, CFG_SRC_FILE, "net.*");
    regex_t* new_re = evtNameFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "whatever", 0, NULL, 0), REG_NOMATCH);
    assert_int_equal(regexec(new_re, "net.tx", 0, NULL, 0), 0);

    // Make sure default is returned for null strings
    evtNameFilterSet(evt, CFG_SRC_FILE, "");
    new_re = evtNameFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "anythingwithlogmatches", 0, NULL, 0), 0);

    // Make sure default is returned for bad regex
    evtNameFilterSet(evt, CFG_SRC_FILE, "W![T^F?");
    new_re = evtNameFilter(evt, CFG_SRC_FILE);
    assert_non_null(new_re);
    assert_int_equal(regexec(new_re, "anythingwithlog", 0, NULL, 0), 0);

    evtDestroy(&evt);

    // Get a default filter, even if evt is NULL
    default_re = evtNameFilter(evt, CFG_SRC_FILE);
    assert_non_null(default_re);
    assert_int_equal(regexec(default_re, "logthingsmatch", 0, NULL, 0), 0);
}

static void
evtSourceEnabledSetAndGet(void** state)
{
    evt_t* evt = evtCreate();

    // Set everything to 1
    int i, j;
    for (i=CFG_SRC_FILE; i<CFG_SRC_MAX+1; i++) {
        evtSourceEnabledSet(evt, i, 1);
        if (i >= CFG_SRC_MAX) {
             assert_int_equal(evtSourceEnabled(evt, i), DEFAULT_SRC_FILE);
             assert_int_equal(dbgCountMatchingLines("src/evt.c"), 1);
             dbgInit(); // reset dbg for the rest of the tests
        } else {
             assert_int_equal(dbgCountMatchingLines("src/evt.c"), 0);
             assert_int_equal(evtSourceEnabled(evt, i), 1);
        }
    }

    // Clear one at a time to see there aren't side effects
    for (i=CFG_SRC_FILE; i<CFG_SRC_MAX; i++) {
        evtSourceEnabledSet(evt, i, 0); // Clear it
        for (j=CFG_SRC_FILE; j<CFG_SRC_MAX; j++) {
            if (i==j)
                 assert_int_equal(evtSourceEnabled(evt, j), 0);
            else
                 assert_int_equal(evtSourceEnabled(evt, j), 1);
        }
        evtSourceEnabledSet(evt, i, 1); // Set it back
    }

    evtDestroy(&evt);

    // Test get with NULL evt
    for (i=CFG_SRC_FILE; i<CFG_SRC_MAX; i++) {
        unsigned expected;
        switch (i) {
            case CFG_SRC_FILE:
                expected = DEFAULT_SRC_FILE;
                break;
            case CFG_SRC_CONSOLE:
                expected = DEFAULT_SRC_CONSOLE;
                break;
            case CFG_SRC_SYSLOG:
                expected = DEFAULT_SRC_SYSLOG;
                break;
            case CFG_SRC_METRIC:
                expected = DEFAULT_SRC_METRIC;
                break;
        }

        assert_int_equal(evtSourceEnabled(evt, i), expected);
    }
}

int
main(int argc, char* argv[])
{
    printf("running %s\n", argv[0]);

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(evtCreateReturnsValidPtr),
        cmocka_unit_test(evtDestroyNullOutDoesntCrash),
        cmocka_unit_test(evtMetricHappyPath),
        cmocka_unit_test(evtMetricWithSourceDisabledReturnsNull),
        cmocka_unit_test(evtMetricWithAndWithoutMatchingNameFilter),
        cmocka_unit_test(evtMetricWithAndWithoutMatchingFieldFilter),
        cmocka_unit_test(evtMetricWithAndWithoutMatchingValueFilter),
        cmocka_unit_test(evtMetricRateLimitReturnsNotice),
        cmocka_unit_test(evtLogWithSourceDisabledReturnsNull),
        cmocka_unit_test(evtLogWithAndWithoutMatchingNameFilter),
        cmocka_unit_test(evtLogWithAndWithoutMatchingValueFilter),
        cmocka_unit_test(evtSourceEnabledSetAndGet),
        cmocka_unit_test(evtValueFilterSetAndGet),
        cmocka_unit_test(evtFieldFilterSetAndGet),
        cmocka_unit_test(evtNameFilterSetAndGet),
        cmocka_unit_test(dbgHasNoUnexpectedFailures),
    };
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}
