#include <GL/glut.h>
#include <stdlib.h>

// For the line
#include <vector>
#include <iostream>

// For circle
#include <cmath>

// For L-Systems
#include <string>
#include <stack>

//misc
//#include <chrono>

using namespace std;

// For L-Systems
// grammar production
string currentString = "0";
string rhs0 = "1[0]1(0)0";
string rhs1 = "11";
string rhsOpenB = "[";
string rhsClosedB = "]";
string rhsOpenP = "(";
string rhsClosedP = ")";
string rhsPlus = "+";
string rhsMinus = "-";


// Stacks for pos and angles
stack<int> positions;
stack<int> angles;
vector<int> lines;


int curColorTree; // 0 is brown 1 is green

// Current pos and angles
int posX, posY, numIter;
float curAngle, turnAngle, len, lenMultiplier, leafLimit;

const float pi = 3.14159;
const float deg2rad = pi/180;

// For midpoint algorithms
std::vector<int> points;
int Cx, Cy, r;

bool isLine, hasCenter, isCircle;
int w, h;

// For colors
float red, g, b;

static void resize(int width, int height)
{
    //const float ar = (float) width / (float) height;
    w = width;
    h = height;


    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, width, 0.0, height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

// For when line being drawn has a slop less than pi/4
static void lessThan45(int x0, int y0, int x1, int y1, int multiplier)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int d = 2*dy - dx;
    int incrE = 2*dy;
    int incrNE = 2*(dy - dx);

    int x = x0, y = y0;
    if(multiplier == 1)
        glVertex2i(x, y);
    else
        glVertex2i(x, 2*y0 - y);

    while(x < x1){
        if(d <= 0){
            d += incrE;
        }else{
            d += incrNE;
            y++;
        }
        x++;
        if(multiplier == 1)
            glVertex2i(x, y);
        else
            glVertex2i(x, 2*y0 - y);

        }
}

// For when line being drawn has a slop greater than pi/4
static void moreThan45(int x0, int y0, int x1, int y1, int multiplier)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int d = dy - 2*dx;
    int incrN = -2*dx;
    int incrNE = 2*(dy - dx);

    int x = x0, y = y0;

    if(multiplier == 1)
        glVertex2i(x, y);
    else
        glVertex2i(x, 2*y0 - y);

    while(y < y1){
        if(d > 0){
            d += incrN;
        }else{
            d += incrNE;
            x++;
        }
        y++;

        if(multiplier == 1)
            glVertex2i(x, y);
        else
            glVertex2i(x, 2*y0 - y);
        }
}

