#include "HangulTF.h"

#include "platform/CCFileUtils.h"
#include "base/ccUTF8.h"

#include <imm.h>
#include <string>
#include <algorithm>

#pragma comment(lib, "imm32")

#define CURSOR_DEFAULT_CHAR '|'
#define PASSWORD_STYLE_TEXT_DEFAULT "\xe2\x80\xa2"

USING_NS_CC;
using namespace std;

const std::string HangulTF::ENG = "rRseEfaqQtTdwWczxvgkoiOjpuPhynbml";
const std::u16string HangulTF::CHO = u"��������������������������������������";
const std::u16string HangulTF::JUNG = u"�������¤äĤŤƤǤȤɤʤˤ̤ͤΤϤФѤҤ�";
const std::u16string HangulTF::JONG = u"������������������������������������������������������";

string toUTF8(const std::u16string & u16) {
	string ret;
	StringUtils::UTF16ToUTF8(u16, ret);
	return ret;
}

HangulTF::HangulTF() {
	_delegate = 0;
	_charCount = 0;
	_inputText = "";
	_placeHolder = "";
	_colorText = Color4B::WHITE;
	_secureTextEntry = false;
	_passwordStyleText = PASSWORD_STYLE_TEXT_DEFAULT;
	_cursorEnabled = false;
	_cursorPosition = 0;
	_cursorChar = CURSOR_DEFAULT_CHAR;
	_cursorShowingTime = 0.0f;
	_isAttachWithIME = false;
	_colorSpaceHolder.r = _colorSpaceHolder.g = _colorSpaceHolder.b = 127;
	_colorSpaceHolder.a = 255;
	ImmDisableIME(0);
}

