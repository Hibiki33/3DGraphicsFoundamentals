#version 330 core

#define PI 3.1415926
#define EPSILON 0.000001

out vec4 FragColor;

uniform vec3 point_light_pos;
uniform vec3 point_light_radiance;
uniform sampler2D shadowmap;				// 阴影贴图
uniform bool have_shadow;

//TODO: 2.2 添加变量，从世界空间映射到阴影映射平面空间
uniform mat4 lightSpaceMatrix;

uniform float ambient;
uniform float specular;
uniform sampler2D color_texture;

uniform vec3 camera_pos;

in VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;// 将裁切空间坐标的范围-w到w转为-1到1
																				// 变换到[0,1]以比较深度贴图的深度
    float closestDepth = texture(shadowmap, projCoords.xy).r;		// 最近距离
    float currentDepth = projCoords.z;								// 最近距离
	vec2 texelSize = 1.0 / textureSize(shadowmap, 0);				// 单独纹理片段的大小
	float bias = 0.0005;				// 添加偏移值 试验得出 0 < bias < 0.001 是比较合适的偏移量
										// 当bias == 0时 出现Shadow Acne
										// 当bias == 0.005f时 出现阴影丢失
	float shadow = 0.0;
	// 从片段四周对深度贴图采样并平均 以降低锯齿块
	for(int x = -2; x <= 2; x++)
	{
		for(int y = -2; y <= 2; y++)
		{
			float pcfDepth = texture(shadowmap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 25.0;						// 采样了周围共5 * 5个片段（像素） 平均
    return shadow;
}

void main() {
	vec3 color = texture(color_texture, vs_out.TexCoord).rgb;
	//TODO: 2.2 首先将第一次完成light_dn.frag的Phong shading 代码粘贴到这里，然后添加阴影
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	vec3 lightColor = point_light_radiance / 255.0;
	vec3 fragPos = vs_out.WorldPos;
	vec3 normal = normalize(vs_out.Normal);
	vec3 lightPos = point_light_pos;
	vec3 viewPos  = camera_pos;
    vec3 lightDirect = normalize(lightPos - fragPos);
    vec3 viewDirect = normalize(viewPos - fragPos);
	vec3 A = ambient * lightColor;
    float diffColor = max(dot(normal, lightDirect), 0.0);
    vec3 D = diffColor * lightColor;
    vec3 reflectDirect = reflect(-lightDirect, normal);  
    float specColor = pow(max(dot(viewDirect, reflectDirect), 0.0), 32);
    vec3 S = specular * specColor * lightColor;

	// 调整阴影
	float shadow = 0.0;
	if(have_shadow)
	{
		shadow = ShadowCalculation(fragPosLightSpace); 
	}

	vec3 Lo = color * (A + (1.0 - shadow) * (D + S));
	FragColor = vec4(Lo, 1.0);
}
