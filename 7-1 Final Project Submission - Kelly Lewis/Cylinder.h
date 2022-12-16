#pragma once

//Cylinder Header modeled after Song Ho Ahn (http://www.songho.ca/opengl/gl_cylinder.html)

#ifndef GEOMETRY_CYLINDER_H
#define GEOMETRY_CYLINDER_H

#include <iostream>         // Output log and error
#include <cstdlib>          // C Standard Library for EXIT_FAILURE
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // standard namespace

	class Cylinder {
	public:
		Cylinder(GLfloat bRadius = 1.0f, GLfloat tRadius = 1.0f, GLfloat height = 1.0f, GLint numSlices = 36, GLint numStacks = 1);
		~Cylinder() {}

		// Main Attributes
		// -----------------
		//Setters / Mutators
		void set(GLfloat bRadius = 1.0f, GLfloat tRadius = 1.0f, GLfloat height = 1.0f, GLint numSlices = 36, GLint numStacks = 1);
		void setBaseRadius(GLfloat newBRadius);
		void setTopRadius(GLfloat newTRadius);
		void setHeight(GLfloat newHeight);
		void setSectorCount(GLint newSlices);
		void setStackCount(GLint newStacks);

		//Getters / Accessors
		GLfloat getBaseRadius()		const { return bRadius; }
		GLfloat getTopRadius()		const { return tRadius; }
		GLfloat getHeight()			const { return height; }
		GLint	getSectorCount()	const { return numSlices; }
		GLint	getStackCount()		const { return numStacks; }

		//Vertex Attributes
		//------------------
		//Getters / Accessors
		GLuint getVertCount()		const { return (GLuint)vertices.size() / 3; }
		GLuint getVertSize()		const { return (GLuint)vertices.size() * sizeof(GLfloat); }

		GLuint getNormCount()		const { return (GLuint)normals.size() / 3; }
		GLuint getNormSize()		const { return (GLuint)normals.size() * sizeof(GLfloat); }

		GLuint getTextCoordCount()	const { return (GLuint)texCoords.size() / 2; }
		GLuint getTextCoordSize()	const { return (GLuint)texCoords.size() * sizeof(GLfloat); }

		GLuint getIndexCount()		const { return (GLuint)indices.size(); }
		GLuint getIndexSize()		const { return (GLuint)indices.size() * sizeof(GLuint); }
		const GLuint* getStartIndex() const { return &indices[0]; }

		GLuint getLineIndexCount()	const { return (GLuint)lineIndices.size(); }
		GLuint getLineIndexSize()	const { return (GLuint)lineIndices.size() * sizeof(GLuint); }

		GLuint getTriangleCount()	const { return getIndexCount() / 3; }

		const GLfloat* getVerts()	const { return vertices.data(); }
		const GLfloat* getNorms()		const { return normals.data(); }
		const GLfloat* getTextCoords()	const { return texCoords.data(); }
		const GLuint* getIndices()		const { return indices.data(); }
		const GLuint* getLineIndices()	const { return lineIndices.data(); }

		//Getters for Invterleaved Vertices
		GLuint	getIVertCount() const { return getVertCount(); }    // # of vertices
		GLuint	getIVertSize()	const { return (GLuint)iVerts.size() * sizeof(GLuint); }    // # of bytes
		GLint	getIStride()		const { return iStride; }   // should be 32 bytes
		const GLfloat* getIVerts() const { return &iVerts[0]; }

		//Getters for the indices of base, top, and sides
		GLuint getBaseIndexCount()	const { return ((GLuint)indices.size() - bIndex) / 2; }
		GLuint getTopIndexCount()	const { return ((GLuint)indices.size() - bIndex) / 2; }

		GLuint getSideIndexCount()	const { return bIndex; }
		GLuint getBaseStartIndex()	const { return bIndex; }
		GLuint getTopStartIndex()	const { return tIndex; }
		GLuint getSideStartIndex()	const { return 0; }

		//Draw Func Declaration
		void draw() const;

		// debug
		void printSelf() const;

	private:
		//Private Funcs
		void clearArrays();

		void buildVerticesSmooth();
		void buildVerticesFlat();
		void buildInterleavedVertices();
		void buildUnitCircleVertices();

		void addVert(GLfloat x, GLfloat y, GLfloat z);
		void addNorm(GLfloat x, GLfloat y, GLfloat z);
		void addTextCoord(GLfloat s, GLfloat t);
		void addIndices(GLuint i1, GLuint i2, GLuint i3);

		//Normals Vectors
		vector<GLfloat> getSideNorms();
		vector<GLfloat> calcFaceNorm(GLfloat x1, GLfloat y1, GLfloat z1,
			GLfloat x2, GLfloat y2, GLfloat z2,
			GLfloat x3, GLfloat y3, GLfloat z3);

		//Member Variables
		GLfloat bRadius;
		GLfloat tRadius;
		GLfloat height;
		GLint numSlices;
		GLint numStacks;
		GLuint bIndex;
		GLuint tIndex;

		vector<GLfloat> unitCircleVertices;
		vector<GLfloat> vertices;
		vector<GLfloat> normals;
		vector<GLfloat> texCoords;
		vector<GLuint>	indices;
		vector<GLuint>  lineIndices;

		// interleaved
		vector<GLfloat> iVerts;	  //interleaved verts vector, for smoothing
		GLint iStride;                  // Stride should be 32

	};
#endif
//END