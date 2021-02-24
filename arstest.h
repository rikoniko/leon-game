#pragma once

#ifndef _DEBUG
#pragma comment(lib, "../lib/ARS.lib")
#pragma comment(lib, "../lib/WML.lib")
#else
#pragma comment(lib, "../lib/ARSd.lib")
#pragma comment(lib, "../lib/WMLd.lib")
#endif

#include <string>

#define _CRT_SECURE_NO_WARNINGS

#define CLASSNAME "ARSTEST"
#define APPNAME "ARSTEST"

//跳ね返るクラス
class Bounce : public Reaction{
public:
	//targetが跳ね返る
	Bounce(Movable* target) : Reaction{ target }{}
	void onBegin(Event* ev) override;
};

class Reflect : public Reaction{
	bool orientation; //true:vertical, false:horizontal
public:
	Reflect(Movable* target, bool ori) :Reaction{ target }, orientation{ ori }{}
	void onBegin(Event*) override;
};

//レオンのクラス：画像は２D、動く
class Leon : public Texture2D, public Movable {
	//触れたかどうかの判定
	Reflect reflect_h{ this ,false };
	Reflect reflect_v{ this ,true };
public:
	Leon(ARSG* _g, wchar_t fln[])
		:Texture2D{ _g, fln }
		//レオンは画面をx座標だけ移動する
		, Movable{ 2.5, 0, 0 }
	{ }
	void move() override;
};

//２D画像（レオン、ご飯、スタート画面、成功・失敗画面）のクラス（
class Plate2D :public Texture2D {
public:
	Plate2D(ARSG* _g, wchar_t fln[])
		:Texture2D{ _g,fln }
	{}
};

//邪魔をするワンちゃんのクラス：画像は３D、動く
class Jama : public Texture3D, public Movable{
	//プレイヤーと邪魔をするワンちゃんが触れた
	Touchability touchability;
	//邪魔をするワンちゃんが触れた時に跳ね返る
	Bounce bounce{ this };
	//触れたかどうかの判定
	Reflect reflect_h{ this ,false };
	Reflect reflect_v{ this ,true };
	//邪魔をするワンちゃんが左右の画面に触れたら反射する
	ReflectivitySide side_reflective{this,{ &reflect_h } };
	//邪魔をするワンちゃんが床（画面下）に触れたら反射する
	ReflectivityGround ground_reflective{ this,{ &reflect_v } };
public:
	Jama(ARSG* _g, wchar_t fln[], Texture* hA, unsigned int threshold)
		:Texture3D{ _g, fln }
		, Movable{ 0.0f, 0.0f, 0.0f }
		//プレイヤーと邪魔をするワンちゃんが触れた時に跳ね返る
		, touchability{ this, hA, threshold,{ &bounce} }
	{ }
	
	void move() override;
};











