//
//  CommandManager.h
//  gifFromWebcam
//
//

#ifndef CommandManager_h
#define CommandManager_h

#ifdef CINDER_MSW
#include <windows.h>
#include <process.h>
#else
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <cstdint>
#include <deque>
#include <thread>
#endif //CINDER_MSW
using namespace std;

string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}


string exec(const char * cmd){
#ifdef CINDER_MSW
	system(cmd);
	return "OK";
#else
     array<char, 128> buffer;
     string result;
     unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
     if (!pipe) {
         throw runtime_error("popen() failed!");
     }
     while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
         result += buffer.data();
     }
     return result;
#endif //CINDER_MSW
}
#endif /* CommandManager_h */
