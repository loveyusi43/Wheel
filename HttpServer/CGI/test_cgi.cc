#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>

bool GetQueryString(std::string &quert_string);

void CutString(std::string &in, const std::string &sep, std::string &out1, std::string &out2);

int main(void){
    std::cerr << std::endl;
    std::cerr << "--------------------------------------------------------" << std::endl;

    std::string query_string;
    GetQueryString(query_string);

    std::string str1;
    std::string str2;
    CutString(query_string, "&", str1, str2);

    std::string name1;
    std::string value1;
    CutString(str1, "=", name1, value1);

    std::string name2;
    std::string value2;
    CutString(str2, "=", name2, value2);

    std::cout << name1 << " : " << value1 << std::endl;
    std::cout << name2 << " : " << value2 << std::endl;

    std::cerr << name1 << " : " << value1 << std::endl;
    std::cerr << name2 << " : " << value2 << std::endl;

    int x = std::atoi(value1.c_str());
    int y = std::atoi(value2.c_str());
    std::cout << "<html>";
    std::cout << "<head><meta charset=\"utf-8\"></head>";
    std::cout << "<body>";
    std::cout << "<h3>" << value1 << "+" << value2 << "=" << x+y << "</h3>";
    std::cout << "<h3>" << value1 << "-" << value2 << "=" << x-y << "</h3>";
    std::cout << "<h3>" << value1 << "*" << value2 << "=" << x*y << "</h3>";
    std::cout << "<h3>" << value1 << "/" << value2 << "=" << x/y << "</h3>";
    std::cout << "</body>";
    std::cout << "</html>";

    std::cerr << "--------------------------------------------------------" << std::endl;
    std::cerr << std::endl;
    return 0;
}

void CutString(std::string &in, const std::string &sep, std::string &out1, std::string &out2) {
    size_t pos = in.find(sep);
    if (pos == std::string::npos) {
        return;
    }
    out1 = in.substr(0, pos);
    out2 = in.substr(pos+sep.size());
}

bool GetQueryString(std::string &query_string) {
    std::string method = getenv("METHOD");

    if (method == "GET") {
        query_string = getenv("QUERY_STRING");
        return true;
    }else if (method == "POST") {
        int content_length = std::atoi(getenv("CONTENT_LENGTH"));
        char c = 0;
        while (content_length--) {
            read(0, &c, 1);
            query_string += c;
        }
        return true;
    }else{
        return false;
    }
}