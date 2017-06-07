#ifndef _HANGUL_AUTOMATA_H_
#define _HANGUL_AUTOMATA_H_

#include <string>
#include <map>

#include "cocos2d.h"

class HangulAutomata : cocos2d::Ref {
public:
	static HangulAutomata * getInstance();

	std::string combine(std::string& eng);
	void clear();

private:
	static HangulAutomata * _instance;

	static const std::map<char, std::u16string> _alphabets;

private:
	HangulAutomata() {}

	std::u16string combineHangul(int cho, int jung, int jong);
	std::string toUTF8(const std::u16string& u16);

	std::string _temp = "";

};


#endif