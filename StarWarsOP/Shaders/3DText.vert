#version 330 core

// Uniforms for world transform and view-proj
uniform mat4 uTranslate;
uniform mat4 uRotate;
uniform mat4 uView;
uniform mat4 uProj;

// Attribute 0 is position, 1 is normal, 2 is tex coords.
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

// Any vertex outputs (other than position)
out vec2 fragTexCoord;

void main()
{
	// Convert position to homogeneous coordinates
	vec4 pos = vec4(inPosition, 0.0, 1.0);
	gl_Position = uProj * uView * uTranslate  * uRotate * pos;

	// Pass along the texture coordinate to frag shader
	fragTexCoord = inTexCoord;
}