HangulTF * HangulTF::textFieldWithPlaceHolder(const std::string& placeholder, const Size& dimensions, TextHAlignment alignment, const std::string& fontName, float fontSize) {
	HangulTF *ret = new (std::nothrow) HangulTF();
	if (ret && ret->initWithPlaceHolder("", dimensions, alignment, fontName, fontSize)) {
		ret->autorelease();
		if (placeholder.size()>0) {
			ret->setPlaceHolder(placeholder);
		}
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

HangulTF * HangulTF::textFieldWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize) {
	HangulTF *ret = new (std::nothrow) HangulTF();
	if (ret && ret->initWithPlaceHolder("", fontName, fontSize)) {
		ret->autorelease();
		if (placeholder.size()>0) {
			ret->setPlaceHolder(placeholder);
		}
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

static std::size_t _calcCharCount(const char * text) {
	int n = 0;
	char ch = 0;
	while ((ch = *text)) {
		CC_BREAK_IF(!ch);

		if (0x80 != (0xC0 & ch)) {
			++n;
		}
		++text;
	}
	return n;
}

void HangulTF::insertText(const char * text, size_t len) {
	std::string insert(text, len);

	// insert \n means input end
	int pos = static_cast<int>(insert.find((char)TextFormatter::NewLine));
	if ((int)insert.npos != pos) {
		len = pos;
		insert.erase(pos);
	}

	if (len > 0) {
		if (_delegate && _delegate->onTextFieldInsertText(this, insert.c_str(), len)) {
			// delegate doesn't want to insert text
			return;
		}

		if (_cursorEnabled) {
			StringUtils::StringUTF8 stringUTF8;



			insert = hangulAutomata(insert);

			std::size_t countInsertChar = _calcCharCount(insert.c_str());
			_charCount += countInsertChar;

			stringUTF8.replace(_inputText);
			stringUTF8.insert(_cursorPosition, insert);

			setCursorPosition(_cursorPosition + countInsertChar);

			setString(stringUTF8.getAsCharSequence());
		} else {
			std::string sText(_inputText);
			sText.append(insert);
			setString(sText);
		}
	}

	if ((int)insert.npos == pos) {
		return;
	}

	// '\n' inserted, let delegate process first
	if (_delegate && _delegate->onTextFieldInsertText(this, "\n", 1)) {
		return;
	}

	// if delegate hasn't processed, detach from IME by default
	detachWithIME();
}

void HangulTF::deleteBack() {
	size_t len = _inputText.length();
	if (!len) {
		// there is no string
		return;
	}

	// get the delete byte number
	size_t deleteLen = 1;    // default, erase 1 byte

	while (0x80 == (0xC0 & _inputText.at(len - deleteLen))) {
		++deleteLen;
	}

	if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + len - deleteLen, static_cast<int>(deleteLen))) {
		// delegate doesn't want to delete backwards
		return;
	}

	// if all text deleted, show placeholder string
	if (len <= deleteLen) {
		_inputText = "";
		_charCount = 0;
		setCursorPosition(0);
		setString(_inputText);
		return;
	}

	// set new input text
	if (_cursorEnabled) {
		if (_cursorPosition) {
			setCursorPosition(_cursorPosition - 1);

			StringUtils::StringUTF8 stringUTF8;

			stringUTF8.replace(_inputText);
			stringUTF8.deleteChar(_cursorPosition);

			_charCount = stringUTF8.length();
			setString(stringUTF8.getAsCharSequence());
		}
	} else {
		std::string text(_inputText.c_str(), len - deleteLen);
		setString(text);
	}
}

void HangulTF::deleteBackward() {
	deleteBack();
	clearState();
}

void HangulTF::controlKey(EventKeyboard::KeyCode keyCode) {
	if (_cursorEnabled) {
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_HOME:
		case EventKeyboard::KeyCode::KEY_KP_HOME:
			setCursorPosition(0);
			updateCursorDisplayText();

			clearState();
			break;
		case EventKeyboard::KeyCode::KEY_END:
			setCursorPosition(_charCount);
			updateCursorDisplayText();

			clearState();
			break;
		case EventKeyboard::KeyCode::KEY_DELETE:
		case EventKeyboard::KeyCode::KEY_KP_DELETE:
			if (_cursorPosition < (std::size_t)_charCount) {
				StringUtils::StringUTF8 stringUTF8;

				stringUTF8.replace(_inputText);
				stringUTF8.deleteChar(_cursorPosition);
				setCursorPosition(_cursorPosition);
				_charCount = stringUTF8.length();
				setString(stringUTF8.getAsCharSequence());

				clearState();
			}
			break;
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			if (_cursorPosition) {
				setCursorPosition(_cursorPosition - 1);
				updateCursorDisplayText();

				clearState();
			}
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			if (_cursorPosition < (std::size_t)_charCount) {
				setCursorPosition(_cursorPosition + 1);
				updateCursorDisplayText();

				clearState();
			}
			break;
		case EventKeyboard::KeyCode::KEY_ESCAPE:
			clearState();
			detachWithIME();
			break;
		case EventKeyboard::KeyCode::KEY_ENTER:
			clearState();
			detachWithIME();
			break;
		default:
			break;
		}
	}
}

