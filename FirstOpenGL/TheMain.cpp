#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>			
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>		
#include <string>
#include <vector>		
#include "Utilities.h"
#include "ModelUtilities.h"
#include "cMesh.h"
#include "cShaderManager.h" 
#include "cGameObject.h"
#include "cVAOMeshManager.h";
#include<vector>
#include"myInterface.h"


int uni=0;
float camerafactor = 5.0f;
bool debugrender = 0;
//Game Objects get stored here
std::vector< cGameObject* >  g_vecGameObjects;
float theta = 180;
//Camera Params
glm::vec3 g_cameraXYZ = glm::vec3(0.0f, 0.0f, 10.0f);	// 5 units "down" z
glm::vec3 g_cameraTarget_XYZ = glm::vec3(0.0f, 0.0f, 0.0f);
//Shader and Mesh manager
cVAOMeshManager* g_pVAOManager = 0;
cShaderManager*		g_pShaderManager;




//Error Callback
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


//Load the ply file with a name to reference it
void LoadPly(std::string name, std::string file)
{
	cMesh testMesh;
	testMesh.name = name;
	if (!LoadPlyFileIntoMesh(file, testMesh))
	{
		std::cout << "Didn't load model" << std::endl;
		// do something??
	}
	if (!::g_pVAOManager->loadMeshIntoVAO(testMesh, ::g_pShaderManager->getIDFromFriendlyName("mySexyShader")))
	{
		std::cout << "Could not load mesh into VAO" << std::endl;
	}
}



//Create a 3D object out of the loaded ply file by its name reference
void makeObject(glm::vec3 pos, glm::vec3 or, glm::vec3 or2, float scale, glm::vec4 color, std::string name, eTypeOfObject type, bool phys )
{
	cGameObject* pTempGO = new cGameObject();
	pTempGO->position = pos;
	
	pTempGO->orientation = or;	// Degrees			
	pTempGO->orientation2 = glm::radians(or2) ;	
	
	pTempGO->scale = scale;

	pTempGO->diffuseColour = color;

	pTempGO->meshName = name;		// Was teapot
	pTempGO->typeOfObject = type;
	// ***
	pTempGO->bIsUpdatedInPhysics = phys;
	// ***
	::g_vecGameObjects.push_back(pTempGO);		// Fastest way to add
}


//To load the scene from the scene.txt file
bool SceneDescriptor(std::string file)
{
	std::string typer;
	glm::vec3 pos; glm::vec3 or ; glm::vec3 or2; float scale;  glm::vec4 color; std::string name; eTypeOfObject type; bool phys;
	std::ifstream plyFile(file.c_str());
	if (!plyFile.is_open())
	{	// Didn't open file, so return
		return false;
	}

	
	ReadFileToToken(plyFile, "models");
	int count;
	plyFile >> count;
	
	for (int i = 1; i <= count; i++)
	{
		
		ReadFileToToken(plyFile, std::to_string(i));
		ReadFileToToken(plyFile, "position");
		plyFile >> pos.x;
		plyFile >> pos.y;
		plyFile >> pos.z;
		
		ReadFileToToken(plyFile, "orientation");
		plyFile >> or.x;
		plyFile >> or.y;
		plyFile >> or.z;

		
		ReadFileToToken(plyFile, "orientation2");
		plyFile >> or2.x;
		plyFile >> or2.y;
		plyFile >> or2.z;
		
		ReadFileToToken(plyFile, "scale");
		plyFile >> scale;
		
		ReadFileToToken(plyFile, "color");
		plyFile >> color.a;
		plyFile >> color.r;
		plyFile >> color.g;
		plyFile >> color.b;
	
		ReadFileToToken(plyFile, "name");
		plyFile >> name;
		
		ReadFileToToken(plyFile, "type");
		plyFile >> typer;
		if (typer == "sphere")
		{
			type = eTypeOfObject::SPHERE;
		}
		else
		{
			type = eTypeOfObject::PLANE;
		}

		ReadFileToToken(plyFile, "physics");
		plyFile >> phys;
		
		makeObject(pos, or , or2, scale, color, name, type, phys);

	}

}


