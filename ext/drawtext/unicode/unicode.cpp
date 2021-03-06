/* Unicode libdrawtext example.
 *
 * Important parts are marked with XXX comments.
 */
#include "../../../src/libs.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

/* various UTF-8 strings */
const char *english_text = "Hello world!";
const char *greek_text = "\xce\x9a\xce\xbf\xcf\x8d\xcf\x81\xce\xb1\xcf\x83\xce\xb7";
const char *russian_text = "\xd0\xa0\xd0\xb0\xd1\x81\xd1\x86\xd0\xb2\xd0\xb5\xd1\x82\xd0\xb0\xd0\xbb\xd0\xb8 \xd1\x8f\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xbd\xd0\xb8 \xd0\xb8 \xd0\xb3\xd1\x80\xd1\x83\xd1\x88\xd0\xb8";
const char *kanji_text = "\xe4\xb9\x97\xe4\xba\xac";
const char *klingon_text = "\xef\xa3\xa3\xef\xa3\x9d\xef\xa3\x93\xef\xa3\x98\xef\xa3\x9d\xef\xa3\xa2\xef\xa3\xa1\xef\xa3\x9d\xef\xa3\x99";

#include "drawtext/drawtext.h"

// GUI START
#include <string>
#include <vector>
#include <map>

#define loopi(start_l,end_l) for ( int i=start_l;i<end_l;++i )
#define loopj(start_l,end_l) for ( int j=start_l;j<end_l;++j )
#define loopk(start_l,end_l) for ( int k=start_l;k<end_l;++k )

void ogl_drawquad(	float x0,float y0,float x1,float y1,
					float tx0=0,float ty0=0,float tx1=0,float ty1=0)
{
	glBegin(GL_QUADS);
	glTexCoord2f(tx0,ty0);glVertex2f(x0,y0);
	glTexCoord2f(tx1,ty0);glVertex2f(x1,y0);
	glTexCoord2f(tx1,ty1);glVertex2f(x1,y1);
	glTexCoord2f(tx0,ty1);glVertex2f(x0,y1);
	glEnd();
}

void ogl_drawlinequad(float x0,float y0,float x1,float y1)
{
	glBegin(GL_LINE_LOOP);
	glVertex2f(x0,y0);
	glVertex2f(x1,y0);
	glVertex2f(x1,y1);
	glVertex2f(x0,y1);
	glEnd();
}

struct vec4{ float x,y,z;};

struct Gui
{
	struct dtx_font *font;

	static std::map<std::string,float>			cfg_float;
	static std::map<std::string,std::string>	cfg_string;

	enum WindowFlags { RESIZEABLE=1 , TITLEBAR=2 , CLOSEBUTTON=4 , LOCKED=8 };

	struct Mouse
	{
		int x,y,x2,y2,dx,dy,button[3],button2[3],button_pressed[3],button_released[3];
		
		void init(){x=y=x2=y2=dx=dy=0;loopi(0,3)button[i]=button2[i]=button_pressed[i]=button_released[i]=0;};
		void update()
		{
			dx=x-x2;dy=y-y2;x2=x;y2=y;
			loopi(0,3) button_pressed [i]= (button [i]&&!button2[i]) ? 1 : 0;
			loopi(0,3) button_released[i]= (button2[i]&&!button [i]) ? 1 : 0;
			loopi(0,3) button2[i]=button[i];
		}
	};
	struct Keyb
	{
		int key[512],key2[512],key_pressed[512],key_released[512];
		
		void init(){loopi(0,512)key[i]=key2[i]=key_pressed[i]=key_released[i]=0;}
		void update()
		{
			loopi(0,512) key_pressed [i]= (key [i]&&!key2[i]) ? 1 : 0;
			loopi(0,512) key_released[i]= (key2[i]&&!key [i]) ? 1 : 0;
			loopi(0,512) key2[i]=key[i];
		}
	};
	static Mouse mouse;
	static Keyb keyb;

	class Button 
	{  
		public:

		bool pressed,resizeable;
		float x,y,sx,sy;
		std::string text;
		vec4 color;

		Button(){pressed=resizeable=x=sx=y=sy=0;};

		Button(std::string buttontext,int x=0,int y=0,int width=50,int height=30)
		{
			pressed=resizeable=0;
			this->x=x; 
			this->y=y;
			sx=width;
			sy=height;
			text=buttontext;
		}

		bool mouseinside(float ox=0,float oy=0)
		{
			if( mouse.x>=ox+x ) if( mouse.x<=ox+x+sx ) 
			if( mouse.y>=oy+y ) if( mouse.y<=oy+y+sy ) 
				return 1;			
			if( mouse.x2>=ox+x ) if( mouse.x2<=ox+x+sx ) 
			if( mouse.y2>=oy+y ) if( mouse.y2<=oy+y+sy ) 
				return 1;
			return 0;
		}
		
