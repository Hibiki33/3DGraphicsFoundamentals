#version 330 core

#define PI 3.1415926
#define EPSILON 0.000001

out vec4 FragColor;							// 输出片段着色器颜色

uniform vec3 point_light_pos;				// 点光源位置
uniform vec3 point_light_radiance;			// 光线

uniform float ambient;						// 环境光强度
uniform sampler2D color_texture;			// 纹理颜色
uniform sampler2D normalmap;				// 法线贴图
uniform bool have_normal_and_displacement;	// 是否加上法线贴图和置换贴图

uniform vec3 camera_pos;					// 视角坐标位置

in VS_OUT {
    vec3 WorldPos;							// 片段的世界坐标
    vec2 TexCoord;							// 片段的纹理坐标
    mat3 TBN;								// 这里T和B都为0 N是片段的法向量
} vs_out;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main() {
	vec3 albedo = texture(color_texture, vs_out.TexCoord).rgb;
	// TODO: 2.0 Phong Shading
	// 将光线向量的每个分量化为0.0-1.0之间的浮点数 转化为光线颜色
	vec3 lightColor = point_light_radiance / 255.0;
	vec3 fragPos = vs_out.WorldPos;
	// 获得朝向相机方向的归一化的法向量
	vec3 normal = normalize(vs_out.TBN[2]);
	if (dot((camera_pos - vs_out.WorldPos), normal) < -EPSILON) 
	{
		normal = -normal;
	}
	vec3 lightPos = point_light_pos;
	vec3 viewPos  = camera_pos;
    vec3 lightDirect = normalize(lightPos - fragPos);
    vec3 viewDirect = normalize(viewPos - fragPos);
	
	if(have_normal_and_displacement)
	{
		// TODO: 2.1 法线贴图 
		lightPos = transpose(vs_out.TBN) * lightPos;
		viewPos = transpose(vs_out.TBN) * viewPos;
		fragPos = transpose(vs_out.TBN) * fragPos;

        lightDirect = normalize(lightPos - fragPos);
        viewDirect = normalize(viewPos - fragPos);

        normal = texture(normalmap, vs_out.TexCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0); // 将法线向量转换为范围[-1,1] or normalize(normal * 0.5 + 0.5)
		if (dot((viewPos - fragPos), normal) < -EPSILON) 
		{
			normal = -normal;
		}
	}

	// 环境光
	vec3 A = ambient * lightColor;
	// 漫反射 
    float diffColor = max(dot(normal, lightDirect), 0.0);
    vec3 D = diffColor * lightColor;
	// 高光
	float specular = 0.5;
    vec3 reflectDirect = reflect(-lightDirect, normal);  
    float specColor = pow(max(dot(viewDirect, reflectDirect), 0.0), 32);
    vec3 S = specular * specColor * lightColor;

	// vec3 Lo = albedo; (orig)
	FragColor = vec4((A + D + S) * albedo, 1.0);
}

