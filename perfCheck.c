/*
 * perfCheck.c
 *
 *  Created on: Apr 19, 2017
 *      Author: kenan
 *
 * Manual for perf_event_open()
 * If PERF_FORMAT_GROUP was specified to allow reading all events in a
 * group at once:
 *
 *     struct read_format {
 *         u64 nr;            * The number of events *
 *         u64 time_enabled;  * if PERF_FORMAT_TOTAL_TIME_ENABLED *
 *         u64 time_running;  * if PERF_FORMAT_TOTAL_TIME_RUNNING *
 *         struct {
 *             u64 value;     * The value of the event *
 *             u64 id;        * if PERF_FORMAT_ID *
 *         } values[nr];
 *     };
 *
 * If PERF_FORMAT_GROUP was not specified:
 *
 *     struct read_format {
 *         u64 value;         * The value of the event *
 *         u64 time_enabled;  * if PERF_FORMAT_TOTAL_TIME_ENABLED *
 *         u64 time_running;  * if PERF_FORMAT_TOTAL_TIME_RUNNING *
 *         u64 id;            * if PERF_FORMAT_ID *
 *     };
 *
 */

#include <stdio.h>
#include <jni.h>
#include <stdlib.h>
#include <errno.h>
#include <perfmon/pfmlib_perf_event.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>

static int enabled = 0;
static int isGrouped = 0;
static int *perf_event_fds;
static perf_event_attr_t *perf_event_attrs;
static size_t dataLength;
static int buffer_size;
static int num_events;
//Value from previous read
static long previousValue = 0;

/**
 *  Set all counter values in an event group to be read with one read or not
 */
void setGroup(bool isSet) {
	isGrouped = isSet;
}

JNIEXPORT void JNICALL Java_PerfCheckUtils_perfInit(JNIEnv * env, jclass jcls, jint numEvents, jint isSet) {
	setGroup(isSet);
	num_events = numEvents;
	int i;
	int ret = pfm_initialize();
	if (ret != PFM_SUCCESS) {
		errx(1, "cannot initialize library: %s", pfm_strerror(ret));
	}

	perf_event_fds = (int *) calloc(numEvents, sizeof(int));
	if (!perf_event_fds) {
		errx(1, "error allocating perf_event_fds");
	}

	if (isGrouped) {
		buffer_size = 3 + (2 * numEvents);
	} else {
		buffer_size = 3;
	}

	dataLength = buffer_size * sizeof(long);

	perf_event_attrs = (perf_event_attr_t *) calloc(numEvents, sizeof(perf_event_attr_t));
	if (!perf_event_attrs) {
		errx(1, "error allocating perf_event_attrs");
	}

	for (i = 0; i < numEvents; i++) {
		perf_event_attrs[i].size = sizeof(perf_event_attr_t);
	}

	enabled = 1;
}

JNIEXPORT void JNICALL Java_PerfCheckUtils_groupPerfEventsCheck(JNIEnv * env, jclass jcls, jstring name) {

	char *eventName;
	char *event[num_events];
	int id = 0;
	int i = 0;

	eventName = (*env)->GetStringUTFChars( env, name , NULL );
	while((event[id] = strsep(&eventName, ",")) != NULL) ++id;

	for(i = 0; i < id; i++) {
		perf_event_attr_t *attr = (perf_event_attrs + i);

		int ret = pfm_get_perf_event_encoding(event[i], PFM_PLM3, attr, NULL,NULL);
		if (ret != PFM_SUCCESS) {
			errx(1, "cannot get encoding %d '%s': %s\n", i, event[i],
					pfm_strerror(ret));
		}

		attr->read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
		//Enable event group to be read with one read
		attr->read_format |= PERF_FORMAT_GROUP | PERF_FORMAT_ID;
		//Inherit does not work for some combinations of read_format
		//values, such as PERF_FORMAT_GROUP.
		attr->inherit = 0;
		attr->disabled = 1;

		//Kenan: edit for group reading
		if (i == 0) {
			//	  Open perf event for group leader, and set its own group leader fd to be -1.
			perf_event_fds[i] = perf_event_open(attr, 0, -1, -1, 0);
		} else {
			//Open perf event for the rest members in this group.
			perf_event_fds[i] = perf_event_open(attr, 0, -1, perf_event_fds[0], 0);
		}

		if (perf_event_fds[i] == -1) {
			errx(1, "cannot open library perf_event_open: %d '%s'", i, event[i]);
		}
	}
}

