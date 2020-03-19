// VERTEX SHADER
#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
 
layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

uniform vec2 offset;

out vec4 color;
out vec4 position; // output variables sent off to the fragment shader
out vec3 normal;
out vec3 texCoordCubeMap; // cube map 3D texture

out vec2 texCoord0; // bitmap

// Shadow mapping
uniform mat4 matrixShadow;
out vec4 shadowCoord;


struct AMBIENT
{	
	int on;
	vec3 color;
};
uniform AMBIENT lightAmbient, lightAmbient2, lightAmbient3, lightAmbient4;

vec4 AmbientLight(AMBIENT light)
{
// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}


void main(void) 
{
	
	// calculate texture coordinate
	texCoord0 = aTexCoord;
	

	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;
	// calculate reflection vector (cube map)
	//This makes the reflective object transparent!---> // texCoordCubeMap = inverse(mat3(matrixView)) * mix(reflect(position.xyz, normal.xyz), normal.xyz, 0.8);
	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);
	
	// calculate light
	color = vec4(0, 0, 0, 1);
	
	

	if (lightAmbient.on == 1) 
		color += AmbientLight(lightAmbient);
	if (lightAmbient2.on == 1)
		color += AmbientLight(lightAmbient2);
	if (lightAmbient3.on == 1)
		color += AmbientLight(lightAmbient3);
	if (lightAmbient4.on == 1)
		color += AmbientLight(lightAmbient4);
	if (lightDir.on == 1) 
		color += DirectionalLight(lightDir);
	
	// calculate reflection vector (cube map)
	texCoordCubeMap = inverse(mat3(matrixView)) * mix(reflect(position.xyz, normal.xyz), normal.xyz, 0.95);

	// calculate shadow coordinate – using the Shadow Matrix
	mat4 matrixModel = inverse(matrixView) * matrixModelView;
	shadowCoord = matrixShadow * matrixModel * vec4(aVertex, 1);
}