//Keyboard key calls for camera control
void handlekeys(float deltaTime)
{
	
	if (GetKeyState(0x25) < 0)
	{
		theta += (float)deltaTime;
	}

	if (GetKeyState(0x27) < 0)
	{
		theta -= (float)deltaTime;
	}

	if (GetKeyState(0x26) < 0)
	{
		camerafactor += 7 * (float)deltaTime;
	}

	if (GetKeyState(0x28) < 0)
	{
		camerafactor -= 7 * (float)deltaTime;
	}


}


//Returns the dimension of the maze directly from the file
int*  mazedimension(std::string file)
{
	int * dimension = new int[2] ;
	std::ifstream plyFile(file.c_str());
	if (!plyFile.is_open())
	{	// Didn't open file, so return
		
	}


	ReadFileToToken(plyFile, "size");
	plyFile >> dimension[0];
	plyFile >> dimension[1];
	return dimension;
}




//LOAD maze dimension in a variable to avoid reading the file too often
int mazex = mazedimension("maze.txt")[0];
int mazez = mazedimension("maze.txt")[1];


//load maze from the text file into a matrix
int ** loadmaze(std::string file)
{
	
	int* dimension = new int[2];
	dimension = mazedimension(file);
	int ** maze = new int*[dimension[0]];
	for (int i = 0; i < dimension[0]; i++)
	{
		maze[i] = new int[dimension[1]];
	}
	std::ifstream plyFile(file.c_str());
	if (!plyFile.is_open())
	{	// Didn't open file, so return

	}
	ReadFileToToken(plyFile, "maze");
	for (int i = 0; i < dimension[0]; i++)
	{
		for (int j = 0; j < dimension[1]; j++)
		{
			plyFile >> maze[i][j];
			std::cout << maze[i][j] << "   ";
		}
		std::cout << "\n";
	}
	
	return maze;
}




// Draw maze from the matrix into actual OPEN GL environment
void drawmaze(int ** &maze)
{

	for (int i = 0; i < mazex; i++)
	{
		for (int j = 0; j < mazez; j++)
		{
			if(maze[i][j]==1)
			makeObject(glm::vec3 ( (float)i,0,(float)j ), glm::vec3 (0,0,0) , glm::vec3(0, 0, 0), 0.1, glm::vec4(1,0,0,1) , "SphereRadius1", eTypeOfObject::SPHERE, 0);
			else if(maze[i][j]==0)
			makeObject(glm::vec3((float)i, 0, (float)j), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.1, glm::vec4(1, 1, 1, 1), "SphereRadius1", eTypeOfObject::SPHERE, 0);
			else if (maze[i][j] == 3)
				makeObject(glm::vec3((float)i, 0, (float)j), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.1, glm::vec4(0, 1, 0, 1), "SphereRadius1", eTypeOfObject::SPHERE, 0);
			else
				makeObject(glm::vec3((float)i, 0, (float)j), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.1, glm::vec4(1, 1, 1, 1), "SphereRadius1", eTypeOfObject::SPHERE, 0);
			
		}
	}
}

// Draw the Moving Object from the mymaze matrix
void drawmazeobj(int ** &maze,int &ballx, int &ballz, int &destx, int &destz)
{


	for (int i = 0; i < mazex; i++)
	{
		for (int j = 0; j < mazez; j++)
		{
			if (maze[i][j] == 2)
			{
				ballx = i; ballz = j;
				makeObject(glm::vec3((float)i, 0, (float)j), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.2, glm::vec4(0, 0, 1, 1), "SphereRadius1", eTypeOfObject::SPHERE, 0);
			}
			else if (maze[i][j] == 3)
			{
				destx = i; destz = j;
			}

		}
	}
}


