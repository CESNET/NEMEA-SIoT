#include <iostream>
#include "INIReader.h"
#include <vector>

// Data format
// - integers value: negative value means missing value, each field has it's owen data range
// - string value: dash (-) means missing values, other fileds must be verified. 


using namespace std;

vector<string> parseString(string value, string delimiter){

    size_t pos = 0;
    string token;
    vector<string> parsedData;
    while ((pos = value.find(delimiter)) != string::npos) {
        token = value.substr(0, pos);
        parsedData.push_back(token);
        value.erase(0, pos + delimiter.length());
    }

    parsedData.push_back(value);
    return parsedData;
}

int main (){
INIReader reader("test.ini");

if (reader.ParseError() < 0){
    cout << "Can not load test file";
    return 1;
}

set<string> sections = reader.Sections();
string profile = "";
vector<string> subsections;

for (auto it=sections.begin(); it != sections.end(); ++it){ 

    cout << "Parsing section " << (*it) << endl;      
    cout << endl;

    if ((*it).find(".") == string::npos ){
    
        //TODO validate each parameter
        cout << reader.GetInteger(*it,"id",-1) << endl;
        cout << reader.GetInteger(*it,"len",-1) << endl;
        cout << reader.GetInteger(*it,"learn",-1) << endl;
        cout << reader.GetInteger(*it,"ignore",-1) << endl;
        cout << reader.Get(*it,"store","-") << endl;
        cout << reader.GetInteger(*it,"check",-1) << endl;
        cout << reader.GetInteger(*it,"export",-1) << endl;
        cout << reader.Get(*it,"profile","-") << endl;
        profile = reader.Get(*it,"profile","-");
        cout << reader.Get(*it,"export_fields","-") << endl;
        
        //parse profile values;
        subsections = parseString(profile,",");
    } else {
        //parse subsections
        vector<string> tmp = parseString((*it),".");
        if ( find(subsections.begin(),subsections.end(),tmp[1]) != subsections.end() ){
            //continue with parsing the specific section
            cout << reader.GetInteger(*it,"soft_min",-1) << endl;
            cout << reader.GetInteger(*it,"soft_max",-1) << endl;
            cout << reader.GetInteger(*it,"hard_min",-1) << endl;
            cout << reader.GetInteger(*it,"hard_max",-1) << endl;
            cout << reader.GetInteger(*it,"grace_period",-1) << endl;
            cout << reader.GetInteger(*it,"grow_up",-1) << endl;
            cout << reader.GetInteger(*it,"grow_down",-1) << endl;

        } else {
            cout << "subsection filed was not found" << endl;
            // found subsection has no configuration
        }
    }
        // check section for each profile
        // verify if section exist in profile
        // if yes parse data, if print error message and continue
}

return 1;
}
