
//Cylinder Class modeled after Song Ho Ahn (http://www.songho.ca/opengl/gl_cylinder.html)

#include <iostream>         // Output log and error
#include <cstdlib>          // C Standard Library for EXIT_FAILURE
#include <string>
#include <fstream>
#include <sstream>

#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Cylinder.h"

using namespace std; // standard namespace

const GLint MIN_SECTOR_COUNT = 3;
const GLint MIN_STACK_COUNT = 1;



    Cylinder::Cylinder(GLfloat bRadius, GLfloat tRadius, GLfloat height, GLint numSlices, GLint numStacks) : iStride(32)
    {
        set(bRadius, tRadius, height, numSlices, numStacks);
    }

    void Cylinder::set(GLfloat bRadius, GLfloat tRadius, GLfloat height, GLint numSlices, GLint numStacks)
    {
        this->bRadius = bRadius;
        this->tRadius = tRadius;
        this->height = height;
        this->numSlices = numSlices;
        if (numSlices < MIN_SECTOR_COUNT)
            this->numSlices = MIN_SECTOR_COUNT;
        this->numStacks = numStacks;
        if (numStacks < MIN_STACK_COUNT)
            this->numStacks = MIN_STACK_COUNT;

        // generate unit circle vertices first
        buildUnitCircleVertices();
        //Build with smoothing
        buildVerticesSmooth();

    }

    void Cylinder::setBaseRadius(GLfloat newBRadius)
    {
        if (this->bRadius != newBRadius)
            set(newBRadius, tRadius, height, numSlices, numStacks);
    }

    void Cylinder::setTopRadius(GLfloat newTRadius)
    {
        if (this->tRadius != newTRadius)
            set(bRadius, newTRadius, height, numSlices, numStacks);
    }

    void Cylinder::setHeight(GLfloat newHeight)
    {
        if (this->height != newHeight)
            set(bRadius, tRadius, newHeight, numSlices, numStacks);
    }

    void Cylinder::setSectorCount(GLint newSlices)
    {
        if (this->numSlices != newSlices)
            set(bRadius, tRadius, height, newSlices, numStacks);
    }

    void Cylinder::setStackCount(GLint newStacks)
    {
        if (this->numStacks != newStacks)
            set(bRadius, tRadius, height, numSlices, newStacks);
    }

    void Cylinder::clearArrays()
    {
        vector<GLfloat>().swap(vertices);
        vector<GLfloat>().swap(normals);
        vector<GLfloat>().swap(texCoords);
        vector<GLuint>().swap(indices);
        vector<GLuint>().swap(lineIndices);
    }

    void Cylinder::buildVerticesSmooth()
    {
        // clear memory of prev arrays
        clearArrays();

        GLfloat x, y, z;                                  // vertex position
        //float s, t;                                     // texCoord
        GLfloat radius;                                   // radius for each stack

        // get normals for cylinder sides
        vector<GLfloat> sideNormals = getSideNorms();

        // put vertices of side cylinder to array by scaling unit circle
        for (GLint i = 0; i <= numStacks; ++i)
        {
            z = -(height * 0.5f) + (GLfloat)i / numStacks * height;      // vertex position z
            radius = bRadius + (GLfloat)i / numStacks * (tRadius - bRadius);     // lerp
            GLfloat t = 1.0f - (GLfloat)i / numStacks;   // top-to-bottom

            for (GLint j = 0, k = 0; j <= numSlices; ++j, k += 3)
            {
                x = unitCircleVertices[k];
                y = unitCircleVertices[k + 1];
                addVert(x * radius, y * radius, z);   // position
                addNorm(sideNormals[k], sideNormals[k + 1], sideNormals[k + 2]); // normal
                addTextCoord((GLfloat)j / numSlices, t); // tex coord
            }
        }

        // remember where the base.top vertices start
        GLuint baseVertexIndex = (GLuint)vertices.size() / 3;

        // put vertices of base of cylinder
        z = -height * 0.5f;
        addVert(0, 0, z);
        addNorm(0, 0, -1);
        addTextCoord(0.5f, 0.5f);
        for (GLint i = 0, j = 0; i < numSlices; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVert(x * bRadius, y * bRadius, z);
            addNorm(0, 0, -1);
            addTextCoord(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f);    // flip horizontal
        }

        // remember where the base vertices start
        GLuint topVertexIndex = (GLuint)vertices.size() / 3;

        // put vertices of top of cylinder
        z = height * 0.5f;
        addVert(0, 0, z);
        addNorm(0, 0, 1);
        addTextCoord(0.5f, 0.5f);
        for (GLint i = 0, j = 0; i < numSlices; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVert(x * tRadius, y * tRadius, z);
            addNorm(0, 0, 1);
            addTextCoord(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
        }

        // put indices for sides
        GLuint k1, k2;
        for (GLint i = 0; i < numStacks; ++i)
        {
            k1 = i * (numSlices + 1);     // bebinning of current stack
            k2 = k1 + numSlices + 1;      // beginning of next stack

            for (GLint j = 0; j < numSlices; ++j, ++k1, ++k2)
            {
                // 2 trianles per sector
                addIndices(k1, k1 + 1, k2);
                addIndices(k2, k1 + 1, k2 + 1);

                // vertical lines for all slices
                lineIndices.push_back(k1);
                lineIndices.push_back(k2);
                // horizontal lines
                lineIndices.push_back(k2);
                lineIndices.push_back(k2 + 1);
                if (i == 0)
                {
                    lineIndices.push_back(k1);
                    lineIndices.push_back(k1 + 1);
                }
            }
        }

        // remember where the base indices start
        bIndex = (GLuint)indices.size();

        // put indices for base
        for (GLint i = 0, k = baseVertexIndex + 1; i < numSlices; ++i, ++k)
        {
            if (i < (numSlices - 1))
                addIndices(baseVertexIndex, k + 1, k);
            else    // last triangle
                addIndices(baseVertexIndex, baseVertexIndex + 1, k);
        }

        // remember where the base indices start
        tIndex = (GLuint)indices.size();

        for (GLint i = 0, k = topVertexIndex + 1; i < numSlices; ++i, ++k)
        {
            if (i < (numSlices - 1))
                addIndices(topVertexIndex, k, k + 1);
            else
                addIndices(topVertexIndex, k, topVertexIndex + 1);
        }

        // generate interleaved vertex array as well
        buildInterleavedVertices();
    }

    void Cylinder::buildVerticesFlat()
    {
        // tmp vertex definition (x,y,z,s,t)
        struct Vertex
        {
            GLfloat x, y, z, s, t;
        };
        vector<Vertex> tmpVertices;

        GLint i, j, k;    // indices
        GLfloat x, y, z, s, t, radius;

        // put tmp vertices of cylinder side to array by scaling unit circle
        //NOTE: start and end vertex positions are same, but texcoords are different
        //      so, add additional vertex at the end point
        for (i = 0; i <= numStacks; ++i)
        {
            z = -(height * 0.5f) + (GLfloat)i / numStacks * height;      // vertex position z
            radius = bRadius + (GLfloat)i / numStacks * (tRadius - bRadius);     // lerp
            t = 1.0f - (GLfloat)i / numStacks;   // top-to-bottom

            for (j = 0, k = 0; j <= numSlices; ++j, k += 3)
            {
                x = unitCircleVertices[k];
                y = unitCircleVertices[k + 1];
                s = (GLfloat)j / numSlices;

                Vertex vertex;
                vertex.x = x * radius;
                vertex.y = y * radius;
                vertex.z = z;
                vertex.s = s;
                vertex.t = t;
                tmpVertices.push_back(vertex);
            }
        }

        // clear memory of prev arrays
        clearArrays();

        Vertex v1, v2, v3, v4;      // 4 vertex positions v1, v2, v3, v4
        vector<GLfloat> n;       // 1 face normal
        GLint vi1, vi2;               // indices
        GLint index = 0;

        // v2-v4 <== stack at i+1
        // | \ |
        // v1-v3 <== stack at i
        for (i = 0; i < numStacks; ++i)
        {
            vi1 = i * (numSlices + 1);            // index of tmpVertices
            vi2 = (i + 1) * (numSlices + 1);

            for (j = 0; j < numSlices; ++j, ++vi1, ++vi2)
            {
                v1 = tmpVertices[vi1];
                v2 = tmpVertices[vi2];
                v3 = tmpVertices[vi1 + 1];
                v4 = tmpVertices[vi2 + 1];

                // compute a face normal of v1-v3-v2
                n = calcFaceNorm(v1.x, v1.y, v1.z, v3.x, v3.y, v3.z, v2.x, v2.y, v2.z);

                // put quad vertices: v1-v2-v3-v4
                addVert(v1.x, v1.y, v1.z);
                addVert(v2.x, v2.y, v2.z);
                addVert(v3.x, v3.y, v3.z);
                addVert(v4.x, v4.y, v4.z);

                // put tex coords of quad
                addTextCoord(v1.s, v1.t);
                addTextCoord(v2.s, v2.t);
                addTextCoord(v3.s, v3.t);
                addTextCoord(v4.s, v4.t);

                // put normal
                for (k = 0; k < 4; ++k)  // same normals for all 4 vertices
                {
                    addNorm(n[0], n[1], n[2]);
                }

                // put indices of a quad
                addIndices(index, index + 2, index + 1);    // v1-v3-v2
                addIndices(index + 1, index + 2, index + 3);    // v2-v3-v4

                // vertical line per quad: v1-v2
                lineIndices.push_back(index);
                lineIndices.push_back(index + 1);
                // horizontal line per quad: v2-v4
                lineIndices.push_back(index + 1);
                lineIndices.push_back(index + 3);
                if (i == 0)
                {
                    lineIndices.push_back(index);
                    lineIndices.push_back(index + 2);
                }

                index += 4;     // for next
            }
        }

        // remember where the base index starts
        bIndex = (GLuint)indices.size();
        GLuint baseVertexIndex = (GLuint)vertices.size() / 3;

        // put vertices of base of cylinder
        z = -height * 0.5f;
        addVert(0, 0, z);
        addNorm(0, 0, -1);
        addTextCoord(0.5f, 0.5f);
        for (i = 0, j = 0; i < numSlices; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVert(x * bRadius, y * bRadius, z);
            addNorm(0, 0, -1);
            addTextCoord(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f); // flip horizontal
        }

        // put indices for base
        for (i = 0, k = baseVertexIndex + 1; i < numSlices; ++i, ++k)
        {
            if (i < numSlices - 1)
                addIndices(baseVertexIndex, k + 1, k);
            else
                addIndices(baseVertexIndex, baseVertexIndex + 1, k);
        }

        // remember where the top index starts
        tIndex = (GLuint)indices.size();
        GLuint topVertexIndex = (GLuint)vertices.size() / 3;

        // put vertices of top of cylinder
        z = height * 0.5f;
        addVert(0, 0, z);
        addNorm(0, 0, 1);
        addTextCoord(0.5f, 0.5f);
        for (i = 0, j = 0; i < numSlices; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVert(x * tRadius, y * tRadius, z);
            addNorm(0, 0, 1);
            addTextCoord(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
        }

        for (i = 0, k = topVertexIndex + 1; i < numSlices; ++i, ++k)
        {
            if (i < numSlices - 1)
                addIndices(topVertexIndex, k, k + 1);
            else
                addIndices(topVertexIndex, k, topVertexIndex + 1);
        }

        // generate interleaved vertex array as well
        buildInterleavedVertices();
    }

    void Cylinder::buildInterleavedVertices()
    {
        vector<GLfloat>().swap(iVerts);

        size_t i, j;
        size_t count = vertices.size();
        for (i = 0, j = 0; i < count; i += 3, j += 2)
        {
            iVerts.insert(iVerts.end(), &vertices[i], &vertices[i] + 3);

            iVerts.insert(iVerts.end(), &normals[i], &normals[i] + 3);

            iVerts.insert(iVerts.end(), &texCoords[j], &texCoords[j] + 2);
        }
    }

    void Cylinder::buildUnitCircleVertices()
    {
        const GLfloat PI = acos(-1);
        GLfloat sectorStep = 2 * PI / numSlices;
        GLfloat sectorAngle;  // radians

        vector<GLfloat>().swap(unitCircleVertices);
        for (GLint i = 0; i <= numSlices; ++i)
        {
            sectorAngle = i * sectorStep;
            unitCircleVertices.push_back(cos(sectorAngle)); // x
            unitCircleVertices.push_back(sin(sectorAngle)); // y
            unitCircleVertices.push_back(0);                // z
        }
    }

    void Cylinder::addVert(GLfloat x, GLfloat y, GLfloat z)
    {
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    void Cylinder::addNorm(GLfloat nx, GLfloat ny, GLfloat nz)
    {
        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(nz);
    }

    void Cylinder::addTextCoord(GLfloat s, GLfloat t)
    {
        texCoords.push_back(s);
        texCoords.push_back(t);
    }

    void Cylinder::addIndices(GLuint i1, GLuint i2, GLuint i3)
    {
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i3);
    }

    vector<GLfloat> Cylinder::getSideNorms()
    {
        const GLfloat PI = acos(-1);
        GLfloat sectorStep = 2 * PI / numSlices;
        GLfloat sectorAngle;  // radian

        // compute the normal vector at 0 degree first
        // tanA = (bRadius-tRadius) / height
        GLfloat zAngle = atan2(bRadius - tRadius, height);
        GLfloat x0 = cos(zAngle);     // nx
        GLfloat y0 = 0;               // ny
        GLfloat z0 = sin(zAngle);     // nz

        // rotate (x0,y0,z0) per sector angle
        vector<GLfloat> normals;
        for (GLint i = 0; i <= numSlices; ++i)
        {
            sectorAngle = i * sectorStep;
            normals.push_back(cos(sectorAngle) * x0 - sin(sectorAngle) * y0);   // nx
            normals.push_back(sin(sectorAngle) * x0 + cos(sectorAngle) * y0);   // ny
            normals.push_back(z0);  // nz
        }

        return normals;
    }

    vector<GLfloat> Cylinder::calcFaceNorm(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2, GLfloat x3, GLfloat y3, GLfloat z3) {

        const GLfloat EPSILON = 0.000001f;

        vector<GLfloat> normal(3, 0.0f);     // default return value (0,0,0)
        GLfloat nx, ny, nz;

        // find 2 edge vectors: v1-v2, v1-v3
        GLfloat ex1 = x2 - x1;
        GLfloat ey1 = y2 - y1;
        GLfloat ez1 = z2 - z1;
        GLfloat ex2 = x3 - x1;
        GLfloat ey2 = y3 - y1;
        GLfloat ez2 = z3 - z1;

        // cross product: e1 x e2
        nx = ey1 * ez2 - ez1 * ey2;
        ny = ez1 * ex2 - ex1 * ez2;
        nz = ex1 * ey2 - ey1 * ex2;

        // normalize only if the length is > 0
        GLfloat length = sqrtf(nx * nx + ny * ny + nz * nz);
        if (length > EPSILON)
        {
            // normalize
            GLfloat lengthInv = 1.0f / length;
            normal[0] = nx * lengthInv;
            normal[1] = ny * lengthInv;
            normal[2] = nz * lengthInv;
        }

        return normal;
    }

    void Cylinder::printSelf() const
    {
                cout << "===== Cylinder =====\n"
            << "   Base Radius: " << bRadius << "\n"
            << "    Top Radius: " << tRadius << "\n"
            << "        Height: " << height << "\n"
            << "  Sector Count: " << numSlices << "\n"
            << "   Stack Count: " << numStacks << "\n"
            << "Triangle Count: " << getTriangleCount() << "\n"
            << "   Index Count: " << getIndexCount() << "\n"
            << "  Vertex Count: " << getVertCount() << "\n"
            << "  Normal Count: " << getNormCount() << "\n"
            << "TexCoord Count: " << getTextCoordCount() << std::endl;
    }