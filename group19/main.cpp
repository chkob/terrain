// Copyright (C) 2014 - LGG EPFL

#include <iostream>
#include <sstream>

#include "common.h"
#include "heightmap.h"
#include "shadowmap.h"
#include "watermap.h"
#include "waterReflection.h"
#include "skybox.h"
#include "particles_control.h"
#include "particles_render.h"
#include "terrain.h"
#include "camera_control.h"
#include "camera_path.h"
#include "vertices.h"
#include "vertices_quad.h"
#include "vertices_grid.h"
#include "vertices_skybox.h"
#include "vertices_camera_path.h"


/// Screen size.
const unsigned int windowWidth(1024);
const unsigned int windowHeight(768);

/// Textures (heightmap and shadowmap) sizes.
const unsigned int textureWidth(1024);
const unsigned int textureHeight(1024);

/// Number of particles on the side. That makes nParticlesSide^3 particles.
const unsigned int nParticlesSide(20);

/// Instanciate the rendering contexts that render to the screen.
Skybox skybox(windowWidth, windowHeight);
Terrain terrain(windowWidth, windowHeight);
CameraPath cameraPath(windowWidth, windowHeight);
ParticlesRender particlesRender(windowWidth, windowHeight, nParticlesSide);

/// Instanciate the rendering contexts that render to FBO.
Shadowmap shadowmap(textureWidth, textureHeight);
ParticlesControl particlesControl(nParticlesSide);

Watermap water(windowWidth, windowHeight);
WaterReflection reflection(windowWidth, windowHeight);

/// Camera position controller.
CameraControl cameraControl;

/// Instanciate the vertices.
Vertices* verticesQuad = new VerticesQuad();
Vertices* verticesGrid = new VerticesGrid();
Vertices* verticesSkybox = new VerticesSkybox();
VerticesCameraPath* verticesCameraPath = new VerticesCameraPath();

/// Matrices that have to be shared between functions.
static mat4 lightMVP;
static vec3 lightPositionModel;



/// Projection parameters.
// Horizontal field of view in degrees : amount of "zoom" ("camera lens").
// Usually between 90° (extra wide) and 30° (quite zoomed in).
const float fieldOfView = 45.0f;
// Aspect ratio depends on the window size (for example 4/3 or 1).
const float aspectRatio = float(windowWidth) / float(windowHeight);
// Near clipping plane. Keep as little as possible (precision issues).
const float nearPlane = 0.1f;
// Far clipping plane. Keep as big as possible (usually 10.0f or 100.0f).
const float farPlane  = 100.0f;

/// Camera projection matrix (camera intrinsics).
const mat4 cameraProjection = Eigen::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);

/// Spot light projection matrix.
const mat4 lightProjection = Eigen::perspective(fieldOfView, float(textureWidth)/float(textureHeight), nearPlane, farPlane);


/// Key press callback.
void GLFWCALL keyboard_callback(int key, int action) {

    /// Distance from center (0,0,0) to sun.
    const float r = 3.0f;
	std::cout << "Pressed key : " << key << std::endl;

    if(action == GLFW_PRESS) {

       // std::cout << "Pressed key : " << key << std::endl;

        /// 49 corressponds to 1, 57 to 9 (keyboard top keys).
        if(key >= 49 && key <= 57) {

            /// Angle from 0° (key 1) to 90° (key 9).
            float theta = M_PI / 8.0f * float(key-49);

            /// Position from sunrise (-r,0,0) to noon (0,0,r).
            lightPositionModel = vec3(-std::cos(theta)*r, 0.0, std::sin(theta)*r);

            /// Light source position (model coordinates).
            const vec3 lightLookAt(0.0, 0.0, 0.0);
            const vec3 lightUp(0.0, 1.0, 0.0);
            mat4 lightView = Eigen::lookAt(lightPositionModel, lightLookAt, lightUp);

            /// Assemble the lightMVP matrix for a spotlight source.
            lightMVP = lightProjection * lightView;
        }
    }
    cameraControl.handleCameraControls(key, action);
}


void init() {
	
    /// OpenGL parameters.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_CLIP_DISTANCE0);

	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    //glEnable(GL_CULL_FACE);

    /// Generate the vertices.
    verticesQuad->generate();
    verticesGrid->generate();
    verticesSkybox->generate();
    verticesCameraPath->generate();

    /// Generate the heightmap texture.
    Heightmap heightmap(textureWidth, textureHeight);
    GLuint heightMapTexID = heightmap.init(verticesQuad);
	heightmap.draw();
    heightmap.clean();
//    verticesQuad->clean();
//    delete verticesQuad;

    /// Initialize the rendering contexts.
    GLuint shadowMapTexID = shadowmap.init(verticesGrid, heightMapTexID);
    terrain.init(verticesGrid, heightMapTexID, shadowMapTexID);
    skybox.init(verticesSkybox);

    water.init(verticesGrid, heightMapTexID);
    //reflection.init(verticesGrid, heightMapTexID);

    /// Pass the particles position textures from control to render.
    GLuint particlePosTexID[2];
    particlesControl.init(verticesQuad, particlePosTexID);
    particlesRender.init(particlePosTexID);

    /// CameraPath is a rendering object.
    /// Camera is able to change the rendered vertices.
    cameraControl.init(verticesCameraPath, heightMapTexID);
    cameraPath.init(verticesCameraPath);

    /// Initialize the light position.
    keyboard_callback(50, GLFW_PRESS);

}


void display() {

    /// Measure and print FPS (every second).
	static double lastTime = glfwGetTime();
    static int nbFrames = 0;
    double currentTime = glfwGetTime();
    nbFrames++;
    if(currentTime - lastTime >= 1.0) {
        std::cout << nbFrames << " FPS" << std::endl;
        nbFrames = 0;
        lastTime = currentTime;
    }

    /// Control the camera position.
    mat4 cameraModelview;
    cameraControl.updateCameraPosition(cameraModelview);

	/// Uncomment to render only the boundary (not full triangles).
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Generate the shadowmap.
    shadowmap.draw(lightMVP);

    /// Render opaque primitives on screen.
    terrain.draw(cameraProjection, cameraModelview, lightMVP, lightPositionModel);
    skybox.draw(cameraProjection, cameraModelview);
    cameraPath.draw(cameraProjection, cameraModelview);


//    water.draw(cameraProjection, cameraModelview, flippedCameraModelview, lightMVP, lightPositionModel);
//    reflection.draw(cameraProjection, flippedCameraModelview, lightMVP, lightPositionModel);


    /// First control particle positions, then render them on screen.
    /// Render the translucent primitives last. Otherwise opaque objects that
    /// may be visible behind get discarded by the depth test.
    particlesControl.draw();
    particlesRender.draw(cameraProjection, cameraModelview);

}


void trackball(const mat4& model) {
    cameraControl.trackball(model);
}


int main(int, char**) {
    glfwInitWindowSize(windowWidth, windowHeight);
    glfwCreateWindow("EPFL - Computer Graphics - Project - Group 19");
    glfwDisplayFunc(display);
    init();
    glfwTrackball(trackball);
    glfwSetKeyCallback(keyboard_callback);
    glfwMainLoop();
    return EXIT_SUCCESS;    
}
