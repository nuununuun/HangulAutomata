#include "HangulTF.h"

#include "platform/CCFileUtils.h"
#include "base/ccUTF8.h"

#include "Hangul.h"

#include <imm.h>
#include <string>

#pragma comment(lib, "imm32")

#define CURSOR_DEFAULT_CHAR '|'
#define PASSWORD_STYLE_TEXT_DEFAULT "\xe2\x80\xa2"

USING_NS_CC;
using namespace std;

const std::string HangulTF::ENG_KEY = "rRseEfaqQtTdwWczxvgkoiOjpuPhynbml";
const std::u16string HangulTF::KOR_KEY = u"ぁあいぇえぉけげこさざしじすずせぜそぞただちぢっつづてでにぬばぱび";
const std::u16string HangulTF::CHO_DATA = u"ぁあいぇえぉけげこさざしじすずせぜそぞ";
const std::u16string HangulTF::JUNG_DATA = u"ただちぢっつづてでとどなにぬねのはばぱひび";
const std::u16string HangulTF::JONG_DATA = u"ぁあぃいぅうぇぉおかがきぎくぐけげごさざしじずせぜそぞ";

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

			insert = HangulAutomata::getInstance()->combine(insert);

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

void HangulTF::deleteBackward() {
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
		_saveText = "";
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

void HangulTF::controlKey(EventKeyboard::KeyCode keyCode) {
	if (_cursorEnabled) {
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_HOME:
		case EventKeyboard::KeyCode::KEY_KP_HOME:
			setCursorPosition(0);
			updateCursorDisplayText();
			break;
		case EventKeyboard::KeyCode::KEY_END:
			setCursorPosition(_charCount);
			updateCursorDisplayText();
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
			}
			break;
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			if (_cursorPosition) {
				setCursorPosition(_cursorPosition - 1);
				updateCursorDisplayText();
			}
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			if (_cursorPosition < (std::size_t)_charCount) {
				setCursorPosition(_cursorPosition + 1);
				updateCursorDisplayText();
			}
			break;
		case EventKeyboard::KeyCode::KEY_ESCAPE:
			detachWithIME();
			break;
		case EventKeyboard::KeyCode::KEY_ENTER:
			detachWithIME();
			break;
		default:
			break;
		}
	}
}