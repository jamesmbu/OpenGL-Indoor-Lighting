// FRAGMENT SHADER

#version 330
// Matrices
uniform mat4 matrixView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

//processing the texture
uniform sampler2D texture0;



in vec4 position;
in vec3 normal;

in vec4 color;
out vec4 outColor;

in vec2 texCoord0; // bitmap

//cube map stuff
in vec3 texCoordCubeMap;
uniform samplerCube textureCubeMap;
uniform float reflectionPower;



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
	vec3 L = normalize((matrixView * (vec4(light.position, 1))) - position).xyz;
	
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

struct SPOT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;

	vec3 direction;
	float cutoff;
	float attenuation;

	mat4 matrix;
};
uniform SPOT spotLight1;

vec4 SpotLight(SPOT light)
{
	// Calculate Point Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize((light.matrix * (vec4(light.position, 1))) - position).xyz;
	
	float NdotL = dot(normal, L);
	
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);	
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	vec3 D = normalize(mat3(light.matrix) * (light.direction));
	float spotFactor = dot(-L, D);
	float angle_ = acos(spotFactor);
	if (angle_ <= clamp(radians(light.cutoff),0.0f,90.0f))
	{
		spotFactor = pow(spotFactor, light.attenuation);
	}
	else if (angle_ > clamp(radians(light.cutoff),0.0f,90.0f))
	{
		spotFactor = 0.0f;
	}
	return spotFactor * color;
}

void main(void) 
{
	outColor = color;

  	// multiple light points
	if (lightPoint1.on == 1) 
		outColor += PointLight(lightPoint1);
	if (lightPoint2.on == 1) 
		outColor += PointLight(lightPoint2);

	if (spotLight1.on == 1)
		outColor += SpotLight(spotLight1);
	//outColor *= texture(texture0, texCoord0);

	outColor = mix(outColor * texture(texture0, texCoord0.st), texture(textureCubeMap, texCoordCubeMap), reflectionPower);

}
