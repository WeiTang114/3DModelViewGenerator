#define GL_GLEXT_PROTOTYPES
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include "mesh.h"
#include "view_post_proc.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>
#else
#include <GL/glu.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#endif

using namespace std;

typedef unsigned char UCHAR;

const string OFF_DIR = "/Users/weitang114/Dev/Sketch3d/dataset/SHREC13_SBR_TARGET_MODELS/models/";
const string IMG_DIR = "/Users/weitang114/Dev/Sketch3d/dataset/SHREC13_SBR_TARGET_MODELS/views/";
bool offscreen = true;
int W = 100;
int H = 100;
static Mesh *mesh = NULL;

// 2 directions
const int numberOfPoints = 2;
static float radius = 1.0;
static float eyeVertex[numberOfPoints][3] = {
    {  4.0824829e-001,  4.0824829e-001,  8.1649658e-001}, // 28
    { -0.61237243, 0.5, 0.61237243},
    // {  9.2387953e-001,  3.8268343e-001,  0.0000000e+000}, // 31ma
    // { 0, 0, 1},
};


bool onlyAmbient = false;








time_t timeBegin, timeEnd;

GLdouble eyeX = 0.0;
GLdouble eyeY = 0.0;
GLdouble eyeZ = 0.0;
// GLdouble eyeZ = 1.73205081;
// GLdouble centerX = 0.0;
GLdouble centerX = 0.0;
GLdouble centerY = 0.0;
GLdouble centerZ = 0.0;
GLdouble upX = 0.0;
GLdouble upY = 0.0;
GLdouble upZ = 1.0;


int forEyeView = 0;
float localScaling;

// Program arguments

static char *filename = 0;



// for glutTimerFunc and saveImage
int globalX = 0;
int globalY = 0;
int globalZ = 0;

// Display variables

static int scaling = 0;
static int translating = 0;
static int rotating = 0;
static float scale = 1.0;
static float center[3] = { 0.0, 0.0, 0.0 };
static float rotation[3] = { 0.0, 0.0, 0.0 };
static float translation[3] = { 0.0, 0.0, -4.0 };




void init();
string buildOffPath(int offIdx);
void createImgDir(int offIdx);
string buildImgPath(int offIdx, int id);
void display();
Mesh * readOffFile(string filename);
void initOffscreen();
void setBoxSize(void);
void initMatrices();
void showAndCapture(int id);
void findUpVector();
void saveImage(string);
// void thinningIteration(cv::Mat& im, int iter);
// void thinning(cv::Mat& im);
// void callCvSmooth(cv::Mat srcmtx,
//                   cv::Mat dstmtx,
//                   int smooth_type,
//                   int param1,
//                   int param2,
//                   double param3,
//                   double param4 );


int main(int argc, char **argv) {
    GLFWwindow * window;

    /* Initialize the library */
    if (!glfwInit()) return - 1;

    if (argc != 2) {
        cout << "argc != 2" << endl;
        return -1;
    }
    int offIdx = atoi(argv[1]);
    string offFile = buildOffPath(offIdx);
    mesh = readOffFile(offFile);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow( W/2, H/2,  " Hello World " , NULL, NULL);
    if (!window) {
        glfwTerminate();
        return - 1 ;
    }

    glfwMakeContextCurrent(window);



    int width, height;
    glfwGetWindowSize(window, &width, &height);
    // cout << "window size:" << width << "," << height << endl;

    glfwGetFramebufferSize(window, &width, &height);
    // cout << "framebuffer size:" << width << "," << height << endl;

    init();

    if (offscreen) {
    	initOffscreen();
    }
    glfwHideWindow(window);

    setBoxSize();
    initMatrices();

    double sec = 0;
    int id = 0;
    /* Loop until the user closes the window */
    while (! glfwWindowShouldClose(window)) {

        /* Poll for and process events */
        glfwPollEvents();
        sleep(0);
        double tmpSec = glfwGetTime();
        createImgDir(offIdx);
        if (tmpSec - sec > 0.001) {
            string imgPath = buildImgPath(offIdx, id);
            showAndCapture(id);

            id++;
        	sec = tmpSec;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
            saveImage(imgPath);

            // sleep(2);

            if (id == numberOfPoints) break;
        }
    }

    glfwTerminate();
    return  0 ;
}