		void draw(float ox=0,float oy=0)
		{
			pressed=0;
			float light=mouseinside(ox,oy) ? 0.5 : 0;
			if(light>0.1 && mouse.button[0]) { light=1;pressed=1; }

			if( text=="" )light=0;

			glPushMatrix();
			glColor3f(0, light, 1); ogl_drawquad(ox+x,oy+y,ox+x+sx,oy+y+sy);
			glColor3f(0,     1, 0); ogl_drawlinequad(ox+x,oy+y,ox+x+sx,oy+y+sy);
			
			glTranslatef(ox+x+   cfg_float["button_text_padx"], 
					     oy+y+sy-cfg_float["button_text_pady"] , 0);

			glColor3f(1,     1, 1); dtx_string(text.c_str());	
			glPopMatrix();
		};
	};
	class Window
	{
		public:

		int flags;

		Button box,title,close,resize;

		std::map<std::string,Window> window;
		std::map<std::string,Button> button;

		Window (){box.x=box.y=box.sx=box.sy=flags=0;};

		Window (std::string windowtitle,int x=0,int y=0,int width=200,int height=200, int wflags=RESIZEABLE|TITLEBAR|CLOSEBUTTON)
		{
			flags=wflags;
			box.x=x; 
			box.y=y;
			box.sx=width;
			box.sy=height;
			title.text=windowtitle;
		}
		void draw(float ox=0,float oy=0)
		{
			box.draw(ox,oy);

			if(flags&TITLEBAR)
			{
				//title
				title.x=0;
				title.y=-cfg_float["window_title_height"];
				title.sx=box.sx;
				title.sy=-title.y;
				title.draw(box.x+ox,box.y+oy);

				if(!(flags&LOCKED))
				if(mouse.button[0]&&title.mouseinside(box.x+ox,box.y+oy))
				{
					box.x+=mouse.dx;box.y+=mouse.dy;
				}

				if(flags&CLOSEBUTTON)
				{
					//close button
					close.sx=cfg_float["window_title_closebuttonwidth"];
					close.sy=cfg_float["window_title_height"];
					close.x=box.sx-close.sx;
					close.y=-close.sy;
					close.draw(box.x+ox,box.y+oy);
				}
			}

			if(flags&RESIZEABLE)
			{
				//resize button
				resize.sx=cfg_float["window_title_height"];
				resize.sy=cfg_float["window_title_height"];
				resize.x=box.sx-resize.sx;
				resize.y=box.sy-resize.sy;
				resize.draw(box.x+ox,box.y+oy);
			}

			if(flags&RESIZEABLE)
			if(mouse.button[0]&&resize.mouseinside(box.x+ox,box.y+oy))
			{
				box.sx+=mouse.dx;box.sy+=mouse.dy;
			}

			//content
			for (auto it=window.begin(); it!=window.end(); ++it) it->second.draw(box.x+ox,box.y+oy);
			for (auto it=button.begin(); it!=button.end(); ++it) it->second.draw(box.x+ox,box.y+oy);
		}
	};
	std::map<std::string,Window> dialog;
	std::map<std::string,Window> window;
	std::map<std::string,Button> button;

	bool init()
	{
		mouse.init(); keyb.init();

		cfg_float["font_size"]=24;

		cfg_float["window_title_height"]=30;
		cfg_float["window_title_closebuttonwidth"]=30;

		cfg_float["button_text_padx"]=3;
		cfg_float["button_text_pady"]=3;

		if(!(font = dtx_open_font("../data/meiryob.ttc", 0)))return 0;
		dtx_prepare_range(font, cfg_float["font_size"], 0, 256);			/* ASCII */
		dtx_prepare_range(font, cfg_float["font_size"], 0x370, 0x400);		/* greek */
		dtx_prepare_range(font, cfg_float["font_size"], 0x400, 0x500);		/* cyrilic */
		dtx_prepare_range(font, cfg_float["font_size"], 0x4e00, 0x9fc0);	/* kanji */

		return true;
	}
	void update()
	{
		keyb.update();
		mouse.update();
	}
	void draw()
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		dtx_use_font(font, cfg_float["font_size"]);
		for (auto it=button.begin(); it!=button.end(); ++it) it->second.draw();
		for (auto it=window.begin(); it!=window.end(); ++it) it->second.draw();
	}
};
std::map<std::string,float>			Gui::cfg_float;
std::map<std::string,std::string>	Gui::cfg_string;
Gui::Mouse Gui::mouse;
Gui::Keyb Gui::keyb;
Gui gui;

