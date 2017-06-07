#ifndef _HANGUL_H_
#define _HANGUL_H_

#include "2d/CCTextFieldTTF.h"

std::string toUTF8(const std::u16string &u16);

class HangulTF : public cocos2d::TextFieldTTF {
public:
	HangulTF();

	static HangulTF * textFieldWithPlaceHolder(const std::string& placeholder, const cocos2d::Size& dimensions, cocos2d::TextHAlignment alignment, const std::string& fontName, float fontSize);
	static HangulTF * textFieldWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize);

protected:
	std::string engToKor(const std::string &eng);
	virtual void insertText(const char * text, size_t len) override;
	virtual void deleteBackward() override;
	virtual void controlKey(cocos2d::EventKeyboard::KeyCode keyCode) override;

	std::string _saveText;
	int _startIdx = 0, _combCount = 0, _mode = 0, _bufCount = 0;

	static const std::string ENG_KEY;
	static const std::u16string KOR_KEY, CHO_DATA, JUNG_DATA, JONG_DATA;
};

#endif