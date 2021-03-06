
#ifndef __vertices_quad_h__
#define __vertices_quad_h__

#include "vertices.h"

class VerticesQuad : public Vertices {

public:

    /// Generate the vertices.
    void generate();

    /// Bind the vertex attribute to the VBO (retained in VAO state).
    void bind(unsigned int vertexAttribIDs[]) const;

    /// Draw the scene.
    void draw() const;

    /// Delete the buffers.
    void clean();

};

#endif /* __vertices_quad_h__ */