void draw_gui()
{
	static bool init=1;
	if(init)
	{
		init=0;
		gui.init();

		// Dialog Template 
		gui.dialog["test"]=Gui::Window("Title",100,100,200,200,Gui::TITLEBAR);
		gui.dialog["test"].button["Ok1"]=Gui::Button("OK",20,20);
		gui.dialog["test"].button["Ok2"]=Gui::Button(kanji_text,120,20,60,30);

		// Window 1 from template
		gui.window["test3"]=gui.dialog["test"];
		gui.window["test3"].box.y=300;

		// Window 2 from template
		gui.window["test4"]=gui.dialog["test"];
		gui.window["test4"].box.x=300;
		gui.window["test4"].box.y=300;
		gui.window["test4"].flags|=Gui::CLOSEBUTTON | Gui::RESIZEABLE ;

		// Window 3 from scratch
		gui.window["test2"]=Gui::Window("Win2",400,200,300,300);
		gui.window["test2"].button["Ok1"]=Gui::Button("Huhu",20,20,80,30);
		gui.window["test2"].button["Ok2"]=Gui::Button("Push",140,20,70,30);
		gui.window["test2"].window["ww"]=Gui::Window("sub",100,100,200,200); //subwindow
		gui.window["test2"].flags|=Gui::LOCKED;

		// Some unorganized buttons
		gui.button["load"]=Gui::Button("Load",20,20,80,30);
		gui.button["save"]=Gui::Button("Save",20,60,80,30);
	}
	if(gui.window["test3"].button["Ok1"].pressed) gui.window.erase("test3");
	if(gui.window["test4"].button["Ok1"].pressed) gui.window.erase("test4");

	if(gui.window["test3"].close.pressed) gui.window.erase("test3");
	if(gui.window["test4"].close.pressed) gui.window.erase("test4");
	
	gui.update();
	gui.draw();
}
////////////////////////////////////////////////////////////////////////////////
void KeyDown1Static(int key, int x, int y)           { gui.keyb.key[ key&255 ] =true;  }
void KeyDown2Static(unsigned char key, int x, int y) { gui.keyb.key[ key&255 ] =true;  }
void KeyUp1Static(int key, int x, int y)             { gui.keyb.key[ key&255 ] =false; }
void KeyUp2Static(unsigned char key, int x, int y)   { gui.keyb.key[ key&255 ] =false; }
void MouseMotionStatic (int x,int y){	gui.mouse.x = x;gui.mouse.y = y;}
void MouseButtonStatic(int button_index, int state, int x, int y)
{
	gui.mouse.button[button_index] =  ( state == GLUT_DOWN ) ? true : false;
	MouseMotionStatic (x,y);
}
////////////////////////////////////////////////////////////////////////////////

void disp(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("GUI");

	glutIdleFunc(disp);
	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);

	glutSpecialFunc(&KeyDown1Static);
	glutSpecialUpFunc(&KeyUp1Static);
	glutKeyboardFunc(&KeyDown2Static);
	glutKeyboardUpFunc(&KeyUp2Static);
	glutMotionFunc(&MouseMotionStatic);
	glutPassiveMotionFunc(&MouseMotionStatic);
	glutMouseFunc (&MouseButtonStatic);
	glutMainLoop();
	return 0;
}


void disp(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	draw_gui();

	glutSwapBuffers();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, x, y, 0, -1, 1);

}














/*
void keyb(unsigned char key, int x, int y)
{
	if(key == 27) {
		exit(0);
	}
}





	 XXX dtx_open_font opens a font file and returns a pointer to dtx_font */
	
	//if(!(fntcjk = dtx_open_font("../data/meiryob.ttc", 0)))return 1;
	//if(!(fntklingon = dtx_open_font("../data/meiryob.ttc", 0))) return 1;

	
	/*if(!(fntcjk = dtx_open_font("../data/meiryob.ttc", 0))) {
		return 1;
	}
	
	dtx_prepare_range(fntcjk, FONT_SZ, 0x4e00, 0x9fc0);		

	if(!(fntklingon = dtx_open_font("../data/meiryob.ttc", 0))) {
		return 1;
	}
	dtx_prepare_range(fntklingon, FONT_SZ, 0xf8d0, 0xf900);
	*/
	

	//glTranslatef(-200, 150, 0);
	/* XXX call dtx_string to draw utf-8 text.
	 * any transformations and the current color apply
	 */
	//dtx_string(english_text);
	/*
	glTranslatef(0, -40, 0);
	dtx_string(greek_text);

	glTranslatef(0, -40, 0);
	dtx_string(russian_text);

	dtx_use_font(fntcjk, FONT_SZ);

	glTranslatef(0, -40, 0);
	dtx_string(kanji_text);

	dtx_use_font(fntklingon, FONT_SZ);

	glTranslatef(0, -40, 0);
	dtx_string(klingon_text);
	*/
	