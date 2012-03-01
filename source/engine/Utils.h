#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <JTS_Eng.h>

using namespace std;
using namespace JTS;

//-----------------------------------------------------------------------
string ToLowerCase(string &inString);
string ToUpperCase(string &inString);
//-----------------------------------------------------------------------
int SearchStr(string str, string where, int num=1, int after=0);
int SearchStrSens(const string& str, const string& where, int num=1, int after=0);
bool TpltSearchStr(string what, string where);
bool TpltSearchStrSens(string what, string where);
int CountOf(string what, string where);
int CountOfSens(string what, string where);
//-----------------------------------------------------------------------
int StrToInt(const string &str);
uint StrToUInt(const string &str);
float StrToFloat(const string &str);
bool StrToBool(string &str);
//-----------------------------------------------------------------------
string IntToStr(int val);
string Int64ToStr(int64 val);
string UIntToStr(uint val);
string UInt64ToStr(uint64 val);
string UIntToStrX(uint val);
string FloatToStr(float val);
string DoubleToStr(double val);
string BoolToStr(bool val);
//-----------------------------------------------------------------------
string GetFilePath(const char *name);
string GetFileName(const char *name);
string GetFileExt(const char *name);
string GetOnlyFileName(const char *name);
//-----------------------------------------------------------------------
uchar EngKeyToASCIIKey(const uint8 key);
uint8 ASCIIKeyToEngKey(const uchar key);
//-----------------------------------------------------------------------

#endif //_UTILS_H