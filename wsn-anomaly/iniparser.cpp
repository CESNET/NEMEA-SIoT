#include <iostream>
#include "INIReader.h"


using namespace std;

int main (){
INIReader reader("config.txt");
if (reader.ParseError() < 0){
    cout << "can not load test file";
    return 1;
}

cout << "config file loaded" << endl;

cout << reader.Get("SOFCount","profile","UNK") << endl;


return 1;
}
