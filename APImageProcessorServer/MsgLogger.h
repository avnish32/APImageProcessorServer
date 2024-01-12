#include<fstream>
#include<mutex>
#include<iostream>

#pragma once

using std::ofstream;
using std::mutex;
using std::string;

//Code referred from Lecture material of Week A12 (w/c 11 Dec 2023)
/*
This class is responsible for logging messages to an external log file.
It is built on the singleton pattern, so only one instance of the class
is present in memory at any given time during program execution.
*/
class MsgLogger
{
private:
	static MsgLogger* logger_instance_;
	mutex mtx_;
	ofstream log_file_;

	MsgLogger();

public:
	~MsgLogger();
	static MsgLogger* GetInstance();
	void LogDebug(const string&);
	void LogError(const string&);
	static void ReleaseInstance();
};

