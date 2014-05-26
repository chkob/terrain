
#include "rendering_simple.h"
#include "vertices.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glfw.h>
#include "opengp.h"

#include "rendering_simple_vshader.h"
#include "rendering_simple_fshader.h"


RenderingSimple::RenderingSimple(unsigned int width, unsigned int height) :
    RenderingContext(width, height) {
}


void RenderingSimple::init(Vertices* vertices) {

    /// Common initialization.
    RenderingContext::init(vertices, rendering_simple_vshader, rendering_simple_fshader, NULL, "vertexPosition3DModel", 0);

    /// Set uniform IDs.
    _projectionID = glGetUniformLocation(_programID, "projection");
    _viewID = glGetUniformLocation(_programID, "view");
    _modelID = glGetUniformLocation(_programID, "model");
    _colorID = glGetUniformLocation(_programID, "color");

}


void RenderingSimple::draw(const mat4& projection, const mat4& view, const mat4& model, const vec3& color) const {

    /// Common drawing.
    RenderingContext::draw();

    /// Update the content of the uniforms.
    glUniformMatrix4fv( _projectionID, 1, GL_FALSE, projection.data());
    glUniformMatrix4fv( _viewID, 1, GL_FALSE, view.data());
    glUniformMatrix4fv( _modelID, 1, GL_FALSE, model.data());
    glUniform3fv( _colorID, 1, color.data());

    /// Do not clear the default framebuffer (screen) : done by Terrain.
    /// Otherwise already drawn pixels will be cleared.

    /// Render to default framebuffer.
    _vertices->draw();

}