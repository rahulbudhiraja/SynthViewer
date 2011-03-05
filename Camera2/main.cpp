/**********************************************************************

Camera with OpenGL

March, 13th, 2003

This tutorial was written by Philipp Crocoll
Contact: 
philipp.crocoll@web.de
www.codecolony.de

Every comment would be appreciated.

If you want to use parts of any code of mine:
let me know and
use it!

**********************************************************************
ESC: exit

CAMERA movement:
w : forwards
s : backwards
a : turn left
d : turn right
x : turn up
y : turn down
v : strafe right
c : strafe left
r : move up
f : move down
m/n : roll

***********************************************************************/
#include <windows.h>
#include <GL\glut.h>
#include "camera.h"
#include<iostream>
#include<stdlib.h>
#include<highgui.h>
#include <AntTweakBar.h>
#include<cv.h>
#include<cxcore.h>
#include<highgui.h>

#define ShowUpvector

CCamera Camera;

#define MAXPOINTS 400000


FILE *bundle_out_fp,*csv_fp;	
char *dummy;
char input_line[800];
int count_points,num_cameras;
double x3d[MAXPOINTS], y3d[MAXPOINTS], z3d[MAXPOINTS];
double r[MAXPOINTS],g[MAXPOINTS],b[MAXPOINTS];
double camerax[MAXPOINTS],cameray[MAXPOINTS],cameraz[MAXPOINTS],camerarotx[MAXPOINTS],cameraroty[MAXPOINTS],camerarotz[MAXPOINTS];
double useless;
double rotx=0,roty=0,rotz=0;
static int cameraid=0;
int buttonstate=923;

GLdouble projection_matrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLdouble modelview_matrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
int viewport_matrix[4] = {0,0,640,480};

float stepx=0.0,stepy=0.0,stepz=0.0;
TwBar *myBar;

GLdouble xx,yy,zz;
GLuint texture;
bool drawcameras=1,drawclouds=1;
int point_number=-1,stored_points=0,cloud_point_size=2;
IplImage *img;
CvCapture* capture;
float stored_points_position[4][3],frame_rate=33;
using namespace std;

void DrawNet(GLfloat size, GLint LinesX, GLint LinesZ)
{
	glBegin(GL_LINES);
	for (int xc = 0; xc < LinesX; xc++)
	{
		glVertex3f(	-size / 2.0 + xc / (GLfloat)(LinesX-1)*size,
			0.0,
			size / 2.0);
		glVertex3f(	-size / 2.0 + xc / (GLfloat)(LinesX-1)*size,
			0.0,
			size / -2.0);
	}
	for (int zc = 0; zc < LinesX; zc++)
	{
		glVertex3f(	size / 2.0,
			0.0,
			-size / 2.0 + zc / (GLfloat)(LinesZ-1)*size);
		glVertex3f(	size / -2.0,
			0.0,
			-size / 2.0 + zc / (GLfloat)(LinesZ-1)*size);
	}
	glEnd();
}

void load_ply()
{

	bundle_out_fp=fopen("indian_temple.ply","r");
	/* Skip over the Ply File Information till the end_header string is not reached , this is because different ply_files have different end_header locations */
	int i;		
	for(i=0;i<13;i++)
		dummy = fgets(input_line, 800, bundle_out_fp);

	cout<<"\n\n"<<dummy;
	count_points=0;

	while(!feof(bundle_out_fp))

	{
		dummy = fgets(input_line, 800, bundle_out_fp);

		sscanf(input_line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", & x3d[count_points],&y3d[count_points],&z3d[count_points],&r[count_points], &g[count_points], &b[count_points]);
		// cout<<"\n\n "<<x3d[count_points]<<"   "<<y3d[count_points]<<"    "<<z3d[count_points];
		//cout<<input_line;
		count_points++;

	}

	cout<<count_points;

	cout<<"\n\n The number of points in the scene are"<<count_points<<endl;
}

void draw_camera()
{

	glEnable(GL_POINT_SMOOTH);
	glPointSize(cloud_point_size+8);

	glBegin(GL_POINTS);

	glColor3f(1,1,1);

	for (int i = 0; i < num_cameras; i++) 
	{
		if(i==cameraid)
			glColor3f(0,0,1);
		else glColor3f(1,1,1);
		//glColor3f(r[i]/255.0,g[i]/255.0,b[i]/255.0); // Randomizing the color of the points ..
		glVertex3f(camerax[i],cameray[i],cameraz[i]);
		//glVertex2f(x3d[i],y3d[i]);

	}

	glEnd();


	glDisable(GL_POINT_SMOOTH);

}


int loadTexture_Ipl(IplImage *image, GLuint *text) 
{

	if (image==NULL) 
	{
		cout<<"Image not found";
		return -1;
	} 

	glGenTextures(1, text); 
	glBindTexture( GL_TEXTURE_2D, *text ); //bind the texture to it's array
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height,0, GL_BGR_EXT, GL_UNSIGNED_BYTE, image->imageData);

	return 0;

}

