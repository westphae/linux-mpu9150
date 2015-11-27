////////////////////////////////////////////////////////////////////////////
//
//  This file is part of
//
//  Copyright (c) 2015 Joseph D Poirier
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// To reshape a window in GLUT you need one command in the 'main' function:
//      glutInitWindowSize (500, 500);
// this will set it to 500 pixels wide and 500 high.
// The other line you may notice I have added is:
//      glutInitWindowPosition (100, 100);
// This sets where the window is located on the screen, in this case
// 100 pixels down and 100 to the right.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <GL/gl.h>

#if defined(__APPLE__) && defined(__MACH__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

// Hmmmm, what to use here???
#if defined(__APPLE__) && defined(__MACH__) || defined(__linux__) || defined(__unix__)
    #include <unistd.h>     // UNIX standard function definitions
#endif

#include "ogcCircleEvaluator.h"
#include "libimu.h"

#define SHOW_FRAME_RATE     1

enum {
    kNumRetries         = 3,
    MAIN_WINDOW_WIDTH   = 300,
    MAIN_WINDOW_HEIGHT  = 300,
    MAIN_WINDOW_XPOS    = 100,
    MAIN_WINDOW_YPOS    = 100
};

using namespace OpenGC;

// For drawing circles
static CircleEvaluator m_aCircle;

static int             g_frame             = 0;
static int             g_timebase          = 0;

static int             g_fd                = -1;
static double          g_pitch             = 0.0;
static double          g_roll              = 0.0;
static double          g_heading           = 0.0;

static double          g_pitch_offset      = 0.0;
static double          g_roll_offset       = 0.0;
static double          g_heading_offset    = 0.0;

// The location in pixels is calculated based on the size of the
// gauge component and the offset of the parent guage
static int             g_PixelPosition_x   = 25;
static int             g_PixelPosition_y   = 25;

// The size in pixels of the gauge is the physical size / mm per pixel
static int             g_PixelSize_x       = 250;
static int             g_PixelSize_y       = 250;

static float           g_Scale_x           = 1.0;
static float           g_Scale_y           = 1.0;

static int             g_PhysicalSize_x    = 94;
static int             g_PhysicalSize_y    = 98;

static char            g_headingStr[32]    = {};


/** \brief
 *
 *
 *
 *  \param void
 *  \retval void
 */
