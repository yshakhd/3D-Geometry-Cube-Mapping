#define _USE_MATH_DEFINES
#include "scene.h"
#include <cmath>
#include "V3.h"
#include "M33.h"
#include "tm.h"
#include <cstdio>

Scene *scene;

using namespace std;

#include <iostream>
#include <fstream>

Scene::Scene() {
	gui = new GUI();
	gui->show();

	int u0 = 10;
	int v0 = 20;
	int h = 512;
	int w = 512;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("1st person");
	fb->show();
	fb->SetBGR(0xFF0000FF);
	fb->redraw();

	fb3 = new FrameBuffer(u0, v0, w, h);
	fb3->position(u0 + fb->w + u0, v0);
	fb3->label("3rd person");
	fb3->show();
	fb3->SetBGR(0xFFFF0000);
	fb3->redraw();

	float hfov = 60.0f;
	ppc = new PPC(hfov, w, h);
	ppc3 = new PPC(hfov, w, h);
	ppc3->C = V3(200.0f, 200.0f, 200.0f);

	tmsN = 1;
	tms = new TM[tmsN];
	tms[0].LoadBin("geometry/teapot1k.bin");
	tms[0].SM = 0;
	/*tms[4].LoadBin("geometry/teapot1k.bin");
	tms[4].SM = 1;
	tms[4].SetCenter(V3(0.0f, 0.0f, -225.0f));*/

	/*tms[1].LoadBin("geometry/tree1.bin");
	tms[1].SetCenter(V3(46.0f, 40.0f, -225.0f));
	tms[1].Scale(5.f);

	tms[2].SetFloor();*/

	lightppc = new PPC(hfov * 2.f, 600, 400);
	V3 lightpos = V3(0.f, 100.0f, -100.f);
	//lightppc->PositionAndOrient(lightpos, (tms[0].GetCenter() + tms[1].GetCenter()) / 2, V3(0.f, 1.0f, 0.f));
	
	/*for (int i = 0; i < lightppc->w * lightppc->h; i++)
		shmap[i] = 0;
	ComputeShmap();*/

	ppc3->PositionAndOrient(ppc3->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	
	envmap = new CubeMap(V3(0.0f, 0.0f, 0.0f), w);
	envmap->LoadFaces("uffizi_crossUP.tiff", "uffizi_crossLEFT.tiff", "uffizi_crossFRONT.tiff", "uffizi_crossRIGHT.tiff", "uffizi_crossDOWN.tiff", "uffizi_crossBACK.tiff");

	gui->uiw->position(u0, v0 + fb->h + v0);

}

void Scene::AMB() {
	return;
}
void Scene::SPEC() {
	return;
}

void Scene::SM1() {
	for (int i = 0; i < tmsN; i++)
		tms[i].SM = 1;
	while (1) {
		for (int i = 0; i < tmsN; i++) {
			for (int vi = 0; vi < tms[i].vertsN; vi++) {
				tms[i].cols[vi] = tms[i].og_cols[vi];
			}
			tms[i].SM = 1;
		}
		Render();
		Fl::check();
	}
	return;
}

void Scene::SM2() {
	for (int i = 0; i < tmsN; i++)
		tms[i].SM = 2;

	while (1) {
		Render();
		Fl::check();
	}
	return;
}

void Scene::SM3() {
	
	for(int i = 0; i < tmsN; i++)
		tms[i].SM = 3;

	while (1) {
		Render();
		Fl::check();
	}
	return;
}

void Scene::ClearShmap() {
	for (int i = 0; i < 512 * 512; i++)
		shmap[i] = 0;
}

void Scene::ComputeShmap() {
	ClearShmap();

	for (int tmi = 0; tmi < tmsN; tmi++) {
		for (int tri = 0; tri < tms[tmi].trisN; tri++) {
			V3 v0 = (&(tms[tmi]))->verts[(&(tms[tmi]))->tris[3 * tri]];
			V3 v1 = (&(tms[tmi]))->verts[(&(tms[tmi]))->tris[3 * tri + 1]];
			V3 v2 = (&(tms[tmi]))->verts[(&(tms[tmi]))->tris[3 * tri + 2]];

			V3 p0, p1, p2;

			if (!lightppc->Project(v0, p0))
				continue;
			if (!lightppc->Project(v1, p1))
				continue;
			if (!lightppc->Project(v2, p2))
				continue;

			int umin = (int)(min(min(p0[0], p1[0]), p2[0]) - 0.5);
			int umax = (int)(max(max(p0[0], p1[0]), p2[0]) + 0.5);
			int vmin = (int)(min(min(p0[1], p1[1]), p2[1]) - 0.5);
			int vmax = (int)(max(max(p0[1], p1[1]), p2[1]) + 0.5);

			
			if ((umax < 0) || (vmax < 0) || (umin > lightppc->w) || (vmin > lightppc->h))
				return;
			if (umin < 0)
				umin = 0;
			if (vmin < 0)
				vmin = 0;
			if (umax >= lightppc->w)
				umax = lightppc->w - 1;
			if (vmax >= lightppc->h)
				vmax = lightppc->h - 1;

			V3 edge1(p1[1] - p0[1], p0[0] - p1[0], -p0[0] * p1[1] + p0[1] * p1[0]);
			float sidedness = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
			if (sidedness < 0) {
				edge1[0] = -edge1[0];
				edge1[1] = -edge1[1];
				edge1[2] = -edge1[2];
			}

			V3 edge2(p2[1] - p1[1], p1[0] - p2[0], -p1[0] * p2[1] + p1[1] * p2[0]);
			sidedness = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
			if (sidedness < 0) {
				edge2[0] = -edge2[0];
				edge2[1] = -edge2[1];
				edge2[2] = -edge2[2];
			}

			V3 edge3(p2[1] - p0[1], p0[0] - p2[0], -p0[0] * p2[1] + p0[1] * p2[0]);
			sidedness = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];
			if (sidedness < 0) {
				edge3[0] = -edge3[0];
				edge3[1] = -edge3[1];
				edge3[2] = -edge3[2];
			}

			float edge1_p2 = edge1[0] * p2[0] + edge1[1] * p2[1] + edge1[2];
			float edge2_p0 = edge2[0] * p0[0] + edge2[1] * p0[1] + edge2[2];
			float edge3_p1 = edge3[0] * p1[0] + edge3[1] * p1[1] + edge3[2];

			M33 mat = M33(p0, p1, p2);
			mat[0][2] = 1;
			mat[1][2] = 1;
			mat[2][2] = 1;

			M33 invmat = mat.Inverted();
			V3 z_abc = invmat * V3(p0[2], p1[2], p2[2]);

			for (int v = vmin; v <= vmax; v++) {
				for (int u = umin; u <= umax; u++) {
					float edge1_cp = edge1[0] * u + edge1[1] * v + edge1[2];
					float edge2_cp = edge2[0] * u + edge2[1] * v + edge2[2];
					float edge3_cp = edge3[0] * u + edge3[1] * v + edge3[2];

					if ((edge1_p2 * edge1_cp > 0) && (edge2_p0 * edge2_cp > 0) && (edge3_p1 * edge3_cp > 0)) {
						float z_depth = V3(u, v, 1) * z_abc;
						if (z_depth > shmap[(lightppc->h - v - 1) * lightppc->w + u]) {
							shmap[(lightppc->h - v - 1) * lightppc->w + u] = z_depth;
						}
					}
				}
			}
		}
	}
}

