#version 330 core

#define PI 3.1415926
#define EPSILON 0.000001

out vec4 FragColor;

uniform vec3 point_light_pos;
uniform vec3 point_light_radiance;
uniform sampler2D shadowmap;
uniform bool have_shadow;

//TODO: 2.2 添加变量，从世界空间映射到阴影映射平面空间
uniform mat4 lightSpaceMatrix;

uniform float ambient;
uniform sampler2D color_texture;

uniform vec3 camera_pos;

in VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float closestDepth = texture(shadowmap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.0005;
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

void main() {
	vec3 color = texture(color_texture, vs_out.TexCoord).rgb;
	//TODO: 2.2 首先将第一次完成light_dn.frag的Phong shading 代码粘贴到这里，然后添加阴影
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	vec3 lightColor = point_light_radiance / 255.0;
	vec3 fragPos = vs_out.WorldPos;
	vec3 normal = normalize(vs_out.Normal);
	//if (dot((camera_pos - vs_out.WorldPos), normal) < -EPSILON) 
	//{
	//	normal = -normal;
	//}
	vec3 lightPos = point_light_pos;
	vec3 viewPos  = camera_pos;
    vec3 lightDirect = normalize(lightPos - fragPos);
    vec3 viewDirect = normalize(viewPos - fragPos);
	vec3 A = ambient * lightColor;
    float diffColor = max(dot(normal, lightDirect), 0.0);
    vec3 D = diffColor * lightColor;
	float specular = 0.5;
    vec3 reflectDirect = reflect(-lightDirect, normal);  
    float specColor = pow(max(dot(viewDirect, reflectDirect), 0.0), 32);
    vec3 S = specular * specColor * lightColor;

	float shadow = 0.0;
	if(have_shadow)
	{
		shadow = ShadowCalculation(fragPosLightSpace);   
	}

	vec3 Lo = color * (A + (1.0 - shadow) * (D + S));
	FragColor = vec4(Lo, 1.0);
}
