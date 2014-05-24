
#include "terrain.h"
#include "vertices.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glfw.h>
#include "opengp.h"

#include "terrain_vshader.h"
#include "terrain_fshader.h"


Terrain::Terrain(unsigned int width, unsigned int height) :
    RenderingContext(width, height) {
}


void Terrain::init(Vertices* vertices, GLuint heightMapTexID, GLuint shadowMapTexID) {

    /// Common initialization.
    RenderingContext::init(vertices, terrain_vshader, terrain_fshader, "vertexPosition2DModel", 0);

    /// Bind the heightmap and shadowmap to textures 0 and 1.
    set_texture(0, heightMapTexID, "heightMapTex", GL_TEXTURE_2D);
    set_texture(1, shadowMapTexID, "shadowMapTex", GL_TEXTURE_2D);

    /// Load material textures and bind them to textures 2 - 7.
    set_texture(2, -1, "sandTex", GL_TEXTURE_2D);
    load_texture("../../textures/sand.tga");
    set_texture(3, -1, "iceMoutainTex", GL_TEXTURE_2D);
    load_texture("../../textures/dordona_range.tga");
    set_texture(4, -1, "treeTex", GL_TEXTURE_2D);
    load_texture("../../textures/Mossy_Rock.tga");
    set_texture(5, -1, "stoneTex", GL_TEXTURE_2D);
    load_texture("../../textures/Fault_Zone.tga");
    set_texture(6, -1, "underWaterTex", GL_TEXTURE_2D);
    load_texture("../../textures/under_water.tga");
    set_texture(7, -1, "snowTex", GL_TEXTURE_2D);
	load_texture("../../textures/snow.tga");
  
    /// Define light properties and pass them to the shaders.
    vec3 Ia(1.0f, 1.0f, 1.0f);
    vec3 Id(1.0f, 1.0f, 1.0f);
    vec3 Is(1.0f, 1.0f, 1.0f);
    GLuint _IaID = glGetUniformLocation(_programID, "Ia");
    GLuint _IdID = glGetUniformLocation(_programID, "Id");
    GLuint _IsID = glGetUniformLocation(_programID, "Is");
    glProgramUniform3fv(_programID, _IaID, 1, Ia.data());
    glProgramUniform3fv(_programID, _IdID, 1, Id.data());
    glProgramUniform3fv(_programID, _IsID, 1, Is.data());

    /// Set uniform IDs.
    _modelviewID = glGetUniformLocation(_programID, "modelview");
    _projectionID = glGetUniformLocation(_programID, "projection");
    _lightMVPID = glGetUniformLocation(_programID, "lightMVP");
    _lightPositionModelID = glGetUniformLocation(_programID, "lightPositionModel");
    _timeID = glGetUniformLocation(_programID, "time");

}


void Terrain::draw(const mat4& projection, const mat4& modelview,
                   const mat4& lightMVP, const vec3& lightPositionModel) const {

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /// Common drawing. 
    RenderingContext::draw();

    /// Update the content of the uniforms.
    glProgramUniformMatrix4fv(_programID, _modelviewID, 1, GL_FALSE, modelview.data());
    glProgramUniformMatrix4fv(_programID, _projectionID, 1, GL_FALSE, projection.data());
    glProgramUniformMatrix4fv(_programID, _lightMVPID, 1, GL_FALSE, lightMVP.data());
    glProgramUniform3fv(_programID, _lightPositionModelID, 1, lightPositionModel.data());

    /// Clear the default framebuffer (screen).
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Render the terrain from camera point of view to default framebuffer.
    _vertices->draw();

	//Disable blending
	glDisable(GL_BLEND);

}


GLuint Terrain::load_texture(const char * imagepath) const {

    // Read the file, call glTexImage2D with the right parameters
    if (glfwLoadTexture2D(imagepath, 0)) {
        // Nice trilinear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Cannot load texture file : " << imagepath << std::endl;
        exit(EXIT_FAILURE);
    }

}
