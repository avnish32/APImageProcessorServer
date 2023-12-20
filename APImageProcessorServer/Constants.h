#pragma once
#include<iostream>

using namespace std;

const short NUM_THREADS = 4;

const short RESPONSE_SUCCESS = 0;
const short RESPONSE_FAILURE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;

const string SIZE_PAYLOAD_PREFIX = "Size ";