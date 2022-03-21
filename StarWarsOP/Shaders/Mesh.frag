#version 330 core

// Inputs the color from the Vertex Shader
// in vec3 color;
// Inputs the texture coordinates from the Vertex Shader
in vec2 texCoord;
in vec3 fragNormal;
in vec3 fragWorldPos;

// Gets the Texture Unit from the main function
uniform sampler2D tex0;

// Camera position (in world space)
uniform vec3 uCameraPos;


// light settings
// Ambient light level
uniform vec3 uAmbientLight;
uniform float uSpecPower;

// Create a struct for directional light
struct DirectionalLight
{
	// Direction of light
	vec3 mDirection;
	// Diffuse color
	vec3 mDiffuseColor;
	// Specular color
	vec3 mSpecColor;
};
// Directional Light
uniform DirectionalLight uDirLight;


// Outputs colors in RGBA
out vec4 FragColor;

void main()
{
	// Surface normal
	vec3 N = normalize(fragNormal);
	// Vector from surface to light
	vec3 L = normalize(-uDirLight.mDirection);
	// Vector from surface to camera
	vec3 V = normalize(uCameraPos - fragWorldPos);
	// Reflection of -L about N
	vec3 R = normalize(reflect(-L, N));

	// Compute phong reflection
	vec3 Phong = uAmbientLight;
	float NdotL = dot(N, L);
	if (NdotL > 0)
	{
		vec3 Diffuse = uDirLight.mDiffuseColor * NdotL;
		vec3 Specular = uDirLight.mSpecColor * pow(max(0.0, dot(R, V)), uSpecPower);
		Phong += Diffuse + Specular;
	}

	// FragColor = texture(tex0, texCoord);
	FragColor = texture(tex0, texCoord) * vec4(Phong, 1.0f);
}