//Implementation of the A* algorithm on the object to move it
//MoveTo(objno, mymaze, destx, destz, H, open);
bool MoveTo(int objno, int**&mymaze,int destx, int destz, int **H)
{
	//Calculate and store F values for all 8 directions
	int f[8];
	int px, pz;
	int temp;
	
	//Current position of the moving object
	px = (int)::g_vecGameObjects[objno]->position.x;
	pz = (int)::g_vecGameObjects[objno]->position.z;
	//open[px][pz] = 0;


	//Until the object reaches its destination
	if (px != destx || pz != destz)
	{

		
		{

			//Calculate F while making corners costlier than 90 degree straight movements
			f[0] = ( 1 + H[px-1][pz] ) ;
			f[1] = ( 2 + H[px - 1][pz+1] );
			f[2] = ( 1 + H[px][pz + 1] ) ;
			f[3] = ( 2 + H[px + 1][pz + 1] ) ;
			f[4] = ( 1 + H[px+1][pz] );
			f[5] = ( 2 + H[px + 1][pz - 1] ) ;
			f[6] = ( 1 + H[px][pz-1] ) ;
			f[7] = ( 2 + H[px - 1][pz-1] ) ;
			temp = 9999;
			for (int u = 0; u < 8; u++)
			{
				std::cout << "\nf" << f[u];
			}

			
			//std::cout << "\nCurrent f: " << << "," << pz << std::endl;
			for (int i = 0; i <= 7; i++)
			{
				if (f[i] < temp && f[i]>0)
					temp = f[i];
			}

			//Move towards the destination based on the direction with the smallestF value
			if (f[0] == temp) { ::g_vecGameObjects[objno]->position.x -= 1; std::cout << "Moved in 0" << std::endl;}
			else if (f[1] == temp) {::g_vecGameObjects[objno]->position.x -= 1; ::g_vecGameObjects[objno]->position.z += 1;  std::cout << "Moved in 1" << std::endl;
			}
			else if (f[2] == temp) { ::g_vecGameObjects[objno]->position.z += 1; std::cout << "f2" << std::endl; std::cout << "Moved in 2" << std::endl;
			}
			else if (f[3] == temp) { ::g_vecGameObjects[objno]->position.x += 1; ::g_vecGameObjects[objno]->position.z += 1; std::cout << "Moved in 3" << std::endl;
			}
			else if (f[4] == temp) { ::g_vecGameObjects[objno]->position.x += 1; std::cout << "f4"<<std::endl; std::cout << "Moved in 4" << std::endl;
			}
			else if (f[5] == temp) { ::g_vecGameObjects[objno]->position.x += 1; ::g_vecGameObjects[objno]->position.z -= 1; std::cout << "Moved in 5" << std::endl;
			}
			else if (f[6] == temp) {
				::g_vecGameObjects[objno]->position.z -= 1; std::cout << "Moved in 6" << std::endl;
			}
			else if (f[7] == temp) {::g_vecGameObjects[objno]->position.x -= 1;  ::g_vecGameObjects[objno]->position.z -= 1; std::cout << "Moved in 7" << std::endl;
			}
			
		}


		//Test prints of matrices
		//////////////////////////////////////////////////////////
		std::cout << "H Matrix in Method: \n";
		for (int i = 0; i < mazex; i++)
		{
			for (int j = 0; j < mazez; j++)
			{
				if (px == i && pz == j)
				{
					std::cout << H[i][j] << "*  ";
				}
				else
				{
					std::cout << H[i][j] << "  ";
				}

			}
			std::cout << " \n";
		}
		/////////////////////////////////////////////////////////////

	}
		
	return 0;
}





