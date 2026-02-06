#ifndef __STRING_PROCESSING_H__
#define __STRING_PROCESSING_H__
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <stack>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NStr
{
	// все операции со скобками учитывают следующие скобки: (), {}, [], ""
	//
	// получить закрывающую скобку по открывающей
	// символ 'cOpenBracket' должен об€зательно быть открывающей скобкой
	const char GetCloseBracket( const char cOpenBracket );
	// €вл€етс€ ли символ открывающей скобкой
	inline bool IsOpenBracket( const char cSymbol )
	{
		return ( (cSymbol == '(') || (cSymbol == '{') || (cSymbol == '[') || (cSymbol == '\"') );
	}
	// €вл€етс€ ли символ закрывающей скобкой
	inline bool IsCloseBracket( const char cSymbol )
	{
		return ( (cSymbol == ')') || (cSymbol == '}') || (cSymbol == ']') || (cSymbol == '\"') );
	}
	// разделить строку на массив строк по заданному разделителю
	void SplitString( const std::string &szString, std::vector<std::string> &szVector, const char cSeparator );
	// разделить строку на массив строк по заданному разделителю с учЄтом скобок одной вложенности
	void SplitStringWithBrackets( const std::string &szString, std::vector<std::string> &szVector, const char cSeparator );
	// разделить строку на массив строк по заданному разделителю с учЄтом скобок любой вложенности
	void SplitStringWithMultipleBrackets( const std::string &szString, std::vector<std::string> &szVector, const char cSeparator );
	// найти закрывающую скобку без учЄта внутренних скобок
	int FindCloseBracket( const std::string &szString, int nPos, const char cOpenBracket );
	// найти закрывающую скобку с учЄтом внутренних скобок
	int FindMultipleCloseBracket( const std::string &szString, int nPos, const char cOpenBracket );
	// отрезать все символы 'cTrim'
	// отрезать все 'cTrim' слева
	inline void TrimLeft( std::string &szString, const char cTrim ) { szString.erase( 0, szString.find_first_not_of( cTrim ) ); }
	// отрезать все 'pszTrim' слева
	inline void TrimLeft( std::string &szString, const char *pszTrim ) { szString.erase( 0, szString.find_first_not_of( pszTrim ) ); }
	// отрезать все whitespaces слева
  inline void TrimLeft( std::string &szString ) { TrimLeft(szString, " \t\n"); } 
	// отрезать все 'pszTrim' справа
	void TrimRight( std::string &szString, const char *pszTrim );
	// отрезать все 'cTrim' справа
	void TrimRight( std::string &szString, const char cTrim );   
	// отрезать все whitespaces справа
  inline void TrimRight( std::string &szString ) { TrimRight(szString, " \t\n"); }
	// отрезать все 'pszTrim' с обоих концов
	inline void TrimBoth( std::string &szString, const char *pszTrim ) { TrimLeft( szString, pszTrim ); TrimRight( szString, pszTrim ); }
	// отрезать все 'cTrim' с обоих концов
	inline void TrimBoth( std::string &szString, const char cTrim ) { TrimLeft( szString, cTrim ); TrimRight( szString, cTrim ); }
	// отрезать все whitespaces с обоих концов
  inline void TrimBoth( std::string &szString ) { TrimBoth(szString, " \t\n"); }
	// вырезать все символы 'cTrim' из строки
	void TrimInside( std::string &szString, const char *pszTrim );
	inline void TrimInside( std::string &szString, const char cTrim ) { szString.erase( std::remove(szString.begin(), szString.end(), cTrim), szString.end() ); }
  inline void TrimInside( std::string &szString ) { TrimInside(szString, " \t\n"); }
	// привести к верхнему или нижнему регистру
	// MSVCMustDie_* are required to keep compiler happy when default calling conversion is __fastcall
	inline int MSVCMustDie_tolower( int a ) { return tolower(a); } 
	inline int MSVCMustDie_toupper( int a ) { return toupper(a); }
	inline void ToLower( std::string &szString ) { std::transform( szString.begin(), szString.end(), szString.begin(), std::ptr_fun(MSVCMustDie_tolower) ); }
	inline void ToUpper( std::string &szString ) { std::transform( szString.begin(), szString.end(), szString.begin(), std::ptr_fun(MSVCMustDie_toupper) ); }
  // преобразовать целое в строку, раздел€€ каждые три знака (три пор€дка) специальным разделителем (default = '.')
  void ConvertIntToDotString( int nVal, std::string &szString, const char cSeparator = '.' );
	// форматирование строки как в sprintf. 
	// NON-REENTRANT!!! Uses internal static buffer. Max string length = 2048 chars
	// € знаю, что non-reentrant функции это плохо, но дл€ данного случа€ это ќ„≈Ќ№ удобно
	const char* __cdecl Format( const char *pszFormat, ... );
	//
	//
	// default separator functor for CStringIterator
	class CCharSeparator
	{
		const char cSeparator;
	public:
		explicit CCharSeparator( const char _cSeparator )	: cSeparator( _cSeparator ) {  }
		bool operator()( const char cSymbol ) const { return cSymbol == cSeparator; }
	};
	// separator with recursive bracket functor
	class CBracketCharSeparator
	{
		const char cSeparator;
		std::stack<char> stackBrackets;
	public:
		explicit CBracketCharSeparator( const char _cSeparator )	: cSeparator( _cSeparator ) {  }
		bool operator()( const char cSymbol );
	};
	// std::string iteration class
	template <class TSeparator = CCharSeparator>
	class CStringIterator
	{
	private:
		const std::string &szInput;           // input string
		std::string szString;                 // result string
		TSeparator &tSeparator;               // functor, which returns true, if next char is separator
		int nLastPos;                         // current lexeme begin position
		int nPos;                             // current position
	public:
		CStringIterator( const std::string &_szInput, TSeparator &_tSeparator, int _nPos = 0 )
			: szInput( _szInput ), tSeparator( _tSeparator ), nPos( _nPos ), nLastPos( _nPos ) { operator++(); }
		// iteration
		// extract next lexem
		const CStringIterator& Next()
		{
			nLastPos = nPos;
			for ( ; nPos<szInput.size(); ++nPos )
			{
				if ( tSeparator(szInput[nPos]) )
				{
					szString = szInput.substr( nLastPos, nPos - nLastPos );
					++nPos;
					return *this;
				}
			}
			szString = szInput.substr( nLastPos, nPos - nLastPos );
			return *this;
		}
		const CStringIterator& operator++() { return Next(); }
		// is 'nPos' at the end or at the begining of the string
		bool IsBegin() const { return nPos == 0; }
		bool IsEnd() const { return nLastPos == szInput.size(); }
		// positions
		int GetLastPos() const { return nLastPos; }
		int GetPos() const { return nPos; }
		// access lexem (const and non-const)
		operator const std::string*() const { return &szString; }
		operator std::string*() { return &szString; }
		const std::string* operator->() const { return &szString; }
		std::string* operator->() { return &szString; }
		const std::string& operator*() const { return szString; }
		std::string& operator*() { return szString; }
		// access lexem as decimal or floating-point values
		operator const double() const { return atof( szString.c_str() ); }
		operator const float() const { return float( atof( szString.c_str() ) ); }
		operator const int() const { return atoi( szString.c_str() ); }
	};
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __STRING_PROCESSING_H__