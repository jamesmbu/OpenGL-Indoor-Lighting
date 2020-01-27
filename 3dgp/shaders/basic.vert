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

out vec4 color;
vec4 position;
vec3 normal;

struct AMBIENT
{	
	int on;
	vec3 color;
};
uniform AMBIENT lightAmbient;

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

struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2;

vec4 PointLight(POINT light)
{
	// Calculate Point Light
	vec4 color = vec4(0, 0, 0, 0);
	//vec3 L = (normalize((matrixView) * vec4(light.position, 1) - position)).xyz;
	vec3 L = normalize((matrixView * (vec4(light.position, 1))) - vec4(aVertex, 1.0)).xyz;
	
	float NdotL = dot(normal, L);
	
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);	
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	
	
	return color;

}


void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;
	
	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);
	
	// calculate light
	color = vec4(0, 0, 0, 1);
	
	if (lightAmbient.on == 1) 
		color += AmbientLight(lightAmbient);

	if (lightDir.on == 1) 
		color += DirectionalLight(lightDir);
	
	// multiple light points
	if (lightPoint1.on == 1) 
		color += PointLight(lightPoint1);
	if (lightPoint2.on == 1) 
		color += PointLight(lightPoint2);


}