void Scene::Render(PPC* rppc, FrameBuffer* rfb) {
	rfb->SetBGR(0xFFFFFFFF);
	rfb->SetZB(0.0f);
	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (tms[tmi].SM == 3)
			tms[tmi].RenderFilledTrisSM3(rppc, rfb, lightppc, shmap);
		else if (tms[tmi].SM == 2) {
			tms[tmi].Lightpoint(V3(1.0f, 0.0f, 0.0f), scene->ka, floor(scene->sa), lightppc->C, lightppc, shmap);
			tms[tmi].RenderFilledTrisSM1_2(rppc, rfb);
		}
		else if (tms[tmi].SM == 1)
			tms[tmi].RenderFilledTrisSM1_2(rppc, rfb);
		else
			tms[tmi].RenderFilledTrisCM(rppc, rfb, envmap);
	}
	rfb->RenderCM(rppc, envmap);
	rfb->redraw();
}

void Scene::Render() {
	// first person
	Render(ppc, fb);
	// third person
	Render(ppc3, fb3);
	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (tms[tmi].SM > 1) {
			fb3->Render3DCircle(ppc3, lightppc->C, V3(0.75f, 0.8f, 0.f), 7.f);
			fb->Render3DCircle(ppc, lightppc->C, V3(0.75f, 0.8f, 0.f), 7.f);
		}
	}

	fb3->RenderCamera(ppc, ppc3);
	fb3->redraw();
}

