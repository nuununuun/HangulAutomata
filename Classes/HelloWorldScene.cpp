#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "ui/UIScale9Sprite.h"

#include "HangulTF.h"
#include "Hangul.h"

USING_NS_CC;
using namespace ui;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

bool HelloWorld::init()
{
    if ( !Scene::init() )
    {
        return false;
    }

	auto bg = LayerColor::create(Color4B::WHITE);
	addChild(bg);
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = visibleSize / 2;

	Scale9Sprite * textBox = Scale9Sprite::create(Rect(3, 0, 1, 32), "res/textBox.png");
	textBox->setContentSize(Size(480, 32));
	textBox->setPosition(origin.x, 300 - 32);
	textBox->setName("textBox");
	this->addChild(textBox);

	auto hangul = HangulTF::textFieldWithPlaceHolder("", "fonts/NanumGothic.ttf", 16 * 2);
	hangul->setScale(0.5f);
	hangul->setTextColor(Color4B::BLACK);
	hangul->setPosition(origin.x, 300 - 31);
	hangul->setAnchorPoint(Vec2(0.5, 0.5));
	hangul->setCursorEnabled(true);
	hangul->setName("hangul");
	this->addChild(hangul);

	auto listener = EventListenerMouse::create();
	listener->onMouseUp = [&](EventMouse *e) {
		Vec2 v = Vec2(e->getCursorX(), e->getCursorY());

		if (getChildByName("textBox")->getBoundingBox().containsPoint(v)) {
			getChildByName<HangulTF*>("hangul")->attachWithIME();
		}
	};

	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}