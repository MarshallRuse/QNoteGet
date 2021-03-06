#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>

class CourseCategory;

using namespace std;

class CourseCategory {
public:
        CourseCategory(string categoryName, string urlPath, vector<string> fileExtensions);
        vector<string> getExtensionPreferences();
        string getCategoryName();
        string getUrlPath();
        void setExtensionPreferences(vector<string> extensions);

private:
        string categoryName;
        string urlPath;
        vector<string> fileExtensions;

};
