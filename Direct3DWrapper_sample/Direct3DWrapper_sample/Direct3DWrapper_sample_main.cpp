//////////////////////////////////Common/Direct3DWrapper sample////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include"../../../Common/Direct3DWrapper/Dx12Process.h"
#include"../../../Common/Direct3DWrapperOption/DxText.h"
#include"../../../Common/Window/Win.h"
#include"../../../JPGLoader/JPGLoader.h"
#include"../../../PNGLoader/PNGLoader.h"
#include"../../../Common/SearchFile/SearchFile.h"
#include"../../../UserInterface/UserInterface.h"
#include"../../../TIFLoader/TIFLoader.h"
#include"../../../CreateGeometry/CreateGeometry.h"

#define CURRWIDTH 1024
#define CURRHEIGHT 768

using namespace CoordTf;

#include <vector>
#include <memory>

//テクスチャ読み込み
void createTexture(Dx_TextureHolder* dx) {
	SearchFile* sf = new SearchFile(1);
	char** strE = new char* [3];
	strE[0] = "jpg";
	strE[1] = "png";
	strE[2] = "tif";
	sf->Search(L"./tex/*", 0, strE, 3);//指定ディレクトリ内からファイル名取得
	UINT numFile1 = sf->GetFileNum(0);
	JPGLoader jpg;
	PNGLoader png;
	TIFLoader tif;
	UINT resCnt = 0;
	ARR_DELETE(strE);
	for (int i = 0; i < sf->GetFileNum(0); i++) {
		char* str = sf->GetFileName(0, i);
		UCHAR* byte = jpg.loadJPG(str, 0, 0, nullptr);
		unsigned int w = jpg.getSrcWidth();//テクスチャの元のサイズ
		unsigned int h = jpg.getSrcHeight();
		if (byte == nullptr) {
			byte = png.loadPNG(str, 0, 0, nullptr);
			w = png.getSrcWidth();
			h = png.getSrcHeight();
		}
		if (byte == nullptr) {
			byte = jpg.loadJPG(str, 0, 0, nullptr);
			w = jpg.getSrcWidth();
			h = jpg.getSrcHeight();
		}
		//展開したテクスチャの登録, ファイル名で関連付け
		//GetNameFromPassでファイル名のみ取得出来る
		dx->createTextureArr(numFile1, resCnt++, Dx_Util::GetNameFromPass(str),
			byte, DXGI_FORMAT_R8G8B8A8_UNORM,
			w, w * 4, h);
		ARR_DELETE(byte);
	}
	S_DELETE(sf);
}

static Vertex ver4[] =
{
	{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} ,{0.0f,0.0f}},
	{ {1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} ,{1.0f,0.0f}},
	{ {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} ,{1.0f,1.0f}},
	{ {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} ,{0.0f,1.0f}},
};

