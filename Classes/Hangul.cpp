#include "Hangul.h"

#include "cocos2d.h"
//#include "cocos2d/cocos/base/ccUTF8.h"

using namespace std;
USING_NS_CC;

HangulAutomata * HangulAutomata::_instance = nullptr;

const map<char, u16string> HangulAutomata::_alphabets = {
	{ 'q', u"��" },
	{ 'w', u"��" },
	{ 'e', u"��" },
	{ 'r', u"��" },
	{ 't', u"��" },
	{ 'y', u"��" },
	{ 'u', u"��" },
	{ 'i', u"��" },
	{ 'o', u"��" },
	{ 'p', u"��" },
	{ 'O', u"��" },
	{ 'P', u"��" },
	{ 'a', u"��" },
	{ 's', u"��" },
	{ 'd', u"��" },
	{ 'f', u"��" },
	{ 'g', u"��" },
	{ 'h', u"��" },
	{ 'j', u"��" },
	{ 'k', u"��" },
	{ 'l', u"��" },
	{ 'z', u"��" },
	{ 'x', u"��" },
	{ 'c', u"��" },
	{ 'v', u"��" },
	{ 'b', u"��" },
	{ 'n', u"��" },
	{ 'm', u"��" },
	{ 'Q', u"��" },
	{ 'W', u"��" },
	{ 'E', u"��" },
	{ 'R', u"��" },
	{ 'T', u"��" },
};

HangulAutomata * HangulAutomata::getInstance() {
	if (_instance == nullptr) {
		_instance = new HangulAutomata;
		//PoolManager::getInstance()->getCurrentPool()->addObject(_instance);
	}
	return _instance;
}

string HangulAutomata::combine(string& eng) {
	u16string ret16 = u"";

	_temp += eng;

	for (auto i : eng) {
		if (_alphabets.find(i) != _alphabets.end()) { 
			//ret16 += combineHangul(0, 0, 0);
			ret16 += _alphabets.at(i);
		} else ret16 += i;
	}

	return toUTF8(ret16);
}

void HangulAutomata::clear() {
	_temp.clear();
}

u16string HangulAutomata::combineHangul(int cho, int jung, int jong) {
	u16string ret(1, 0xac00 + cho * 21 * 28 + jung * 28 + jong);
	return  ret;
}

string HangulAutomata::toUTF8(const std::u16string & u16) {
	string ret;
	cocos2d::StringUtils::UTF16ToUTF8(u16, ret);
	return ret;
}