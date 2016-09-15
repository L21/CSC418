/***********************************************************
             CSC418, FALL 2009
 
                 penguin.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

		Main source file for assignment 2
		Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
		
		Be sure to also look over keyframe.h and vector.h.
		While no changes are necessary to these files, looking
		them over will allow you to better understand their
		functionality and capabilites.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "keyframe.h"
#include "timer.h"
#include "vector.h"


// *************** GLOBAL VARIABLES *************************

const float PI = 3.14159;

const float SPINNER_SPEED = 1.0;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_joints;			// Glui window with joint controls
GLUI* glui_keyframe;		// Glui window with keyframe controls
GLUI* glui_render;			// Glui window for render style

char msg[256];				// String used for status message
GLUI_StaticText* status;	// Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -7.5;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

// Render settings
enum { WIREFRAME, SOLID, OUTLINED, MATTE, METALLIC};	// README: the different render styles
int renderStyle = WIREFRAME;			// README: the selected render style

// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate
float LIGHT = 0;
// Keyframe settings
const char filenameKF[] = "keyframes.txt";	// file for loading / saving keyframes

Keyframe* keyframes;			// list of keyframes

int maxValidKeyframe   = 0;		// index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 32;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

int frameNumber = 0;			// current frame being dumped
int frameToFile = 0;			// flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 10.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

// README: To change the range of a particular DOF,
// simply change the appropriate min/max values below
const float ROOT_TRANSLATE_X_MIN = -5.0;
const float ROOT_TRANSLATE_X_MAX =  5.0;
const float ROOT_TRANSLATE_Y_MIN = -5.0;
const float ROOT_TRANSLATE_Y_MAX =  5.0;
const float ROOT_TRANSLATE_Z_MIN = -5.0;
const float ROOT_TRANSLATE_Z_MAX =  5.0;
const float ROOT_ROTATE_X_MIN    = -180.0;
const float ROOT_ROTATE_X_MAX    =  180.0;
const float ROOT_ROTATE_Y_MIN    = -180.0;
const float ROOT_ROTATE_Y_MAX    =  180.0;
const float ROOT_ROTATE_Z_MIN    = -180.0;
const float ROOT_ROTATE_Z_MAX    =  180.0;
const float HEAD_MIN             = -180.0;
const float HEAD_MAX             =  180.0;
const float SHOULDER_PITCH_MIN   = -45.0;
const float SHOULDER_PITCH_MAX   =  45.0;
const float SHOULDER_YAW_MIN     =  0.0;
const float SHOULDER_YAW_MAX     =  45.0;
const float SHOULDER_ROLL_MIN    = -45.0;
const float SHOULDER_ROLL_MAX    =  45.0;
const float HIP_PITCH_MIN        = -45.0;
const float HIP_PITCH_MAX        =  45.0;
const float HIP_YAW_MIN          = -45.0;
const float HIP_YAW_MAX          =  45.0;
const float HIP_ROLL_MIN         = -45.0;
const float HIP_ROLL_MAX         =  45.0;
const float BEAK_MIN             =  0.0;
const float BEAK_MAX             =  0.1;
const float ELBOW_MIN            =  -75.0;
const float ELBOW_MAX            = 0.0;
const float KNEE_MIN             =  0.0;
const float KNEE_MAX             = 75.0;
const float LIGHT_MIN            = -PI;
const float LIGHT_MAX            = PI;


// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void reshape(int w, int h);
void animate();
void display(void);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);


// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);
void drawBody();
void drawHead();
void drawPenguin();
void drawbeak();
void drawL_Arm();
void drawR_Arm();
void drawelbow();
void drawL_Leg();
void drawR_Leg();
void drawL_Knee();
void drawR_Knee();
void lighting_Matte();
void lighting_Metallic();
// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640;
        Win[1] = 480;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize data structs, glut, glui, and opengl
	initDS();
    initGlut(argc, argv);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
	glutMouseFunc(mouse);		// Call mouse whenever mouse button pressed
	glutMotionFunc(motion);		// Call motion whenever mouse moves while button pressed
}


// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	*joint_ui_data = keyframes[keyframeID];

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
	status->set_text(msg);
}

// Update Keyframe button handler. Called when the "update keyframe" button is pressed
void updateKeyframeButton(int)
{
	///////////////////////////////////////////////////////////
	// TODO:
	//   Modify this function to save the UI joint values into
	//   the appropriate keyframe entry in the keyframe list
	//   when the user clicks on the 'Update Keyframe' button.
	//   Refer to the 'loadKeyframeButton' function for help.
	///////////////////////////////////////////////////////////

	// Get the keyframe ID from the UI
	int keyframeID = 0;
	keyframeID = joint_ui_data->getID();
	

	// Update the 'maxValidKeyframe' index variable
	// (it will be needed when doing the interpolation)
	if (keyframeID > maxValidKeyframe) {
		maxValidKeyframe = keyframeID;
	}
	
	keyframes[keyframeID].setTime(joint_ui_data->getTime());
	keyframes[keyframeID].setID(keyframeID);
	
	// Update the appropriate entry in the 'keyframes' array
	// with the 'joint_ui_data' data
	int index = 0;
	for (index = 0 ; index < 24; index++) {
		keyframes[keyframeID].setDOF(index, joint_ui_data->getDOF(index));
	}
	// Let the user know the values have been updated
	sprintf(msg, "Status: Keyframe %d updated successfully", keyframeID);
	status->set_text(msg);
}

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton(int)
{
	// Open file for reading
	FILE* file = fopen(filenameKF, "r");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Read in maxValidKeyframe first
	fscanf(file, "%d", &maxValidKeyframe);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fscanf(file, "%d", keyframes[i].getIDPtr());
		fscanf(file, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been loaded
	sprintf(msg, "Status: Keyframes loaded successfully");
	status->set_text(msg);
}

// Save Keyframes To File button handler. Called when the "save keyframes to file" button is pressed
void saveKeyframesToFileButton(int)
{
	// Open file for writing
	FILE* file = fopen(filenameKF, "w");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Write out maxValidKeyframe first
	fprintf(file, "%d\n", maxValidKeyframe);
	fprintf(file, "\n");

	// Now write out all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fprintf(file, "%d\n", keyframes[i].getID());
		fprintf(file, "%f\n", keyframes[i].getTime());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fprintf(file, "%f\n", keyframes[i].getDOF(j));

		fprintf(file, "\n");
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been saved
	sprintf(msg, "Status: Keyframes saved successfully");
	status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Status: Animating...");
	status->set_text(msg);
  }
  else
  {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	// Let the user know the animation has stopped
	sprintf(msg, "Status: Animation stopped");
	status->set_text(msg);
  }
}

// Render Frames To File button handler. Called when the "Render Frames To File" button is pressed.
void renderFramesToFileButton(int)
{
	// Calculate number of frames to generate based on dump frame rate
	int numFrames = int(keyframes[maxValidKeyframe].getTime() * DUMP_FRAME_PER_SEC) + 1;

	// Generate frames and save to file
	frameToFile = 1;
	for( frameNumber = 0; frameNumber < numFrames; frameNumber++ )
	{
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(frameNumber * DUMP_SEC_PER_FRAME) );

		// Let the user know which frame is being rendered
		sprintf(msg, "Status: Rendering frame %d...", frameNumber);
		status->set_text(msg);

		// Render the frame
		display();
	}
	frameToFile = 0;

	// Let the user know how many frames were generated
	sprintf(msg, "Status: %d frame(s) rendered to file", numFrames);
	status->set_text(msg);
}

// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Panel* glui_panel;
	GLUI_Spinner* glui_spinner;
	GLUI_RadioGroup* glui_radio_group;

    GLUI_Master.set_glutIdleFunc(NULL);


	// Create GLUI window (joint controls) ***************
	//
	glui_joints = GLUI_Master.create_glui("Joint Control", 0, Win[0]+12, 0);

    // Create controls to specify root position and orientation
	glui_panel = glui_joints->add_panel("Root");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_X));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_X_MIN, ROOT_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Y));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Y_MIN, ROOT_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Z));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Z_MIN, ROOT_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_X));
	glui_spinner->set_float_limits(ROOT_ROTATE_X_MIN, ROOT_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Y));
	glui_spinner->set_float_limits(ROOT_ROTATE_Y_MIN, ROOT_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Z));
	glui_spinner->set_float_limits(ROOT_ROTATE_Z_MIN, ROOT_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify head rotation
	glui_panel = glui_joints->add_panel("Head");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "head:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::HEAD));
	glui_spinner->set_float_limits(HEAD_MIN, HEAD_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify beak
	glui_panel = glui_joints->add_panel("Beak");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "beak:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BEAK));
	glui_spinner->set_float_limits(BEAK_MIN, BEAK_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right arm
	glui_panel = glui_joints->add_panel("Right arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left arm
	glui_panel = glui_joints->add_panel("Left arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right leg
	glui_panel = glui_joints->add_panel("Right leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left leg
	glui_panel = glui_joints->add_panel("Left leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_panel = glui_joints->add_panel("Light");
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "light:", GLUI_SPINNER_FLOAT, &LIGHT);
	glui_spinner->set_float_limits(LIGHT_MIN, LIGHT_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(0.1);



	///////////////////////////////////////////////////////////
	// TODO (for controlling light source position & additional
	//      rendering styles):
	//   Add more UI spinner elements here. Be sure to also
	//   add the appropriate min/max range values to this
	//   file, and to also add the appropriate enums to the
	//   enumeration in the Keyframe class (keyframe.h).
	///////////////////////////////////////////////////////////

	//
	// ***************************************************


	// Create GLUI window (keyframe controls) ************
	//
	glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

	// Create a control to specify the time (for setting a keyframe)
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
	glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create a control to specify a keyframe (for updating / loading a keyframe)
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
	glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_keyframe->add_separator();

	// Add buttons to load and update keyframes
	// Add buttons to load and save keyframes from a file
	// Add buttons to start / stop animation and to render frames to file
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframes From File", 0, loadKeyframesFromFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

	glui_keyframe->add_separator();

	// Add status line
	glui_panel = glui_keyframe->add_panel("");
	status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

	// Add button to quit
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
	//
	// ***************************************************


	// Create GLUI window (render controls) ************
	//
	glui_render = GLUI_Master.create_glui("Render Control", 0, 367, Win[1]+64);

	// Create control to specify the render style
	glui_panel = glui_render->add_panel("Render Style");
	glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle);
	glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid w/ outlines");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Matte");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Metallic");

	//
	// ***************************************************


	// Tell GLUI windows which window is main graphics window
	glui_joints->set_main_gfx_window(windowID);
	glui_keyframe->set_main_gfx_window(windowID);
	glui_render->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes bewteen which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe )
		return keyframes[maxValidKeyframe].getDOFVector();

	// Need to normalize time to (0, 1]
	float alpha = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points
	// for computing the interpolation
	Vector p0 = keyframes[i-1].getDOFVector();
	Vector p1 = keyframes[i].getDOFVector();

	//this interpolation code is from github public source,  

	Vector t0, t1;
	if( i == 1 ) {
		t0 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}
	else if( i == maxValidKeyframe ) {
		t0 = (keyframes[i].getDOFVector() - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
	} else {
		t0 = (keyframes[i].getDOFVector()   - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}

	// Return the interpolated Vector
	Vector a0 = p0;
	Vector a1 = t0;
	Vector a2 = p0 * (-3) + p1 * 3 + t0 * (-2) + t1 * (-1);
	Vector a3 = p0 * 2 + p1 * (-2) + t0 + t1;

	return (((a3 * alpha + a2) * alpha + a1) * alpha + a0);

	///////////////////////////////////////////////////////////
	// TODO (animation using Catmull-Rom):
	//   Currently the code operates using linear interpolation
    //   Modify this function so it uses Catmull-Rom instead.
	///////////////////////////////////////////////////////////


}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object (once you've written
		// the callback).
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}



// display callback
//
// README: This gets called by the event handler
// to draw the scene, so this is where you need
// to build your scene -- make your changes and
// additions here. All rendering happens in this
// function. For Assignment 2, updates to the
// joint DOFs (joint_ui_data) happen in the
// animate() function.
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);


	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

		///////////////////////////////////////////////////////////
		// README:
		//   This statement loads the interpolated joint DOF vector
		//   into the global 'joint_ui_data' variable. Use the
		//   'joint_ui_data' variable below in your model code to
		//   drive the model for animation.
		///////////////////////////////////////////////////////////
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

		// Update user interface
		joint_ui_data->setTime(curTime);
		glui_keyframe->sync_live();
	}


    ///////////////////////////////////////////////////////////
    // TODO:
	//   Modify this function to draw the scene.
	//   This should include function calls that apply
	//   the appropriate transformation matrices and render
	//   the individual body parts.
	//   Use the 'joint_ui_data' data structure to obtain
	//   the joint DOFs to specify your transformations.
	//   Sample code is provided below and demonstrates how
	//   to access the joint DOF values. This sample code
	//   should be replaced with your own.
	//   Use the 'renderStyle' variable and the associated
	//   enumeration to determine how the geometry should be
	//   rendered.
    ///////////////////////////////////////////////////////////


	//draw the penguin
	drawPenguin();

	if (renderStyle == MATTE or renderStyle == METALLIC) {
		//if the render style we choose is MATTE or METALLIC, then we should disable the light
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
	// }
    // Execute any GL functions that are in the queue just to be safe
    glFlush();

	// Dump frame to file, if requested
	if( frameToFile )
	{
		sprintf(filenameF, "frame%03d.ppm", frameNumber);
		writeFrame(filenameF, false, false);
	}

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( button == GLUT_RIGHT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			lastX = x;
			lastY = y;
			updateCamZPos = true;
		}
		else
		{
			updateCamZPos = false;
		}
	}
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( updateCamZPos )
	{
		// Update camera z position
		camZPos += (x - lastX) * ZOOM_SCALE;
		lastX = x;

		// Redraw the scene from updated camera position
		glutSetWindow(windowID);
		glutPostRedisplay();
	}
}
void drawPenguin() 
{
	glPushMatrix();

		
		
		// determine render style and set glPolygonMode appropriately
		if (renderStyle == SOLID) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 1);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glColor3f(96.0/255.0, 96.0/255.0, 96.0/255.0);
		} else if (renderStyle == WIREFRAME) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glColor3f(0.0, 0.0, 0.0);
		} else if (renderStyle == MATTE) {
   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   			//enable the corresponding light
		   	lighting_Matte();
		} else if (renderStyle == METALLIC){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//enable the corresponding light
		   	lighting_Metallic();
		}

		// setup transformation for body part
		glTranslatef(joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Z));
		

		// setup rotation for body part
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_X), 1.0, 0.0, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y), 0.0, 1.0, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z), 0.0, 0.0, 1.0);
		
		//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
		//it again with wireflame
		if (renderStyle == OUTLINED) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 1);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glColor3f(96.0/255.0, 96.0/255.0, 96.0/255.0);
			drawBody();
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glColor3f(0.0, 0.0, 0.0);
		}
		drawBody();

		//head
		glPushMatrix();

			// setup transformation and rotation for head
			glTranslatef(0.0, 1.75, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::HEAD), 0.0, 1.0, 0.0);

			// determine render style and set glPolygonMode appropriately
			if (renderStyle == SOLID) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(1.0, 1.0, 1.0);
			} else if (renderStyle == WIREFRAME) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			} else if (renderStyle == MATTE or renderStyle == METALLIC) {
	   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			 	
			} else if (renderStyle == OUTLINED) {

				//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
				//it again with wireflame
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(1.0, 1.0, 1.0);
				drawHead();
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			}
			drawHead();

			//mouth
			glPushMatrix();
			// determine render style and set glPolygonMode appropriately
				if (renderStyle == SOLID) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1.0, 1.0, 153.0/255.0);
				} else if (renderStyle == WIREFRAME) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				} else if (renderStyle == MATTE or renderStyle == METALLIC) {
		   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					
				} else if (renderStyle == OUTLINED) {

					//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
					//it again with wireflame
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1.0, 1.0, 153.0/255.0);
					drawbeak();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				}
				
				drawbeak();


				// setup transformation and rotation for beak, 
				glTranslatef(0.0, -0.1, 0.0);
				glScalef(1.0, -1.0, 1.0);
				glTranslatef(0.0, joint_ui_data->getDOF(Keyframe::BEAK), 0.0);

				//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
				//it again with wireflame
				if (renderStyle == OUTLINED) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1.0, 1.0, 153.0/255.0);
					drawbeak();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				}
				drawbeak();

			glPopMatrix();


		glPopMatrix();


		//left arm
		glPushMatrix();

			// determine render style and set glPolygonMode appropriately
			if (renderStyle == SOLID) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(1, 102.0/255.0, 102.0/255.0);
			} else if (renderStyle == WIREFRAME) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			} else if (renderStyle == MATTE or renderStyle == METALLIC) {
	   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			 	
			}

			// setup transformation and rotation for left arm, 
			glTranslatef(0.0, 0.75, 0.5);
			glRotatef(-joint_ui_data->getDOF(Keyframe::L_SHOULDER_YAW), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_ROLL), 0.0, 1.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_PITCH), 0.0, 0.0, 1.0);

			//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
			//it again with wireflame
			if (renderStyle == OUTLINED) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(1, 102.0/255.0, 102.0/255.0);
				drawL_Arm();
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			}
			
			drawL_Arm();

			//left elbow
			glPushMatrix();
				// determine render style and set glPolygonMode appropriately
				if (renderStyle == SOLID) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1, 153.0/255.0, 153.0/255.0);
				} else if (renderStyle == WIREFRAME) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				} else if (renderStyle == MATTE or renderStyle == METALLIC) {
			   		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				 	
				} 

				// setup transformation and rotation for left elbow, 
				glTranslatef(0.0, -0.9, 0.0);
				glRotatef(joint_ui_data->getDOF(Keyframe::L_ELBOW), 0.0, 0.0, 1.0);

				//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
				//it again with wireflame
				if (renderStyle == OUTLINED) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1, 153.0/255.0, 153.0/255.0);
					drawelbow();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				}
				drawelbow();

			glPopMatrix();

		glPopMatrix();




		//right arm
		glPushMatrix();

			// determine render style and set glPolygonMode appropriately
			if (renderStyle == SOLID) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(1, 102.0/255.0, 102.0/255.0);
			} else if (renderStyle == WIREFRAME) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			} else if (renderStyle == MATTE or renderStyle == METALLIC) {
	   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			  	
			}
			// setup transformation and rotation for right arm, 
			glTranslatef(0.0, 0.75, -0.5);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_YAW), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_ROLL), 0.0, 1.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_PITCH), 0.0, 0.0, 1.0);

			//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
			//it again with wireflame
			if (renderStyle == OUTLINED) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(1, 102.0/255.0, 102.0/255.0);
				drawR_Arm();
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			}

			
			drawR_Arm();

			glPushMatrix();

			// determine render style and set glPolygonMode appropriately
				if (renderStyle == SOLID) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1, 153.0/255.0, 153.0/255.0);
				} else if (renderStyle == WIREFRAME) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				} else if (renderStyle == MATTE or renderStyle == METALLIC) {
		   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				  	
				}

				// setup transformation and rotation for right elbow
				glTranslatef(0.0, -0.9, -0.1);
				glRotatef(joint_ui_data->getDOF(Keyframe::R_ELBOW), 0.0, 0.0, 1.0);

				//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
				//it again with wireflame
				if (renderStyle == OUTLINED) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(1, 153.0/255.0, 153.0/255.0);
					drawelbow();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				}
				
				drawelbow();

			glPopMatrix();

		glPopMatrix();

		//left leg
		glPushMatrix();

			// determine render style and set glPolygonMode appropriately
			if (renderStyle == SOLID) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(0.0, 128.0/255.0, 1.0);
			} else if (renderStyle == WIREFRAME) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			} else if (renderStyle == MATTE or renderStyle == METALLIC) {
	   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			 	
			}

			// setup transformation and rotation for left leg
			glTranslatef(0.0, -1.3, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_YAW), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_ROLL), 0.0, 1.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_PITCH), 0.0, 0.0, 1.0);

			//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
			//it again with wireflame
			if (renderStyle == OUTLINED) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(0.0, 128.0/255.0, 1.0);
				drawL_Leg();
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			}
			
			drawL_Leg();

			glPushMatrix();

				// determine render style and set glPolygonMode appropriately
				if (renderStyle == SOLID) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(0.0, 0.0, 153.0/255.0);
				} else if (renderStyle == WIREFRAME) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				} else if (renderStyle == MATTE or renderStyle == METALLIC) {
		   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				  	
				}

				// setup transformation and rotation for left knee
				glTranslatef(0.0, -1.5, 0.0);
				glRotatef(joint_ui_data->getDOF(Keyframe::L_KNEE), 0.0, 0.0, 1.0);

				//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
				//it again with wireflame
				if (renderStyle == OUTLINED) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(0.0, 0.0, 153.0/255.0);
					drawL_Knee();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);

				}
				
				drawL_Knee();

			glPopMatrix();


			

		glPopMatrix();

		//right leg
		glPushMatrix();

			// determine render style and set glPolygonMode appropriately
			if (renderStyle == SOLID) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(0.0, 128.0/255.0, 1.0);
			} else if (renderStyle == WIREFRAME) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			} else if (renderStyle == MATTE or renderStyle == METALLIC) {
	   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			  	
			}

			// setup transformation and rotation for right leg
			glTranslatef(0.0, -1.3, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_YAW), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_ROLL), 0.0, 1.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_PITCH), 0.0, 0.0, 1.0);


			//if the renderstyle is outlined, then draw the penguin with solid first, then draw 
			//it again with wireflame
			if (renderStyle == OUTLINED) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 1);
				glDisable(GL_POLYGON_OFFSET_FILL);
				glColor3f(0.0, 128.0/255.0, 1.0);
				drawR_Leg();
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
			}
			
			drawR_Leg();

			glPushMatrix();

				// determine render style and set glPolygonMode appropriately
				if (renderStyle == SOLID) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(0.0, 0.0, 153.0/255.0);
				} else if (renderStyle == WIREFRAME) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				} else if (renderStyle == MATTE or renderStyle == METALLIC) {
		   			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					
				}

				// setup transformation and rotation for right elbow
				glTranslatef(0.0, -1.5, 0.0);
				glRotatef(joint_ui_data->getDOF(Keyframe::R_KNEE), 0.0, 0.0, 1.0);
				if (renderStyle == OUTLINED) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 1);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glColor3f(0.0, 0.0, 153.0/255.0);
					drawR_Knee();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
				}
				
				drawR_Knee();

			glPopMatrix();


			

		glPopMatrix();





	glPopMatrix();
}

// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube
void drawBody() 
{
	glBegin(GL_QUADS);
		// draw front body
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-1.0, -1.5, 0.5);
		glVertex3f( 1.0, -1.5, 0.5);
		glVertex3f( 0.5,  1.5, 0.5);
		glVertex3f(-0.5,  1.5, 0.5);

		// draw back body
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-1.0, -1.5, -0.5);
		glVertex3f(-0.5,  1.5, -0.5);
		glVertex3f( 0.5,  1.5, -0.5);
		glVertex3f( 1.0, -1.5, -0.5);

		// draw left body 
		glNormal3f(-0.986394, 0.164399, 0.0);
		glVertex3f(-1.0, -1.5, -0.5);
		glVertex3f(-1.0, -1.5, 0.5);
		glVertex3f(-0.5,  1.5, 0.5);
		glVertex3f(-0.5,  1.5, -0.5);

		// draw right body
		glNormal3f(0.986394, 0.164399, 0.0);
		glVertex3f( 1.0, -1.5, 0.5);
		glVertex3f( 1.0, -1.5, -0.5);
		glVertex3f( 0.5,  1.5, -0.5);
		glVertex3f( 0.5,  1.5, 0.5);

		// draw up body face
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.5,  1.5, 0.5);
		glVertex3f( 0.5,  1.5, 0.5);
		glVertex3f( 0.5,  1.5, -0.5);
		glVertex3f(-0.5,  1.5, -0.5);

		// draw bottom face
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-1.0, -1.5, 0.5);
		glVertex3f(-1.0, -1.5, -0.5);
		glVertex3f(1.0, -1.5, -0.5);
		glVertex3f(1.0, -1.5,  0.5);
	glEnd();
}


void drawHead()
{
	glBegin(GL_QUADS);
		// draw front head
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-0.6, -0.3, 0.5);
		glVertex3f( 0.6, -0.3, 0.5);
		glVertex3f( 0.5,  0.3, 0.5);
		glVertex3f(-0.5,  0.3, 0.5);

		// draw back head
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-0.6, -0.3, -0.5);
		glVertex3f(-0.5,  0.3, -0.5);
		glVertex3f( 0.5,  0.3, -0.5);
		glVertex3f( 0.6, -0.3, -0.5);

		// draw left head
		glNormal3f(-0.986394, 0.164399, 0.0);
		glVertex3f(-0.6, -0.3, -0.5);
		glVertex3f(-0.6, -0.3, 0.5);
		glVertex3f(-0.5,  0.3, 0.5);
		glVertex3f(-0.5,  0.3, -0.5);

		// draw right head
		glNormal3f(0.986394, 0.164399, 0.0);
		glVertex3f( 0.6, -0.3, 0.5);
		glVertex3f( 0.6, -0.3, -0.5);
		glVertex3f( 0.5,  0.3, -0.5);
		glVertex3f( 0.5,  0.3, 0.5);

		// draw up body head
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.5,  0.3, 0.5);
		glVertex3f( 0.5,  0.3, 0.5);
		glVertex3f( 0.5,  0.3, -0.5);
		glVertex3f(-0.5,  0.3, -0.5);

		// draw bottom head
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-0.6, -0.3, 0.5);
		glVertex3f(-0.6, -0.3, -0.5);
		glVertex3f(0.6, -0.3, -0.5);
		glVertex3f(0.6, -0.3,  0.5);
	glEnd();
}

void drawbeak() {
	glBegin(GL_QUADS);
		// draw front beak
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f( -1, 0.0, 0.25);
		glVertex3f( -0.55, 0.0, 0.25);
		glVertex3f( -0.55,  0.1, 0.25);
		glVertex3f( -0.9,  0.1, 0.25);

		// draw back beak
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f( -1, 0.0, -0.25);
		glVertex3f( -0.9,  0.1, -0.25);
		glVertex3f( -0.55,  0.1, -0.25);
		glVertex3f( -0.55, 0.0, -0.25);

		// draw left beak
		glNormal3f(-0.707107, 0.707107, 0.0);
		glVertex3f( -1, 0.0, 0.25);
		glVertex3f( -0.9,  0.1, 0.25);
		glVertex3f( -0.9,  0.1, -0.25);
		glVertex3f( -1, 0.0, -0.25);

		// draw right beak
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f( -0.55, 0.0, 0.25);
		glVertex3f( -0.55, 0.0, -0.25);
		glVertex3f( -0.55,  0.1, -0.25);
		glVertex3f( -0.55,  0.1, 0.25);

		// draw up beak
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.9,  0.1, 0.25);
		glVertex3f( -0.55,  0.1, 0.25);
		glVertex3f( -0.55,  0.1, -0.25);
		glVertex3f(-0.9,  0.1, -0.25);

		// draw bottom beak
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-1, 0.0, 0.25);
		glVertex3f(-1, 0.0, -0.25);
		glVertex3f(-0.55, 0.0, -0.25);
		glVertex3f(-0.55, 0.0, 0.25);
	glEnd();
}

void drawL_Arm()
{
	glBegin(GL_QUADS);
		// draw front arm
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-0.2, -1.0, 0.1);
		glVertex3f( 0.2, -1.0, 0.1);
		glVertex3f( 0.4,  0.0, 0.1);
		glVertex3f(-0.4,  0.0, 0.1);

		// draw back arm
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-0.2, -1.0, 0.0);
		glVertex3f(-0.4,  0.0, 0.0);
		glVertex3f( 0.4,  0.0, 0.0);
		glVertex3f( 0.2, -1.0, 0.0);

		// draw left arm
		glNormal3f(-0.980581, -0.196116, 0.0);
		glVertex3f(-0.2, -1.0, 0.1);
		glVertex3f(-0.4,  0.0, 0.1);
		glVertex3f(-0.4,  0.0, 0.0);
		glVertex3f(-0.2, -1.0, 0.0);

		// draw right arm
		glNormal3f(0.980581, -0.196116, 0.0);
		glVertex3f( 0.2, -1.0, 0.1);
		glVertex3f( 0.2, -1.0, 0.0);
		glVertex3f( 0.4,  0.0, 0.0);
		glVertex3f( 0.4,  0.0, 0.1);

		// draw up body arm
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.4,  0.0, 0.1);
		glVertex3f( 0.4,  0.0, 0.1);
		glVertex3f( 0.4,  0.0, 0.0);
		glVertex3f(-0.4,  0.0, 0.0);

		// draw bottom arm
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-0.2, -1.0, 0.1);
		glVertex3f(-0.2, -1.0, 0.0);
		glVertex3f(0.2, -1.0, 0.0);
		glVertex3f(0.2, -1.0,  0.1);
	glEnd();
}

void drawelbow(){
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-0.2, -0.4, 0.1);
		glVertex3f( 0.2, -0.4, 0.1);
		glVertex3f( 0.2,  0.0, 0.1);
		glVertex3f(-0.2,  0.0, 0.1);

		// draw back elbow
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-0.2, -0.4, 0.0);
		glVertex3f(-0.2,  0.0, 0.0);
		glVertex3f( 0.2,  0.0, 0.0);
		glVertex3f( 0.2, -0.4, 0.0);

		// draw left elbow
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-0.2, -0.4, 0.1);
		glVertex3f(-0.2,  0.0, 0.1);
		glVertex3f(-0.2,  0.0, 0.0);
		glVertex3f(-0.2, -0.4, 0.0);

		// draw right elbow
		glNormal3f( 1.0, 0.0, 0.0);
		glVertex3f( 0.2, -0.4, 0.1);
		glVertex3f( 0.2, -0.4, 0.0);
		glVertex3f( 0.2,  0.0, 0.0);
		glVertex3f( 0.2,  0.0, 0.1);

		// draw up elbow
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.2,  0.0, 0.1);
		glVertex3f( 0.2,  0.0, 0.1);
		glVertex3f( 0.2,  0.0, 0.0);
		glVertex3f(-0.2,  0.0, 0.0);

		// draw bottom elbow
		glNormal3f( 0.0, -1.0, 0.0);
		glVertex3f(-0.2, -0.4, 0.1);
		glVertex3f(-0.2, -0.4, 0.0);
		glVertex3f( 0.2, -0.4, 0.0);
		glVertex3f( 0.2, -0.4, 0.1);
	glEnd();

}

void drawR_Arm()
{
	glBegin(GL_QUADS);
		// draw front arm
		glNormal3f( 0.0, 0.0, 1.0);
		glVertex3f(-0.2, -1.0, 0.0);
		glVertex3f(-0.4,  0.0, 0.0);
		glVertex3f( 0.4,  0.0, 0.0);
		glVertex3f( 0.2, -1.0, 0.0);

		// draw back arm
		glNormal3f( 0.0, 0.0, -1.0);
		
		glVertex3f(-0.2, -1.0, -0.1);
		glVertex3f( 0.2, -1.0, -0.1);
		glVertex3f( 0.4,  0.0, -0.1);
		glVertex3f(-0.4,  0.0, -0.1);

		// draw left arm
		glNormal3f(-0.980581, -0.196116, 0.0);
		glVertex3f(-0.2, -1.0, 0.0);
		glVertex3f(-0.4,  0.0, 0.0);
		glVertex3f(-0.4,  0.0, -0.1);
		glVertex3f(-0.2, -1.0, -0.1);

		// draw right arm
		glNormal3f(0.980581, -0.196116, 0.0);
		glVertex3f( 0.2, -1.0, -0.1);
		glVertex3f( 0.2, -1.0, 0.0);
		glVertex3f( 0.4,  0.0, 0.0);
		glVertex3f( 0.4,  0.0, -0.1);

		// draw up body arm
		glNormal3f( 0.0, 1.0, 0.0);
		glVertex3f(-0.4,  0.0, -0.1);
		glVertex3f( 0.4,  0.0, -0.1);
		glVertex3f( 0.4,  0.0, 0.0);
		glVertex3f(-0.4,  0.0, 0.0);

		// draw bottom arm
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-0.2, -1.0, -0.1);
		glVertex3f(-0.2, -1.0, 0.0);
		glVertex3f(0.2, -1.0, 0.0);
		glVertex3f(0.2, -1.0,  -0.1);
	glEnd();
}
void drawL_Leg(){
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-0.2, -1.5, 0.4);
		glVertex3f( 0.2, -1.5, 0.4);
		glVertex3f( 0.2,  0.0, 0.4);
		glVertex3f(-0.2,  0.0, 0.4);

		// draw back leg
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-0.2, -1.5, 0.2);
		glVertex3f(-0.2,  0.0, 0.2);
		glVertex3f( 0.2,  0.0, 0.2);
		glVertex3f( 0.2, -1.5, 0.2);

		// draw left leg
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-0.2, -1.5, 0.4);
		glVertex3f(-0.2,  0.0, 0.4);
		glVertex3f(-0.2,  0.0, 0.2);
		glVertex3f(-0.2, -1.5, 0.2);

		// draw right leg
		glNormal3f( 1.0, 0.0, 0.0);
		glVertex3f( 0.2, -1.5, 0.4);
		glVertex3f( 0.2, -1.5, 0.2);
		glVertex3f( 0.2,  0.0, 0.2);
		glVertex3f( 0.2,  0.0, 0.4);

		// draw up leg
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.2,  0.0, 0.4);
		glVertex3f( 0.2,  0.0, 0.4);
		glVertex3f( 0.2,  0.0, 0.2);
		glVertex3f(-0.2,  0.0, 0.2);

		// draw bottom leg
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-0.2, -1.5, 0.4);
		glVertex3f(-0.2, -1.5, 0.2);
		glVertex3f( 0.2, -1.5, 0.2);
		glVertex3f( 0.2, -1.5, 0.4);
	glEnd();
}

void drawR_Leg(){
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-0.2, -1.5, -0.2);
		glVertex3f( 0.2, -1.5, -0.2);
		glVertex3f( 0.2,  0.0, -0.2);
		glVertex3f(-0.2,  0.0, -0.2);

		// draw back leg
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-0.2, -1.5, -0.4);
		glVertex3f(-0.2,  0.0, -0.4);
		glVertex3f( 0.2,  0.0, -0.4);
		glVertex3f( 0.2, -1.5, -0.4);

		// draw left leg
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-0.2, -1.5, -0.2);
		glVertex3f(-0.2,  0.0, -0.2);
		glVertex3f(-0.2,  0.0, -0.4);
		glVertex3f(-0.2, -1.5, -0.4);

		// draw right leg
		glNormal3f( 1.0, 0.0, 0.0);
		glVertex3f( 0.2, -1.5, -0.2);
		glVertex3f( 0.2, -1.5, -0.4);
		glVertex3f( 0.2,  0.0, -0.4);
		glVertex3f( 0.2,  0.0, -0.2);

		// draw up leg
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-0.2,  0.0, -0.2);
		glVertex3f( 0.2,  0.0, -0.2);
		glVertex3f( 0.2,  0.0, -0.4);
		glVertex3f(-0.2,  0.0, -0.4);

		// draw bottom leg
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-0.2, -1.5, -0.2);
		glVertex3f(-0.2, -1.5, -0.4);
		glVertex3f( 0.2, -1.5, -0.4);
		glVertex3f( 0.2, -1.5, -0.2);
	glEnd();
}

void drawL_Knee() {
	glBegin(GL_QUADS);
		glNormal3f(0.14834, 0.0, 0.988936);
		glVertex3f(-1.0, 0.0, 0.45);
		glVertex3f( 0.0, 0.0, 0.3);
		glVertex3f( 0.0, 0.1, 0.3);
		glVertex3f(-1.0, 0.1, 0.45);

		// draw back knee
		glNormal3f(0.242536, 0.0, -0.970143);
		glVertex3f(-1.0, -0.0, 0.05);
		glVertex3f(-1.0, 0.1, 0.05);
		glVertex3f( 0.0, 0.1, 0.3);
		glVertex3f( 0.0, 0.0, 0.3);

		// draw left knee
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-1.0, 0.0, 0.45);
		glVertex3f(-1.0, 0.1, 0.45);
		glVertex3f(-1.0, 0.1, 0.05);
		glVertex3f(-1.0, 0.0, 0.05);

		// draw up knee
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0,  0.1, 0.45);
		glVertex3f( 0.0,  0.1, 0.3);
		glVertex3f(-1.0,  0.1, 0.05);
		glVertex3f(-1.0,  0.1, 0.45);

		// draw bottom knee
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-1.0,  0.0, 0.45);
		glVertex3f(-1.0,  0.0, 0.05);
		glVertex3f( 0.0,  0.0, 0.3);
		glVertex3f(-1.0,  0.0, 0.45);
	glEnd();
}

void drawR_Knee() { 
	glBegin(GL_QUADS);
		glNormal3f(0.242536, 0.0, 0.970143);
		glVertex3f(-1.0, 0.0, -0.05);
		glVertex3f( 0.0, 0.0, -0.3);
		glVertex3f( 0.0, 0.1, -0.3);
		glVertex3f(-1.0, 0.1, -0.05);

		// draw back knee
		glNormal3f(0.14834, 0.0, -0.988936);
		glVertex3f(-1.0, -0.0, -0.45);
		glVertex3f(-1.0, 0.1, -0.45);
		glVertex3f( 0.0, 0.1, -0.3);
		glVertex3f( 0.0, 0.0, -0.3);

		// draw left knee
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-1.0, 0.0, -0.05);
		glVertex3f(-1.0, 0.1, -0.05);
		glVertex3f(-1.0, 0.1, -0.45);
		glVertex3f(-1.0, 0.0, -0.45);

		// draw up knee
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0,  0.1, -0.05);
		glVertex3f( 0.0,  0.1, -0.3);
		glVertex3f(-1.0,  0.1, -0.45);
		glVertex3f(-1.0,  0.1, -0.05);

		// draw bottom knee
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-1.0,  0.0, -0.05);
		glVertex3f(-1.0,  0.0, -0.45);
		glVertex3f( 0.0,  0.0, -0.3);
		glVertex3f(-1.0,  0.0, -0.05);
	glEnd();
}

void lighting_Matte()
{
	//enable the light
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0);

	// add a directional light and material
	GLfloat position[] = { 15 * cos(LIGHT), 15 * sin(LIGHT), 0.0, 0.0};
	GLfloat diffuseRGBA[] = {0.55, 0.55, 0.55, 1.0};
	GLfloat specularRGBA[] = {0.7, 0.7, 0.7, 1.0};
	GLfloat ambientLight[] = {0.0, 0.0, 0.0, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseRGBA);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularRGBA);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	GLfloat ambientMaterial[] = {0.0, 0.0, 0.0, 0.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseRGBA);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularRGBA);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.25*128);
}

void lighting_Metallic()
{
	//enable the light
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0);

	// add a directional light and material
	GLfloat position[] = { 0, 0, 0.0, 1.0};
	GLfloat diffuseRGBA[] = {0.714, 0.4284, 0.18144, 1.0};
	GLfloat specularRGBA[] = {0.393548, 0.271906, 0.166721, 1.0};
	GLfloat ambientLight[] = {0.2125, 0.2175, 0.054, 1.0};
	
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseRGBA);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularRGBA);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	GLfloat ambientMaterial[] = {0.0, 0.0, 0.0, 0.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseRGBA);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularRGBA);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.2*128);
}
///////////////////////////////////////////////////////////
//
// BELOW ARE FUNCTIONS FOR GENERATING IMAGE FILES (PPM/PGM)
//
///////////////////////////////////////////////////////////

void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P5\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			fwrite(&buffer[y*width],sizeof(GLubyte),width,fp);
			/*
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%c",int(buffer[x+y*width];
			}
			*/
		}
	}
	else
	{
		fprintf(fp,"P2\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%d ",int(buffer[x+y*width]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P6\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%c%c%c",int(pix[RED_OFFSET]),
									int(pix[GREEN_OFFSET]),
									int(pix[BLUE_OFFSET]));
			}
		}
	}
	else
	{
		fprintf(fp,"P3\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%d %d %d ",int(pix[RED_OFFSET]),
									   int(pix[GREEN_OFFSET]),
									   int(pix[BLUE_OFFSET]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

void writeFrame(char* filename, bool pgm, bool frontBuffer)
{
	static GLubyte* frameData = NULL;
	static int currentSize = -1;

	int size = (pgm ? 1 : 4);

	if( frameData == NULL || currentSize != size*Win[0]*Win[1] )
	{
		if (frameData != NULL)
			delete [] frameData;

		currentSize = size*Win[0]*Win[1];

		frameData = new GLubyte[currentSize];
	}

	glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

	if( pgm )
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
		writePGM(filename, frameData, Win[0], Win[1]);
	}
	else
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_RGBA, GL_UNSIGNED_BYTE, frameData);
		writePPM(filename, frameData, Win[0], Win[1]);
	}
}