void Scene::TilingMode() {
	for (int i = 0; i < tmsN; i++) {
		if (tms[i].tilm == 1)
			tms[i].tilm = 2;
		else if (tms[i].tilm == 2)
			tms[i].tilm = 1;
	}
	Render();
	Fl::check();
	return;
}

void Scene::DBG() {
	{
		ppc->PositionAndOrient(V3(0.f, 20.f, -200.f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
		
		while (1) {
			Render();
			Fl::check();
		}
		
		return;
	}
	{
		FrameBuffer* sidewall = new FrameBuffer(100, 100, 128, 64);
		sidewall->LoadTiff("walldesign.tiff");
		FrameBuffer* backwall = new FrameBuffer(100, 100, 128, 64);
		backwall->LoadTiff("kim.tiff");
		FrameBuffer* floor = new FrameBuffer(100, 100, 128, 64);
		floor->LoadTiff("carpet.tiff");
		FrameBuffer* subject = new FrameBuffer(100, 100, 128, 64);
		subject->LoadTiff("bunk.tiff");


		//texture->SetChecker(0xFF000000, 0xFFDDDDDD, 16);
		ppc->PositionAndOrient(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 0.0f, -100.0f), V3(0.0f, 1.0f, 0.0f));
		float rw = 200.0f;
		float rh = 100.0f;
		tms[0].SetTexturedRectangle(rw, rh, 1.0f, 2.0f);
		tms[0].Translate(V3(0.0f, 0.0f, -200.0f));
		tms[0].Rotate(tms[0].GetCenter(), V3(1.0f, 0.0f, 0.0f), 90.0f);
		tms[0].Translate(V3(0.0f, -rh / 2.f, -rw / 4.f));
		tms[0].texture = floor;
		tms[0].SM = 3;
		tms[0].tilm = 2;

		tms[1].SetTexturedRectangle(rh, rh, 2.0f, 4.0f);
		tms[1].Translate(V3(0.0f, 0.0f, -200.0f));
		tms[1].Rotate(tms[1].GetCenter(), V3(0.0f, 1.0f, 0.0f), 90.0f);
		tms[1].Translate(V3(rw / 2.0f, 0.0f, -rw / 4.));
		tms[1].texture = sidewall;
		tms[1].SM = 3;
		tms[1].tilm = 1;

		tms[2].SetTexturedRectangle(rh, rh, 2.0f, 4.0f);
		tms[2].Translate(V3(0.0f, 0.0f, -200.0f));
		tms[2].Rotate(tms[2].GetCenter(), V3(0.0f, -1.0f, 0.0f), 90.0f);
		tms[2].Translate(V3(-rw / 2.f, 0.0f, -rw / 4.));
		tms[2].texture = sidewall;
		tms[2].SM = 3;
		tms[2].tilm = 1;

		tms[3].SetTexturedRectangle(rw, rh, 2.0f, 1.0f);
		tms[3].Translate(V3(0.0f, 0.0f, -200.0f));
		tms[3].Rotate(tms[3].GetCenter(), V3(0.f, 0.0f, 1.0f), 180.0f);
		tms[3].Translate(V3(0.0f, 0.0f, -rw / 2.f));
		tms[3].texture = backwall;
		tms[3].SM = 3;
		tms[3].tilm = 2;
		
		tms[4].SetTexturedRectangle(rw, rh / 2.0, 2.0f, 1.0f);
		tms[4].Translate(V3(0.0f, 0.0f, -200.0f));
		tms[4].Rotate(tms[4].GetCenter(), V3(0.f, 0.0f, 1.0f), 180.0f);
		tms[4].Translate(V3(0.0f, -rh / 4.f, -rw / 4.f));
		tms[4].texture = subject;
		tms[4].SM = 3;
		tms[4].tilm = 2;

		V3 sum = V3(0.f, 0.f, 0.f);
		for (int i = 0; i < tmsN; i++) {
			sum += tms[i].GetCenter();
		}
		V3 avg = sum / tmsN;
		ppc->PositionAndOrient(ppc->C + V3(0.f, 50.f, 0.f), avg, V3(0.f, 1.f, 0.f));
		Render();
		ppc3->PositionAndOrient(ppc3->C, avg, V3(0.0f, 1.0f, 0.0f));
		fb->SaveAsTiff("texture_scene.tif");
		while (1) {
			Render();
			Fl::check();
		}
		return;

		fb3->hide();
		for (int fi = 0; fi < 450; fi++) {
			Render();
			Fl::check();
			tms[0].Rotate(tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f), .1f);
		}
		Render();
		return;
	}
	{
		while (1) {
			Render();
			Fl::check();
		}
		return;
	}
	{
		Render();
		return;
	}
	{
		V3 p0(20.0f, 330.0f, 0.0f);
		V3 p1(420.0f, 330.0f, 0.0f);
		V3 p2(420.0f, 100.0f, 0.0f);
		V3 tlc(p0[0], p2[1], 0.0f);
		V3 brc(p1[0], p0[1], 0.0f);
		cerr << tlc << endl;
		cerr << brc << endl;
		fb->SetBGR(0xff000000);
		fb->SetZB(0.f);
		//fb->DrawAARectangle(tlc, brc, 0xffffffff);
		//fb->DrawTriangle(p0, p1, p2, V3(1.f, 0.f, 0.f), V3(0.f, 1.f, 0.f), V3(0.f, 0.f, 1.f));
		fb->redraw();
		return;
	}
	{
		V3 p0(20.0f, 430.0f, 0.0f);
		V3 p1(520.0f, 331.0f, 0.0f);
		V3 p2(520.0f, 100.0f, 0.0f);
		fb->SetBGR(0xFFFFFFFF);
		//fb->DrawTriangle(p0, p1, p2, V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), V3(0.0f, 0.0f, 1.0f));
		fb->redraw();
		return;
	}
	{
		V3 cc(0.0f, 0.0f, -100.0f);
		float cubeSideLength = 30.0f;
		TM tm;
		//		tm.SetAsAACube(cc, cubeSideLength);
		tm.LoadBin("geometry/teapot57k.bin");
		tm.SetCenter(V3(0.0f, 0.0f, -150.0f));
		int psize = 5;
		V3 Cn(ppc->C);
		V3 L = tm.GetCenter();
		V3 upg(0.0f, 1.0f, 0.0f);
		PPC ppc0(*ppc);
		int fN = 360;
		//		fN = 1;
		for (int fi = 0; fi < fN; fi++) {
			fb->SetBGR(0xFFFFFFFF);
			fb->SetZB(FLT_MAX);
			//			tm.RenderAsPoints(psize, ppc, fb);
			tm.RenderWireFrame(ppc, fb);
			fb->redraw();
			Fl::check();
			float tstep = 0.1f;
			V3 tv = ppc->GetVD() * tstep;
			tv = ppc->a * tstep;
			V3 center = tm.GetCenter();
			ppc->Rotate('u', 0.01f);
			//			tm.Rotate(center, V3(0.0f, 1.0f, 0.0f), 0.1f);
			//			ppc->C = ppc->C + tv;
			ppc->PositionAndOrient(Cn, L, upg);
			Cn = Cn + V3(0.0f, 0.1f, 0.0f);
		}
		*ppc = ppc0;
		return;
	}
	//{
	//	V3 housetlc(150, 250, 0);
	//	V3 housebrc(350, 450, 0);

	//	V3 roof1(100, 250, 0);
	//	V3 roof2(250, 200, 0);
	//	V3 roof3(400, 250, 0);

	//	V3 doortlc(225, 350, 0);
	//	V3 doorbrc(275, 450, 0);

	//	V3 windowtlc1(180, 280, 0);
	//	V3 windowbrc1(220, 320, 0);

	//	V3 windowtlc2(280, 280, 0);
	//	V3 windowbrc2(320, 320, 0);

	//	V3 suncenter(500, 100, 0);

	//	fb->SetBGR(0xFF000000);
	//	fb->DrawAARectangle(housetlc, housebrc, 0xFF7F7F00);
	//	fb->DrawTriangle(roof1, roof2, roof3, 0xFFF54242);
	//	fb->DrawAARectangle(doortlc, doorbrc, 0xFF4254F5);
	//	fb->DrawAARectangle(windowtlc1, windowbrc1, 0xFF4254F5);
	//	fb->DrawAARectangle(windowtlc2, windowbrc2, 0xFF4254F5);
	//	fb->DrawDisk(suncenter, 50, 0xFF7F7FFF);
	//	fb->SaveAsTiff("2d_graphics.tif");
	//	fb->LoadTiff("2d_graphics.tif");
	//	fb->redraw();
	//	return;
	//}

	//{
	//	V3 p0(20.0f, 430.0f, 0.0f);
	//	V3 p1(520.0f, 331.0f, 0.0f);
	//	V3 p2(520.0f, 100.0f, 0.0f);
	//	V3 tlc(p0[0], p2[1], 0.0f);
	//	V3 brc(p1[0], p0[1], 0.0f);
	//	cerr << tlc << endl;
	//	cerr << brc << endl;
	//	fb->SetBGR(0xFF000000);
	//	fb->DrawAARectangle(tlc, brc, 0xFFFFFFFF);
	//	fb->DrawTriangle(p0, p1, p2, 0xFF0000FF);
	//	fb->redraw();
	//	return;

	//}

	//{
	//	V3 tlc(100.5f, 45.5f, 0.0f);
	//	V3 brc(300.5f, 145.5f, 0.0f);
	//	unsigned int col = 0xFF0000FF;
	//	unsigned int colDisk = 0xFFFFFF00;
	//	V3 center(114.1f, 300.1f, 0.0f);
	//	float r = 110.0f;
	//	int fN = 1000;
	//	float du = 1.f;
	//	for (int fi = 0; fi < fN; fi++) {
	//		fb->SetBGR(0xFFFFFFFF);
	//		fb->DrawAARectangle(tlc, brc, col);
	//		//	fb->DrawDisk(center, r, colDisk);
	//		fb->redraw();
	//		Fl::check();
	//		tlc[0] += du;
	//		brc[0] += du;
	//		center[0] += du;
	//	}
	//	return;
	//}
	
	{
		cerr << "INFO: pressed DBG button on GUI" << endl;

		V3 v0(1.0, 2.0, 0.0);
		V3 v1(2.0, 1.0, 1.0);
		V3 v2(10.0, 20.0, 10.0);
		fb->SetBGR(0xFF000000);
		for (int i = 0; i <= 720; i += 2) {
			cerr << v2 << endl;
			v2 = v2.rot_arb_axis(v0, v1, 2);
			unsigned int colx = 0xFFFF0000;
			unsigned int coly = 0xFF00FF00;
			unsigned int colz = 0xFF0000FF;
			fb->Set(320 + v2[0] - 1, 240 + v2[1] + 1, 0xFFFFFFFF);
			fb->Set(320 + v2[0] - 1, 240 + v2[1], 0xFFFFFFFF);
			fb->Set(320 + v2[0] - 1, 240 + v2[1] - 1, 0xFFFFFFFF);
			fb->Set(320 + v2[0], 240 + v2[1] - 1, 0xFFFFFFFF);
			fb->Set(320 + v2[0] + 1, 240 + v2[1] - 1, 0xFFFFFFFF);
			fb->Set(320 + v2[0] + 1, 240 + v2[1], 0xFFFFFFFF);
			fb->Set(320 + v2[0] + 1, 240 + v2[1] + 1, 0xFFFFFFFF);
			fb->Set(320 + v2[0], 240 + v2[1] + 1, 0xFFFFFFFF);
			fb->Set(320 + v2[0], 240 + v2[1], 0xFFFFFFFF);
			fb->Set(i, 240 - 4 * v2[0], colx);
			fb->Set(i, 240 - 4 * v2[1], coly);
			fb->Set(i, 240 - 4 * v2[2], colz);
			fb->redraw();
			Fl::check();
		}
		cerr << v2 << endl;

		return;
	}
	
}