void draw_ply(){

	glEnable(GL_POINT_SMOOTH);
	glPointSize(cloud_point_size);

	glBegin(GL_POINTS);
	glDisable(GL_DITHER);

	for (int i = 0; i < count_points; i++) 
	{
		glPushMatrix();
		glColor3f(r[i]/255.0,g[i]/255.0,b[i]/255.0); //  the color of the points ..

		glVertex3f(x3d[i],y3d[i],z3d[i]);
		//glVertex2f(x3d[i],y3d[i]);
		glPopMatrix();
	}
	//glDisable(GL_DEPTH_TEST);
	//glPointSize(10);
	glEnable(GL_DITHER);

	glEnd();
	glDisable(GL_POINT_SMOOTH);

}


void Display(void)
{
 glEnable(GL_DEPTH_TEST);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glLoadIdentity();
glScalef(10.0,10.0,10.0);
Camera.Render();

//Draw the "world":
if(drawclouds)
draw_ply();
if(drawcameras)
draw_camera();
//
//if(point_number>0)
//{
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, texture );
	glPointSize(cloud_point_size+18);
	glColor3f(1,1,1);
	glBegin(GL_POINTS);
	//glColor3f(1,1,1);
	// 
	//glTexCoord2d(0,0);
	glVertex3f(x3d[point_number],y3d[point_number],z3d[point_number]);
	//glTexCoord2d(1.0,0.0);
	//glVertex3f(x3d[point_number]+3,y3d[point_number],z3d[point_number]);
	////glTexCoord2d(1.0,1.0);
	//glVertex3f(x3d[point_number]+3,y3d[point_number]+3,z3d[point_number]);
	////glTexCoord2d(0,1.0);
	//glVertex3f(x3d[point_number],y3d[point_number]+3,z3d[point_number]);
	glEnd();
	glDisable(GL_TEXTURE_2D);
//}

//cout<<"rotations x"<<rotx<<"   "<<roty<<"        "<<rotz<<endl;
//cout<<cameraid<<endl;
//Camera.Move( F3dVector(camerax[2],cameray[2],cameraz[2]));

//	for (int i = 0; i < num_cameras; i++) 
//{

//	//glColor3f(r[i]/255.0,g[i]/255.0,b[i]/255.0); // Randomizing the color of the points ..
//	Camera.Move( F3dVector(camerax[i],cameray[i],cameraz[i]));
//	//glVertex2f(x3d[i],y3d[i]);