void artificial_horizon(void) {
    char*   pStr        = 0;
    bool    blending    = false;

    // The viewport is established in order to clip things
    // outside the bounds of the GaugeComponent
    glViewport(g_PixelPosition_x, g_PixelPosition_y, g_PixelSize_x, g_PixelSize_y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Define the projection so that we're drawing in "real" space
    glOrtho(0, (g_Scale_x * g_PhysicalSize_x), 0, (g_Scale_y * g_PhysicalSize_y), -1, 1);

    // Prepare the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(g_Scale_x, g_Scale_y, 1.0f);

    // First, store the "root" position of the gauge component
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Move to the center of the window
    glTranslated(47, 49, 0);


    // Rotate based on the bank
    // Note, the roll direction is reversed so we adjust by adding a negative sign
    glRotated((-g_roll) - (-g_roll_offset), 0, 0, 1);

    // Translate in the direction of the rotation based on the pitch.
    // On the 777, a pitch of 1 degree = 2 mm
    glTranslated(0, ((g_pitch - g_pitch_offset) * -2.0), 0);

    //--------------------------------------------------------------------------
    //-------------------Gauge Background------------------
    // It's drawn oversize to allow for pitch and bank

    // The "ground" rectangle
    // Remember, the coordinate system is now centered in the gauge component
    glColor3ub(179, 102, 0);

    glBegin(GL_POLYGON);
        glVertex2f(-300, -300);
        glVertex2f(-300, 0);
        glVertex2f(300, 0);
        glVertex2f(300, -300);
        glVertex2f(-300, -300);
    glEnd();

    // The "sky" rectangle
    // Remember, the coordinate system is now centered in the gauge component
    glColor3ub(0, 153, 204);

    glBegin(GL_POLYGON);
        glVertex2f(-300, 0);
        glVertex2f(-300, 300);
        glVertex2f(300, 300);
        glVertex2f(300, 0);
        glVertex2f(-300, 0);
    glEnd();

    //--------------------------------------------------------------------------
    //------------Draw the pitch markings--------------
    // Draw in white
    glColor3ub(255, 255, 255);

    // Specify line width
    glLineWidth(1.0);

// XXX:
#if 0
    // The size for all pitch text
    m_pFontManager->SetSize(m_Font, 4.0, 4.0);
#endif

    glBegin(GL_LINES);
        // The dividing line between sky and ground
        glVertex2f(-100, 0);
        glVertex2f(100, 0);

        // +2.5 degrees
        glVertex2f(-5, 5);
        glVertex2f(5, 5);

        // +5.0 degrees
        glVertex2f(-10, 10);
        glVertex2f(10, 10);

        // +7.5 degrees
        glVertex2f(-5, 15);
        glVertex2f(5, 15);

        // +10.0 degrees
        glVertex2f(-20, 20);
        glVertex2f(20, 20);

        // +12.5 degrees
        glVertex2f(-5, 25);
        glVertex2f(5, 25);

        // +15.0 degrees
        glVertex2f(-10, 30);
        glVertex2f(10, 30);

        // +17.5 degrees
        glVertex2f(-5, 35);
        glVertex2f(5, 35);

        // +20.0 degrees
        glVertex2f(-20, 40);
        glVertex2f(20, 40);

        // -2.5 degrees
        glVertex2f(-5, -5);
        glVertex2f(5, -5);

        // -5.0 degrees
        glVertex2f(-10, -10);
        glVertex2f(10, -10);

        // -7.5 degrees
        glVertex2f(-5, -15);
        glVertex2f(5, -15);

        // -10.0 degrees
        glVertex2f(-20, -20);
        glVertex2f(20, -20);

        // -12.5 degrees
        glVertex2f(-5, -25);
        glVertex2f(5, -25);

        // -15.0 degrees
        glVertex2f(-10, -30);
        glVertex2f(10, -30);

        // -17.5 degrees
        glVertex2f(-5, -35);
        glVertex2f(5, -35);

        // -20.0 degrees
        glVertex2f(-20, -40);
        glVertex2f(20, -40);
    glEnd();

// XXX:
#if 0
    // +10
    m_pFontManager->Print(-27.5, 18.0, "10", m_Font);
    m_pFontManager->Print(21.0, 18.0, "10", m_Font);

    // -10
    m_pFontManager->Print(-27.5, -22.0, "10", m_Font);
    m_pFontManager->Print(21.0, -22.0, "10", m_Font);

    // +20
    m_pFontManager->Print(-27.5, 38.0, "20", m_Font);
    m_pFontManager->Print(21.0, 38.0, "20", m_Font);

    // -20
    m_pFontManager->Print(-27.5, -42.0, "20", m_Font);
    m_pFontManager->Print(21.0, -42.0, "20", m_Font);
#endif

    //--------------------------------------------------------------------------
    //----- The background behind the bank angle markings -------
    // Reset the modelview matrix
    glPopMatrix();
    glPushMatrix();

    // Draw in the sky color
    glColor3ub(0, 153, 204);

    m_aCircle.SetOrigin(47, 49);
    m_aCircle.SetRadius(46);
    m_aCircle.SetDegreesPerPoint(5);
    m_aCircle.SetArcStartEnd(300.0, 360.0);

    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 98);
        glVertex2f(0, 72);
        m_aCircle.Evaluate();
        glVertex2f(47, 98);
    glEnd();

    m_aCircle.SetArcStartEnd(0.0, 60.0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(94, 98);
        glVertex2f(47, 98);
        m_aCircle.Evaluate();
        glVertex2f(94, 72);
    glEnd();

    //--------------------------------------------------------------------------
    //----------------The bank angle markings----------------
    // Left side bank markings
    // Reset the modelview matrix
    glPopMatrix();
    glPushMatrix();

    // Draw in white
    glColor3ub(255, 255, 255);

    // Move to the center of the window
    glTranslated(47, 49, 0);

    // Draw the center detent
    glBegin(GL_POLYGON);
        glVertex2f(0.0, 46.0);
        glVertex2f(-2.3, 49.0);
        glVertex2f(2.3, 49.0);
        glVertex2f(0.0, 46.0);
    glEnd();

    glRotated(10.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 49);
    glEnd();

    glRotated(10.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 49);
    glEnd();

    glRotated(10.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0 ,53);
    glEnd();

    glRotated(15.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 49);
    glEnd();

    glRotated(15.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 51);
    glEnd();

    // Right side bank markings
    // Reset the modelview matrix
    glPopMatrix();
    glPushMatrix();

    // Move to the center of the window
    glTranslated(47, 49, 0);

    glRotated(-10.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 49);
    glEnd();

    glRotated(-10.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 49);
    glEnd();

    glRotated(-10.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 53);
    glEnd();

    glRotated(-15.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 49);
    glEnd();

    glRotated(-15.0, 0, 0, 1);

    glBegin(GL_LINES);
        glVertex2f(0, 46);
        glVertex2f(0, 51);
    glEnd();

    //--------------------------------------------------------------------------
    //----------------Bank Indicator----------------
    // Reset the modelview matrix
    glPopMatrix();
    glPushMatrix();

    // Move to the center of the window
    glTranslated(47, 49, 0);

    // Rotate based on the bank
    // Note, the roll direction is reversed so we adjust by addind a negative sign
    glRotated((-g_roll) - (-g_roll_offset), 0, 0, 1);

    // Draw in white
    glColor3ub(255, 255, 255);
    // Specify line width
    glLineWidth(2.0);

    glBegin(GL_LINE_LOOP); // the bottom rectangle
        glVertex2f(-4.5, 39.5);
        glVertex2f(4.5, 39.5);
        glVertex2f(4.5, 41.5);
        glVertex2f(-4.5, 41.5);
    glEnd();

    glBegin(GL_LINE_STRIP); // the top triangle
        glVertex2f(-4.5, 41.5);
        glVertex2f(0, 46);
        glVertex2f(4.5, 41.5);
    glEnd();

    //--------------------------------------------------------------------------
    //---------------- Attitude Indicator
    // Reset the modelview matrix
    glPopMatrix();
    glPushMatrix();

    // Move to the center of the window
    glTranslated(47, 49, 0);

    // The center axis indicator
    // Black background
    glColor3ub(0, 0, 0);

    glBegin(GL_POLYGON);
        glVertex2f(1.25, 1.25);
        glVertex2f(1.25, -1.25);
        glVertex2f(-1.25, -1.25);
        glVertex2f(-1.25, 1.25);
        glVertex2f(1.25, 1.25);
    glEnd();

    // White lines
    glColor3ub(255, 255, 255);
    glLineWidth(2.0);

    glBegin(GL_LINE_LOOP);
        glVertex2f(1.25, 1.25);
        glVertex2f(1.25, -1.25);
        glVertex2f(-1.25, -1.25);
        glVertex2f(-1.25, 1.25);
    glEnd();

    // The left part
    // Black background
    glColor3ub(0, 0, 0);

    glBegin(GL_POLYGON);
        glVertex2f(-39, 1.25);
        glVertex2f(-19, 1.25);
        glVertex2f(-19, -1.25);
        glVertex2f(-39, -1.25);
        glVertex2f(-39, 1.25);
    glEnd();

    glBegin(GL_POLYGON);
        glVertex2f(-19, 1.25);
        glVertex2f(-19, -5.75);
        glVertex2f(-22, -5.75);
        glVertex2f(-22, 1.25);
        glVertex2f(-19, 1.25);
    glEnd();

    // White lines
    glColor3ub(255, 255, 255);
    glLineWidth(2.0);

    glBegin(GL_LINE_LOOP);
        glVertex2f(-39, 1.25);
        glVertex2f(-19, 1.25);
        glVertex2f(-19, -5.75);
        glVertex2f(-22, -5.75);
        glVertex2f(-22, -1.25);
        glVertex2f(-39, -1.25);
    glEnd();

    // The right part
    // Black background
    glColor3ub(0, 0, 0);

    glBegin(GL_POLYGON);
        glVertex2f(39, 1.25);
        glVertex2f(19, 1.25);
        glVertex2f(19, -1.25);
        glVertex2f(39, -1.25);
        glVertex2f(39, 1.25);
    glEnd();

    glBegin(GL_POLYGON);
        glVertex2f(19, 1.25);
        glVertex2f(19, -5.75);
        glVertex2f(22, -5.75);
        glVertex2f(22, 1.25);
        glVertex2f(19, 1.25);
    glEnd();

    // White lines
    glColor3ub(255, 255, 255);
    glLineWidth(2.0);

    glBegin(GL_LINE_LOOP);
        glVertex2f(39, 1.25);
        glVertex2f(19, 1.25);
        glVertex2f(19, -5.75);
        glVertex2f(22, -5.75);
        glVertex2f(22, -1.25);
        glVertex2f(39, -1.25);
    glEnd();

    //--------------------------------------------------------------------------
    // Reset the modelview matrix
    glPopMatrix();
    glPushMatrix();

    m_aCircle.SetRadius(3.77);

    //--------------------------------------------------------------------------
    //-------------------Rounded corners------------------
    // The corners of the artificial horizon are rounded off by
    // drawing over them in black. The overlays are essentially the
    // remainder of a circle subtracted from a square, and are formed
    // by fanning out triangles from a point just off each corner
    // to an arc descrbing the curved portion of the art. horiz.

    glColor3ub(0, 0, 0);

    // Lower left
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(-1.0, -1.0);
        m_aCircle.SetOrigin(3.77, 3.77);
        m_aCircle.SetArcStartEnd(180, 270);
        m_aCircle.SetDegreesPerPoint(15);
        m_aCircle.Evaluate();
    glEnd();

    // Upper left
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(-1.0, 99.0);
        m_aCircle.SetOrigin(3.77, 94.23);
        m_aCircle.SetArcStartEnd(270, 360);
        m_aCircle.SetDegreesPerPoint(15);
        m_aCircle.Evaluate();
    glEnd();

    // Upper right
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(95.0, 99.0);
        m_aCircle.SetOrigin(90.23, 94.23);
        m_aCircle.SetArcStartEnd(0, 90);
        m_aCircle.SetDegreesPerPoint(15);
        m_aCircle.Evaluate();
    glEnd();

    //Lower right
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(95.0, -1);
        m_aCircle.SetOrigin(90.23, 3.77);
        m_aCircle.SetArcStartEnd(90, 180);
        m_aCircle.SetDegreesPerPoint(15);
        m_aCircle.Evaluate();
    glEnd();

    //--------------------------------------------------------------------------
    //----- heading
//GLUT_BITMAP_9_BY_15,
//GLUT_BITMAP_8_BY_13,
//GLUT_BITMAP_TIMES_ROMAN_10,
//GLUT_BITMAP_TIMES_ROMAN_24,
//GLUT_BITMAP_HELVETICA_10,
//GLUT_BITMAP_HELVETICA_12,
//GLUT_BITMAP_HELVETICA_18
    sprintf(g_headingStr, "%0.1f\0", g_heading);

    if(glIsEnabled(GL_BLEND))
        blending = true;

    glEnable(GL_BLEND);
    glColor4f(1.0, 1.0, 1.0, 0.1); // r, g, b, a
    glRasterPos2f(37.0, 00.0);

    pStr = g_headingStr;

    while(*pStr)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *pStr);
        pStr++;
    }

    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 176);

    if(!blending)
        glDisable(GL_BLEND);
    // ----

    // Finally, restore the modelview matrix to what we received
    glPopMatrix();