string HangulTF::hangulAutomata(const string& str) {
	string insert = str;

	int key = ENG.find(insert);
	/// A�� S���� �Էµ��� �� �߻��ϴ� ���� ����
	if (key == -1) {
		std::transform(insert.begin(), insert.end(), insert.begin(), ::tolower);
		key = ENG.find(insert);
	}
	switch (state) {
	case 0:
		if (key <= 18) { /// ����
			cho = key;
			/// �̸�����
			insert = toUTF8(u16string(1, CHO[cho]));
			state = 1;
		} else { /// ����
			jung = calcVowel(key);
			/// �̸�����
			insert = toUTF8(u16string(1, JUNG[jung]));
			state = 2;
		}
		break;
	case 1:
		deleteBack();
		if (key <= 18) {
			insert = toUTF8(u16string(1, CHO[cho]));
			cho = key;
			/// �̸�����
			insert += toUTF8(u16string(1, CHO[cho]));

			state = 1;
		} else {
			jung = calcVowel(key);
			insert = toUTF8(combineHangul(cho, jung, jong));
			state = 3;
		}
		break;
	case 2:
		deleteBack();
		if (key <= 18) {
			insert = toUTF8(u16string(1, JUNG[jung]));
			cho = key;
			/// �̸�����
			insert += toUTF8(u16string(1, CHO[cho]));

			state = 1;
		} else {
			int tmp = calcJungComplex(key);
			if (tmp != -1) { /// ���� �߼�
				jung = tmp;
				/// �̸�����
				insert = toUTF8(u16string(1, JUNG[jung]));
				state = 4;
			} else { /// ����
				insert = toUTF8(u16string(1, JUNG[jung]));
				jung = calcVowel(key);
				/// �̸�����
				insert += toUTF8(u16string(1, JUNG[jung]));
				state = 2;
			}
		}
		break;
	case 3:
		deleteBack();
		if (key <= 18) {
			jong = calcJong(key);
			insert = toUTF8(combineHangul(cho, jung, jong));
			state = 5;
		} else {
			int tmp = calcJungComplex(key);
			if (tmp != -1) { /// ���� �߼�
				jung = tmp;
				/// �̸�����
				insert = toUTF8(combineHangul(cho, jung, jong));
				state = 6;
			} else { /// ����
				insert = toUTF8(combineHangul(cho, jung, jong));
				jung = calcVowel(key);
				/// �̸�����
				insert += toUTF8(u16string(1, JUNG[jung]));
				state = 2;
			}
		}
		break;
	case 4:
		deleteBack();
		if (key <= 18) {
			insert = toUTF8(u16string(1, JUNG[jung]));
			cho = key;
			/// �̸�����
			insert += toUTF8(u16string(1, CHO[cho]));
			state = 1;
		} else {
			insert = toUTF8(u16string(1, JUNG[jung]));
			jung = calcVowel(key);
			/// �̸�����
			insert += toUTF8(u16string(1, JUNG[jung]));
			state = 2;
		}
		break;
	case 5:
		deleteBack();
		if (key <= 18) {
			int tmp = calcJongComplex(key);
			if (tmp != -1) {
				jong = tmp;
				insert = toUTF8(combineHangul(cho, jung, jong));
				state = 7;
			} else {
				insert = toUTF8(combineHangul(cho, jung, jong));
				cho = key;
				jung = -1;
				jong = -1;
				/// �̸�����
				insert += toUTF8(u16string(1, CHO[cho]));
				state = 1;
			}
		} else {
			insert = toUTF8(combineHangul(cho, jung, -1));
			cho = jongToCho(jong);
			jung = calcVowel(key);
			jong = -1;
			/// �̸�����
			insert += toUTF8(combineHangul(cho, jung, -1));
			state = 3;
		}
		break;
	case 6:
		deleteBack();
		if (key <= 18) {
			jong = calcJong(key);
			/// �̸�����
			insert = toUTF8(combineHangul(cho, jung, jong));
			state = 8;
		} else {
			insert = toUTF8(combineHangul(cho, jung, jong));
			jung = calcVowel(key);
			/// �̸�����
			insert += toUTF8(u16string(1, JUNG[jung]));
			state = 2;
		}
		break;
	case 7:
		deleteBack();
		if (key <= 18) {
			insert = toUTF8(combineHangul(cho, jung, jong));
			cho = key;
			jung = -1;
			jong = -1;
			/// �̸�����
			insert += toUTF8(u16string(1, CHO[cho]));
			state = 1;
		} else {
			int tmp = splitJongComplex();
			insert = toUTF8(combineHangul(cho, jung, jong));
			cho = tmp;
			jung = calcVowel(key);
			jong = -1;
			/// �̸�����
			insert += toUTF8(combineHangul(cho, jung, jong));
			state = 3;
		}
		break;
	case 8:
		deleteBack();
		if (key <= 18) {
			int tmp = calcJongComplex(key);
			if (tmp != -1) {
				jong = tmp;
				insert = toUTF8(combineHangul(cho, jung, jong));
				state = 9;
			} else {
				insert = toUTF8(combineHangul(cho, jung, jong));
				cho = key;
				jung = -1;
				jong = -1;
				/// �̸�����
				insert += toUTF8(u16string(1, CHO[cho]));
				state = 1;
			}
		} else {
			insert = toUTF8(combineHangul(cho, jung, -1));
			cho = jongToCho(jong);
			jung = calcVowel(key);
			jong = -1;
			/// �̸�����
			insert += toUTF8(combineHangul(cho, jung, -1));
			state = 3;
		}
		break;
	case 9:
		deleteBack();
		if (key <= 18) {
			insert = toUTF8(combineHangul(cho, jung, jong));
			cho = key;
			jung = -1;
			jong = -1;
			/// �̸�����
			insert += toUTF8(u16string(1, CHO[cho]));
			state = 1;
		} else {
			int tmp = splitJongComplex();
			insert = toUTF8(combineHangul(cho, jung, jong));
			cho = tmp;
			jung = calcVowel(key);
			jong = -1;
			/// �̸�����
			insert += toUTF8(combineHangul(cho, jung, jong));
			state = 3;
		}
		break;
	}

	return insert;
}

