#include <iostream>
#include "INIReader.h"


using namespace std;

int main (){
INIReader reader("test.ini");

if (reader.ParseError() < 0){
    cout << "Can not load test file";
    return 1;
}

cout << "config file loaded" << endl;

set<string> sections = reader.Sections();

for (auto it=sections.begin(); it != sections.end(); ++it){ 
        cout << ' ' << *it; 
        cout << reader.GetInteger(*it,"id",-1) << endl;
        cout << reader.GetInteger(*it,"len",-1) << endl;
        cout << reader.GetInteger(*it,"learn",-1) << endl;
        cout << reader.GetInteger(*it,"ignore",-1) << endl;
        cout << reader.Get(*it,"store","None") << endl;
        cout << reader.GetInteger(*it,"check",-1) << endl;
        cout << reader.GetInteger(*it,"export",-1) << endl;
}

//cout << reader.GetInteger("SOFCount","profile","UNK") << endl;


//reader.Sections();


return 1;
}

/*
td::string Get(std::string section, std::string name,
335                     std::string default_value) const;
336
337     // Get an integer (long) value from INI file, returning default_value if
338     // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
339     long GetInteger(std::string section, std::string name, long default_value) const;
340
341     // Get a real (floating point double) value from INI file, returning
342     // default_value if not found or not a valid floating point value
343     // according to strtod().
344     double GetReal(std::string section, std::string name, double default_value) const;
345
346     // Get a boolean value from INI file, returning default_value if not found or if
347     // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
348     // and valid false values are "false", "no", "off", "0" (not case sensitive).
349     bool GetBoolean(s
*/