void init() {
    static GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    static GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    if (!onlyAmbient) {
        glEnable(GL_LIGHT0); // only ambient for silhouette
    }

    static GLfloat light1_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    if (!onlyAmbient) {
        glEnable(GL_LIGHT1); // only ambient for silhouette
    }
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);

    // Initialize graphics modes
    glEnable(GL_DEPTH_TEST);
}

string buildOffPath(int offIdx) {
    stringstream ss;
    ss << OFF_DIR << "m" << offIdx << ".off";
    return ss.str();
}

string buildImgPath(int offIdx, int id) {
    char filename[16] = "";
    sprintf(filename, "m%d_%03d.png", offIdx, id);
    stringstream ss;
    ss << IMG_DIR << "m" << offIdx
        << "/" << filename;
    return ss.str();
}

void createImgDir(int offIdx) {
    stringstream ss;
    ss<< IMG_DIR << "m" << offIdx;
    mkdir(ss.str().c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void initOffscreen() {
    // render on memory
    GLuint fbo, rboColor, rboDepth;

    // Color renderbuffer.
    glGenRenderbuffers(1,&rboColor);
    glBindRenderbuffer(GL_RENDERBUFFER, rboColor);
    // Set storage for currently bound renderbuffer.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, W, H);


    // DEPTH
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, W, H
    );


    // Framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColor);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            cout << "Unsupported framebuffer." << endl;
            exit(0);
    }


    // Set to write to the framebuffer.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbo);

    // Tell glReadPixels where to read from.
    //glReadBuffer(GL_COLOR_ATTACHMENT0);


}


Mesh * readOffFile(string filenameStr) {
    int i;
    const char *filename = filenameStr.c_str();

    // Open file
    FILE *fp;
    if (!(fp = fopen(filename, "r"))) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        return 0;
    }

    // Allocate mesh structure
    Mesh *mesh = new Mesh();
    if (!mesh) {
        fprintf(stderr, "Unable to allocate memory for file %s\n", filename);
        fclose(fp);
        return 0;
    }

    // Read file
    int nverts = 0;
    int nfaces = 0;
    int nedges = 0;
    int line_count = 0;
    char buffer[1024];
    while (fgets(buffer, 1023, fp)) {
        // Increment line counter
        line_count++;

        // Skip white space
        char *bufferp = buffer;
        while (isspace(*bufferp)) bufferp++;

        // Skip blank lines and comments
        if (*bufferp == '#') continue;
        if (*bufferp == '\0') continue;

        // Check section
        if (nverts == 0) {
            // Read header
            if (!strstr(bufferp, "OFF")) {
                // Read mesh counts
                if ((sscanf(bufferp, "%d%d%d", &nverts, &nfaces, &nedges) != 3) || (nverts == 0)) {
                    fprintf(stderr, "Syntax error reading header on line %d in file %s\n", line_count, filename);
                    fclose(fp);
                    return NULL;
                }

                // Allocate memory for mesh
                mesh->verts = new Vertex [nverts];
                assert(mesh->verts);
                mesh->faces = new Face [nfaces];
                assert(mesh->faces);
            }
        }
        else if (mesh->nverts < nverts) {
            // Read vertex coordinates
            Vertex& vert = mesh->verts[mesh->nverts++];
            if (sscanf(bufferp, "%f%f%f", &(vert.x), &(vert.y), &(vert.z)) != 3) {
                fprintf(stderr, "Syntax error with vertex coordinates on line %d in file %s\n", line_count, filename);
                fclose(fp);
                return NULL;
            }
        }
        else if (mesh->nfaces < nfaces) {
            // Get next face
            Face& face = mesh->faces[mesh->nfaces++];

            // Read number of vertices in face
            bufferp = strtok(bufferp, " \t");
            if (bufferp) face.nverts = atoi(bufferp);
            else {
                fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
                fclose(fp);
                return NULL;
            }

            // Allocate memory for face vertices
            face.verts = new Vertex *[face.nverts];
            assert(face.verts);

            // Read vertex indices for face
            for (i = 0; i < face.nverts; i++) {
                bufferp = strtok(NULL, " \t");
                if (bufferp) face.verts[i] = &(mesh->verts[atoi(bufferp)]);
                else {
                    fprintf(stderr, "Syntax error with face on line %d in file %s\n", line_count, filename);
                    fclose(fp);
                    return NULL;
                }
            }

            // Compute normal for face
            face.normal[0] = face.normal[1] = face.normal[2] = 0;
            Vertex *v1 = face.verts[face.nverts-1];
            for (i = 0; i < face.nverts; i++) {
                Vertex *v2 = face.verts[i];
                face.normal[0] += (v1->y - v2->y) * (v1->z + v2->z);
                face.normal[1] += (v1->z - v2->z) * (v1->x + v2->x);
                face.normal[2] += (v1->x - v2->x) * (v1->y + v2->y);
                v1 = v2;
            }

            // Normalize normal for face
            float squared_normal_length = 0.0;
            squared_normal_length += face.normal[0]*face.normal[0];
            squared_normal_length += face.normal[1]*face.normal[1];
            squared_normal_length += face.normal[2]*face.normal[2];
            float normal_length = sqrt(squared_normal_length);
            if (normal_length > 1.0E-6) {
                face.normal[0] /= normal_length;
                face.normal[1] /= normal_length;
                face.normal[2] /= normal_length;
            }
        }
        else {
            // Should never get here
            fprintf(stderr, "Found extra text starting at line %d in file %s\n", line_count, filename);
            break;
        }
    }

    // Check whether read all faces
    if (nfaces != mesh->nfaces) {
        fprintf(stderr, "Expected %d faces, but read only %d faces in file %s\n", nfaces, mesh->nfaces, filename);
    }

    // Close file
    fclose(fp);

    // Return mesh
    return mesh;
}



