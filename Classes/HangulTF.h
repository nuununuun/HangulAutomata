#ifndef _HANGULTF_H_
#define _HANGULTF_H_

#include "2d/CCTextFieldTTF.h"

class HangulTF : public cocos2d::TextFieldTTF {
public:
	HangulTF();

	static HangulTF * textFieldWithPlaceHolder(const std::string& placeholder, const cocos2d::Size& dimensions, cocos2d::TextHAlignment alignment, const std::string& fontName, float fontSize);
	static HangulTF * textFieldWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize);

protected:
	virtual void insertText(const char * text, size_t len) override;
	virtual void deleteBackward() override;
	virtual void controlKey(cocos2d::EventKeyboard::KeyCode keyCode) override;

	std::string hangulAutomata(const std::string& str);

	void deleteBack();

	std::string toUTF8(const std::u16string& u16);
	std::u16string combineHangul(int cho, int jung, int jong);
	int calcVowel(int key);
	int calcJong(int key);
	int jongToCho(int jong);
	int splitJungComplex();
	int splitJongComplex();
	int calcJungComplex(int key);
	int calcJongComplex(int key);
	void clearState();

	static const std::string ENG, ENG_JUNG, ENG_JONG;
	static const std::u16string CHO, JUNG, JONG;

	bool hangulMode = true;

	int state = 0, cho = -1, jung = -1, jong = -1;
};

#endif