// The midpoint line function
static void midpoint_line(int x0, int y0, int x1, int y1)
{
    if(x1 < x0){
        int t = x1;
        x1 = x0;
        x0 = t;

        t = y1;
        y1 = y0;
        y0 = t;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int multiplier = 1;
    if(dy * dx < 0){
        multiplier = -1;
        y1 = 2*y0 - y1;
    }

    if(dx == 0 && y1 < y0)
    {
        int t = y1;
        y1 = y0;
        y0 = t;
    }

    dy = y1 - y0;

    if(dy > dx){
        moreThan45(x0, y0, x1, y1, multiplier);
    }else{
        lessThan45(x0, y0, x1, y1, multiplier);
    }

}

// Function to perform 8-way symmetry for a circle
static void circlePoints(int x0, int y0, int x, int y)
{
    int x2 = 2*x0;
    int y2 = 2*y0;

    int t1 = x0 - y0;

    glColor3f(1.0, 0.0, 0.0);       //red
    glVertex2i(x, y);
    glColor3f(1.0, 1.0, 0.0);       //purple
    glVertex2i(x2 - x, y);
    glColor3f(1.0, 0.0, 0.0);       //red
    glVertex2i(x2 - x, y2 - y);
    glColor3f(1.0, 1.0, 0.0);       //purple
    glVertex2i(x, y2 - y);

    glColor3f(0.0, 1.0, 0.0);       //blue
    glVertex2i(y + t1, x - t1);
    glColor3f(0.0, 0.0, 1.0);       //green
    glVertex2i(y + t1, x2 - x - t1);
    glColor3f(0.0, 1.0, 0.0);       //blue
    glVertex2i(y2 - y + t1, x2 - x - t1);
    glColor3f(0.0, 0.0, 1.0);       //greem
    glVertex2i(y2 - y + t1, x - t1);

}

// midpoint circle function
static void midpoint_circle(int x0, int y0, int r)
{
    int x = x0;
    int y = y0 + r;

    int d = 1 - r;
    int deltaE = 3;
    int deltaSE = -2 * r + 5;

    while(y - y0 > x - x0)
    {
        if(d < 0)
        {
            d += deltaE;
            deltaE += 2;
            deltaSE += 2;
        }else
        {
            d += deltaSE;
            deltaE += 2;
            deltaSE += 4;
            y--;
        }
        x++;

        circlePoints(x0, y0 ,x, y);
    }

}

// Calculates the next point for L-systems and adds it to vector
static void nextPoint(int x0, int y0, int len, float angle, int index)
{
    int x1 = x0 + len*cos(curAngle*deg2rad);
    int y1 = y0 + len*sin(curAngle*deg2rad);

    auto it = lines.begin() + index;

    lines.push_back(x0);
    lines.push_back(y0);
    lines.push_back(x1);
    lines.push_back(y1);

    posX = x1;
    posY = y1;

    //cout << "added line: " << x0 << " " << y0 << "->" << x1 << " " << y1 << " Cur poss: " << posX << " " << posY << " cur angle: " << curAngle << "\n";
}

// L-Systems function
static void Lsystems()
{
    // Resets positon and startAngle for each iteration
    posX = (int)w/2;
    posY = 0;
    curAngle = 90;

    // Clears L-system lines
    lines.clear();

    // For decay
    len = len * lenMultiplier;

    // Number of iterations
    numIter++;

    // If no pixels to be rendered, abort
    if(len < 1)
    {
        cout << "length too small, nothing to be rendered\n";
        return;
    }

    //auto startTime = chrono::high_resolution_clock::now();

    // Loop through every letter in current string and apply grammar to it
    for(int i = 0; i < currentString.length(); i++)
    {
        char temp = currentString.at(i);
        //cout << "current char: " << temp << " ---- ";
        currentString.erase(i, 1);
        if(temp == '0')                                 // Just go forwards
        {
            nextPoint(posX, posY, len, curAngle, i);

            currentString.insert(i, rhs0);
            i += rhs0.length()-1;
        } else
        if(temp == '1')                                 // Just go forwards
        {
            nextPoint(posX, posY, len, curAngle, i);

            currentString.insert(i, rhs1);
            i += rhs1.length()-1;
        } else
        if(temp == '[')                                 // Push position and angle, turn by turnAngle
        {
            positions.push(posX);
            positions.push(posY);
            angles.push(curAngle);

            curAngle += turnAngle;

            //cout << "Encountered a opening bracket Cur poss: " << posX << " " << posY << " cur angle: " << curAngle << "\n";
            currentString.insert(i, rhsOpenB);
            i += rhsOpenB.length() - 1;
        }else
        if(temp == ']')                                 // Pop position and angle,
        {
            posY = positions.top();
            positions.pop();
            posX = positions.top();
            positions.pop();

            curAngle = angles.top();
            angles.pop();

            //cout << "Encountered a closing bracket Cur poss: " << posX << " " << posY << " cur angle: " << curAngle << "\n";
            currentString.insert(i, rhsClosedB);
            i += rhsClosedB.length() - 1;
        } else
        if(temp == '(')                                 // Push position and angle, turn by -turnAngle
        {
            positions.push(posX);
            positions.push(posY);
            angles.push(curAngle);

            curAngle -= turnAngle;

            //cout << "Encountered a opening bracket Cur poss: " << posX << " " << posY << " cur angle: " << curAngle << "\n";
            currentString.insert(i, rhsOpenP);
            i += rhsOpenP.length() - 1;
        }else
        if(temp == ')')                                 // Pop position and angle
        {
            posY = positions.top();
            positions.pop();
            posX = positions.top();
            positions.pop();

            curAngle = angles.top();
            angles.pop();

            //cout << "Encountered a closing bracket Cur poss: " << posX << " " << posY << " cur angle: " << curAngle << "\n";
            currentString.insert(i, rhsClosedP);
            i += rhsClosedP.length() - 1;
        }
        if(temp == '+')                                 // Just turn by turnAngle
        {
            curAngle += turnAngle;
            currentString.insert(i, rhsPlus);
            i += rhsPlus.length() - 1;
        }
        if(temp == '-')                                 // Just turn by -turnAngle
        {
            curAngle -= turnAngle;
            currentString.insert(i, rhsMinus);
            i += rhsMinus.length() - 1;
        }
    }

    //auto endTime = chrono::high_resolution_clock::now();

    //auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime);

    //cout << "Current String: " << currentString << "\n";
    cout << "Current Iteration: " << numIter << " Current len == " << len << "\n";//time taken: " << duration.count() << "\n";
}

// Cycles through colors for L-system rendering
static void nextColor()
{
    if(red == 1 && g == 0 && b == 0)
    {
        red = 0;
        g = 1;
    }else if(red == 0 && g == 1 && b == 0)
    {
        g = 0;
        b = 1;
    }else if(red == 0 && g == 0 && b == 1)
    {
        red = 1;
        g = 1;
        b = 0;
    }else if(red == 1 && g == 1 && b == 0)
    {
        g = 0;
        b = 1;
    } else if(red == 1 && g == 0 && b == 1)
    {
        red = 0;
        g = 1;
    } else
    {
        red = 1;
        g = 0;
        b = 0;
    }
}

static void display(void)
{
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    glBegin(GL_POINTS);

    // lines are red
    glColor3f(1.0, 0.0, 0.0);
    if(isLine)
       midpoint_line(points.at(0), points.at(1), points.at(2), points.at(3));

    midpoint_line(100, 100, 100, 50);

    // if circle, render it
    if(isCircle)
    {
        midpoint_circle(Cx, Cy, r);

        glColor3f(1.0, 0.0, 1.0);
        glVertex2i(Cx, Cy);
    }

    // render L-system lines
    for(int i = 0; i < lines.size(); i++)
    {
        glColor3f(red, g, b);
        midpoint_line(lines.at(i),
                      lines.at(i+1),
                      lines.at(i+2),
                      lines.at(i+3));

        //nextColor();
        i += 3;
    }

    red = 1;
    g = 0;
    b = 0;


    glEnd();

    glFlush();
}

void mousePress(int button, int state, int x, int y)
{
    // Right button to input line points
    if(button == GLUT_RIGHT_BUTTON    && state == GLUT_DOWN)
    {
        if(points.size() == 4){
            std::cout << "Cleared points\n";
            points.pop_back();
            points.pop_back();
            points.pop_back();
            points.pop_back();

            isLine = false;
        }else
        {
        std::cout << "Added a point " << x << " " << h-y << "\n";
        points.push_back(x);
        points.push_back(h - y);

        if(points.size() == 4)
            isLine = true;
        }
    }

    // left button for circle
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if(isCircle)
        {
            hasCenter = false;
            isCircle = false;

            return;
        }

        if(!hasCenter)
        {
            Cx = x;
            Cy = h - y;
            std::cout << "Center: " << x << ", " << y << "\n";
            hasCenter = true;
        }else
        {
            int delX = (x - Cx);
            int delY = (h - y - Cy);
            r = (int)sqrt(delX*delX + delY*delY);
            std::cout << "OuterPt: " << x << ", " << y << "\n";
            std::cout << "Radius: " << r << "\n";
            isCircle = true;
        }
    }
}

static void key(unsigned char key, int x, int y)
{
    // If q is pressed, exit, if space is pressed, progress L-system
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;
        case 32:
            Lsystems();
            break;
    }

    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

/* Program entry point */

int main(int argc, char *argv[])
{
    // init
    w = 1920;
    h = 1080;

    // For circle
    isCircle = false;
    hasCenter = false;

    // for L- system
    posX = (int)w/2;
    posY = 0;
    curAngle = 90;
    turnAngle = 30;

    len = 20;
    lenMultiplier = 0.9;

    red = 1;
    g = 0;
    b = 0;


    cout << "Current Iteration: " << numIter << " Current len == " << len << "\n";

    glutInit(&argc, argv);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

    glutCreateWindow("Bresenham's line renderer, the midway circle and a render of certain L-Systems");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutMouseFunc(mousePress);
    glutIdleFunc(idle);

    glutMainLoop();

    return EXIT_SUCCESS;
}