//}
/*glPushMatrix();
glTranslatef(0.0276335, -0.0101761, 0.00350319);
glColor3f(1,1,1);
glutSolidCube(0.1);
glPopMatrix();*/
//	glTranslatef(0.0,-0.5,-6.0);
//
//	glScalef(3.0,1.0,3.0);
//	
//	GLfloat size = 2.0;
//	GLint LinesX = 30;
//	GLint LinesZ = 30;
//
//// Rahul Testing ..
//	//glColor3f(0,1,0);
//	//glutWireCube(1);
//// End of testing ..
//
//	GLfloat halfsize = size / 2.0;
//	glColor3f(1.0,1.0,1.0);
//	
//	glPushMatrix();
//		glTranslatef(0.0,-halfsize ,0.0);
//		DrawNet(size,LinesX,LinesZ);
//		glTranslatef(0.0,size,0.0);
//		DrawNet(size,LinesX,LinesZ);
//	glPopMatrix();
//	glColor3f(0.0,0.0,1.0);
//	glPushMatrix();
//		glTranslatef(-,0.0,0.0,halfsize);
//	DrawNet(size,LinesX,LinesZ);
//		glTranslatef(0.0,-size,0.0);
//		DrawNet(size,LinesX,LinesZ);
//	glPopMatrix();
//	glColor3f(1.0,0.0,0.0);
//	glPushMatrix();
//		glTranslatef(0.0,0.0,-halfsize);	
//		glRotatef(90.0,halfsize,0.0,0.0);
//		DrawNet(size,LinesX,LinesZ);
//		glTranslatef(0.0,size,0.0);
//		DrawNet(size,LinesX,LinesZ);
//	glPopMatrix();

	if(stored_points==4)
	{

		cvGrabFrame(capture);
		img=cvRetrieveFrame(capture); 
		loadTexture_Ipl(img, &texture);

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, texture );
	glBegin(GL_QUADS);
	glTexCoord2d(0,0);
	glVertex3f(stored_points_position[0][0],stored_points_position[0][1],stored_points_position[0][2]);
	glTexCoord2d(1.0,0.0);
	glVertex3f(stored_points_position[1][0],stored_points_position[1][1],stored_points_position[1][2]);
	glTexCoord2d(1.0,1.0);
	glVertex3f(stored_points_position[2][0],stored_points_position[2][1],stored_points_position[2][2]);
	glTexCoord2d(0,1.0);
	glVertex3f(stored_points_position[3][0],stored_points_position[3][1],stored_points_position[3][2]);
	glEnd();
	glDisable(GL_TEXTURE_2D);	
	cvWaitKey(1000/frame_rate);
		glDeleteTextures( 1, &texture );

	}//disable the right click ...and display the image at that point.. add r for the reset buttn ..
	
TwDraw();

//finish rendering:
glFlush();  
glutSwapBuffers();

}






void reshape(int x, int y)
{
	if (y == 0 || x == 0) return;  //Nothing is visible then, so return

	//Set a new projection matrix
	glMatrixMode(GL_PROJECTION);  
	glLoadIdentity();
	//Angle of view:40 degrees
	//Near clipping plane distance: 0.5
	//Far clipping plane distance: 20.0
	gluPerspective(40.0,(GLdouble)x/(GLdouble)y,0.5,2000.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0,0,x,y);  //Use the whole window for rendering
	TwWindowSize(x, y);
}

void TranslateCam(int cameraid,string str)
{

	int i=0;

	if (str=="back")
	{
		for(i=1;i<1000;i+=5)
		{
			Camera.Position.x=0;Camera.Position.y=0;Camera.Position.z=0;
			Camera.Move(F3dVector(camerax[cameraid]/i,cameray[cameraid]/i,cameraz[cameraid]/i)) ;
			Camera.Render();
			Display();

		}

		Camera.Position.x=0;Camera.Position.y=0;Camera.Position.z=0;


		for(i=1001;i>0;i-=5)
		{
			Camera.Position.x=0;Camera.Position.y=0;Camera.Position.z=0;
			Camera.Move(F3dVector(camerax[cameraid-1]/i,cameray[cameraid-1]/i,cameraz[cameraid-1]/i)) ;
			Camera.Render();
			Display();

		}

		//}

		cout<<"Moving back\n";

	}


	if (str=="front")
	{  

		for(i=1001;i>0;i-=5)
		{
			Camera.Position.x=0;Camera.Position.y=0;Camera.Position.z=0;
			Camera.Move(F3dVector(camerax[cameraid+1]/i,cameray[cameraid+1]/i,cameraz[cameraid+1]/i)) ;    		   Camera.Render();
			Display();

		}	

		cout<<"Moving front\n";

	}


}