void Scene::NewButton() {
	cerr << "INFO: pressed Play button on GUI" << endl;

	for (int tmi = 0; tmi < tmsN; tmi++) {
		tms[tmi].SM = 0;
	}
	for (int vi = 0; vi < tms[0].vertsN; vi++) {
		tms[0].cols[vi] = tms[0].og_cols[vi];
	}

	Render();
	Fl::check();

	float hfov = 60.0f;
	int h = 512;
	int w = 512;
	ppc->PositionAndOrient(V3(0.f, 20.f, -200.f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	PPC ppc1 = *ppc;
	PPC ppc2 = ppc1;
	PPC ppc_3 = PPC(hfov, w, h);
	PPC ppc4 = PPC(hfov, w, h);
	PPC ppc5 = PPC(hfov, w, h);
	PPC ppc6 = PPC(hfov, w, h);
	PPC ppc7 = PPC(hfov, w, h);
	PPC ppc8 = PPC(hfov, w, h);

	ppc2.Translate('d', 120.f);
	ppc2.Rotate('h', 35.f);

	ppc_3 = ppc2;
	ppc_3.Translate('d', 100.f);
	ppc_3.Rotate('h', 40.f);
	ppc_3.Translate('d', 120.f);

	ppc4 = ppc_3;
	ppc4.Translate('w', 85.f);
	ppc4.Rotate('j', 25.f);
	ppc4.Translate('q', 40.f);

	ppc5 = ppc4;
	ppc5.Translate('d', 20.f);
	ppc5.Translate('s', 30.f);
	ppc5.Rotate('h', 30.f);
	ppc5.Translate('d', 60.f);

	ppc6 = ppc2;
	ppc6.Translate('a', 100.f);
	ppc6.Rotate('k', 45.f);
	ppc6.Translate('a', 100.f);

	ppc7 = ppc6;
	ppc7.Translate('a', 100.f);
	ppc7.Translate('s', 40.f);
	ppc7.Rotate('n', 20.f);
	ppc7.Rotate('u', 25.f);
	ppc7.Rotate('k', 45.f);
	ppc7.Translate('a', 100.f);

	int frames = 60;

	for (int i = 0; i < frames; i++) {
		*ppc = ppc1.Interpolate(ppc2, (float)i / (float)frames);
		Render();
		Fl::check();
	}
	for (int i = 0; i < frames; i++) {
		*ppc = ppc2.Interpolate(ppc_3, (float)i / (float)frames);
		Render();
		Fl::check();
	}
	for (int i = 0; i < frames; i++) {
		*ppc = ppc_3.Interpolate(ppc4, (float)i / (float)frames);
		Render();
		Fl::check();
	}
	for (int i = 0; i < frames; i++) {
		*ppc = ppc4.Interpolate(ppc5, (float)i / (float)frames);
		Render();
		Fl::check();
	}
	for (int i = 0; i < frames; i++) {
		*ppc = ppc5.Interpolate(ppc_3, (float)i / (float)frames);
		Render();
		Fl::check();
	}
	for (int i = 0; i < frames; i++) {
		*ppc = ppc_3.Interpolate(ppc2, (float)i / (float)frames);
		Render();
		Fl::check();
	}
	for (int i = 0; i < frames; i++) {
		*ppc = ppc2.Interpolate(ppc6, (float)i / (float)frames);
		Render();
		Fl::check();
	}

	for (int i = 0; i < frames; i++) {
		*ppc = ppc6.Interpolate(ppc7, (float)i / (float)frames);
		Render();
		Fl::check();
	}

	//Render();
	//Fl::check();

	//PPC lightppc1 = *lightppc;
	//PPC lightppc2 = PPC(hfov * 2.f, 512, 512);
	//PPC lightppc3 = PPC(hfov * 2.f, 512, 512);
	//PPC lightppc4 = PPC(hfov * 2.f, 512, 512);
	//PPC lightppc5 = PPC(hfov * 2.f, 512, 512);

	//lightppc2.PositionAndOrient(V3(0.0f, 40.f, -40.f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//lightppc3.PositionAndOrient(V3(-80.f, 60.f, -225.f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//lightppc4.PositionAndOrient(V3(0.0f, 60.f, -450.f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//lightppc5.PositionAndOrient(V3(80.0f, 60.f, -225.f), tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));

	//for (int i = 0; i < frames; i++) {
	//	*lightppc = lightppc1.Interpolate(lightppc2, (float)i / (float)frames);
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//for (int i = 0; i < frames; i++) {
	//	*lightppc = lightppc2.Interpolate(lightppc3, (float)i / (float)frames);
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//for (int i = 0; i < frames; i++) {
	//	*lightppc = lightppc3.Interpolate(lightppc4, (float)i / (float)frames);
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}
	//for (int i = 0; i < frames; i++) {
	//	*lightppc = lightppc4.Interpolate(lightppc5, (float)i / (float)frames);
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//*lightppc = lightppc1;
	//for (int i = 0; i < frames; i++) {
	//	tms[0].Translate(V3(0.0f, 0.2f, 0.0f));
	//	tms[1].Translate(V3(0.0f, 0.4f, 0.0f));
	//	lightppc->PositionAndOrient(lightppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//for (int i = 0; i < frames; i++) {
	//	tms[0].Rotate(tms[0].GetCenter(), V3(0.0f, 0.0f, -1.0f), 0.5);
	//	lightppc->PositionAndOrient(lightppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//for (int i = 0; i < frames; i++) {
	//	tms[0].Rotate(tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f), 1.0f);
	//	lightppc->PositionAndOrient(lightppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//for (int i = 0; i < frames; i++) {
	//	tms[0].Translate(V3(0.0f, 0.1f, 0.2f));
	//	lightppc->PositionAndOrient(lightppc->C, tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f));
	//	ComputeShmap();
	//	Render();
	//	Fl::check();
	//}

	//int stepsize = 200;

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(0.2f, 0.2f, 0.0f);
	//}

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(-0.3f, 0.0f, 0.0f);
	//}

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(0.2f, -0.2f, 0.0f);
	//}

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(0.0f, 0.3f, 0.0f);
	//}

	//tms[0].SM = 3;
	//tms[1].SM = 3;
	////source = V3(0.0f, 0.0f, -100.0f);
	//Render();
	//Fl::check();

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(0.2f, 0.1f, -0.2f);
	//}

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(-0.2f, -0.1f, -0.2f);
	//}

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(-0.2f, 0.1f, 0.2f);
	//}

	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(0.2f, -0.1f, 0.2f);
	//}
	//for (int i = 0; i < stepsize; i++) {
	//	Render();
	//	Fl::check();
	//	//source += V3(0.0f, 0.0f, 0.0f);
	//}
}


//void Scene::Name() {
//	{
//		// V
//		V3 Vp1(0, 200, 0);
//		V3 Vp2(60, 300, 0);
//		V3 Vp3(20, 200, 0);
//
//		V3 Vp1r(100, 200, 0);
//		V3 Vp2r(60, 300, 0);
//		V3 Vp3r(120, 200, 0);
//
//		// A
//		V3 Ap1(140, 300, 0);
//		V3 Ap2(160, 300, 0);
//		V3 Ap3(200, 200, 0);
//
//		V3 Ap1r(240, 300, 0);
//		V3 Ap2r(260, 300, 0);
//		V3 Ap3r(200, 200, 0);
//
//		V3 Atlc(180, 245, 0);
//		V3 Abrc(220, 255, 0);
//
//		// I
//		V3 Itlc1(300, 200, 0);
//		V3 Ibrc1(380, 210, 0);
//
//		V3 Itlc2(335, 210, 0);
//		V3 Ibrc2(345, 290, 0);
//
//		V3 Itlc3(300, 290, 0);
//		V3 Ibrc3(380, 300, 0);
//
//		int fN = 1450;
//		float du = 1.5f;
//		for (int fi = 0; fi < fN; fi++) {
//			fb->SetBGR(0xFF000000);
//			fb->DrawTriangle(Vp1, Vp2, Vp3, 0xFFFF0000);
//			fb->DrawTriangle(Vp1r, Vp2r, Vp3r, 0xFFFF0000);
//			fb->DrawTriangle(Ap1, Ap2, Ap3, 0xFF00FF00);
//			fb->DrawTriangle(Ap1r, Ap2r, Ap3r, 0xFF00FF00);
//			fb->DrawAARectangle(Atlc, Abrc, 0xFF00FF00);
//			fb->DrawAARectangle(Itlc1, Ibrc1, 0xFF0000FF);
//			fb->DrawAARectangle(Itlc2, Ibrc2, 0xFF0000FF);
//			fb->DrawAARectangle(Itlc3, Ibrc3, 0xFF0000FF);
//			fb->redraw();
//			Fl::check();
//			if (Vp1[0] > 640) {
//				du = -1020;
//			}
//			Vp1[0] += du;
//			Vp2[0] += du;
//			Vp3[0] += du;
//			Vp1r[0] += du;
//			Vp2r[0] += du;
//			Vp3r[0] += du;
//			Ap1[0] += du;
//			Ap2[0] += du;
//			Ap3[0] += du;
//			Ap1r[0] += du;
//			Ap2r[0] += du;
//			Ap3r[0] += du;
//			Atlc[0] += du;
//			Abrc[0] += du;
//			Itlc1[0] += du;
//			Ibrc1[0] += du;
//			Itlc2[0] += du;
//			Ibrc2[0] += du;
//			Itlc3[0] += du;
//			Ibrc3[0] += du;
//			du = 1.5f;
//		}
//		fb->SaveAsTiff("name.tif");
//		fb->LoadTiff("name.tif");
//
//		return;
//	}
//}