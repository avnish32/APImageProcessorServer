#include "FlipFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;

FlipFilterParamsValidator::FlipFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool FlipFilterParamsValidator::ValidateFilterParams()
{
	short direction = _filterParams.at(0);

	//TODO use enum for this
	if (direction != 0 && direction != 1) {
		cout << "\nERROR: Invalid direction given for flipping.";
		return false;
	}
	return true;
}