void KeyDown(unsigned char key, int x, int y)
{

	if(!TwEventKeyboardGLUT(key,x,y))
	{
		switch (key) 
		{
		case 27:		//ESC
			PostQuitMessage(0);
			break;
		case 'a':		
			Camera.RotateY(5.0);roty+=5;
			Display();

			break;
		case 'd':		
			Camera.RotateY(-5.0);roty-=5;
			Display();

			break;
		case 'w':		
			Camera.MoveForward(-0.1) ;
			Display();
			break;
		case 's':		
			Camera.MoveForward(0.1) ;
			Display();
			break;
		case 'x':		
			Camera.RotateX(5.0);	rotx+=5;
			Display();

			break;
		case 'y':		
			Camera.RotateX(-5.0);rotx-=5;
			Display();

			break;
		case 'c':		
			Camera.StrafeRight(-0.1);
			Display();
			break;
		case 'C':
			cout<<"Point Stored";
			if(stored_points<4)
			{
			stored_points_position[stored_points][0]=x3d[point_number];	stored_points_position[stored_points][1]=y3d[point_number];	stored_points_position[stored_points][2]=z3d[point_number];
			stored_points++;
			}
			Display();
			break;
		case 'v':		
			Camera.StrafeRight(0.1);
			Display();
			break;
		case 'f':
			Camera.MoveUpward(-0.3);
			Display();
			break;
		case 'r':
			Camera.MoveUpward(0.3);
			Display();
			break;

		case 'm':
			Camera.RotateZ(-5.0);rotz-=5;
			Display();

			break;
		case 'n':
			Camera.RotateZ(5.0);rotz+=5;
			Display();

			break;
		case '[':
			TranslateCam(cameraid,"back");
			Display();
			cameraid--;
			if(cameraid<0)
				cameraid=num_cameras;
			break;
		case ']':
			TranslateCam(cameraid,"front");
			Display();
			cameraid=(cameraid++)%num_cameras;
			break;
		case 'R':
			stored_points=0;
		}
	}
	Display();
}

void load_cameras()
{

	csv_fp=fopen("indian_temple.csv","r");

	dummy = fgets(input_line, 800, csv_fp); // Useless line
	num_cameras=0;

	while(!feof(csv_fp))

	{
		dummy = fgets(input_line, 800, csv_fp);

		sscanf(input_line, "%lf,%lf,%lf,%lf,%lf,%lf,%lf ", &useless,&camerax[num_cameras],&cameray[num_cameras],&cameraz[num_cameras],&camerarotx[num_cameras], &cameraroty[num_cameras], &camerarotz[num_cameras]);

		//cout<<input_line;
		num_cameras++;

	}
	// cout<<"\n\n "<<camerax[10]<<"   "<<cameray[10]<<"    "<<cameraz[10];
	cout<<"\n\n The number of cameras in the scene are"<<num_cameras<<endl;

}	
//		glRotatef(90.0

float lastx=0, lasty=0;

void mouseClick(int button,int state,int x,int y)  //mouseClick
{
	/*double cx,cy,cz;
	double z;
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview_matrix );
	glGetDoublev( GL_PROJECTION_MATRIX, projection_matrix );
	glGetIntegerv( GL_VIEWPORT, viewport_matrix );

	glReadPixels(x,y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	gluUnProject( x, y, z, modelview_matrix , projection_matrix ,  viewport_matrix , &cx, &cy, &cz );*/

	GLint viewport[4];
	GLubyte pixel[3];

	glGetIntegerv(GL_VIEWPORT,viewport);

	glReadPixels(x,viewport[3]-y,1,1,
		GL_RGB,GL_UNSIGNED_BYTE,(void *)pixel);


	if( !TwEventMouseButtonGLUT(button,state,x, y) )  // send event to AntTweakBar
	{ 
		buttonstate=-1;
		lastx=x;
		lasty=y;
		if(button==GLUT_LEFT_BUTTON)
			buttonstate=0;
		else if(button==GLUT_RIGHT_BUTTON)
			buttonstate=1;
		else if(button == GLUT_WHEEL_UP)
			Camera.MoveForward(-10);
		else if(button == GLUT_WHEEL_DOWN)
			Camera.MoveForward(10);
	}
	//
	//		cout<<x<<"  "<<y<<endl;
	//
	//cout<<cx<<cy<<"  "<<cz<<endl;

	if(buttonstate==1)
	{
		printf("%d %d %d\n",pixel[0],pixel[1],pixel[2]);

		float pointno=0;
		int flag=0,i=-1;
		for(i=0;i<count_points;i++)
		{
			if(r[i]==pixel[0]&&g[i]==pixel[1]&&b[i]==pixel[2])
			{point_number=i;flag=1;}
		}

		if(flag==1)
		{
			cout<<"Found Point Number"<<point_number<<endl;
			glEnd();
		}

	}
	Display();
}