void setBoxSize(void) {
    float boxUpperX = -1.0E30F;
    float boxLowerX = 1.0E30F;
    float boxUpperY = -1.0E30F;
    float boxLowerY = 1.0E30F;
    float boxUpperZ = -1.0E30F;
    float boxLowerZ = 1.0E30F;
    for (int i = 0; i < mesh->nverts; i++) {
        Vertex& vert = mesh->verts[i];
        if (vert.x < boxLowerX) {
            boxLowerX = vert.x;
        } else if (vert.x > boxUpperX) {
            boxUpperX = vert.x;
        }
        if (vert.y < boxLowerY) {
            boxLowerY = vert.y;
        } else if (vert.y > boxUpperY) {
            boxUpperY = vert.y;
        }
        if (vert.z < boxLowerZ) {
            boxLowerZ = vert.z;
        } else if (vert.z > boxUpperZ) {
            boxUpperZ = vert.z;
        }
    }

    // printf("in setBoxSize at D%05d.bmp\n", (itsfilenumber-1));
    // printf("center[0] = %f, center[1] = %f, center[2] = %f\n", center[0], center[1], center[2]);
    // printf("scale = %f\n", scale);

    // Setup initial viewing scale
    float dx = boxUpperX - boxLowerX;
    float dy = boxUpperY - boxLowerY;
    float dz = boxUpperZ - boxLowerZ;
    scale = 2.0 / sqrt(dx*dx + dy*dy + dz*dz);

    // Setup initial viewing center
    center[0] = 0.5 * (boxUpperX + boxLowerX);
    center[1] = 0.5 * (boxUpperY + boxLowerY);
    center[2] = 0.5 * (boxUpperZ + boxLowerZ);


    // cout << "u x " << boxUpperX << endl
    //      << "l x " << boxLowerX << endl
    //      << "u y " << boxUpperY << endl
    //      << "l y " << boxLowerY << endl
    //      << "u z " << boxUpperZ << endl
    //      << "l z " << boxLowerZ << endl;
    // cout << "center " << center[0] << ","
    //      << center[1] << ","
    //      << center[2] << endl;
    // cout << "scale " << scale << endl;

    // printf("center[0] = %f, center[1] = %f, center[2] = %f\n", center[0], center[1], center[2]);
    // printf("scale = %f\n", scale);
    // printf("\n");
}



void initMatrices() {
    glLoadIdentity();
    glScalef(scale, scale, scale);
    glTranslatef(translation[0], translation[1], 0.0);

    // Set projection transformation
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat) W /(GLfloat) H, 0.1, 100.0);

    // Set camera transformation
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translation[0], translation[1], translation[2]);
    glScalef(scale, scale, scale);
    glRotatef(rotation[0], 1.0, 0.0, 0.0);
    glRotatef(rotation[1], 0.0, 1.0, 0.0);
    glRotatef(rotation[2], 0.0, 0.0, 1.0);
    glTranslatef(-center[0], -center[1], -center[2]);
}