JNIEXPORT void JNICALL Java_PerfCheckUtils_singlePerfEventCheck(JNIEnv * env, jclass jcls, jstring name) {
	char *eventName;
	char *event[num_events];
	int i = 0;
	int id = 0;
	eventName = (*env)->GetStringUTFChars( env, name , NULL );
	while((event[id] = strsep(&eventName, ",")) != NULL) ++id;

	for(i = 0; i < id; i++) {
		perf_event_attr_t *attr = (perf_event_attrs + i);

		//Helper library to configure perf
		int ret = pfm_get_perf_event_encoding(event[i], PFM_PLM3, attr, NULL,NULL);
		if (ret != PFM_SUCCESS) {
			errx(1, "cannot get encoding %d '%s': %s\n", i, event[i],
					pfm_strerror(ret));
		}

		attr->read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

		attr->disabled = 1;
		attr->inherit = 1;

		//Read perf counter values
		perf_event_fds[i] = perf_event_open(attr, 0, -1, -1, 0);

		if (perf_event_fds[i] == -1) {
			errx(1, "cannot open library perf_event_open: %d '%s'", i, event[i]);
		}
	}

}

JNIEXPORT void JNICALL Java_PerfCheckUtils_perfEnable(JNIEnv * env, jclass jcls) {
	if (enabled) {
		if (prctl(PR_TASK_PERF_EVENTS_ENABLE)) {
			errx(1, "cannot enable perf in prctl(PR_TASK_PERF_EVENTS_ENABLE)");
		}
	}
}

JNIEXPORT void JNICALL Java_PerfCheckUtils_perfDisable(JNIEnv * env, jclass jcls) {
	if (!enabled) {
		if (prctl(PR_TASK_PERF_EVENTS_DISABLE)) {
			errx(1, "cannot disable perf in prctl(PR_TASK_PERF_EVENTS_DISABLE)");
		}
	}
}


JNIEXPORT void JNICALL Java_PerfCheckUtils_perfSingleRead(JNIEnv * env, jclass jcls, jint id, jlongArray buffer) {
//	size_t dataLength = 3 * sizeof(long long);
	long *values;
	values = (*env)->GetLongArrayElements(env, buffer, NULL);

	int ret = read(perf_event_fds[id], values, dataLength);

	if (ret < 0) {
		errx(1, "Failed to open: %s", strerror(errno));
	}

	if (ret != dataLength) {
		errx(1, "Failed to read");
	}

	(*env)->ReleaseLongArrayElements(env, buffer, values, 0);
}

JNIEXPORT void JNICALL Java_PerfCheckUtils_perfMultRead(JNIEnv * env, jclass jcls, jlongArray buffer) {

	long *values;
	values = (*env)->GetLongArrayElements(env, buffer, NULL);

	int ret = read(perf_event_fds[0], values, dataLength);

	if (ret < 0) {
		err(1, "Failed to open: %s", strerror(errno));
	}

	if (ret != dataLength) {
		errx(1, "Failed to read");
	}

	(*env)->ReleaseLongArrayElements(env, buffer, values, 0);
}