//Main
int main(void)
{
	
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;
	int height;	/* default */
	int width;	// default
	std::string title;


	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);


	std::ifstream infoFile("config.txt");
	if (!infoFile.is_open())
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so read it... 
		std::string a;

		infoFile >> a;	// "Game"	//std::cin >> a;
		infoFile >> a;	// "Config"
		infoFile >> a;	// "width"

		infoFile >> width;	// 1080

		infoFile >> a;	// "height"

		infoFile >> height;	// 768

		infoFile >> a;		// Title_Start

		std::stringstream ssTitle;		// Inside "sstream"
		bool bKeepReading = true;
		do
		{
			infoFile >> a;		// Title_End??
			if (a != "Title_End")
			{
				ssTitle << a << " ";
			}
			else
			{	// it IS the end! 
				bKeepReading = false;
				title = ssTitle.str();
			}
		} while (bKeepReading);


	}

	/////////////////////////////////////////////////////

 //                      _           __  __       _        _               
 //              /\   /\| |/\       |  \/  |     | |      (_)              
 //             /  \  \ ` ' /       | \  / | __ _| |_ _ __ _  ___ ___  ___ 
 //            / /\ \|_     _|      | |\/| |/ _` | __| '__| |/ __/ _ \/ __|
 //           / ____ \/ , . \       | |  | | (_| | |_| |  | | (_|  __/\__ \
 //          /_/    \_\/|_|\/       |_|  |_|\__,_|\__|_|  |_|\___\___||___/
                                                     
               
	//INITIALIZE THE MATRIX TO STORE THE MAZE
	int ** mymaze = new int*[mazex];
	for (int i = 0; i < mazex; i++)
	{
		mymaze[i] = new int[mazez];
	}

	//LOAD THE MAZE
 //           _      ____          _____ _____ _   _  _____          __  __           ____________ 
 //          | |    / __ \   /\   |  __ \_   _| \ | |/ ____|        |  \/  |   /\    |___  /  ____|
 //          | |   | |  | | /  \  | |  | || | |  \| | |  __         | \  / |  /  \      / /| |__   
 //          | |   | |  | |/ /\ \ | |  | || | | . ` | | |_ |        | |\/| | / /\ \    / / |  __|  
 //          | |___| |__| / ____ \| |__| || |_| |\  | |__| |        | |  | |/ ____ \  / /__| |____ 
 //          |______\____/_/    \_\_____/_____|_| \_|\_____|        |_|  |_/_/    \_\/_____|______|
 //                                                                               
                                                                                
	mymaze = loadmaze("maze.txt");
	std::cout << "Maze Matrix: \n";
	for (int i = 0; i < mazex; i++)
	{
		for (int j = 0; j < mazez; j++)
		{
			std::cout << mymaze[i][j]<<"  ";
		}
		std::cout << " \n";
	}  

