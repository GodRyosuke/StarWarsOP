#version 330 core

// Tex coord input from vertex shader
in vec2 fragTexCoord;

// This corresponds to the output color to the color buffer
out vec4 outColor;

// This is used for the texture sampling
uniform vec3 textColor;
uniform sampler2D uTexture;
uniform float uTextAlpha;


void main()
{
	// vec4 sampled = vec4(textColor.x, textColor.y, texture(uTexture, fragTexCoord).r, uTextAlpha);
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(uTexture, fragTexCoord).r * uTextAlpha);
	// Sample color from texture
    // outColor = texture(uTexture, fragTexCoord);
	// outColor = sampled;
	outColor = vec4(textColor, 1.0) * sampled;
	// outColor.a = uTextAlpha;
}