static UINT index6[] =
{
	0,1,2,
	0,2,3
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	srand((unsigned)time(NULL));

	HWND hWnd;
	MSG msg;

	if (Createwindow(&hWnd, hInstance, nCmdShow, CURRWIDTH, CURRHEIGHT, L"Common/Direct3DWrapper sample") == -1)return FALSE;

	Dx_Device::InstanceCreate();
	Dx_Device::GetInstance()->createDevice();
	Dx_Device::GetInstance()->reportLiveDeviceObjectsOn();
	Dx_CommandManager::InstanceCreate();
	Dx_SwapChain::InstanceCreate();

	Dx_TextureHolder::InstanceCreate();
	Dx_TextureHolder* dx = Dx_TextureHolder::GetInstance();
	
	Control* con = Control::GetInstance();
	Dx_Device* dev = Dx_Device::GetInstance();
	dev->dxrCreateResource();
	Dx_SwapChain* sw = Dx_SwapChain::GetInstance();

	Dx_CommandManager::setNumResourceBarrier(1024);

	createTexture(dx);

	sw->Initialize(hWnd, CURRWIDTH, CURRHEIGHT);

	sw->setPerspectiveFov(55, 1, 10000);
	Dx_Light::Initialize();
	Dx_ShaderHolder::CreateShaderByteCode();

	Dx_Light::setGlobalAmbientLight(0.2f, 0.2f, 0.2f);//アンビエント(ラスタライザ, DXR共用)
	DxInput* di = DxInput::GetInstance();
	di->create(hWnd);
	di->SetWindowMode(true);
	di->setCorrectionX(1.015f);
	di->setCorrectionY(1.055f);

	//文字入力
	DxText::InstanceCreate();
	DxText* text = DxText::GetInstance();

	Dx_CommandManager* cMa = Dx_CommandManager::GetInstance();
	Dx_CommandListObj* cObj = cMa->getGraphicsComListObj(0);
	Dx_CommandListObj* cObj1 = cMa->getGraphicsComListObj(1);
	Dx_CommandListObj* cObj2 = cMa->getGraphicsComListObj(2);
	Dx_CommandListObj* cObjCom = cMa->getComputeComListObj(0);

	PolygonData2D* d2;
	d2 = new PolygonData2D();
	d2->GetVBarray2D(1);

	PolygonData* pd[4];//手打ちの頂点使用
	pd[0] = new PolygonData();
	pd[0]->GetVBarray(SQUARE, 1);
	pd[1] = new PolygonData();
	pd[1]->GetVBarray(SQUARE, 1);
	pd[2] = new PolygonData();
	pd[2]->GetVBarray(SQUARE, 1);
	pd[3] = new PolygonData();
	pd[3]->GetVBarray(SQUARE, 1);

	PolygonData* light;//DXR光源用メッシュ
	light = new PolygonData();
	light->GetVBarray(SQUARE, 1);

	VECTOR3 v3[] = { {} };
	VECTOR3 v3s[] = { {1,1,1} };
	Vertex* v = (Vertex*)CreateGeometry::createCube(1, v3, v3s, false);
	Vertex* vr = (Vertex*)CreateGeometry::createCube(1, v3, v3s, true);
	unsigned int* ind = CreateGeometry::createCubeIndex(1);

	Vertex* sv = (Vertex*)CreateGeometry::createSphere(10, 10, 1, v3, v3s, false);
	unsigned int* svI = CreateGeometry::createSphereIndex(10, 10, 1);

	pd[0]->setVertex(v, 24, ind, 36);
	pd[1]->setVertex(vr, 24, ind, 36);
	pd[2]->setVertex(ver4, 4, index6, 6);
	pd[3]->setVertex(sv, 11 * 11, svI, 10 * 10 * 6);
	light->setVertex(v, 24, ind, 36);

	//スキンメッシュFBXのみ
	//使用するテクスチャは事前にcreateTextureArrで登録しておく
	//内部でfbx読み込み時にファイル名からテクスチャを読み込む
	SkinMesh* sk;
	sk = new SkinMesh();
	sk->SetState(true, true);
	sk->GetFbx("player1_fbx_att.fbx");
	sk->GetBuffer(1, sk->getMaxEndframe(0, 0));
	sk->SetVertex(true, true);

	UserInterfaceWindow* uw;
	uw = new UserInterfaceWindow();
	uw->setNumWindow(1);
	
	//↓各メッシュ生成, コマンドリスト必要
	cObj->Bigin();
	d2->CreateBox(0, 0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 1.0f, 1.0f, 1.0f, 1.0f, true, true);
	pd[0]->Create(0, true, dx->GetTexNumber("wall1.jpg"),
		dx->GetTexNumber("wall1Nor.png"),
		dx->GetTexNumber("wall1.jpg"), false, false, false);
	pd[1]->Create(0, true, dx->GetTexNumber("wall1.jpg"),
		dx->GetTexNumber("wall1Nor.png"),
		dx->GetTexNumber("wall1.jpg"), false, false, false);
	pd[2]->setMaterialType(METALLIC);//DXR用マテリアル設定
	pd[2]->Create(0, true, dx->GetTexNumber("wave.jpg"),
		-1,
		-1, true, true, false);
	pd[3]->setMaterialType(METALLIC);
	pd[3]->Create(0, true, dx->GetTexNumber("siro.png"),
		-1,
		-1, true, true, false);
	light->setMaterialType(EMISSIVE);
	light->Create(0, false, dx->GetTexNumber("siro.png"),
		-1,
		-1, false, false, false);

	sk->CreateFromFBX(0);

	uw->create(0, 15);//comIndexは0固定・・
	char* str[2];
	char* st0 = "レイトレON";
	char* st1 = "レイトレOFF";
	str[0] = st0;
	str[1] = st1;
	uw->setMenuName(0, 2, str);

	cObj->End();
	cMa->RunGpu();
	cMa->WaitFence();

	ARR_DELETE(v);
	ARR_DELETE(vr);
	ARR_DELETE(ind);
	ARR_DELETE(sv);
	ARR_DELETE(svI);

	//DXR描画用
	DxrRenderer* dxr;
	dxr = new DxrRenderer();

	//DXRで使用するデータを読み込み, 登録する
	int numMesh = sk->getNumMesh();
	std::vector<ParameterDXR*> pdx;
	for (int i = 0; i < numMesh; i++) {
		pdx.push_back(sk->getParameter(i));
	}
	for (int i = 0; i < 4; i++) {
		pdx.push_back(pd[i]->getParameter());
	}
	pdx.push_back(light->getParameter());

	dxr->initDXR(pdx, 9);//初期化,登録

	float lightTheta = 0.0f;
	float cubeTheta = 0.0f;
	bool ray = false;

	while (1) {
		if (!DispatchMSG(&msg)) {
			break;
		}

		//各CBがダブルバッファになっている
		//ダブルバッファ使用するときは毎回実行
		//ダブルバッファをスワップしてる
		dxr->allSwapIndex();

		MATRIX camThetaZ;
		VECTOR3 cam1 = { 0, -90, 20 };
		MatrixRotationZ(&camThetaZ, 0.0f);
		VectorMatrixMultiply(&cam1, &camThetaZ);

		sw->Cameraset({ cam1.x, cam1.y, cam1.z }, { 0, 0, 0 });

		MATRIX thetaZ;
		VECTOR3 lightPos = { 0, -35, 15 };
		MatrixRotationZ(&thetaZ, lightTheta);
		if (360.0f < (lightTheta += 0.2f))lightTheta = 0.0f;
		VectorMatrixMultiply(&lightPos, &thetaZ);

		//ラスタライザ用光源のアップデート
		Dx_Light::PointLightPosSet(0,
			lightPos,
			{ 1, 1, 1, 1 }, true, 1000);

		//各バッファ更新

		//DXR用光源
		light->Instancing(lightPos,
			{ 0, 0, 0 },
			{3, 3, 3 }, { 0, 0, 0, 0 });
		light->InstancingUpdate(0.2f);
		light->setPointLight(0, true, 1000);

		d2->Update(0, 0, 0,
			1.0f, 1.0f, 1.0f, 1.0f, 50, 50);

		pd[0]->Instancing({18, 0, 10},
			{ 0, 0, cubeTheta },
			{ 7, 7, 7 }, { 0, 0, 0, 0 });
		pd[0]->InstancingUpdate(0.2f);
		if (360.0f < (cubeTheta += 0.1f))cubeTheta = 0.0f;

		pd[1]->Instancing({ 0, 0, 25 },
			{ 0, 0, 0 },
			{ 40, 40, 30 }, { 0, 0, 0, 0 });
		pd[1]->InstancingUpdate(0.2f);

		pd[2]->Instancing({ 0, 0, 0 },
			{ 0, 0, 0 },
			{ 40, 40, 40 }, { 0, 0, 0, -0.2f });
		pd[2]->InstancingUpdate(0.2f);

		pd[3]->Instancing({ 0, 0, 10 },
			{ 0, 0, 0 },
			{ 7, 7, 7 }, { 0, 0, 0, 0 });
		pd[3]->InstancingUpdate(0.2f);

		sk->Update(0, 0.1f, { -15,0,15 }, { 0,0,0,0 }, { -90,0,0 }, { 2.0f,2.0f,2.0f }, 0);

		int rF = uw->updatePos(0, 200, 100);
		if (rF == 0)ray = true;
		if (rF == 1)ray = false;

		text->UpDateText("↑↓で選択『Ctrl』で決定   ", 200, 85, 15, {1.0f, 1.0f, 1.0f, 1.0f});
		text->UpDate();//テキストのアップデートの最後に実行

		//コマンドリスト実行
		if (!ray) {
			//ラスタライザ
			cObj->Bigin();
			sw->BiginDraw(0);//ラスタライザ描画有りの場合実行必要
			d2->Draw(0);
			light->Draw(0);
			pd[0]->Draw(0);
			pd[1]->Draw(0);
			sk->Draw(0);//透けさせたい場合先に描画
			pd[2]->Draw(0);//半透明
			pd[3]->Draw(0);
			uw->Draw(0, 0);
			text->Draw(0);
			sw->EndDraw(0);
			cObj->End();
			cMa->RunGpu();
			cMa->WaitFence();
			sw->DrawScreen();
		}
		else {
			//DXR

			//DXRに送るデータのアウトプット
			cObj->Bigin();
			light->StreamOutput(0);
			pd[0]->StreamOutput(0);
			pd[1]->StreamOutput(0);
			sk->StreamOutput(0);
			pd[2]->StreamOutput(0);
			pd[3]->StreamOutput(0);
			cObj->End();
			cMa->RunGpu();

			//AS更新
			cObj1->Bigin();
			dxr->update_g(1, 4);
			cObj1->End();
			cMa->RunGpu();

			//レイトレ実行
			cObjCom->Bigin();
			dxr->raytrace_c(0);
			cObjCom->End();
			cMa->RunGpuCom();
			
			//レイトレ後のバッファコピー
			cObj2->Bigin();
			dxr->copyBackBuffer(2);
			dxr->copyDepthBuffer(2);
			cObj2->End();
			cMa->RunGpu();

			cMa->WaitFenceCom();
			cMa->WaitFence();

			//レイトレ後のラスタライザ描画
			cObj->Bigin();
			sw->BiginDraw(0, false);
			d2->Draw(0);
			uw->Draw(0, 0);
			text->Draw(0);
			sw->EndDraw(0);
			cObj->End();
			cMa->RunGpu();
			cMa->WaitFence();
			sw->DrawScreen();
		}
	}

	cMa->WaitFence();
	cMa->WaitFenceCom();

	S_DELETE(light);
	S_DELETE(uw);
	S_DELETE(d2);
	S_DELETE(pd[0]);
	S_DELETE(pd[1]);
	S_DELETE(pd[2]);
	S_DELETE(pd[3]);
	S_DELETE(sk);
	S_DELETE(dxr);
	DxInput::DeleteInstance();
	Control::DeleteInstance();
	DxText::DeleteInstance();
	Dx_SwapChain::DeleteInstance();
	Dx_TextureHolder::DeleteInstance();
	Dx_CommandManager::DeleteInstance();
	Dx_Device::DeleteInstance();
	return 0;
}