void showAndCapture(int id) {
    cout << "id = " << id << endl;
    eyeX = eyeVertex[id][0];
    eyeY = eyeVertex[id][1];
    eyeZ = eyeVertex[id][2];
    findUpVector();
    display();
}

void findUpVector() {
    if (0.0 == eyeZ) {
        upX = 0.0;
        upY = 0.0;
        upZ = 1.0;
    } else if (0.0 == eyeY) {
        upX = 0.0;
        upY = 1.0;
        upZ = 0.0;
    } else {
        upX = eyeY;
        upY = eyeX * (eyeX >= 0 ? 1 : -1);
        upZ = 0.0;
    }
}

void display() {



    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(scale, scale, scale);
    localScaling = 4 / (scale * radius);
    // cout << "localScaling " << localScaling << endl;
    gluLookAt(localScaling * eyeX,
              localScaling * eyeY,
              localScaling * eyeZ,
              center[0],
              center[1],
              center[2],
              upX,
              upY,
              upZ
    );

    // cout << "localScaling * eyeX " << localScaling * eyeX
    //      << ", eyeX " << eyeX << endl
    //      << "localScaling * eyeY " << localScaling * eyeY
    //      << ", eyeY " << eyeY  << endl
    //      << "localScaling * eyeZ " << localScaling * eyeZ
    //      << ", eyeZ " << eyeZ  << endl
    //      << "upX " << upX << endl
    //      << "upY " << upY << endl
    //      << "upZ " << upZ << endl;


    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set lights
    static GLfloat light0_position[] = { 3.0, 4.0, 5.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    static GLfloat light1_position[] = { -3.0, -2.0, -3.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    // Set material
    static GLfloat material[] = { 1.0, 0.5, 0.5, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);

    // Draw faces
    for (int i = 0; i < mesh->nfaces; i++) {
        Face& face = mesh->faces[i];
        glBegin(GL_POLYGON);
        glNormal3fv(face.normal);
        for (int j = 0; j < face.nverts; j++) {
            Vertex *vert = face.verts[j];
            glVertex3f(vert->x, vert->y, vert->z);
        }
        glEnd();
    }
}


void saveImage(string path) {
	int arrLen = W * H * 3;
    GLbyte* colorArr = new GLbyte[ arrLen ];

    if (offscreen) glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, colorArr);

    cv::Mat img;
    std::vector<cv::Mat> imgPlanes(3);
    img.create(H, W, CV_8UC3);
    cv::split(img, imgPlanes);

    int count = 0;
    for(int i = 0; i < H; i ++) {
        UCHAR* plane0Ptr = imgPlanes[0].ptr<UCHAR>(i);
        UCHAR* plane1Ptr = imgPlanes[1].ptr<UCHAR>(i);
        UCHAR* plane2Ptr = imgPlanes[2].ptr<UCHAR>(i);
        for(int j = 0; j < W; j ++) {
            int k = 3 * (i * W + j);
            // BGR to RGB
            plane2Ptr[j] = colorArr[k];
            plane1Ptr[j] = colorArr[k+1];
            plane0Ptr[j] = colorArr[k+2];
            count ++;
        }
    }


    // TEST
    // cv::Mat img2(H, W, CV_8UC3);
    // //use fast 4-byte alignment (default anyway) if possible
    // glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

    // //set length of one complete row in destination data (doesn't need to equal img.cols)
    // glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());

    // glReadPixels(0, 0, img2.cols, img2.rows, GL_BGR, GL_UNSIGNED_BYTE, img2.data);
    // cv::Mat flipped;
    // cv::flip(img2, flipped, 0);
    // imshow("img2 ", flipped);

    // TEST END


    cv::merge(imgPlanes, img);
    imwrite("/Users/weitang114/Dev/Sketch3d/dataset/SHREC13_SBR_TARGET_MODELS/models/m1/test1.bmp", img);


    cv::flip(img, img ,0); // !!!

    ViewProcessor viewProc(img, W, H);
    viewProc.gray();
    viewProc.canny();
    viewProc.black2white();
    viewProc.blur();
    viewProc.save(path);

    img.release();
    delete [] colorArr;
}

