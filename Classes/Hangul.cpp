#include "Hangul.h"

#include "cocos2d.h"
//#include "cocos2d/cocos/base/ccUTF8.h"

using namespace std;
USING_NS_CC;

HangulAutomata * HangulAutomata::_instance = nullptr;

const map<char, u16string> HangulAutomata::_alphabets = {
	{ 'q', u"け" },
	{ 'w', u"じ" },
	{ 'e', u"ぇ" },
	{ 'r', u"ぁ" },
	{ 't', u"さ" },
	{ 'y', u"に" },
	{ 'u', u"づ" },
	{ 'i', u"ち" },
	{ 'o', u"だ" },
	{ 'p', u"つ" },
	{ 'O', u"ぢ" },
	{ 'P', u"て" },
	{ 'a', u"け" },
	{ 's', u"い" },
	{ 'd', u"し" },
	{ 'f', u"ぉ" },
	{ 'g', u"ぞ" },
	{ 'h', u"で" },
	{ 'j', u"っ" },
	{ 'k', u"た" },
	{ 'l', u"び" },
	{ 'z', u"せ" },
	{ 'x', u"ぜ" },
	{ 'c', u"ず" },
	{ 'v', u"そ" },
	{ 'b', u"ば" },
	{ 'n', u"ぬ" },
	{ 'm', u"ぱ" },
	{ 'Q', u"こ" },
	{ 'W', u"す" },
	{ 'E', u"え" },
	{ 'R', u"あ" },
	{ 'T', u"ざ" },
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