#if 0
    //--------------------------------------------------------------------------
    //----------------Flight Director----------------

    if(m_pDataSource->Director_Active == 1)
    {
      // Move to the center of the window
      glTranslated(47, 49, 0);
      glColor3ub(255, 0, 255);
      glLineWidth(3.0);

      glPushMatrix();
      glTranslated(0,(m_pDataSource->Director_Pitch) * 2.0, 0);

      glBegin(GL_LINES);
          glVertex2f(-20, 0);
          glVertex2f(20, 0);
      glEnd();

      glPopMatrix();

      glPushMatrix();
      glTranslated(m_pDataSource->Director_Bank, 0, 0);

      glBegin(GL_LINES);
          glVertex2f(0, -20);
          glVertex2f(0, 20);
      glEnd();

      glPopMatrix();
    }

    // Draw the glideslope needles only if the flight director
    // isn't activated and the glideslope is alive
    if (m_pDataSource->ILS_Glideslope_Alive && m_pDataSource->Director_Active == false) {
      // Move to the center of the window
      glTranslated(47, 49, 0);
      glColor3ub(255, 0, 255);
      glLineWidth(3.0);

      glPushMatrix();
      glTranslated(0, (m_pDataSource->Nav1_Glideslope_Needle) * -20, 0);

      glBegin(GL_LINES);
          glVertex2f(-20, 0);
          glVertex2f(20, 0);
      glEnd();

      glPopMatrix();

      glPushMatrix();
      glTranslated( (m_pDataSource->Nav1_Course_Indicator)*20, 0, 0);

      glBegin(GL_LINES);
          glVertex2f(0, -20);
          glVertex2f(0, 20);
      glEnd();

      glPopMatrix();

      // Undo the translation to the window center
      glPopMatrix();
    }