JNIEXPORT jlongArray JNICALL Java_PerfCheckUtils_processMultiValue(JNIEnv * env, jclass jcls, jlongArray buffer) {

	int RAW_COUNT = 0;
	int TIME_ENABLED = 1;
	int TIME_RUNNING = 2;
	bool overflowed = false;
	int dataWasScaled = 0;
	long *readBuffer;
	readBuffer = (*env)->GetLongArrayElements(env, buffer, NULL);

	long * values = malloc(num_events * sizeof(long));
	jlongArray result = (*env)->NewLongArray(env, num_events);
	int i;

	if (!readBuffer[TIME_ENABLED] || !readBuffer[TIME_RUNNING]) {
		printf("Time enabled: %ld, time running: %ld\n", readBuffer[TIME_ENABLED],
				readBuffer[TIME_RUNNING]);
	}
	if (readBuffer[TIME_ENABLED] != readBuffer[TIME_RUNNING]) {
		if (readBuffer[TIME_ENABLED] == 0) {
			dataWasScaled = 0;
		} else {
			dataWasScaled = readBuffer[TIME_ENABLED] / readBuffer[TIME_RUNNING];
		}

		for (i = 0; i < num_events; i++) {
			readBuffer[3 + i * 2] *= dataWasScaled;
			values[i] = readBuffer[3 + i * 2];
		}

		(*env)->SetLongArrayRegion(env, result, 0, num_events, values);
		free(values);
		return result;
	}
	//Return event counters only
	for (i = 0; i < num_events; i++) {
		values[i] = readBuffer[3 + i * 2];
	}

	(*env)->SetLongArrayRegion(env, result, 0, num_events, values);
	free(values);
	return result;

}

JNIEXPORT jlong JNICALL Java_PerfCheckUtils_processSingleValue(JNIEnv * env, jclass jcls, jlongArray buffer) {

	int RAW_COUNT = 0;
	int TIME_ENABLED = 1;
	int TIME_RUNNING = 2;
	bool overflowed = false;
	int dataWasScaled = 0;
	long *readBuffer;

	readBuffer = (*env)->GetLongArrayElements(env, buffer, NULL);

	if (readBuffer[TIME_ENABLED] == 0) {
		// Counter never run (assume contention)
//			contended = true;
		errx(1, "Event counter contended!\n");
	}
	if (readBuffer[RAW_COUNT] == 0) {
		errx(1, "Event counter is 0\n");
	}

	//When multiplexing is happening
	if (readBuffer[TIME_ENABLED] != readBuffer[TIME_RUNNING]) {
//			scaled = true;
		dataWasScaled = 1;
		double scaleFactor;
		if (readBuffer[TIME_RUNNING] == 0) {
			scaleFactor = 0;
		} else {
			scaleFactor = readBuffer[TIME_ENABLED] / readBuffer[TIME_RUNNING];
		}

		readBuffer[RAW_COUNT] = (long) (readBuffer[RAW_COUNT] * scaleFactor);
	}

	if (readBuffer[RAW_COUNT] < previousValue) {
		//TODO: figure out the maximum of PMU counters can hold
		//and do readBuffer[RAW_COUNT] += MAXIMUM
		errx(1, "overflow is happening");
	}

	previousValue = readBuffer[RAW_COUNT];

	return (jlong)readBuffer[RAW_COUNT];
}

int main() {

	long long readBuffer[3];
	perfInit(2, false);
	singlePerfEventCheck(0, "cache-misses");
	singlePerfEventCheck(1, "cache-references");
	perfEnable();

	printf("before sleep\n");

	perfSingleRead(0, readBuffer);
	printf("counter value 1 is: %d\n", processSingleValue(readBuffer));

	perfSingleRead(1, readBuffer);
	printf("counter value 2 is: %d\n", processSingleValue(readBuffer));

	sleep(10);

	printf("After sleep\n");

	perfSingleRead(0, readBuffer);
	printf("counter value 1 is: %d\n", processSingleValue(readBuffer));

	perfSingleRead(1, readBuffer);
	printf("counter value 2 is: %d\n", processSingleValue(readBuffer));

}

