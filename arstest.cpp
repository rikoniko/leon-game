#include <tchar.h>
#include <windows.h>
#define D3D_DEBUG_INFO
#include <stdlib.h>
#include <math.h>
#include <d3dx9.h>
#include <XAudio2.h>
#include <vector>
#include <list>
#include <iostream>
#include <time.h>

#include "../include/WindowManager.h"
#include "../include/ars.h"
#include "reactive.h"
#include "touchable.h"
#include "reflective.h"
#include "arstest.h"



//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------

using namespace std;
int limit = 10;
void subtract_mask(Texture* result, Texture* bg, Texture* src, DWORD border);

time_t s_time, e_time, n_time;
float lx, ly, lz,fx,fy,fz,mx,my,mz;
double back = 0;

UINT MainLoop(WindowManager *winmgr)
{
	ShowDebugWindow();

	Window window;
	winmgr->RegisterWindow(&window);

	ARSG g{ window.hWnd, sizex, sizey, true };
	g.SetBackgroundColor(255, 0, 0, 0);

	Light light{ &g };
	g.Register(&light);

	ARSD d;
	d.Init();
	d.AttachCam(0);
	d.StartGraph();

	Texture hitArea{ &g, sizex, sizey };
	Texture stored{ &g, sizex, sizey };
	Texture2D source{ &g, sizex, sizey };

	g.Register(&source);

	//ご飯の大きさと位置の設定
	Plate2D food{ &g,L"Food.PNG" };
	food.SetPosition(400, 100, 0, GL_ABSOLUTE);
	food.SetScale(0.2f, 0.2f, 0.2f);

	//レオンの大きさと位置の設定
	Leon leon{ &g,L"Leon.PNG" };
	leon.SetScale(0.2f, 0.2f, 0.2f);
	leon.SetPosition(0.0f, 100, 0.0f, GL_ABSOLUTE);

	//邪魔をするワンちゃんの大きさと位置の設定
	Jama jama{ &g,L"Maron.PNG" , &hitArea,100 };
	jama.SetScale(10, 10, 10);
	jama.SetPosition(0.0f, 6.0f, 0, GL_ABSOLUTE);

	//ご飯、レオン、邪魔をするワンちゃんの順で画面に表示
	g.Register(&food);
	g.Register(&leon);
	g.Register(&jama);

	InputHandler *keyIn = window.GetInputHandler();

	//スタート画面の大きさと位置の設定
	Plate2D start{ &g,L"start.JPG" };
	start.SetScale(0.38f, 0.34f, 0);
	start.SetPosition(0.0f, 0.0f, 0.0f, GL_ABSOLUTE);

	//失敗画面の大きさと位置の設定
	Plate2D gameover{ &g,L"Gameover.JPG" };
	gameover.SetScale(0.35f, 0.3f, 0);
	gameover.SetPosition(0.0f, 0.0f, 0.0f, GL_ABSOLUTE);

	//成功画面の大きさと位置の設定
	Plate2D complete{ &g,L"Complete.JPG" };
	complete.SetScale(0.35f, 0.34f, 0);
	complete.SetPosition(0.0f, 0.0f, 0.0f, GL_ABSOLUTE);

	//カウントダウンの画像
	Plate2D limited[11]{
		{ &g,L"Limit_0.PNG" },
		{ &g,L"Limit_1.PNG" },
		{ &g,L"Limit_2.PNG" },
		{ &g,L"Limit_3.PNG" },
		{ &g,L"Limit_4.PNG" },
		{ &g,L"Limit_5.PNG" },
		{ &g,L"Limit_6.PNG" },
		{ &g,L"Limit_7.PNG" },
		{ &g,L"Limit_8.PNG" },
		{ &g,L"Limit_9.PNG" },
		{ &g,L"Limit_10.PNG" } };
	//カウントダウンの画像の大きさと位置の設定
	for (int i = 0; i < 11; i++) {
		limited[i].SetScale(0.15f, 0.15f, 0.15f);
		limited[i].SetPosition(480, -20, 0, GL_ABSOLUTE);
	}

	while (!d.GetCamImage(&stored));

	//最初にスタート画面を表示
	while (1) {
		d.GetCamImage(&source);
		g.Draw();
		g.Register(&start);
		
		//コントロール＋Sキーでスタート
		if (keyIn->GetKeyTrig('S')) {
			d.GetCamImage(&stored);
			g.Unregister(&start);
			break;
		}
	}

	int success = 0;
	time(&s_time);   //ゲームの開始時の時刻を取得
    e_time = s_time + limit;    //開始時刻に10を加える

	while(!winmgr->WaitingForTermination() ) {
		//コントロール＋Aキーで画面クリア
		if (keyIn->GetKeyTrig('A'))
			d.GetCamImage(&stored);
		d.GetCamImage(&source);
		//コントロール＋Qキーで画面閉じる
		if (keyIn->GetKeyTrig('Q')) break;

			subtract_mask(&hitArea, &stored, &source, 0x20202020);

			if (success) {
				g.Register(&complete);//成功画像の登録
			}else {
				//レオン、ご飯、邪魔をするワンちゃんの画像
				leon.GetPosition(&lx, &ly, &lz);
				food.GetPosition(&fx, &fy, &fz);
				jama.GetPosition(&mx, &my, &mz);

				time(&n_time);//ゲーム中の時刻を取得
        		int c = e_time - n_time;//ゲーム終了までの時間を求める
				int store = c + 1;//表示した時間の一つ前の時間を取得
				g.Unregister(&limited[store]);//一つ前の時間を非表示

				//10秒以内で、ゲームがまだ成功していないときに実行
				if ((e_time + 1 > n_time) && (lx + 20 < fx - 40)) {
					g.Register(&limited[c]);//カウントダウンの画像を登録
				}
				
				Movable::move_all();
				Reactivity::all_react();

				//レオンとマロンが同じ領域にいるときに実行
				if (mx > -2 && lx > 120 && my<4 && my>-4) {
					back = lx - 130.0;//現在のx座標から130だけ引いた値を設定
					leon.SetPosition(back, 100.0, 0.0f, GL_ABSOLUTE);//レオンが左に戻る
				}
				
				//時間内にご飯にたどり着けなかったときに実行（失敗）
				if ((c < 0) && (lx + 20 < fx - 40)) {
					g.Register(&gameover);//失敗画面を登録
				}

				//時間にご飯いたどり着くことができたとき実行（成功）
				if ((0 < c) && (fx - 40 < lx + 20)) {
					//成功
					success = 1;
				}
			}
		//登録した順番に画像を表示する
		g.Draw();
	}
	
	d.StopGraph();
	return 0;
}