#endif
}

/** \brief
 *
 *
 *
 *  \param w
 *  \param h
 *  \retval void
 */
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluPerspective(60, (GLfloat) w / (GLfloat) h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

/** \brief
 *
 *
 *
 *  \param void
 *  \retval void
 */
void display(void) {
    int time;

    glClearColor(0.0, 0.0, 0.0, 1.0); //clear the color of the window
    glClear(GL_COLOR_BUFFER_BIT); //Clear the Color Buffer (more buffers later on)
    glLoadIdentity();  //load the Identity Matrix
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //set the view
    artificial_horizon();
    glutSwapBuffers();
    glutPostRedisplay();
//    glFlush(); //flush it all to the screen

#if SHOW_FRAME_RATE
    g_frame++;
    time = glutGet(GLUT_ELAPSED_TIME);

    if (time - g_timebase > 1000) {
        printf("FPS: %4.2f\n", g_frame * 1000.0 / (time - g_timebase));
        fflush(stdout);
        g_timebase = time;
        g_frame = 0;
    }
#endif
}

/** \brief Mouse-event handler.
 *
 *  Handles left and right mouse-events.
 *
 *  \param button
 *  \param state
 *  \param x
 *  \param y
 *  \retval void
 */
void mouse(int button, int state, int x, int y) {
    static float po, ro, h;
    switch (button) {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN){
            if (read_mpu(&po, &ro, &h) != -1) {
                g_pitch_offset = (double)po;
                g_roll_offset = (double)ro;
                g_heading = (double)h;
            }
        }
        break;
    case GLUT_MIDDLE_BUTTON:
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN) {
            g_pitch_offset      = 0;
            g_roll_offset       = 0;
            g_heading_offset    = 0;
        }
        break;
    default:
        break;
   }
}