//  _____                     _                   _______ _                   __  __               
// |  __ \                   (_)                 |__   __| |                 |  \/  |              
// | |  | |_ __ __ ___      ___ _ __   __ _         | |  | |__   ___         | \  / | __ _ _______ 
// | |  | | '__/ _` \ \ /\ / / | '_ \ / _` |        | |  | '_ \ / _ \        | |\/| |/ _` |_  / _ \
// | |__| | | | (_| |\ V  V /| | | | | (_| |        | |  | | | |  __/        | |  | | (_| |/ /  __/
// |_____/|_|  \__,_| \_/\_/ |_|_| |_|\__, |        |_|  |_| |_|\___|        |_|  |_|\__,_/___\___|
//                                     __/ |                                                       
//                                    |___/                                                      
	
	drawmaze(mymaze);



	int objx, objz, destx, destz;
	drawmazeobj(mymaze, objx, objz, destx, destz);




	int objno = size(::g_vecGameObjects) - 1;




	//Initialize H
	int ** H = new int*[mazex];
	for (int i = 0; i < mazex; i++)
	{
		H[i] = new int[mazez];
	}

	int i2,j2;
	for (int i = 0; i < mazex; i++)
	{
		for (int j = 0; j < mazez; j++)
		{
			H[i][j] = 0;
			i2 = i;
			if (i < destx)
			{
				
				while (i2 != destx)
				{
					i2++;
					H[i][j] += 1;
				}
			}
			else
			{
				while (i2 != destx)
				{
					i2--;
					H[i][j] += 1;
				}
			}

			j2 = j;
			if (j < destz)
			{

				while (j2 != destz)
				{
					j2++;
					H[i][j] += 1;
				}
			}
			else
			{
				while (j2 != destz)
				{
					j2--;
					H[i][j] += 1;
				}
			}

		}
	}
	for (int i = 0; i < mazex; i++)
	{
		for (int j = 0; j < mazez; j++)
		{
			if (mymaze[i][j] == 1)
			{
				H[i][j] = -10;
			}
		}
		
	}
	std::cout << "H Matrix: \n";
	for (int i = 0; i < mazex; i++)
	{
		for (int j = 0; j < mazez; j++)
		{
			std::cout << H[i][j] << "  ";
		}
		std::cout << " \n";
	}



	
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// C++ string
	// C no strings. Sorry. char    char name[7] = "Michael\0";
    window = glfwCreateWindow( width, height, 
							   title.c_str(), 
							   NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

	std::cout << glGetString(GL_VENDOR) << " " 
		<< glGetString(GL_RENDERER) << ", " 
		<< glGetString(GL_VERSION) << std::endl;
	std::cout << "Shader language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	
	::g_pShaderManager = new cShaderManager();

	cShaderManager::cShader vertShader;
	cShaderManager::cShader fragShader;

	vertShader.fileName = "simpleVert.glsl";	
	fragShader.fileName = "simpleFrag.glsl"; 

	::g_pShaderManager->setBasePath( "assets//shaders//" );

	// Shader objects are passed by reference so that
	//	we can look at the results if we wanted to. 
	if ( ! ::g_pShaderManager->createProgramFromFile(
		        "mySexyShader", vertShader, fragShader ) )
	{
		std::cout << "Oh no! All is lost!!! Blame Loki!!!" << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		// Should we exit?? 
		return -1;	
//		exit(
	}
	std::cout << "The shaders comipled and linked OK" << std::endl;


	// Load models
	::g_pVAOManager = new cVAOMeshManager();
	LoadPly("PlaneXZ", "Flat_XZ_Plane_xyz.ply");
	LoadPly("SphereRadius1", "Sphereply_xyz.ply");


	GLint currentProgID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	mvp_location = glGetUniformLocation(currentProgID, "MVP");		// program, "MVP");

	glEnable( GL_DEPTH );

	// Gets the "current" time "tick" or "step"
	double lastTimeStep = glfwGetTime();

	// Main game or application loop
	while ( GetKeyState('U')>=0 )
    {
        float ratio;
        int width, height;
        glm::mat4x4 m, p, mvp;			//  mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT );

		unsigned int sizeOfVector = ::g_vecGameObjects.size();	//*****//
		for ( int index = 0; index != sizeOfVector; index++ )
		{
			// Is there a game object? 
			if ( ::g_vecGameObjects[index] == 0 )	//if ( ::g_GameObjects[index] == 0 )
			{	// Nothing to draw
				continue;		// Skip all for loop code and go to next
			}

			// Was near the draw call, but we need the mesh name
			std::string meshToDraw = ::g_vecGameObjects[index]->meshName;		//::g_GameObjects[index]->meshName;

			sVAOInfo VAODrawInfo;
			if ( ::g_pVAOManager->lookupVAOFromName( meshToDraw, VAODrawInfo ) == false )
			{	// Didn't find mesh
				continue;
			}



			// There IS something to draw

			m = glm::mat4x4(1.0f);	//		mat4x4_identity(m);

			glm::mat4 matRreRotZ = glm::mat4x4(1.0f);
			matRreRotZ = glm::rotate( matRreRotZ, ::g_vecGameObjects[index]->orientation.z, 
								     glm::vec3(0.0f, 0.0f, 1.0f) );
			m = m * matRreRotZ;

			glm::mat4 trans = glm::mat4x4(1.0f);
			trans = glm::translate( trans, 
								    ::g_vecGameObjects[index]->position );
			m = m * trans; 

			glm::mat4 matPostRotZ = glm::mat4x4(1.0f);
			matPostRotZ = glm::rotate( matPostRotZ, ::g_vecGameObjects[index]->orientation2.z, 
								     glm::vec3(0.0f, 0.0f, 1.0f) );
			m = m * matPostRotZ;

//			::g_vecGameObjects[index]->orientation2.y += 0.01f;

			glm::mat4 matPostRotY = glm::mat4x4(1.0f);
			matPostRotY = glm::rotate( matPostRotY, ::g_vecGameObjects[index]->orientation2.y, 
								     glm::vec3(0.0f, 1.0f, 0.0f) );
			m = m * matPostRotY;


			glm::mat4 matPostRotX = glm::mat4x4(1.0f);
			matPostRotX = glm::rotate( matPostRotX, ::g_vecGameObjects[index]->orientation2.x, 
								     glm::vec3(1.0f, 0.0f, 0.0f) );
			m = m * matPostRotX;
			
			float finalScale = ::g_vecGameObjects[index]->scale;

			glm::mat4 matScale = glm::mat4x4(1.0f);
			matScale = glm::scale( matScale, 
								   glm::vec3( finalScale,
								              finalScale,
								              finalScale ) );
			m = m * matScale;

			p = glm::perspective( 0.6f,			// FOV
								  ratio,		// Aspect ratio
								  0.1f,			// Near (as big as possible)
								  1000.0f );	// Far (as small as possible)

		
			// View or "camera" matrix
			glm::mat4 v = glm::mat4(1.0f);	// identity



			//Camera Handler
			glm::vec3 g_cameraXYZ = { ::g_vecGameObjects[objno]->position.x + camerafactor*cosf(theta), -0.8*camerafactor,::g_vecGameObjects[objno]->position.z + camerafactor * sin(theta) };	// 5 units "down" z
			glm::vec3 g_cameraTarget_XYZ =::g_vecGameObjects[objno]->position;
			
			glm::vec3 vectorf;
			vectorf = g_cameraXYZ - g_cameraTarget_XYZ;
			//std::cout << vectorf.x << "," << vectorf.y << "," << vectorf.z << std::endl;

				



			//glm::vec3 cameraXYZ = glm::vec3( 0.0f, 0.0f, 5.0f );	// 5 units "down" z
			v = glm::lookAt( g_cameraXYZ ,						// "eye" or "camera" position
							 g_cameraTarget_XYZ,		// "At" or "target" 
							 glm::vec3( 0.0f, -1.0f, 0.0f ) );	// "up" vector

			mvp = p * v * m;			// This way (sort of backwards)

			GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName("mySexyShader");
			GLint diffuseColour_loc = glGetUniformLocation( shaderID, "diffuseColour" );

			glUniform4f( diffuseColour_loc, 
						::g_vecGameObjects[index]->diffuseColour.r, 
						::g_vecGameObjects[index]->diffuseColour.g, 
						::g_vecGameObjects[index]->diffuseColour.b, 
						::g_vecGameObjects[index]->diffuseColour.a );


	//        glUseProgram(program);
			::g_pShaderManager->useShaderProgram( "mySexyShader" );

			
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, 
							   (const GLfloat*) glm::value_ptr(mvp) );

	//		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	

				glBindVertexArray( VAODrawInfo.VAO_ID );

				glDrawElements( GL_TRIANGLES, 
								VAODrawInfo.numberOfIndices,		// testMesh.numberOfTriangles * 3,	// How many vertex indices
								GL_UNSIGNED_INT,					// 32 bit int 
								0 );

				glBindVertexArray( 0 );

//			}// if ( ::g_pVAOManager->lookupVAOFromName(

		}//for ( int index = 0...


		std::stringstream ssTitle;
		ssTitle << "PRESS SPACE BAR TO STEP ONE STEP FURTHER USING A*";
		glfwSetWindowTitle( window, ssTitle.str().c_str() );

        glfwSwapBuffers(window);
        glfwPollEvents();


		// Essentially the "frame time"
		// Now many seconds that have elapsed since we last checked
		double curTime = glfwGetTime();
		double deltaTime =  curTime - lastTimeStep;
		//std::cout << g_vecGameObjects[objno]->position.x<<" ," << g_vecGameObjects[objno]->position.z<<std::endl;


		//Press Space Bar to Furthur each step
		if(GetKeyState(VK_SPACE)<0 )
		{
			if ((int)g_vecGameObjects[objno]->position.x != destx || (int)g_vecGameObjects[objno]->position.z != destz)
			{
				MoveTo(objno, mymaze, destz, destz, H);
				Sleep(100);
			}
		}
		
		//std::cout << g_vecGameObjects[objno]->position.z << std::endl;

		

		//std::cout << "\nCurrent Location: " << ::g_vecGameObjects[objno]->position.x << "," << ::g_vecGameObjects[objno]->position.z << std::endl;

		//std::cout << "\nDestination Location: " << destx << "," << destz << std::endl;

		handlekeys((float)deltaTime);
		
		//PhysicsStep( deltaTime );

		lastTimeStep = curTime;

    }// while ( ! glfwWindowShouldClose(window) )





	//Sleep(1000000000000);

    glfwDestroyWindow(window);
    glfwTerminate();

	// 
	delete ::g_pShaderManager;
	delete ::g_pVAOManager;

//    exit(EXIT_SUCCESS);



	return 0;
}