inline void subtract_mask(Texture* result, Texture* backgrnd, Texture* src, DWORD border)
{
	ARSC::diff(result,backgrnd,src,border);
	ARSC::monochrome(result,result);
	ARSC::thresholding(result,result,border);
}

inline void Bounce::onBegin(Event* p)
{	
	SetVelocity(		
		(p->x - static_cast<TouchEvent*>(p)->gx) * 0.02f,
		(-(p->y - static_cast<TouchEvent*>(p)->gy)) * 0.02f,
		0.0f );
}


inline void Reflect::onBegin(Event*)
{
	(orientation ? target->velocity.y : target->velocity.x) *= -1.0f;
}

void Leon::move()
{
	Movable::move();
}

int APIENTRY _tWinMain(
	HINSTANCE hInstance
	, 	HINSTANCE // hPrevInstance
	, 	LPTSTR // lpCmdLine
	, 	int // nCmdShow
)
{
	WindowManager program(hInstance, &MainLoop);
#ifdef DEBUG
    MessageBox(NULL,L"OK?",TEXT(APPNAME), NULL);
#endif
    return 0;
}



void Jama::move()
{
	VECTOR2D c = getPosition2D();

	
	if (c.y > sizey - 50 && velocity.y<0.1f)
		velocity.y = 0.2f;
	else
		velocity.y -= 0.01f;


	//��C��R
	velocity.x *= 0.8f;
	velocity.y *= 0.8f;

	Movable::move();
}