/** \brief
 *
 *
 *
 *  \param void
 *  \retval void
 */
void idle_handler(void) {
    static float p, r, h;
    if (read_mpu(&p, &r, &h) != -1) {
        g_pitch = (double)p;
        g_roll = (double)r;
        g_heading = (double)h;
    }
    display();
}

/** \brief
 *
 *
 *
 *  \param signal
 *  \retval void
 */
void signal_handler(int32_t signal) {
    time_t      tm  = time(0);
    struct tm*  ptr = localtime(&tm);

    switch (signal) {
    case SIGFPE:
        perror("---------------- \n");
        perror(asctime(ptr));
        perror("A floating point exception occured.\n");
        break;
    case SIGILL:
        perror("---------------- \n");
        perror(asctime(ptr));
        perror("An illegal instruction occured.\n");
        break;
    case SIGINT:
        // the user hit CTRL-C
        break;
    case SIGSEGV:
        perror("---------------- \n");
        perror(asctime(ptr));
        perror("A segmentation violation occured.\n");
        break;
    default:
        perror("---------------- \n");
        perror(asctime(ptr));
        perror("An unknown signal was caught.\n");
        break;
    }

    close_mpu();
    // pass a successful exit so our atexit handler is called
    exit(EXIT_SUCCESS);
}

/** \brief
 *
 *
 *
 *  \param void
 *  \retval void
 */
void do_cleanup(void) {
    printf("    Exiting...\n" );
    if (g_fd != -1) {
        printf("closing the comm port...\n");
        close(g_fd);
    }
}

/** \brief
 *
 *
 *
 *  \param argc
 *  \param argv
 *  \retval exit success
 *  \retval exit failure
 */
int main(int argc, char* argv[]) {
    atexit(do_cleanup);

    // siganl handlers
    if (signal(SIGFPE, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGFPE signal handler.\n");

    if (signal(SIGILL, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGILL signal handler.\n");

    if (signal(SIGINT, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGINT signal handler.\n");

    if (signal(SIGSEGV, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGSEGV signal handler.\n");

#if 0
    if (init_mpu() == -1)
        printf("exiting, mpu init failed\n");

    // set_cal(mag, cal_file);

    if (g_fd == -1) {
        printf("error opening comm port...\n");
        return 0;
    } else {
       printf("success opening comm port...\n");
   }
#endif

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // GLUT_RGB GLUT_SINGLE
    glutInitWindowSize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    glutInitWindowPosition(MAIN_WINDOW_XPOS, MAIN_WINDOW_XPOS);
    glutCreateWindow("Glass Cockpit");

    glutDisplayFunc(display);
    glutIdleFunc(idle_handler);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMainLoop();             // initialize the OpenGL loop cycle

    return 0;
}

