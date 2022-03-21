#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec3 inNormal;
// Texture Coordinates
layout (location = 2) in vec2 aTex;


// Outputs the color for the Fragment Shader
// out vec3 color;
// Outputs the texture coordinates to the fragment shader
out vec2 texCoord;
out vec3 fragNormal;
out vec3 fragWorldPos;

// Controls the scale of the vertices
uniform float scale;

// Inputs the matrices needed for 3D viewing with perspective
uniform mat4 uTranslate;
uniform mat4 uRotate;
uniform mat4 uScale;
uniform mat4 uView;
uniform mat4 uProj;


void main()
{
	// Outputs the positions/coordinates of all vertices
	mat4 WorldTransform = uTranslate * uRotate * uScale;
	gl_Position = uProj * uView * WorldTransform * vec4(aPos, 1.0);
	// Assigns the colors from the Vertex Data to "color"
	// color = aColor;
	// Assigns the texture coordinates from the Vertex Data to "texCoord"
	texCoord = aTex;
	fragNormal = (WorldTransform * vec4(inNormal, 0.0f)).xyz;
	fragWorldPos = (WorldTransform * vec4(aPos, 1.0)).xyz;
}