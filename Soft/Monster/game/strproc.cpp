#include "StdAfx.h"

#include "StrProc.h"

#include <hash_map>
#include <stack>
#include <math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NStr
{
	std::hash_map<char, char> brackets;   // map with open bracket <=> close bracket respection
	char cBracketTypes[8] = "({[\" ";     // all available brackets (open)
	const int NUM_BRACKET_TYPES = 4;      // number of available brackets
	//
	void InitStringProcessor();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// проинициализировать внутренние структуры string processor'а
void NStr::InitStringProcessor()
{
	brackets['('] = ')';
	brackets['['] = ']';
	brackets['{'] = '}';
	brackets['\"'] = '\"';
}
// это вспомогательная структура для автоматической инициализации string processor'а
struct SStrProcInit
{
	SStrProcInit() { NStr::InitStringProcessor(); }
};
static SStrProcInit spInit;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// получить закрывающую скобку по открывающей
const char NStr::GetCloseBracket( const char cOpenBracket )
{
	return brackets[cOpenBracket];
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// разделить строку на массив строк по заданному разделителю
void NStr::SplitString( const std::string &szString, std::vector<std::string> &szVector, const char cSeparator )
{
	int nPos = 0, nLastPos = 0;
	//
	do
	{
		nPos = szString.find( cSeparator, nLastPos );
		// add string
		szVector.push_back( szString.substr( nLastPos, nPos - nLastPos ) );
		nLastPos = nPos + 1;//szString.find_first_not_of( cSeparator, nPos );
		//
	} while( nPos != std::string::npos );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// разделить строку на массив строк по заданному разделителю с учётом скобок одной вложенности
void NStr::SplitStringWithBrackets( const std::string &szString, std::vector<std::string> &szVector, const char cSeparator )
{
	int nPos = 0, nLastPos = 0;
	//
	cBracketTypes[NUM_BRACKET_TYPES] = cSeparator;
	//
	do
	{
		nPos = szString.find_first_of( cBracketTypes, nLastPos );
		if ( nPos != std::string::npos )
		{
			if ( szString[nPos] != cSeparator )      // this is a bracket
			{
				nPos = szString.find( brackets[szString[nPos]], nPos + 1 );
				continue;
			}
		}
		// add string
		szVector.push_back( szString.substr( nLastPos, nPos - nLastPos ) );
		nLastPos = nPos + 1;//szString.find_first_not_of( cSeparator, nPos );
		//
	} while( nPos != std::string::npos );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// разделить строку на массив строк по заданному разделителю с учётом скобок любой вложенности
void NStr::SplitStringWithMultipleBrackets( const std::string &szString, std::vector<std::string> &szVector, const char cSeparator )
{
	std::stack<char> stackBrackets;
	int nLastPos = 0;
  DWORD i;
	//
	for ( i=0; i<szString.size(); ++i )
	{
		char c = szString[i];
		if ( IsOpenBracket(c) )
			stackBrackets.push( brackets[c] );
		else if ( stackBrackets.empty() )
		{
			if ( c == cSeparator )
			{
				szVector.push_back( szString.substr( nLastPos, i - nLastPos ) );
				nLastPos = i + 1; // szString.find_first_not_of( cSeparator, i );
			}
		}
		else if ( c == stackBrackets.top() )
			stackBrackets.pop();
	}
	// last substring
	if ( nLastPos + 1 < int( i ) )
		szVector.push_back( szString.substr( nLastPos ) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// найти закрывающую скобку без учёта внутренних скобок
int NStr::FindCloseBracket( const std::string &szString, int nPos, const char cOpenBracket )
{
	return szString.find( brackets[cOpenBracket], nPos );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// найти закрывающую скобку с учётом внутренних скобок
int NStr::FindMultipleCloseBracket( const std::string &szString, int nPos, const char cOpenBracket )
{
	std::stack<char> stackBrackets;
	//
	stackBrackets.push( brackets[cOpenBracket] );
	for ( DWORD i=nPos; i<szString.size(); ++i )
	{
		char c = szString[i];
		if ( IsOpenBracket(c) )
			stackBrackets.push( brackets[c] );
		else if ( c == stackBrackets.top() )
		{
			stackBrackets.pop();
			// check bracket stack for empty in the case of the bracket pop
			if ( stackBrackets.empty() )
				return i;
		}
	}
	return std::string::npos;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// отрезать все символы 'cTrim' справа
void NStr::TrimRight( std::string &szString, const char cTrim )
{
	int nPos = szString.find_last_not_of( cTrim );
	if ( nPos != std::string::npos )
		szString.erase( nPos + 1, std::string::npos );
}
void NStr::TrimRight( std::string &szString, const char *pszTrim )
{
	int nPos = szString.find_last_not_of( pszTrim );
	if ( nPos != std::string::npos )
		szString.erase( nPos + 1, std::string::npos );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// вырезать все символы 'cTrim' из строки
class CSymbolCheckFunctional
{
private:
  const char *pszSymbols;
public:
  explicit CSymbolCheckFunctional( const char *pszNewSymbols ) : pszSymbols( pszNewSymbols ) {  }
  bool operator()( const char cSymbol )
  {
    for ( const char *p = pszSymbols; *p != 0; ++p )
    {
      if ( *p == cSymbol )
        return true;
    }
    return false;
  }
};
void NStr::TrimInside( std::string &szString, const char *pszTrim )
{
  szString.erase( std::remove_if(szString.begin(), szString.end(), CSymbolCheckFunctional(pszTrim)), szString.end() );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// преобразовать целое в строку, разделяя каждые три знака (три порядка) специальным разделителем (.)
void NStr::ConvertIntToDotString( int nVal, std::string &szString, const char cSeparator )
{
	char buff[32], buff2[32];
	buff[0] = buff2[0] = 0;
  int nOrder = static_cast<int>( log10( nVal ) );
  int nOrderVal = static_cast<int>( pow( 10, nOrder - (nOrder % 3) ) );
  while ( nOrderVal > 1 )
  {
    int nVal1 = nVal / nOrderVal;
		sprintf( buff2, "%d%c", nVal1, cSeparator );
		strcat( buff, buff2 );
    nVal -= nVal1 * nOrderVal;
    nOrderVal /= 1000;
  }
	strcat( buff, _itoa(nVal, buff2, 10) );
  szString = buff;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// форматирование строки
const char* __cdecl NStr::Format( const char *pszFormat, ... )
{
  static char buff[2048];
  va_list va;
	// 
  va_start( va, pszFormat );
  vsprintf( buff, pszFormat, va );
  va_end( va );
	//
	return buff;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NStr::CBracketCharSeparator::operator()( const char cSymbol )
{
	if ( IsOpenBracket(cSymbol) )
		stackBrackets.push( brackets[cSymbol] );
	else if ( stackBrackets.empty() )
		return cSymbol == cSeparator;
	else if ( cSymbol == stackBrackets.top() )
		stackBrackets.pop();
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
