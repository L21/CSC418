/***********************************************************
             CSC418/2504, Fall 2009
  
                 penguin.cpp
                 
       Simple demo program using OpenGL and the glut/glui 
       libraries

  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design.

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

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size


// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation

// Joint parameters
const float JOINT_MIN = -45.0f;
const float JOINT_MAX =  45.0f;
const float JOINT_FEET_MAX =  90.0f;
const float JOINT_FEET_MIN = 0.0f;
const float JOINT_HEAD_MAX =  5.0f;
const float JOINT_HEAD_MIN =  -5.0f;
const float MOVEMENT_MAX = 10.0f;
const float MOVEMENT_MIN = 0.0f;
const float BODY_MIN = -100.0f;
const float BODY_MAX = 100.0f;

float joint_rot = 0.0f;
float joint_left_leg = 0.0f;
float joint_right_leg = 0.0f;
float joint_left_foot = 0.0f;
float joint_right_foot = 0.0f;
float joint_head = 0.0f;
float joint_move_mouth = 0.0f;
float joint_body_x = 0.0f;
float joint_body_y = 0.0f;

//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////



// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);



// Functions to help draw the object
void drawSquare(float size);
void drawpolygon(float size);
void DrawCircle(float r, int num);
void DrawTrapezoid(float r);
void DrawLeg(float width);
void DrawFeet(float width);
void DrawHead(float width);
void DrawHollowCircle(float r, int num);
void DrawMouth(float width);
void DrawMouth2(float width);
// Return the current system clock (in seconds)
double getTime();


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
        printf("Using 300x200 window by default...\n");
        Win[0] = 1000;
        Win[1] = 800;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);

    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed 
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui->sync_live();

  animation_frame = 0;
  if(animate_mode == 1) {
    // start animation
    GLUI_Master.set_glutIdleFunc(animate);
  } else {
    // stop animation
    GLUI_Master.set_glutIdleFunc(NULL);
  }
}

// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);

    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);

    // Create a control to specify the rotation of the joint
    GLUI_Spinner *joint_spinner
        = glui->add_spinner("Joint", GLUI_SPINNER_FLOAT, &joint_rot);
    joint_spinner->set_speed(0.1);
    joint_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);


    //spinner for the left leg
    GLUI_Spinner *joint_left_leg_spinner
        = glui->add_spinner("joint_left_leg", GLUI_SPINNER_FLOAT, &joint_left_leg);
    joint_left_leg_spinner->set_speed(0.1);
    joint_left_leg_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);


    //spinner for the right leg
    GLUI_Spinner *joint_right_leg_spinner
        = glui->add_spinner("joint_right_leg", GLUI_SPINNER_FLOAT, &joint_right_leg);
    joint_right_leg_spinner->set_speed(0.1);
    joint_right_leg_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    //spinner for the right foot
    GLUI_Spinner *joint_right_foot_spinner
        = glui->add_spinner("joint_right_foot", GLUI_SPINNER_FLOAT, &joint_right_foot);
    joint_right_foot_spinner->set_speed(0.1);
    joint_right_foot_spinner->set_float_limits(JOINT_FEET_MIN, JOINT_FEET_MAX, GLUI_LIMIT_CLAMP);

    //spinner for the left foot
    GLUI_Spinner *joint_left_foot_spinner
        = glui->add_spinner("joint_left_foot", GLUI_SPINNER_FLOAT, &joint_left_foot);
    joint_left_foot_spinner->set_speed(0.1);
    joint_left_foot_spinner->set_float_limits(JOINT_FEET_MIN, JOINT_FEET_MAX, GLUI_LIMIT_CLAMP);


    //spinner for the head
    GLUI_Spinner *joint_head_spinner
        = glui->add_spinner("joint_head", GLUI_SPINNER_FLOAT, &joint_head);
    joint_head_spinner->set_speed(0.1);
    joint_head_spinner->set_float_limits(JOINT_HEAD_MIN, JOINT_HEAD_MAX, GLUI_LIMIT_CLAMP);

    //spinner for the move
    GLUI_Spinner *joint_move_mouth_spinner
        = glui->add_spinner("joint_move_mouth", GLUI_SPINNER_FLOAT, &joint_move_mouth);
    joint_move_mouth_spinner->set_speed(0.1);
    joint_move_mouth_spinner->set_float_limits(MOVEMENT_MIN, MOVEMENT_MAX, GLUI_LIMIT_CLAMP);

    GLUI_Spinner *joint_body_x_spinner
        = glui->add_spinner("joint_body_x", GLUI_SPINNER_FLOAT, &joint_body_x);
    joint_body_x_spinner->set_speed(0.1);
    joint_body_x_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    GLUI_Spinner *joint_body_y_spinner
        = glui->add_spinner("joint_body_y", GLUI_SPINNER_FLOAT, &joint_body_y);
    joint_body_y_spinner->set_speed(0.1);
    joint_body_y_spinner->set_float_limits(BODY_MIN, BODY_MAX, GLUI_LIMIT_CLAMP);


    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////

    // Add button to specify animation mode 
    glui->add_separator();
    glui->add_checkbox("Animate", &animate_mode, 0, animateButton);

    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton);

    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}




// Callback idle function for animating the scene
void animate()
{
    // Update geometry
    const double joint_rot_speed = 1.0;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;

    const double joint_left_leg_speed = 0.5;
    double joint_left_leg_t = (sin(animation_frame*joint_left_leg_speed) + 1.0) / 2.0;
    joint_left_leg = joint_left_leg_t * JOINT_MIN + (1 - joint_left_leg_t) * JOINT_MAX;

    const double joint_right_leg_speed = -0.5;
    double joint_right_leg_t = (sin(animation_frame*joint_right_leg_speed) + 1.0) / 2.0;
    joint_right_leg = joint_right_leg_t * JOINT_MIN + (1 - joint_right_leg_t) * JOINT_MAX;

    const double joint_left_foot_speed = 0.5;
    double joint_left_foot_t = (sin(animation_frame*joint_left_foot_speed ) + 1.0) / 2.0;
    joint_left_foot= joint_left_foot_t * JOINT_FEET_MIN + (1 - joint_left_foot_t) * JOINT_FEET_MAX;

    const double joint_right_foot_speed = -0.5;
    double joint_right_foot_t = (sin(animation_frame*joint_right_foot_speed ) + 1.0) / 2.0;
    joint_right_foot = joint_right_foot_t * JOINT_FEET_MIN + (1 - joint_right_foot_t) * JOINT_FEET_MAX;

    const double joint_head_speed = 1.0;
    double joint_head_t = (sin(animation_frame*joint_head_speed ) + 1.0) / 2.0;
    joint_head = joint_head_t * JOINT_HEAD_MIN + (1 - joint_head_t) * JOINT_HEAD_MAX;

    const double joint_move_mouth_speed = 1.0;
    double joint_move_mouth_t = (sin(animation_frame*joint_move_mouth_speed) + 1.0) / 2.0;
    joint_move_mouth = joint_move_mouth_t * MOVEMENT_MIN + (1 - joint_move_mouth_t) * MOVEMENT_MAX;

    const double joint_body_x_speed = 0.1;
    double joint_body_x_t = (sin(animation_frame*joint_body_x_speed + 3.0) + 1.0) / 2.0;
    joint_body_x = joint_body_x_t * BODY_MIN + (1 - joint_body_x_t) * BODY_MAX;

    const double joint_body_y_speed = 0.2;
    double joint_body_y_t = (sin(animation_frame*joint_body_y_speed + 3.0) + 1.0) / 2.0;
    joint_body_y = joint_body_y_t * BODY_MIN + (1 - joint_body_y_t) * BODY_MAX;

    
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////

    // Update user interface
    glui->sync_live();

    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();

    // increment the frame number.
    animation_frame++;

    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}


// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}



// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For Assignment 1,
// updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);

    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////

    // Draw our hinged object
    const float BODY_WIDTH = 30.0f;
    const float BODY_LENGTH = 50.0f;
    const float ARM_LENGTH = 50.0f;
    const float ARM_WIDTH = 10.0f;

    // Push the current transformation matrix on the stack
    glPushMatrix();
        glPushMatrix();
            glTranslatef(joint_body_y, joint_body_x, 0.0);
        
        // Draw the 'body'
            glPushMatrix();
                // Scale square to size of body
                glScalef(BODY_WIDTH, BODY_LENGTH, 1.0);

                // Set the colour to green
                glColor3f(0.0, 1.0, 0.0);

                // Draw the square for the body
                drawpolygon(2.0);

                //reset the scale
                glScalef(1/BODY_WIDTH, 1/BODY_LENGTH, 1.0);


                
                //Draw the head
                glPushMatrix();

                    //Move the head to the joint hinge
                    glTranslatef(0, 160, 0.0);
                    // Rotate along the hinge
                    glRotatef(joint_head, 0.0, 0.0, 1.0);
                    //set the colour to white
                    glColor3f(1.0, 1.0, 1.0);

                    glScalef(BODY_WIDTH, BODY_LENGTH, 1.0);
                    DrawHead(2.0);
                    //reset the scale
                    glScalef(1/BODY_WIDTH, 1/BODY_LENGTH, 1.0);

                    

                    //joint
                    glPushMatrix();
                        // Set the colour to blue
                        glColor3f(0.0, 0.0, 1.0);
                        

                        glScalef( 4.0, 4.0, 1.0);

                        // Draw the circle for the body
                        DrawCircle(1.0, 360);
                    glPopMatrix();

                    //draw the eye(part1)
                    glPushMatrix();

                        //set the colour to black
                        glColor3f(0.0, 0.0, 0.0);

                        //Move the eye to the joint hinge
                        glTranslatef(-20, 30, 0.0);
                        
                        glScalef( 3.0, 3.0, 1.0);

                        // Draw the eye
                        DrawCircle(1.0, 360);
                    glPopMatrix();

                    //part2
                    glPushMatrix();

                        glColor3f(0.0, 0.0, 0.0);
                        glTranslatef(-18, 30, 0.0);
                        glScalef( 5.0, 5.0, 5.0);

                        DrawHollowCircle(1.0, 360);
                    glPopMatrix();


                    //draw the mouth(part1)
                    glPushMatrix();

                        //set the colour to yellow
                        glColor3f(1.0, 1.0, 0.0);
                        glScalef(10.0, 10.0, 1.0);

                        DrawMouth(1.0);
                    glPopMatrix();

                    //mouth(part2)
                    glPushMatrix();
                        // Rotate along the hinge
                        glTranslatef(0, joint_move_mouth, 0.0);
                        glColor3f(1.0, 1.0, 0.0);
                        glScalef(10.0, 10.0, 1.0);

                        DrawMouth2(1.0);
                    glPopMatrix();
     
                glPopMatrix();

            glPopMatrix();





            //arm
            glPushMatrix();
                
                glTranslatef(15, 100, 0.0);
                glRotatef(joint_rot, 0.0, 0.0, 1.0);
                // Scale square 
                glScalef( 4.0, 4.0, 1.0);
                // Set the colour to red
                glColor3f(1.0, 0.0, 0.0);
                // Draw the arm for the body
                DrawTrapezoid(8.0);

                //joint
                glPushMatrix();
                    // Set the colour to blue
                    glColor3f(0.0, 0.0, 1.0);

                    // Draw the circle for the body
                    DrawCircle(1.0, 360);

                glPopMatrix();
                
            glPopMatrix();


            //left_leg with foot

            glPushMatrix();

                //Move the left leg to the joint hinge
                glTranslatef(-40, -14, 0.0);

                // Rotate along the hinge
                glRotatef(joint_left_leg, 0.0, 0.0, 1.0);

                glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);
                // Set the colour to red
                glColor3f(1.0, 0.0, 0.0);

                // Draw the leg for the body
                DrawLeg(1.0);

                //reset the scale
                glScalef(1/ARM_WIDTH, 1/ARM_LENGTH, 1.0);


                //joint
                glPushMatrix();
                    glScalef(4.0, 4.0, 1.0);

                    // Set the colour to blue
                    glColor3f(0.0, 0.0, 1.0);
                    DrawCircle(1.0, 360);

                glPopMatrix();

                //left feet
                glPushMatrix();
                    
                    //Move the left feet to the joint hinge
                    glTranslatef(0, -40, 0.0);
                    glRotatef(joint_left_foot, 0.0, 0.0, 1.0);
                    glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);
                    // Set the colour to yellow
                    glColor3f(1.0, 1.0, 0.0);

                    // Draw the feet for the leg
                    DrawFeet(1.0);

                    //reset the scale
                    glScalef(1/ARM_WIDTH, 1/ARM_LENGTH, 1.0);

                    //joint for the feet
                    glPushMatrix();
                        glScalef(4.0, 4.0, 1.0);

                        // Set the colour to blue
                        glColor3f(0.0, 0.0, 1.0);
                        DrawCircle(1.0, 360);

                    glPopMatrix();

                glPopMatrix();

            glPopMatrix();



            //right_leg with foot
            glPushMatrix();

                //Move the right leg to the joint hinge
                glTranslatef(40, -14, 0.0);

                // Rotate along the hinge
                glRotatef(joint_right_leg, 0.0, 0.0, 1.0);
                glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);
                // Set the colour to red
                glColor3f(1.0, 0.0, 0.0);

                // Draw the leg for the body
                DrawLeg(1.0);

                //reset the scale
                glScalef(1/ARM_WIDTH, 1/ARM_LENGTH, 1.0);


                //joint
                glPushMatrix();
                     glScalef(4.0, 4.0, 1.0);

                     // Set the colour to blue
                     glColor3f(0.0, 0.0, 1.0);
                     DrawCircle(1.0, 360);

                glPopMatrix();


                //right feet
                glPushMatrix();

                    //Move the right feet to the joint hinge
                    glTranslatef(0, -40, 0.0);
                    glRotatef(joint_right_foot, 0.0, 0.0, 1.0);
                    glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);
                    // Set the colour to yellow
                    glColor3f(1.0, 1.0, 0.0);

                    // Draw the feet for the leg
                    DrawFeet(1.0);
                    glScalef(1/ARM_WIDTH, 1/ARM_LENGTH, 1.0);

                    //joint for the feet
                    glPushMatrix();
                        glScalef(4.0, 4.0, 1.0);
                        glColor3f(0.0, 0.0, 1.0);
                        DrawCircle(1.0, 360);
                    glPopMatrix();

                glPopMatrix();

            glPopMatrix();

        glPopMatrix();

    glPopMatrix();

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
    // Draw the square
    glBegin(GL_POLYGON);
    glVertex2d(-width, -2*width/3);
    glVertex2d(width, -2*width/3);
    glVertex2d(width, 2*width/3);
    glVertex2d(-width, 2*width/3);
    glEnd();
}

void drawpolygon(float width){
    glBegin(GL_POLYGON);
    glVertex2d(-width, 0);
    glVertex2d(-width/4, -width/2);
    glVertex2d(width/4, -width/2);
    glVertex2d(width, 0);
    glVertex2d(width/2, width * 5/3);
    glVertex2d(-width/2, width * 5/3);
    glEnd();
}


void DrawHead(float width) {

    glBegin(GL_POLYGON);
    glVertex2d(-0.7*width, -0.1*width);
    glVertex2d(2*width/3, -0.1*width);
    glVertex2d(width/2, 0.5*width);
    glVertex2d(-0.4*width, width * 0.7) ;
    glVertex2d(-0.65*width, width * 0.5);
    glEnd();

}

void DrawMouth(float width){
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(-8*width, 2*width);
    glVertex2d(-3*width, 2*width);
    glVertex2d(-4*width, 3.5*width);
    glEnd();
}

void DrawMouth2(float width){
    glBegin(GL_POLYGON);
    glVertex2d(-8*width, 1*width);
    glVertex2d(-3*width, 1*width);
    glVertex2d(-3*width, 1.3*width);
    glVertex2d(-8*width, 1.3*width);
    glEnd();
}


void DrawCircle(float r, int num)
{
    glBegin(GL_POLYGON);
    for(int i = 0; i < num; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(num);//get the current angle

        float x = r * cosf(theta);//calculate the x component
        float y = r * sinf(theta);//calculate the y component

        glVertex2f(x, y);//output vertex

    }
    glEnd();
}


void DrawHollowCircle(float r, int num)
{
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < num; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(num);//get the current angle

        float x = r * cosf(theta);//calculate the x component
        float y = r * sinf(theta);//calculate the y component

        glVertex2f(x, y);//output vertex

    }
    glEnd();
}


 void DrawTrapezoid(float r) {
    glBegin(GL_POLYGON);
    glVertex2d(-2*r/3, r/4);
    glVertex2d(-r/4, -2*r);
    glVertex2d(r/4, -2*r);
    glVertex2d(r/2, r/4);

    glEnd();
 }


 void DrawLeg(float width)
{
    // Draw the leg
    glBegin(GL_POLYGON);
    glVertex2d(-width, -width);
    glVertex2d(width, -width);
    glVertex2d(width, width/4);
    glVertex2d(-width, width/4);
    glEnd();
}

void DrawFeet(float width)
{
    // Draw the feet
    glBegin(GL_POLYGON);
    glVertex2d(-4*width, -width/4);
    glVertex2d(width, -width/4);
    glVertex2d(width, width/4);
    glVertex2d(-6*width, width/4);
    glEnd();
}






