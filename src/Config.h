
/****************************************************************************
 * *   日期:  2014-12-5
 * *   目的:  读取配置文件的信息，以map的形式存入
 * *   要求:  配置文件的格式，以#作为行注释，配置的形式是key = value，中间可有空
 * 格，也可没有空格
 *  http://blog.csdn.net/dinghqalex/article/details/41750403
 * *****************************************************************************/

#ifndef _GET_CONFIG_H_
#define _GET_CONFIG_H_

#define COMMENT_CHAR '#'
#include <string>
#include <map>

using namespace std;


bool ReadConfig(const string & filename, map<string,string> &m);
void PrintConfig(const map<string, string> & m);
    
bool IsSpace(char c);
bool IsCommentChar(char c);
void Trim(string & str);
bool AnalyseLine(const string & line, string & key, string & value);
    
int getDouble(map<string, string> &m, const string& key, double &num) ; 
 





#endif