void mouseMovement(int x, int y) {

	if(!TwEventMouseMotionGLUT(x,y))
	{   
		float diffx=x-lastx; //check the difference between the current x and the last x position
		float diffy=y-lasty; //check the difference between the current y and the last y position
		lastx=x; //set lastx to the current x position
		lasty=y; //set lasty to the current y position

		if(buttonstate==0)	
		{
			Camera.RotateY(diffx/10); //set the xrot to xrot with the addition of the difference in the y position
			Camera.RotateX(diffy/10); //set the xrot to xrot with the addition of the difference in the y position
		}
		else if(buttonstate==1)
		{
			//Camera.StrafeRight(diffx/10);
			//Camera.MoveUpward(diffy/10);
		}//set the xrot to yrot with the addition of the difference in the x position

	}
	Display();
}

void Initialize_GUI()
{

	myBar = TwNewBar("Controls");
	TwAddVarRW(myBar , "Show Cameras", TW_TYPE_BOOL8 , &drawcameras, "");
	TwAddVarRW(myBar , "Show Points", TW_TYPE_BOOL8 , &drawclouds, "");
	TwAddVarRW(myBar , "Point Size",TW_TYPE_INT32, &cloud_point_size, "step=1");
	TwAddVarRW(myBar , "Frame Rate", TW_TYPE_FLOAT, &frame_rate, "step=0.1");
	//TwAddVarRW(myBar , "Translation X", TW_TYPE_FLOAT, &move_x, " group='Translation' step='0.5'");
	//TwAddVarRW(myBar , "Translaton Y", TW_TYPE_FLOAT, &move_y, " group='Translation' step='0.5'");
	//TwAddVarRW(myBar , "Translation Z", TW_TYPE_FLOAT, &move_z, " group='Translation' step='0.5'");
	//
	//TwAddVarRW(myBar , "Point Size", TW_TYPE_FLOAT, &point_size, " label='Point Size' ");
	//   TwAddVarRW(myBar , "Show Object", TW_TYPE_BOOL8, &show_object, " label='Show Object' ");     	
	//TwAddVarRW(myBar , "Scale", TW_TYPE_FLOAT, &scale_parameter, "step=0.01");
}

void mouse_passive(int x,int y)
{
	if(!TwEventMouseMotionGLUT(x,y))
	{
	}
	Display();
}


void load_convert_image()
{

	img=cvLoadImage("toco.jpg");
	cvShowImage("Turtle",img);
	loadTexture_Ipl(img, &texture);
}
void load_video()
{
 capture = cvCaptureFromAVI("Test_PTAMM.avi");

	if(!cvGrabFrame(capture))
		{              // capture a frame 
		  printf("Could not grab a frame\n\7");
		  exit(0);
		}

}

int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(500,300);
	TwWindowSize(500, 300);
	glutCreateWindow("Camera");
	//Load the files ...
	load_convert_image();
	load_cameras();
	load_ply();
	load_video();

	if( !TwInit(TW_OPENGL, NULL) )
	{
		// A fatal error occured    
		fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
		return 1;
	}

	Camera.Move( F3dVector(camerax[cameraid],cameray[cameraid], cameraz[cameraid]));
	Camera.RotateY(90);
	Camera.RotateZ(90);
	Camera.Render();
	Initialize_GUI();
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMovement); //check for mouse  movement
	glutDisplayFunc(Display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(KeyDown);
	TwGLUTModifiersFunc(glutGetModifiers);
	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	glutPassiveMotionFunc(mouse_passive); 
	glutIdleFunc(Display);
	glutMainLoop();
	TwTerminate();
	glDeleteTextures( 1, &texture );
	return 0;             
}