u16string HangulTF::combineHangul(int cho, int jung, int jong) {
	u16string ret(1, 0xac00 + cho * 21 * 28 + jung * 28 + jong + 1);
	return  ret;
}

string HangulTF::toUTF8(const std::u16string & u16) {
	string ret;
	cocos2d::StringUtils::UTF16ToUTF8(u16, ret);
	return ret;
}

int HangulTF::calcVowel(int key) {
	switch (key) {
	case 28: return 12; /* �� */
	case 29: return 13; /* �� */
	case 30: return 17; /* �� */
	case 31: return 18; /* �� */
	case 32: return 20; /* �� */
	}
	return key - 19;
}

int HangulTF::calcJong(int key) {
	switch (key) {
	case 2:  return 3;  /* �� */
	case 3:  return 6;  /* �� */
	case 5:  return 7;  /* �� */
	case 6:  return 15; /* �� */
	case 7:  return 16; /* �� */
	case 9:  return 18; /* �� */
	case 10: return 19; /* �� */
	case 11: return 20; /* �� */
	case 12: return 21; /* �� */
	case 14: return 22; /* �� */
	case 15: return 23; /* �� */
	case 16: return 24; /* �� */
	case 17: return 25; /* �� */
	case 18: return 26; /* �� */
	}
	return key;
}

int HangulTF::jongToCho(int jong) {
	switch (jong) {
	case 3:  return 2;	/* �� */
	case 6:  return 3;	/* �� */
	case 7:  return 5;  /* �� */
	case 15: return 6;  /* �� */
	case 16: return 7;  /* �� */
	case 18: return 9;  /* �� */
	case 20: return 11;	/* �� */
	case 21: return 12; /* �� */
	case 22: return 14; /* �� */
	case 23: return 15; /* �� */
	case 24: return 16; /* �� */
	case 25: return 17; /* �� */
	case 26: return 18; /* �� */
	}
	return jong;
}

int HangulTF::splitJongComplex() {
	switch (jong) {
	case 2:	 jong = 0; return 9;	/* �� */
	case 4:	 jong = 3; return 12;	/* �� */
	case 5:	 jong = 3; return 18;	/* �� */
	case 8:	 jong = 7; return 0;	/* �� */
	case 9:	 jong = 7; return 6;	/* �� */
	case 10: jong = 7; return 7;	/* �� */
	case 11: jong = 7; return 9;	/* �� */
	case 12: jong = 7; return 16;	/* �� */
	case 13: jong = 7; return 17;	/* �� */
	case 14: jong = 7; return 18;	/* �� */
	}
	return -1;
}

int HangulTF::calcJungComplex(int key) {
	key = calcVowel(key);
	switch (jung) {
	case 8: 
		if		(key == 0)	return 9;	/* �� */
		else if (key == 1)	return 10;	/* �� */
		else if (key == 20)	return 11;	/* �� */
		break;
	case 13:
		if		(key == 4)	return 14;	/* �� */
		else if (key == 5)	return 15;	/* �� */
		else if (key == 20) return 16;	/* �� */
		break;
	case 18:
		if		(key == 20) return 19;	/* �� */
	}
	return -1;
}

int HangulTF::calcJongComplex(int key) {
	key = calcJong(key);
	switch (jong) {
	case 0:
		if		(key == 18) return 2;	/* �� */
		break;
	case 3:
		if		(key == 21) return 4;	/* �� */
		else if (key == 27) return 5;	/* �� */
		break;
	case 7:
		if		(key == 0)	return 8;	/* �� */
		else if (key == 15) return 9;	/* �� */
		else if (key == 16) return 10;	/* �� */
		else if (key == 18) return 11;	/* �� */
		else if (key == 24) return 12;	/* �� */
		else if (key == 25) return 13;	/* �� */
		else if (key == 26) return 14;	/* �� */
		break;
	}
	return -1;
}

void HangulTF::clearState() {
	state = 0;
	cho = -1;
	jung = -1;
	jong = -1;
}
