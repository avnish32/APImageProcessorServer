#include "RotateFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	short direction = _filterParams.at(0);
	short numOfTurns = _filterParams.at(1);

	//TODO use enum for this
	if (direction != 0 && direction != 1) {
		//cout << "\nERROR: Invalid direction given for rotation.";
		_msgLogger->LogError("ERROR: Invalid direction given for rotation.");
		return false;
	}

	if (numOfTurns < 0) {
		//cout << "\nERROR: Invalid number of turns given for rotation.";
		_msgLogger->LogError("ERROR: Invalid number of turns given for rotation.");
		return false;
	}

	return true;
}