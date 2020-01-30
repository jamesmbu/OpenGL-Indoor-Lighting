#include <iostream>
#include <sstream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Include GLM extensions
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D models
C3dglModel camera;
C3dglModel vase;
C3dglModel table;
C3dglModel horse;
C3dglModel lamp;

// bitmaps
C3dglBitmap bm;
GLuint idTexWood;
GLuint idTexFabric;
GLuint idTexNone;
GLuint idTexMetallicBrushed;

//(W2,step 2, GLSL Program
C3dglProgram Program;

// Vertex buffers
float vertices[] = {
	-10, 1.8, -1,
	-14, 1.8, -1,
	-12, -1.15, -3,
	-10, 1.8, -5,
	-14, 1.8, -5,
	-12, -1.15, -3,
	-10, 1.8, -1,
	-10, 1.8, -5,
	-12, -1.15, -3,
	-14, 1.8, -1,
	-14, 1.8, -5,
	-12, -1.15, -3,
	-10, 1.8, -1,
	-10, 1.8, -5,
	-14, 1.8, -1,
	-14, 1.8, -5
};
float normals[] = {
	0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,
	-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };
unsigned indices[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };



// buffers names
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15;		// Tilt Angle
float angleRot = 0.1f;		// Camera orbiting angle
vec3 cam(0);				// Camera movement values

int lamp1 = 0; // initially, point lights are OFF
int lamp2 = 0;  // these are used in PointLightSwitching function for toggling lamp lighting


bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	//Initialise shaders, this sets up the programmable pipeline
	C3dglShader VertexShader;
	C3dglShader FragmentShader;
	if(!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if(!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if(!VertexShader.Compile()) return false;
	if(!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if(!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if(!FragmentShader.Compile()) return false;
	if(!Program.Create()) return false;
	if(!Program.Attach(VertexShader)) return false;
	if(!Program.Attach(FragmentShader)) return false;
	if(!Program.Link()) return false;
	if(!Program.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// load your 3D models here!
	if (!camera.load("models\\camera.3ds")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!horse.load("models\\SHIRE_01.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;

	// bitmap textures
	bm.Load("models/oak.png", GL_RGBA); if (!bm.GetBits()) return false; //wooden
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());
	//glActiveTexture(GL_TEXTURE0); // for multitexturing, not required here
	

	glGenTextures(1, &idTexNone); //blank for solid colour objects
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
	
	
	bm.Load("models/fabric.png", GL_RGBA); if (!bm.GetBits()) return false; //fabric
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexFabric);
	glBindTexture(GL_TEXTURE_2D, idTexFabric);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());
	
	
	bm.Load("models/goldy.png", GL_RGBA); if (!bm.GetBits()) return false; //golden
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexMetallicBrushed);
	glBindTexture(GL_TEXTURE_2D, idTexMetallicBrushed);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.GetBits());

	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);


	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(30.0, 16.0, 25.0),
		vec3(8.0, 16.0, 0.0),
		vec3(0.0, 16.0, 0.0));

	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift+AD or arrow key to auto-orbit" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << "  Press 1 or 2 to toggle lighting of lamps" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void render()
{
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	m = m * matrixView;
	//m = rotate(m, radians(angleRot), vec3(0.f, 1.f, 0.f));				// animate camera orbiting
	matrixView = m;
	Program.SendUniform("matrixView", matrixView);

	// LIGHTING and initial light settings
	Program.SendUniform("lightAmbient.on", 1);
	Program.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);

	Program.SendUniform("lightDir.on", 1);
	Program.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	Program.SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2);	  // dimmed white light
	Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0); // color of the light
	
	Program.SendUniform("lightPoint1.position", -1.55, 13.9, -4.0);
	Program.SendUniform("lightPoint1.diffuse", 0.5, 0.5, 0.5); //brightness
	
	Program.SendUniform("lightPoint1.specular", 1.0,1.0,1.0); //brightestness .0 to 1.0
	
	Program.SendUniform("lightPoint2.position", 13.45, 13.9, 4.0);
	Program.SendUniform("lightPoint2.diffuse", 0.5, 0.5, 0.5); //brightness
	
	Program.SendUniform("lightPoint2.specular", 1.0, 1.0, 1.0); //brightestness .0 to 1.0
	Program.SendUniform("materialSpecular", 0.6, 0.6, 1.0); //colouring of reflection
	Program.SendUniform("shininess", 20.0); //shine
	
	glBindTexture(GL_TEXTURE_2D, idTexNone); //blank texture

	// spheres (light bulbs, visual for light point positions, emmisive lighting)
	
	Program.SendUniform("lightAmbient2.on", 1); // for emissive light bulb effect
	
	m = matrixView;
	m = translate(m, vec3(-1.55f, 13.9f, -4.0f));
	m = scale(m, vec3(0.3f, 0.3f, 0.3f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	
	Program.SendUniform("lightAmbient2.on", 0);

	Program.SendUniform("lightAmbient3.on", 1);
	m = matrixView;
	m = translate(m, vec3(13.45f, 13.9f, 4.0f));
	m = scale(m, vec3(0.3f, 0.3f, 0.3f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	Program.SendUniform("lightAmbient3.on", 0);
	
	Program.SendUniform("materialDiffuse", 1.0, 0.0, 0.0); // red lamp models
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	//lamp 1
	m = matrixView;
	m = translate(m, vec3(1.0f, 9.7f, -4.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.07f, 0.07f, 0.07f));
	lamp.render(m);

	//lamp 2
	m = matrixView;
	m = translate(m, vec3(16.0f, 9.7f, 4.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.07f, 0.07f, 0.07f));
	lamp.render(m);

	// vase
	Program.SendUniform("materialDiffuse", 0.4, 0.9, 1.0); // blue vase
	Program.SendUniform("materialSpecular", 0.4, 0.97, 1.0); //colouring of reflection
	m = matrixView;
	m = translate(m, vec3(9.0f, 9.7f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.4f, 0.4f, 0.4f));
	vase.render(m);
	
	//extra obj - horse
	glBindTexture(GL_TEXTURE_2D, idTexMetallicBrushed); //gold texture
	Program.SendUniform("shininess", 50.0); //more shiny
	Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	Program.SendUniform("materialSpecular", 0.95, 0.85, 0.3); //colouring of reflection is gold
	
	m = matrixView;
	m = translate(m, vec3(2.8f, 12.5f, 3.0f));
	m = rotate(m, radians(230.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	horse.render(m);

	Program.SendUniform("shininess", 20.0); //shine reverted to 20
	Program.SendUniform("materialSpecular", 0.6, 0.6, 1.0); //colouring of reflection reverted
	//table
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0); //black makes the table and chairs non-shiny
	m = matrixView;
	m = translate(m, vec3(9.0f, -1, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.014f, 0.014f, 0.014f));
	table.render(1,m);

	//chairs
	glBindTexture(GL_TEXTURE_2D, idTexFabric);

	m = matrixView;
	m = translate(m, vec3(8.0f, -1, -0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.014f, 0.014f, 0.014f));
	table.render(0, m);

	m = matrixView;
	m = translate(m, vec3(12.0f, -1, 0.0f));
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.014f, 0.014f, 0.014f));
	table.render(0,m);

	m = matrixView;
	m = translate(m, vec3(4.0f, -1, 0.0f));
	m = rotate(m, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.014f, 0.014f, 0.014f));
	table.render(0, m);
	
	m = matrixView;
	m = translate(m, vec3(8.0f, -1, 0.0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.014f, 0.014f, 0.014f));
	table.render(0, m);

	glBindTexture(GL_TEXTURE_2D, idTexNone); //blank texture
	
	// teapot
	Program.SendUniform("materialDiffuse", 0.6, 0.1, 1.0); // purple 
	Program.SendUniform("materialSpecular", 0.6, 0.6, 1.0);
	m = matrixView;
	m = translate(m, vec3(15.0f, 10.75f, 0.0f));
	m = rotate(m, radians(30.0f), vec3(0.0f, 1.0f, 0.0f));
	// the GLUT objects require the Model View Matrix setup
	Program.SendUniform("matrixModelView", m);
	glutSolidTeapot(1.5);
	
	// Get Attribute Locations
	GLuint attribVertex = Program.GetAttribLocation("aVertex");
	GLuint attribNormal = Program.GetAttribLocation("aNormal");

	// Enable vertex attribute arrays
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);

	// Bind (activate) the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind (activate) the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Draw triangles – using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	// Disable arrays
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	
	//W2, step 4.1 (replace deprecated code)
	Program.SendUniform("matrixProjection", matrixProjection); 

}

void PointLightSwitching(int PLightID)
{
	if (PLightID == 1)
	{
		if (lamp1 == 0) 
		{
			Program.SendUniform("lightAmbient2.color", 0.8, 0.8, 0.8);
			Program.SendUniform("lightPoint1.on", 1); 
			lamp1 += 1; return; 
		}
		else if (lamp1 == 1)
		{
			Program.SendUniform("lightAmbient2.color", 0.0, 0.0, 0.0);
			Program.SendUniform("lightPoint1.on", 0);
			lamp1 -= 1;
			return;
		}
	}
	else if (PLightID == 2)
	{
		if (lamp2 == 0) 
		{
			Program.SendUniform("lightAmbient3.color", 0.8, 0.8, 0.8);
			Program.SendUniform("lightPoint2.on", 1);
			lamp2 += 1; std::cout << lamp2 << endl;
			return;
		}
		else if (lamp2 == 1)
		{
			Program.SendUniform("lightAmbient3.color", 0.0, 0.0, 0.0);
			Program.SendUniform("lightPoint2.on", 0);
			lamp2 -= 1;
			std::cout << lamp2 << endl;
			return;
		}
	}
}	
// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); angleRot = 0.1f; break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); angleRot = -0.1f; break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case '1':

		PointLightSwitching(1);

		break;

	case '2':
		PointLightSwitching(2);
		break;
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
	// stop orbiting
	if ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) == 0) angleRot = 0;
}


// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}


	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

